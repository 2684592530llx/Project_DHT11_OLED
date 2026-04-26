# DWT 应用与 DHT11 温湿度传感器（含串口）—— 配置步骤总结

## 一、添加串口部分
1. 在 `User` 文件夹中创建 `usart` 文件夹。
2. 从已有的串口模板中复制 `usart.c` 和 `usart.h` 到 `User/usart`，并重命名为 `bsp_usart.c` 和 `bsp_usart.h`。
3. 打开 Keil 管理项目文件：
   - 将项目名修改为 `DWT_DHT1`。
   - 将 `bsp_usart.c` 添加到 `USER` 分组。
   - 在 `STM32F1xx_HAL Driver` 分组中添加 `Libraries/STM32F1xx_HAL_Driver/Src` 下的 `dma.c` 和 `usart.c`。
4. 修改头文件：
   - 在 `main.h` 中添加 `#include <stdio.h>`。
   - 将 `printf` 函数重定向到 `bsp_usart.c`。
5. 将错误中断函数 `void Error_Handler(void)` 放到 `main.c` 中，并在 `main.h` 中声明。
6. 删除主函数中旧代码，添加本次项目所需的初始化：`MX_USART1_UART_Init()`。
7. 在魔术棒（Options for Target）中勾选 `Use MicroLIB`。

## 二、添加 DHT11 底层驱动
8. 在 `User` 文件夹下创建 `dht11` 文件夹，并在其中创建 `bsp_dht11.c` 和 `bsp_dht11.h`。
9. 在 `bsp_dht11.h` 中添加头文件保护符（`#ifndef BSP_DHT11_H` ...），并包含 `main.h`。
10. 在 `bsp_dht11.c` 中包含 `#include "dht11/bsp_dht11.h"`。
11. 在 Keil 管理项目文件的 `USER` 分组中添加 `bsp_dht11.c`。
12. 初始化 GPIO 引脚：可从 `bsp_led.c` 复制 LED 的 GPIO 初始化代码到 `bsp_dht11.c` 中。
13. 在 `bsp_dht11.c` 中进行引脚定义：
    - `DHT11_DATA_Pin` 为 `GPIO_PIN_12`
    - `DHT11_DATA_GPIO_Port` 为 `GPIOB`
14. 修改初始化代码：
    - 使能 GPIOB 时钟
    - 设置引脚为推挽输出，初始拉高
15. 在 `bsp_dht11.h` 中声明 `void DHT11_GPIO_Config(void)`。
16. 实现动态配置引脚工作模式的函数 `void DHT11_SetGPIOMode(uint32_t mode, uint32_t pull)`，用于切换输入/输出及上下拉。

## 三、通信时序与数据读取
17. **通信时序要点**：
    - 上电等待 1 秒（数据线空闲高电平）。
    - 主机发送起始信号：拉低 >18ms → 拉高并保持 30μs。
    - 从机响应：拉低约 83μs → 拉高约 87μs。
    - 数据接收：每 bit 以 54μs 低电平开始，高电平持续 23~27μs 为 `0`，68~74μs 为 `1`。
18. 实现 `uint8_t DHT11_ReadByte(void)`：
    - 等待低电平结束 → 延时 40μs → 判断电平，若为高则对应位置 1，并等待高电平结束；若为低则直接继续。
19. 实现 `HAL_StatusTypeDef DHT11_ReadData(DHT11_DATA_TYPEDEF *data)`：
    - 发送起始信号。
    - 等待 DHT11 响应（三次电平检测，超时返回错误）。
    - 连续读取 5 个字节（湿度整数、湿度小数、温度整数、温度小数、校验和）。
    - 校验前 4 字节之和是否等于校验和，返回 `HAL_OK` 或 `HAL_ERROR`。
20. 注意：上述函数需要包含 `#include "dwt/bsp_dwt.h"` 以使用 DWT 延时。

## 四、应用层封装
21. 在项目文件夹中新建 `APP/dht11`，创建 `app_dht11.c` 和 `app_dht11.h`。
22. 在 Keil 管理项目文件中添加分组 `APP`，并将 `app_dht11.c` 加入。
23. 在魔术棒的 `Include Paths` 中添加 `APP` 路径。
24. 在 `app_dht11.c` 中包含：
    - `#include "dht11/app_dht11.h"`
    - `#include "dht11/bsp_dht11.h"`
25. 在 `app_dht11.h` 中添加必要的头文件保护及函数声明。实现数据读取与打印函数（如 `Dht11_ReadAndPrint`）。

## 五、主函数集成
26. 在 `main.c` 中添加所有需要的头文件。
27. 进行串口初始化、DHT11 引脚初始化。
28. 上电后延时 1 秒，越过不稳定状态。
29. 在主循环 `while(1)` 中添加 DHT11 结果打印任务，并延时 2 秒。