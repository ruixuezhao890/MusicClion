#include "wavplay.h"
#include "MyUsart.h"
#include "Music.h"
#include "stdio.h"
#include "delay.h" 
#include "malloc.h"
#include "ff.h"
#include "i2S2.h"
#include "wm8978.h"
#include "key.h"
//#include "led.h"
//#include "event_groups.h"
//#include "StatusList.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//WAV �������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved				
//********************************************************************************
//V1.0 ˵��
//1,֧��16λ/24λWAV�ļ�����
//2,��߿���֧�ֵ�192K/24bit��WAV��ʽ. 
////////////////////////////////////////////////////////////////////////////////// 	
 
__wavctrl wavctrl;		//WAV���ƽṹ��
vu8 wavtransferend=0;	//i2s������ɱ�־
vu8 wavwitchbuf=0;		//i2sbufxָʾ��־
u8 Playing_Finish_Flag=0;
//WAV������ʼ��
//fname:�ļ�·��+�ļ���
//wavx:wav ��Ϣ��Žṹ��ָ��
//����ֵ:0,�ɹ�;1,���ļ�ʧ��;2,��WAV�ļ�;3,DATA����δ�ҵ�.
u8 wav_decode_init(u8* fname,__wavctrl* wavx)
{
	FIL*ftemp;
	u8 *buf;
    UINT br=0;
	u8 res=0;
	
	ChunkRIFF *riff;
	ChunkFMT *fmt;
	ChunkFACT *fact;
	ChunkDATA *data;
	ftemp=(FIL*)malloc(sizeof(FIL));
	buf=(uint8_t *)malloc(512);
	if(ftemp&&buf)	//�ڴ�����ɹ�
	{
		res=f_open(ftemp,(TCHAR*)fname,FA_READ);//���ļ�
		if(res==FR_OK)
		{
			f_read(ftemp,buf,(UINT)512,&br);	//��ȡ512�ֽ�������
			riff=(ChunkRIFF *)buf;		//��ȡRIFF��
			if(riff->Format==0X45564157)//��WAV�ļ�
			{
				fmt=(ChunkFMT *)(buf+12);	//��ȡFMT�� 
				fact=(ChunkFACT *)(buf+12+8+fmt->ChunkSize);//��ȡFACT��
				if(fact->ChunkID==0X74636166||fact->ChunkID==0X5453494C)wavx->datastart=12+8+fmt->ChunkSize+8+fact->ChunkSize;//����fact/LIST���ʱ��(δ����)
				else wavx->datastart=12+8+fmt->ChunkSize;  
				data=(ChunkDATA *)(buf+wavx->datastart);	//��ȡDATA��
				if(data->ChunkID==0X61746164)//�����ɹ�!
				{
					wavx->audioformat=fmt->AudioFormat;		//��Ƶ��ʽ
					wavx->nchannels=fmt->NumOfChannels;		//ͨ����
					wavx->samplerate=fmt->SampleRate;		//������
					wavx->bitrate=fmt->ByteRate*8;			//�õ�λ��
					wavx->blockalign=fmt->BlockAlign;		//�����
					wavx->bps=fmt->BitsPerSample;			//λ��,16/24/32λ
					
					wavx->datasize=data->ChunkSize;			//���ݿ��С
					wavx->datastart=wavx->datastart+8;		//��������ʼ�ĵط�. 
					 
					Serial.println("wavx->audioformat:%d\r\n",wavx->audioformat);
					Serial.println("wavx->nchannels:%d\r\n",wavx->nchannels);
					Serial.println("wavx->samplerate:%d\r\n",wavx->samplerate);
					Serial.println("wavx->bitrate:%d\r\n",wavx->bitrate);
					Serial.println("wavx->blockalign:%d\r\n",wavx->blockalign);
					Serial.println("wavx->bps:%d\r\n",wavx->bps);
					Serial.println("wavx->datasize:%d\r\n",wavx->datasize);
					Serial.println("wavx->datastart:%d\r\n",wavx->datastart);
				}else res=3;//data����δ�ҵ�.
			}else res=2;//��wav�ļ�
			
		}else res=1;//���ļ�����
	}
	f_close(ftemp);
	free(ftemp);//�ͷ��ڴ�
	free(buf);
	return 0;
}

//���buf
//buf:������
//size:���������
//bits:λ��(16/24)
//����ֵ:���������ݸ���
u32 wav_buffill(u8 *buf,u16 size,u8 bits)
{
	u16 readlen=0;
	u32 bread;
	u16 i;
	u8 *p;
	if(bits==24)//24bit��Ƶ,��Ҫ����һ��
	{
		readlen=(size/4)*3;							//�˴�Ҫ��ȡ���ֽ���
		f_read(audiodev.file,audiodev.tbuf,readlen,(UINT*)&bread);	//��ȡ����
		p=audiodev.tbuf;
		for(i=0;i<size;)
		{
			buf[i++]=p[1];
			buf[i]=p[2];
			i+=2;
			buf[i++]=p[0];
			p+=3;
		}
		bread=(bread*4)/3;		//����Ĵ�С.
	}else
	{
		f_read(audiodev.file,buf,size,(UINT*)&bread);//16bit��Ƶ,ֱ�Ӷ�ȡ����
		if(bread<size)//����������,����0
		{
			for(i=bread;i<size-bread;i++)buf[i]=0;
		}
	}
	return bread;
}
void DMAEx_XferCpltCallback(struct __DMA_HandleTypeDef *hdma){
//    if (wav_buffill(audiodev.i2sbuf1,WAV_I2S_TX_DMA_BUFSIZE,wavctrl.bps)
//    !=WAV_I2S_TX_DMA_BUFSIZE){
//        //todo ��״̬��Ϊ��һ��
//    }
    u16 i;
    wavwitchbuf = 1;
    if ((audiodev.status & 0X01) == 0) {
        for (i = 0; i < WAV_I2S_TX_DMA_BUFSIZE; i++)//��ͣ
        {
            audiodev.i2sbuf2[i] = 0;//���0
        }
    }
    wavtransferend = 1;
}
void DMAEx_XferM1CpltCallback(struct __DMA_HandleTypeDef *hdma){
//    if (wav_buffill(audiodev.i2sbuf2,WAV_I2S_TX_DMA_BUFSIZE,wavctrl.bps)
//        !=WAV_I2S_TX_DMA_BUFSIZE){
//        //todo ��״̬��Ϊ��һ��
//    }
    u16 i;
    wavwitchbuf = 1;
    if ((audiodev.status & 0X01) == 0) {
        for (i = 0; i < WAV_I2S_TX_DMA_BUFSIZE; i++)//��ͣ
        {
            audiodev.i2sbuf2[i] = 0;//���0
        }
    }
    wavtransferend = 1;
}
//WAV����ʱ,I2S DMA����ص�����
void wav_i2s_dma_tx_callback(void) 
{
//    BaseType_t Result,xHigherPriorityTaskWoken;
//    if(__HAL_DMA_GET_FLAG(&I2S2_TXDMA_Handler,DMA_FLAG_TCIF0_4)!=RESET) //DMA�������
//    {
//        __HAL_DMA_CLEAR_FLAG(&I2S2_TXDMA_Handler, DMA_FLAG_TCIF0_4);
        u16 i;
        if (DMA1_Stream4->CR & (1 << 19)) {
            wavwitchbuf = 0;
            if ((audiodev.status & 0X01) == 0) {
                for (i = 0; i < WAV_I2S_TX_DMA_BUFSIZE; i++)//��ͣ
                {
                    audiodev.i2sbuf1[i] = 0;//���0
                }
            }
        } else {
            wavwitchbuf = 1;
            if ((audiodev.status & 0X01) == 0) {
                for (i = 0; i < WAV_I2S_TX_DMA_BUFSIZE; i++)//��ͣ
                {
                    audiodev.i2sbuf2[i] = 0;//���0
                }
            }
        }
       // HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_10);
        wavtransferend = 1;
//    }
//     Result=xEventGroupSetBitsFromISR(EventGroupHandler,EVENTBIT_DMAFinish,&xHigherPriorityTaskWoken);
//    if(Result!=pdFAIL)
//    {
//        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//    }

} 
//�õ���ǰ����ʱ��
//fx:�ļ�ָ��
//wavx:wav���ſ�����
void wav_get_curtime(FIL*fx,__wavctrl *wavx)
{
	long long fpos;  	
 	wavx->totsec=wavx->datasize/(wavx->bitrate/8);	//�����ܳ���(��λ:��) 
	fpos=fx->fptr-wavx->datastart; 					//�õ���ǰ�ļ����ŵ��ĵط� 
	wavx->cursec=fpos*wavx->totsec/wavx->datasize;	//��ǰ���ŵ��ڶ�������?	
}
//����ĳ��WAV�ļ�
//fname:wav�ļ�·��.
//����ֵ:
//KEY0_PRES:��һ��
//KEY1_PRES:��һ��
//����:����
u8 wav_play_song(u8* fname)
{
    Serial0.println("wav_play_song:fname:%s",fname);
    FIL  tempFil;
	u8 key;
	u8 t=0;
	u8 res;
	u32 fillnum;
    uint8_t i2s1[WAV_I2S_TX_DMA_BUFSIZE]={0};
    uint8_t i2s2[WAV_I2S_TX_DMA_BUFSIZE]={0};
    uint8_t tbuf[WAV_I2S_TX_DMA_BUFSIZE]={0};
    audiodev.file=&tempFil;
    audiodev.i2sbuf1=i2s1;
    audiodev.i2sbuf2=i2s2;
    audiodev.tbuf=tbuf;
//    audiodev.tbuf=(uint8_t *)malloc(WAV_I2S_TX_DMA_BUFSIZE);
//	audiodev.file=(FIL*)malloc(sizeof(FIL));
//	audiodev.i2sbuf1=(uint8_t *)malloc(WAV_I2S_TX_DMA_BUFSIZE);
//	audiodev.i2sbuf2=(uint8_t *)malloc(WAV_I2S_TX_DMA_BUFSIZE);
//	audiodev.tbuf=(uint8_t *)malloc(WAV_I2S_TX_DMA_BUFSIZE);
	if(audiodev.file&&audiodev.i2sbuf1&&audiodev.i2sbuf2&&audiodev.tbuf)
	{
		res=wav_decode_init(fname,&wavctrl);//�õ��ļ�����Ϣ

		if(res==0)//�����ļ��ɹ�
		{
			if(wavctrl.bps==16)
			{
				WM8978_I2S_Cfg(2,0);	//�����ֱ�׼,16λ���ݳ���
				I2S2_Init(I2S_STANDARD_PHILIPS,I2S_MODE_MASTER_TX,I2S_CPOL_LOW,I2S_DATAFORMAT_16B_EXTENDED);	//�����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,16λ��չ֡����
			}else if(wavctrl.bps==24)
			{
				WM8978_I2S_Cfg(2,2);	//�����ֱ�׼,24λ���ݳ���
				I2S2_Init(I2S_STANDARD_PHILIPS,I2S_MODE_MASTER_TX,I2S_CPOL_LOW,I2S_DATAFORMAT_24B);	//�����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,24λ����
			}
			I2S2_SampleRate_Set(wavctrl.samplerate);//���ò�����
			I2S2_TX_DMA_Init(audiodev.i2sbuf1,audiodev.i2sbuf2,WAV_I2S_TX_DMA_BUFSIZE/2); //����TX DMA
			i2s_tx_callback=wav_i2s_dma_tx_callback;			//�ص�����ָwav_i2s_dma_callback
			Music::audio_stop();
			res=f_open(audiodev.file,(TCHAR*)fname,FA_READ);	//���ļ�
			if(res==0)
			{
				f_lseek(audiodev.file, wavctrl.datastart);		//�����ļ�ͷ
				fillnum=wav_buffill(audiodev.i2sbuf1,WAV_I2S_TX_DMA_BUFSIZE,wavctrl.bps);
				fillnum=wav_buffill(audiodev.i2sbuf2,WAV_I2S_TX_DMA_BUFSIZE,wavctrl.bps);
				Music::audio_start();
                while (res==0){

                     while(wavtransferend==0);//�ȴ�wav�������;
                            wavtransferend=0;
                    if(fillnum!=WAV_I2S_TX_DMA_BUFSIZE)//���Ž���?
                    {
                        res=KEY0_PRES;
                        break;
                    }
                    if(wavwitchbuf)fillnum=wav_buffill(audiodev.i2sbuf2,WAV_I2S_TX_DMA_BUFSIZE,wavctrl.bps);//���buf2
                    else fillnum=wav_buffill(audiodev.i2sbuf1,WAV_I2S_TX_DMA_BUFSIZE,wavctrl.bps);//���buf1
                    while(1) {
                        key = KEY_Scan(0);
                        if (key == WKUP_PRES)//��ͣ
                        {
                            if (audiodev.status & 0X01)audiodev.status &= ~(1 << 0);
                            else audiodev.status |= 0X01;
                        }
                        if (key == KEY2_PRES || key == KEY0_PRES)//��һ��/��һ��
                        {
                            res = key;
                            break;
                        }
                        wav_get_curtime(audiodev.file, &wavctrl);//�õ���ʱ��͵�ǰ���ŵ�ʱ��
//                        audio_msg_show(wavctrl.totsec,wavctrl.cursec,wavctrl.bitrate);
                        t++;
                        if (t == 20) {
                            t = 0;
                            HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_10);
                            delay_ms(5);
                        }
                        if ((audiodev.status & 0X01) == 0)delay_ms(10);
                        else break;
                    }
                }
                f_close(audiodev.file);
                Music::audio_stop();
			}else res=0XFF;
		}else res=0XFF;
	}else res=0XFF;
//        free(audiodev.tbuf);	//�ͷ��ڴ�
//        free(audiodev.i2sbuf1);//�ͷ��ڴ�
//        free(audiodev.i2sbuf2);//�ͷ��ڴ�
//        free(audiodev.file);	//�ͷ��ڴ�
    audiodev.tbuf=nullptr;
    audiodev.i2sbuf1=nullptr;
    audiodev.i2sbuf2=nullptr;
    audiodev.file=nullptr;
    return res;
}
void wav_free(){
    free(audiodev.tbuf);	//�ͷ��ڴ�
    free(audiodev.i2sbuf1);//�ͷ��ڴ�
    free(audiodev.i2sbuf2);//�ͷ��ڴ�
    free(audiodev.file);	//�ͷ��ڴ�
}
u32 fillnum;

uint8_t Play_State_Flag=1;
u8 wav_play_song_song(u8* fname)
{
    u8 res=0;

    if(((wavtransferend==1)&&(Play_State_Flag)))//�ȴ�wav�������;
    {
        wavtransferend=0;///�����ɱ�־λ

        if(Playing_Finish_Flag==0)
        {
            if(fillnum!=WAV_I2S_TX_DMA_BUFSIZE)//���Ž���?
            {
                Playing_Finish_Flag=0xAB;
            }
            else
            {
                if(wavwitchbuf)fillnum=wav_buffill(audiodev.i2sbuf2,WAV_I2S_TX_DMA_BUFSIZE,wavctrl.bps);//���buf2
                else fillnum=wav_buffill(audiodev.i2sbuf1,WAV_I2S_TX_DMA_BUFSIZE,wavctrl.bps);//���buf1
                wav_get_curtime(audiodev.file,&wavctrl);//�õ���ʱ��͵�ǰ���ŵ�ʱ��
//                audio_msg_show(wavctrl.totsec,wavctrl.cursec,wavctrl.bitrate);

//                audiodev.totsec=wavctrl.totsec;//��������
//                audiodev.cursec=wavctrl.cursec;
//                audiodev.bitrate=wavctrl.bitrate;

//                if(Fast_Forward_Flag==2)
//                {
//                    if(Fast_Forward_50ms_Flag==1)
//                    {
//                        Fast_Forward_50ms_Flag=2;
//                        Fast_Forward_Flag=0;
//                        I2S_Play_Start();
//                    }
//                }
            }
        }

        if(Playing_Finish_Flag==0xAB)
        {
            Music::audio_stop();

            free(audiodev.tbuf);	//�ͷ��ڴ�
            free(audiodev.i2sbuf1);//�ͷ��ڴ�
            free(audiodev.i2sbuf2);//�ͷ��ڴ�
            free(audiodev.file);	//�ͷ��ڴ�

            res=0xAB;
        }
    }

    return res;
}








