#ifndef __BSP_ESP8266_H
#define __BSP_ESP8266_H

#include "main.h"


/******************************* ESP8266 数据类型定义 ***************************/

/**
  * @brief  ESP8266 网络工作模式枚举类型
  */
typedef enum{
    STA,        // 站点模式（Station），ESP8266 作为客户端连接 Wi-Fi
    AP,         // 接入点模式（Access Point），ESP8266 作为热点
    STA_AP      // 站点+接入点模式，同时作为客户端和热点
} ENUM_Net_ModeTypeDef;

extern volatile bool ESP_RX_Complete;
extern volatile bool ESP_TX_Complete;

extern uint8_t ESP8266_RxBuff[];


/******* ESP8266 控制引脚用户宏定义 *******/
#define ESP8266_IO_Pin       	GPIO_PIN_8
#define ESP8266_IO_GPIO_Port 	GPIOB
#define ESP8266_RST_Pin      	GPIO_PIN_9
#define ESP8266_RST_GPIO_Port GPIOB

/* 定义缓冲区长度 */
#define UART_RX_BUFF_LEN 512

/************ 函数声明 ************/
void ESP8266_GPIO_Init(void);
void ESP8266_Reset(void);
void UART_StartReceive(void);
void ESP8266_Init(void);
void ESP8266_SendString(char *str);
uint8_t ESP8266_Cmd(const char *cmd);
uint8_t ESP8266_Test(void);
uint8_t ESP8266_ResetWait(void);
uint8_t ESP8266_SetMode(ENUM_Net_ModeTypeDef mode);
uint8_t ESP8266_ConnectWiFi(const char *ssid, const char *password);

#endif /* __BSP_ESP8266_H  */
