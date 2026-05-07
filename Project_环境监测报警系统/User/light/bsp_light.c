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

float Get_LightPercent(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++)
    {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        {
            sum += HAL_ADC_GetValue(&hadc1);
        }
        HAL_Delay(5);
    }
	uint16_t adc_avg = sum / 10;
    float percent = (1.0f - (float)adc_avg / 4095.0f) * 100.0f;//将 ADC 值取反
    return percent;
}



