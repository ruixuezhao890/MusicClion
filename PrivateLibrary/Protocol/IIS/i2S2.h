 #ifndef __I2S_H
#define __I2S_H
#include "sys.h"    									
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//I2S 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2017/5/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
//******************************************************************************** 
//V1.1 20141220  
//修正I2S2_SampleRate_Set函数ODD位设置的bug 	
////////////////////////////////////////////////////////////////////////////////// 	
#ifdef __cplusplus
 extern "C"
{
#endif
/*______________________类的声明在这下面________________:*/


extern I2S_HandleTypeDef I2S2_Handler;			//I2S2句柄
extern DMA_HandleTypeDef I2S2_TXDMA_Handler;   //I2S2发送DMA句柄
extern void DMAEx_XferCpltCallback(struct __DMA_HandleTypeDef *hdma);
extern void DMAEx_XferM1CpltCallback(struct __DMA_HandleTypeDef *hdma);

extern void (*i2s_tx_callback)(void);		//IIS TX回调函数指针

void I2S2_Init(u32 I2S_Standard,u32 I2S_Mode,u32 I2S_Clock_Polarity,u32 I2S_DataFormat);
//void I2S2_Init(u8 std,u8 mode,u8 cpol,u8 datalen);  
u8 I2S2_SampleRate_Set(u32 samplerate);
void I2S2_TX_DMA_Init(u8* buf0,u8 *buf1,u16 num);
HAL_StatusTypeDef HAL_I2S_Transmit_DMAEx(I2S_HandleTypeDef *hi2s, uint16_t *FirstBuffer, uint16_t *SecondBuffer, uint16_t Size);
void I2S_Play_Start(void); 
void I2S_Play_Stop(void);
#ifdef __cplusplus
 }
#endif
#endif





















