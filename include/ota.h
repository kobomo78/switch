/*
 * ota.h
 *
 *  Created on: 28 авг. 2023 г.
 *      Author: boyarkin.k
 */

#ifndef INCLUDE_OTA_H_
#define INCLUDE_OTA_H_



#include <stdio.h>
#include <map>
#include <string>
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"


#define OTA_HOME_SERVER_URL  "https://109.194.141.27:34002/firmware/switch.bin"

esp_err_t _http_event_handler(esp_http_client_event_t *evt);

void task_ota(void *arg);

class COtaUpdate
{
	private:



	esp_err_t validate_image_header(esp_app_desc_t *new_app_info);


	public:


		EventGroupHandle_t  m_s_ip_event_group;


		COtaUpdate(void);

		void Init(void);

		void update(void);

};





#endif /* INCLUDE_OTA_H_ */
