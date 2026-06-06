#include "oled_dht11/app_oled_dht11.h"
#include "dht11/bsp_dht11.h" //需要用到dht11/bsp_dht11.h中的结构体
#include "fonts/bsp_fonts.h"
#include "oled/bsp_oled.h"
#include "dht11/bsp_dht11.h"
#include "dwt/bsp_dwt.h"
#include "gpio/bsp_gpio.h"
#include "light/bsp_light.h"
#include "esp8266/bsp_esp8266.h"
#include "mqtt/bsp_mqtt.h"

DHT11_DATA_TYPEDEF dht11_data= {0};

void DHT11_ReadAndShow(void)
{
	if(DHT11_ReadData(&dht11_data) == HAL_OK)
	{
		float temperature = dht11_data.temp_int + dht11_data.temp_deci / 10.0f;
		
		/* 蜂鸣器控制：手动模式优先，否则自动根据温度判断 */
		if(mqtt_manual_beep)
		{
			/* 手动模式：按照 OneNET 命令下行控制蜂鸣器，跳过温度判断 */
			if(mqtt_beep_state)
			{
				HAL_GPIO_WritePin(Beep_PA6_GPIO_Port, Beep_PA6_Pin, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(Beep_PA6_GPIO_Port, Beep_PA6_Pin, GPIO_PIN_RESET);
			}
		}
		else
		{
			/* 自动模式：根据温度阈值自动控制蜂鸣器 */
			if(temperature > HUMI_MAX)
			{
				Beep_ReadTemp();  // 蜂鸣器报警
			}
			else
			{
				HAL_GPIO_WritePin(Beep_PA6_GPIO_Port, Beep_PA6_Pin, GPIO_PIN_RESET);  // 关闭蜂鸣器
			}
		}
		
		// OLED显示（无论是否报警都显示）
		Show_Chinese();
		
		char temp_data[8];
		sprintf(temp_data , "%.2d.%.1dC" , dht11_data.temp_int , dht11_data.temp_deci);
		OLED_ShowString_F8X16(1 , 5 , (uint8_t *)(temp_data));
		
		char humi_data[8];
		sprintf(humi_data , "%.2d.%.1d%%" , dht11_data.humi_int , dht11_data.humi_deci);
		OLED_ShowString_F8X16(2 , 5 , (uint8_t *)(humi_data));
		
		char light_data[8];
		uint8_t light_int = Get_LightPercent();   
		sprintf(light_data, "%d%%", light_int);
		OLED_ShowString_F8X16(3, 9, (uint8_t*)light_data);
		
		// MQTT上传数据（无论是否报警都上传）
		float temp = dht11_data.temp_int + dht11_data.temp_deci / 10.0f;
		float humi = dht11_data.humi_int + dht11_data.humi_deci / 10.0f;
		int   light = light_int;
		MQTT_Upload_ConstData(temp, humi, light);
		
		// 读取并上传设备状态
		uint8_t beep_state = HAL_GPIO_ReadPin(Beep_PA6_GPIO_Port, Beep_PA6_Pin);
		uint8_t beep_on = (beep_state == GPIO_PIN_SET) ? 1 : 0;
		
		uint8_t ledr_state = HAL_GPIO_ReadPin(Light_LED_R_PB5_GPIO_Port, Light_LED_R_PB5_Pin);
		uint8_t ledr_on = (ledr_state == GPIO_PIN_RESET) ? 1 : 0;
		
		MQTT_Upload_StatusData(beep_on, ledr_on);
		
		/* 把长延时拆开，保证等待期间也能及时处理平台下行命令 */
		for(uint8_t i = 0; i < 40; i++)
		{
			MQTT_ProcessCommand();
			HAL_Delay(100);
		}
		OLED_Clear();
	}
	else
	{
		OLED_ShowString_F8X16(0 , 0 , (uint8_t *)"DHT11");
		OLED_ShowString_F8X16(1 , 0 , (uint8_t *)"ERROR!!");
	}
}

/**
  * @brief  ESP8266 任务函数
  * @note   完成 ESP8266 上电初始化、模式设置、Wi-Fi 连接、TCP 连接及透传模式配置
  * @retval 无
  */
void ESP8266_Task(void)
{
    ESP8266_Init();                                    // 初始化 ESP8266 模块及相关外设
	OLED_ShowString_F8X16(2 , 2 , (uint8_t *)"ESP8266_Init");
	DWT_DelayS(1);
	OLED_Clear();
    printf(">>> ESP8266 已初始化\r\n");

    // 软件复位 ESP8266，并等待 ready
    printf(">>> ESP8266 软件复位中...\r\n");
    if(ESP8266_ResetWait())       // 软件复位并等待 ready
	{
		printf(">>> ESP8266 软件复位完成，ready 已收到\r\n");
		OLED_ShowString_F8X16(1 , 5 , (uint8_t *)"AT+RST");
		OLED_ShowString_F8X16(2 , 7 , (uint8_t *)"OK!");
	    DWT_DelayS(1);
	    OLED_Clear();
	}
    else
    {
        printf(">>> ESP8266 复位超时!\r\n");
        return;                              // 复位失败则退出任务
    }
	
    // 测试 ESP8266 模块是否可用
    if(ESP8266_Test())
	{
		printf(">>> ESP8266 模块测试通过\r\n");
		OLED_ShowString_F8X16(1 , 7 , (uint8_t *)"AT");
		OLED_ShowString_F8X16(2 , 7 , (uint8_t *)"OK!");
		DWT_DelayS(1);
	    OLED_Clear();
	}
    else
    {
        printf(">>> ESP8266 模块不可用!\r\n");
        return;                                       // 模块不可用则退出任务
    }

    // 设置工作模式为 STA
    if(ESP8266_SetMode(STA))
	{
		printf(">>> ESP8266 已设置为 STA 模式\r\n");
		OLED_ShowString_F8X16(1 , 5 , (uint8_t *)"AT+CWMODE");
		OLED_ShowString_F8X16(2 , 7 , (uint8_t *)"OK!");
		DWT_DelayS(1);
		OLED_Clear();
	}
    else
    {
        printf(">>> 设置工作模式失败!\r\n");
        return;                                       // 设置失败则退出任务
    }
	
    // 连接指定 Wi-Fi
    if(!ESP8266_ConnectWiFi(ApSsid, ApPwd))
	{
		printf(">>> Wi-Fi 已连接\r\n");
		OLED_ShowString_F8X16(1 , 5 , (uint8_t *)"AT+CWJAP");
		OLED_ShowString_F8X16(2 , 7 , (uint8_t *)"OK!");
		DWT_DelayS(1);
		OLED_Clear();
	}     
    else
	{
        printf(">>> Wi-Fi 连接失败! \r\n");
		return; 
	}

}

void MQTT_Task(void)
{
	/* EMQX Cloud：三步连接 — 配置用户 → 连接 Broker → 订阅命令主题 */
	MQTT_SetUserConfig();
	HAL_Delay(200);

	MQTT_Connect();
	HAL_Delay(200);

	MQTT_CommandSubscribe();
	HAL_Delay(200);
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

/**
  * @brief  LEDR 控制（手动/自动模式切换）
  * @note   手动模式下，根据 OneNET 命令下行设置 LED 状态
  *         自动模式下，调用 Light_LEDR() 根据光照自动控制
  *         PB5 引脚低电平点亮, 高电平熄灭
  * @retval 无
  */
void LEDR_Control(void)
{
	if(mqtt_manual_ledr)
	{
		/* 手动模式：按照 OneNET 命令下行控制 LED */
		/* mqtt_ledr_state 已在 MQTT_ProcessCommand() 中用于设置 GPIO，无需重复操作 */
	}
	else
	{
		/* 自动模式：根据光照强度自动控制 LED */
		Light_LEDR();
	}
}
