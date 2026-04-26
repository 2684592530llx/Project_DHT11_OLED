手动建立工程

1.创建 Doc，Libraries，Project，User 四个文件夹。

2.Doc：存放项目文档，Libraries：存放HAL驱动库，CMSIS核心库，Project：存放Keil工程文件，User：用户编写的业务逻辑代码。

3.打开Hal库文件夹找到 STM32F1xx-HAL-Driver 和 CMSIS 复制到 Libraries 中。

4. CMSIS 中只需要留下 Device 和 Include ，Device -> ST (官方内核文件) -> STM32F1xx 只需要留下Include 和 Source。

5. STM32F1xx-HAL-Driver 中只需要保留 Inc 和 Src 。

6. Hal库文件夹找到 Projects -> STM3210E-EVAL -> Templates -> Inc 复制这个三个 .h 文件到 User中，-> Src 复制三个 .c 文件（除stm32f1xx-hal-msp.c）到User中，在Doc中添加 readme.txt 。

7.打开 Keil 新建工程，选择路径（project），选择芯片型号（FC8T6），删除部分文件夹（Listings）。
##在新建工程选择路径时，如果文件出现红色感叹号！导致无法新建，就进入任务管理器找到TortoiseGit status cache，然后关闭它就行了。

8.设置工程名和分组名，添加启动文件，添加 system 源码文件，添加 STM32F1xx-HAL-Driver 源码，添加 USER 源码文件，添加DOC中的 readme.txt 文件。

9.打开 Manage Project Items（三个不同颜色的方块），命名 Project Targets ：Template ，分组Groups：STARTUP，CMSIS，STM32F1xx-HAL-Driver，USER，DOC.

选择 STARTUP 点击 ADD-Files  ->Libraries->CMSIS->Driver->ST->STM32F1xx->Source->Templates->arm->startup-stm32f103xb.s 点击ADD

选择 CMSIS 点击 ADD-Files  ->Libraries->CMSIS->Driver->ST->STM32F1xx->Source->Templates->system-stm32f1xx.c 点击ADD

选择 STM32F1xx-HAL-Driver 点击 ADD-Files ->Libraries->STM32F1xx-HAL-Driver->Src->选择stm32f1xx-hal.c + stm32f1xx-hal-cortex.c +stm32f1xx-hal-gpio.c + stm32f1xx-hal-rcc.c

选择 USER 点击 ADD-Files ->User->main.c + stm32f1xx-it.c

选择 DOC 点击 ADD-Files ->Doc->readme.txt

10.魔术棒设置
Target 编译器设置 ： ARM Compiler ：Use default compiler version 5

Output ：勾选 Create HEX File

C/C++ ：Define：USE_HAL_DRIVER,STM32F103xB  
              勾选C99 Mode
              Include Paths：->Libraries->CMSIS->Device->ST->STM32F1xx->Include
				     ->Libraries->CMSIS->Include
				     ->Libraries->STM32F1xx_HAL-Driver->Inc
				     ->User
Debug ：Use：CMSIS-DAP Debugger

11.编译main.c 在 main.h 中删除 #include "stm3210e_eval.h"








