#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "driver/gpio.h"

static const char *TAG = "COMPLEX_EVENTS";

// GPIO สำหรับ Smart Home System
#define LED_LIVING_ROOM GPIO_NUM_2
#define LED_KITCHEN GPIO_NUM_4
#define LED_BEDROOM GPIO_NUM_5
#define LED_SECURITY GPIO_NUM_18
#define LED_EMERGENCY GPIO_NUM_19

// ───────────────────────────────
// 🔹 Enum & Struct Definitions
// ───────────────────────────────
typedef enum
{
    HOME_STATE_IDLE = 0,
    HOME_STATE_OCCUPIED,
    HOME_STATE_AWAY,
    HOME_STATE_SLEEP,
    HOME_STATE_SECURITY_ARMED,
    HOME_STATE_EMERGENCY,
    HOME_STATE_MAINTENANCE
} home_state_t;

// Event Group Handles
EventGroupHandle_t sensor_events;
EventGroupHandle_t system_events;
EventGroupHandle_t pattern_events;

// Event Bits
#define MOTION_DETECTED_BIT (1 << 0)
#define DOOR_OPENED_BIT (1 << 1)
#define DOOR_CLOSED_BIT (1 << 2)
#define LIGHT_ON_BIT (1 << 3)
#define LIGHT_OFF_BIT (1 << 4)
#define TEMPERATURE_HIGH_BIT (1 << 5)
#define TEMPERATURE_LOW_BIT (1 << 6)
#define SOUND_DETECTED_BIT (1 << 7)
#define PRESENCE_CONFIRMED_BIT (1 << 8)

#define SYSTEM_INIT_BIT (1 << 9)
#define USER_HOME_BIT (1 << 10)
#define USER_AWAY_BIT (1 << 11)
#define SLEEP_MODE_BIT (1 << 12)
#define SECURITY_ARMED_BIT (1 << 13)
#define EMERGENCY_MODE_BIT (1 << 14)
#define MAINTENANCE_MODE_BIT (1 << 15)

// ───────────────────────────────
// 🔹 Global State Variables
// ───────────────────────────────
static home_state_t current_home_state = HOME_STATE_IDLE;
static SemaphoreHandle_t state_mutex;

// ───────────────────────────────
// 🔹 Event History Buffer
// ───────────────────────────────
#define EVENT_HISTORY_SIZE 20
typedef struct
{
    EventBits_t event_bits;
    uint64_t timestamp;
    home_state_t state_at_time;
} event_record_t;

static event_record_t event_history[EVENT_HISTORY_SIZE];
static int history_index = 0;

// ───────────────────────────────
// 🔹 Helper Functions
// ───────────────────────────────
static const char *get_state_name(home_state_t state)
{
    switch (state)
    {
    case HOME_STATE_IDLE:
        return "Idle";
    case HOME_STATE_OCCUPIED:
        return "Occupied";
    case HOME_STATE_AWAY:
        return "Away";
    case HOME_STATE_SLEEP:
        return "Sleep";
    case HOME_STATE_SECURITY_ARMED:
        return "Secured";
    case HOME_STATE_EMERGENCY:
        return "Emergency";
    case HOME_STATE_MAINTENANCE:
        return "Maintenance";
    default:
        return "Unknown";
    }
}

static void change_home_state(home_state_t new_state)
{
    if (xSemaphoreTake(state_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        home_state_t old_state = current_home_state;
        current_home_state = new_state;
        ESP_LOGI(TAG, "🏠 State changed: %s → %s",
                 get_state_name(old_state), get_state_name(new_state));
        xSemaphoreGive(state_mutex);
    }
}

// ───────────────────────────────
// 🔹 Event History Functions
// ───────────────────────────────
static void add_event_to_history(EventBits_t event_bits)
{
    event_history[history_index].event_bits = event_bits;
    event_history[history_index].timestamp = esp_timer_get_time();
    event_history[history_index].state_at_time = current_home_state;
    history_index = (history_index + 1) % EVENT_HISTORY_SIZE;
}

// 🕒 Visualization: Show last 10 recent events
void print_event_sequence(void)
{
    ESP_LOGI(TAG, "\n🕒 Recent Event Sequence:");
    uint64_t current_time = esp_timer_get_time();

    for (int h = 0; h < 10; h++)
    { // Show last 10 events
        int hist_idx = (history_index - 1 - h + EVENT_HISTORY_SIZE) % EVENT_HISTORY_SIZE;
        event_record_t *record = &event_history[hist_idx];

        if (record->timestamp > 0)
        {
            uint32_t age_ms = (current_time - record->timestamp) / 1000;
            ESP_LOGI(TAG, "  [-%4lu ms] State: %-12s | Events: 0x%08X",
                     age_ms, get_state_name(record->state_at_time),
                     record->event_bits);
        }
    }
    ESP_LOGI(TAG, "──────────────────────────────");
}

// ───────────────────────────────
// 🔹 Example Pattern (Simplified)
// ───────────────────────────────
static void mock_pattern_task(void *pv)
{
    while (1)
    {
        EventBits_t event = 0;

        // Random event generator (simulation)
        switch (esp_random() % 4)
        {
        case 0:
            event = MOTION_DETECTED_BIT;
            break;
        case 1:
            event = DOOR_OPENED_BIT;
            break;
        case 2:
            event = LIGHT_ON_BIT;
            break;
        case 3:
            event = SOUND_DETECTED_BIT;
            break;
        }

        ESP_LOGI(TAG, "⚡ Event triggered: 0x%08X", event);
        xEventGroupSetBits(sensor_events, event);
        add_event_to_history(event);

        vTaskDelay(pdMS_TO_TICKS(2000 + (esp_random() % 3000)));
    }
}

// ───────────────────────────────
// 🔹 Status Monitor Task
// ───────────────────────────────
static void status_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "📊 Status monitor started");
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));

        ESP_LOGI(TAG, "\n🏠 ───── HOME STATUS ─────");
        ESP_LOGI(TAG, "Current State: %s", get_state_name(current_home_state));
        ESP_LOGI(TAG, "Sensor Events: 0x%08X", xEventGroupGetBits(sensor_events));
        ESP_LOGI(TAG, "System Events: 0x%08X", xEventGroupGetBits(system_events));
        ESP_LOGI(TAG, "Pattern Events:0x%08X", xEventGroupGetBits(pattern_events));

        // 🔍 แสดงลำดับ Event ล่าสุด 10 รายการ
        print_event_sequence();

        ESP_LOGI(TAG, "Free Heap: %d bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "──────────────────────────────\n");
    }
}

// ───────────────────────────────
// 🔹 app_main
// ───────────────────────────────
void app_main(void)
{
    ESP_LOGI(TAG, "🚀 Event Sequence Visualization Demo Starting...");

    // GPIO setup (optional for LED demo)
    gpio_set_direction(LED_LIVING_ROOM, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_LIVING_ROOM, 0);

    // Mutex & Event Groups
    state_mutex = xSemaphoreCreateMutex();
    sensor_events = xEventGroupCreate();
    system_events = xEventGroupCreate();
    pattern_events = xEventGroupCreate();

    if (!state_mutex || !sensor_events)
    {
        ESP_LOGE(TAG, "❌ Failed to init synchronization primitives!");
        return;
    }

    change_home_state(HOME_STATE_IDLE);

    // Create tasks
    xTaskCreate(mock_pattern_task, "MockPattern", 2048, NULL, 5, NULL);
    xTaskCreate(status_monitor_task, "StatusMonitor", 3072, NULL, 4, NULL);

    ESP_LOGI(TAG, "✅ Tasks created successfully!");
}
