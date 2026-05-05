#include "main.h"
#include "gpio/bsp_gpio.h"
#include "dwt/bsp_dwt.h"
#include "usart/bsp_usart.h"
#include "i2c/bsp_i2c.h"
#include "oled/bsp_oled.h"
#include "fonts/bsp_fonts.h"
#include "oled_dht11/app_oled_dht11.h"
#include "light/bsp_light.h"
#include "light/bsp_adc.h"

void SystemClock_Config(void);


int main(void)
{

	HAL_Init();  						// 初始化 HAL 库 

	SystemClock_Config();   // 配置系统时钟，设置为 72MHz
	
	MX_USART1_UART_Init();	// 初始化串口，用于打印调试信息
	
	DWT_Init();							// 启动 DWT 计数器，用于精确测量程序运行时间

    MX_I2C1_Init();    			// 初始化 I2C1 总线，配置时钟、引脚等硬件资源

    OLED_Init();      			// 初始化 OLED 显示屏，发送初始化命令配置显示参数以及清屏、显示
	
    MX_GPIO_Init();				//初始化 Beep 和 光敏电阻的GPIO引脚
	
	MX_ADC1_Init();				//初始化 ADC ，用于计算光照强度
	
	HAL_ADCEx_Calibration_Start(&hadc1);//校准ADC
	
  while (1)
  {
	  Light_LEDR();
	  DHT11_ReadAndShow();
	  
  }
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {

    while(1);
  }


  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {

    while(1);
  }
}


#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{ 

  while (1)
  {
  }
}
#endif


