/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : mp3play.cpp
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/1/30 
*********************************************************************
*********
*/
//

#include "mp3play.h"
#include "MyUsart.h"
#include "i2S2.h"
#include "wm8978.h"
#include "Music.h"
#include "key.h"
#include "StatusList.h"
uint32_t ucFreq=2;
uint8_t mp3transferend=1;   //i2s传输完成指示标志
uint8_t mp3witchbuf=0;		//i2sbufx指示标志
short mp3Play::outbuffer[2][MP3_I2S_TX_DMA_BUFSIZE]={0};  /* 解码输出缓冲区，也是I2S输入数据，实际占用字节数：RECBUFFER_SIZE*2 */
void mp3_i2s_dma_tx_callback(void){
    uint16_t i;
    if(DMA1_Stream4->CR&(1<<19))
    {
    mp3witchbuf=0;
#if 0
    if((audiodev.status&0X01)==0)//暂停了,填充0
		{
			for(i=0;i<2304*2;i++)mp3Play::outbuffer[0][i]=0;
		}
#endif
}else
{
mp3witchbuf=1;
#if 0
if((audiodev.status&0X01)==0)//暂停了,填充0
		{
			for(i=0;i<2304*2;i++)mp3Play::outbuffer[1][i]=0;
		}
#endif
}
mp3transferend=1;
}

mp3Play::mp3Play() {

}

mp3Play::~mp3Play() {

}

void mp3Play::audioGetCurtime(FIL *fx) {

}
u8 mp3Play::mp3_id3v2_decode(u8 *buf, u32 size) {
    ID3V2_TagHead *taghead;
    ID3V23_FrameHead *framehead;
    u32 t;
    u32 tagsize;	//tag大小
    u32 frame_size;	//帧大小
    taghead=(ID3V2_TagHead*)buf;
    if(strncmp("ID3",(const char*)taghead->id,3)==0)//存在ID3?
    {
        tagsize=((u32)taghead->size[0]<<21)|((u32)taghead->size[1]<<14)|((u16)taghead->size[2]<<7)|taghead->size[3];//得到tag 大小
        datastart=tagsize;		//得到mp3数据开始的偏移量
        if(tagsize>size)tagsize=size;	//tagsize大于输入bufsize的时候,只处理输入size大小的数据
        if(taghead->mversion<3)
        {
            Serial.print("not supported mversion!\r\n");
            return 1;
        }
        t=10;
        while(t<tagsize)
        {
            framehead=(ID3V23_FrameHead*)(buf+t);
            frame_size=((u32)framehead->size[0]<<24)|((u32)framehead->size[1]<<16)|((u32)framehead->size[2]<<8)|framehead->size[3];//得到帧大小
            if (strncmp("TT2",(char*)framehead->id,3)==0||strncmp("TIT2",(char*)framehead->id,4)==0)//找到歌曲标题帧,不支持unicode格式!!
            {
                strncpy((char*)title,(char*)(buf+t+sizeof(ID3V23_FrameHead)+1),AUDIO_MIN(frame_size-1,FILESIZEMAX-1));
            }
            if (strncmp("TP1",(char*)framehead->id,3)==0||strncmp("TPE1",(char*)framehead->id,4)==0)//找到歌曲艺术家帧
            {
                strncpy((char*)artist,(char*)(buf+t+sizeof(ID3V23_FrameHead)+1),AUDIO_MIN(frame_size-1,FILESIZEMAX-1));
            }
            t+=frame_size+sizeof(ID3V23_FrameHead);
        }
    }else datastart=0;//不存在ID3,mp3数据是从0开始
    return 0;
}

u8 mp3Play::mp3_id3v1_decode(u8 *buf) {
    ID3V1_Tag *id3v1tag;
    id3v1tag=(ID3V1_Tag*)buf;
    if (strncmp("TAG",(char*)id3v1tag->id,3)==0)//是MP3 ID3V1 TAG
    {
        if(id3v1tag->title[0])strncpy((char*)title,(char*)id3v1tag->title,30);
        if(id3v1tag->artist[0])strncpy((char*)artist,(char*)id3v1tag->artist,30);
    }else return 1;
    return 0;
}
uint8_t mp3Play::audioAnalysis(uint8_t *FileName) {
    HMP3Decoder decoder;
    MP3FrameInfo frame_info;
    MP3_FrameXing* fxing;
    MP3_FrameVBRI* fvbri;
    FIL*fmp3;
    u8 *buf;
    u32 br;
    u8 res;
    int offset=0;
    u32 p;
    short samples_per_frame;	//一帧的采样个数
    u32 totframes;				//总帧数
    uint16_t fsize;
    fmp3=new FIL;
    buf=new uint8_t [5*1024];		//申请5K内存
    if(fmp3&&buf)//内存申请成功
    {
        f_open(fmp3,(const TCHAR*)FileName,FA_READ);//打开文件
        res=f_read(fmp3,(char*)buf,5*1024,(UINT*)&br);
        fsize= f_size(fmp3);
        if(res==0)//读取文件成功,开始解析ID3V2/ID3V1以及获取MP3信息
        {
            mp3_id3v2_decode(buf,br);	//解析ID3V2数据
            f_lseek(fmp3,fsize-128);	//偏移到倒数128的位置
            f_read(fmp3,(char*)buf,128,(UINT*)&br);//读取128字节
            mp3_id3v1_decode(buf);	//解析ID3V1数据
            decoder=MP3InitDecoder(); 		//MP3解码申请内存
            f_lseek(fmp3,datastart);	//偏移到数据开始的地方
            f_read(fmp3,(char*)buf,5*1024,(UINT*)&br);	//读取5K字节mp3数据
            offset=MP3FindSyncWord(buf,(int)br);	//查找帧同步信息
            if(offset>=0&&MP3GetNextFrameInfo(decoder,&frame_info,&buf[offset])==0)//找到帧同步信息了,且下一阵信息获取正常
            {
                p=offset+4+32;
                fvbri=(MP3_FrameVBRI*)(buf+p);
                if(strncmp("VBRI",(char*)fvbri->id,4)==0)//存在VBRI帧(VBR格式)
                {
                    if (frame_info.version==MPEG1)samples_per_frame=1152;//MPEG1,layer3每帧采样数等于1152
                    else samples_per_frame=576;//MPEG2/MPEG2.5,layer3每帧采样数等于576
                    totframes=((u32)fvbri->frames[0]<<24)|((u32)fvbri->frames[1]<<16)|((u16)fvbri->frames[2]<<8)|fvbri->frames[3];//得到总帧数
                    totsec=totframes*samples_per_frame/frame_info.samprate;//得到文件总长度
                }else	//不是VBRI帧,尝试是不是Xing帧(VBR格式)
                {
                    if (frame_info.version==MPEG1)	//MPEG1
                    {
                        p=frame_info.nChans==2?32:17;
                        samples_per_frame = 1152;	//MPEG1,layer3每帧采样数等于1152
                    }else
                    {
                        p=frame_info.nChans==2?17:9;
                        samples_per_frame=576;		//MPEG2/MPEG2.5,layer3每帧采样数等于576
                    }
                    p+=offset+4;
                    fxing=(MP3_FrameXing*)(buf+p);
                    if(strncmp("Xing",(char*)fxing->id,4)==0||strncmp("Info",(char*)fxing->id,4)==0)//是Xng帧
                    {
                        if(fxing->flags[3]&0X01)//存在总frame字段
                        {
                            totframes=((u32)fxing->frames[0]<<24)|((u32)fxing->frames[1]<<16)|((u16)fxing->frames[2]<<8)|fxing->frames[3];//得到总帧数
                         totsec=totframes*samples_per_frame/frame_info.samprate;//得到文件总长度
                        }else	//不存在总frames字段
                        {
                        totsec=fsize/(frame_info.bitrate/8);
                        }
                    }else 		//CBR格式,直接计算总播放时间
                    {
                        totsec=fsize/(frame_info.bitrate/8);
                    }
                }
                bitrate=frame_info.bitrate;			//得到当前帧的码率
                samplerate=frame_info.samprate; 	//得到采样率.
                if(frame_info.nChans==2)outsamples=frame_info.outputSamps; //输出PCM数据量大小
                else outsamples=frame_info.outputSamps*2; //输出PCM数据量大小,对于单声道MP3,直接*2,补齐为双声道输出
            }else res=0XFE;//未找到同步帧
            MP3FreeDecoder(decoder);//释放内存
        }
        f_close(fmp3);
    }else res=0XFF;
    delete fmp3;
    delete buf;
    return res;
}

uint32_t mp3Play::audioFillBuffer(uint8_t *buf, uint16_t size, uint8_t bits) {
    //未用但是纯虚函数必须继承
    return 0;
}
uint8_t mp3Play::audioPlaySong(uint8_t *FileName) {
//先完成解码，然后对相应的数组进行填充
    uint8_t res=0,err=0;uint8_t result=0;uint8_t i=0;uint32_t outputSamps=0;
    /* 进入主程序循环体 (mp3player.ucStatus == STA_PLAYING*/
    if (mp3transferend==1&&audiodev.playStatus==1)
    {
        mp3transferend=0;
        //寻找帧同步，返回第一个同步字的位置
        read_offset = MP3FindSyncWord(read_ptr, bytes_left);
        //没有找到同步字
        if(read_offset < 0)
        {
            result=f_read(audiodev.file,buffer,MP3_FILE_BUF_SZ,&bw1);
            if(result!=FR_OK)
            {
                printf("读取%s失败 -> %d\r\n",FileName,result);
                res=PlayStatus_Finish;
                return res;
            }
            read_ptr=buffer;
            bytes_left=bw1;
            return 0;
        }

        read_ptr += read_offset;					//偏移至同步字的位置
        bytes_left -= read_offset;				//同步字之后的数据大小
        if(bytes_left < 1024)							//补充数据
        {
            /* 注意这个地方因为采用的是DMA读取，所以一定要4字节对齐  */
            i=(uint32_t)(bytes_left)&3;									//判断多余的字节
            if(i) i=4-i;														//需要补充的字节
            memcpy(buffer+i, read_ptr, bytes_left);	//从对齐位置开始复制
            read_ptr = buffer+i;										//指向数据对齐位置
            //补充数据
            result = f_read(audiodev.file, buffer+bytes_left+i, MP3_FILE_BUF_SZ-bytes_left-i, &bw1);
            bytes_left += bw1;										//有效数据流大小
        }
        //开始解码 参数：mp3解码结构体、输入流指针、输入流大小、输出流指针、数据格式
        err = MP3Decode(mp3decoder, &read_ptr, &bytes_left, outbuffer[mp3witchbuf], 0);
        //错误处理
        if (err != ERR_MP3_NONE)
        {
            switch (err)
            {
                case ERR_MP3_INDATA_UNDERFLOW:
                    printf("ERR_MP3_INDATA_UNDERFLOW\r\n");
                    result = f_read(audiodev.file, buffer, MP3_FILE_BUF_SZ, &bw1);
                    read_ptr = buffer;
                    bytes_left = bw1;
                    break;
                case ERR_MP3_MAINDATA_UNDERFLOW:
                    /* do nothing - next call to decode will provide more mainData */
                    printf("ERR_MP3_MAINDATA_UNDERFLOW\r\n");
                    break;
                default:
                    printf("UNKNOWN ERROR:%d\r\n", err);
                    res=0;
                    // 跳过此帧
                    if (bytes_left > 0)
                    {
                        bytes_left --;
                        read_ptr ++;
                    }
                    break;
            }
            mp3transferend=1;
        }
        else		//解码无错误，准备把数据输出到PCM
        {
            MP3GetLastFrameInfo(mp3decoder, &mp3frameinfo);		//获取解码信息
            /* 输出到DAC */
            outputSamps = mp3frameinfo.outputSamps;							//PCM数据个数
            if (outputSamps > 0)
            {

                if (mp3frameinfo.nChans == 1)	//单声道
                {
                    //单声道数据需要复制一份到另一个声道
                    for (i = outputSamps - 1; i >= 0; i--)
                    {
                        outbuffer[mp3witchbuf][i * 2] = outbuffer[mp3witchbuf][i];
                        outbuffer[mp3witchbuf][i * 2 + 1] = outbuffer[mp3witchbuf][i];
                    }
                    outputSamps *= 2;
                }//if (Mp3FrameInfo.nChans == 1)	//单声道
            }//if (outputSamps > 0)
            /* 根据解码信息设置采样率 */
            if (mp3frameinfo.samprate != ucFreq)	//采样率
            {
                ucFreq = mp3frameinfo.samprate;
                printf(" \r\n Bitrate       %dKbps", mp3frameinfo.bitrate/1000);
                printf(" \r\n Samprate      %dHz", samplerate);
                printf(" \r\n BitsPerSample %db", mp3frameinfo.bitsPerSample);
                printf(" \r\n nChans        %d", mp3frameinfo.nChans);
                printf(" \r\n Layer         %d", mp3frameinfo.layer);
                printf(" \r\n Version       %d", mp3frameinfo.version);
                printf(" \r\n OutputSamps   %d", mp3frameinfo.outputSamps);
                printf("\r\n");
                //I2S_AudioFreq_Default = 2，正常的帧，每次都要改速率
                if(samplerate!=mp3frameinfo.samprate)
                {
                    //根据采样率修改I2S速率
                    I2S2_Init(I2S_STANDARD_PHILIPS,I2S_MODE_MASTER_TX,I2S_CPOL_LOW,I2S_DATAFORMAT_16B_EXTENDED);	//飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度
                    I2S2_SampleRate_Set(samplerate);		//设置采样率
                    I2S2_TX_DMA_Init((uint8_t *)outbuffer[0],(uint8_t *)outbuffer[1],outsamples);//配置TX DMA
                }
                Music::audio_start();
            }
        }//else 解码正常
        if(audiodev.file->fptr==audiodev.file->obj.objsize) 		//mp3文件读取完成，退出
        {
            printf("END\r\n");
            res=KEY0_PRES;
//            MP3FreeDecoder(mp3decoder);
//            f_close(audiodev.file);
//            ucFreq=0;
            return res;
        }

#if 0
        while(1)	//按键检测
        {
            key=KEY_Scan(0);
            if(key==WKUP_PRES)//暂停
            {
                if(audiodev.status&0X01){
                    I2S_Play_Stop();
                    audiodev.status&=~(1<<0);
                }
                else{
                    I2S_Play_Start();
                    audiodev.status|=0X01;
                }
            }
            if(key==KEY2_PRES||key==KEY0_PRES)//下一曲/上一曲
            {
                res=key;
//                mp3player.ucStatus = STA_IDLE;
                break;
            }
//            mp3_get_curtime(&file_1,mp3ctrl); //获取解码信息
//            audio_msg_show(mp3ctrl->totsec,mp3ctrl->cursec,mp3ctrl->bitrate);
            t++;
            if(t==20)
            {
                t=0;
                HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_9);
            }
            if((audiodev.status&0X01)==0){

                HAL_Delay(10);
            }
            else break;
        }
#endif
    }
    return res;
}

uint8_t mp3Play::audioPlaySongInit(uint8_t *FileName) {
    uint8_t res=0;
    audiodev.file=&tempFil;
    audiodev.i2sbuf1=(uint8_t *)outbuffer[0];
    audiodev.i2sbuf2=(uint8_t *)outbuffer[1];
    if(!audiodev.file||!audiodev.i2sbuf1||!audiodev.i2sbuf2)//内存申请失败
    {
        audiodev.file=nullptr;
        audiodev.i2sbuf1=nullptr;
        audiodev.i2sbuf2=nullptr;
        audiodev.tbuf=nullptr;
        return 0xff;
    }
    res=f_open(audiodev.file,(TCHAR*)FileName,FA_READ);
    if(res!=FR_OK)
    {
        printf("Open mp3file :%s fail!!!->%d\r\n",FileName,res);
        res = f_close(audiodev.file);
        return 0xf0;	/* 停止播放 */
    }
    printf("当前播放文件 -> %s\n",FileName);
    res=audioAnalysis(FileName);
    if(res!=0)return 0xf4;
    //初始化MP3解码器
    mp3decoder = MP3InitDecoder();
    if(mp3decoder==0)
    {
        printf("初始化helix解码库设备\n");
        return 0xf1;	/* 停止播放 */
    }
    printf("初始化中...\n");

    HAL_Delay(10);	/* 延迟一段时间，等待I2S中断结束 */

    WM8978_ADDA_Cfg(1,0);	//开启DAC
    WM8978_Input_Cfg(0,0,0);//关闭输入通道
    WM8978_Output_Cfg(1,0);	//开启DAC输出
    WM8978_I2S_Cfg(2,0);	//飞利浦标准,16位数据长度

    /*  初始化并配置I2S  */
    I2S_Play_Stop();
    I2S2_Init(I2S_STANDARD_PHILIPS,I2S_MODE_MASTER_TX,I2S_CPOL_LOW,I2S_DATAFORMAT_16B_EXTENDED);	//飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度
    I2S2_SampleRate_Set(samplerate);		//设置采样率
    I2S2_TX_DMA_Init((uint8_t *)outbuffer[0],(uint8_t *)outbuffer[1],MP3_I2S_TX_DMA_BUFSIZE);//配置TX DMA
    i2s_tx_callback=mp3_i2s_dma_tx_callback;		//回调函数指向mp3_i2s_dma_tx_callback
    mp3transferend=1;
    res=f_read(audiodev.file,buffer,MP3_FILE_BUF_SZ,&bw1);
    if(res!=FR_OK)
    {
        printf("读取%s失败 -> %d\r\n",FileName,res);
        MP3FreeDecoder(mp3decoder);
       res=PlayStatus_Finish;
    }
    read_ptr=buffer;
    bytes_left=bw1;
    return res;
}

void mp3Play::audioVarRelease() {
    ucFreq=0;
    MP3FreeDecoder(mp3decoder);
    f_close(audiodev.file);
    audiodev.file=nullptr;
    audiodev.i2sbuf1=nullptr;
    audiodev.i2sbuf2=nullptr;
    audiodev.tbuf=nullptr;
}




