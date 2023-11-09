

#include "esp_log.h"
#include "main.h"
#include "blynk_management.h"
#include "protocol.h"


static const char *TAG = "blynk_management";


extern char 			WorkTime[64];
extern char  			Ver[16];
extern SSensorInfo 		SensorInfo[SENSOR_COUNT];
extern enum eMode 		Switch_Mode[COUNT_SWITCH];
extern uint8_t			Switch_Source[COUNT_SWITCH];
extern int8_t			Switch_Temp_Low[COUNT_SWITCH];
extern int8_t			Switch_Temp_High[COUNT_SWITCH];
extern bool 			test_mode;

blynk_client_t *client;

/* Blynk client state handler */
char *getesp_reset_reason_str(esp_reset_reason_t reason)
{
	static char reasonStr[12][64]={"Reason not in diapason",
									"Reset reason can not be determined",
									"Reset due to power-on event",
									"Reset by external pin (not applicable for ESP32)",
									"Software reset via esp_restart",
									"Software reset due to exception/panic",
									"Reset (software or hardware) due to interrupt watchdog",
									"Reset due to task watchdog",
									"Reset due to other watchdogs",
									"Reset after exiting deep sleep mode",
									"Brownout reset (software or hardware)",
									"Reset over SDIO"};


	return (reason>=12)? reasonStr[0]:reasonStr[reason+1];

}
void state_handler(blynk_client_t *c, const blynk_state_evt_t *ev, void *data) {
	ESP_LOGI(TAG, "state: %d\n", ev->state);

	if (ev->state==BLYNK_STATE_AUTHENTICATED)
	{
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_1_TEMPERATURE_SOURCE);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_2_TEMPERATURE_SOURCE);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_3_TEMPERATURE_SOURCE);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_4_TEMPERATURE_SOURCE);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_5_TEMPERATURE_SOURCE);

		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_1_TEMPERATURE_LOW);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_2_TEMPERATURE_LOW);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_3_TEMPERATURE_LOW);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_4_TEMPERATURE_LOW);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_5_TEMPERATURE_LOW);

		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_1_TEMPERATURE_HIGH);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_2_TEMPERATURE_HIGH);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_3_TEMPERATURE_HIGH);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_4_TEMPERATURE_HIGH);
		blynk_send(c, BLYNK_CMD_HARDWARE_SYNC, 0, "si", "vr", VP_SWITCH_5_TEMPERATURE_HIGH);


	}

}

/* Virtual write handler */
void vw_handler(blynk_client_t *c, uint16_t id, const char *cmd, int argc, char **argv, void *data) {


	if (argc<=1) {
		return;
	}

	int pin = atoi(argv[0]);

	//printf("vw_handler pin=%d\n",pin);

	switch (pin) {
		case VP_RESTART_CMD:
		{
			if (atoi(argv[1])==1)
				esp_restart();

			break;

		}
		case VP_SWITCH_1:case VP_SWITCH_2:case VP_SWITCH_3:
		case VP_SWITCH_4:case VP_SWITCH_5:
		{
			if (atoi(argv[1])==1)
			{
				ChangeSwitchState(pin-VP_SWITCH_1);
				SendSwitchState(c,pin-VP_SWITCH_1);
			}

			break;

		}
		case VP_MODE_SWITCH_1:case VP_MODE_SWITCH_2:case VP_MODE_SWITCH_3:
		case VP_MODE_SWITCH_4:case VP_MODE_SWITCH_5:
		{
			if (atoi(argv[1])==1)
			{
				ChangeSwitchMode(pin-VP_MODE_SWITCH_1);
				SendSwitchMode(c,pin-VP_MODE_SWITCH_1);
			}

			break;

		}
		case VP_SWITCH_1_TEMPERATURE_SOURCE:case VP_SWITCH_2_TEMPERATURE_SOURCE:case VP_SWITCH_3_TEMPERATURE_SOURCE:
		case VP_SWITCH_4_TEMPERATURE_SOURCE:case VP_SWITCH_5_TEMPERATURE_SOURCE:
		{
			Switch_Source[pin-VP_SWITCH_1_TEMPERATURE_SOURCE]=atoi(argv[1])-1;
			break;

		}
		case VP_SWITCH_1_TEMPERATURE_LOW:case VP_SWITCH_2_TEMPERATURE_LOW:case VP_SWITCH_3_TEMPERATURE_LOW:
		case VP_SWITCH_4_TEMPERATURE_LOW:case VP_SWITCH_5_TEMPERATURE_LOW:
		{
			Switch_Temp_Low[pin-VP_SWITCH_1_TEMPERATURE_LOW]=atoi(argv[1]);
			break;
		}
		case VP_SWITCH_1_TEMPERATURE_HIGH:case VP_SWITCH_2_TEMPERATURE_HIGH:case VP_SWITCH_3_TEMPERATURE_HIGH:
		case VP_SWITCH_4_TEMPERATURE_HIGH:case VP_SWITCH_5_TEMPERATURE_HIGH:
		{
			Switch_Temp_High[pin-VP_SWITCH_1_TEMPERATURE_HIGH]=atoi(argv[1]);
			break;
		}






	}

}

/* Blynk client state handler */
/* Virtual read handler */
void vr_handler(blynk_client_t *c, uint16_t id, const char *cmd, int argc, char **argv, void *data) {

	if (!argc) {
		return;
	}

	int pin = atoi(argv[0]);


	switch (pin) {
		case VP_RESTART_REASON:
		{
			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", VP_RESTART_REASON, getesp_reset_reason_str(esp_reset_reason()));
			break;
		}
		case VP_COUNTER:
		{
			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", VP_COUNTER, WorkTime);
			break;
		}
		case VP_VERSION:
		{
			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", VP_VERSION, Ver);
			break;
		}
		case VP_SENSOR_1_COUNTER:case VP_SENSOR_2_COUNTER:case VP_SENSOR_3_COUNTER:case VP_SENSOR_4_COUNTER:case VP_SENSOR_5_COUNTER:
		{
			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sii", "vw", pin, SensorInfo[pin-VP_SENSOR_1_COUNTER].counter);
			break;
		}
		case VP_SENSOR_1_TEMPERATURE:case VP_SENSOR_2_TEMPERATURE:case VP_SENSOR_3_TEMPERATURE:case VP_SENSOR_4_TEMPERATURE:case VP_SENSOR_5_TEMPERATURE:
		{
			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sif", "vw", pin, SensorInfo[pin-VP_SENSOR_1_TEMPERATURE].SensorData.temperature);
			break;
		}
		case VP_SENSOR_1_HUMIDITY:case VP_SENSOR_2_HUMIDITY:case VP_SENSOR_3_HUMIDITY:case VP_SENSOR_4_HUMIDITY:case VP_SENSOR_5_HUMIDITY:
		{
			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sif", "vw", pin, SensorInfo[pin-VP_SENSOR_1_HUMIDITY].SensorData.humidity);
			break;
		}

	}
}

void Blynk_Timer(void *pvParameter)
{
	static uint8_t init=0;

	while(1)
	{
		if (blynk_get_state((blynk_client_t*)pvParameter)==BLYNK_STATE_AUTHENTICATED)
		{

			char tableCommand[16];

			if (init==0)
			{
				snprintf(tableCommand,sizeof(tableCommand),"add");
				init=1;
			}
			else
				snprintf(tableCommand,sizeof(tableCommand),"update");

			for(uint8_t i=0;i<SENSOR_COUNT;i++)
			{
				char szStr[16];

				snprintf(szStr,sizeof(szStr),"Датчик %d",i+1);
				if (SensorInfo[i].timer_no_data)
					blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisi", "vw",VP_TABLE_TEST,"select",i*4+1);
				else
					blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisi", "vw",VP_TABLE_TEST,"deselect",i*4+1);

				blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_TEST,tableCommand,i*4+1,szStr," ");

				if (SensorInfo[i].timer_no_data)
				{
					snprintf(szStr,sizeof(szStr),"%d",SensorInfo[i].counter);
					blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisi", "vw",VP_TABLE_TEST,"select",i*4+2);
				}
				else
				{
					snprintf(szStr,sizeof(szStr)," ");
					blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisi", "vw",VP_TABLE_TEST,"deselect",i*4+2);
				}

				blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_TEST,tableCommand,i*4+2,"Счетчик",szStr);

				if (SensorInfo[i].timer_no_data)
				{
					snprintf(szStr,sizeof(szStr),"%.1fC°",SensorInfo[i].SensorData.temperature);
					blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisi", "vw",VP_TABLE_TEST,"select",i*4+3);
				}
				else
				{
					snprintf(szStr,sizeof(szStr)," ");
					blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisi", "vw",VP_TABLE_TEST,"deselect",i*4+3);
				}

				blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_TEST,tableCommand,i*4+3,"Температура",szStr);

				if (SensorInfo[i].timer_no_data)
				{
					snprintf(szStr,sizeof(szStr),"%.1f%%",SensorInfo[i].SensorData.humidity);
					blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisi", "vw",VP_TABLE_TEST,"select",i*4+4);
				}
				else
				{
					snprintf(szStr,sizeof(szStr)," ");
					blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisi", "vw",VP_TABLE_TEST,"deselect",i*4+4);
				}

				blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_TEST,tableCommand,i*4+4,"Влажность",szStr);
			}


			for(uint8_t i=0;i<COUNT_SWITCH;i++)
			{
				SendSwitchState(pvParameter,i);
				SendSwitchMode(pvParameter,i);
			}

		}

            vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);

}
void SendSwitchState(blynk_client_t* pvParameter,uint8_t pin)
{
	if (GetSwitchState(pin)==0)
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sii", "vw", VP_LED_SWITCH_1+pin,0);
	else
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sii", "vw", VP_LED_SWITCH_1+pin,255);


}
void SendSwitchMode(blynk_client_t* pvParameter,uint8_t pin)
{
	if (GetSwitchMode(pin)==0)
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sii", "vw", VP_LED_MODE_SWITCH_1+pin,0);
	else
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sii", "vw", VP_LED_MODE_SWITCH_1+pin,255);


}

void BlynkInit(void)
{


	client = malloc(sizeof(blynk_client_t));

	blynk_init(client);

	blynk_options_t opt = {
	    		.server = BLYNK_SERVER
	   	};

	ESP_LOGI(TAG,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! test_mode=%d",test_mode);

	if (!test_mode)
		snprintf(opt.token,sizeof(opt.token),"%s",BLYNK_TOKEN);
	else
		snprintf(opt.token,sizeof(opt.token),"%s",BLYNK_TOKEN_TEST);


   	blynk_set_options(client, &opt);

	/* Subscribe to state changes and errors */
	blynk_set_state_handler(client, state_handler, NULL);

   	/* blynk_set_handler sets hardware (BLYNK_CMD_HARDWARE) command handler */
	blynk_set_handler(client, "vw", vw_handler, NULL);
	blynk_set_handler(client, "vr", vr_handler, NULL);

	/* Start Blynk client task */

	xTaskCreate(&Blynk_Timer, "Blynk_Timer", 2048*2, (void*)client, 5, NULL );

	blynk_start(client);
}
