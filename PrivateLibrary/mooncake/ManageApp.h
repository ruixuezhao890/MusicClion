/**
*********************************************************************
*********
* @project_name :2048
* @file : test.h
* @author : zen3
* @brief : None
* @attention : None
* @date : 2023/12/14
*********************************************************************
*********
*/
//

#ifndef INC_2048_MANAGEAPP_H
#define INC_2048_MANAGEAPP_H
#include <mooncake.h>
#include "pageManage.h"
#define ICONWIDTH 60
#define ICONHIGHT 60
#define ICONINTERVALY 15
#define ICONINTERVALX 15
using namespace MOONCAKE;
class DesktopInitiator:public Mooncake{
private:
    PAGE home_page{};
    PAGE second_page{};
    const char* Srceen1;
    const char* Srceen2;
protected:

public:
    DesktopInitiator();

    const char *getSrceen1() const;

    const char *getSrceen2() const;

    void createAPP();

    void DesktopInitiatorInit();
};
extern DesktopInitiator mooncake;


#endif //INC_2048_MANAGEAPP_H
