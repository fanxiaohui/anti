/**********************    COPYRIGHT 2014-2100, QIYI    ************************ 
 * @File    : Buzzer.c
 * @Author  : CGQ
 * @Date    : 2018-01-12
 * @Brief   : 
 ******************************************************************************/
#include "Beep.h"
#include "Ql_type.h"
#include "Ql_system.h"
#include "appTask.h"
#include "SysPara.h"
#include "memory.h"

static char beepCnt;

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void Beep_init(void)
{
	beepCnt = 0;
	Ql_GPIO_Init(BEEP_PORT, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLDOWN);
	Beep_on(1);
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void Beep_on(char cnt)
{
	if (cnt < 10 && cnt > 0)
	{
		beepCnt = cnt;
		Ql_OS_SendMessage(APP_TASK, BEEP_CMD_ID, 0, 0);
	}
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void Beep_run(void)
{
	Buffer_t buf;

	SysPara_Get(BUZER_EN_ADDR, &buf);
	if (buf.payload != null && buf.lenght > 0)
	{
		int num;
		num = Swap_charNum((char*)buf.payload);
		memory_release(buf.payload);
		if (num == 1)
		{
			return;
		}
	}
	Ql_GPIO_SetLevel(BEEP_PORT, PINLEVEL_HIGH);
	Ql_Sleep(200);
	Ql_GPIO_SetLevel(BEEP_PORT, PINLEVEL_LOW);
	Ql_Sleep(200);
	if (--beepCnt > 0)
	{
		Ql_OS_SendMessage(APP_TASK, BEEP_CMD_ID, 0, 0);
	}
}
/******************************************************************************/
