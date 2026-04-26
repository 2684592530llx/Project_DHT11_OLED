# DWT 精确延时 & 时间测量模块

## 1. DWT 简介
在 Cortex-M 内核中有一个外设叫 **DWT (Data Watchpoint and Trace)**，用于系统调试及跟踪。它有一个 32 位的寄存器 `CYCCNT`，这是一个向上计数器，记录内核时钟运行的个数。内核时钟每跳动一次，该计数器就加 1。精度非常高，取决于内核的频率（例如 72MHz 时每个计数对应约 13.9ns）。

## 2. DWT 相关寄存器
- **DEMCR**（地址 `0xE000EDFC`）：内核调试寄存器，位 24 写 1 使能 DWT 外设。
- **DWT_CYCCNT**（地址 `0xE0001004`）：32 位计数器，使能前应先清零。复位默认值为 0，可读写。
- **DWT_CTRL**（地址 `0xE0001000`）：DWT 控制寄存器，位 0（CYCCNTENA）写 1 启动 CYCCNT 计数器。

## 3. 模块使用步骤

### 3.1 创建文件
- 复制之前手动创建的 HAL 库模板。
- 在 `User` 文件夹下新建 `dwt` 文件夹，在其中建立 `bsp-dwt.c` 和 `bsp-dwt.h` 两个文件。
- 打开管理项目文件，将工程名 `Template` 修改为 `DWT`，在分组 `USER` 中添加 `bsp-dwt.c`。

### 3.2 头文件保护
在 `bsp-dwt.h` 中添加头文件保护符：使用 `#ifndef BSP_DWT_H`、`#define BSP_DWT_H`，并包含 `#include "main.h"`。这样可以防止同一个头文件被重复包含导致的编译错误。

### 3.3 移植 LED 模块（可选，用于演示）
- 从 HAL 库 GPIO 模板中复制 `gpio.c` 和 `gpio.h` 到 `User` 下的 `led` 文件夹，并重命名为 `bsp-led.c` / `bsp-led.h`。
- 在管理项目文件中将 `bsp-led.c` 添加到 `USER` 分组。
- 修改 `bsp-led.c` 中的头文件引用为 `#include "led/bsp-led.h"`。
- 在 `bsp-led.h` 中定义 LED 引脚（`R_LED_Pin` 为 PA1，`G_LED_Pin` 为 PA2，`B_LED_Pin` 为 PA3，`LED_Port` 为 GPIOA），并将头文件保护符修改为 `BSP_LED_H`。
- 定义 LED 控制宏：点亮用 `GPIO_PIN_RESET`（低电平），熄灭用 `GPIO_PIN_SET`（高电平），并组合出 `RGB_ALL_ON()`、`RGB_ALL_OFF()` 以及单个 LED 点亮等宏。

### 3.4 编写 DWT 初始化函数
在 `bsp-dwt.c` 中实现 `DWT_Init(void)`：
- 将 `DEMCR` 的第 24 位置 1，使能 DWT 外设。
- 将 `DWT_CYCCNT` 清零。
- 将 `DWT_CTRL` 的第 0 位置 1，使能 CYCCNT 计数器。

### 3.5 定义寄存器地址和位掩码
在 `bsp-dwt.h` 中：
- 定义 `DEMCR`、`DWT_CTRL`、`DWT_CYCCNT` 为指向对应地址的指针解引用。
- 定义 `DEMCR_TRCENA` 为 `(1<<24)`，`DWT_CTRL_CYCCNTENA` 为 `(1<<0)`。

### 3.6 获取当前节拍数
定义函数 `uint32_t DWT_GetTick(void)`，返回 `DWT_CYCCNT` 的值。

### 3.7 节拍转微秒
定义函数 `uint32_t DWT_TickToMs(uint32_t tick, uint32_t frequency)`，利用公式 `1000000.0 / frequency * tick` 将节拍数转换为微秒。

### 3.8 延时函数
- `DWT_DelayUs`：根据输入微秒数计算所需节拍数（使用 `time * (SystemCoreClock / 1000000.0)`，注意括号不能省略，否则会溢出），然后循环等待计数器差值达到该节拍数。
- `DWT_DelayMs`：循环调用 `DWT_DelayUs(1000)` 实现毫秒延时。
- `DWT_DelayS`：循环调用 `DWT_DelayMs(1000)` 实现秒延时。

### 3.9 函数声明
在 `bsp-dwt.h` 中声明上述所有函数。

### 3.10 在主函数中使用
- 在 `main.c` 中包含头文件 `"led/bsp-led.h"` 和 `"dwt/bsp-dwt.h"`。
- 定义变量 `uint64_t begin, end, duration = 0` 用于记录时间，`uint32_t task_us` 用于转换为微秒。
- 在主函数中调用 `DWT_Init()` 启动 DWT 计数器，调用 `LED_GPIO_Config()` 初始化 LED。
- 在主循环中：
  - 调用 `DWT_GetTick()` 记录开始节拍。
  - 执行 RGB 灯全亮，延时 1 秒，再全灭，延时 1 秒。
  - 再次调用 `DWT_GetTick()` 记录结束节拍，计算差值得到任务耗时（节拍数），然后调用 `DWT_TickToMs()` 将节拍数转换为微秒。

## 4. 注意事项
- 使能顺序：必须先使能 `DEMCR` 的 TRCENA 位，再使能 `DWT_CTRL` 的 CYCCNTENA 位。
- 延时计算中 `duration = time * (SystemCoreClock / 1000000.0)` 的括号不能省略，否则 `time * SystemCoreClock` 会溢出。
- `CYCCNT` 是 32 位计数器，在 72MHz 时钟下约 59.6 秒溢出一次。短时间测量（<1 秒）使用差值计算即可正确处理。
- `DWT_TickToMs` 实际返回的是微秒值，命名可根据喜好调整。

## 5. 常见问题
| 现象 | 解决方法 |
|------|----------|
| 编译报错 `undefined symbol` | 检查 `bsp-dwt.c` 是否已添加到工程，头文件路径是否正确 |
| 延时时间不准 | 确认 `SystemCoreClock` 与实际内核时钟一致（例如 72MHz） |
| 计数器始终为 0 | 单步调试，检查 `DEMCR` 和 `DWT_CTRL` 的对应位是否成功置 1 |

> 本模块基于 STM32F103C8T6 + HAL 库，可移植至其他 Cortex-M 内核芯片（需核对寄存器地址）。