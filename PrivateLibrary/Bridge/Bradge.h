/**
*********************************************************************
*********
* @project_name :car0
* @file : Bridge.h
* @author : zen3
* @brief : None
* @attention : None
* @date : 2023/10/24 
*********************************************************************
*********
*/
//

#ifndef CAR0_BRIDGE_H
#define CAR0_BRIDGE_H


#include "stm32f4xx.h"

#ifdef __cplusplus
extern "C"
{
#endif

void setup();
void  loop();
void APPManagerUpData();
extern void MusicTaskFunctionControl();
void HardwareUpData();
#ifdef __cplusplus
}
#endif


#endif //CAR0_BRIDGE_H