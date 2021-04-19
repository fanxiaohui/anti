/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : appTask.h
 * @Author  : CGQ
 * @Date    : 2017-12-01
 * @Brief   : 
 ******************************************************************************/
#ifndef __APP_TASK_
#define __APP_TASK_

#include "ql_type.h"
#include "ql_timer.h"

#define APP_TASK_ID		5

#define APP_TASK			subtask1_id
#define USER_TIMER_ID        (TIMER_ID_USER_START + 1)
#define AT_CMD_ID		0x00FA00
#define WDG_CMD_ID		0x00FAFF




void proc_app_task(s32 taskId);


#endif //__APP_TASK_
/******************************************************************************/
