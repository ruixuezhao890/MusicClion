/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : StatusList.h
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/1/29 
*********************************************************************
*********
*/
//

#ifndef MUSICCLION_STATUSLIST_H
#define MUSICCLION_STATUSLIST_H
#define EVENTBIT_DMAFinish	(1<<0)
#define EVENTBIT_VolDw		(1<<1)
#define EVENTBIT_Prev		(1<<2)
#define EVENTBIT_Play		(1<<3)
#define EVENTBIT_Next		(1<<4)
#define EVENTBIT_Forward	(1<<5)
#define EVENTBIT_ALL	(EVENTBIT_DMAFinish|EVENTBIT_VolDw|EVENTBIT_Prev|EVENTBIT_Next|EVENTBIT_Play|EVENTBIT_Forward)
#endif //MUSICCLION_STATUSLIST_H
