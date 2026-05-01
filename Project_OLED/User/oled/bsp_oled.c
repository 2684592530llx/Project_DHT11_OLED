#include "oled/bsp_oled.h"
#include "i2c/bsp_i2c.h"
#include "usart/bsp_usart.h"

ErrorStatus OLED_CheckDevice(uint8_t slave_addr)//检测OLED设备是否存在,slave_addr是设备七位地址
{
	uint8_t dummty = 0;//要传一个数据下去，我们定义一个dummty随意传一个值就行。
	if(HAL_I2C_Master_Transmit(&hi2c1 , (slave_addr << 1) , &dummty , 0 , IIC_TIMEOUT) == HAL_OK)
	{
		return SUCCESS;
	}
	return ERROR;
}

ErrorStatus OLED_WriteByte(uint8_t cmd , uint8_t byte) //单字符传递，cmd：写入模式（传数据 | 传命令），byte：写入（具体数据 | 命令字节）
{
	uint8_t buf[2] = {cmd,byte};
	if(HAL_I2C_Master_Transmit(&hi2c1 , (OLED_SALVE_ADDER << 1) , buf , 2 , IIC_TIMEOUT) == HAL_OK)
	{
		return SUCCESS;
	}
	return ERROR;
}

ErrorStatus OLED_WriteBuffer(uint8_t cmd , uint8_t *buffer , uint32_t num)//多字节传输
{
	uint8_t buf[256];
	if(num > sizeof(buf) - 1)
	{
		printf("数据过长\n");
	}
	buf[0] = cmd;//第一个字节为标识
	
	memcpy(&buf[1],buffer,num);//将实际数据复制到buf数组

	if(HAL_I2C_Master_Transmit(&hi2c1 , (OLED_SALVE_ADDER << 1) , buf , num + 1 , IIC_TIMEOUT) == HAL_OK)
	{
		return SUCCESS;
	}
	return ERROR;
}

void OLED_SetPos(uint8_t y , uint8_t x)//设置OLED显示的位置
{
	uint8_t pos_buf[3] ={
		0xB0 + y ,             		// 设置页地址命令，0xB0 是页地址起始，后加页码
		((x & 0xF0) >> 4) | 0x10, 	// 设置列地址高4位 + 0x10 (列地址高4位起始命令)
		(x & 0x0F)             		// 设置列地址低4位
	};
	OLED_WriteBuffer(OLED_WR_CMD , pos_buf , sizeof(pos_buf));
}

void OLED_Fill(uint8_t fill_data)//填充整个屏幕
{
	uint8_t data_buffer[128];
	memset(data_buffer , fill_data , 128);//fill_data是0xFF，就点亮整个屏幕。fill_data是0x00，就清屏
	
	for(uint8_t i = 0 ; i < 8 ; i++)
	{
		OLED_SetPos(i , 0);// 设置光标位置到第m页第0列
		OLED_WriteBuffer(OLED_WR_DATA , data_buffer , sizeof(data_buffer));//写入整页数据
	}
}

void OLED_Clear(void)//清屏
{
	OLED_Fill(0x00);
}

void OLED_FillFull(void)//填充整个屏幕
{
	OLED_Fill(0xFF);
}

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

