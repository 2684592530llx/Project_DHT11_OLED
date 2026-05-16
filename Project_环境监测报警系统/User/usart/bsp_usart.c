#include "usart/bsp_usart.h"

UART_HandleTypeDef huart1;            // 调试串口 USART1 句柄
UART_HandleTypeDef huart3;            // ESP8266 串口 USART3 句柄
DMA_HandleTypeDef hdma_usart3_rx;     // USART3 接收 DMA 句柄

/**
  * @brief  ESP8266 RX DMA 初始化
  * @note   配置 DMA1 用于 USART3 接收数据
  * @param  无
  * @retval 无
  */
void ESP8266_RX_DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();                   // 使能 DMA1 控制器时钟

    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);// 设置 DMA1_Channel3 中断优先级
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);       // 使能 DMA1_Channel3 中断
}

/**
  * @brief  调试串口 UART 初始化函数
  * @note   配置 USART1 的波特率、数据位、停止位、校验位与工作模式
  * @param  无
  * @retval 无
  */
void Debug_UART_Init(void)
{
  huart1.Instance = USART1;                          // 选择 USART1 外设
  huart1.Init.BaudRate = 115200;                     // 设置波特率为 115200
  huart1.Init.WordLength = UART_WORDLENGTH_8B;       // 数据位长度设置为 8 位
  huart1.Init.StopBits = UART_STOPBITS_1;            // 停止位设置为 1 位
  huart1.Init.Parity = UART_PARITY_NONE;             // 无奇偶校验
  huart1.Init.Mode = UART_MODE_TX_RX;                // 配置为收发模式
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;       // 不使用硬件流控
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;   // 过采样设置为 16 倍

  if (HAL_UART_Init(&huart1) != HAL_OK)              // 初始化 USART1 并检查是否成功
  {
    Error_Handler();                                 // 初始化失败，进入错误处理
  }
}

/**
  * @brief  ESP8266 串口 UART 初始化函数
  * @note   配置 USART3 的波特率、数据位、停止位、校验位与工作模式
  * @param  无
  * @retval 无
  */
void ESP8266_UART_Init(void)
{
  huart3.Instance = USART3;                          // 选择 USART3 外设
  huart3.Init.BaudRate = 115200;                     // 设置波特率为 115200
  huart3.Init.WordLength = UART_WORDLENGTH_8B;       // 数据位长度设置为 8 位
  huart3.Init.StopBits = UART_STOPBITS_1;            // 停止位设置为 1 位
  huart3.Init.Parity = UART_PARITY_NONE;             // 无奇偶校验
  huart3.Init.Mode = UART_MODE_TX_RX;                // 配置为收发模式
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;       // 不使用硬件流控
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;   // 过采样设置为 16 倍

  if (HAL_UART_Init(&huart3) != HAL_OK)              // 初始化 USART3 并检查是否成功
  {
    Error_Handler();                                 // 初始化失败，进入错误处理
  }
}

/**
  * @brief  UART MSP 底层初始化函数
  * @note   配置 UART 对应的时钟、GPIO、DMA 和中断
  * @param  uartHandle: UART 句柄指针
  * @retval 无
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};            // GPIO 配置结构体

  if(uartHandle->Instance==USART1)                   // 判断是否为 USART1
  {
    __HAL_RCC_USART1_CLK_ENABLE();                   // 使能 USART1 外设时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();                    // 使能 GPIOA 端口时钟

    /* USART1 GPIO 配置：PA9 -> TX，PA10 -> RX */
    GPIO_InitStruct.Pin = DEBUG_TX_Pin;              // 选择 TX 引脚
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;          // 复用推挽输出
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 高速输出
    HAL_GPIO_Init(DEBUG_TX_GPIO_Port, &GPIO_InitStruct); // 初始化 TX 引脚

    GPIO_InitStruct.Pin = DEBUG_RX_Pin;              // 选择 RX 引脚
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;          // 输入模式
    GPIO_InitStruct.Pull = GPIO_NOPULL;              // 无上下拉
    HAL_GPIO_Init(DEBUG_RX_GPIO_Port, &GPIO_InitStruct); // 初始化 RX 引脚

    /* USART1 中断配置 */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);         // 设置 USART1 中断优先级为 1
    HAL_NVIC_EnableIRQ(USART1_IRQn);                 // 使能 USART1 中断
  }
  else if(uartHandle->Instance==USART3)              // 判断是否为 USART3
  {
    __HAL_RCC_USART3_CLK_ENABLE();                   // 使能 USART3 外设时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();                    // 使能 GPIOB 端口时钟

    /* USART3 GPIO 配置：PB10 -> TX，PB11 -> RX */
    GPIO_InitStruct.Pin = ESP8266_TX_Pin;            // 选择 TX 引脚
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;          // 复用推挽输出
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 高速输出
    HAL_GPIO_Init(ESP8266_TX_GPIO_Port, &GPIO_InitStruct); // 初始化 TX 引脚

    GPIO_InitStruct.Pin = ESP8266_RX_Pin;            // 选择 RX 引脚
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;          // 输入模式
    GPIO_InitStruct.Pull = GPIO_NOPULL;              // 无上下拉
    HAL_GPIO_Init(ESP8266_RX_GPIO_Port, &GPIO_InitStruct); // 初始化 RX 引脚

    /* USART3 DMA 初始化：RX 使用 DMA1 通道3 */
    hdma_usart3_rx.Instance = DMA1_Channel3;         // 选择 DMA1 通道3
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY; // 数据方向：外设到内存
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;     // 外设地址不自增
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;         // 内存地址自增
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; // 外设数据按字节对齐
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;    // 内存数据按字节对齐
    hdma_usart3_rx.Init.Mode = DMA_NORMAL;            // 普通模式（非循环）
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;  // DMA 优先级低
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)      // 初始化 DMA 并检查是否成功
    {
      Error_Handler();                                // 初始化失败，进入错误处理
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart3_rx);  // 关联 DMA 句柄到 UART 的 hdmarx 成员

    /* USART3 中断配置 */
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);          // 设置 USART3 中断优先级为 1
    HAL_NVIC_EnableIRQ(USART3_IRQn);                  // 使能 USART3 中断
  }
}

/**
  * @brief  UART MSP 底层反初始化函数
  * @note   关闭 UART 外设时钟，释放 GPIO、DMA 和中断资源
  * @param  uartHandle: UART 句柄指针
  * @retval 无
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if(uartHandle->Instance==USART1)                    // 判断是否为 USART1
  {
    __HAL_RCC_USART1_CLK_DISABLE();                   // 关闭 USART1 外设时钟

    /* 释放 USART1 GPIO 引脚：PA9、PA10 */
    HAL_GPIO_DeInit(GPIOA, DEBUG_TX_Pin|DEBUG_RX_Pin);// 反初始化 TX/RX 引脚

    /* 释放 USART1 中断 */
    HAL_NVIC_DisableIRQ(USART1_IRQn);                 // 禁用 USART1 中断
  }
  else if(uartHandle->Instance==USART3)               // 判断是否为 USART3
  {
    __HAL_RCC_USART3_CLK_DISABLE();                   // 关闭 USART3 外设时钟

    /* 释放 USART3 GPIO 引脚：PB10、PB11 */
    HAL_GPIO_DeInit(GPIOB, ESP8266_TX_Pin|ESP8266_RX_Pin); // 反初始化 TX/RX 引脚

    /* 释放 USART3 DMA */
    HAL_DMA_DeInit(uartHandle->hdmarx);               // 反初始化与 UART3 关联的 RX DMA

    /* 释放 USART3 中断 */
    HAL_NVIC_DisableIRQ(USART3_IRQn);                 // 禁用 USART3 中断
  }
}

/**
  * @brief  重定向 printf 的输出到串口
  * @param  ch 要发送的字符
  * @param  f  文件指针（标准库要求的参数，一般不使用）
  * @retval 返回发送的字符
  */
int fputc(int ch, FILE *f)
{
         HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
         return ch;
}

