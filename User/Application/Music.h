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
#include "mp3play.h"
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

protected:
    uint8_t audioPlaySong();
    void audioControl();
    void gainsPname();
    uint16_t getAllMusciNum(String path);
public:
    static void audio_start(void);
    static void audio_stop(void);
    Music();
    Music(String path);
    ~Music();
    void begin();
    void audioPlay();
    u8 *getPname() const;
    u16 getTotwavnum() const;
    const String &getPromptMessage() const;

};
extern __audiodev audiodev;
extern uint8_t Runflag;
extern  Music g_MusicPlayer;
#endif //MUSICCLION_MUSIC_H
