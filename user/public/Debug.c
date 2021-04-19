/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : Debug.c
 * @Author  : CGQ
 * @Date    : 2017-12-01
 * @Brief   : 
 ******************************************************************************/
#include "Debug.h"
#include "appTask.h"
#include "ql_system.h"
#include "ql_uart.h"
#include "ql_stdlib.h"
#include "memory.h"
#include "ql_trace.h"
#include "appTask.h"
#include "typedef.h"

#ifdef  DEBUG_ENABLE

static Buffer_t buf;

char DebugBuffer[DBG_BUF_LEN];


/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
static void UARTCallBack(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
	s32 rdLen;

	if (buf.payload != NULL)
	{
		memory_release(buf.payload);
	}

	buf.payload = memory_apply(1024);
	if (buf.payload == NULL)
	{
		APP_DEBUG("memory alloc Fail\r\n");
		return ;
	}
	buf.size = 1024;
	buf.lenght = 0;

    while (1)
    {
        rdLen = Ql_UART_Read(port, &buf.payload[buf.lenght],  buf.size - buf.lenght);
        if (rdLen <= 0)  // All data is read out, or Serial Port Error!
        {
            break;
        }
        buf.lenght += rdLen;
        // Continue to read... */
    }

	
	if (buf.lenght > 0)
	{
		Ql_OS_SendMessage(APP_TASK, DEBUG_MSG_ID, (u32)(&buf), (u32)((void*)Debug_buffer));
	}
}
#endif

/*******************************************************************************
 Brief    : Debug_init
 Parameter: 
 return   : 
*******************************************************************************/
void Debug_init(void)
{
	#ifdef  DEBUG_ENABLE
	Ql_UART_Register(DEBUG_PORT, UARTCallBack,  NULL);
	Ql_UART_Open(DEBUG_PORT, 115200, FC_NONE);
	buf.payload = null;
	#endif 
}

/*******************************************************************************
 Brief    : Debug_printf
 Parameter: 
 return   : 
*******************************************************************************/
#ifdef  DEBUG_ENABLE
 void  Debug_buffer(Buffer_t *buf)
{
	Debug_output(buf->payload, buf->lenght);
	memory_release(buf->payload);
	buf->payload = null;
}

/*******************************************************************************
 Brief    : Debug_output
 Parameter: 
 return   : 
*******************************************************************************/

void Debug_output(u8* p, u16 len)
{
	#if 0//(UART_PORT2 == DEBUG_PORT) 
        Ql_Debug_Trace(DebugBuffer);
	#else
        Ql_UART_Write(DEBUG_PORT, p, len);
	#endif
}

#endif 

/*********************************FILE END*************************************/

