/******************************************************************************           
* name:                       
* introduce:        头文件
* author:           Luee                                     
******************************************************************************/ 
#ifndef __DEBUG_
#define __DEBUG_

#include "r_stdlib.h"
#include "oc_uart.h"
#include "typedef.h"

#include "stdio.h"
//#include "stdlib.h"
//#include "string.h"

#define EYBOND_DEBUG_ENABLE    // 打开DEBUG log
//#define EYBOND_TRACE_ENABLE    // 将DEBUG log输出改为TRACE模式,不能直接用TRACE_ENABLE做开关,会影响SDK里面的接口
#define DBG_BUF_LEN     1024

#define DEBUG_PORT      2 //调试串口
char DebugBuffer[DBG_BUF_LEN];
#define DEBUG_PORT_BITRATE 115200

#define DEBUG_INPUT_EHCO   0

#ifdef EYBOND_DEBUG_ENABLE
void  Debug_buffer(Buffer_t *buf);
extern void Debug_output(u8_t *p, u16_t len);

#define APP_DEBUG(FORMAT,...)  {\
  r_memset(DebugBuffer, 0, DBG_BUF_LEN);\
  snprintf(DebugBuffer,DBG_BUF_LEN,"%s:%d>>"FORMAT,__func__,__LINE__,##__VA_ARGS__);\
  Debug_output((u8_t*)DebugBuffer, r_strlen(DebugBuffer));\
}
extern void Print_output(u8_t *p, u16_t len);
#define APP_PRINT(FORMAT,...) {\
  r_memset(DebugBuffer, 0, DBG_BUF_LEN);\
  snprintf(DebugBuffer,DBG_BUF_LEN,"%s:%d>>"FORMAT,__func__,__LINE__,##__VA_ARGS__);\
  Print_output((u8_t*)DebugBuffer, r_strlen(DebugBuffer));\
}

#define DEBUG_PRINT(FORMAT,...)  {\
  r_memset(DebugBuffer, 0, DBG_BUF_LEN);\
  snprintf(DebugBuffer, DBG_BUF_LEN, "%s:%d %s::"FORMAT, __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
  Debug_output((u8_t*)DebugBuffer, r_strlen(DebugBuffer));\
}

extern void Debug_trace(u8_t *p, u16_t len);
#define APP_TRACE(FORMAT,...) {\
  r_memset(DebugBuffer, 0, DBG_BUF_LEN);\
  snprintf(DebugBuffer, DBG_BUF_LEN, "%s:%d %s::"FORMAT, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
  Debug_trace((u8_t*)DebugBuffer, r_strlen(DebugBuffer));\
}
#else
#define APP_DEBUG(FORMAT,...)
#define APP_PRINT(FORMAT,...)
#define APP_TRACE(FORMAT,...)
#endif


void Debug_init(void);


#endif //__DEBUG_
/******************************************************************************/
