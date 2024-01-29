/**
*********************************************************************
*********
* @project_name :2048
* @file : test.cpp
* @author : zen3
* @brief : None
* @attention : None
* @date : 2023/12/14
*********************************************************************
*********
*/
//

#include "ManageApp.h"
#include "MusicPaker.h"
#include "fattester.h"
#include "hal.h"
//#include "FreeRTOS.h"
//#include "task.h"
//#include "cmsis_os.h"

DesktopInitiator mooncake;
static void win1_contanierInit(lv_obj_t* page);
static void win2_contanierInit(lv_obj_t* page);
static void APPIconCloseInit(lv_obj_t* page);
static void APPIconStartCallBack(lv_event_t * e);
static void APPIconStopCallBack(lv_event_t * e);


 extern "C" void APPManagerUpData() {
//    HardwareManage.begin();
    mooncake.init();
    mooncake.installApp(new MusicPaker("music", nullptr));
//    mooncake.installApp(new Game2048_paker("4096", nullptr));
//    mooncake.installApp(new Game2048_paker("5096", nullptr));
    mooncake.DesktopInitiatorInit();
    while (1){
        mooncake.update();
        osDelay(5);
    }
}
void DesktopInitiator::createAPP() {
    for (int i = 0; i < mooncake.getInstalledAppNum(); ++i) {
      mooncake.createApp(mooncake.getInstalledAppList()[i]);
    }
}

void DesktopInitiator::DesktopInitiatorInit() {createAPP();
    GreatPageManage(&home_page, win1_contanierInit);
    PageManage->page_manage_add_page(&second_page,getSrceen2(),LV_SCR_LOAD_ANIM_NONE,win2_contanierInit);
    for (auto &it:APP_BASEHandle) {
        PageManage->page_manage_add_page
        (&it.second->getAppPacker()->getAppPage(),
         it.second->getAppName().c_str(),
         LV_SCR_LOAD_ANIM_NONE, APPIconCloseInit);
    }
 }

DesktopInitiator::DesktopInitiator(){
     Srceen1="home page";
     Srceen2="second_page";
 }

const char *DesktopInitiator::getSrceen1() const {
    return Srceen1;
}

const char *DesktopInitiator::getSrceen2() const {
    return Srceen2;
}

void APPIconStartCallBack(lv_event_t * e){
    auto EventCode= lv_event_get_code(e);
    if (EventCode==LV_EVENT_CLICKED) {
        auto * code= (APP_BASE *)lv_event_get_user_data(e);
        mooncake.setCurrentApp(code);
        for (auto &it: mooncake.getAppBaseHandle()) {
            if (it.first == code->getAppName()) {
                mooncake.startApp(code);
                return;
            }
        }
    }
 }
void APPIconStopCallBack(lv_event_t * e){
    auto EventCode= lv_event_get_code(e);
    if (EventCode==LV_EVENT_CLICKED) {
        for (auto &it: mooncake.getAppBaseHandle()) {
            if (it.first == mooncake.getCurrentApp()->getAppName()) {
                mooncake.closeApp(mooncake.getCurrentApp());
                mooncake.setCurrentApp(nullptr);
                return;
            }
        }
    }
 }
 //屏幕1初始化
void  win1_contanierInit(lv_obj_t* page) {
     int16_t i=0;uint16_t j=0;
    if (page) {
//        ui_Screen1_screen_init(page);
       for(const auto& it:mooncake.getAppBaseHandle()) {
//            lv_obj_t * btn= lv_btn_create(ui_ApplicationPanel);
            lv_obj_t * btn= lv_btn_create(page);
            lv_obj_set_size(btn,ICONWIDTH,ICONHIGHT);
            auto tempX=lv_coord_t((i*ICONWIDTH)+((i+1)*ICONINTERVALX));
            auto tempY=lv_coord_t(j*ICONHIGHT+(j+1)*ICONINTERVALY);
            lv_obj_set_pos(btn,tempX,tempY);
            lv_obj_t * label= lv_label_create(btn);
            lv_label_set_long_mode(label,LV_LABEL_LONG_SCROLL);
            lv_label_set_text(label,it.first.c_str());
            lv_obj_set_style_text_color(label,lv_color_black(),LV_STATE_DEFAULT);
            lv_obj_align_to(label,btn,LV_ALIGN_CENTER,0,0);
            lv_obj_add_event_cb(btn, APPIconStartCallBack, LV_EVENT_CLICKED, (void *) it.second);
            i++;if (i>=5){  i=0;j++;}
        }
    }
}
void APPIconCloseInit(lv_obj_t* page){
    lv_obj_t * deleteBtn= lv_btn_create(page);
    lv_obj_set_pos(deleteBtn,20,35);
    lv_obj_t * deleteLabel= lv_label_create(deleteBtn);
    lv_obj_set_style_text_color(deleteLabel,lv_color_black(),LV_STATE_DEFAULT);
    lv_label_set_text(deleteLabel,"close");
    lv_obj_align_to(deleteLabel,deleteBtn,LV_ALIGN_CENTER,0,0);
    lv_obj_add_event_cb(deleteBtn,APPIconStopCallBack,LV_EVENT_CLICKED, nullptr);
 }
 //屏幕二初始化
void  win2_contanierInit(lv_obj_t* page) {
    //ui_Screen2_screen_init(page);
}
//回调函数切换桌面
void ui_event_UpwardSlidePanel( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM  ) {
        lv_indev_wait_release(lv_indev_get_act());
        mooncake.getPageManage()->page_manage_switch_page(mooncake.getSrceen2());
    }
}
void ui_event_MessageListParentClass( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP  ) {
        lv_indev_wait_release(lv_indev_get_act());
        mooncake.getPageManage()->page_manage_switch_page(mooncake.getSrceen1());
    }
}

