/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : OSTask.cpp
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/2/4 
*********************************************************************
*********
*/
//
//#include "stm32f4xx.h"
#include "hal.h"
#include "key.h"
#include "StatusList.h"
#include "Music.h"
#include "exfuns.h"
//#include "Include.h"
#include "OSTask.h"
#include "cmsis_os.h"
#include "MyUsart.h"
extern "C" void Music(){
    uint32_t flag;g_MusicPlayer.begin();audiodev.playStatus=1;
    setMusicStatus(EVENTBIT_DMAFinish);
    while (1){
       flag=osEventFlagsWait(MusicStauteHandle,EVENTBIT_ALL,osFlagsWaitAny,osWaitForever);
        if(flag&EVENTBIT_Next){
            Serial0<<"EVENTBIT_Next"<<endl;
            g_MusicPlayer.getManage()->audioVarRelease();
            g_MusicPlayer.deleteManage();
            g_MusicPlayer.setCurindex(g_MusicPlayer.getCurindex()+1);
            if (g_MusicPlayer.getCurindex()>=g_MusicPlayer.getTotwavnum()){
                g_MusicPlayer.setCurindex(0);
            }
            Serial<<"Curindex:"<<g_MusicPlayer.getCurindex()<<endl;
            audiodev.status=PlayStatus_Not;
        }
        if (flag&EVENTBIT_Play){
            Serial0<<"EVENTBIT_Play"<<endl;
            if (audiodev.playStatus){
                Music::audio_start();
            }else{
                Music::audio_stop();
            }
        }
        if (flag&EVENTBIT_Prev){
            Serial0<<"EVENTBIT_Prev"<<endl;
            g_MusicPlayer.getManage()->audioVarRelease();
            g_MusicPlayer.deleteManage();
            if (g_MusicPlayer.getCurindex()){
                g_MusicPlayer.setCurindex(g_MusicPlayer.getCurindex()-1);
            }else{
                g_MusicPlayer.setCurindex(g_MusicPlayer.getTotwavnum()-1);
            }
            audiodev.status=PlayStatus_Not;
        }
        if (flag&EVENTBIT_Forward){

        }
        if (audiodev.status==PlayStatus_Not){
            g_MusicPlayer.gainsPname();
            g_MusicPlayer.setCurType(f_typetell(g_MusicPlayer.getPname()));
            if (g_MusicPlayer.audioGreatType()){ continue;}
            audiodev.status=PlayStatus_TypeInit;
            Serial<<"great ok"<<endl;
        }
        if (audiodev.status==PlayStatus_TypeInit){
            if (g_MusicPlayer.getManage()->audioPlaySongInit(g_MusicPlayer.getPname())){
                Serial<<"InitFail"<<endl;
                audiodev.status=PlayStatus_Finish;
            }else{
                audiodev.status=PlayStatus_Playing;
            }
            if (audiodev.playStatus){
               g_MusicPlayer.audio_start();
            }else{
                Music::audio_stop();
            }
        }
        if (audiodev.status==PlayStatus_Playing){
            uint8_t res=g_MusicPlayer.audioPlaySong();
            if (res){//出现错误
                audiodev.status=PlayStatus_Finish;
            }
        }
        if (audiodev.status==PlayStatus_Finish){
            Music::audio_stop();
//            g_MusicPlayer.setCurindex(g_MusicPlayer.getCurindex()+1);
            setMusicStatus(EVENTBIT_Next);
        }
    }
}
extern "C" void KeyScan(){
    KEY_Init();uint8_t key;
    while (1){
        key=KEY_Scan(0);
        if (key==KEY0_PRES){
            Serial<<"press1"<<endl;
            setMusicStatus(EVENTBIT_Next);
        }
        if (key==KEY2_PRES){
            Serial<<"press2"<<endl;
            setMusicStatus(EVENTBIT_Prev);
        }
        if (key==WKUP_PRES){
            Serial<<"press3"<<endl;
            audiodev.playStatus=!audiodev.playStatus;
            setMusicStatus(EVENTBIT_Play);
        }
        osDelay(20);
    }
}