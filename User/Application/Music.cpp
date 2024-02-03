/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : Music.cpp
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/1/27 
*********************************************************************
*********
*/
//

#include "Music.h"
#include "wm8978.h"
#include "exfuns.h"
#include "i2S2.h"
#include "key.h"
#include "hal.h"
#include "AudioFileParser.h"
//#include "StatusList.h"
#include "MyUsart.h"
__audiodev audiodev;
uint8_t Runflag=0;
uint8_t ret;
Music g_MusicPlayer(PATH);
//EventBits_t EventValue;
Music::Music() {

}

Music::Music(String path) {
    _path=path;
    totwavnum=0;
}

void Music::begin() {
    uint8_t res;
    uint32_t temp;
    WM8978_ADDA_Cfg(1,0);	//开启DAC
    WM8978_Input_Cfg(0,0,0);//关闭输入通道
    WM8978_Output_Cfg(1,0);	//开启DAC输出
    res=f_mount(fs[0], "0:", 1);
    if (res!=FR_OK){
        PromptMessage="Failed to f_mount:";
        PromptMessage+=res;
        return;
    }
    res=f_opendir(&wavdir,_path.c_str());
    if (res!=FR_OK)//打开音乐文件夹
    {
        PromptMessage="Failed to open folder:0:/MUSIC:";
        PromptMessage+=res;
        return;
    }
    totwavnum= getAllMusciNum(_path); //得到总有效文件数
    Serial0<<"allNUm:"<<totwavnum<<endl;
    if(totwavnum== 0)//音乐文件总数为0
    {
        PromptMessage="No music files/Num==0";
        return;
    }
    wavfileinfo=(FILINFO*)malloc(sizeof(FILINFO));	//申请内存
    pname=(uint8_t *)malloc(_MAX_LFN*2+1);//为带路径的文件名分配内存
    memset(pname,0,_MAX_LFN*2+1);
    wavoffsettbl=(uint32_t *)malloc(4*totwavnum);				//申请4*totwavnum个字节的内存,用于存放音乐文件off block索引
    if (!wavfileinfo||!pname||!wavoffsettbl)//内存分配出错
    {
        PromptMessage="Memory allocation failed";
        return;
    }
    res=f_opendir(&wavdir,_path.c_str()); //打开目录
    if(res==FR_OK)
    {
        curindex=0;//当前索引为0
        while(1)//全部查询一遍
        {
             temp=wavdir.dptr;								//记录当前index
            res=f_readdir(&wavdir,wavfileinfo);       		//读取目录下的一个文件
            if(res!=FR_OK||wavfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出
            res=f_typetell((uint8_t *)wavfileinfo->fname);
            if((res&0XF0)==0X40||(res&0XF0)==0X41)//取高四位,看看是不是音乐文件
            {
                wavoffsettbl[curindex]=temp;//记录索引
                curindex++;
            }
        }
    }else{
        PromptMessage="Failed to open folder:0:/MUSIC";
        PromptMessage+=res;
        return;
    }
    res= f_closedir(&wavdir);
    if (res!=FR_OK){PromptMessage="close failed";
                    PromptMessage+=res;
            return;
    }
    curindex=0;
    PromptMessage="READ OK";

}

uint16_t Music::getAllMusciNum(String path) {
    u8 res;
    u16 rval=0;
    DIR tdir;	 		//临时目录
    FILINFO* tfileinfo;	//临时文件信息
    tfileinfo=(FILINFO*)malloc(sizeof(FILINFO));//申请内存
    res=f_opendir(&tdir,(const TCHAR*)path.c_str()); //打开目录
    if(res==FR_OK&&tfileinfo)
    {
        while(1)//查询总的有效文件数
        {
            res=f_readdir(&tdir,tfileinfo);       			//读取目录下的一个文件
            if(res!=FR_OK||tfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出
            res=f_typetell((u8*)tfileinfo->fname);
            if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件
            {
                rval++;//有效文件数增加1
            }
        }
    }
    free(tfileinfo);//释放内存
    return rval;
}

u16 Music::getTotwavnum() const {
    return totwavnum;
}

const String &Music::getPromptMessage() const {
    return PromptMessage;
}

Music::~Music() {
    free(wavfileinfo);
    free(pname);
    free(wavoffsettbl);
    pname= nullptr;
    wavfileinfo= nullptr;
    wavoffsettbl= nullptr;
}

void Music::audioPlay() {
    g_MusicPlayer.gainsPname();
    g_MusicPlayer.audioPlaySong();
    g_MusicPlayer.audioControl();
}

void Music::gainsPname() {
    uint8_t res= f_opendir(&wavdir,_path.c_str());
    if (res==FR_OK){
        dir_sdi(&wavdir,wavoffsettbl[curindex]);
        res=f_readdir(&wavdir,wavfileinfo);       				//读取目录下的一个文件
        if(res!=FR_OK||wavfileinfo->fname[0]==0)return;			//错误了/到末尾了,退出
        strcpy((char*)pname,"0:/MUSIC/");						//复制路径(目录)
        strcat((char*)pname,(const char*)wavfileinfo->fname);	//将文件名接在后面
    }
     res= f_closedir(&wavdir);
    if (res!=FR_OK){
        PromptMessage="close fail";
    }
}

uint8_t Music::audioPlaySong() {
            AudioFileParser *manage;
            auto res = f_typetell(pname);
            switch (res) {
                case T_WAV:{
                    manage=new wavPlay;
                    break;
                }
                case T_MP3:{
                    manage=new mp3Play;
                    break;
                }
//                    res = wav_play_song(pname);

                default://其他文件,自动跳转到下一曲
                {
                    PromptMessage = "can't play:";
                    PromptMessage += (*pname);
                    ret=res = KEY0_PRES;
                    return res;
                }
            }
            Serial<<"pname:";Serial.println("%s",pname);
            manage->audioPlaySongInit(pname);

            res=manage->audioPlaySong(pname);
            manage->audioVarRelease();
            ret = res;
            delete manage;
            return res;

}

void Music::audioControl() {

    if(ret==KEY2_PRES)		//上一曲
    {
        if(curindex)curindex--;
        else curindex=totwavnum-1;
    }else if(ret==KEY0_PRES)//下一曲
    {
        curindex++;
        if(curindex>=totwavnum)curindex=0;//到末尾的时候,自动从头开始
    }else {
        PromptMessage="KEY error";
    };	//产生了错误
}

u8 *Music::getPname() const {
    return pname;
}

void Music::audio_start(void) {
    audiodev.status=3<<0;//开始播放+非暂停
    I2S_Play_Start();
}

void Music::audio_stop(void) {
    audiodev.status=0;
    I2S_Play_Stop();
}

//void Music::audioPlay() {
//    u8 res;
//    DIR wavdir;	 			//目录
//    FILINFO *wavfileinfo;	//文件信息
//    u8 *pname;				//带路径的文件名
//    u16 totwavnum; 			//音乐文件总数
//    u16 curindex;			//当前索引
//    u8 key;					//键值
//    u32 temp;
//    u32 *wavoffsettbl;		//音乐offset索引表
//    f_mount(fs[0], "0:", 1);
//    WM8978_ADDA_Cfg(1,0);	//开启DAC
//    WM8978_Input_Cfg(0,0,0);//关闭输入通道
//    WM8978_Output_Cfg(1,0);	//开启DAC输出
//
//    while(f_opendir(&wavdir,"0:/MUSIC"))//打开音乐文件夹
//    {
//        Serial0<<"open fail"<<endl;
//        HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_9);
//        HAL_Delay(500);
//    }
//    totwavnum=getAllMusciNum("0:/MUSIC"); //得到总有效文件数
//    while(totwavnum==NULL)//音乐文件总数为0
//    {
//        Serial0<<"num==0"<<endl;
//        HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_9);
//        HAL_Delay(800);
//    }
//    wavfileinfo=(FILINFO*)malloc(sizeof(FILINFO));	//申请内存
//    pname=(u8 *)malloc(_MAX_LFN*2+1);					//为带路径的文件名分配内存
//    this->pname=(u8 *)malloc(_MAX_LFN*2+1);					//为带路径的文件名分配内存
//    wavoffsettbl=(u32 *)malloc(4*totwavnum);				//申请4*totwavnum个字节的内存,用于存放音乐文件off block索引
//    while(!wavfileinfo||!pname||!wavoffsettbl)//内存分配出错
//    {
//        Serial0<<"malloc fail"<<endl;
//        HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_9);
//        HAL_Delay(1000);
//    }
//    //记录索引
//    res=f_opendir(&wavdir,"0:/MUSIC"); //打开目录
//    if(res==FR_OK)
//    {
//        curindex=0;//当前索引为0
//        while(1)//全部查询一遍
//        {
//            temp=wavdir.dptr;								//记录当前index
//            res=f_readdir(&wavdir,wavfileinfo);       		//读取目录下的一个文件
//            if(res!=FR_OK||wavfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出
//            res=f_typetell((u8*)wavfileinfo->fname);
//            if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件
//            {
////                printf("temp:%d\n",temp);
//                wavoffsettbl[curindex]=temp;//记录索引
//                curindex++;
//            }
//        }
//    }
//    curindex=0;											//从0开始显示
//    res=f_opendir(&wavdir,(const TCHAR*)"0:/MUSIC"); 	//打开目录
//    while(res==FR_OK)//打开成功
//    {
//        dir_sdi(&wavdir,wavoffsettbl[curindex]);				//改变当前目录索引
//        res=f_readdir(&wavdir,wavfileinfo);       				//读取目录下的一个文件
//        if(res!=FR_OK||wavfileinfo->fname[0]==0)break;			//错误了/到末尾了,退出
//        strcpy((char*)pname,"0:/MUSIC/");						//复制路径(目录)
//        strcat((char*)pname,(const char*)wavfileinfo->fname);	//将文件名接在后面
//        printf("strcat:pname:%s\n",pname);
//
//        this->pname=pname;
//        key= audioPlaySong(); 			 		//播放这个音频文件
//        if(key==KEY2_PRES)		//上一曲
//        {
//            if(curindex)curindex--;
//            else curindex=totwavnum-1;
//        }else if(key==KEY0_PRES)//下一曲
//        {
//            curindex++;
//            if(curindex>=totwavnum)curindex=0;//到末尾的时候,自动从头开始
//        }else break;	//产生了错误
//    }
//    free(wavfileinfo);			//释放内存
//    free(pname);				//释放内存
//    free(wavoffsettbl);		//释放内存
//}

