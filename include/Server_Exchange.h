/*
 * Server_Exchange.h
 *
 *  Created on: 7 сент. 2023 г.
 *      Author: boyarkin.k
 */

#ifndef INCLUDE_SERVER_EXCHANGE_H_
#define INCLUDE_SERVER_EXCHANGE_H_

#define LIMIT_ACHIEVE_BIT BIT0

bool SocketInit(void);
void Server_Exchange(void *pvParameter);
void Server_Receive(void *pvParameter);
void Server_Save_Data(void *pvParameter);
void Server_Save_Data_Power(void *pvParameter);
void Server_Send_Data_Core_Dump(uint8_t *pData,uint16_t length);






#endif /* INCLUDE_SERVER_EXCHANGE_H_ */
