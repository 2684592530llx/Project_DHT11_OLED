#include "beep/bsp_beep.h"
#include "oled_dht11/app_oled_dht11.h"
#include "fonts/bsp_fonts.h"
#include "oled/bsp_oled.h"
#include "dht11/bsp_dht11.h"
#include "dwt/bsp_dwt.h"
#include "gpio/bsp_gpio.h"

void Beep_ReadTemp(void)
{
	HAL_GPIO_WritePin(Beep_PA6_GPIO_Port, Beep_PA6_Pin, GPIO_PIN_SET);
	OLED_Clear();//«Â∆¡
	OLED_ShowString_F8X16(1 , 2 , (uint8_t *)"High");
	OLED_ShowString_F8X16(2 , 2 , (uint8_t *)"temperture!!");
	DWT_DelayS(1);
	OLED_Clear();//«Â∆¡
}

