#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "main.h"

void MX_GPIO_Init(void);
	
/*定义LED灯的引脚*/
#define R_LED_Pin		GPIO_PIN_1
#define G_LED_Pin		GPIO_PIN_2
#define B_LED_Pin		GPIO_PIN_3
#define LED_Port		GPIOA

/*点亮LED灯*/
#define R_LED_ON()		HAL_GPIO_WritePin(LED_Port , R_LED_Pin , GPIO_PIN_RESET)
#define G_LED_ON()		HAL_GPIO_WritePin(LED_Port , G_LED_Pin , GPIO_PIN_RESET)
#define B_LED_ON()		HAL_GPIO_WritePin(LED_Port , B_LED_Pin , GPIO_PIN_RESET)

/*关闭LED灯*/
#define R_LED_OFF()		HAL_GPIO_WritePin(LED_Port , R_LED_Pin , GPIO_PIN_SET)
#define G_LED_OFF()		HAL_GPIO_WritePin(LED_Port , G_LED_Pin , GPIO_PIN_SET)
#define B_LED_OFF()		HAL_GPIO_WritePin(LED_Port , B_LED_Pin , GPIO_PIN_SET)

/*翻转LED灯*/
#define R_LED_TOGGLE()	HAL_GPIO_TogglePin(LED_Port , R_LED_Pin);
#define G_LED_TOGGLE()	HAL_GPIO_TogglePin(LED_Port , G_LED_Pin);
#define B_LED_TOGGLE()	HAL_GPIO_TogglePin(LED_Port , B_LED_Pin);

/*点亮三个LED灯*/
#define RGB_ALL_ON()		R_LED_ON(); G_LED_ON(); B_LED_ON()

/*关闭三个LED灯*/
#define RGB_ALL_OFF()		R_LED_OFF(); G_LED_OFF(); B_LED_OFF()

/*仅点亮其中一个LED灯*/
#define R_LED_ON_ONLY()		R_LED_ON(); G_LED_OFF(); B_LED_OFF()
#define G_LED_ON_ONLY()		R_LED_OFF(); G_LED_ON(); B_LED_OFF()
#define B_LED_ON_ONLY()		R_LED_OFF(); G_LED_OFF(); B_LED_ON()

void MX_GPIO_Init(void);




#endif /*__ BSP_LED_H__ */

