#include "main.h"

#include <Arduino.h>

// Basic Config
#include "globals.h"
#include "loraconf.h"

// WiFi Functions
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>

#define WIFI_INIT_CONFIG_PAXCOUNTER() { \
    .event_handler = &esp_event_send, \
    .wpa_crypto_funcs = g_wifi_default_wpa_crypto_funcs, \
    .static_rx_buf_num = CONFIG_ESP32_WIFI_STATIC_RX_BUFFER_NUM,\
    .dynamic_rx_buf_num = CONFIG_ESP32_WIFI_DYNAMIC_RX_BUFFER_NUM,\
    .tx_buf_type = CONFIG_ESP32_WIFI_TX_BUFFER_TYPE,\
    .static_tx_buf_num = WIFI_STATIC_TX_BUFFER_NUM,\
    .dynamic_tx_buf_num = WIFI_DYNAMIC_TX_BUFFER_NUM,\
    .ampdu_rx_enable = WIFI_AMPDU_RX_ENABLED,\
    .ampdu_tx_enable = WIFI_AMPDU_TX_ENABLED,\
    .nvs_enable = 0,\
    .nano_enable = WIFI_NANO_FORMAT_ENABLED,\
    .tx_ba_win = WIFI_DEFAULT_TX_BA_WIN,\
    .rx_ba_win = WIFI_DEFAULT_RX_BA_WIN,\
    .magic = WIFI_INIT_CONFIG_MAGIC\
};

// Local logging tag
static const char *TAG = "wifisniffer";

static wifi_country_t wifi_country = {.cc="EU", .schan=1, .nchan=13, .policy=WIFI_COUNTRY_POLICY_AUTO};

#ifdef VENDORFILTER
#include <array>
#include <algorithm>
#include "vendor_array.h"
#endif

extern void wifi_sniffer_init(void);
extern void wifi_sniffer_set_channel(uint8_t channel);
extern void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

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

void wifi_sniffer_init(void) {
		wifi_init_config_t cfg = WIFI_INIT_CONFIG_PAXCOUNTER();
    	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    	ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) );
    	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
    	ESP_ERROR_CHECK( esp_wifi_start() );
    	ESP_ERROR_CHECK( esp_wifi_set_max_tx_power(-128) ); // we don't need to TX, so we use lowest power level to save energy
    	wifi_promiscuous_filter_t filter = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT}; // we need only MGMT frames
    	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter)); // set MAC frame filter
    	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
    	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
}

void wifi_sniffer_set_channel(uint8_t channel) {
	esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {
	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
	char counter [10];

	if (( cfg.rssilimit == 0 ) || (ppkt->rx_ctrl.rssi > cfg.rssilimit )) { // rssi is negative value
	    uint64_t addr2int = ( (uint64_t)hdr->addr2[0] ) | ( (uint64_t)hdr->addr2[1] << 8 ) | ( (uint64_t)hdr->addr2[2] << 16 ) | \
			( (uint64_t)hdr->addr2[3] << 24 ) | ( (uint64_t)hdr->addr2[4] << 32 ) | ( (uint64_t)hdr->addr2[5] << 40 );

#ifdef VENDORFILTER
		uint32_t vendor2int = ( (uint32_t)hdr->addr2[2] ) | ( (uint32_t)hdr->addr2[1] << 8 ) | ( (uint32_t)hdr->addr2[0] << 16 );

		if ( std::find(vendors.begin(), vendors.end(), vendor2int) != vendors.end() ) {
#endif
		    macs.insert(addr2int);

			// INFO: RSSI when adding MAC
			ESP_LOGI(TAG, "WiFi RSSI: %02d", ppkt->rx_ctrl.rssi);

			// if new unique MAC logged increment counter on display
			if ( macs.size() > macnum ) {
				macnum = macs.size();
				itoa(macnum, counter, 10);
				u8x8.draw2x2String(0, 0, counter);
				ESP_LOGI(TAG, "MAC counter: %4i", macnum);
			}
			
			
#ifdef VENDORFILTER
		}
#endif
	} else {
		ESP_LOGI(TAG, "Ignoring RSSI %02d (limit: %i)", ppkt->rx_ctrl.rssi, cfg.rssilimit );
	}
	yield();
}
