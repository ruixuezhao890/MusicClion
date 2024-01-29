/**
*********************************************************************
*********
* @project_name :2048
* @file : pageManage.cpp
* @author : zen3
* @brief : None
* @attention : None
* @date : 2023/12/26 
*********************************************************************
*********
*/
//

#include "pageManage.h"
#include "lcd.h"
#include "touch.h"

pageManage::pageManage(PAGE* home_page, void (*init)(lv_obj_t* page))
{
    home_page->page = lv_obj_create(NULL);
    home_page->loadAnim = LV_SCR_LOAD_ANIM_NONE;
//    home_page->pageID = 0;
    home_page->init = init;
    home_page->ScreenOrientation=0;//默认横屏
    curr_page = home_page;

//    curr_page = NULL;
    init(home_page->page);                      //初始化home_page
//    pagelist.push_back(home_page);
    pagelist["home page"]=home_page;
    lv_scr_load(home_page->page);
}
pageManage::~pageManage()
{

}

void pageManage::page_manage_add_page(PAGE* page, String id, lv_scr_load_anim_t anim, void (*init)(lv_obj_t* page))
{
//    page->pageID = id;
    page->ScreenOrientation=0;
    page->page = lv_obj_create(NULL);
    page->loadAnim = anim;
    page->init = init;
//    init(page->page);
//    pagelist.push_back(page);
    pagelist[id]=page;
}
void pageManage::page_manage_switch_page(String id)
{


        PAGE* page =pagelist[id];
        if(curr_page != NULL) {
                lv_obj_clean(curr_page->page);
                lv_obj_set_style_bg_color(curr_page->page,lv_color_white(),LV_STATE_DEFAULT);
            }
            RotateTheScreen(page->page,page->ScreenOrientation);
            page->init(page->page);
            if (page->loadAnim!=LV_SCR_LOAD_ANIM_NONE) {
                lv_scr_load_anim(page->page, page->loadAnim, 80, 0, false);
            }else{
                lv_scr_load_anim(page->page, LV_SCR_LOAD_ANIM_NONE, 80, 0, false);
            }

            curr_page = page;

}

void pageManage::RotateTheScreen(lv_obj_t *page, uint8_t Direction) {
    lcd_display_dir(!Direction);
    tp_dev.init();
    auto getDisp=lv_disp_get_default();
    if (!Direction){//横屏
        getDisp->driver->hor_res=800;
        getDisp->driver->ver_res=480;
        lv_obj_set_size(page, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    } else{//竖屏
        getDisp->driver->hor_res=480;
        getDisp->driver->ver_res=800;
        lv_obj_set_size(page, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));

    }
    lv_disp_drv_update(lv_disp_get_default(),getDisp->driver);
}






