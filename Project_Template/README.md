# STM32 手动创建 Keil 工程模板

## 简介
本项目记录了一个基于 **STM32F103C8T6** 的 **HAL 库** 手动 Keil 工程创建过程，适合初学者理解工程结构、文件组织及编译器配置。

## 目录结构

## 手动创建工程步骤

### 一、准备文件夹
1. 在项目根目录下创建四个文件夹：`Doc`、`Libraries`、`Project`、`User`。

### 二、填充 Libraries 文件夹
2. 从 HAL 库包（STM32Cube_FW_F1_V1.8.0）中复制以下内容：
   - 将 `STM32F1xx-HAL-Driver` 文件夹复制到 `Libraries` 中。
   - 将 `CMSIS` 文件夹复制到 `Libraries` 中。

3. 精简 `Libraries/CMSIS`：
   - 只保留 `Device` 和 `Include` 文件夹。
   - 进入 `Device -> ST -> STM32F1xx`，只保留 `Include` 和 `Source` 文件夹。

4. 精简 `Libraries/STM32F1xx-HAL-Driver`：
   - 只保留 `Inc` 和 `Src` 文件夹。

### 三、填充 User 和 Doc 文件夹
5. 从 HAL 库包的以下路径：
   `Projects/STM3210E-EVAL/Templates/`
   - 将 `Inc` 中的 `main.h`、`stm32f1xx_it.h` 复制到 `User` 文件夹。
   - 将 `Src` 中的 `main.c`、`stm32f1xx_it.c`、`system_stm32f1xx.c` 复制到 `User` 文件夹（**不要**复制 `stm32f1xx_hal_msp.c`）。

6. 在 `Doc` 文件夹中新建 `readme.txt`（可存放工程说明）。

### 四、Keil 工程配置

#### 4.1 新建工程
- 打开 Keil uVision5，点击 `Project` -> `New uVision Project`。
- 路径选择 `Project` 文件夹，工程名自定（如 `Template`）。
- 选择芯片型号：`STM32F103C8T6`。

#### 4.2 管理项目文件
- 点击工具栏的 **三个不同颜色方块**（Manage Project Items）。
- 设置 `Project Targets` 为 `Template`。
- 新建分组（Groups）：`STARTUP`、`CMSIS`、`STM32F1xx-HAL-Driver`、`USER`、`DOC`。

##### 添加文件到各分组：
| 分组 | 添加的文件路径 |
|------|----------------|
| **STARTUP** | `Libraries/CMSIS/Device/ST/STM32F1xx/Templates/arm/startup_stm32f103xb.s` |
| **CMSIS** | `Libraries/CMSIS/Device/ST/STM32F1xx/Templates/system_stm32f1xx.c` |
| **STM32F1xx-HAL-Driver** | `Libraries/STM32F1xx-HAL-Driver/Src/stm32f1xx_hal.c`<br>`.../stm32f1xx_hal_cortex.c`<br>`.../stm32f1xx_hal_gpio.c`<br>`.../stm32f1xx_hal_rcc.c` |
| **USER** | `User/main.c`<br>`User/stm32f1xx_it.c` |
| **DOC** | `Doc/readme.txt` |

#### 4.3 魔术棒（Options for Target）设置

##### Target 标签
- **ARM Compiler**：`Use default compiler version 5`

##### Output 标签
- 勾选 **Create HEX File**

##### C/C++ 标签
- **Define**：输入 `USE_HAL_DRIVER,STM32F103xB`
- 勾选 **C99 Mode**
- **Include Paths** 添加以下路径：
  
##### Debug 标签
- **Use**：选择 `CMSIS-DAP Debugger`

### 五、编译与调试
- 编译前，打开 `User/main.h`，删除或注释掉 `#include "stm3210e_eval.h"`（该头文件不属于本工程）。
- 按 `F7` 编译，如无错误则工程创建成功。
- 连接调试器（如 DAP-Link），按 `Ctrl+F5` 进入调试。

## 注意事项
- 芯片型号选择 `STM32F103C8T6`（中等容量产品）。
- 启动文件选择 `startup_stm32f103xb.s`（对应 64K Flash）。
- 如果使用其他型号，请调整启动文件和 `Define` 中的宏（如 `STM32F103xE`）。
- 工程中未添加 `stm32f1xx_hal_msp.c`，该文件通常由 CubeMX 生成，手动工程可暂不添加。

## 参考资源
- [CubeMX与HAL库](https://pan.baidu.com/s/1We7j85Dt1IRDG2LDegTB4A) 百度网盘提取码: 1235
- [Keil MDK 官方文档](https://www.keil.com/support/man/docs/)


## 更新记录
- 2026.04.19 初始版本，整理手动创建工程流程。
  
## 作者
- 刘礼鑫