#include "dht11/bsp_dht11.h"
#include "dwt/bsp_dwt.h"

void DHT11_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LED_R_Pin LED_G_Pin LED_B_Pin */
  GPIO_InitStruct.Pin = DHT11_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);

}

void DHT11_SetGPIOMode(uint32_t mode , uint32_t pull)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT11_DATA_Pin;
  GPIO_InitStruct.Mode = mode;
  GPIO_InitStruct.Pull = pull;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);
}

uint8_t DHT11_ReadByte(void)
{
	uint8_t value = 0;
	
	for(uint8_t i = 0 ; i < 8 ; i++)
	{
		 //前面低电平我们不需要判断，跳过这部分低电平
		while(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port , DHT11_DATA_Pin) == RESET);
	    DWT_DelayUs(40);//等待40us后，看是高电平还是低电平
		
		if(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port , DHT11_DATA_Pin) == SET)
		{
			value |= (1 << (7 - i));//对应为设置为1
			while(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port , DHT11_DATA_Pin) == SET);
		}
		//低电平的话，对应位设置为0，直接继续下一位
	}
	return value;
}

HAL_StatusTypeDef DHT11_ReadData(DHT11_DATA_TYPEDEF *data)
{
	uint8_t retry = 0;
	DHT11_SetGPIOMode(GPIO_MODE_OUTPUT_PP , GPIO_NOPULL);//推挽输出
	HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port , DHT11_DATA_Pin , GPIO_PIN_RESET);//输出状态拉低数据线
	DWT_DelayMs(20);
	HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port , DHT11_DATA_Pin , GPIO_PIN_SET);
	DWT_DelayUs(30);
	
	DHT11_SetGPIOMode(GPIO_MODE_AF_INPUT , GPIO_PULLUP);// 设置引脚为输入，等待DHT11响应信号
	
	retry = 0;
		while(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port , DHT11_DATA_Pin) == RESET)
	{
		if(++retry > 100) return HAL_ERROR;
		DWT_DelayUs(1);
	}
	retry = 0;
		while(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port , DHT11_DATA_Pin) == SET)
	{
		if(++retry > 100) return HAL_ERROR;
		DWT_DelayUs(1);
	}
	retry = 0;
		while(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port , DHT11_DATA_Pin) == RESET)
	{
		if(++retry > 100) return HAL_ERROR;
		DWT_DelayUs(1);
	}
	data->humi_int = DHT11_ReadByte();
	data->humi_deci = DHT11_ReadByte();
	data->temp_int = DHT11_ReadByte();
	data->temp_deci = DHT11_ReadByte();
	data->check_sum = DHT11_ReadByte();
	
	uint8_t sum = data->humi_int + data->humi_deci + data->temp_int +data->temp_deci;
	return (data->check_sum == sum) ? HAL_OK : HAL_ERROR;
}
