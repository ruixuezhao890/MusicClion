/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : Music.h
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/1/27 
*********************************************************************
*********
*/
//

#ifndef MUSICCLION_MUSIC_H
#define MUSICCLION_MUSIC_H
#include "wavplay.h"
#include "WString.h"
#include "ff.h"
#define PATH "0:/MUSIC"
typedef __packed struct
{
    //2个I2S解码的BUF
    u8 *i2sbuf1;
    u8 *i2sbuf2;
    u8 *tbuf;				//零时数组,仅在24bit解码的时候需要用到
    FIL *file;				//音频文件指针
    u8 status;				//bit0:0,暂停播放;1,继续播放
    //bit1:0,结束播放;1,开启播放
}__audiodev;
extern __audiodev audiodev;
class Music {
private:
    u8 *pname;				//带路径的文件名

    u16 totwavnum; 			//音乐文件总数
    u16 curindex;			//当前索引
    u32 *wavoffsettbl;		//音乐offset索引表
    String PromptMessage;   //提示消息
    String _path;           //路径
    DIR wavdir;	 			//目录
    FILINFO *wavfileinfo;	//文件信息

public:
    Music();
    Music(String path);
    ~Music();
    void begin();
    void arduioPlay();
    u16 getTotwavnum() const;
    uint8_t ArduioPlaySong(uint8_t*pname);
    void ArduioControl();
   static void audio_start(void);
   static void audio_stop(void);
   void audioPlay();
    void gainsPname();
    const String &getPromptMessage() const;

    u8 *getPname() const;
protected:
    uint16_t getAllMusciNum(String path);


protected:

};
extern uint8_t Runflag;
//extern TaskHandle_t MusicTaskHandle;
//extern EventGroupHandle_t EventGroupHandler;
extern  Music g_MusicPlayer;
#endif //MUSICCLION_MUSIC_H
