/*
 * ntp.h
 *
 *  Created on: 21 нояб. 2023 г.
 *      Author: boyarkin.k
 */

#ifndef INCLUDE_NTP_H_
#define INCLUDE_NTP_H_


#define CONFIG_SNTP_TIME_SERVER "time.windows.com"

void task_ntp(void *arg);
void obtain_time(void);
void initialize_sntp(void);
void ntp_start(void);


#endif /* INCLUDE_NTP_H_ */
