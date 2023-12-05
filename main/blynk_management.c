

#include <math.h>
#include "esp_log.h"
#include "main.h"
#include "blynk_management.h"
#include "protocol.h"

#include "../../../esp-idf-v5.0.1/components/spi_flash/include/esp_flash.h"

static const char *TAG = "blynk_management";


extern char 			WorkTime[64];
extern char  			Ver[16];
extern SSensorInfo 		SensorInfo[SENSOR_COUNT];
extern enum eMode 		Switch_Mode[COUNT_SWITCH];
extern uint8_t			Switch_Source[COUNT_SWITCH];
extern float			Switch_Temp_Low[COUNT_SWITCH];
extern float			Switch_Temp_High[COUNT_SWITCH];
extern bool 			test_mode;
extern 					SSwitchStat SwitchStat[COUNT_SWITCH];

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
				ChangeSwitchState(pin-VP_SWITCH_1);

			break;

		}
		case VP_MODE_SWITCH_1:case VP_MODE_SWITCH_2:case VP_MODE_SWITCH_3:
		case VP_MODE_SWITCH_4:case VP_MODE_SWITCH_5:
		{
			if (atoi(argv[1])==1)
				ChangeSwitchMode(pin-VP_MODE_SWITCH_1);

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
			Switch_Temp_Low[pin-VP_SWITCH_1_TEMPERATURE_LOW]=atof(argv[1]);
			break;
		}
		case VP_SWITCH_1_TEMPERATURE_HIGH:case VP_SWITCH_2_TEMPERATURE_HIGH:case VP_SWITCH_3_TEMPERATURE_HIGH:
		case VP_SWITCH_4_TEMPERATURE_HIGH:case VP_SWITCH_5_TEMPERATURE_HIGH:
		{
			Switch_Temp_High[pin-VP_SWITCH_1_TEMPERATURE_HIGH]=atof(argv[1]);
			break;
		}
		case VP_READ_CORE_BUTTON:
		{
			if (atoi(argv[1])==1)
				SendCoreDump();

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
		case VP_CURRENT_DATE_TIME:
		{
			time_t now;
			struct tm timeinfo;

		    time(&now);
		    char strftime_buf[64];

		    localtime_r(&now, &timeinfo);
		    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", VP_CURRENT_DATE_TIME, strftime_buf);
			break;
		}
		case VP_LED_SWITCH_1:case VP_LED_SWITCH_2:case VP_LED_SWITCH_3:case VP_LED_SWITCH_4:case VP_LED_SWITCH_5:
		{
			if (GetSwitchState(pin-VP_LED_SWITCH_1)==0)
				blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", pin,"OFF");
			else
				blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", pin,"ON");

			break;
		}
		case VP_LED_MODE_SWITCH_1:case VP_LED_MODE_SWITCH_2:case VP_LED_MODE_SWITCH_3:case VP_LED_MODE_SWITCH_4:case VP_LED_MODE_SWITCH_5:
		{
			if (GetSwitchMode(pin-VP_LED_MODE_SWITCH_1)==0)
				blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", pin,"OFF");
			else
				blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", pin,"ON");

			break;


		}
		case VP_GLOBAL_KWT:
		{
			char szStr[16];
			snprintf(szStr,sizeof(szStr),"%.2f кВт",Get_GlobalPower());
			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", VP_GLOBAL_KWT, szStr);
			break;
		}
		case VP_GLOBAL_MONEY:
		{
			char szStr[16];
			snprintf(szStr,sizeof(szStr),"%.2f руб.",Get_GlobalMoney());
			blynk_send(c, BLYNK_CMD_HARDWARE, 0, "sis", "vw", VP_GLOBAL_MONEY, szStr);
			break;
		}


	}
}
void UpdateTableSensor(void *pvParameter)
{
		static uint8_t init=0;

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
	}


}
void UpdateTableSwitchRow(void *pvParameter,uint8_t id,bool afterstart)
{
	char szStr[32];
	struct tm timeinfo;
	char strftime_buf[64];



	if (afterstart)
	{
		snprintf(szStr,sizeof(szStr),"Розетка %d",id);
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+1,szStr," ");
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+2,"Последнее включение"," ");
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+3," "," ");
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+4,"Время работы"," ");
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+5," "," ");
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+6,"Global On/Off (%)"," ");
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+7," "," ");
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+8,"Last On/Off (%)"," ");
		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"add",(id-1)*9+9," "," ");


	}
	else
	{

		if (SwitchStat[id-1].lastOn!=0)
		{
			localtime_r(&SwitchStat[id-1].lastOn, &timeinfo);
			strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
		}
		else
			snprintf(strftime_buf, sizeof(strftime_buf), " ");


		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"update",(id-1)*9+3,strftime_buf," ");

		if (SwitchStat[id-1].DurationOn!=0)
		{
			struct tm *p = gmtime(&SwitchStat[id-1].DurationOn);
			snprintf(strftime_buf,sizeof(strftime_buf),"%d days %d:%d:%d",p->tm_yday,p->tm_hour,p->tm_min,p->tm_sec);
		}
		else
			snprintf(strftime_buf, sizeof(strftime_buf), " ");


		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"update",(id-1)*9+5,strftime_buf," ");

		uint32_t TimeOnPlusOff=SwitchStat[id-1].GlobalTimeSecStateOn+SwitchStat[id-1].GlobalTimeSecStateOff;
		float OnProcent=0;
		if ((SwitchStat[id-1].GlobalTimeSecStateOn)&&(SwitchStat[id-1].GlobalTimeSecStateOff))
			OnProcent=((float)SwitchStat[id-1].GlobalTimeSecStateOn/(float)TimeOnPlusOff)*100;
		else
			if (SwitchStat[id-1].GlobalTimeSecStateOn==0)
				OnProcent=0;
			else
				OnProcent=100;


		OnProcent=round(OnProcent);
		snprintf(szStr,sizeof(szStr),"%d/%d",(uint8_t)OnProcent,100-(uint8_t)OnProcent);

		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"update",(id-1)*9+7,szStr," ");

		OnProcent=round(SwitchStat[id-1].LastTimeStateOnRelationStateOff*100);
		snprintf(szStr,sizeof(szStr),"%d/%d",(uint8_t)OnProcent,100-(uint8_t)OnProcent);

		blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sisiss", "vw",VP_TABLE_SWITCH_STAT,"update",(id-1)*9+9,szStr," ");

	}


}
void UpdateTableSwitch(void *pvParameter)
{
	static bool bAfterStart=true;

	if (blynk_get_state((blynk_client_t*)pvParameter)==BLYNK_STATE_AUTHENTICATED)
	{
		if (bAfterStart)
			blynk_send((blynk_client_t*)pvParameter, BLYNK_CMD_HARDWARE, 0, "sis", "vw",VP_TABLE_SWITCH_STAT,"clr");

		for(uint8_t i=0;i<COUNT_SWITCH;i++)
		  UpdateTableSwitchRow(pvParameter,i+1,bAfterStart);

		if (bAfterStart)
			bAfterStart=false;
	}


}

void Blynk_Timer(void *pvParameter)
{

	while(1)
	{

			UpdateTableSensor(pvParameter);
			UpdateTableSwitch(pvParameter);

			vTaskDelay(5000 / portTICK_PERIOD_MS);

		}


    vTaskDelete(NULL);

}
void BlynkInit(void)
{


	client = malloc(sizeof(blynk_client_t));

	blynk_init(client);

	blynk_options_t opt = {
	    		.server = BLYNK_SERVER
	   	};


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
