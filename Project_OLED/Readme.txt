OLED模块初始化与清屏

模块简介
0.96寸IIC OLED模块，屏幕分辨率为128*64像素，白色显示，驱动芯片为SSD1306，使用11C接口，用于显示字符、图片等信息

操作步骤
1.复制前一个 Project_DWT_DHT11 文件夹重命名 Project OLED Init.

2.将文件夹内的 APP ， Use r-> dht11删除掉。然后打开工程，修改工程名，在管理项目文件中也把对应的删除掉，魔术棒中也取消 APP 的路径。把代码部分也清除。

3.打开 STM32CubeMX ，  配置完 SYS RCC 的基础配置后，将 PB6 PB7，设置为 I2C，在 I2C Speed Mode 中选择 Fase Mode。

4.创建好后，在 User 中创建 i2c  将 Core -> Src -> i2c.c 复制到 i2c 中 ，Core -> Inc -> i2c.h 复制到 i2c 中，修改名称 bsp-i2c。

5.在项目 User 中新建 oled 文件夹，在里面新建 bsp-oled.c bsp-oled.h

6.在管理项目文件 USER 中添加好头文件，以及stm32f1xx-hal-i2c.c。

7.在 bsp-i2c-oled.h 中进行宏定义
#define	SALVE-ADDER	(0x3c)	//OLED屏幕I2C从机地址
#define	IIC-TIMEOUT	100	//超时时间

#define	OLED-WR-CMD		(0x00)	//OLED写指令
#define	OLED-WR-DATA	(0x40)	//OLED写数据

8.添加好对应的头文件，

#include <string.h>
#include<stdlib.h>

在bsp-i2c-oled.c中添加函数
ErrorStatus OLED-CheckDevice(uint8-t  slave-addr)
{
	uint8-t   dummy = 0;
	if(HAL-I2C-Master -Transmit(&hi2c1 , (salve-adder << 1), &dummy , 0 , IIC-TIMEOUT) == HAL-OK)
	{
		return SUCCESS;
	}
	return ERROR;
}
//检测 I2C 设备是否存在

9.ErrorStatus OLED-WriteByte(uint8-t  cmd , uint8-t byte)
//cmd：写入模式，取值为 OLED-WR-CMD | OLED-WR-DATA
//byte：写入的具体数据 | 命令字节
{
	uint8-t  buf[2] = {cmd , byte};     
	if(HAL-I2C-Master-Transmit(&hi2c1,OLED-SLAVER-ADDR << 1),buf,2,IIC-TIMEOUT) == HAL-OK)
//通过 I2C 主机发送数据包给 OLED 设备，地址左移1位为7位地址格式，发送两个字节超时为 IIC-TIMEOUT
	return SUCCESS;
	
	printf("失败");
	return ERROR;
}
//写入单字节函数

10.
ErrorStatus OLED-WriteBuffer(uiint8-t  cmd , uint8-t  *buffer , uint32-t  num)
{
	uint8-t  buf[256];
	if(num > sizeof(buf) - 1)
	{
		printf（“数据过长”）;
	}
	buf[0] = cmd;//第一个字节为标识
	memcpy(&buf[1] , buffer , num);//将实际数据复制到buf数组
	HAL-StatusTypeDef  status = HAL-I2C-Master-Transmit(&hi2c1 , OLED-SLAVER-ADDR << 1, buf , num + 1 , IIC-TIMEOUT);

	if(status == HAL-OK)  return SUCCESS;
	printf("失败,cmd = 0x%02x,len=%u",cmd , num); 
	return ERROR;
}
//写入多字节函数

这里注意 num + 1 OLED 的 I2C 通信协议中，每个数据包的开头必须附带一个“控制字节”


11.
void OLED-SetPos(uint8-t  y  ,  uint8-t  x)
{
	uint8-t  pos-buf[3] = {
		0xB0 + y ,			//页地址命令
		((x & 0xF0) >>4) | 0x10 ,	//列地址的高四位
		(x & 0x0F)			//列地址的低四位
	}
	OLED-WriteBuffer(OLED-WR-CMD , pos-buf , sizeof (pos-buf));		
}
//设置OLED显示的位置

这里要注意 x 列地址要右移四位：

(x & 0xF0) | 0x10

缺少了右移4位的操作 `>> 4`。  
以 `x = 100`（0x64）为例：
- 正确做法：`(0x64 & 0xF0) = 0x60`，右移4位得 `0x06`，再 `| 0x10` 得 `0x16`。  
- 你的错误代码：`(0x64 & 0xF0) | 0x10 = 0x60 | 0x10 = 0x70`。

`0x70` 并不在 SSD1306 列地址高4位命令的有效范围（`0x10`~`0x1F`）内。OLED 控制器无法解析这个非法命令，可能会：
- 忽略该命令，使用默认的高4位（通常是0）。
- 或将其误解为其他功能，导致列地址高4位被错误地设置为0或其他值。

后果
如果列地址高4位被设为0，那么实际列地址 = 0x00（低4位） = 0，无论你的 `x` 是多少，都会从第0列开始显示。因此：
- 本应在 `x=100` 列显示的内容，被写到了第0列。
- 而真正 `x=0~99` 的区域可能没有被写入（或写入了后来的数据），视觉上表现为屏幕**左边大部分区域没有点亮**（实际是数据错位）。


12.
  void OLED-Fill(uint8-t  fill-data)
{
	uint8-t  data-buffer [128]= {0};
	memset(data-buffer , fill-data , 128);

	for(uint8-t  i = 0 ; i < 8 ,  i++)
	{
		OLED-SetPos( i , 0 );
		OLED-WriteBuffer(OLED-WR-DATA , data-buffer , sizeof(data-buffer));
	}
}
//填充整个屏幕

13.我们有了填充屏幕的代码，就可以写出 点亮全屏 清屏 。
void OELD-FillFull(void)
{
	OLED-Fill(0xFF);
}
void OELD-Clear(void)
{
	OLED-Fill(0x00);
}

14.OLED 的初始化，查看手册的内容，看对应地址怎么设置。
void OLED_Init(void)
{
    HAL_Delay(100);  // 延时保证电源稳定

    while (OLED_CheckDevice(OLED_SALVE_ADDER) != SUCCESS);

		/* 控制显示 */
		OLED_WriteByte(OLED_WR_CMD,0xAE);//设置显示打开/关闭(AFh/AEh)

		/* 控制内存寻址模式 */
		OLED_WriteByte(OLED_WR_CMD,0x20);//设置内存寻址模式(20h)
		OLED_WriteByte(OLED_WR_CMD,0x02);//00b，水平寻址模式;01b，垂直寻址模式;10b，页面寻址模式(RESET);11，无效

		/* 页起始地址 */ 
		OLED_WriteByte(OLED_WR_CMD,0xB0);//设置页面寻址模式的页面起始地址，0-7(B0-B7)(PAGE0-PAGE7)

		/* COM输出扫描方向 */
		OLED_WriteByte(OLED_WR_CMD,0xA1);//设置左右方向，0xA1正常 0xA0左右反置
		OLED_WriteByte(OLED_WR_CMD,0xC8);//设置上下方向，0xC8正常 0xC0上下反置

		/* 页内列起始地址 */
		OLED_WriteByte(OLED_WR_CMD,0x00);//设置列地址低位0-7
		OLED_WriteByte(OLED_WR_CMD,0x10);//设置列地址高位0-F(10h-1Fh) 列序号=列地址低位*列地址高位(最高位不参与乘积)

		/* 页内行起始地址 */
		OLED_WriteByte(OLED_WR_CMD,0x40);//设置起始行地址

		/* 设置对比度 */
		OLED_WriteByte(OLED_WR_CMD,0x81);//设置对比度控制寄存器(81h)
		OLED_WriteByte(OLED_WR_CMD,0xff);//0x00~0xff

		/* 设置显示方向 */
		OLED_WriteByte(OLED_WR_CMD,0xA1);//将列地址0映射到SEG0(A0h)/将列地址127映射到SEG0(A1h)
		OLED_WriteByte(OLED_WR_CMD,0xA6);//设置正常显示(A6h)/倒转显示(A7h)

		/* 设置多路复用率 */
		OLED_WriteByte(OLED_WR_CMD,0xA8);//多路复用率(A8h)
		OLED_WriteByte(OLED_WR_CMD,0x3F);//(1 ~ 64)

		/* 全屏显示 */
		OLED_WriteByte(OLED_WR_CMD,0xA4);//设置整个显示打开/关闭(A4恢复到RAM内容显示,输出遵循RAM内容/A5全屏显示,输出忽略RAM内容)

		/*设置显示偏移量*/
		OLED_WriteByte(OLED_WR_CMD,0xd3);//设置显示偏移量(D3h)
		OLED_WriteByte(OLED_WR_CMD,0x00);

		/* 设置显示时钟分频比/振荡器频率 */
		OLED_WriteByte(OLED_WR_CMD,0xD5);//设置显示时钟分频比/振荡器频率
		OLED_WriteByte(OLED_WR_CMD,0xf0);//设定分割比

		/* 设置预充期 */
		OLED_WriteByte(OLED_WR_CMD,0xD9);//设置预充期
		OLED_WriteByte(OLED_WR_CMD,0x22);

		/* 设置com引脚硬件配置 */
		OLED_WriteByte(OLED_WR_CMD,0xDA);//设置com引脚硬件配置
		OLED_WriteByte(OLED_WR_CMD,0x12);

		/* 设置VCOMH取消选择级别 */
		OLED_WriteByte(OLED_WR_CMD,0xDB);//设置VCOMH取消选择级别
		OLED_WriteByte(OLED_WR_CMD,0x20);//0x20,0.77xVcc

		OLED_WriteByte(OLED_WR_CMD,0x8D);//设置DC-DC使能
		OLED_WriteByte(OLED_WR_CMD,0x14);

		/* 显示打开 */
		OLED_WriteByte(OLED_WR_CMD,0xAF);//设置显示打开/关闭(AFh/AEh)

		OLED_Clear();     // 初始化完成后清屏，确保屏幕全黑，清除残留显示内容
		
		OLED_FillFull(); // 全屏点亮，便于观察OLED显示是否正常工作

}





