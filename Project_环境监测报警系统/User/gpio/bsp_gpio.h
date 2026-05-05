#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "main.h"


void MX_GPIO_Init(void);

#define Beep_PA6_GPIO_Port    		GPIOA
#define Beep_PA6_Pin          		GPIO_PIN_6

#define Light_LED_R_PB5_GPIO_Port 	GPIOB
#define Light_LED_R_PB5_Pin       	GPIO_PIN_5
#define Light_Do_PA11_GPIO_Port   	GPIOA
#define Light_Do_PA11_Pin         	GPIO_PIN_11

#endif /*__ GPIO_H__ */

