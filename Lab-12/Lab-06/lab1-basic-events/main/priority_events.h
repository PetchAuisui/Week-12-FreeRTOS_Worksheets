#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <stdint.h>

typedef enum {
  EV_PRIO_LOW = 0,
  EV_PRIO_MED = 1,
  EV_PRIO_HIGH = 2,
  EV_PRIO_CRIT = 3,
} ev_priority_t;

typedef struct {
  EventBits_t bits;
  ev_priority_t prio;
  uint32_t ts_ms;
  const char* source;   // pointer only
} ev_msg_t;

bool evprio_init(EventGroupHandle_t group, UBaseType_t queue_len);
bool evprio_post(EventBits_t bits, ev_priority_t prio, const char* source);
void evprio_dispatcher_task(void *unused);

// stats (ข้อ 5.3)
uint32_t evprio_get_stats(void);
