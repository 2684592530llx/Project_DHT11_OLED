#ifndef __BSP_OLED_H__
#define __BSP_OLED_H__

#include "main.h"

#define OLED_WR_DATA (0x40)		//写数据
#define OLED_WR_CMD  (0x00)		//写命令

#define OLED_SALVE_ADDER  (0x3c)		//OLED屏幕从机地址
#define IIC_TIMEOUT	  100		//超时时间

#define TEXTSIZE_F16X16 16
#define TEXTSIZE_F8X16 8
#define TEXTSIZE_F6X8 6

void OLED_Init(void);
ErrorStatus OLED_CheckDevice(uint8_t slave_addr);
ErrorStatus OLED_WriteByte(uint8_t cmd , uint8_t byte);
ErrorStatus OLED_WriteBuffer(uint8_t cmd , uint8_t *buffer , uint32_t num);
void OLED_SetPos(uint8_t y , uint8_t x);
void OLED_Fill(uint8_t fill_data);
void OLED_Clear(void);
void OLED_FillFull(void);

void OLED_ShowChinese(uint8_t y , uint8_t x , uint8_t n);
void OLED_ShowChinese_F16X16(uint8_t line , uint8_t offset , uint8_t n);
void OLED_ShowChar(uint8_t y , uint8_t x , uint8_t data_char , uint8_t textsize);
void OLED_ShowChar_F8X16(uint8_t line , uint8_t offset , uint8_t *string_data);
void OLED_ShowString(uint8_t y , uint8_t x , uint8_t *string_data, uint8_t textsize);
void OLED_ShowString_F8X16(uint8_t line , uint8_t offset , uint8_t *string_data);
void OLED_ShowString_F6X8(uint8_t line , uint8_t offset , uint8_t *string_data);

#endif
