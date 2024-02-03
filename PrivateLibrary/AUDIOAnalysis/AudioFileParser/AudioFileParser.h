/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : AudioFileParser.h
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/1/30 
*********************************************************************
*********
*/
//

#ifndef MUSICCLION_AUDIOFILEPARSER_H
#define MUSICCLION_AUDIOFILEPARSER_H
#include "stm32f4xx.h"
#include "ff.h"
#define MINIMP3_IMPLEMENTATION
#define FILESIZEMAX 32
#define AUDIO_MIN(x,y)	((x)<(y)? (x):(y))
class AudioFileParser {
protected:
    uint8_t title[FILESIZEMAX];     //歌曲名字
    uint8_t artist[FILESIZEMAX];    //歌曲作家（wav格式无）
    uint32_t totsec ;				//整首歌时长,单位:秒
    uint32_t cursec ;				//当前播放时长
    uint32_t bitrate;	   			//比特率(位速)
    uint32_t samplerate;			//采样率
    uint32_t datastart;				//数据帧开始的位置(在文件里面的偏移)
public:
    AudioFileParser()=default;
    virtual ~AudioFileParser();
    virtual void audioGetCurtime(FIL*fx)=0;
    virtual uint8_t audioAnalysis(uint8_t *FileName) =0;
    virtual uint32_t audioFillBuffer(uint8_t *buf, uint16_t size, uint8_t bits)=0;
    virtual uint8_t audioPlaySong(uint8_t* FileName)=0;
    virtual uint8_t audioPlaySongInit(uint8_t* FileName)=0;
    virtual void audioVarRelease()=0;

    const uint8_t *getTitle() const;

    const uint8_t *getArtist() const;

    uint32_t getTotsec() const;

    void setTotsec(uint32_t totsec);

    uint32_t getCursec() const;

    void setCursec(uint32_t cursec);

    uint32_t getBitrate() const;

    void setBitrate(uint32_t bitrate);

    uint32_t getSamplerate() const;

    void setSamplerate(uint32_t samplerate);

    uint32_t getDatastart() const;

    void setDatastart(uint32_t datastart);
};


#endif //MUSICCLION_AUDIOFILEPARSER_H
