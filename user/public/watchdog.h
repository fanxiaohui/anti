/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : watchdog.h
 * @Author  : CGQ
 * @Date    : 2017-12-05
 * @Brief   : 
 ******************************************************************************/
#ifndef __WATCH_DOG_
#define __WATCH_DOG_

#include "ql_type.h"
#include "Ql_gpio.h"

#define WATCHDOG_PIN		(PINNAME_DTR)


void Watchdog_init(void);
void Watchdog_feed(void);
void Watchdog_stop(void);


#endif //__WATCH_DOG_
/******************************************************************************/
