 #ifndef __I2S_H
#define __I2S_H
#include "sys.h"    									
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F407������
//I2S ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/5/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//******************************************************************************** 
//V1.1 20141220  
//����I2S2_SampleRate_Set����ODDλ���õ�bug 	
////////////////////////////////////////////////////////////////////////////////// 	
#ifdef __cplusplus
 extern "C"
{
#endif
/*______________________���������������________________:*/


extern I2S_HandleTypeDef I2S2_Handler;			//I2S2���
extern DMA_HandleTypeDef I2S2_TXDMA_Handler;   //I2S2����DMA���
extern void DMAEx_XferCpltCallback(struct __DMA_HandleTypeDef *hdma);
extern void DMAEx_XferM1CpltCallback(struct __DMA_HandleTypeDef *hdma);

extern void (*i2s_tx_callback)(void);		//IIS TX�ص�����ָ��  

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




















