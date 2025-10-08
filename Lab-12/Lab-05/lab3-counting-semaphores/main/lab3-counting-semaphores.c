// main.c - Real-time Scheduler (Priority + Deadline + Load Balancing)
// ESP-IDF v5.x

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

static const char *TAG = "RT_SCHED";

// ====== Demo GPIO (optional visual) ======
#define LED_OK      GPIO_NUM_2     // แจ้งสถานะปกติ (วิ่งงาน)
#define LED_MISS    GPIO_NUM_4     // กระพริบเมื่อ deadline miss
#define LED_SCHED   GPIO_NUM_5     // กระพริบเมื่อ scheduler dispatch

// ====== Config ======
#define NUM_WORKERS         2      // worker tasks (pin ลงคนละ core)
#define WORKER_STACK        4096
#define SCHED_STACK         6144
#define MON_STACK           4096
#define LOAD_STACK          3072

#define SCHED_TICK_MS       10     // ช่วง scheduler loop (ไม่ใช่ period งาน)
#define DISPATCH_BUDGET     8      // max jobs dispatch per tick (กัน starvation)
#define WORKER_QUEUE_LEN    16
#define COMPLETE_QUEUE_LEN  32

// ====== Job Model ======
typedef struct {
    int     id;
    char    name[16];
    int     priority;          // มาก = สำคัญกว่า
    uint32_t period_ms;        // คาบของงาน
    uint32_t wcet_ms;          // เวลาใช้จริงโดยประมาณ (เดโม)
    uint32_t deadline_ms;      // relative deadline จาก release

    // runtime
    int64_t next_release_us;   // เวลา release รอบถัดไป
    // stats
    uint32_t releases;
    uint32_t completions;
    uint32_t deadline_miss;
    uint64_t sum_response_ms;
    uint32_t max_response_ms;
} job_desc_t;

typedef struct {
    int     job_id;
    int     priority;
    uint32_t exec_ms;
    int64_t  abs_deadline_us;
    int64_t  release_us;
} worker_cmd_t;

typedef struct {
    int     job_id;
    int64_t finish_us;
    int64_t abs_deadline_us;
    int64_t release_us;
} completion_t;

// ====== Demo jobs ======
enum { JOB_A = 0, JOB_B, JOB_C, NUM_JOBS };
static job_desc_t g_jobs[NUM_JOBS] = {
    { .id=JOB_A, .name="A", .priority=3, .period_ms=50,  .wcet_ms=12, .deadline_ms=40 },
    { .id=JOB_B, .name="B", .priority=2, .period_ms=100, .wcet_ms=20, .deadline_ms=60 },
    { .id=JOB_C, .name="C", .priority=1, .period_ms=200, .wcet_ms=60, .deadline_ms=150 },
};

// ====== Queues & Handles ======
static QueueHandle_t q_worker[NUM_WORKERS];
static QueueHandle_t q_complete;
static TaskHandle_t  h_worker[NUM_WORKERS];

// ====== Helpers ======
static inline int64_t now_us(void) { return esp_timer_get_time(); }

static void busy_exec_ms(uint32_t ms)
{
    // เดโม: ใช้ vTaskDelay เพื่อไม่บล็อก scheduler จริง (ถ้าต้องการ busy loop ให้เปลี่ยน)
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void blink_once(gpio_num_t pin, int ms)
{
    gpio_set_level(pin, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_set_level(pin, 0);
}

// เลือก worker ตามความยาวคิว (น้อยกว่าดีกว่า)
static int select_worker(void)
{
    int best = 0;
    UBaseType_t best_len = uxQueueMessagesWaiting(q_worker[0]);
    for (int i=1;i<NUM_WORKERS;i++){
        UBaseType_t l = uxQueueMessagesWaiting(q_worker[i]);
        if (l < best_len) { best = i; best_len = l; }
    }
    return best;
}

// ====== Worker ======
static void worker_task(void *arg)
{
    int wid = (int)(intptr_t)arg;
    gpio_num_t led = LED_OK;

    ESP_LOGI(TAG, "Worker %d start", wid);
    while (1) {
        worker_cmd_t cmd;
        if (xQueueReceive(q_worker[wid], &cmd, portMAX_DELAY) == pdTRUE) {
            // ทำงาน
            // int64_t start_us = now_us(); // ไม่ได้ใช้งาน - ถ้าต้องใช้ response breakdown ค่อยเปิด
            busy_exec_ms(cmd.exec_ms);
            int64_t fin_us = now_us();

            // ส่งผลกลับ
            completion_t c = {
                .job_id = cmd.job_id,
                .finish_us = fin_us,
                .abs_deadline_us = cmd.abs_deadline_us,
                .release_us = cmd.release_us
            };
            xQueueSend(q_complete, &c, 0);

            // วิชวลเล็กน้อย
            blink_once(led, 5);
        }
    }
}

// ====== Scheduler ======
typedef struct {
    int job_id;
    int priority;
    int64_t abs_deadline_us;
    int64_t release_us;
} ready_item_t;

static void scheduler_task(void *arg)
{
    // ตั้งค่า next_release ครั้งแรก
    int64_t t0 = now_us();
    for (int i=0;i<NUM_JOBS;i++){
        g_jobs[i].next_release_us = t0; // เริ่มพร้อมกัน
    }

    // เตรียม GPIO
    gpio_set_direction(LED_SCHED, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_OK, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_MISS, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_SCHED, 0);
    gpio_set_level(LED_OK, 0);
    gpio_set_level(LED_MISS, 0);

    ready_item_t ready[NUM_JOBS];

    ESP_LOGI(TAG, "Scheduler start (tick=%dms)", SCHED_TICK_MS);

    while (1) {
        int64_t tnow = now_us();

        // 1) เก็บ ready jobs
        int nready = 0;
        for (int i=0;i<NUM_JOBS;i++){
            if (tnow >= g_jobs[i].next_release_us) {
                // สร้าง instance ใหม่
                ready[nready].job_id = g_jobs[i].id;
                ready[nready].priority = g_jobs[i].priority;
                ready[nready].release_us = g_jobs[i].next_release_us;
                ready[nready].abs_deadline_us = g_jobs[i].next_release_us + (int64_t)g_jobs[i].deadline_ms*1000;
                nready++;

                // อัปเดต release รอบถัดไป (periodic)
                g_jobs[i].next_release_us += (int64_t)g_jobs[i].period_ms*1000;
                g_jobs[i].releases++;
            }
        }

        // 2) Sort ready ตาม priority (DESC) แล้วตาม earliest deadline (ASC)
        for (int i=0;i<nready;i++){
            for (int j=i+1;j<nready;j++){
                bool swap = false;
                if (ready[j].priority > ready[i].priority) swap = true;
                else if (ready[j].priority == ready[i].priority &&
                         ready[j].abs_deadline_us < ready[i].abs_deadline_us) swap = true;
                if (swap){ ready_item_t tmp = ready[i]; ready[i]=ready[j]; ready[j]=tmp; }
            }
        }

        // 3) Dispatch (limit budget/tick กันยาวไป)
        int dispatched = 0;
        for (int k=0;k<nready && dispatched<DISPATCH_BUDGET; k++){
            int id = ready[k].job_id;
            job_desc_t *jb = &g_jobs[id];

            worker_cmd_t cmd = {
                .job_id = id,
                .priority = jb->priority,
                .exec_ms = jb->wcet_ms,
                .abs_deadline_us = ready[k].abs_deadline_us,
                .release_us = ready[k].release_us
            };

            int w = select_worker();
            if (xQueueSend(q_worker[w], &cmd, 0) == pdPASS){
                dispatched++;
                // กระพริบ LED scheduler สั้น ๆ
                gpio_set_level(LED_SCHED, 1);
                esp_rom_delay_us(300);   // ← แทน ets_delay_us(300)
                gpio_set_level(LED_SCHED, 0);
                ESP_LOGD(TAG, "Dispatch job %s -> worker %d (prio=%d, ddl=%" PRId64 ")",
                         jb->name, w, jb->priority, cmd.abs_deadline_us);
            } else {
                ESP_LOGW(TAG, "Worker %d queue full, defer job %s", w, jb->name);
                // ระบบจริงอาจเก็บ carry-over queue
            }
        }

        // 4) เก็บ completion และตรวจ deadline
        completion_t comp;
        while (xQueueReceive(q_complete, &comp, 0) == pdTRUE) {
            job_desc_t *jb = &g_jobs[comp.job_id];
            jb->completions++;

            bool miss = (comp.finish_us > comp.abs_deadline_us);
            if (miss) {
                jb->deadline_miss++;
                // กระพริบ LED miss ให้เห็น
                blink_once(LED_MISS, 10);
                ESP_LOGW(TAG, "DEADLINE MISS job %s: finish=%" PRId64 " ddl=%" PRId64,
                         jb->name, comp.finish_us, comp.abs_deadline_us);
            }

            // response time (finish - release)
            uint32_t resp_ms = (uint32_t)((comp.finish_us - comp.release_us)/1000);
            jb->sum_response_ms += resp_ms;
            if (resp_ms > jb->max_response_ms) jb->max_response_ms = resp_ms;
        }

        vTaskDelay(pdMS_TO_TICKS(SCHED_TICK_MS));
    }
}

// ====== Monitor ======
static void monitor_task(void *arg)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "===== MONITOR =====");
        for (int i=0;i<NUM_JOBS;i++){
            job_desc_t *jb = &g_jobs[i];
            float util = (float)jb->wcet_ms / (float)jb->period_ms * 100.0f;
            float avg_resp = (jb->completions>0) ? (float)jb->sum_response_ms/(float)jb->completions : 0.0f;
            ESP_LOGI(TAG, "Job %s (P%d): period=%ums wcet=%ums ddl=%ums | rel=%u comp=%u miss=%u | util=%.1f%% resp(avg=%.1f max=%u) ms",
                     jb->name, jb->priority, jb->period_ms, jb->wcet_ms, jb->deadline_ms,
                     jb->releases, jb->completions, jb->deadline_miss, util, avg_resp, jb->max_response_ms);
        }
        for (int w=0; w<NUM_WORKERS; w++){
            ESP_LOGI(TAG, "Worker %d queue depth: %u", w, (unsigned)uxQueueMessagesWaiting(q_worker[w]));
        }
        ESP_LOGI(TAG, "====================");
    }
}

// ====== Optional: Load Generator (เพิ่มโหลด burst) ======
static void load_gen_task(void *arg)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(15000));
        ESP_LOGW(TAG, "LOAD: temporary increase wcet for job B/C");
        g_jobs[JOB_B].wcet_ms = 35;
        g_jobs[JOB_C].wcet_ms = 90;
        vTaskDelay(pdMS_TO_TICKS(6000));
        ESP_LOGW(TAG, "LOAD: restore wcet");
        g_jobs[JOB_B].wcet_ms = 20;
        g_jobs[JOB_C].wcet_ms = 60;
    }
}

// ====== app_main ======
void app_main(void)
{
    ESP_LOGI(TAG, "Real-time Scheduler demo starting...");

    // GPIO
    gpio_set_direction(LED_OK, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_MISS, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_SCHED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_OK, 0);
    gpio_set_level(LED_MISS, 0);
    gpio_set_level(LED_SCHED, 0);

    // Queues
    for (int i=0;i<NUM_WORKERS;i++){
        q_worker[i] = xQueueCreate(WORKER_QUEUE_LEN, sizeof(worker_cmd_t));
    }
    q_complete = xQueueCreate(COMPLETE_QUEUE_LEN, sizeof(completion_t));

    // Workers (pin core 0/1)
    xTaskCreatePinnedToCore(worker_task, "worker0", WORKER_STACK, (void*)0, 4, &h_worker[0], 0);
    xTaskCreatePinnedToCore(worker_task, "worker1", WORKER_STACK, (void*)1, 4, &h_worker[1], 1);

    // Scheduler (prio สูงกว่า worker เล็กน้อย)
    xTaskCreatePinnedToCore(scheduler_task, "scheduler", SCHED_STACK, NULL, 5, NULL, 0);

    // Monitor
    xTaskCreate(monitor_task, "monitor", MON_STACK, NULL, 3, NULL);

    // Optional load gen
    xTaskCreate(load_gen_task, "loadgen", LOAD_STACK, NULL, 2, NULL);

    ESP_LOGI(TAG, "Setup complete.");
}
