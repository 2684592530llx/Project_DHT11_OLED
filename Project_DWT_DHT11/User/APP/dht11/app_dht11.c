#include "dht11/app_dht11.h"
#include "dht11/bsp_dht11.h" //需要用到dht11/bsp_dht11.h中的结构体

static DHT11_DATA_TYPEDEF dht11_data= {0};

void DHT11_ReadAndPrint(void)
{
	if(DHT11_ReadData(&dht11_data) == HAL_OK)
	{
		printf("当前数据传输校验正确：");
		
		if(dht11_data.humi_deci & 0x80)//判断是否为负数
		{
			printf("湿度：-%d.%d \n", dht11_data.humi_int , dht11_data.humi_deci);
		}
		else
		{
			printf("湿度：%d.%d \n", dht11_data.humi_int , dht11_data.humi_deci);
		}
		if(dht11_data.temp_deci & 0x80)//判断是否为负数
		{
			printf("湿度：-%d.%d \n", dht11_data.temp_int , dht11_data.temp_deci);
		}
		else
		{
			printf("湿度：%d.%d \n", dht11_data.temp_int , dht11_data.temp_deci);
		}
	}
	else
	{
		printf("当前数据传输错误\n");
	}
}
