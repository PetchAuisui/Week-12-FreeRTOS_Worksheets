// main.c — Priority Events + Event Logging + Dynamic Events + Correlation (ESP-IDF + FreeRTOS)

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"

// ==== Modules ====
#include "priority_events.h"
#include "event_log.h"
#include "dynamic_events.h"
#include "event_corr.h"

static const char *TAG = "MAIN_EVENTS";

// ==== GPIO LEDs ====
#define LED_NETWORK_READY   GPIO_NUM_2
#define LED_SENSOR_READY    GPIO_NUM_4
#define LED_CONFIG_READY    GPIO_NUM_5
#define LED_STORAGE_READY   GPIO_NUM_18
#define LED_SYSTEM_READY    GPIO_NUM_19

// ==== EventGroup & Bits ====
EventGroupHandle_t system_events;
#define NETWORK_READY_BIT   (1 << 0)
#define SENSOR_READY_BIT    (1 << 1)
#define CONFIG_READY_BIT    (1 << 2)
#define STORAGE_READY_BIT   (1 << 3)
#define SYSTEM_READY_BIT    (1 << 4)
#define ALL_SUBSYSTEM_BITS  (NETWORK_READY_BIT|SENSOR_READY_BIT|CONFIG_READY_BIT|STORAGE_READY_BIT)

// ==== Dynamic Event ====
static EventBits_t EVT_TEMP_ALERT = 0;

// ---------------------------------------------------------------------
// 📊 Event-Log Dumper
// ---------------------------------------------------------------------
static void print_record(const ev_record_t *r){
    ESP_LOGI("EVLOG",
        "[%8u ms] src=%-12s set=0x%04X before=0x%04X after=0x%04X",
        r->ts_ms, r->source ? r->source : "?", r->set_bits, r->before_bits, r->after_bits);
}

static void evlog_dumper_task(void *pv){
    ev_record_t buf[16];
    while(1){
        size_t n = evlog_dump(buf,16);
        if(n){
            ESP_LOGI("EVLOG","----- DUMP (last %u records) -----",(unsigned)n);
            for(size_t i=0;i<n;i++) print_record(&buf[i]);
        }else ESP_LOGI("EVLOG","----- DUMP (no records) -----");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// ---------------------------------------------------------------------
// 🌐 Subsystem Simulations
// ---------------------------------------------------------------------
static void network_init_task(void *pv){
    ESP_LOGI(TAG,"🌐 Network init start");
    vTaskDelay(pdMS_TO_TICKS(1200));
    evprio_post(NETWORK_READY_BIT,EV_PRIO_HIGH,"network");
    while(1){ vTaskDelay(pdMS_TO_TICKS(6000));
              evprio_post(NETWORK_READY_BIT,EV_PRIO_MED,"network-hb"); }
}

static void sensor_init_task(void *pv){
    ESP_LOGI(TAG,"🌡️ Sensor init start");
    vTaskDelay(pdMS_TO_TICKS(2000));
    evprio_post(SENSOR_READY_BIT,EV_PRIO_MED,"sensor");
    while(1) vTaskDelay(pdMS_TO_TICKS(8000));
}

static void config_load_task(void *pv){
    ESP_LOGI(TAG,"⚙️ Config load start");
    vTaskDelay(pdMS_TO_TICKS(800));
    evprio_post(CONFIG_READY_BIT,EV_PRIO_LOW,"config");
    while(1) vTaskDelay(pdMS_TO_TICKS(7000));
}

static void storage_init_task(void *pv){
    ESP_LOGI(TAG,"💾 Storage init start");
    vTaskDelay(pdMS_TO_TICKS(1500));
    evprio_post(STORAGE_READY_BIT,EV_PRIO_MED,"storage");
    while(1) vTaskDelay(pdMS_TO_TICKS(9000));
}

// ---------------------------------------------------------------------
// 🔥 Dynamic TEMP_ALERT demo
// ---------------------------------------------------------------------
static void temp_alert_task(void *pv){
    if(EVT_TEMP_ALERT==0){ ESP_LOGW("DYN","TEMP_ALERT bit not allocated"); vTaskDelete(NULL); }
    while(1){
        vTaskDelay(pdMS_TO_TICKS(7000));
        evprio_post(EVT_TEMP_ALERT,EV_PRIO_HIGH,"temp-alert");
        ESP_LOGI("DYN","TEMP_ALERT posted (bit=0x%X name=%s)",
                 (unsigned)EVT_TEMP_ALERT,dyn_name(EVT_TEMP_ALERT));
    }
}

// ---------------------------------------------------------------------
// 🎛️ Coordinator
// ---------------------------------------------------------------------
static void system_coordinator_task(void *pv){
    ESP_LOGI(TAG,"🎛️ Coordinator waiting for ALL subsystems...");
    EventBits_t bits = xEventGroupWaitBits(system_events,ALL_SUBSYSTEM_BITS,
                                           pdFALSE,pdTRUE,pdMS_TO_TICKS(15000));
    if((bits&ALL_SUBSYSTEM_BITS)==ALL_SUBSYSTEM_BITS){
        ESP_LOGI(TAG,"🎉 All subsystems ready -> set SYSTEM_READY");
        evprio_post(SYSTEM_READY_BIT,EV_PRIO_CRIT,"coordinator");
        gpio_set_level(LED_SYSTEM_READY,1);
    }else ESP_LOGW(TAG,"⚠️ Timeout waiting subsystems (bits=0x%X)",(unsigned)bits);

    while(1){
        EventBits_t cur = xEventGroupGetBits(system_events);
        ESP_LOGI(TAG,"Status bits=0x%08X [N:%d S:%d C:%d D:%d SYS:%d]",
                 (unsigned)cur,
                 !!(cur&NETWORK_READY_BIT),
                 !!(cur&SENSOR_READY_BIT),
                 !!(cur&CONFIG_READY_BIT),
                 !!(cur&STORAGE_READY_BIT),
                 !!(cur&SYSTEM_READY_BIT));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ---------------------------------------------------------------------
// 📈 Correlation Dumper
// ---------------------------------------------------------------------
static void corr_dumper_task(void *pv){
    while(1){
        int B = evcorr_bit_count();
        if(B<=0){ vTaskDelay(pdMS_TO_TICKS(15000)); continue; }

        size_t n = (size_t)B*(size_t)B;
        uint16_t *M = calloc(n,sizeof(uint16_t));
        if(!M){ vTaskDelay(pdMS_TO_TICKS(15000)); continue; }

        evcorr_dump(M);

        struct {int i,j;uint16_t v;} top[5]={{0}};
        for(int i=0;i<B;i++) for(int j=0;j<B;j++){
            if(i==j) continue;
            uint16_t v=M[i*B+j];
            if(!v) continue;
            for(int k=0;k<5;k++)
                if(v>top[k].v){
                    for(int m=4;m>k;m--) top[m]=top[m-1];
                    top[k]=(typeof(top[0])){.i=i,.j=j,.v=v};
                    break;
                }
        }
        ESP_LOGI("CORR","===== Top co-occurrence pairs =====");
        for(int k=0;k<5;k++)
            if(top[k].v)
                ESP_LOGI("CORR","bits[%d] with bits[%d] -> %u times",
                         top[k].i,top[k].j,top[k].v);

        free(M);
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}

// ---------------------------------------------------------------------
// 🚀 app_main
// ---------------------------------------------------------------------
void app_main(void){
    ESP_LOGI(TAG,"🚀 Priority + Logger + Dynamic + Correlation Demo Starting...");

    // GPIO
    gpio_reset_pin(LED_NETWORK_READY);
    gpio_reset_pin(LED_SENSOR_READY);
    gpio_reset_pin(LED_CONFIG_READY);
    gpio_reset_pin(LED_STORAGE_READY);
    gpio_reset_pin(LED_SYSTEM_READY);
    gpio_set_direction(LED_NETWORK_READY,GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_SENSOR_READY,GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CONFIG_READY,GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_STORAGE_READY,GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_SYSTEM_READY,GPIO_MODE_OUTPUT);

    // EventGroup
    system_events = xEventGroupCreate();
    if(!system_events){ ESP_LOGE(TAG,"EventGroup create failed"); return; }

    // Logger
    evlog_init(128);
    xTaskCreate(evlog_dumper_task,"EvLogDump",3072,NULL,3,NULL);

    // Priority Dispatcher
    evprio_init(system_events,16);
    xTaskCreate(evprio_dispatcher_task,"EvPrio",2048,NULL,9,NULL);

    // Dynamic Events
    EventBits_t reserved = NETWORK_READY_BIT|SENSOR_READY_BIT|CONFIG_READY_BIT|
                           STORAGE_READY_BIT|SYSTEM_READY_BIT;
    dyn_init(reserved);
    EVT_TEMP_ALERT = dyn_acquire("TEMP_ALERT");
    if(EVT_TEMP_ALERT)
        xTaskCreate(temp_alert_task,"TempAlert",2048,NULL,5,NULL);

    // Correlation Analyzer
    evcorr_init(2000,24);
    xTaskCreate(corr_dumper_task,"EvCorrDump",3072,NULL,3,NULL);

    // Subsystems
    xTaskCreate(network_init_task,"NetworkInit",3072,NULL,5,NULL);
    xTaskCreate(sensor_init_task,"SensorInit",3072,NULL,5,NULL);
    xTaskCreate(config_load_task,"ConfigLoad",3072,NULL,4,NULL);
    xTaskCreate(storage_init_task,"StorageInit",3072,NULL,4,NULL);
    xTaskCreate(system_coordinator_task,"SysCoord",4096,NULL,6,NULL);

    ESP_LOGI(TAG,"✅ Setup complete: Dispatcher + Logger + Dynamic + Correlation ready");
}