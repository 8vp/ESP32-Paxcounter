// Hardware related definitions for Pycom LoPy Board (not: LoPy4)

#define LOPY
#define CFG_sx1272_radio 1

// Hardware pin definitions for Pycom LoPy board
#define PIN_SPI_SS    17
#define PIN_SPI_MOSI  27
#define PIN_SPI_MISO  19
#define PIN_SPI_SCK   5
#define RST   18
#define DIO0  23 // LoRa IRQ
#define DIO1  23 // workaround
#define DIO2  23 // workaround

// select WIFI antenna (internal = onboard / external = u.fl socket)
#define PIN_ANTENNA_SWITCH  16
#define WIFI_ANTENNA ANTENNA_INT // can be switced to ANTENNA_EXT
