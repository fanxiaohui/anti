/******************************************************************************           
* name:                        
* introduce:        头文件
* author:           Luee                                     
******************************************************************************/ 

#ifndef __HARD_WATCHDOG_
#define __HARD_WATCHDOG_

#include "typedef.h"

extern u8_t FeedFlag;

void Watchdog_init(void);
void Watchdog_feed(void);
void Watchdog_stop(void);
void watchdogns(u8 ns);


#endif /* __HARD_WATCHDOG_ */