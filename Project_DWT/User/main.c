#include "main.h"
#include "led/bsp_led.h"
#include "dwt/bsp_dwt.h"

uint32_t begin = 0;
uint32_t end = 0;
uint32_t duration = 0;
uint32_t task_us = 0;

void SystemClock_Config(void);


int main(void)
{


  HAL_Init();

  SystemClock_Config();
	
  DWT_Init();

  MX_GPIO_Init();

  while (1)
  {
	  begin = DWT_GetTick();
	  RGB_ALL_ON();
	  DWT_DelayMs(1000);
	  RGB_ALL_OFF();
	  DWT_DelayMs(1000);
	  
	  end = DWT_GetTick();
	  duration = begin - end;
	  
	  task_us = DWT_TickToMs(duration , SystemCoreClock);
  }
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


