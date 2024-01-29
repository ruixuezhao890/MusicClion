/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : hal.cpp
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/1/27 
*********************************************************************
*********
*/
//

#include "hal.h"
#include "key.h"
#include "beep.h"
#include "exfuns.h"
#include "fattester.h"
#include "MyUsart.h"
//#include "lvgl.h"
//#include "lv_port_indev.h"
//#include "lv_port_disp.h"
#include "wm8978.h"
#include "Music.h"
hal HardwareManage;
hal::hal() {

}

void hal::begin() {
//    lv_init();
//    lv_port_disp_init();
//    lv_port_indev_init();

    WM8978_Init();
    WM8978_HPvol_Set(40,40);		//耳机音量设置
    WM8978_SPKvol_Set(50);			//喇叭音量设置
    exfuns_init();
    BEEP_Init();
    KEY_Init();


}
