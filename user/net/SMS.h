/**************Copyright(C)��2015-2026��Shenzhen Eybond Co., Ltd.***************
  *@brief   : .h 短信消息处理
  *@notes   : 2018.05.08 CGQ 
*******************************************************************************/
#ifndef __SMS_H
#define __SMS_H	 

#include "typedef.h"

#define SMS_OVERTIME_ID	(TIMER_ID_USER_START + 3)

void SMS_init(void);
void SIM_send(void);
void SMS_TextMode_Send(char *PhNum, Buffer_t *buf);
void Hdlr_RecvNewSMS(u32_t nIndex);

#endif //SMS.h
/******************************************************************************/

