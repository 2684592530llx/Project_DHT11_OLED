#include "oled_dht11/app_oled_dht11.h"
#include "dht11/bsp_dht11.h" //需要用到dht11/bsp_dht11.h中的结构体
#include "fonts/bsp_fonts.h"
#include "oled/bsp_oled.h"
#include "dht11/bsp_dht11.h"
#include "dwt/bsp_dwt.h"
#include "gpio/bsp_gpio.h"
#include "light/bsp_light.h"


static DHT11_DATA_TYPEDEF dht11_data= {0};

void DHT11_ReadAndShow(void)
{
	if(DHT11_ReadData(&dht11_data) == HAL_OK)
	{
		float temperature = dht11_data.temp_int + dht11_data.temp_deci / 10.0f;//比较整数部分+小数是否达到阈值
		//除了温度，还可以对湿度也进行阈值设置
		if(temperature> HUMI_MAX)//自己设置一个HUMI_MAX温度的阈值，超过这个温度就报警
		{
			Beep_ReadTemp();
		}
		else
		{
			HAL_GPIO_WritePin(Beep_PA6_GPIO_Port, Beep_PA6_Pin, GPIO_PIN_RESET);//将PA6置低电平,关闭蜂鸣器
			Show_Chinese();

			char temp_data[8];
			sprintf(temp_data , "%.2d.%.1dC" , dht11_data.temp_int , dht11_data.temp_deci);
			OLED_ShowString_F8X16(1 , 5 , (uint8_t *)(temp_data));
			
			char humi_data[8];
			sprintf(humi_data , "%.2d.%.1d%%" , dht11_data.humi_int , dht11_data.humi_deci);
			OLED_ShowString_F8X16(2 , 5 , (uint8_t *)(humi_data));
			
			uint16_t adc = Get_ADC_Average(10);
			uint16_t voltage_mv = (uint16_t)((adc * 3300) / 4095);  // 毫伏
			char light_data[8];
			sprintf(light_data, "%2d.%01dV", voltage_mv/1000, voltage_mv%1000);
			OLED_ShowString_F8X16(3, 9, (uint8_t*)light_data);
			
			DWT_DelayS(1);
			OLED_Clear();
		}
	}
	else
	{
		OLED_ShowString_F8X16(0 , 0 , (uint8_t *)"DHT11");
		OLED_ShowString_F8X16(1 , 0 , (uint8_t *)"ERROR!!");
	}
}


void Show_Chinese(void)
{
	//显示汉字
	OLED_ShowChinese_F16X16(0 , 0 , 0);
	OLED_ShowChinese_F16X16(0 , 1 , 1);
	OLED_ShowChinese_F16X16(0 , 2 , 2);
	OLED_ShowChinese_F16X16(0 , 3 , 3);
	OLED_ShowChinese_F16X16(0 , 4 , 4);
	OLED_ShowChinese_F16X16(0 , 5 , 5);
	OLED_ShowChinese_F16X16(0 , 6 , 6);
	OLED_ShowChinese_F16X16(0 , 7 , 7);
	
	OLED_ShowChinese_F16X16(1 , 0 , 8);
	OLED_ShowChinese_F16X16(1 , 1 , 10);//温度
	OLED_ShowString_F8X16(1, 4, (uint8_t *)":");  // 显示冒号“:”
	
	OLED_ShowChinese_F16X16(2 , 0 , 9);
	OLED_ShowChinese_F16X16(2 , 1 , 10);//湿度
	OLED_ShowString_F8X16(2, 4, (uint8_t *)":");  // 显示冒号“:”
	
	OLED_ShowChinese_F16X16(3 , 0 , 11);
	OLED_ShowChinese_F16X16(3 , 1 , 12);
	OLED_ShowChinese_F16X16(3 , 2 , 13);
	OLED_ShowChinese_F16X16(3 , 3 , 14);//光照强度
	OLED_ShowString_F8X16(3, 8, (uint8_t *)":");  // 显示冒号“:”
}
