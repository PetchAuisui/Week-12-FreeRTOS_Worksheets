// main.c - Dynamic Barrier Size Demo (ESP-IDF v5.x / FreeRTOS)
// แสดงการทำงานของ Barrier ที่สามารถปรับขนาด (จำนวนผู้เข้ารอ) ได้ขณะรันจริง

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "driver/gpio.h"

static const char *TAG     = "APP";
static const char *TAG_DB  = "DYN_BARRIER";

// ===== GPIO (ตัวเลือก) สำหรับแสดงสถานะผ่าน LED =====
#define LED_BARRIER_SYNC   GPIO_NUM_2   // จะกะพริบเมื่อกลุ่มผ่าน barrier พร้อมกัน

// ===== Dynamic Barrier (Two-Phase) ============================================
typedef struct {
    uint32_t size;            // เป้าหมายจำนวนผู้เข้ารอใน barrier "ต่อรอบ"
    uint32_t count;           // จำนวนที่มาถึงแล้วในรอบนี้
    uint32_t generation;      // หมายเลขรอบ (กันการปลดผิดรอบ)
    SemaphoreHandle_t mutex;  // ป้องกันการแก้ไข count/size/generation พร้อมกัน
    SemaphoreHandle_t turnstile;   // เปิดทางให้ทุกคนผ่านพร้อมกัน (phase 1 -> 2)
    SemaphoreHandle_t turnstile2;  // รีเซ็ตรอบ (phase 2 -> พร้อมเริ่มรอบใหม่)
} dynamic_barrier_t;

static void barrier_init(dynamic_barrier_t *b, uint32_t initial_size) {
    configASSERT(initial_size >= 1);
    memset(b, 0, sizeof(*b));
    b->size = initial_size;
    b->count = 0;
    b->generation = 0;
    b->mutex = xSemaphoreCreateMutex();
    b->turnstile  = xSemaphoreCreateCounting(initial_size, 0);
    b->turnstile2 = xSemaphoreCreateCounting(initial_size, 0);
    configASSERT(b->mutex && b->turnstile && b->turnstile2);
}

// รอที่ barrier (คืน true = ผ่าน, false = timeout ใดๆ ในเฟส)
// timeout_ms ถูกใช้กับการรอทั้ง turnstile และ turnstile2
static bool barrier_wait(dynamic_barrier_t *b, uint32_t timeout_ms, uint32_t *wait_ms_out) {
    uint64_t t0 = esp_timer_get_time();

    // ===== Phase 1: arrival =====
    xSemaphoreTake(b->mutex, portMAX_DELAY);
    uint32_t my_gen = b->generation;
    (void)my_gen; // สำหรับ debug เพิ่มเติมได้ภายหลัง
    b->count++;
    bool i_am_last = (b->count == b->size);
    xSemaphoreGive(b->mutex);

    if (i_am_last) {
        // คนสุดท้ายปลดล็อค turnstile ให้ครบ size คน
        for (uint32_t i = 0; i < b->size; i++) {
            xSemaphoreGive(b->turnstile);
        }
    }

    if (xSemaphoreTake(b->turnstile, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        return false; // timeout เฟสแรก
    }

    // ===== Phase 2: departure / reset =====
    xSemaphoreTake(b->mutex, portMAX_DELAY);
    b->count--;
    bool i_am_last_to_leave = (b->count == 0);
    if (i_am_last_to_leave) {
        // คนสุดท้ายของรอบ: ปิดรอบเก่า, เปิด turnstile2 ให้ทุกคนผ่าน
        b->generation++;
        for (uint32_t i = 0; i < b->size; i++) {
            xSemaphoreGive(b->turnstile2);
        }
    }
    xSemaphoreGive(b->mutex);

    if (xSemaphoreTake(b->turnstile2, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        return false; // timeout เฟสสอง
    }

    if (wait_ms_out) {
        *wait_ms_out = (uint32_t)((esp_timer_get_time() - t0) / 1000);
    }
    return true;
}

// ขอปรับขนาด barrier แบบปลอดภัย: จะยอมเปลี่ยนได้เมื่อ "ไม่มีใครอยู่ในรอบ"
// ถ้ายังมีคนค้างอยู่ให้รอ (ภายใน wait_ms) โดยเช็คว่าจำนวน count กลับมา 0
static bool barrier_resize(dynamic_barrier_t *b, uint32_t new_size, uint32_t wait_ms) {
    if (new_size < 1) new_size = 1;

    uint64_t t0 = esp_timer_get_time();
    while (1) {
        xSemaphoreTake(b->mutex, portMAX_DELAY);
        bool can_change = (b->count == 0); // ปลอดภัยเมื่อไม่มีใครอยู่ใน barrier
        if (can_change) {
            b->size = new_size;

            // ปรับ turnstile/turnstile2 ให้สอดคล้องกับขนาดใหม่ (รีสร้างง่ายสุด)
            vSemaphoreDelete(b->turnstile);
            vSemaphoreDelete(b->turnstile2);
            b->turnstile  = xSemaphoreCreateCounting(new_size, 0);
            b->turnstile2 = xSemaphoreCreateCounting(new_size, 0);
            xSemaphoreGive(b->mutex);

            return true;
        }
        xSemaphoreGive(b->mutex);

        // ยังมีคนอยู่ใน barrier → รอแล้วลองใหม่ จนหมดเวลาที่กำหนด
        if (((esp_timer_get_time() - t0) / 1000) > wait_ms) {
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ===== Demo: Dynamic Barrier Workers & Controller =============================
static dynamic_barrier_t g_barrier;        // ตัวแปร barrier กลาง
static volatile uint32_t g_barrier_round;  // หมายเลขรอบ (เพื่อ logging สวย ๆ)

static inline void led_pulse(gpio_num_t pin, uint32_t ms) {
    gpio_set_level(pin, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_set_level(pin, 0);
}

static void dyn_barrier_worker_task(void *pvParameters) {
    uint32_t id = (uint32_t)pvParameters;
    ESP_LOGI(TAG_DB, "Worker %lu started", id);

    while (1) {
        // งานก่อนเข้า barrier (สุ่ม 0.8–2.5s)
        uint32_t work_ms = 800 + (esp_random() % 1700);
        vTaskDelay(pdMS_TO_TICKS(work_ms));

        uint32_t waited_ms = 0;
        bool ok = barrier_wait(&g_barrier, /*timeout_ms*/ 8000, &waited_ms);
        if (ok) {
            uint32_t round = g_barrier_round;
            ESP_LOGI(TAG_DB, "Worker %lu passed barrier (round=%lu, waited=%lums, size=%lu)",
                     id, round, waited_ms, g_barrier.size);

            // ให้ worker 0 กระพริบ LED เพื่อแสดงว่ารอบนี้ผ่านพร้อมกันแล้ว
            if (id == 0) {
                led_pulse(LED_BARRIER_SYNC, 120);
            }

            // งานสั้น ๆ หลัง barrier (ให้เห็นการ sync แล้วแยกย้าย)
            vTaskDelay(pdMS_TO_TICKS(200 + (esp_random() % 200)));
        } else {
            ESP_LOGW(TAG_DB, "Worker %lu barrier TIMEOUT (size=%lu)", id, g_barrier.size);
            // fallback: รอพัก แล้ววนไปใหม่
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

// สาธิตการปรับขนาดระหว่างรัน: 4 -> 3 -> 5 -> 2 -> 4 … วนลูป
static void dyn_barrier_controller_task(void *pvParameters) {
    (void)pvParameters;
    uint32_t pattern[] = {4, 3, 5, 2, 4};
    size_t idx = 0;

    while (1) {
        // ปล่อยให้ทำงานไปสักหลายรอบแล้วค่อยลองปรับ
        vTaskDelay(pdMS_TO_TICKS(15000));

        uint32_t target = pattern[idx];
        ESP_LOGI(TAG_DB, "Request resize barrier to %lu", target);

        bool changed = barrier_resize(&g_barrier, target, /*wait_ms*/ 5000);
        if (changed) {
            g_barrier_round++; // เปิดรอบใหม่ (เพื่อ grouping logs)
            ESP_LOGI(TAG_DB, "Barrier resized to %lu (round=%lu)", target, g_barrier_round);
        } else {
            ESP_LOGW(TAG_DB, "Resize failed (busy). Will retry later.");
        }

        idx = (idx + 1) % (sizeof(pattern)/sizeof(pattern[0]));
    }
}

// ===== App Main ===============================================================
static void gpio_init_outputs(void) {
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << LED_BARRIER_SYNC),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);

    // ปิด LED เริ่มต้น
    gpio_set_level(LED_BARRIER_SYNC, 0);
}

void app_main(void) {
    ESP_LOGI(TAG, "🚀 Dynamic Barrier Size Demo Starting...");

    // ตั้งค่า GPIO สำหรับ LED แสดงผล
    gpio_init_outputs();

    // เริ่มต้น Dynamic Barrier (กำหนดค่าเริ่มต้นเป็น 4 คน)
    barrier_init(&g_barrier, 4);
    g_barrier_round = 1;

    // สร้าง worker จำนวน "สูงสุด" ที่อยากมีในระบบ (เช่น 6 ตัว)
    // แล้วใช้การ "ปรับขนาด barrier" เป็นตัวกำหนดว่าต้องการให้ผ่านพร้อมกันกี่คนในแต่ละรอบ
    const uint32_t NUM_WORKERS = 6;
    for (uint32_t i = 0; i < NUM_WORKERS; i++) {
        char name[16];
        snprintf(name, sizeof(name), "DynWork%lu", i);
        xTaskCreate(dyn_barrier_worker_task, name, 2048, (void*)i, 5, NULL);
    }

    // ตัวควบคุมที่คอยปรับขนาด barrier runtime
    xTaskCreate(dyn_barrier_controller_task, "DynBarrierCtrl", 3072, NULL, 6, NULL);

    ESP_LOGI(TAG, "Dynamic Barrier demo started with initial size=4, workers=%" PRIu32, NUM_WORKERS);
    ESP_LOGI(TAG, "LED indicators: GPIO%d pulses when a barrier round completes.", LED_BARRIER_SYNC);
}
