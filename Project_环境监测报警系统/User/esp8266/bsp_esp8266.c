#include "esp8266/bsp_esp8266.h"
#include "usart/bsp_usart.h"

/* 分别定义两个接收缓冲区 */
uint8_t Debug_RxBuff[UART_RX_BUFF_LEN];      // 上位机串口接收缓冲
uint8_t ESP8266_RxBuff[UART_RX_BUFF_LEN];    // ESP8266串口接收缓冲

volatile bool ESP_RX_Complete = 0;	// ESP8266 接收完成标志（UART/ DMA 发送完成后置位）

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
				ESP_RX_Complete = 1;

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

    // 启动 UART 接收任务
    UART_StartReceive();                                      
}

/**
 * @brief  向 ESP8266 发送字符串数据（常用于 AT 指令）
 * @param  str 待发送的字符串
 * @retval None
 */
void ESP8266_SendString(char *str)
{
    // 1. 完全停止 DMA 接收，并清除接收器状态
    HAL_UART_DMAStop(&huart3);
    HAL_UART_AbortReceive(&huart3);
    __HAL_UART_CLEAR_OREFLAG(&huart3);      // 清除溢出错误标志
    (void)READ_REG(huart3.Instance->DR);     // 读一次数据寄存器，清空残留

    // 2. 阻塞发送数据（超时 5000ms，足够发送几百字节）
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 5000);

    // 3. 重新启动 DMA 空闲接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ESP8266_RxBuff, UART_RX_BUFF_LEN);
}

/**
  * @brief  发送 AT 指令到 ESP8266 并等待发送完成
  * @note   本函数只负责发送，不检查 ESP8266 的响应
  * @param  cmd: 要发送的 AT 指令字符串
  * @retval 1 表示发送成功，0 表示发送失败
  */
uint8_t ESP8266_Cmd(const char *cmd)
{
    // 1. 暂停 DMA，准备清空缓冲区
    HAL_UART_DMAStop(&huart3);
    HAL_UART_AbortReceive(&huart3);
    __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_ORE);
    (void)READ_REG(huart3.Instance->DR);


    // 2. 清空接收缓冲区，重置标志
    memset(ESP8266_RxBuff, 0, UART_RX_BUFF_LEN);
    ESP_RX_Complete = 0;

    // 3. 重新启动 DMA 接收（现在缓冲区干净，准备捕获回显）
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ESP8266_RxBuff, UART_RX_BUFF_LEN);

    // 4. 拼接指令
    static char buf[512];
    snprintf(buf, sizeof(buf), "%s\r\n", cmd);

    // 5. 阻塞发送（DMA 正在后台接收，回显不会丢失）
    HAL_UART_Transmit(&huart3, (uint8_t *)buf, strlen(buf), 5000);

    return 1;
}

/**
  * @brief  测试 ESP8266 模块是否可用
  * @retval 0 表示失败，1 表示成功
  */
uint8_t ESP8266_Test(void)
{
    // 发送 AT 指令，检测模块是否响应
    if(!ESP8266_Cmd("AT")) 
        return 0;  

    // 等待接收完成，最长 1000ms
    uint32_t tick_start = HAL_GetTick();
    while(!ESP_RX_Complete)
    {
        if(HAL_GetTick() - tick_start > 1000)
            return 0;  // 超时未响应，失败
    }

    // 确保接收缓冲区以 '\0' 结尾，防止字符串操作越界
    ESP8266_RxBuff[UART_RX_BUFF_LEN - 1] = '\0';

    // 检查返回数据中是否包含 "OK"
    if(strstr((char *)ESP8266_RxBuff, "OK") == NULL)
        return 0;  // 未检测到 "OK"，失败

    // 测试成功
    return 1;
}

/**
  * @brief  ESP8266 软件复位并等待 ready
  * @retval 1 表示复位完成，0 表示超时
  */
uint8_t ESP8266_ResetWait(void)
{
    // 发送软复位指令 AT+RST
    if(!ESP8266_Cmd("AT+RST"))
        return 0;   // 指令发送失败，返回 0

    // 获取当前系统时间
    uint32_t tick_start = HAL_GetTick();
    // 等待最大 5 秒
    while(HAL_GetTick() - tick_start < 5000)
    {
        // 判断是否接收完成
        if(ESP_RX_Complete)
        {
            // 确保接收字符串以 '\0' 结尾
            ESP8266_RxBuff[UART_RX_BUFF_LEN - 1] = '\0';

            // 检查是否包含 "ready"
            if(strstr((char *)ESP8266_RxBuff, "ready") != NULL)
            {
                ESP_RX_Complete = 0;   // 清除标志位
                return 1;              // 收到 ready，复位完成
            }

            // 未匹配到 ready，清除标志位，继续等待
            ESP_RX_Complete = 0;
        }
    }

    // 超时未收到 "ready"，返回失败
    return 0;
}

/**
  * @brief  设置 ESP8266 工作模式
  * @param  mode: 工作模式枚举类型 (STA/AP/STA+AP)
  * @retval 0 表示失败，1 表示成功
  */
uint8_t ESP8266_SetMode(ENUM_Net_ModeTypeDef mode)
{
    char *cmd;   // 定义 AT 指令字符串指针

    // 根据传入的模式选择对应 AT 指令
    switch(mode)
    {
        case STA:    
            cmd = "AT+CWMODE=1";   // 设置为 STA 模式
            break;
        case AP:     
            cmd = "AT+CWMODE=2";   // 设置为 AP 模式
            break;
        case STA_AP: 
            cmd = "AT+CWMODE=3";   // 设置为 STA+AP 模式
            break;
        default:     
            return 0;              // 非法模式参数，返回失败
    }

    // 发送设置模式指令
    if(!ESP8266_Cmd(cmd)) 
        return 0;   // 发送失败，返回 0

    // 获取当前系统时间
    uint32_t tick_start = HAL_GetTick();
    // 等待接收完成
    while(!ESP_RX_Complete)
    {
        // 超过 1000ms 超时
        if(HAL_GetTick() - tick_start > 1000)
            return 0;   // 返回失败
    }

    // 确保字符串结尾
    ESP8266_RxBuff[UART_RX_BUFF_LEN - 1] = '\0';

    // 检查返回数据是否包含 "OK"
    if(strstr((char *)ESP8266_RxBuff, "OK"))
        return 1;   // 包含 OK，设置成功

    // 未包含 OK，返回失败
    return 0;
}


/**
  * @brief  连接指定 Wi-Fi
  * @param  ssid: Wi-Fi 名称
  * @param  password: Wi-Fi 密码
  * @retval 0 表示成功，非 0 表示失败及错误类型
  *         1：连接超时
  *         2：密码错误
  *         3：找不到目标 AP
  *         4：连接失败（其他原因）
  */
uint8_t ESP8266_ConnectWiFi(const char *ssid, const char *password)
{
    char cmd[128];                                          // 定义发送指令缓冲区
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password); // 构造连接 Wi-Fi 指令

    // 发送连接指令
    if(!ESP8266_Cmd(cmd)) return 4;     	// 发送失败，返回 4
	
    uint32_t tick_start = HAL_GetTick();  // 获取当前系统时间

    // 等待接收最终状态或超时（最大等待 15 秒）
    while(HAL_GetTick() - tick_start < 15000)
    {
        if(ESP_RX_Complete)
        {
            ESP8266_RxBuff[UART_RX_BUFF_LEN - 1] = '\0';

            // 连接成功
            if(strstr((char *)ESP8266_RxBuff, "OK"))
                return 0;

            // 连接失败或错误码
            if(strstr((char *)ESP8266_RxBuff, "+CWJAP:1")) return 1;  // 连接超时
            if(strstr((char *)ESP8266_RxBuff, "+CWJAP:2")) return 2;  // 密码错误
            if(strstr((char *)ESP8266_RxBuff, "+CWJAP:3")) return 3;  // 找不到目标 AP
            if(strstr((char *)ESP8266_RxBuff, "+CWJAP:4") ||
               strstr((char *)ESP8266_RxBuff, "FAIL")) return 4;      // 其他失败

            // 清标志，继续等待下一条信息
            ESP_RX_Complete = 0;
        }
    }

    // 超时未收到最终状态
    return 1;
}


/*****************************END OF FILE***************************************/
