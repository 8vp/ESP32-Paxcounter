#ifndef main_H
#define main_H

#include "led.h"
#include "macsniff.h"
#include "configmanager.h"
#include "senddata.h"

#include <esp_spi_flash.h>  // needed for reading ESP32 chip attributes
#include <esp_event_loop.h> // needed for Wifi event handler

void reset_counters(void);
void blink_LED(uint16_t set_color, uint16_t set_blinkduration);
void led_loop(void);
uint64_t uptime();

#endif