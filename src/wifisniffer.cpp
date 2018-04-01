// Basic Config
#include "main.h"
#include "globals.h"

// ESP32 Functions
#include <esp_wifi.h>

#ifdef VENDORFILTER
	#include <array>
	#include <algorithm>
	#include "vendor_array.h"
#endif

// Local logging tag
static const char *TAG = "wifisniffer";

// function defined in rokkithash.cpp
uint32_t rokkit(const char * , int );

static wifi_country_t wifi_country = {.cc=WIFI_MY_COUNTRY, .schan=WIFI_CHANNEL_MIN, .nchan=WIFI_CHANNEL_MAX, .policy=WIFI_COUNTRY_POLICY_MANUAL};

typedef struct {
	unsigned frame_ctrl:16;
	unsigned duration_id:16;
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

extern void wifi_sniffer_init(void);
extern void wifi_sniffer_set_channel(uint8_t channel);
extern void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

void wifi_sniffer_init(void) {
		wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
		cfg.nvs_enable = 0; // we don't need any wifi settings from NVRAM
		wifi_promiscuous_filter_t filter = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT}; // we need only MGMT frames
    	ESP_ERROR_CHECK(esp_wifi_init(&cfg));						// configure Wifi with cfg
    	ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country));		// set locales for RF and channels
		ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));	// we don't need NVRAM
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));	// set MAC frame filter
    	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
    	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));			// now switch on monitor mode
}

void wifi_sniffer_set_channel(uint8_t channel) {
	esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {
	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
	char counter [6]; // uint16_t -> 2 byte -> 5 decimals + '0' terminator -> 6 chars
	char macbuf [21]; // uint64_t -> 8 byte -> 10 decimals + '0' terminator -> 21 chars
	uint64_t addr2int;
	uint32_t vendor2int;
	uint16_t hashedmac;
	std::pair<std::set<uint16_t>::iterator, bool> newmac;

	if (( cfg.rssilimit == 0 ) || (ppkt->rx_ctrl.rssi > cfg.rssilimit )) { // rssi is negative value
	    addr2int = ( (uint64_t)hdr->addr2[0] ) | ( (uint64_t)hdr->addr2[1] << 8 ) | ( (uint64_t)hdr->addr2[2] << 16 ) | \
			( (uint64_t)hdr->addr2[3] << 24 ) | ( (uint64_t)hdr->addr2[4] << 32 ) | ( (uint64_t)hdr->addr2[5] << 40 );

#ifdef VENDORFILTER // uses vendor array with prefiltered OUIs (no local nd no group MACs, bits 0+1 in 1st byte of OUI)
		vendor2int = ( (uint32_t)hdr->addr2[2] ) | ( (uint32_t)hdr->addr2[1] << 8 ) | ( (uint32_t)hdr->addr2[0] << 16 );
		if ( std::find(vendors.begin(), vendors.end(), vendor2int) != vendors.end() ) {
#endif
			// salt and hash MAC, and if new unique one, store identifier in container and increment counter on display
			// https://en.wikipedia.org/wiki/MAC_Address_Anonymization
			
			addr2int |= (uint64_t) salt << 48;		// prepend 16-bit salt to 48-bit MAC
			snprintf(macbuf, 21, "%llx", addr2int);	// convert unsigned 64-bit salted MAC to 16 digit hex string
			hashedmac = rokkit(macbuf, 5);			// hash MAC string, use 5 chars to fit hash in uint16_t container
			newmac = macs.insert(hashedmac);		// store hashed MAC only if first time seen
			if (newmac.second) {					// if first time seen MAC
				macnum++;								// increment MAC counter
				snprintf(counter, 6, "%i", macnum);		// convert 16-bit MAC counter to decimal counter value
				u8x8.draw2x2String(0, 0, counter);		// display counter
				ESP_LOGI(TAG, "#%05i: RSSI %04d -> Hash %04x", macnum, ppkt->rx_ctrl.rssi, salt, hashedmac);
			}

#ifdef VENDORFILTER
		}
#endif
	} else
		ESP_LOGI(TAG, "RSSI %04d -> ignoring (limit: %i)", ppkt->rx_ctrl.rssi, cfg.rssilimit);

	yield();
}
