// lab2-event-synchronization.c - Priority-based Pipeline (ESP-IDF v5.x / FreeRTOS)
// Pipeline 4 stages, each stage processes jobs in priority order (HIGH > MED > LOW)
// with event-driven wakeup and aging-based promotion to avoid starvation.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "driver/gpio.h"

static const char *TAG = "PRIO_PIPELINE";

// ===== GPIO indicators (optional) ============================================
#define LED_STAGE0  GPIO_NUM_4
#define LED_STAGE1  GPIO_NUM_5
#define LED_STAGE2  GPIO_NUM_18
#define LED_STAGE3  GPIO_NUM_19

static inline void led_on(gpio_num_t p){ gpio_set_level(p, 1); }
static inline void led_off(gpio_num_t p){ gpio_set_level(p, 0); }

// ===== Pipeline & Priority config ============================================
#define NUM_STAGES      4
#define PRI_HIGH        0
#define PRI_MED         1
#define PRI_LOW         2
#define NUM_PRIOS       3

// Aging: promote LOW/MED if waited longer than this (ms)
#define AGING_THRESHOLD_MS  6000

// Max queue depth per priority per stage
#define QUEUE_DEPTH_PER_PRIO  8

// Base processing time per stage (ms) + jitter
static const uint32_t STAGE_COST_MS[NUM_STAGES] = { 180, 260, 220, 160 };

// ===== EventGroup bits (one group for all stages) ============================
// Use 3 bits per stage (HIGH/MED/LOW) = 12 bits total
// Mapping: bit index = stage*3 + pri
#define PIPE_EVENT_BIT(stage, pri)   (1 << ((stage)*NUM_PRIOS + (pri)))
#define PIPE_EVENT_MASK(stage)       ( PIPE_EVENT_BIT(stage, PRI_HIGH) | PIPE_EVENT_BIT(stage, PRI_MED) | PIPE_EVENT_BIT(stage, PRI_LOW) )

static EventGroupHandle_t pipe_events;

// ===== Job data ==============================================================
typedef uint8_t job_prio_t;

typedef struct {
    uint32_t    id;
    job_prio_t  pri;          // 0=HIGH,1=MED,2=LOW
    uint32_t    stage;        // 0..3
    uint64_t    enq_ts_us;    // enqueue timestamp (for aging)
    float       payload[4];   // mock data
    uint32_t    promoted;     // times promoted by aging
} job_t;

// ===== Queues: per-stage per-priority =======================================
static QueueHandle_t q_in[NUM_STAGES][NUM_PRIOS];

// ===== Stats ================================================================
typedef struct {
    uint32_t produced[NUM_PRIOS];
    uint32_t consumed[NUM_STAGES][NUM_PRIOS];
    uint32_t completed;            // increment when leaving stage 3
    uint32_t promoted_count;       // aging promotions
} pipe_stats_t;

static pipe_stats_t stats = {0};

// ===== Utilities ============================================================
static uint64_t now_us(void){ return esp_timer_get_time(); }

static const char* pri_name(job_prio_t p){
    switch(p){ case PRI_HIGH: return "HIGH"; case PRI_MED: return "MED"; default: return "LOW"; }
}

static gpio_num_t stage_led(uint32_t stage){
    switch(stage){
        case 0: return LED_STAGE0;
        case 1: return LED_STAGE1;
        case 2: return LED_STAGE2;
        default:return LED_STAGE3;
    }
}

// Promote priority (LOW->MED->HIGH) if waited too long
static job_prio_t maybe_promote(job_prio_t p, uint64_t waited_ms, uint32_t *promoted_counter){
    if (waited_ms >= AGING_THRESHOLD_MS){
        if (p == PRI_LOW)  { if (promoted_counter) (*promoted_counter)++; return PRI_MED; }
        if (p == PRI_MED)  { if (promoted_counter) (*promoted_counter)++; return PRI_HIGH; }
    }
    return p;
}

// Try get one job from stage queues in priority order. If none, wait on event bits.
static bool stage_take_job(uint32_t stage, job_t *out_job){
    // 1) Non-blocking try in priority order
    for (int p = PRI_HIGH; p <= PRI_LOW; ++p){
        if (xQueueReceive(q_in[stage][p], out_job, 0) == pdTRUE){
            return true;
        }
    }
    // 2) Block until any pri arrives for this stage
    (void)xEventGroupWaitBits(
        pipe_events,
        PIPE_EVENT_MASK(stage),
        pdFALSE,     // don't clear here
        pdFALSE,     // any of them
        portMAX_DELAY
    );

    // 3) After wake, try again (still high->low)
    for (int p = PRI_HIGH; p <= PRI_LOW; ++p){
        if (xQueueReceive(q_in[stage][p], out_job, 0) == pdTRUE){
            return true;
        }
    }
    return false; // unlikely, but safe
}

// Enqueue into a stage with priority; set event bit
static bool stage_enqueue(uint32_t stage, const job_t *job){
    if (xQueueSend(q_in[stage][job->pri], job, 0) == pdTRUE){
        xEventGroupSetBits(pipe_events, PIPE_EVENT_BIT(stage, job->pri));
        return true;
    }
    return false;
}

// ===== Stage task ============================================================
static void stage_task(void *pv){
    uint32_t stage = (uint32_t)pv;
    char tag[16]; snprintf(tag, sizeof(tag), "STAGE_%lu", (unsigned long)stage);
    ESP_LOGI(TAG, "%s started (prio order: HIGH->MED->LOW)", tag);

    gpio_num_t led = stage_led(stage);

    while (1){
        job_t job;
        if (!stage_take_job(stage, &job)){
            continue;
        }

        // Clear bit for this pri if queue became empty (helps reduce spurious wakeups)
        if (uxQueueMessagesWaiting(q_in[stage][job.pri]) == 0){
            xEventGroupClearBits(pipe_events, PIPE_EVENT_BIT(stage, job.pri));
        }

        // Stats: consumed
        stats.consumed[stage][job.pri]++;

        // Aging promotion BEFORE processing (if waited too long in the queue)
        uint64_t waited_ms = (now_us() - job.enq_ts_us) / 1000ULL;
        job_prio_t old_pri = job.pri;
        job.pri = maybe_promote(job.pri, waited_ms, &stats.promoted_count);
        if (job.pri != old_pri){
            ESP_LOGW(TAG, "Stage%lu: Job#%lu promoted by aging %s->%s (waited=%llums, promoted_count=%lu)",
                     (unsigned long)stage, (unsigned long)job.id,
                     pri_name(old_pri), pri_name(job.pri),
                     (unsigned long long)waited_ms,
                     (unsigned long)stats.promoted_count);
        }

        // "Processing"
        led_on(led);
        uint32_t base = STAGE_COST_MS[stage];
        // HIGH slightly faster, LOW slightly slower
        int32_t pri_adjust = (job.pri==PRI_HIGH ? -40 : (job.pri==PRI_LOW ? +60 : 0));
        uint32_t jitter = 50 + (esp_random() % 80);
        uint32_t proc_ms = (uint32_t) fmaxf(40.0f, (float)base + pri_adjust + jitter);
        vTaskDelay(pdMS_TO_TICKS(proc_ms));
        led_off(led);

        // Next stage
        if (stage + 1 < NUM_STAGES){
            job.stage = stage + 1;
            job.enq_ts_us = now_us(); // reset queue wait timer for next stage
            if (!stage_enqueue(job.stage, &job)){
                // If full, retry once as MED (simple backpressure handling)
                ESP_LOGW(TAG, "Stage%lu->%lu: queue full for %s, retry as MED",
                         (unsigned long)stage, (unsigned long)(stage+1), pri_name(job.pri));
                job.pri = PRI_MED;
                (void)stage_enqueue(job.stage, &job);
            } else {
                ESP_LOGI(TAG, "Stage%lu -> Stage%lu: Job#%lu (%s) proc=%lums",
                         (unsigned long)stage, (unsigned long)(stage+1),
                         (unsigned long)job.id, pri_name(job.pri), (unsigned long)proc_ms);
            }
        } else {
            // Finished
            stats.completed++;
            uint64_t local_ms = (now_us() - job.enq_ts_us) / 1000ULL; // since last enqueue into stage3
            ESP_LOGI(TAG, "✅ OUTPUT: Job#%lu (%s) done at Stage%lu (proc=%lums, local_latency=%llums, total_done=%lu)",
                     (unsigned long)job.id, pri_name(job.pri), (unsigned long)stage,
                     (unsigned long)proc_ms, (unsigned long long)local_ms,
                     (unsigned long)stats.completed);
        }
    }
}

// ===== Generator task ========================================================
// Generate jobs with skewed priority distribution, enqueue to Stage0.
static void generator_task(void *pv){
    (void)pv;
    ESP_LOGI(TAG, "GEN started");
    uint32_t id = 0;

    while (1){
        job_t j = {0};
        j.id = ++id;
        // skew: HIGH 40% / MED 35% / LOW 25%
        uint32_t r = esp_random() % 100U;
        if (r < 40U) j.pri = PRI_HIGH;
        else if (r < 75U) j.pri = PRI_MED;
        else j.pri = PRI_LOW;

        j.stage = 0;
        j.enq_ts_us = now_us();
        j.promoted = 0;
        for (int i=0;i<4;i++) j.payload[i] = (esp_random()%1000)/10.0f;

        if (stage_enqueue(0, &j)){
            stats.produced[j.pri]++;
            ESP_LOGI(TAG, "GEN -> Stage0: Job#%lu (%s)", (unsigned long)j.id, pri_name(j.pri));
        } else {
            ESP_LOGW(TAG, "GEN: Stage0 queue full for %s, dropping Job#%lu",
                     pri_name(j.pri), (unsigned long)j.id);
        }

        // burst pattern: alternate slow/fast
        uint32_t sleep_ms = 500U + (esp_random() % 1500U);
        vTaskDelay(pdMS_TO_TICKS(sleep_ms));
    }
}

// ===== Stats monitor =========================================================
static void stats_task(void *pv){
    (void)pv;
    while (1){
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "\n📈 PIPELINE STATS (snapshot)");
        ESP_LOGI(TAG, "produced: HIGH=%lu MED=%lu LOW=%lu",
                 (unsigned long)stats.produced[PRI_HIGH],
                 (unsigned long)stats.produced[PRI_MED],
                 (unsigned long)stats.produced[PRI_LOW]);
        for (uint32_t s=0; s<NUM_STAGES; ++s){
            ESP_LOGI(TAG, "stage%lu consumed: H=%lu M=%lu L=%lu",
                     (unsigned long)s,
                     (unsigned long)stats.consumed[s][PRI_HIGH],
                     (unsigned long)stats.consumed[s][PRI_MED],
                     (unsigned long)stats.consumed[s][PRI_LOW]);
        }
        ESP_LOGI(TAG, "completed: %lu, promotions: %lu, free_heap=%d",
                 (unsigned long)stats.completed,
                 (unsigned long)stats.promoted_count,
                 esp_get_free_heap_size());
    }
}

// ===== Setup helpers =========================================================
static void gpio_init_outputs(void){
    gpio_config_t io = {
        .pin_bit_mask = (1ULL<<LED_STAGE0) | (1ULL<<LED_STAGE1) | (1ULL<<LED_STAGE2) | (1ULL<<LED_STAGE3),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);
    led_off(LED_STAGE0); led_off(LED_STAGE1); led_off(LED_STAGE2); led_off(LED_STAGE3);
}

static void queues_init(void){
    for (int s=0;s<NUM_STAGES;++s){
        for (int p=0;p<NUM_PRIOS;++p){
            q_in[s][p] = xQueueCreate(QUEUE_DEPTH_PER_PRIO, sizeof(job_t));
            configASSERT(q_in[s][p]);
        }
    }
}

void app_main(void){
    ESP_LOGI(TAG, "🚀 Priority-based Pipeline Starting...");

    gpio_init_outputs();

    pipe_events = xEventGroupCreate();
    configASSERT(pipe_events);

    queues_init();

    // Create stage tasks
    for (uint32_t s=0; s<NUM_STAGES; ++s){
        char name[16]; snprintf(name, sizeof(name), "Stage%lu", (unsigned long)s);
        xTaskCreate(stage_task, name, 3072, (void*)s, 6, NULL);
    }

    // Generator & stats
    xTaskCreate(generator_task, "Generator", 3072, NULL, 5, NULL);
    xTaskCreate(stats_task, "Stats", 3072, NULL, 3, NULL);

    ESP_LOGI(TAG, "Ready. LEDs: S0=GPIO%d, S1=GPIO%d, S2=GPIO%d, S3=GPIO%d",
             LED_STAGE0, LED_STAGE1, LED_STAGE2, LED_STAGE3);
    ESP_LOGI(TAG, "Policy: HIGH->MED->LOW with aging >= %d ms (promotion MED/HIGH).", AGING_THRESHOLD_MS);
}
