#include <string.h>
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "nvs_flash.h"
#include "nvs.h"


#include "wifi_smartconfig.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const char *TAG = "wifi_sc";
wifi_config_t *wifi_config;

static void sc_callback(smartconfig_status_t status, void *pdata);


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	nvs_handle handle;
	esp_err_t err;

	const char *NVS_WIFI = "wifi data";
	uint32_t len = sizeof(wifi_config_t);
	// Open
    err = nvs_open(NVS_WIFI, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;


    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
	    // Read
	    err = nvs_get_blob(handle, "wifi_config", wifi_config, &len);
		ESP_LOGI(TAG, "err : %d",err);

	    if (err == ESP_ERR_NVS_NOT_FOUND) 
		{
			ESP_ERROR_CHECK( esp_smartconfig_stop() );
			ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
			ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
		}
		else if(err == ESP_OK)
		{
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
		}

        break;
    case SYSTEM_EVENT_STA_GOT_IP:
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		
        break;
	case SYSTEM_EVENT_STA_CONNECTED:
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");

		// Write
		ESP_ERROR_CHECK( nvs_set_blob(handle, "wifi_config", wifi_config, sizeof(wifi_config_t)) );
		break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");

	    // Read
	    len = 0;
	    err = nvs_get_blob(handle, "wifi_config", NULL, &len);
	    if (err == ESP_ERR_NVS_NOT_FOUND) 
		{
			esp_wifi_disconnect();
			ESP_ERROR_CHECK( esp_wifi_stop() );
			ESP_ERROR_CHECK( esp_wifi_start() );
		}
		else if(err == ESP_OK)
		{
			esp_wifi_connect();
		}
        
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void wifi_smartconfig_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
	wifi_config = (wifi_config_t *)malloc(sizeof(wifi_config_t));
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
            break;
        case SC_STATUS_LINK:
            ESP_LOGI(TAG, "SC_STATUS_LINK");
			

			memset(wifi_config, 0, sizeof(wifi_config_t));
            wifi_config = pdata;
            ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
            ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }

			
			ESP_ERROR_CHECK( esp_smartconfig_stop() );
            break;
        default:
            break;
    }
}

void wifi_wait_connected(void)
{
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, true, true, portMAX_DELAY);
}


