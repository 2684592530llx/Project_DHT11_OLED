DWT简介
在Cortex-M里面有一个外设叫DWT（Data Watchpoint and Trace），是用于系统调试及跟踪，它有一个32位的寄存器叫CYCCNT，它是一个向上的计数器，记录的是内核时钟运行的个数，内核时钟跳动一次，该计数器就加1，精度非常高，决定内核的频率是多少。

DWT相关寄存器

1.DEMCR：想要使能DWT外设，需要由另外的内核调试寄存器DEMCR 的位24 控制，写1 使能。DEMCR的地址是：0xE000EDFC。

2.DWT-CYCCNT：它是计数器，使能DWT_CYCCNT寄存器之前，先清0。其基地址是0xE0001004，复位默认值是0，可读写类型。所以往0xE0001004这个地址写0，就将DWT_CYCCNT 清0 了。

3.CYCCNTENA：它是DWT控制寄存器（DWT_CTRL）的第一位，其基地址为：0xE0001000，写1使能，则启用CYCCNT 计数器，否则CYCCNT计数器将不会工作。

DWT基础应用与获取程序时间和Dubug时间

1.复制之前手动创建的HAL库模板

2.在User中创建一个dwt的文件夹，在里面建立 bsp-dwt.c  bsp-dwt.h 两个文件。第一种方法：在Keil中创建一个 .c 文件然后保存在User的dwt文件下；第二种方法：直接在dwt文件夹中创建这两个文件。

3.打开 管理项目文件，将 Template 修改为 DWT ，在分组 USER 中添加我们创建在文件夹 User 中的bsp-dwt.c 。

4.在 bsp-dwt.c 添加头文件 #include "bsp-dwt.h" , 进入 bsp-dwt.h 添加

#ifndef  BSP-DWT-H
#define BSP-DWT-H
#include "main.h"  //建议在每一个 .h 文件中都引用#include "main.h"  
#endif 

这段代码是 头文件保护符（Header Guard），作用是防止同一个头文件被重复包含（#include）导致的编译错误。

第一次编译单元（.c 文件）遇到这个头文件时，_DWT-H 宏未定义，于是进入 #define 并处理后续内容。
第二次及之后再次遇到同一个头文件时，_DWT-H 已经定义过了，预处理器会跳过 #ifndef 和 #endif 之间的所有内容，从而避免重复定义函数、变量或类型。

5.打开之前创建好的 HAL库GPIO模板 -> 打开 Core -> 找到 gpio.c
                                                     -> 打开 Src -> 找到 gpio.h
复制到文件夹 User 新建的文件夹 led 中 , 修改为 bsp-led

6.在 管理项目文件 中的 USER 分组中添加 bsp-led.c

7.打开 bsp-led.c 将它的头文件改为 #include "led/bsp-led.h"

8.跳转到 bsp-led.h 中，此时是没有 LED 的宏定义的，需要在里面添加上宏定义(LED引脚定义)

#define     R-LED-Pin     GPIO-PIN-1
#define     G-LED-Pin     GPIO-PIN-2
#define     B-LED-Pin     GPIO-PIN-3
#define     LED-Port       GPIOA

因为修改了文件名，所以要将它的重复定义修改为--BSP-LED-H--

9.然后再进行 LED 控制亮灭的宏定义（SET 高电平 灭  ||  RESET  低电平  亮）
 
#define    R-LED-ON（）    HAL-GPIO-WritePin ( LED_Port , R-LED-Pin , GPIO-PIN-RESET );
#define    R-LED-OFF（）    HAL-GPIO-WritePin ( LED_Port , R-LED-Pin , GPIO-PIN-SET );
#define    R-LED-TOGGLE（）    HAL-GPIO-TogglePin ( LED_Port , R-LED-Pin );
...
#define    RGB-ALL-ON（）    R-LED-ON();  G-LED-ON();   B-LED-ON(); 
#define    RGB-ALL-OFF（）    R-LED-OFF();  G-LED-OFF();   B-LED-OFF(); 

仅亮一个灯
#define    R-LED-ON-ONLY    R-LED-ON();  G-LED-OFF();   B-LED-OFF(); 
...

10.由于宏定义的修改，需要在 bsp-led.c 中也进行对应的修改。

11.进入 bsp-dwt.c 添加函数 void DWT-Init(void) 对 DWT 进行初始化：
    void DWT-Init(void)
{
	DEMCR |= （uint32-t）DEMCR-TRCENA; //使能DWT外设，将它的第24位（TRCENA位）设置为 1  
	DWT-CYCCNT |= （uint32-t）0U;  //将计数器清零
	DWT-CTRL |= （uint32-t）DWT-CTRL-CYCCNTENA  //将CTRL寄存器的第 0 位设置为 1，使能CYCCNT计数器
}

12.跳转到 bsp-dwt.h 定义我们刚刚添加的函数

#define	DEMCR	*（uint32-t*）（0xE000EDFC）
#define	DWT-CRTL	*（uint32-t*）（0xE0001004）
#define	DWT-CYCCNT	*（uint32-t*）（0xE0001000）

/(uint32_t*)(0xE000EDFC)：将整数 0xE000EDFC 强制转换为指向 uint32_t 的指针（即地址）。
*( ... )：对这个指针进行解引用，得到该地址处存放的 uint32_t 变量。此后对 DEMCR 的读写就是直接操作该内存地址。*/

#define	DEMCR-TRCENA	（1<<24）               //第24位
#define	DWT-CTRL-CYCCNTENA	（1<<0）         //第0位

13.到 bsp-dwt.c 中添加函数 uint32-t  DWT-GetTick（void）;读取当前时间（节拍数）
	 uint32-t	DWT-GetTick（void）
{
	return （（uint32-t）DWT-CYCCNT）；
}

14.设置 DWT 延时
	uint32-t	DWT-TickToMs（uint32-t	tick , uint32-t	frency）
{
	return   （uint32-t）（1000000.0 / frency * tick）;
}

将给定的节拍数 tick 转换为对应的微秒数，输入节拍数跟系统时钟周期就可以得到对应的时间.

	void	DWT-DelayUs（uint32-t	time）
{
	uint32-t	duration = time * （ SystemCoreClock / 1000000.0 ）;  //总节拍(总时间) = time × 每微秒节拍数。这里的计算括号一定要加上，time * SystemCoreClock的结果很大，会产生下溢。
	uint32-t	start = DWT-GetTick（）;
	while（ DWT-GetTick（）-  start  <  duration ）;
}

	void	DWT-DelayMs（uint32-t	time）

{
	for（ int i = 0 ; i < time ; i ++）
	{
		DWT-DelayUs（1000）;
	}
}

	void	DWT-DelayS（uint32-t	time）

{
	for（ int i = 0 ; i < time ; i ++）
	{
		DWT-DelayMs（1000）;
	}
}

15.设置完成后，到 bsp-led.h 中统一声明一下。
void DWT_Init(void);
uint32_t DWT_GetTick(void);
uint32_t DWT_TickToMs(uint32_t tick , uint32_t frequency);
void DWT_DelayUs(uint32_t time);
void DWT_DelayMs(uint32_t time);
void DWT_DelayS(uint32_t time);

16.到 main.c 中添加好头文件 
#include "led/bsp-led.h" 
#include "dwt/bsp-dwt.h"

17.在 main.c 中定义 uint64-t	begin | end | duration = 0 ;		//记录时间
			     uint32-t	task-us = 0;	//用于将节拍数转化为时间Ms
18.在主函数中添加

DWT-Init（）；		//启动 DWT 计数器，用于精确测量程序运行时间 LED_GPIO_Config （）；
LED-GPIO-Config（）；	//初始化LED引脚
while（1）
{
 begin = DWT-GetTick（）；

RGB ALL ON（）；

DWT DelayMs（1000）；

RGB ALL OFF（）；

DWT_DelayMs（1000）；

end = DWT-GetTick（）；

duration = end - begin；//计算任务耗时（单位为DWT周期）
DWT-TickToMs（duration，SystemCoreClock）；	//转为微秒
}









