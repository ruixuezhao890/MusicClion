#include "Bradge.h"
#include "MyUsart.h"
#include "lcd.h"
#include "hal.h"
//#include "lvgl.h"
#include "ff.h"
#include "Music.h"
//#include "lv_port_indev.h"
//#include "lv_port_disp.h"
//#include "wm8978.h"
#include "i2S2.h"

 void setup(){
    HardwareManage.begin();

	Serial<<"hello world"<<endl;
     g_MusicPlayer.begin();
}
void  loop(){

    g_MusicPlayer.audioPlay();



}