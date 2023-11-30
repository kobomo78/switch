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
#include "ntp.h"
#include "blynk_management.h"
#include "Server_Exchange.h"




extern "C" {void app_main(void);}

static const char *TAG = "switch";

SSensorInfo SensorInfo[SENSOR_COUNT];

uint8_t		Switch_State[COUNT_SWITCH]={0,0,0,0,0};
uint8_t		Switch_State_NVS[COUNT_SWITCH];
eMode 		Switch_Mode[COUNT_SWITCH];
uint8_t		Switch_Source[COUNT_SWITCH];
float		Switch_Temp_Low[COUNT_SWITCH];
float		Switch_Temp_High[COUNT_SWITCH];
SSwitchStat SwitchStat[COUNT_SWITCH];

uint32_t   counter=0;
bool test_mode=false;
char WorkTime[64];


char  Ver[16];

COtaUpdate  OtaUpdate;

void Work_counter(void *pvParameter)
{

	while(1) {
		counter++;
		time_t seconds(counter);
		tm *p = gmtime(&seconds);
		snprintf(WorkTime,sizeof(WorkTime),"%d days %d:%d:%d",p->tm_yday,p->tm_hour,p->tm_min,p->tm_sec);


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
				case 0: SetSwitchState(GPIO_OUTPUT_IO_0,Switch_State_NVS[i]);break;
				case 1: SetSwitchState(GPIO_OUTPUT_IO_1,Switch_State_NVS[i]);break;
				case 2: SetSwitchState(GPIO_OUTPUT_IO_2,Switch_State_NVS[i]);break;
				case 3: SetSwitchState(GPIO_OUTPUT_IO_3,Switch_State_NVS[i]);break;
				case 4: SetSwitchState(GPIO_OUTPUT_IO_4,Switch_State_NVS[i]);break;

			}
		}

	}


}
void SetSwitchState(gpio_num_t gpio_num, uint8_t state)
{

	uint8_t index=GetSwitchIndexPin(gpio_num);


	if (state!=Switch_State[index])
	{
		gpio_set_level(gpio_num,state);
		Switch_State[index]=state;

		if (state)
		{
			time_t now;
			time(&now);

			SwitchStat[index].lastOn=now;
			SwitchStat[index].DurationOn=0;

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
			case 0: SetSwitchState(GPIO_OUTPUT_IO_0,state);break;
			case 1: SetSwitchState(GPIO_OUTPUT_IO_1,state);break;
			case 2: SetSwitchState(GPIO_OUTPUT_IO_2,state);break;
			case 3: SetSwitchState(GPIO_OUTPUT_IO_3,state);break;
			case 4: SetSwitchState(GPIO_OUTPUT_IO_4,state);break;

		}

		Save_Data_NVS_Pin(pin);
	}

}
uint8_t GetSwitchPin(uint8_t pin)
{
	switch(pin)
	{
		case 0: return GPIO_OUTPUT_IO_0;
		case 1: return GPIO_OUTPUT_IO_1;
		case 2: return GPIO_OUTPUT_IO_2;
		case 3: return GPIO_OUTPUT_IO_3;
		case 4: return GPIO_OUTPUT_IO_4;

	}
	return 0xFF;


}
uint8_t GetSwitchIndexPin(gpio_num_t gpio_num)
{
	if (gpio_num==GPIO_OUTPUT_IO_0)
		return 0;
	else
	{
		if (gpio_num==GPIO_OUTPUT_IO_1)
			return 1;
		else
		{
			if (gpio_num==GPIO_OUTPUT_IO_2)
				return 2;
			else
			{
				if (gpio_num==GPIO_OUTPUT_IO_3)
					return 3;
				else
				{
					if (gpio_num==GPIO_OUTPUT_IO_4)
						return 4;
					else
						return 0;


				}


			}


		}


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

   gpio_set_level(GPIO_OUTPUT_IO_0,0);
   gpio_set_level(GPIO_OUTPUT_IO_1,0);
   gpio_set_level(GPIO_OUTPUT_IO_2,0);
   gpio_set_level(GPIO_OUTPUT_IO_3,0);
   gpio_set_level(GPIO_OUTPUT_IO_4,0);

   memset((void*)&io_conf,0,sizeof(io_conf));

   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
   io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
   io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

   gpio_config(&io_conf);



   if (gpio_get_level(GPIO_INPUT_TEST_MODE_IO_1)==0)
	   test_mode=true;



   memset(SensorInfo,0,sizeof(SensorInfo));
   memset(Switch_Source,0xFF,sizeof(Switch_Source));
   memset(Switch_Temp_Low,0,sizeof(Switch_Temp_Low));
   memset(Switch_Temp_High,0,sizeof(Switch_Temp_High));
   memset(SwitchStat,0,sizeof(SwitchStat));


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

	    	err = nvs_get_u8(my_handle, State_Key[i], &Switch_State_NVS[i]);

	    	if (err!=ESP_OK)
	    		Switch_State_NVS[i]=0;
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
		time_t now;
		time(&now);


		for(uint8_t i=0;i<COUNT_SWITCH;i++)
		{
			if (Switch_Mode[i]==AUTO)
			{

					if (Switch_Source[i]<COUNT_SWITCH)
					{
						if (SensorInfo[Switch_Source[i]].timer_no_data)
						{

							if (SensorInfo[Switch_Source[i]].SensorData.temperature<=Switch_Temp_Low[i])
							{
									SetSwitchState(static_cast<gpio_num_t>(GetSwitchPin(i)),1);

							}
							else
								if (SensorInfo[Switch_Source[i]].SensorData.temperature>=Switch_Temp_High[i])
								{
										SetSwitchState(static_cast<gpio_num_t>(GetSwitchPin(i)),0);
								}

						}
						else
						{
								SetSwitchState(static_cast<gpio_num_t>(GetSwitchPin(i)),0);
						}
				}
				else
				{
						SetSwitchState(static_cast<gpio_num_t>(GetSwitchPin(i)),0);

				}

			}

			if (Switch_State[i])
				SwitchStat[i].DurationOn=now-SwitchStat[i].lastOn;

		}

        vTaskDelay(1000 / portTICK_PERIOD_MS);

	}

    vTaskDelete(NULL);


}
void SendCoreDump(void)
{
	esp_partition_iterator_t iterator=NULL;

	iterator=esp_partition_find(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_DATA_COREDUMP,NULL);

	if (iterator)
	{
		const esp_partition_t *esp_partition;

		esp_partition=esp_partition_get(iterator);

		if (esp_partition)
		{

			uint8_t buffer[128];

			for(uint32_t i=0;i<esp_partition->size/sizeof(buffer);i++)
			{
				esp_partition_read(esp_partition, 0+i*sizeof(buffer), buffer, sizeof(buffer));

				Server_Send_Data_Core_Dump(buffer,sizeof(buffer));
			}

		}
		else
			ESP_LOGE(TAG,"esp_partition==NULL");

	}
	else
		ESP_LOGE(TAG,"iterator==NULL");



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

    ESP_LOGI(TAG,"!!!!!!!!!!!!!!!!!!!!!!!!!START switch programm!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Read_Data_NVS();

    ESP_ERROR_CHECK(esp_netif_init());


    xTaskCreate(&Work_counter, "Work_counter", 2048, NULL, 5, NULL );




    Init();
    WifiInit();

    if (SocketInit())
    	{
    		xTaskCreate(&Server_Exchange, "Server_Exchange", 4096, NULL, 5, NULL );
    		xTaskCreate(&Server_Receive, "Server_Receive", 4096, NULL, 5, NULL );
    		xTaskCreate(&Server_Save_Data, "Server_Save_Data", 4096, NULL, 5, NULL );
    		xTaskCreate(&task_ntp, "task_ntp", 4096, NULL, 5, NULL );
    	}

    BlynkInit();
    OtaUpdate.Init();
    xTaskCreate(&Timer_Switch_State, "Timer_Switch_State", 4096, NULL, 5, NULL );


    esp_ota_mark_app_valid_cancel_rollback();


}
