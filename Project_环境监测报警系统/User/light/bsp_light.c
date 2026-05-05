#include "light/bsp_light.h"
#include "gpio/bsp_gpio.h"
#include "light/bsp_adc.h"

void Light_LEDR(void)
{
	if(HAL_GPIO_ReadPin(GPIOA ,Light_Do_PA11_Pin) == GPIO_PIN_RESET)	//光照强度足够的情况
	{
		HAL_GPIO_WritePin(Light_LED_R_PB5_GPIO_Port, Light_LED_R_PB5_Pin, GPIO_PIN_SET);	//LED灯灭
	}
	else if(HAL_GPIO_ReadPin(GPIOA ,Light_Do_PA11_Pin) == GPIO_PIN_SET)	//光照强度不够的情况
	{
		HAL_GPIO_WritePin(Light_LED_R_PB5_GPIO_Port, Light_LED_R_PB5_Pin, GPIO_PIN_RESET);	//LED灯亮
	}
}

uint16_t Get_ADC_Average(uint8_t times) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            sum += HAL_ADC_GetValue(&hadc1);
        }
    }
    return (uint16_t)(sum / times);
}


