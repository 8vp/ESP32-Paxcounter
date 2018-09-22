#ifndef _MAIN_H
#define _MAIN_H

#include <esp_spi_flash.h>  // needed for reading ESP32 chip attributes
#include <esp_event_loop.h> // needed for Wifi event handler
#include <esp32-hal-timer.h> // needed for timers

#include "globals.h"
#include "led.h"
#include "wifiscan.h"
#include "configmanager.h"
#include "cyclic.h"
#include "beacon_array.h"
#include "ota.h"
#include "statemachine.h"

#endif