
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include "ntp.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>


#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
//#include "protocol_examples_common.h"
#include "esp_sntp.h"


static const char *TAG = "ntp";

void task_ntp(void *arg)
{

	ntp_start();


	while (1) {

/*
		struct timeval tv_now;
		gettimeofday(&tv_now, NULL);

		time_t now;
		char strftime_buf[64];
		struct tm timeinfo;

		time(&now);



		setenv("TZ", "MSK-3", 1);
		tzset();

		localtime_r(&now, &timeinfo);
		strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
		ESP_LOGI(TAG, "The current date/time in Moscow is: %s sec=%lld", strftime_buf,now);
		*/
		vTaskDelay(1000 / portTICK_PERIOD_MS );

	        }

}


#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void ntp_start(void)
{
	time_t now;
	struct tm timeinfo;

    obtain_time();
    time(&now);
    char strftime_buf[64];

    setenv("TZ", "MSK-3", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    ESP_LOGI(TAG, "The current date/time in Moscow is: %s", strftime_buf);
}

void obtain_time(void)
{

#ifdef LWIP_DHCP_GET_NTP_SRV
    sntp_servermode_dhcp(1);      // accept NTP offers from DHCP server, if any
#endif

    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    sntp_setservername(0, "pool.ntp.org");

    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

