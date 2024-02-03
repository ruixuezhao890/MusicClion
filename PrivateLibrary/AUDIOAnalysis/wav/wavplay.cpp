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
//1,支持16位/24位WAV文件播放
//2,最高可以支持到192K/24bit的WAV格式.
 
//__wavctrl wavctrl;		//WAV控制结构体
vu8 wavtransferend=0;	//i2s传输完成标志
vu8 wavwitchbuf=0;		//i2sbufx指示标志
u8 Playing_Finish_Flag=0;
void audio_msg_show(wavPlay * wavplay){
    Serial.println("title:%s",wavplay->getTitle());
    Serial.println("cursec:%d",wavplay->getCursec());
    Serial.println("totsec:%d",wavplay->getTotsec());
    Serial.println("samplerate:%d",wavplay->getSamplerate());

}
//WAV播放时,I2S DMA传输回调函数
void wav_i2s_dma_tx_callback(void)
{
//    BaseType_t Result,xHigherPriorityTaskWoken;
//    if(__HAL_DMA_GET_FLAG(&I2S2_TXDMA_Handler,DMA_FLAG_TCIF0_4)!=RESET) //DMA传输完成
//    {
//        __HAL_DMA_CLEAR_FLAG(&I2S2_TXDMA_Handler, DMA_FLAG_TCIF0_4);
        u16 i;
        if (DMA1_Stream4->CR & (1 << 19)) {
            wavwitchbuf = 0;
            if ((audiodev.status & 0X01) == 0) {
                for (i = 0; i < WAV_I2S_TX_DMA_BUFSIZE; i++)//暂停
                {
                    audiodev.i2sbuf1[i] = 0;//填充0
                }
            }
        } else {
            wavwitchbuf = 1;
            if ((audiodev.status & 0X01) == 0) {
                for (i = 0; i < WAV_I2S_TX_DMA_BUFSIZE; i++)//暂停
                {
                    audiodev.i2sbuf2[i] = 0;//填充0
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
////得到当前播放时间
////fx:文件指针
////wavx:wav播放控制器
//void wav_get_curtime(FIL*fx,__wavctrl *wavx)
//{
//	long long fpos;
// 	wavx->totsec=wavx->datasize/(wavx->bitrate/8);	//歌曲总长度(单位:秒)
//	fpos=fx->fptr-wavx->datastart; 					//得到当前文件播放到的地方
//	wavx->cursec=fpos*wavx->totsec/wavx->datasize;	//当前播放到第多少秒了?
//}

uint8_t wavPlay::audioAnalysis(uint8_t *FileName) {

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
	if(ftemp&&buf)	//内存申请成功
	{
		res=f_open(ftemp,(TCHAR*)FileName,FA_READ);//打开文件
		if(res==FR_OK)
		{
			f_read(ftemp,buf,(UINT)512,&br);	//读取512字节在数据
			riff=(ChunkRIFF *)buf;		//获取RIFF块
			if(riff->Format==0X45564157)//是WAV文件
			{
				fmt=(ChunkFMT *)(buf+12);	//获取FMT块
				fact=(ChunkFACT *)(buf+12+8+fmt->ChunkSize);//读取FACT块
				if(fact->ChunkID==0X74636166||fact->ChunkID==0X5453494C)datastart=12+8+fmt->ChunkSize+8+fact->ChunkSize;//具有fact/LIST块的时候(未测试)
				else datastart=12+8+fmt->ChunkSize;
				data=(ChunkDATA *)(buf+datastart);	//读取DATA块
				if(data->ChunkID==0X61746164)//解析成功!
				{
					audioformat=fmt->AudioFormat;		//音频格式
					nchannels=fmt->NumOfChannels;		//通道数
					samplerate=fmt->SampleRate;		//采样率
					bitrate=fmt->ByteRate*8;			//得到位速
					blockalign=fmt->BlockAlign;		//块对齐
					bps=fmt->BitsPerSample;			//位数,16/24/32位

					datasize=data->ChunkSize;			//数据块大小
					datastart=datastart+8;		//数据流开始的地方.

					Serial.println("wavx->audioformat:%d\r\n",audioformat);
					Serial.println("wavx->nchannels:%d\r\n",nchannels);
					Serial.println("wavx->samplerate:%d\r\n",samplerate);
					Serial.println("wavx->bitrate:%d\r\n",bitrate);
					Serial.println("wavx->blockalign:%d\r\n",blockalign);
					Serial.println("wavx->bps:%d\r\n",bps);
					Serial.println("wavx->datasize:%d\r\n",datasize);
					Serial.println("wavx->datastart:%d\r\n",datastart);
				}else res=3;//data区域未找到.
			}else res=2;//非wav文件

		}else res=1;//打开文件错误
	}
	f_close(ftemp);
	free(ftemp);//释放内存
	free(buf);
	return 0;
}

uint32_t wavPlay::audioFillBuffer(uint8_t *buf, uint16_t size, uint8_t bits) {
    u16 readlen=0;
    u32 bread;
    u16 i;
    u8 *p;
    if(bits==24)//24bit音频,需要处理一下
    {
        readlen=(size/4)*3;							//此次要读取的字节数
        f_read(audiodev.file,audiodev.tbuf,readlen,(UINT*)&bread);	//读取数据
        p=audiodev.tbuf;
        for(i=0;i<size;)
        {
            buf[i++]=p[1];
            buf[i]=p[2];
            i+=2;
            buf[i++]=p[0];
            p+=3;
        }
        bread=(bread*4)/3;		//填充后的大小.
    }else
    {
        f_read(audiodev.file,buf,size,(UINT*)&bread);//16bit音频,直接读取数据
        if(bread<size)//不够数据了,补充0
        {
            for(i=bread;i<size-bread;i++)buf[i]=0;
        }
    }
    return bread;
}

uint8_t wavPlay::audioPlaySong(uint8_t *FileName) {
    uint8_t res=0;uint8_t t=0;
    uint8_t key;
    res=f_open(audiodev.file,(TCHAR*)FileName,FA_READ);	//打开文件

    while (res==0){
        while(wavtransferend==0);//等待wav传输完成;
        wavtransferend=0;
        if(fillnum!=WAV_I2S_TX_DMA_BUFSIZE)//播放结束?
        {
            res=KEY0_PRES;
            break;
        }
        if(wavwitchbuf)fillnum=audioFillBuffer(audiodev.i2sbuf2,WAV_I2S_TX_DMA_BUFSIZE,bps);//填充buf2
        else fillnum=audioFillBuffer(audiodev.i2sbuf1,WAV_I2S_TX_DMA_BUFSIZE,bps);//填充buf1
        while(1) {
            key = KEY_Scan(0);
            if (key == WKUP_PRES)//暂停
            {
                if (audiodev.status & 0X01)audiodev.status &= ~(1 << 0);
                else audiodev.status |= 0X01;
            }
            if (key == KEY2_PRES || key == KEY0_PRES)//下一曲/上一曲
            {
                res = key;
                break;
            }
            audioGetCurtime(audiodev.file);
            audio_msg_show(this);
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
    return res;
}

uint8_t wavPlay::audioPlaySongInit(uint8_t *FileName) {
    uint8_t res;
    audiodev.file=&tempFil;
    audiodev.i2sbuf1=i2s1;
    audiodev.i2sbuf2=i2s2;
    audiodev.tbuf=tbuf;
    audioGetName(FileName);
    if(audiodev.file&&audiodev.i2sbuf1&&audiodev.i2sbuf2&&audiodev.tbuf){
        res= audioAnalysis(FileName);
        if (res==0){
            if(bps==16)
            {
                WM8978_I2S_Cfg(2,0);	//飞利浦标准,16位数据长度
                I2S2_Init(I2S_STANDARD_PHILIPS,I2S_MODE_MASTER_TX,I2S_CPOL_LOW,I2S_DATAFORMAT_16B_EXTENDED);	//飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度
            }else if(bps==24)
            {
                WM8978_I2S_Cfg(2,2);	//飞利浦标准,24位数据长度
                I2S2_Init(I2S_STANDARD_PHILIPS,I2S_MODE_MASTER_TX,I2S_CPOL_LOW,I2S_DATAFORMAT_24B);	//飞利浦标准,主机发送,时钟低电平有效,24位长度
            }
            I2S2_SampleRate_Set(samplerate);//设置采样率
            I2S2_TX_DMA_Init(audiodev.i2sbuf1,audiodev.i2sbuf2,WAV_I2S_TX_DMA_BUFSIZE/2); //配置TX DMA
            i2s_tx_callback=wav_i2s_dma_tx_callback;			//回调函数指wav_i2s_dma_callback
            Music::audio_stop();
            res=f_open(audiodev.file,(TCHAR*)FileName,FA_READ);	//打开文件
            if(res==0) {
                f_lseek(audiodev.file, datastart);        //跳过文件头
                fillnum = audioFillBuffer(audiodev.i2sbuf1, WAV_I2S_TX_DMA_BUFSIZE, bps);
                fillnum = audioFillBuffer(audiodev.i2sbuf2, WAV_I2S_TX_DMA_BUFSIZE, bps);
                Music::audio_start();
            }else{
                res=0xFF;
            }
        }else{
            res=0xFF;
        }
    }else{
        res=0xFF;
    }
    f_close(audiodev.file);
    return res;
}

void wavPlay::audioVarRelease() {
    audiodev.tbuf=nullptr;
    audiodev.i2sbuf1=nullptr;
    audiodev.i2sbuf2=nullptr;
    audiodev.file=nullptr;
}

wavPlay::~wavPlay() {

}

void wavPlay::audioGetCurtime(FIL *fx) {
    long long fpos;
    totsec=datasize/(bitrate/8);	//歌曲总长度(单位:秒)
    fpos=fx->fptr-datastart; 					//得到当前文件播放到的地方
    cursec=fpos*totsec/datasize;	//当前播放到第多少秒了?
}

void wavPlay::audioGetName(uint8_t* pname) {
    String substring = "0:/MUSIC/";
    String substring1 = ".wav";
    String tempPname=(char *)pname;
    int index = tempPname.indexOf(substring);
    if (index != -1) {
        tempPname.remove(index, substring.length());
    }
    index=tempPname.indexOf(substring1);
    if (index!=-1){
        tempPname.remove(index,substring1.length());
        strcpy((char *)title,tempPname.c_str());
    }
}
