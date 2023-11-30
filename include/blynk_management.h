/*
 * blynk_management.h
 *
 *  Created on: 24 авг. 2023 г.
 *      Author: boyarkin.k
 */

#ifndef INCLUDE_BLYNK_MANAGEMENT_H_
#define INCLUDE_BLYNK_MANAGEMENT_H_

#include "blynk.h"

#define BLYNK_TOKEN 		"F5Bw_ABp5oD9uR655Je35xm6lDE4CbCu"
#define BLYNK_TOKEN_TEST 	"REv3lP7ai0jixCTWq62xl3PwhG6TVWFI"
#define BLYNK_SERVER "109.194.141.27"

#ifdef __cplusplus
extern "C" {
#endif

#define COUNT_SWITCH      5

enum {
	VP_COUNTER = 0,
	VP_RESTART_CMD=1,
	VP_RESTART_REASON=2,
	VP_VERSION=3,
	VP_SWITCH_1=4,
	VP_SWITCH_2=5,
	VP_SWITCH_3=6,
	VP_SWITCH_4=7,
	VP_SWITCH_5=8,
	VP_LED_SWITCH_1=9,
	VP_LED_SWITCH_2=10,
	VP_LED_SWITCH_3=11,
	VP_LED_SWITCH_4=12,
	VP_LED_SWITCH_5=13,
	VP_LED_MODE_SWITCH_1=14,
	VP_LED_MODE_SWITCH_2=15,
	VP_LED_MODE_SWITCH_3=16,
	VP_LED_MODE_SWITCH_4=17,
	VP_LED_MODE_SWITCH_5=18,
	VP_MODE_SWITCH_1=19,
	VP_MODE_SWITCH_2=20,
	VP_MODE_SWITCH_3=21,
	VP_MODE_SWITCH_4=22,
	VP_MODE_SWITCH_5=23,


	VP_SENSOR_1_COUNTER=34,
	VP_SENSOR_1_TEMPERATURE=35,
	VP_SENSOR_1_HUMIDITY=36,
	VP_SENSOR_2_COUNTER=37,
	VP_SENSOR_2_TEMPERATURE=38,
	VP_SENSOR_2_HUMIDITY=39,
	VP_SENSOR_3_COUNTER=40,
	VP_SENSOR_3_TEMPERATURE=41,
	VP_SENSOR_3_HUMIDITY=42,
	VP_SENSOR_4_COUNTER=43,
	VP_SENSOR_4_TEMPERATURE=44,
	VP_SENSOR_4_HUMIDITY=45,
	VP_SENSOR_5_COUNTER=46,
	VP_SENSOR_5_TEMPERATURE=47,
	VP_SENSOR_5_HUMIDITY=48,

	VP_TABLE_TEST=49,


	VP_SWITCH_1_TEMPERATURE_SOURCE=70,
	VP_SWITCH_2_TEMPERATURE_SOURCE=71,
	VP_SWITCH_3_TEMPERATURE_SOURCE=72,
	VP_SWITCH_4_TEMPERATURE_SOURCE=73,
	VP_SWITCH_5_TEMPERATURE_SOURCE=74,

	VP_SWITCH_1_TEMPERATURE_LOW=75,
	VP_SWITCH_2_TEMPERATURE_LOW=76,
	VP_SWITCH_3_TEMPERATURE_LOW=77,
	VP_SWITCH_4_TEMPERATURE_LOW=78,
	VP_SWITCH_5_TEMPERATURE_LOW=79,

	VP_SWITCH_1_TEMPERATURE_HIGH=80,
	VP_SWITCH_2_TEMPERATURE_HIGH=81,
	VP_SWITCH_3_TEMPERATURE_HIGH=82,
	VP_SWITCH_4_TEMPERATURE_HIGH=83,
	VP_SWITCH_5_TEMPERATURE_HIGH=84,



	VP_CURRENT_DATE_TIME=100,
	VP_TABLE_SWITCH_STAT=101,
	VP_READ_CORE_BUTTON=102,



};


void Blynk_Timer(void *pvParameter);
void BlynkInit(void);
void state_handler(blynk_client_t *c, const blynk_state_evt_t *ev, void *data);
void vw_handler(blynk_client_t *c, uint16_t id, const char *cmd, int argc, char **argv, void *data);
void vr_handler(blynk_client_t *c, uint16_t id, const char *cmd, int argc, char **argv, void *data);
void UpdateTableSensor(void *pvParameter);
void UpdateTableSwitch(void *pvParameter);
void UpdateTableSwitchRow(void *pvParameter,uint8_t id,bool afterstart);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_BLYNK_MANAGEMENT_H_ */
