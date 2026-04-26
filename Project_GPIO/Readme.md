# GPIO 模板工程（STM32CubeMX + Keil MDK）

## 📖 项目简介
本项目是基于 STM32F103C8T6 的 **GPIO 通用输入输出模板**，使用 STM32CubeMX 生成初始化代码，并采用 HAL 库驱动。它实现了通过引脚控制板载 LED（红、绿、蓝）的亮灭，是后续所有外设驱动和传感器采集工程的起点。

所有 GPIO 引脚都具有基本的输入输出功能：
- **输出**：控制引脚输出高、低电平，实现开关控制（如 LED、继电器、三极管等）。
- **输入**：检测外部输入电平（如按键、传感器信号等）。

## 🛠️ 硬件连接 (STM32F103C8T6)
| 引脚 | 功能 | 说明 |
|------|------|------|
| PA1  | RED  | 红色LED，低电平点亮 |
| PA2  | GREEN| 绿色LED，低电平点亮 |
| PA3  | BLUE | 蓝色LED，低电平点亮 |

> **注意**：所有 LED 均配置为 **推挽输出 (Output Push Pull)**，默认输出低电平（灯亮），适合共阳极连接方式。若硬件为共阴极，请将初始电平改为 High。

## ⌨️ 工程创建步骤
1. 打开 **STM32CubeMX**，选择芯片 `STM32F103C8T6`，新建工程。
2. **System Core 配置**：
   - `SYS` → Debug：`Serial Wire`
   - `RCC` → High Speed Clock (HSE)：`Crystal/Ceramic Resonator`
   - `RCC` → Low Speed Clock (LSE)：`Crystal/Ceramic Resonator`
3. **时钟树 (Clock Configuration)**：
   - 外部高速时钟 (HSE) → `/1` → 8 MHz → `×9` → PLLCLK = 72 MHz → `/1` → 系统时钟 72 MHz，APB2 同样 72 MHz。
4. **引脚配置**：
   - 在芯片图上找到 `PA1`、`PA2`、`PA3`，将它们设置为 `GPIO_Output`。
   - 在左侧 `GPIO` 标签页中，为每个引脚设置：
     - `GPIO output level`：**Low**（默认亮）
     - `GPIO mode`：**Output Push Pull**
     - 用户标签（User Label）：`RED`、`GREEN`、`BLUE`
5. **工程管理 (Project Manager)**：
   - 输入工程名称，选择保存路径。
   - `Toolchain / IDE`：选择 `MDK-ARM`。
   - `Code Generator` 选项卡中勾选：
     - `Copy only the necessary library files`
     - `Generate peripheral initialization as a pair of '.c/.h' files per peripheral`
6. 点击 **GENERATE CODE** 生成代码，用 Keil 打开工程。
