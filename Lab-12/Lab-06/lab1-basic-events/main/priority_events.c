// priority_events.c
#include "priority_events.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "event_log.h"           // ← เพิ่ม
#include "event_corr.h"



static const char* PLOG = "EVPRIO";
static EventGroupHandle_t g_group = NULL;
static QueueHandle_t g_q = NULL;

bool evprio_init(EventGroupHandle_t group, UBaseType_t queue_len) {
    g_group = group;
    g_q = xQueueCreate(queue_len, sizeof(ev_msg_t));
    return (g_group && g_q);
}

bool evprio_post(EventBits_t bits, ev_priority_t prio, const char* source){
    if (!g_q) return false;
    ev_msg_t m = {
        .bits=bits,
        .prio=prio,
        .ts_ms=(xTaskGetTickCount()*portTICK_PERIOD_MS),
        .source=source
    };
    return xQueueSend(g_q, &m, 0)==pdTRUE;
}

// ดึงเป็นชุดเล็ก ๆ แล้ว sort ตาม prio (สูงก่อน)
#define MAX_BATCH 8
static int cmp(const void* a, const void* b){
    const ev_msg_t *A=a, *B=b;
    return (int)B->prio - (int)A->prio; // CRIT>HIGH>MED>LOW
}

void evprio_dispatcher_task(void *unused) {
    ev_msg_t batch[MAX_BATCH];
    while (1) {
        UBaseType_t n=0;
        // อย่างน้อย 1 รายการ (block)
        if (xQueueReceive(g_q, &batch[n], portMAX_DELAY) == pdTRUE) n=1;
        // เก็บเพิ่มแบบ non-block เพื่อ batch
        ev_msg_t tmp;
        while (n<MAX_BATCH && xQueueReceive(g_q, &tmp, 0)==pdTRUE) batch[n++]=tmp;

        qsort(batch, n, sizeof(ev_msg_t), cmp);

        for (UBaseType_t i=0;i<n;i++){
            // เดิม: xEventGroupSetBits(g_group, batch[i].bits);
            evlog_add(g_group, batch[i].bits, batch[i].source);   // ← บันทึกก่อน/หลังทุกครั้ง
            evcorr_on_set(g_group, batch[i].bits);
            xEventGroupSetBits(g_group, batch[i].bits);
            ESP_LOGI(PLOG, "[%s] set=0x%X prio=%d",
                    batch[i].source?batch[i].source:"?", batch[i].bits, batch[i].prio);
        }
    }
}
