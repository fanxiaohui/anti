/**********************    COPYRIGHT 2014-2100, QIYI    ************************ 
 * @File    : Buzzer.c
 * @Author  : CGQ
 * @Date    : 2018-01-12
 * @Brief   : 
 ******************************************************************************/
#ifndef __BEEP_H_
#define __BEEP_H_


//#include "Ql_gpio.h"

#define BEEP_PORT			(PINNAME_DCD)
#define BEEP_CMD_ID			(0x0800FF)

void Beep_init(void);
void Beep_on(char cnt);
void Beep_run(void);


#endif //__BEEP_H_
/******************************************************************************/
