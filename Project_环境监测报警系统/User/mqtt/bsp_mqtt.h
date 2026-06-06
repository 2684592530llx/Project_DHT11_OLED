#ifndef __BSP_MQTT_H
#define __BSP_MQTT_H

#include "main.h"
#include <stdint.h>

/* EMQX 公共 Broker 配置 */
#define MQTT_BROKER_HOST    "broker.emqx.io"
#define MQTT_BROKER_PORT    1883
#define MQTT_CLIENT_ID      "STM32_DHT11"

/* MQTT 主题定义 */
#define MQTT_PUB_TOPIC      "STM32/data"
#define MQTT_SUB_TOPIC      "STM32/command"

/* 命令下行：手动控制标志位
 * mqtt_manual_beep=0 自动模式(温度控制蜂鸣器)    =1 手动模式(命令下行控制)
 * mqtt_manual_ledr=0 自动模式(光照控制LED)       =1 手动模式(命令下行控制)
 * 复位后默认为 0（自动模式）
 */
extern volatile uint8_t mqtt_manual_beep;
extern volatile uint8_t mqtt_beep_state;
extern volatile uint8_t mqtt_manual_ledr;
extern volatile uint8_t mqtt_ledr_state;

/* 函数声明 */
uint8_t MQTT_SetUserConfig(void);
uint8_t MQTT_Connect(void);
uint8_t MQTT_CommandSubscribe(void);
uint8_t MQTT_Upload_ConstData(float temp, float humi, int light_percent);
uint8_t MQTT_Upload_StatusData(int beep_on, int ledr_on);
uint8_t MQTT_ProcessCommand(void);

#endif
