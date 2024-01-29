/**
*********************************************************************
*********
* @project_name :2048
* @file : pageManage.h
* @author : zen3
* @brief : None
* @attention : None
* @date : 2023/12/26 
*********************************************************************
*********
*/
//

#ifndef INC_2048_PAGEMANAGE_H
#define INC_2048_PAGEMANAGE_H
#include "lvgl.h"
#include "map"
#include "WString.h"

//#include "vector"
typedef struct {
//    uint8_t pageID;
    uint8_t ScreenOrientation;//屏幕方向 1；竖屏 0；横屏
    lv_obj_t * page;
    lv_scr_load_anim_t loadAnim;
    void (*init)(lv_obj_t* page);         //页面初始化
    void (*exit)(lv_obj_t* page);         //退出动画
}PAGE;

class pageManage {
public:
    pageManage(PAGE *home_page, void (*init)(lv_obj_t* page));     //初始化，并且设置home界面
    ~pageManage();

    void page_manage_add_page(PAGE* page,String id, lv_scr_load_anim_t anim, void (*init)(lv_obj_t* page));


    void page_manage_switch_page(String id);
protected:
    void RotateTheScreen(lv_obj_t* page,uint8_t Direction);

private:

    PAGE* curr_page;
//    std::vector<PAGE *> pagelist;//使用这个容器无法让页面切换类自动管理页面的id 所以使用map容器来进行管理
    std::map<String,PAGE*> pagelist;

};

enum PageID{
    home_page,
    second_page,
    game_2048_page,
};
#endif //INC_2048_PAGEMANAGE_H
