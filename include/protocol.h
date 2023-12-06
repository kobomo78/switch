/*
 * protocol.h
 *
 *  Created on: 2 окт. 2023 г.
 *      Author: boyarkin.k
 */

#ifndef INCLUDE_PROTOCOL_H_
#define INCLUDE_PROTOCOL_H_

#define TYPE_GET_DATA    1
#define TYPE_DATA        2

#define PASSPORT_POWER_SWITCH    2100
#define TARIF_1_KWT    			 4.24

enum eMode
{
	HANDS=0,
	AUTO=1
};


typedef struct SSensorData
{
	uint8_t  type;
	uint8_t  sensor_addr;
	float 	 temperature;
	float 	 humidity;

} SSensorData;

#define SENSOR_COUNT       5

typedef struct
{
	uint16_t counter;
	uint8_t  timer_no_data;
	SSensorData SensorData;

} SSensorInfo;


#endif /* INCLUDE_PROTOCOL_H_ */
