// main/lab3-first-task.c
// Lab 3: FreeRTOS First Tasks (Step 1 + Step 2 + Step 3 + Exercises)
// ESP-IDF v5.x

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

// ================== Pins (ปรับตามบอร์ด) ==================
#define LED1_PIN GPIO_NUM_2   // มักเป็น LED บนบอร์ด
#define LED2_PIN GPIO_NUM_4

static const char *TAG = "LAB3_TASKS";

// ================== Helper ==================
static const char* state_name(eTaskState s) {
    switch (s) {
        case eRunning:   return "Running";
        case eReady:     return "Ready";
        case eBlocked:   return "Blocked";
        case eSuspended: return "Suspended";
        case eDeleted:   return "Deleted";
        default:         return "Unknown";
    }
}

// ==========================================================
// Step 1: Basic Tasks
// ==========================================================

// LED1: กระพริบ 500ms / 500ms
void led1_task(void *pvParameters) {
    int *task_id = (int *)pvParameters;
    ESP_LOGI(TAG, "LED1 Task started (ID=%d, prio=%u)",
             *task_id, (unsigned)uxTaskPriorityGet(NULL));

    while (1) {
        ESP_LOGI(TAG, "LED1 ON");
        gpio_set_level(LED1_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        ESP_LOGI(TAG, "LED1 OFF");
        gpio_set_level(LED1_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// LED2: Fast blink 5 ครั้ง แล้วพัก 1s
void led2_task(void *pvParameters) {
    char *task_name = (char *)pvParameters;
    ESP_LOGI(TAG, "LED2 Task started (%s, prio=%u)",
             task_name, (unsigned)uxTaskPriorityGet(NULL));

    while (1) {
        ESP_LOGI(TAG, "LED2 Fast Blink x5");
        for (int i = 0; i < 5; i++) {
            gpio_set_level(LED2_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED2_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// แสดงข้อมูลระบบทุก 3s
void system_info_task(void *pvParameters) {
    (void)pvParameters;
    ESP_LOGI(TAG, "System Info Task started (prio=%u)", (unsigned)uxTaskPriorityGet(NULL));

    while (1) {
        ESP_LOGI(TAG, "=== System Information ===");
        ESP_LOGI(TAG, "Free heap: %u bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Min free heap: %u bytes", esp_get_minimum_free_heap_size());

        UBaseType_t task_count = uxTaskGetNumberOfTasks();
        ESP_LOGI(TAG, "Number of tasks: %u", (unsigned)task_count);

        TickType_t uptime = xTaskGetTickCount();
        uint32_t uptime_sec = (uptime * portTICK_PERIOD_MS) / 1000;
        ESP_LOGI(TAG, "Uptime: %u seconds", (unsigned)uptime_sec);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ==========================================================
// Step 2: Task Management (Suspend/Resume/Query state)
// ==========================================================

void task_manager(void *pvParameters) {
    ESP_LOGI(TAG, "Task Manager started (prio=%u)", (unsigned)uxTaskPriorityGet(NULL));

    // pvParameters = TaskHandle_t[2] -> [0]=LED1, [1]=LED2
    TaskHandle_t *handles = (TaskHandle_t *)pvParameters;
    TaskHandle_t led1_handle = handles[0];
    TaskHandle_t led2_handle = handles[1];

    int command_counter = 0;

    while (1) {
        command_counter++;
        switch (command_counter % 6) {
            case 1:
                ESP_LOGI(TAG, "Manager: Suspending LED1");
                vTaskSuspend(led1_handle);
                break;
            case 2:
                ESP_LOGI(TAG, "Manager: Resuming  LED1");
                vTaskResume(led1_handle);
                break;
            case 3:
                ESP_LOGI(TAG, "Manager: Suspending LED2");
                vTaskSuspend(led2_handle);
                break;
            case 4:
                ESP_LOGI(TAG, "Manager: Resuming  LED2");
                vTaskResume(led2_handle);
                break;
            case 5: {
                ESP_LOGI(TAG, "Manager: Query states");
                eTaskState s1 = eTaskGetState(led1_handle);
                eTaskState s2 = eTaskGetState(led2_handle);
                ESP_LOGI(TAG, "LED1 State: %s", state_name(s1));
                ESP_LOGI(TAG, "LED2 State: %s", state_name(s2));
                break;
            }
            case 0:
                ESP_LOGI(TAG, "Manager: Reset cycle");
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ==========================================================
// Step 3: Priorities & Runtime Statistics
// ==========================================================

// งาน priority สูง: busy loop แล้วพัก 5s
void high_priority_task(void *pvParameters) {
    (void) pvParameters;
    ESP_LOGW(TAG, "High Priority Task started (prio=%u)", (unsigned)uxTaskPriorityGet(NULL));

    while (1) {
        ESP_LOGW(TAG, "HIGH PRIORITY RUN...");
        for (volatile uint32_t i = 0; i < 1000000; i++) { /* busy */ }
        ESP_LOGW(TAG, "High priority yielding 5s");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// งาน priority ต่ำ: ทำ step 100 ครั้ง มี delay
void low_priority_task(void *pvParameters) {
    (void) pvParameters;
    ESP_LOGI(TAG, "Low Priority Task started (prio=%u)", (unsigned)uxTaskPriorityGet(NULL));

    while (1) {
        ESP_LOGI(TAG, "Low priority loop 100 steps");
        for (int i = 0; i < 100; i++) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// แสดง Runtime Stats + Task List ทุก 10s
void runtime_stats_task(void *pvParameters) {
    (void) pvParameters;
    ESP_LOGI(TAG, "Runtime Stats Task started (prio=%u)", (unsigned)uxTaskPriorityGet(NULL));

    // ใช้พอร์ตดีฟอลต์ของ ESP-IDF: ไม่ต้องกำหนด portCONFIGURE_TIMER_FOR_RUN_TIME_STATS / portGET_RUN_TIME_COUNTER_VALUE
    // เพียงเปิดคอนฟิกใน menuconfig ก็พอ

    char *buffer = (char*) malloc(4096);
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate buffer");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        // Runtime Statistics
        ESP_LOGI(TAG, "\n========== Runtime Statistics ==========");
        vTaskGetRunTimeStats(buffer); // ต้องเปิด generate runtime stats + formatting funcs
        ESP_LOGI(TAG, "Task\t\tAbs Time\tPercent Time\n%s", buffer);

        // Task List
        ESP_LOGI(TAG, "\n============== Task List ==============");
        vTaskList(buffer);
        ESP_LOGI(TAG, "Name\t\tState\tPrio\tStack\tNum\n%s", buffer);

        // Stack high-water mark ของตัวเอง
        UBaseType_t watermark_words = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "RunStats stack remaining: %u bytes",
                 (unsigned)(watermark_words * sizeof(StackType_t)));

        vTaskDelay(pdMS_TO_TICKS(10000));
    }

    free(buffer);
    vTaskDelete(NULL);
}

// ==========================================================
// Exercises
// ==========================================================

void temporary_task(void *pvParameters) {
    int *duration = (int *)pvParameters;
    ESP_LOGI(TAG, "Temporary task run %d s", *duration);

    for (int i = *duration; i > 0; i--) {
        ESP_LOGI(TAG, "Temp countdown: %d", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_LOGI(TAG, "Temporary task self-deleting");
    vTaskDelete(NULL);
}

// Preview communication (ยังไม่กัน race โดยเจตนา—จะเรียนต่อในบทถัดไป)
volatile int shared_counter = 0;
void producer_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        shared_counter++;
        ESP_LOGI(TAG, "Producer: counter=%d", shared_counter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void consumer_task(void *pvParameters) {
    (void)pvParameters;
    int last_value = 0;
    while (1) {
        if (shared_counter != last_value) {
            last_value = shared_counter;
            ESP_LOGI(TAG, "Consumer: got %d", last_value);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ==========================================================
// app_main
// ==========================================================

void app_main(void) {
    ESP_LOGI(TAG, "=== Lab 3: First Tasks (All-in-One, fixed) ===");

    // GPIO config
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED1_PIN) | (1ULL << LED2_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);

    // Parameters
    static int  led1_id = 1;
    static char led2_name[] = "FastBlinker";

    // Handles
    TaskHandle_t led1_handle = NULL;
    TaskHandle_t led2_handle = NULL;
    TaskHandle_t info_handle = NULL;

    // NOTE: ขนาดสแตกใน xTaskCreate คือ "words" (ESP32: 1 word = 4 bytes)
    // 2048 words ≈ 8 KB, 3072 words ≈ 12 KB, 4096 words ≈ 16 KB
    xTaskCreate(led1_task,        "LED1_Task",  2048, &led1_id,   2, &led1_handle);
    xTaskCreate(led2_task,        "LED2_Task",  2048,  led2_name, 2, &led2_handle);
    xTaskCreate(system_info_task, "SysInfo",    3072,  NULL,      1, &info_handle);

    // Step 2: Task Manager (prio สูงกว่าเพื่อควบคุม)
    static TaskHandle_t task_handles[2];
    task_handles[0] = led1_handle;
    task_handles[1] = led2_handle;
    xTaskCreate(task_manager, "TaskManager", 2048, task_handles, 3, NULL);

    // Step 3: Priorities & Runtime Stats
    xTaskCreate(high_priority_task, "HighPri",  3072, NULL, 4, NULL);
    xTaskCreate(low_priority_task,  "LowPri",   3072, NULL, 1, NULL);
    xTaskCreate(runtime_stats_task, "RunStats", 4096, NULL, 2, NULL);

    // Exercises
    static int temp_duration = 10;
    xTaskCreate(temporary_task, "TempTask", 2048, &temp_duration, 1, NULL);
    xTaskCreate(producer_task,  "Producer", 2048, NULL, 1, NULL);
    xTaskCreate(consumer_task,  "Consumer", 2048, NULL, 1, NULL);

    // main task: heartbeat
    while (1) {
        ESP_LOGI(TAG, "Main heartbeat");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
