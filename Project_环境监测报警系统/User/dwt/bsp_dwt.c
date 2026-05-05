#include "dwt/bsp_dwt.h"

void DWT_Init(void)
{
	DEMCR |= (uint32_t)DEMCR_TRCENA; // 将DEMCR使能，第24位（DEMCR_TRCENA）置1
	
	DWT_CYCCNT |= (uint32_t)0U; // 将DWT_CYCCNT计数器清零
	
	DWT_CTRL |= (uint32_t)DWT_CTRL_CYCCNTENA; // 将DWT_CTRL控制器使能，将第0位置1
}

uint32_t DWT_GetTick(void) //得到此时的节拍数（时间）
{
	return ((uint32_t)DWT_CYCCNT);
}
	
uint32_t DWT_TickToMs(uint32_t tick , uint32_t frequency)//将节拍数转换为时间Ms
{
	return (uint32_t)(1000000.0/frequency * tick);
}

void DWT_DelayUs(uint32_t time)//实现Us级延时
{
	uint32_t duration = time * (SystemCoreClock / 1000000.0);//设置需要延时的时长
	uint32_t start = DWT_GetTick();
	while( DWT_GetTick() - start < duration);
}

void DWT_DelayMs(uint32_t time)//实现Ms级延时
{
	for(int i = 0 ; i < time ; i++)
	{
		DWT_DelayUs(1000);
	}
}

void DWT_DelayS(uint32_t time)//实现Ms级延时
{
	for(int i = 0 ; i < time ; i++)
	{
		DWT_DelayMs(1000);
	}
}

