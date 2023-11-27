
#include "wifi.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "string.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define DEFAULT_SCAN_LIST_SIZE       15
#define EXAMPLE_ESP_WIFI_APP_COUNT   3
#define EXAMPLE_ESP_WIFI_SSID_1      "Keenetic-0086"
#define EXAMPLE_ESP_WIFI_PASS_1      "yu8tp06p"
#define EXAMPLE_ESP_WIFI_SSID_2      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS_2      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_SSID_3      "Keenetic-3608-61"
#define EXAMPLE_ESP_WIFI_PASS_3      "prometey_*168_#61"

#define EXAMPLE_ESP_MAXIMUM_RETRY  50

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


int s_retry_num=0;
uint8_t  IndexAppFound=0xFF;

static const char *TAG = "wifi";

char szApName[EXAMPLE_ESP_WIFI_APP_COUNT][32]={EXAMPLE_ESP_WIFI_SSID_1,EXAMPLE_ESP_WIFI_SSID_2,EXAMPLE_ESP_WIFI_SSID_3};
char szApPass[EXAMPLE_ESP_WIFI_APP_COUNT][32]={EXAMPLE_ESP_WIFI_PASS_1,EXAMPLE_ESP_WIFI_PASS_2,EXAMPLE_ESP_WIFI_PASS_3};




static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");

        } else {

				ESP_LOGI(TAG,"connect to the AP fail");
				ESP_LOGI(TAG, "reboot");
				esp_restart();
				xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        		}

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

	wifi_config_t wifi_config;
	memset((void*)&wifi_config,0,sizeof(wifi_config));
	snprintf((char*)wifi_config.sta.ssid,sizeof(wifi_config.sta.ssid),"%s",szApName[IndexAppFound]);
	snprintf((char*)wifi_config.sta.password,sizeof(wifi_config.sta.password),"%s",szApPass[IndexAppFound]);
	wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
	wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
        		szApName[IndexAppFound], szApPass[IndexAppFound]);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
        		szApName[IndexAppFound], szApPass[IndexAppFound]);


    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

void WifiInit(void)
{
   ESP_ERROR_CHECK(esp_netif_init());
   ESP_ERROR_CHECK(esp_event_loop_create_default());

   esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
   assert(sta_netif);

	wifi_scan();
	wifi_init_sta();

}
void wifi_scan(void)
{

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        for(uint8_t j=0;j<EXAMPLE_ESP_WIFI_APP_COUNT;j++)
        	if (strcmp(szApName[j],(char*)ap_info[i].ssid)==0)
        		IndexAppFound=j;
    }
    ESP_ERROR_CHECK(esp_wifi_stop());
    if (IndexAppFound==0xFF)
    {
		ESP_LOGI(TAG,"find the AP fail");
		ESP_LOGI(TAG, "reboot");
		esp_restart();

    }
    else
    	ESP_LOGI(TAG, "Found App index=%d", IndexAppFound);


}

