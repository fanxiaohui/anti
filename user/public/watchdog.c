/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : watchdog.c
 * @Author  : CGQ
 * @Date    : 2017-12-05
 * @Brief   : 
 ******************************************************************************/
#include "watchdog.h"
#include "ql_system.h"
#include "ql_wtd.h"
#include "Debug.h"
#include "Clock.h"

static u8 FeedFlag;
static s32 WatchID;


/*******************************************************************************
 Brief    : Watchdog_init
 Parameter: 
 return   : 
*******************************************************************************/
void Watchdog_init(void)
{
	FeedFlag = 0;
	Ql_WTD_Init(0, WATCHDOG_PIN, 600);
	WatchID = Ql_WTD_Start(1000);
}

/*******************************************************************************
 Brief    : Watchdog_feed
 Parameter: 
 return   : 
*******************************************************************************/
void Watchdog_feed(void)
{
	if (FeedFlag == 0)
	{
		Ql_WTD_Feed(WatchID);
	}
}

/*******************************************************************************
 Brief    : Watchdog_stop
 Parameter: 
 return   : 
*******************************************************************************/
void Watchdog_stop(void)
{
	int i;

	for (i = 0; i < 6; i++)
	{
		Clock_Add();
	}
	FeedFlag = 1;
	Ql_WTD_Stop(WatchID);
	Clock_save();
	APP_DEBUG("feed stop!!\r\n");
}


/*********************************FILE END*************************************/

