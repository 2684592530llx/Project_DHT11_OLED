DHT11温湿度传感器数据显示在OLED屏幕上面

1.用 PCtoLCD 生成字模，先配置他的基础设置。
点阵格式：阴码
取模方式：列行式
取模走向：逆向（低位在前）
自定义格式：C51
输出数制：十六进制数

2.复制文件夹 Project-OLED ,重命名 OLED模块显示字符与DHT11数据

3.删除 OLED-Init（）函数中的 OLED-FillFull（）填充整个屏幕的函数，只留下OLED-Clear（）清屏函数。

4.在 User 中新建 fonts 文件夹，在里面新建 bsp-fonts.c bsp-fonts.h 文件。

5.打开 Keil 的管理项目文件在 USER 中添加 Fonts.c 文件。

6.在 PCtoLCD 中生成 ASCII码 字模索引（12x8）（16x16），然后复制到 bsp-fonts.c 中，用一个二维数组 uint8-t  ascii-code-6x8[ ][6] = { }；uint8-t  ascii-code-8x16[ ][16] = { }；将它们括起来。

外部声明
extern const uint8_t chinese_library_16x16[][16]; 
extern const uint8_t ascll_code_6x8[][6]; 
extern const uint8_t ascll_code_8x16[][16];

这里我们为了节省空间，以及为了防止被修改，我们用 const uint8-t  ascii-code-6x8[ ][8] = { }；修饰后，它会被存放在 flash 中。

7. 显示汉字
void  OLED-ShowChinese(uint-8  y , uint-8  x ,  uint-8  n )
{
	OLED-SetPos(y , x);  //设置第一页位置
	for(uint8-t  i = 0 ; i < 16 ; i++)
	{
		OLED-WriteByte(OLED-WR-DATA , (uint8-t) chinese_libary_16x16[n * 2][i]);
	}
	OLED-SetPos(y + 1 , x);
	for(uint8-t  i = 0 ; i < 16 ; i++)
	{
		OLED-WriteByte(OLED-WR-DATA , (uint8-t) chinese_libary_16x16[n * 2 + 1][i]);
	}
}

void  OLED-ShowChinese-F16X16(uint-8  line , uint-8  offset ,  uint-8  n)
{
	 OLED-ShowChinese(line * 2 , offset * 2,  n);
}

每个汉字用两个连续的16字节块，所以是上半部分：chinese_libary_16x16[n*2]
下半部分：chinese_libary_16x16[n*2 + 1]


8.显示 ASCII码
宏定义：
#define	TEXTSIZE-F6X8		6
#define	TEXTSIZE-F8X16	8
#define	TEXTSIZE-F16X16	16

void  OLED-ShowChar(uint8-t  y , uint8-t  x  , uint8-t  data-char , uint8-t  textsize)
{
	uint8-t	adder = char-data - 32;//计算字符在点阵数组中的索引

	swtich(textsize)
	{
		case TEXTSIZE-F6X8:
				OLES-SetPos(y , x);
				for(uint8-t  i = 0 ; i < 8 ; i++)
				{
					OLED-WriteByte(OLED-WR-DATA , (uint8-t) ascll_code_6x8[adder][i]);
				}
				break;

		case TEXTSIZE-F8X16:
				OLES-SetPos(y , x);
				for(uint8-t  i = 0 ; i < 8 ; i++)
				{
					OLED-WriteByte(OLED-WR-DATA , (uint8-t) ascll_code_8x16[adder][i]);
				}
				OLES-SetPos(y + 1 , x);
				for(uint8-t  i = 0 ; i < 8 ; i++)
				{
					OLED-WriteByte(OLED-WR-DATA , (uint8-t) ascll_code_8x16[adder][i + 8]);
				}
				break;
			//如果还有其他格式的可自己添加
		default：
		break;
	}
}

void OLED_ShowString(uint8_t y, uint8_t x, uint8_t* string_data, uint8_t textsize)
{
    for(uint8_t i = 0; *string_data != '\0'; i++)
    {
        OLED_ShowChar(y, x + i * textsize, *string_data++, textsize);
    }
}

i 从 0 开始，每轮显示一个字符。
*string_data != '\0' 检查是否已到字符串末尾。
x + i * textsize：每个字符占 textsize 像素宽，因此第 i 个字符的起始列是 x + i * textsize。
*string_data++：取出当前字符，并将指针后移一位。
调用 OLED_ShowChar() 绘制单个字符，该函数内部会根据 textsize 选择字体大小和高度，并正确处理跨页问题（如 8x16 字体会自动占用连续两页）。
举例：如果 textsize=8, x=0，则第 0 个字符在 0~7 列，第 1 个字符在 8~15 列，依此类推。


void  OLED-ShowChar-F8X16(uint8-t  line , uint8-t  offset ， uint8-t  *string-data)
{
	OLED-ShowString(line * 2 ,  offset * 2  ,  string-data  , TEXTSIZE-F8X16);
}

9.把之前 User 中的 dht11 文件夹和 APP 文件夹复制过来，再到 Keil 中的管理项目文件添加分组 APP 以及在 USER 中添加 dht11.c ，魔术棒中添加 APP 路径

10.修改名称，以及头文件名称，添加头文件

11.把 app-dht11.c 中的函数进行修改，并在里面添加展示内容。

12.在 app-dht11-oled.c 中进行输出编写
void DHT11_ReadAndShow(void)
{
	//显示汉字
	OLED_ShowChinese_F16X16(0 , 2 , 0);
	OLED_ShowChinese_F16X16(0 , 3 , 1);
	OLED_ShowChinese_F16X16(0 , 4 , 2);
	if(DHT11_ReadData(&dht11_data) == HAL_OK)
	{	
		OLED_ShowChinese_F16X16(1 , 0 , 3);
		OLED_ShowChinese_F16X16(1 , 1 , 4);//温度
		OLED_ShowString_F8X16(1, 4, (uint8_t *)":");  // 显示冒号“:”
		char temp_data[6];
		sprintf(temp_data , "%.2d.%.1d C" , dht11_data.temp_int , dht11_data.temp_deci);
		OLED_ShowString_F8X16(1 , 5 , (uint8_t *)(temp_data));
		
		OLED_ShowChinese_F16X16(2 , 0 , 5);
		OLED_ShowChinese_F16X16(2 , 1 , 6);//湿度
		OLED_ShowString_F8X16(2, 4, (uint8_t *)":");  // 显示冒号“:”
		char humi_data[6];
		sprintf(humi_data , "%.2d.%.1d C" , dht11_data.humi_int , dht11_data.humi_deci);
		OLED_ShowString_F8X16(2 , 5 , (uint8_t *)(humi_data));
		
		DWT_DelayS(2);//每两秒读一次数据
		OLED_Clear();//清屏等待下一次刷新
	}
	else
	{
		OLED_ShowString_F8X16(0 , 0 , (uint8_t *)"DHT11");
		OLED_ShowString_F8X16(1 , 0 , (uint8_t *)"ERROR!!");
	}
}







