#include "light/bsp_light.h"
#include "gpio/bsp_gpio.h"
#include "light/bsp_adc.h"

void Light_LEDR(void)
{
	if(HAL_GPIO_ReadPin(GPIOA ,Light_Do_PA11_Pin) == GPIO_PIN_RESET)	
	{
		HAL_GPIO_WritePin(Light_LED_R_PB5_GPIO_Port, Light_LED_R_PB5_Pin, GPIO_PIN_SET);	//PA11为低电平，那么PB5输出高电平，熄灭LEDR
	}
	else if(HAL_GPIO_ReadPin(GPIOA ,Light_Do_PA11_Pin) == GPIO_PIN_SET)	
	{
		HAL_GPIO_WritePin(Light_LED_R_PB5_GPIO_Port, Light_LED_R_PB5_Pin, GPIO_PIN_RESET);	//PA11为高电平，那么PB5输出低电平，点亮LEDR
	}
}

uint8_t Get_LightPercent(void)
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
    float percent = (1.0f - (float)adc_avg / 4095.0f) * 100.0f;
    return (uint8_t)percent;
}



