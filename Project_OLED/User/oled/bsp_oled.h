#ifndef __BSP_OLED_H__
#define __BSP_OLED_H__

#include "main.h"

#define OLED_WR_DATA (0x40)		//写数据
#define OLED_WR_CMD  (0x00)		//写命令

#define OLED_SALVE_ADDER  (0x3c)		//OLED屏幕从机地址
#define IIC_TIMEOUT	  100		//超时时间

void OLED_Init(void);
ErrorStatus OLED_CheckDevice(uint8_t slave_addr);
ErrorStatus OLED_WriteByte(uint8_t cmd , uint8_t byte);
ErrorStatus OLED_WriteBuffer(uint8_t cmd , uint8_t *buffer , uint32_t num);
void OLED_SetPos(uint8_t y , uint8_t x);
void OLED_Fill(uint8_t fill_data);
void OLED_Clear(void);
void OLED_FillFull(void);

#endif
