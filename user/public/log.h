/**********************    COPYRIGHT 2014-2100, QIYI    ************************ 
 * @File    : log.c
 * @Author  : CGQ
 * @Date    : 2017-08-17
 * @Brief   : 
 ******************************************************************************/

#ifndef __LOG_
#define __LOG_

#include "typedef.h"


void log_init(void);
void log_clear(void);
void log_save(char const * note);
void log_saveAbnormal(char const * note, int num);
u16_t log_get(Buffer_t *buf);
u16_t log_see(Buffer_t *buf);
void log_saveBuf(Buffer_t *buf);

#endif //log
/******************************************************************************/
