/*
 * main.h
 *
 *  Created on: 25 сент. 2023 г.
 *      Author: boyarkin.k
 */

#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

#include "driver/gpio.h"


#ifdef __cplusplus
extern "C" {
#endif

		void SetSwitchStateAfterStart(void);
		void ChangeSwitchState(uint8_t pin);
		uint8_t GetSwitchState(uint8_t pin);
		void ChangeSwitchMode(uint8_t pin);
		uint8_t GetSwitchMode(uint8_t pin);
		uint8_t GetSwitchPin(uint8_t pin);
		uint8_t GetSwitchIndexPin(gpio_num_t gpio_num);
		void SendCoreDump(void);


#ifdef __cplusplus
}
#endif

#define GPIO_OUTPUT_IO_0    GPIO_NUM_13
#define GPIO_OUTPUT_IO_1    GPIO_NUM_12
#define GPIO_OUTPUT_IO_2    GPIO_NUM_14
#define GPIO_OUTPUT_IO_3    GPIO_NUM_27
#define GPIO_OUTPUT_IO_4    GPIO_NUM_26

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1) | (1ULL<<GPIO_OUTPUT_IO_2) | (1ULL<<GPIO_OUTPUT_IO_3) | (1ULL<<GPIO_OUTPUT_IO_4))


#define GPIO_INPUT_TEST_MODE_IO_1    GPIO_NUM_25

#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_TEST_MODE_IO_1)

int my_log(const char *, va_list);
void Init(void);
void Read_Data_NVS(void);
void Save_Data_NVS_Pin(uint8_t pin);
void Timer_Switch_State(void *pvParameter);
void SetSwitchState(gpio_num_t gpio_num, uint8_t state);


typedef struct
{
	time_t lastOn;
	time_t DurationOn;

} SSwitchStat;



#endif /* INCLUDE_MAIN_H_ */
