/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#include "main.h"
#include "protocol.h"
#include "wifi.h"
#include "ota.h"
#include "blynk_management.h"
#include "Server_Exchange.h"




extern "C" {void app_main(void);}

static const char *TAG = "switch";

SSensorInfo SensorInfo[SENSOR_COUNT];

uint8_t		Switch_State[COUNT_SWITCH];
eMode 		Switch_Mode[COUNT_SWITCH];
uint8_t		Switch_Source[COUNT_SWITCH];
int8_t		Switch_Temp_Low[COUNT_SWITCH];
int8_t		Switch_Temp_High[COUNT_SWITCH];

uint32_t   counter=0;

char  Ver[16];

COtaUpdate  OtaUpdate;

void Work_counter(void *pvParameter)
{

	while(1) {
		counter++;
		if (counter==10000)
			counter=0;


		vTaskDelay(1000 / portTICK_PERIOD_MS );
	}
}

void SetSwitchStateAfterStart(void)
{
	for(uint8_t i=0;i<COUNT_SWITCH;i++)
	{
		if (Switch_Mode[i]==HANDS)
		{
			switch(i)
			{
				case 0: gpio_set_level(GPIO_OUTPUT_IO_0,Switch_State[i]);break;
				case 1: gpio_set_level(GPIO_OUTPUT_IO_1,Switch_State[i]);break;
				case 2: gpio_set_level(GPIO_OUTPUT_IO_2,Switch_State[i]);break;
				case 3: gpio_set_level(GPIO_OUTPUT_IO_3,Switch_State[i]);break;
				case 4: gpio_set_level(GPIO_OUTPUT_IO_4,Switch_State[i]);break;

			}
		}

	}


}
void ChangeSwitchState(uint8_t pin)
{

	if (Switch_Mode[pin]==HANDS)
	{
		uint8_t state;

		if (Switch_State[pin]==0)
			state=1;
		else
			state=0;

		switch(pin)
		{
			case 0: gpio_set_level(GPIO_OUTPUT_IO_0,state);break;
			case 1: gpio_set_level(GPIO_OUTPUT_IO_1,state);break;
			case 2: gpio_set_level(GPIO_OUTPUT_IO_2,state);break;
			case 3: gpio_set_level(GPIO_OUTPUT_IO_3,state);break;
			case 4: gpio_set_level(GPIO_OUTPUT_IO_4,state);break;

		}

		Switch_State[pin]=state;
		Save_Data_NVS_Pin(pin);
	}

}
void ChangeSwitchMode(uint8_t pin)
{

	if (Switch_Mode[pin]==HANDS)
		Switch_Mode[pin]=AUTO;
	else
		Switch_Mode[pin]=HANDS;

	Save_Data_NVS_Pin(pin);
}


uint8_t GetSwitchState(uint8_t pin)
{
	return Switch_State[pin];
}
uint8_t GetSwitchMode(uint8_t pin)
{
	return Switch_Mode[pin];
}


void Init(void)
{

   gpio_config_t io_conf;
   memset((void*)&io_conf,0,sizeof(io_conf));

   io_conf.mode = GPIO_MODE_OUTPUT;
   io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
   io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;

   gpio_config(&io_conf);

   memset(SensorInfo,0,sizeof(SensorInfo));
   memset(Switch_Source,0xFF,sizeof(Switch_Source));
   memset(Switch_Temp_Low,0,sizeof(Switch_Temp_Low));
   memset(Switch_Temp_High,0,sizeof(Switch_Temp_High));



   SetSwitchStateAfterStart();

   esp_log_level_set("ota", ESP_LOG_NONE);
   esp_log_level_set("esp_ota_ops", ESP_LOG_NONE);
   esp_log_level_set("HTTP_CLIENT", ESP_LOG_NONE);
   esp_log_level_set("esp_https_ota", ESP_LOG_NONE);





}
void Read_Data_NVS(void)
{
	 esp_err_t err;
	 nvs_handle_t my_handle;
	 char Mode_Key[COUNT_SWITCH][16]={"mode_1","mode_2","mode_3","mode_4","mode_5"};
	 char State_Key[COUNT_SWITCH][16]={"state_1","state_2","state_3","state_4","state_5"};


	 err = nvs_open("storage", NVS_READONLY, &my_handle);
	     if (err != ESP_OK) {
	    	 ESP_LOGE(TAG,"Error (%s) opening NVS handle!", esp_err_to_name(err));
	         return;
	     } else {

	     }


	    for(uint8_t i=0;i<COUNT_SWITCH;i++)
	    {
	    	err = nvs_get_u8(my_handle, Mode_Key[i], (uint8_t*)&Switch_Mode[i]);

	    	if (err!=ESP_OK)
	    		Switch_Mode[i]=HANDS;

	    	err = nvs_get_u8(my_handle, State_Key[i], &Switch_State[i]);

	    	if (err!=ESP_OK)
	    		Switch_State[i]=0;
	    }

      nvs_close(my_handle);


}
void Save_Data_NVS_Pin(uint8_t pin)
{
	 esp_err_t err;
	 nvs_handle_t my_handle;
	 char Mode_Key[COUNT_SWITCH][16]={"mode_1","mode_2","mode_3","mode_4","mode_5"};
	 char State_Key[COUNT_SWITCH][16]={"state_1","state_2","state_3","state_4","state_5"};


	 err = nvs_open("storage", NVS_READWRITE, &my_handle);
	     if (err != ESP_OK) {
	    	 ESP_LOGE(TAG,"Error (%s) opening NVS handle!", esp_err_to_name(err));
	         return;
	     } else {

	     }


   		nvs_set_u8(my_handle, Mode_Key[pin], Switch_Mode[pin]);
   		nvs_set_u8(my_handle, State_Key[pin], Switch_State[pin]);

     nvs_close(my_handle);


}
void Timer_Switch_State(void *pvParameter)
{

	while(1)
	{
		for(uint8_t i=0;i<COUNT_SWITCH;i++)
		{
			if (Switch_Mode[i]==AUTO)
			{
				if (SensorInfo[i].timer_no_data)
				{
					if (Switch_Source[i]<COUNT_SWITCH)
					{
						if (SensorInfo[Switch_Source[i]].SensorData.temperature<=Switch_Temp_Low[i])
						{
							gpio_set_level(static_cast<gpio_num_t>(GPIO_OUTPUT_IO_0+i),1);
							Switch_State[i]=1;
						}
						else
							if (SensorInfo[Switch_Source[i]].SensorData.temperature>=Switch_Temp_High[i])
							{
								gpio_set_level(static_cast<gpio_num_t>(GPIO_OUTPUT_IO_0+i),0);
								Switch_State[i]=0;
							}

					}
				}
				else
				{
					gpio_set_level(static_cast<gpio_num_t>(GPIO_OUTPUT_IO_0+i),0);
					Switch_State[i]=0;


				}


			}

		}

            vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);


}
void app_main(void)
{

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    Read_Data_NVS();

    ESP_ERROR_CHECK(esp_netif_init());


    xTaskCreate(&Work_counter, "Work_counter", 2048, NULL, 5, NULL );

    if (SocketInit())
    {
    	xTaskCreate(&Server_Exchange, "Server_Exchange", 4096, NULL, 5, NULL );
    	xTaskCreate(&Server_Receive, "Server_Receive", 4096, NULL, 5, NULL );
    }



    Init();
    WifiInit();
    BlynkInit();
    OtaUpdate.Init();
    xTaskCreate(&Timer_Switch_State, "Timer_Switch_State", 4096, NULL, 5, NULL );


    esp_ota_mark_app_valid_cancel_rollback();


}
