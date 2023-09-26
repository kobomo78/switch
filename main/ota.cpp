
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "ota.h"

static const char *TAG = "ota";

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern char  Ver[16];

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;

    }
    return ESP_OK;
}


static esp_err_t _http_client_init_cb(esp_http_client_handle_t http_client)
{
    esp_err_t err = ESP_OK;
    /* Uncomment to add custom headers to HTTP request */
    // err = esp_http_client_set_header(http_client, "Custom-Header", "Value");
    return err;
}

void task_ota(void *arg)
{


while (1) {

//                EventBits_t bits = xEventGroupWaitBits(((COtaUpdate*)arg)->m_s_ip_event_group,
//                WIFI_GOT_IP_ADDRESS_BIT,
//                pdFALSE,
//                pdFALSE,
//                portMAX_DELAY);

                ((COtaUpdate*)arg)->update();
                vTaskDelay(10000 / portTICK_PERIOD_MS);



        }

}


COtaUpdate::COtaUpdate(void)
{


}
void COtaUpdate::Init(void)
{

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;

    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
    	snprintf(Ver,sizeof(Ver),"%s",running_app_info.version);

    xTaskCreate(task_ota, "task_ota", 8192, (void*)this, 5, NULL);

}

void  COtaUpdate::update(void)
{
        esp_err_t ota_finish_err = ESP_OK;

        esp_http_client_config_t config;
        memset(&config,0,sizeof(config));

        config.url=OTA_HOME_SERVER_URL;
        config.port=34002;
        config.cert_pem = (char *)server_cert_pem_start;

        config.event_handler = _http_event_handler;
        config.keep_alive_enable=true;

        esp_https_ota_config_t ota_config;
        memset(&ota_config,0,sizeof(ota_config));

        ota_config.http_config = &config;
        ota_config.http_client_init_cb = _http_client_init_cb; // Register a callback to be invoked after esp_http_client is initialized

        esp_https_ota_handle_t https_ota_handle = NULL;
        esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
            goto ota_end;
//            vTaskDelete(NULL);
        }
        esp_app_desc_t app_desc;
        err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
            goto ota_end;
        }
        err = validate_image_header(&app_desc);

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "image header verification failed");
            goto ota_end;
        }

        while (1) {
            err = esp_https_ota_perform(https_ota_handle);
            if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
                break;
            }
        // esp_https_ota_perform returns after every read operation which gives user the ability to
        // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
        // data read so far.
            ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
        }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        // the OTA image was not completely received and user can customise the response to this situation.
        ESP_LOGE(TAG, "Complete data was not received.");
    } else {
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
            vTaskDelay(1000 / portTICK_PERIOD_MS);



            esp_restart();
        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            }
            ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
            goto ota_end;
//            vTaskDelete(NULL);
        }
    }

    ota_end:

        esp_https_ota_abort(https_ota_handle);
        ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
//        vTaskDelete(NULL);


}

esp_err_t COtaUpdate::validate_image_header(esp_app_desc_t *new_app_info)
{
    if (new_app_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

    if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0) {
        ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
        return ESP_FAIL;
    }

    return ESP_OK;
}
