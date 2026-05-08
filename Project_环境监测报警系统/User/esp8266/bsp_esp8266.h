#ifndef __BSP_ESP8266_H
#define __BSP_ESP8266_H

#include "main.h"

/******* ESP8266 控制引脚用户宏定义 *******/
#define ESP8266_IO_Pin       	GPIO_PIN_8
#define ESP8266_IO_GPIO_Port 	GPIOB
#define ESP8266_RST_Pin      	GPIO_PIN_9
#define ESP8266_RST_GPIO_Port GPIOB

/* 定义缓冲区长度 */
#define UART_RX_BUFF_LEN 1024

/************ 函数声明 ************/
void ESP8266_GPIO_Init(void);
void ESP8266_Reset(void);
void UART_StartReceive(void);
void ESP8266_Init(void);

#endif /* __BSP_ESP8266_H  */
