#ifndef _TTGOBEAM_H
#define _TTGOBEAM_H

#include <stdint.h>

// Hardware related definitions for TTGO T-Beam board

#define HAS_LORA 1       // comment out if device shall not send data via LoRa
#define HAS_SPI 1        // comment out if device shall not send data via SPI
#define CFG_sx1276_radio 1 // HPD13A LoRa SoC

#define BOARD_HAS_PSRAM // use extra 4MB external RAM
#define HAS_LED GPIO_NUM_21 // on board green LED

#define HAS_BUTTON GPIO_NUM_39 // on board button "BOOT" (next to reset button)

#define HAS_BATTERY_PROBE ADC1_GPIO35_CHANNEL // battery probe GPIO pin -> ADC1_CHANNEL_7
#define BATT_FACTOR 2 // voltage divider 100k/100k on board
#define HAS_GPS 1 // use on board GPS
#define GPS_SERIAL 9600, SERIAL_8N1, GPIO_NUM_12, GPIO_NUM_15 // UBlox NEO 6M or 7M with default configuration

// Pins for LORA chip reset and interrupt lines
#define RST   LMIC_UNUSED_PIN
#define DIO0  (26)
#define DIO1  (32) // !! NEEDS EXTERNAL WIRING !!
//#define DIO1  (33)  // for T-Beam T22_V05 and T22_V07, other versions may need external wiring
#define DIO2  LMIC_UNUSED_PIN

#endif
