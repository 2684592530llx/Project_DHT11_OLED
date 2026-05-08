/**
  ******************************************************************************
  * @file       bsp_esp8266.c
  * @author     embedfire
  * @version     V1.0
  * @date        2025
  * @brief      esp8266 函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台  ：野火 STM32F103C8T6-STM32开发板 
  * 论坛      ：http://www.firebbs.cn
  * 官网      ：https://embedfire.com/
  * 淘宝      ：https://yehuosm.tmall.com/
  *
  ******************************************************************************
  */
  
#include "esp8266/bsp_esp8266.h"
#include "usart/bsp_usart.h"

/* 分别定义两个接收缓冲区 */
uint8_t Debug_RxBuff[UART_RX_BUFF_LEN];      // 上位机串口接收缓冲
uint8_t ESP8266_RxBuff[UART_RX_BUFF_LEN];    // ESP8266串口接收缓冲

/**
  * @brief  ESP8266 外设 GPIO 初始化函数
  * @note   配置 ESP8266 的 IO 和复位引脚为推挽输出，并设置默认输出状态
  * @param  无
  * @retval 无
  */
void ESP8266_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};    // GPIO 配置结构体初始化

  /* GPIO 端口时钟使能 */
  __HAL_RCC_GPIOB_CLK_ENABLE();              // 使能 GPIOB 时钟

  /* 配置 GPIO 输出电平为默认高电平 */
  HAL_GPIO_WritePin(GPIOB, ESP8266_IO_Pin|ESP8266_RST_Pin, GPIO_PIN_SET); // IO 和 RST 引脚置高

  /* 配置 GPIO 引脚模式 */
  GPIO_InitStruct.Pin = ESP8266_IO_Pin|ESP8266_RST_Pin; // 选择 ESP8266 IO 和 RST 引脚
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;           // 设置为推挽输出
  GPIO_InitStruct.Pull = GPIO_NOPULL;                  // 无上下拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;         // 低速模式
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);             // 初始化 GPIO 引脚
}

/**
  * @brief  ESP8266 硬件复位
  * @note   将 ESP8266 RST 引脚拉低一定时间再拉高，实现复位
  * @param  无
  * @retval 无
  */
void ESP8266_Reset(void)
{
    // 拉低复位引脚
    HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port, ESP8266_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);  // 保持低电平 100ms

    // 拉高复位引脚
    HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port, ESP8266_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(500);  // 等待模块启动完成
}


/**
  * @brief  启动 UART 接收
  * @note   配置 UART1 和 UART3 分别使用空闲中断和 DMA 模式接收数据
  * @param  无
  * @retval 无
  */
void UART_StartReceive(void)
{
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, Debug_RxBuff, UART_RX_BUFF_LEN);  // UART1 使用空闲中断模式接收数据到缓冲区 Debug_RxBuff
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ESP8266_RxBuff, UART_RX_BUFF_LEN); // UART3 使用 DMA 模式接收数据到缓冲区 ESP8266_RxBuff
}

/**
  * @brief  UART 空闲中断/ DMA 接收完成回调函数
  * @note   根据不同的 UART 实例处理接收到的数据
  *         - USART1: 上位机数据到达，转发到 ESP8266 并重新开启 IT 接收
  *         - USART3: ESP8266 数据到达，转发到上位机并重新开启 DMA 接收
  * @param  huart: UART 句柄指针
  * @param  Size: 接收到的数据长度
  * @retval 无
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance == USART1)  // 上位机数据到达
    {
        // 将上位机数据转发到 ESP8266
        HAL_UART_Transmit(&huart3, Debug_RxBuff, Size, HAL_MAX_DELAY);

        // 清空缓冲，防止残留
        memset(Debug_RxBuff, 0, UART_RX_BUFF_LEN);

        // 重新开启上位机 IT 接收
        HAL_UARTEx_ReceiveToIdle_IT(&huart1, Debug_RxBuff, UART_RX_BUFF_LEN);
    }
    else if(huart->Instance == USART3)  // ESP8266 数据到达
    {
				// 将ESP8266数据转发到 上位机
        HAL_UART_Transmit_IT(&huart1, ESP8266_RxBuff, Size);

        // 清空缓冲，防止残留
        memset(Debug_RxBuff, 0, UART_RX_BUFF_LEN);

        // 重新开启 ESP8266 DMA 接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ESP8266_RxBuff, UART_RX_BUFF_LEN);
    }
}

/**
  * @brief  UART 错误回调函数
  * @note   当 UART 出现错误时触发
  *         - USART3: 清除错误标志，并重新开启 DMA 接收
  * @param  huart: UART 句柄指针
  * @retval 无
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        // 清除帧错误标志
        __HAL_UART_CLEAR_FEFLAG(huart);

        // 重新启动 ESP8266 DMA 接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ESP8266_RxBuff, UART_RX_BUFF_LEN);
    }
}


/**
  * @brief  ESP8266 初始化函数
  * @note   初始化 ESP8266 的 GPIO、串口，并执行复位
  * @param  无
  * @retval 无
  */
void ESP8266_Init(void)
{
    // 复位 ESP8266 模块
    ESP8266_Reset();                        

    // 初始化 ESP8266 DMA 接收
    ESP8266_RX_DMA_Init();                                    

    // 初始化 ESP8266 串口
    ESP8266_UART_Init();                                      

    // 初始化 ESP8266 控制引脚
    ESP8266_GPIO_Init();                                      

		// 打印上电信息
    printf("欢迎使用野火小智开发板\n\n");                  
    printf("这是一个ESP8266AT指令测试实验\n\n");              
    printf("请使用串口调试助手发送\"AT+换行回车\"测试ESP8266是否准备好\n\n");

    // 启动 UART 接收任务
    UART_StartReceive();                                      
}



/*****************************END OF FILE***************************************/
