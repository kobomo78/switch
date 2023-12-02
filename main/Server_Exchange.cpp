/*
 * Server_Exchange.cpp
 *
 *  Created on: 7 сент. 2023 г.
 *      Author: boyarkin.k
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "math.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "esp_netif_lwip_internal.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "protocol.h"
#include "Server_Exchange.h"
#include "blynk_management.h"
#include "cJSON.h"
#include "esp_partition.h"


#define PORT		 34004

#define SERVER_DATA_IP_ADDR 		 "109.194.141.27"
#define SERVER_DATA_PORT	 		 34004
#define SERVER_DATA_PORT_FOR_CORE	 34005

extern EventGroupHandle_t s_server_exchange_event_group;

static const char *TAG = "Server_Exchange";
int sock=0;
int sock_for_core=0;

extern SSensorInfo SensorInfo[SENSOR_COUNT];

bool SocketInit(void)
{
    int addr_family = 0;
    int ip_protocol = 0;

	addr_family = AF_INET;
	ip_protocol = IPPROTO_IP;

	sock = socket(addr_family, SOCK_DGRAM, ip_protocol);

	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d %s", errno,esp_err_to_name(errno));
		return false;
	}

	sock_for_core = socket(addr_family, SOCK_DGRAM, ip_protocol);

	if (sock_for_core < 0) {
		ESP_LOGE(TAG, "Unable to create socket sock_for_core: errno %d %s", errno,esp_err_to_name(errno));
		return false;
	}



	return true;

}
void Server_Send_Data_Core_Dump(uint8_t *pData,uint16_t length)
{
	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(SERVER_DATA_IP_ADDR);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(SERVER_DATA_PORT_FOR_CORE);

		int err = sendto(sock_for_core, pData, length, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    	if (err < 0) {
    		ESP_LOGE(TAG, "Error occurred during sending sock_for_core: errno %d (%s)", errno,esp_err_to_name(errno));
    	}
    	else
    		ESP_LOGI(TAG, "sendto OK");


}
void Server_Save_Data(void *pvParameter)
{

	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(SERVER_DATA_IP_ADDR);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(SERVER_DATA_PORT);


    	while(1)
    	{

    		char data[512];

    		cJSON *root,*fmt;
   			root = cJSON_CreateObject();
   			bool bNeedSend=false;

   			for(uint8_t i=0;i<SENSOR_COUNT;i++)
   			{
					char str[16];
					snprintf(str,sizeof(str),"sensor_%d",i);
					cJSON_AddItemToObject(root, str, fmt=cJSON_CreateObject());
	   				if (SensorInfo[i].timer_no_data)
	   				{
						snprintf(str,sizeof(str),"%.1f",SensorInfo[i].SensorData.temperature);
						cJSON_AddStringToObject(fmt,"temperature",str);
						snprintf(str,sizeof(str),"%.1f",SensorInfo[i].SensorData.humidity);
						cJSON_AddStringToObject(fmt,"humidity",str);
						bNeedSend=true;
	   				}
	   				else
	   				{
						cJSON_AddStringToObject(fmt,"temperature","0");
						cJSON_AddStringToObject(fmt,"humidity","0");

	   				}

   			}

   			if (bNeedSend)
   			{
   				char *my_json_string = cJSON_Print(root);
   				//ESP_LOGI(TAG, "my_json_string\n%s",my_json_string);

   				int err = sendto(sock, my_json_string, strlen(my_json_string), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	            	if (err < 0) {
	            		ESP_LOGE(TAG, "Error occurred during sending: errno %d (%s)", errno,esp_err_to_name(errno));
	            	}

   			}

   			cJSON_Delete(root);

   		    xEventGroupWaitBits(s_server_exchange_event_group,
   		            			LIMIT_ACHIEVE_BIT,
								pdTRUE,
								pdFALSE,
								60000 / portTICK_PERIOD_MS);


	    }

	    vTaskDelete(NULL);

}
void Server_Exchange(void *pvParameter)
{
	uint32_t br_addr=0;

      esp_netif_t* esp_netif=NULL;

       do
        {
    	   esp_netif=esp_netif_next(esp_netif);
	        	if (esp_netif)
	        	{
	        		if (strcmp(esp_netif->if_key,"WIFI_STA_DEF")==0)
	        		{
	        			br_addr=(esp_netif->ip_info->ip.addr&esp_netif->ip_info->netmask.addr)|(~esp_netif->ip_info->netmask.addr);
	        			break;
	        		}


	        	}
	        }while(esp_netif);

	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = br_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);


    	while(1)
    	{

   			//ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);
        	SSensorData data;
        	data.type=TYPE_GET_DATA;
        	data.sensor_addr=255;
        	data.temperature=0;
        	data.humidity=0;

            int err = sendto(sock, &data, sizeof(data), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	            if (err < 0) {
	                ESP_LOGE(TAG, "Error occurred during sending: errno %d (%s)", errno,esp_err_to_name(errno));
	            }


				for(uint8_t i=0;i<COUNT_SWITCH;i++)
					if (SensorInfo[i].timer_no_data)
						SensorInfo[i].timer_no_data--;

	            vTaskDelay(2000 / portTICK_PERIOD_MS);
	    }

	    vTaskDelete(NULL);

}
void Server_Receive(void *pvParameter)
{
	char rx_buffer[128];
    char addr_str[128];
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);
    ip_protocol = IPPROTO_IP;


   // Set timeout
   struct timeval timeout;
   timeout.tv_sec = 10;
   timeout.tv_usec = 0;
   setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

   int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
   if (err < 0) {
       ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
   }
   ESP_LOGI(TAG, "Socket bound, port %d", PORT);

   struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
   socklen_t socklen = sizeof(source_addr);

        while (1) {
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Data received
            if (len >= 0) {
                // Get the sender's ip address as string
                if (source_addr.ss_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
                }

                if (((SSensorData*)rx_buffer)->type==TYPE_DATA)
                {
                	if (((SSensorData*)rx_buffer)->sensor_addr<SENSOR_COUNT)
                	{
                		SensorInfo[((SSensorData*)rx_buffer)->sensor_addr].counter++;
                		SensorInfo[((SSensorData*)rx_buffer)->sensor_addr].timer_no_data=5;
                		SensorInfo[((SSensorData*)rx_buffer)->sensor_addr].SensorData=*((SSensorData*)rx_buffer);

                	}

                }


                }

            }


    vTaskDelete(NULL);

}
