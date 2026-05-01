#include "oled_dht11/app_oled_dht11.h"
#include "dht11/bsp_dht11.h" //需要用到dht11/bsp_dht11.h中的结构体
#include "fonts/bsp_fonts.h"
#include "oled/bsp_oled.h"
#include "dht11/bsp_dht11.h"
#include "dwt/bsp_dwt.h"

static DHT11_DATA_TYPEDEF dht11_data= {0};

void DHT11_ReadAndShow(void)
{
	//显示汉字
	OLED_ShowChinese_F16X16(0 , 2 , 0);
	OLED_ShowChinese_F16X16(0 , 3 , 1);
	OLED_ShowChinese_F16X16(0 , 4 , 2);
	if(DHT11_ReadData(&dht11_data) == HAL_OK)
	{	
		OLED_ShowChinese_F16X16(1 , 0 , 3);
		OLED_ShowChinese_F16X16(1 , 1 , 4);//温度
		OLED_ShowString_F8X16(1, 4, (uint8_t *)":");  // 显示冒号“:”
		char temp_data[6];
		sprintf(temp_data , "%.2d.%.1d C" , dht11_data.temp_int , dht11_data.temp_deci);
		OLED_ShowString_F8X16(1 , 5 , (uint8_t *)(temp_data));
		
		OLED_ShowChinese_F16X16(2 , 0 , 5);
		OLED_ShowChinese_F16X16(2 , 1 , 6);//湿度
		OLED_ShowString_F8X16(2, 4, (uint8_t *)":");  // 显示冒号“:”
		char humi_data[6];
		sprintf(humi_data , "%.2d.%.1d C" , dht11_data.humi_int , dht11_data.humi_deci);
		OLED_ShowString_F8X16(2 , 5 , (uint8_t *)(humi_data));
		
		DWT_DelayS(2);//每两秒读一次数据
		OLED_Clear();//清屏等待下一次刷新
	}
	else
	{
		OLED_ShowString_F8X16(0 , 0 , (uint8_t *)"DHT11");
		OLED_ShowString_F8X16(1 , 0 , (uint8_t *)"ERROR!!");
	}
}
