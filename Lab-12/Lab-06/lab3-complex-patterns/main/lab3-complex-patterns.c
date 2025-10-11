// lab3-ml.c — TinyML-style logistic scoring on ESP32 (no external libs)
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "driver/gpio.h"

static const char *TAG = "ML_DEMO";

// LEDs
#define LED_LIVING_ROOM GPIO_NUM_2
#define LED_KITCHEN GPIO_NUM_4
#define LED_BEDROOM GPIO_NUM_5

// Events (จำลอง)
typedef struct
{
    uint32_t temp_c;
    uint32_t light_level; // 0-100
    uint8_t motion;       // 0/1
} features_t;

static inline float sigmoidf_fast(float x)
{
    if (x > 8.0f)
        return 0.9997f;
    if (x < -8.0f)
        return 0.0003f;
    return 1.0f / (1.0f + expf(-x));
}

// “โมเดล” แบบ logistic regression (เทรนไว้ล่วงหน้าเชิงสมมติ)
typedef struct
{
    float w_temp;   // น้ำหนักอุณหภูมิ (normalize ~ 0..40C)
    float w_light;  // น้ำหนักแสง      (0..100%)
    float w_motion; // น้ำหนัก motion   (0/1)
    float bias;     // ค่าคงที่
} lr_model_t;

static lr_model_t model_wakeup = {
    .w_temp = -0.05f,  // อุ่นไป → ไม่ค่อยตื่น
    .w_light = 0.03f,  // สว่างขึ้น → มีแนวโน้มตื่น
    .w_motion = 1.20f, // มี motion → ใกล้ตื่น
    .bias = -1.2f      // ฐานโน้มว่าจะยังไม่ตื่น
};

static lr_model_t model_return_home = {
    .w_temp = 0.00f,
    .w_light = 0.01f,
    .w_motion = 0.90f, // มี motion ที่ประตู/ห้องนั่งเล่น
    .bias = -1.0f};

static float predict_prob(const lr_model_t *m, const features_t *f)
{
    float x = m->w_temp * (float)f->temp_c + m->w_light * (float)f->light_level + m->w_motion * (float)f->motion + m->bias;
    return sigmoidf_fast(x);
}

static void set_leds(bool lr, bool kit, bool bed)
{
    gpio_set_level(LED_LIVING_ROOM, lr);
    gpio_set_level(LED_KITCHEN, kit);
    gpio_set_level(LED_BEDROOM, bed);
}

static void sensor_sim_task(void *pv)
{
    ESP_LOGI(TAG, "Sensor simulator started");
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

static void ml_inference_task(void *pv)
{
    ESP_LOGI(TAG, "ML inference started");
    uint32_t step = 0;
    while (1)
    {
        // สร้างฟีเจอร์จำลอง
        features_t f = {
            .temp_c = 20 + (esp_random() % 15), // 20-34C
            .light_level = esp_random() % 100,  // 0-99%
            .motion = (esp_random() % 100) < 20 // 20% chance
        };

        float p_wakeup = predict_prob(&model_wakeup, &f);
        float p_return = predict_prob(&model_return_home, &f);

        // smoothing เล็กน้อย
        static float ema_w = 0.0f, ema_r = 0.0f;
        ema_w = 0.6f * ema_w + 0.4f * p_wakeup;
        ema_r = 0.6f * ema_r + 0.4f * p_return;

        ESP_LOGI(TAG,
                 "F(temp=%uC,light=%u%%,motion=%u) => WakeUp=%.2f (ema=%.2f), Return=%.2f (ema=%.2f)",
                 f.temp_c, f.light_level, f.motion, p_wakeup, ema_w, p_return, ema_r);

        // นโยบายควบคุมอุปกรณ์จากผลทำนาย
        if (ema_w > 0.75f)
        {
            ESP_LOGI(TAG, "☀️ Predicted WAKE-UP → เปิดไฟห้องนอนและครัว");
            set_leds(false, true, true);
        }
        else if (ema_r > 0.65f)
        {
            ESP_LOGI(TAG, "🚪 Predicted RETURN HOME → เปิดไฟห้องนั่งเล่น");
            set_leds(true, false, false);
        }
        else
        {
            set_leds(false, false, false);
        }

        if (++step % 6 == 0)
        {
            ESP_LOGI(TAG, "— periodic idle —");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    gpio_set_direction(LED_LIVING_ROOM, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_KITCHEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BEDROOM, GPIO_MODE_OUTPUT);
    set_leds(false, false, false);

    xTaskCreate(sensor_sim_task, "sensor_sim", 2048, NULL, 4, NULL);
    xTaskCreate(ml_inference_task, "ml_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "ML demo running");
}