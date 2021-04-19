/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : debug.h
 * @Author  : CGQ
 * @Date    : 2017-12-01
 * @Brief   : 
 ******************************************************************************/
#ifndef __DEBUG_
#define __DEBUG_

#include "ql_stdlib.h"
#include "ql_uart.h"
#include "typedef.h"

#define DEBUG_PORT			(UART_PORT3)
#define DEBUG_MSG_ID		(0x00FB00)

#define  DEBUG_ENABLE 
#define DBG_BUF_LEN   512

#ifdef DEBUG_ENABLE
extern char DebugBuffer[DBG_BUF_LEN];
void  Debug_buffer(Buffer_t *buf);
extern void Debug_output(u8* p, u16 len);
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DebugBuffer, 0, DBG_BUF_LEN);\
    Ql_sprintf(DebugBuffer,FORMAT,##__VA_ARGS__); \
    Debug_output((u8*)DebugBuffer, Ql_strlen(DebugBuffer));\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif

void Debug_init(void);


#endif //__DEBUG_
/******************************************************************************/
