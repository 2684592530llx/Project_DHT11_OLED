#ifndef __APP_OLED_DHT11_H
#define __APP_OLED_DHT11_H

#include "main.h"
#include <stdio.h>

void DHT11_ReadAndShow(void);
void Beep_ReadTemp(void);
void Show_Chinese(void);
void ESP8266_Task(void);
void MQTT_Task(void);

#define      ApSsid        "siyuan"         		//要连接的热点的名称
#define      ApPwd         "88888888"           	//要连接的热点的密钥

#define	HUMI_MAX	28


#endif
