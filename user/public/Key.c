/**********************    COPYRIGHT 2014-2100, QIYI    ************************ 
 * @File    : Key.c
 * @Author  : CGQ
 * @Date    : 2018-01-12
 * @Brief   : 
 ******************************************************************************/
#include "key.h"
//#include "Ql_GPIO.h"
#include "Ql_system.h"
#include "Beep.h"
#include "sysPara.h"
#include "log.h"


/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void Key_init(void)
{
	Ql_GPIO_Init(KEY_PORT, PINDIRECTION_IN, PINLEVEL_HIGH, PINPULLSEL_PULLUP);
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void Key_scan(void)
{
	static cnt = 0;

	if (Ql_GPIO_GetLevel(KEY_PORT) == 0)
	{
		if (++cnt > 5)
		{
			log_save("Reset key user.\r\n");
			Beep_on(1);
			SysPara_default();
			Watchdog_stop();
		}
	}
	else
	{
		cnt = 0;
	}
}

/******************************************************************************/
