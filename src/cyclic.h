#ifndef _CYCLIC_H
#define _CYCLIC_H

#include "senddata.h"
#include "ota.h"
#include "led.h"

void doHousekeeping(void);
void IRAM_ATTR homeCycleIRQ(void);
uint64_t uptime(void);
void reset_counters(void);
int redirect_log(const char *fmt, va_list args);

#endif