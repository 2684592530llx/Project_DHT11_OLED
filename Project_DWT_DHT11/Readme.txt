DWT的应用和DHT11温湿度传感器

添加串口部分
1.在 User 文件夹中创建 usart 文件夹。

2.先从创建好的 usart 串口部分中复制 usart.c  usart.h 到 User 的 usart 中，修改名称 bsp-usart.c

3.再到 Keil 打开管理项目文件，修改项目名称 DWT-DHT1，
   把 bsp-usart.c 添加到 USER 中
   在 STM32F1xx_HAL Driver 中 -> Libraries -> STM32F1xx_HAL Driver -> Src -> dma.c + usart.c

4.修改头文件名称，在 main.h 中添加头文件 #include<stdio.h> ， 并重定向 printf（） 函数到 bsp-usart.c。

5.把错误中断函数 void Error-Handler(void); 放到 main.c 中，在 main.h 中进行申明。

6.将主函数中之前的代码删除，添加这次项目需要的代码。
  初始化 MX-USART1-UART-Init（）；

7.记得在魔术棒里，勾选上 Use MicroLIB

8.在 User 文件夹下创建好 dht11 文件夹，在 dht11 文件夹下添加 bsp-dht11.c  bsp-dht11.h。

9.在 bsp-dht11.h 中添加头文件保护符
  #ifndef --BSP-DHT11-H
  #define --BSP-DHT11-H
  
  #include "main.h"

  #endif 

  在 bsp-dht11.c 中添加头文件 #include "dht11/bsp-dht11.h"

  在管理项目文件中 USER 中添加 bsp-dht11.c

10.我们要初始化 GPIO 引脚，这里我们可以之间到 bsp-led.c 中复制 LED 的 GPIO 引脚的初始化 -> bsp-dht11.c 中。

11.在 bsp-dht11.c 中进行引脚定义
  #define	DHT11-DATA-Pin	        GPIO-PIN-12
  #define	DHT11-DATA-GPIO-Port	GPIOB

12.我们要用 PB12 引脚作为数据传输引脚,把初始化代码进行修改:

/*使能GPIOB端口时钟*/
_HAL_RCC_GPIOB_CLK_ENABLE（）；
/*设置引脚为推挽输出/
GPIO InitStruct.PinDHT11 DATA Pin；

HAL_GPIO_Init(DHT11-DATA_GPIO_Port，&GPIO_InitStruct）；

HAL_GPIO_WritePin（DHT11_DATA_GPIO_Port，DHT11_DATA_Pin，GPIO_PIN_SET）；

13.把 void	DHT11-GPIO-Config（）；放到 bsp-dht11.h 中进行申明。

14.  动态配置 DHT11 数据引脚的工作模式，因为在读取 DHT11 时需要切换 GPIO 方向：主机先输出（拉低总线发送起始信号），然后立即切换为输入（读取 DHT11 的响应和数据）。

void DHT11_SetGPIOMode(uint32_t mode, uint32_t pull)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin  = DHT11_DATA_Pin;        // 选择 DHT11 数据引脚
    GPIO_InitStruct.Mode = mode;                  // 设置输入/输出模式
    GPIO_InitStruct.Pull = pull;                  // 设置上拉/下拉/无拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // 低速即可，DHT11 时序要求不高

    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct); // 重新初始化引脚
}

模式的动态配置，可复制初始化代码，再进行修改。

15.通信时序 ：建立连接，数据接收

建立连接：
DHT11温湿度传感器上电后要等待1S以越过不稳定状态，在此期间不能发送任何指令，同时数据线处于空闲状态（数据线由上拉电阻立高一直保持高电平）

主机发送起始信号：主机的1/0设置为输出状态拉低数据线（设置为低电平）（数据线拉低时间>18ms），然后主机的1/0设置为输入状态，释放总线（保持30us），由于上拉电阻，数据线也随之变高电平

从机响应信号：
从机响应信号：从机接收到主机发送的起始信号后，先拉低数据线作为应答信号，再拉高数据线表示连接建立成功，通知主机准备接收数据（拉低数据线83us，拉高数据线87us，都延时等待 100us）

数据接收：
从机发送40位数据，每1bit数据都以54us低电平开始，主机通过判断低电平后的高电平时间来决定接收数据的种类
位数据"0”的格式为：54 us的低电平和23-27 us的高电平位数据“1”的格式为：54 us 的低电平加 68-74 us 的高电平



16.在发送数据之前是以一个 54us 的低电平开始的，低电平我们不需要判断，我们只需要延时 40us 看数据位是1还是0，如果 40 us 后线仍然是高，表示这一位是1，对应位就设置为 1，如果线为低，直接继续下一位。

uint8-t	DHT11-ReadByte（void）
{
	uint8-t   value = 0;
	for(uint8-t i = 0 ; i < 8; i++)
	{
	while（HAL-GPIO-ReadPin（DHT11-DATA-GPIO-Port ，DHT11-DATA-Pin）== RESET）；

	DWT-DelayUs（40）;
		if(HAL-GPIO-ReadPin（DHT11-DATA-GPIO-Port ，DHT11-DATA-Pin）== SET)
		{
			value |= (1 << (7 - i )); //对应位设置为 1
			while( HAL-GPIO-ReadPin（DHT11-DATA-GPIO-Port ，DHT11-DATA-Pin）== SET);
		}
	//如果 40us 后线为低，该位为 0 ， 直接继续下一位
	}
	return value;
}

17.读取DHT11的数据
/**
 * @brief  读取DHT11传感器数据
 * @param  data 指向存放温湿度数据的结构体指针
 * @retval HAL_OK 成功，HAL_ERROR 失败
 */
HAL_StatusTypeDef DHT11_ReadData(DHT11_DATA_TYPEDEF *data)
{
    uint8_t retry = 0;

    // 1. 主机拉低总线，发送起始信号（至少18ms）
    DHT11_SetGPIOMode(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);     // 设置为推挽输出
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET);
    DWT_DelayMs(20);  // 保持低电平20ms，通知DHT11开始传输
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);
    DWT_DelayUs(30); 	// 延时30微秒

    // 2. 设置引脚为输入，等待DHT11响应信号
    DHT11_SetGPIOMode(GPIO_MODE_INPUT, GPIO_PULLUP);

    // 3. 等待DHT11拉低响应信号（最大等待100us）
    retry = 0;
    while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET)
    {
        if (++retry > 100)
            return HAL_ERROR;    // 超时无响应，读取失败
        DWT_DelayUs(1);
    }

    // 等待DHT11拉高信号（最大等待100us）
    retry = 0;
    while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_RESET)
    {
        if (++retry > 100)
            return HAL_ERROR;    // 超时，读取失败
        DWT_DelayUs(1);
    }

    // 等待DHT11再次拉低信号（最大等待100us）
    retry = 0;
    while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET)
    {
        if (++retry > 100)
            return HAL_ERROR;    // 超时，读取失败
        DWT_DelayUs(1);
    }

    // 4. 读取5字节数据（湿度整数、小数，温度整数、小数，校验和）
    data->humi_int   = DHT11_ReadByte();
    data->humi_deci  = DHT11_ReadByte();
    data->temp_int   = DHT11_ReadByte();
    data->temp_deci  = DHT11_ReadByte();
    data->check_sum  = DHT11_ReadByte();

    // 5. 校验数据是否正确
    uint8_t sum = data->humi_int + data->humi_deci + data->temp_int + data->temp_deci;
    return (sum == data->check_sum) ? HAL_OK : HAL_ERROR;
}

写好后，记得添加 #include "dwt/bsp-dwt.h" 的头文件

18.在项目文件中新建 APP -> dht11-> app-dht11.c + app-dht11.h

19.在 Keil 的管理项目文件中添加分组 APP 以及 APP 文件中的 app-dht11.c

20.点开魔术棒在 Include Paths 中添加头文件 APP

21.在 app-dht11.c 中添加
#include "dht11/app-dhtl1.h"
#include "dht11/bsp-dht11.h"

读取数据打印结果

在app-dht11.h中添加头文件

22.在main.c中添加好全部的头文件，以及进行串口的初始化，DHT11的引脚初始化。

上电后要延迟一秒，越过不稳定状态；

在while（1）中添加DHT11的结果打印任务，并且延时两秒。













