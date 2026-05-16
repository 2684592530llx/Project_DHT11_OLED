#ifndef __BSP_MQTT_H
#define __BSP_MQTT_H

#include "main.h"
#include <stdint.h>


/* MQTT 配置参数（根据您实际平台修改） */
#define MQTT_USER       	"AT+MQTTUSERCFG=0,1,\"DHT11\",\"ia7oX3UrRq\",\"version=2018-10-31&res=products%2Fia7oX3UrRq%2Fdevices%2FDHT11&et=1809748855&method=sha1&sign=k2KkpjREaoeoSFtBVNyJXUSVX6A%3D\",0,0,\"\"\r\n"
#define MQTT_CONN            "AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n"

#define MQTT_SUB_REPLY      "\"$sys/ia7oX3UrRq/DHT11/thing/property/post/reply\",0\r\n"
#define MQTT_SUB_SET     	"\"$sys/ia7oX3UrRq/DHT11/thing/property/set\",0\r\n"

#define MQTT_PUB_TOPIC		"$sys/ia7oX3UrRq/DHT11/thing/property/post"

#define MQTT_LINK_ID		0

/* 函数声明 */
uint8_t MQTT_SetUserConfig(void);
uint8_t MQTT_Connect(void);
uint8_t MQTT_ReplySubscribe(void);
uint8_t MQTT_SetSubscribe(void);
uint8_t MQTT_Upload_ConstData(float temp , float humi , int light_percent);
uint8_t MQTT_Upload_StatusData(int beep_on , int ledr_on);

#endif

