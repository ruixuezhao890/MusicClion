/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : mp3play.h
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/1/30 
*********************************************************************
*********
*/
//

#ifndef MUSICCLION_MP3PLAY_H
#define MUSICCLION_MP3PLAY_H
#include "sys.h"
#include "mp3dec.h"
#include "AudioFileParser.h"
#define MP3_FILE_BUF_SZ    3000	//MP3解码时,文件buf大小
#define MP3_I2S_TX_DMA_BUFSIZE 2304
//ID3V1 标签
typedef __packed struct
{
    u8 id[3];		   	//ID,TAG三个字母
    u8 title[30];		//歌曲名字
    u8 artist[30];		//艺术家名字
    u8 year[4];			//年代
    u8 comment[30];		//备注
    u8 genre;			//流派
}ID3V1_Tag;

//ID3V2 标签头
typedef __packed struct
{
    u8 id[3];		   	//ID
    u8 mversion;		//主版本号
    u8 sversion;		//子版本号
    u8 flags;			//标签头标志
    u8 size[4];			//标签信息大小(不包含标签头10字节).所以,标签大小=size+10.
}ID3V2_TagHead;

//ID3V2.3 版本帧头
typedef __packed struct
{
    u8 id[4];		   	//帧ID
    u8 size[4];			//帧大小
    u16 flags;			//帧标志
}ID3V23_FrameHead;

//MP3 Xing帧信息(没有全部列出来,仅列出有用的部分)
typedef __packed struct
{
    u8 id[4];		   	//帧ID,为Xing/Info
    u8 flags[4];		//存放标志
    u8 frames[4];		//总帧数
    u8 fsize[4];		//文件总大小(不包含ID3)
}MP3_FrameXing;

//MP3 VBRI帧信息(没有全部列出来,仅列出有用的部分)
typedef __packed struct
{
    u8 id[4];		   	//帧ID,为Xing/Info
    u8 version[2];		//版本号
    u8 delay[2];		//延迟
    u8 quality[2];		//音频质量,0~100,越大质量越好
    u8 fsize[4];		//文件总大小
    u8 frames[4];		//文件总帧数
}MP3_FrameVBRI;

class mp3Play: public AudioFileParser{
private:
    u16 outsamples;

    HMP3Decoder  mp3decoder;
    MP3FrameInfo mp3frameinfo;
    FIL  tempFil;
    uint8_t  buffer[MP3_FILE_BUF_SZ];
    uint8_t i2s1[MP3_I2S_TX_DMA_BUFSIZE]={0};
    uint8_t i2s2[MP3_I2S_TX_DMA_BUFSIZE]={0};
    uint8_t tbuf[MP3_I2S_TX_DMA_BUFSIZE]={0};
protected:
    void    mp3FindSynchronization();

    u8 mp3_id3v2_decode(u8* buf,u32 size);

    u8 mp3_id3v1_decode(u8* buf);

public:
    mp3Play();

    ~mp3Play() override;

    void audioGetCurtime(FIL *fx) override;

    uint8_t audioAnalysis(uint8_t *FileName) override;

    uint32_t audioFillBuffer(uint8_t *buf, uint16_t size, uint8_t bits) override;

    uint8_t audioPlaySong(uint8_t *FileName) override;

    uint8_t audioPlaySongInit(uint8_t *FileName) override;

    void audioVarRelease() override;
};


#endif //MUSICCLION_MP3PLAY_H
