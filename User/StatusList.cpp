/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : StatusList.cpp
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/2/4 
*********************************************************************
*********
*/
//
#include "StatusList.h"
uint32_t setMusicStatus(const uint32_t set){
    return osEventFlagsSet(MusicStauteHandle,set);
}