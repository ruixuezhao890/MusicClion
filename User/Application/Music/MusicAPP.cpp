///**
//*********************************************************************
//*********
//* @project_name :MusicClion
//* @file : MusicAPP.cpp
//* @author : zen3
//* @brief : None
//* @attention : None
//* @date : 2024/1/27
//*********************************************************************
//*********
//*/
////
//
//#include "MusicAPP.h"
//#include "ManageApp.h"
//
//MusicAPP::MusicAPP() {
//    MusicPlayer=&g_MusicPlayer;
//}
//MusicAPP::~MusicAPP() {
//
//}
//
//void MusicAPP::onCreate() {
//    setAllowBgRunning(false);
//    mooncake.setAppBaseHandle(getAppName(),this);
//    MusicPlayer->begin();
//    Serial<<"info:"<<MusicPlayer->getPromptMessage()<<endl;
//    Serial<<"Music Num:"<<MusicPlayer->getTotwavnum()<<endl;
//    String pname=(char *)MusicPlayer->getPname();
//    Serial<<"pname:"<<pname<<endl;
//
//}
//
//void MusicAPP::onResume() {
//    mooncake.setCurrentApp(this);
//    mooncake.getPageManage()->page_manage_switch_page(getAppName().c_str());
////    Runflag=1;
////    vTaskResume(MusicTaskHandle);
//}
//
//void MusicAPP::onRunning() {
//
//}
//
//void MusicAPP::onRunningBG() {
//
//}
//
//void MusicAPP::onPause() {
//  mooncake.getPageManage()->page_manage_switch_page(mooncake.getSrceen1());
//    vTaskSuspend(MusicTaskHandle);
//}
//
//void MusicAPP::onDestroy() {
//
//}
//
//
