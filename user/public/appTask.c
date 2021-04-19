/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : appTask.c
 * @Author  : CGQ
 * @Date    : 2017-12-01
 * @Brief   : 
 ******************************************************************************/
#include "appTask.h"
#include "Debug.h"
#include "Watchdog.h"
#include "ql_system.h"
#include "ql_memory.h"
#include "Device.h"
#include "net.h"
#include "typedef.h"
#include "ril_util.h"
#include "Eybond.h"
#include "memory.h"
#include "SysPara.h"
#include "Clock.h"
#include "Swap.h"
#include "r_stdlib.h"
#include "SysPara.h"
#include "Eybond.h"
#include "DeviceIO.h"
#include "x25Qxx.h"
#include "log.h"
#include "Beep.h"
#include "key.h"
#include "CommonServer.h"
#include "antiReflux.h"
#include "sarnath.h"
#include "adc.h"

static void_fun_bufp outputFun;
static DeviceInfo_t cmdHead;
static u16_t deviceLockTime;
static u16_t logGetFlag;

static void UserTimer1scallback(u32 timerId, void* param);
static void strCmp(Buffer_t *strBuf, void_fun_bufp output);
static void outputCh(Buffer_t *buf);
static void UanrtCh(DeviceAck_e e);
static int logGet(Buffer_t *buf);


/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void proc_app_task(s32 taskId)
{
	u32 timer = 0;  //unit 1s
	ST_MSG msg;
	Buffer_t *buf;	
	
	outputFun = null;
	logGetFlag = 0;
	
	ADC_Init();
	Watchdog_init();
	x25Qxx_init();
	Clock_init();
	log_init();
	Key_init();
    Ql_Timer_Register(USER_TIMER_ID, UserTimer1scallback, &timer);
	Ql_Timer_Start(USER_TIMER_ID, 500, TRUE);//*/
	SysPara_init();
	// log_save("App statr run...\r\n");
	Beep_init();
	
	while (TRUE)
	{
		Ql_OS_GetMessage(&msg);
		switch(msg.message)
		{
			case BEEP_CMD_ID:
				Beep_run();
				break;
			case DEBUG_MSG_ID:
			case DEVICE_IO_ID:
				buf = (Buffer_t*)msg.param1;
				if (buf->lenght > 2 && 0 == Ql_strncmp((char*)buf->payload, "AT", 2))
				{
					Ql_OS_SendMessage(NET_TASK, AT_CMD_ID, msg.param1, msg.param2);
				}
				else
				{
					strCmp(buf, (void_fun_bufp)((void*)msg.param2));
				}//*/
				//Uart_write(buf->payload, buf->lenght);
				//memory_release(buf->payload);
				break;
			case WDG_CMD_ID:
				Watchdog_feed();
				Key_scan();
				break;
			case USER_TIMER_ID:
				Clock_Add();
				if ((DeviceIO_lockState() == &cmdHead) && deviceLockTime++ > (60 *2))
				{
					DeviceIO_unlock();
					memory_release(cmdHead.buf);
					cmdHead.buf = null;
				}
				if (logGetFlag == 0x5AA5)
				{
					Buffer_t log;
					logGet(&log);
					if (log.lenght == 0 || log.payload == null)
					{
						logGetFlag = 0;
					}
					else
					{
						Debug_output(log.payload, log.lenght);
					}
					memory_release(log.payload);
				}
				break;
			default:
				break;
		}

	}
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
static void UserTimer1scallback(u32 timerId, void* param)
{
	static u8_t timeCnt = 0;
	Ql_OS_SendMessage(APP_TASK, WDG_CMD_ID, 0, 0);
	Ql_OS_SendMessage(NET_TASK, timerId, 0, 0);

	if (timeCnt++ > 0)
	{
		u32 *p = param;
		
		(*p)++;
		timeCnt = 0;
		
		Ql_OS_SendMessage(APP_TASK, timerId, *p, 0);
		Ql_OS_SendMessage(EYBOND_TASK, timerId, *p, 0);
		Ql_OS_SendMessage(DEVICE_TASK, timerId, 0, 0);
		Ql_OS_SendMessage(COMMON_SERVER_TASK, timerId, 0, 0);
		Ql_OS_SendMessage(ANTI_REFLUX_TASK, timerId, 0, 0);
		Ql_OS_SendMessage(SARNATH_TASK, timerId, 0, 0);
	}
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
static void strCmp(Buffer_t *strBuf, void_fun_bufp output)
{
	const char testServer[] = "solar.eybond.com";
	const char setNote[] = "Para error!! (SET_*=***)\r\n";
	const char setOk[] = "Set Comple\r\n";
	const char getNote[] = "Para error!! (GET_*=?)\r\n";
	const char getFail[] = "No find!!\r\n";
	char *str;
	int offset;
	u32_t num;
	Buffer_t buf;
	
	if (strBuf == null || strBuf->payload == null)
	{
		return;
	}
	
	str = strBuf->payload;
	str[strBuf->lenght] = '\0';

	if ((Ql_strncmp(str, "SET-", 4) == 0)
		|| (Ql_strncmp(str, "SET_", 4) == 0)
		|| (Ql_strncmp(str, "set_", 4) == 0)
		)
	{
		str += 4;
		num = Swap_charNum(str);
		offset = r_strfind("=",str);
		if (num == 0 || offset < 0)
		{
			buf.lenght = sizeof(setNote);
			buf.payload = setNote;
			output(&buf);		
		}
		else
		{
			SysPara_auth();

			if (num == 21 && (r_strfind("TEST",str) >= 0))
			{
				buf.lenght = r_strlen(testServer);
				buf.payload = (u8_t*)testServer;
			}
			else
			{
				str = &str[offset] + 1;
				offset = r_strfind("\r", str);
				if (offset > 0)
				{
					buf.lenght = offset;
				}
				else
				{
					buf.lenght = r_strlen(str);
				}
				buf.payload = str;
			}
			offset = SysPara_Set(num, &buf);
			if (offset == 0)
			{
				buf.lenght = sizeof(setOk);
				buf.payload = setOk;
			}
			else
			{
				int i;

				buf.payload = memory_apply(20);
				i = r_strcpy(buf.payload, "ERR_");
				Swap_signedString(&buf.payload[i], offset);

				buf.lenght = r_strlen(buf.payload) + 1;
			}
			output(&buf);
		}
	}
	else if ((Ql_strncmp(str, "GET-", 4) == 0)
			|| (Ql_strncmp(str, "GET_", 4) == 0)
			|| (Ql_strncmp(str, "get_", 4) == 0)
			)
	{
		if (r_strfind("ALL", str) >= 0)
		{
			SysPara_printf(output);
		}
		else if (r_strfind("LOG", str) >= 0
		        ||r_strfind("log", str) >= 0)
		{
			logGetFlag = 0x5AA5;
		}
		else
		{
			num = Swap_charNum(str+4);
			offset = r_strfind("=?",str);
			if (num == 0 || offset < 0)
			{
				buf.lenght = sizeof(getNote);
				buf.payload = getNote;
				output(&buf);		
			}
			else
			{
				SysPara_Get(num, &buf);
				if (buf.lenght > 0 && buf.payload != null)
				{
					int i;
					u8_t *pd = buf.payload;

					buf.payload = memory_apply(buf.lenght + 16);
					Swap_numChar(buf.payload, num);
					i = r_stradd(buf.payload, "=");
					r_memcpy(&buf.payload[i], pd, buf.lenght);
					i += buf.lenght;
					i += r_strcpy(&buf.payload[i], "\r\n");
					buf.lenght = i+1;
					memory_release(pd);
					output(&buf);
				}
				else
				{
					buf.lenght = sizeof(getFail);
					buf.payload = getFail;
					output(&buf);	
				}
				memory_release(buf.payload);
			}
		}
	}
	else if (ESP_check(strBuf) == 0)
	{
		outputFun = output;
		ESP_cmd(strBuf, outputCh);
		cmdHead.buf = memory_apply(1056);
		cmdHead.buf->size = 1040;
		cmdHead.buf->lenght = 0;
		cmdHead.buf->payload = (u8_t*)(cmdHead.buf + 1);
        strBuf->payload = null;
        strBuf->size = 0;
		cmdHead.waitTime = 2000;
		cmdHead.callback = UanrtCh;
		DeviceIO_lock(&cmdHead);
	}
	// else 
	// {
	// 	output(strBuf);
	// }

	memory_release(strBuf->payload);
	strBuf->payload = null;
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
static void outputCh(Buffer_t *buf)
{
	if (outputFun != null && buf != null && buf->payload != null)
	{
		deviceLockTime = 0;
		outputFun(buf);
		memory_release(buf->payload);
		buf->payload = null;
	}
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
static void UanrtCh(DeviceAck_e e)
{
	if (e == DEVICE_ACK_FINISH)
	{
		ESP_cmd(cmdHead.buf, outputCh);
		cmdHead.buf->lenght = 0;
	}
    deviceLockTime += 10;
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
static int logGet(Buffer_t *buf)
{
	int ret;
    Buffer_t getBuf;

	buf->size = 1024;
	buf->payload = memory_apply(buf->size);
	buf->lenght = 0;

	while (buf->payload != null)
	{
        getBuf.size = buf->size - buf->lenght;
        getBuf.payload = &buf->payload[buf->lenght];
		ret = log_see(&getBuf);
        
        if (ret > 0)
        {
            buf->lenght += ret-1;
            if (buf->lenght > 900)
            {
                break;
            }
        }
        else
        {
            break;
        }
	}

	return 0;
}


/*********************************FILE END*************************************/

