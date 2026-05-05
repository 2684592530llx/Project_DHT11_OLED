#ifndef __BSP_DHT11_H
#define __BSP_DHT11_H

#include "main.h"

#define DHT11_DATA_Pin			GPIO_PIN_12
#define DHT11_DATA_GPIO_Port	GPIOB

/* DHT11数据结构体 */
typedef struct
{                            
   uint8_t humi_int;        // 湿度整数部分
   uint8_t humi_deci;       // 湿度小数部分
   uint8_t temp_int;        // 温度整数部分
   uint8_t temp_deci;       // 温度小数部分
   uint8_t check_sum;       // 校验和                              
} DHT11_DATA_TYPEDEF;

void DHT11_GPIO_Init(void);
void DHT11_SetGPIOMode(uint32_t mode , uint32_t pull);
uint8_t DHT11_ReadByte(void);
HAL_StatusTypeDef DHT11_ReadData(DHT11_DATA_TYPEDEF *data);
	
#endif
