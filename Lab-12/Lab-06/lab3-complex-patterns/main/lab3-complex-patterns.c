// lab3-voice.c — Voice via Serial Commands (simulate speech recognition)
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"

static const char *TAG = "VOICE_SIM";

// LEDs
#define LED_LIVING_ROOM GPIO_NUM_2
#define LED_KITCHEN GPIO_NUM_4
#define LED_BEDROOM GPIO_NUM_5

static void set_leds(bool lr, bool kit, bool bed)
{
    gpio_set_level(LED_LIVING_ROOM, lr);
    gpio_set_level(LED_KITCHEN, kit);
    gpio_set_level(LED_BEDROOM, bed);
}

static void trim_lower(char *s)
{
    int n = (int)strlen(s);
    while (n > 0 && (s[n - 1] == '\r' || s[n - 1] == '\n' || isspace((unsigned char)s[n - 1])))
        s[--n] = 0;
    while (*s && isspace((unsigned char)*s))
        memmove(s, s + 1, strlen(s));
    for (char *p = s; *p; ++p)
        *p = (char)tolower((unsigned char)*p);
}

static void voice_uart_task(void *pv)
{
    ESP_LOGI(TAG, "Type: on | off | kitchen | bedroom | goodnight | wake");
    char line[64];
    while (1)
    {
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        trim_lower(line);

        if (!strcmp(line, "on"))
        {
            gpio_set_level(LED_LIVING_ROOM, 1);
            ESP_LOGI(TAG, "💡 Living room ON");
        }
        else if (!strcmp(line, "off"))
        {
            gpio_set_level(LED_LIVING_ROOM, 0);
            ESP_LOGI(TAG, "💡 Living room OFF");
        }
        else if (!strcmp(line, "kitchen"))
        {
            gpio_set_level(LED_KITCHEN, 1);
            ESP_LOGI(TAG, "🍳 Kitchen light ON");
        }
        else if (!strcmp(line, "bedroom"))
        {
            gpio_set_level(LED_BEDROOM, 1);
            ESP_LOGI(TAG, "🛏 Bedroom light ON");
        }
        else if (!strcmp(line, "goodnight"))
        {
            set_leds(0, 0, 1);
            ESP_LOGI(TAG, "🌙 Goodnight routine");
        }
        else if (!strcmp(line, "wake"))
        {
            set_leds(0, 1, 1);
            ESP_LOGI(TAG, "☀️ Wake-up routine");
        }
        else if (strlen(line) > 0)
        {
            ESP_LOGW(TAG, "Unknown: %s", line);
        }
    }
}

void app_main(void)
{
    // Map stdin/stdout to UART0 using driver
    const int uart_num = 0;
    uart_driver_install(uart_num, 256, 0, 0, NULL, 0);
    esp_vfs_dev_uart_use_driver(uart_num);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    gpio_set_direction(LED_LIVING_ROOM, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_KITCHEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BEDROOM, GPIO_MODE_OUTPUT);
    set_leds(0, 0, 0);

    xTaskCreate(voice_uart_task, "voice_uart", 4096, NULL, 4, NULL);
    ESP_LOGI(TAG, "Voice (UART) ready. Open monitor and type commands.");
}
