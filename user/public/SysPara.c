/**************Copyright(C)??2015-2026??QIYI Temp    Co., Ltd.******************
  *@brief   : sysPata
  *@notes   : 2017.12.12 CGQ ????   
*******************************************************************************/
#include "SysPara.h"
#include "memory.h"
#include "r_stdlib.h"
#include "SysAttr.h"
#include "ParaSave.h"
#include "Algorithm.h"
#include "debug.h"
#include "Device.h"
#include "Eybond.h"
#include "net.h"
#include "log.h"
#include "watchdog.h"
#include "FlashHard.h"
#include "FlashEquilibria.h"
#include "list.h"
#include "Swap.h"
#include "CommonServer.h"
#include "sarnath.h"

#define AUTH 		(0x5A)

const char SoftVer[] = "3.3.7.4";//"test log";//
const char PortocolVer[] = "2.0.0.0";
const char SoftCompileDate[] = __DATE__;
const char SoftCompileTime[] = __TIME__;

static u8_t auth;  //锟斤拷权锟睫憋拷锟�

const SysDefParaTab_t defaultPara[] = {
	{  0, 1, "Enable"},	//enable parameter write
	//{  2, 0, "G0612345678910"},
	{  6, 0, "3.0.0.1"},	//Hard version	
	{ 12, 0, "1"},	//monitor count
	{ 14, 0, "02FF,1,1,#0#"}, //Device attr 
	{ 21, 1, "www.shinemonitor.com"}, //Default server "solar.eybond.com"
	{ 24, 1, ""}, //Default server "solar.eybond.com"
	{ 25, 1, "20"}, //default time  none
	{ 32, 0, "0"}, //trans mode
	{ 65, 1, ""},
	{ 66, 1, ""},
	{ 67, 1, ""},
	{ 68, 1, ""},
};

static void firstUser(void);
static int authGet(Buffer_t *buf);
static int authSet(Buffer_t *buf);
static SysParaTab_t *paraGet(u16_t num);
static u8_t Flash_check(void);




const SysParaTab_t sysPara[] = {
	{  0, P_F|PNA|P_W|P_R,  0, authGet, authSet},	//authority option                  00
	{  1, P_F|P_A|P_W|P_R,  0, null, null},	//type                                      01
	{  2, P_F|P_A|P_W|P_R,  1, null, null}, //SN                                        02
	{  3, P_F|PNA|P_W|P_R,  2, null, null}, //customer                                  03
	{  4, P_S|PNA|PNW|P_R,  0, (void*)PortocolVer, null}, //protocol version            04
	{  5, P_S|PNA|PNW|P_R,  0, (void*)SoftVer, null}, //soft version                    05
	{  6, P_F|P_A|P_W|P_R,  3, null, null}, //hard version                              06
	{  7, P_F|P_A|P_W|P_R,  4, null, null}, //time                                      07
	{  8, P_F|PNA|P_W|P_R,  5, null, null}, //device
	{ 11, P_F|PNA|PNW|P_R,  0, Attr_onlineCnt, null}, //online count
	{ 12, P_F|P_A|P_W|P_R,  6, null, null}, //monitor count
	{ 13, P_F|PNA|P_W|P_R,  7, null, null}, 
	{ 14, P_F|PNA|P_W|P_R,  8, null, null},
	{ 15, P_F|PNA|P_W|P_R, 23, null, null},  // meter procotol
	{ 21, P_F|P_A|P_W|P_R,  9, null, null}, 
	{ 22, P_F|P_A|P_W|P_R, 10, null, null}, 
	{ 23, P_F|P_A|P_W|P_R, 11, null, null}, 
	{ 24, P_F|P_A|P_W|P_R, 12, null, null}, 
	{ 25, P_F|P_A|P_W|P_R, 13, null, null}, 
	{ 26, P_F|PNA|PNW|P_R,  0, Attr_timeGet, null},  //system time
	{ 27, P_F|PNA|PNW|PNR,  0, null, null}, 
	{ 28, P_F|PNA|PNW|PNR,  0, null, null}, 
	{ 29, P_F|PNA|P_W|PNR,  0, null, Attr_option}, //System option



	{ 32, P_F|PNA|P_W|P_R, 14, null, null}, 
	{ 33, P_F|PNA|P_W|P_R, 15, null, null}, 
	{ 34, P_F|PNA|P_W|P_R,  0, Attr_Uart_Config_Get, Attr_Uart_Config_Set}, 

	{ 36, P_F|P_A|P_W|P_R, 30, null, null}, // hongguangtotal_power_1 	time + value


	{ 48, P_F|PNA|P_W|P_R, 18, null, null},  //Buzzer enabl
	{ 51, P_S|PNA|PNW|P_R,  0, (void*)SoftCompileDate, null},  //soft date
	{ 52, P_S|PNA|PNW|P_R,  0, (void*)SoftCompileTime, null},  //soft date
	{ 53, P_F|PNA|PNW|P_R,  0, Attr_onlineType, null}, //online device code and addr
	{ 54, P_F|PNA|P_W|P_R,  0, Attr_logGet, Attr_logSet}, //hostory data
	{ 55, P_F|PNA|PNW|P_R,  0, Attr_CSQ, null}, //GPRS CSQ
	{ 56, P_F|PNA|P_W|P_R, 16, null, null},//sim ccid
	{ 57, P_F|P_A|P_W|P_R, 17, null, null},//GPRS connect APN
	{ 58, P_F|PNA|PNW|P_R,  0, Attr_CPUID, null},//cpuid 
	{ 59, P_F|PNA|P_W|P_R, 19, null, null}, //GPRS base station ID and position 

	{ 65, P_F|PNA|P_W|P_R, 24, null, null},  // sn
	{ 66, P_F|PNA|P_W|P_R, 20, null, null}, // user name
	{ 67, P_F|PNA|P_W|P_R, 21, null, null}, // paddword
	{ 68, P_F|PNA|P_W|P_R, 22, null, null}, // number

	{ 71, P_F|PNA|P_W|P_R, 25, null, null}, // sarnath serverip
	{ 72, P_F|PNA|P_W|P_R, 26, null, null}, // sarnath server port
	{ 73, P_F|PNA|P_W|P_R, 27, null, null}, // sarnath factor id
	{ 74, P_F|PNA|P_W|P_R, 28, null, null}, // sarnath collector port
	{ 75, P_F|PNA|P_W|P_R, 29, null, null}, // sarnath collector upload data time
};

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
int SysPara_init(void)
{
	int ret;
	
	auth = AUTH;
	ret = Para_init();
	
	if (ret != 0)
	{
		for (ret = 0; ret < 3; ret++)
		{
			if (Flash_check() != 0)
			{
				Watchdog_stop();
				return -1;
			}
		}
		firstUser();
	}
	
 	SysPara_printf(Debug_buffer);
    return 0;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void SysPara_printf(void_fun_bufp output)
{
	const SysParaTab_t *para;
	Buffer_t buf;
	Buffer_t ackBuf;
	int offset;
	char str[64];

	offset = 0;
	ackBuf.size = 2048;
	ackBuf.lenght = 0;
	ackBuf.payload = memory_apply(ackBuf.size);
	for (para = sysPara; para < (&sysPara + 1); para++)
	{
		if ((0 == para->number)
			|| (11 == para->number)
            || (54 == para->number)
            || (53 == para->number)
        )
		{
			continue;
		}
		
		SysPara_Get(para->number, &buf);
		if (buf.lenght > 0 && buf.payload != null)
		{
			ackBuf.lenght += Swap_numChar(&ackBuf.payload[ackBuf.lenght], para->number);
			ackBuf.lenght += r_strcpy(&ackBuf.payload[ackBuf.lenght], "=");
			r_memcpy(&ackBuf.payload[ackBuf.lenght], (char*)buf.payload, r_strlen((char*)buf.payload));
			ackBuf.lenght += r_strlen((char*)buf.payload);
			ackBuf.lenght += r_strcpy(&ackBuf.payload[ackBuf.lenght], ";\r\n");
		}
		memory_release(buf.payload);
		buf.payload = null;
	}
	output(&ackBuf);
	memory_release(ackBuf.payload);
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void firstUser(void)
{
	char defaultVal[] = "\0";
	const SysParaTab_t *para;
	const SysDefParaTab_t *def;
	Buffer_t buf;
	int ret;

	log_save("Para Set to Default value!!\r\n");
	for (para = sysPara; para < (&sysPara + 1); para++)
	{
		if ((para->authority&P_W) && (para->authority&P_F))
		{
			def = (const SysDefParaTab_t*)ALG_binaryFind(para->number, BINARY_FIND(defaultPara));

			if (def != null)
			{
				buf.lenght = r_strlen(def->str); // + '\0'
				buf.payload = (u8_t*)def->str;
			}
			else 
			{
				buf.lenght = sizeof(defaultVal);
				buf.payload = (u8_t*)defaultVal;
			}

			ret = SysPara_Set(para->number, &buf);			
		}
	}
	
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void SysPara_default(void)
{
	const SysParaTab_t *para;
	const SysDefParaTab_t *def;
	Buffer_t buf;
	
	for (def = defaultPara; def < (&defaultPara + 1); def++)
	{
		if (def->def)
		{
			buf.lenght = r_strlen(def->str); // + '\0'
			buf.payload = (u8_t*)def->str;
			SysPara_Set(def->code, &buf);
		}
	}
}


/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void SysPara_Get(u8_t num, Buffer_t *buf)
{
    const SysParaTab_t *paraTab;

	paraTab = paraGet(num);

	if (paraTab == null || (paraTab->authority&P_R) != P_R)
	{
		buf->size = 0;
		buf->lenght = 0; 
		buf->payload = null;
	}
	else if (paraTab->authority&P_F)
	{
		if (paraTab->read == null)
		{
			int i;
			
			buf->size = SYY_PARA_LEN;
			buf->payload = memory_apply(buf->size);
			buf->lenght = SYY_PARA_LEN;
			i = Para_read(paraTab->offset*SYY_PARA_LEN, buf);
			if (i <= 0)
			{
				memory_release(buf->payload);
				buf->lenght = 0;
				buf->payload = null;
				APP_DEBUG("Para %d read err: %d.\r\n", num, i);
			}
			else
			{
				buf->lenght = r_strlen((char*)buf->payload);
			}
		}
		else
		{
			paraFun fun = (paraFun)paraTab->read;
			fun(buf);
		}
	}
	else 
	{
		buf->lenght = r_strlen((const char*)paraTab->read);
		buf->payload = (u8_t*)paraTab->read;
	}
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
int SysPara_Set(u8_t num, Buffer_t *buf)
{
	const SysParaTab_t *paraTab;
	int i = 0x01;

	paraTab = paraGet(num);
    if (paraTab == null || 0 == (paraTab->authority&P_W))
    {
		i = 0x03;
    }
	else if ((paraTab->authority&P_A) && auth != AUTH)
	{
		i = 0x02;
	}
	else
	{	
        if (buf != null && buf->lenght < SYY_PARA_LEN && buf->payload)
        {
			char str[SYY_PARA_LEN] = {0};

			r_memcpy(str, buf->payload, buf->lenght);
			memory_release(buf->payload);
			buf->lenght = SYY_PARA_LEN;
			buf->payload = (u8_t*)str;
			if (paraTab->write == null)
			{
				i = Para_write(SYY_PARA_LEN*paraTab->offset, buf);
			}
			else
			{
				paraFun fun = (paraFun)paraTab->write;

				i = fun(buf);
			}
			buf->payload = null;
			
			if (num == DEVICE_MONITOR_NUM || num == DEVICE_PROTOCOL || num == 34)
			{
				Ql_OS_SendMessage(DEVICE_TASK, SYS_PARA_CHANGE, num, 0);
			}
			else if (num == EYBOND_SERVER_ADDR)
			{
				Ql_OS_SendMessage(EYBOND_TASK, SYS_PARA_CHANGE, num, 0);
			}
			else if (num > 21 && num < 25)
			{
				Ql_OS_SendMessage(COMMON_SERVER_TASK, SYS_PARA_CHANGE, num, 0);
			}
			else if (num == SARNATH_SERVER_ADDR || num == SARNATH_SERVER_PORT ||
			         num == SARNATH_UPLOAD_TIME || num == SARNATH_FACTORY_ID || num == SARNATH_LOGGER_ID)
			{
                Ql_OS_SendMessage(SARNATH_TASK, SYS_PARA_CHANGE, 0, 0);
			}
			/*else if (num == GPRS_APN_ADDR)
			{
				Ql_OS_SendMessage(NET_TASK, SYS_PARA_CHANGE, num, 0);
			}//*/
			APP_DEBUG("Para %d wrtie ret: %d.\r\n", num, i);
        }
    }

    return i;
}
/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void SysPara_auth(void)
{
	auth = AUTH;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static SysParaTab_t *paraGet(u16_t num)
{
	return (SysParaTab_t*)ALG_binaryFind(num, BINARY_FIND(sysPara));
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static int authSet(Buffer_t *buf)
{
	if (0 == r_strcmp((char*)buf->payload, "Enable"))
	{
		SysPara_auth();
	}
	else
	{
		auth = 0;
	}
	return 0;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static int authGet(Buffer_t *buf)
{
	buf->size = 16;
	buf->payload = memory_apply(buf->size);
	
	if (auth == AUTH)
	{
		buf->lenght = r_strcpy((char*)buf->payload, "Enable");
	}
	else
	{
		buf->lenght = r_strcpy((char*)buf->payload, "Unable");
	}
	
	return 0;
}

/*******************************************************************************
 Brief    : Flash 锟剿讹拷
 Parameter: null
 return   : 0 : 锟斤拷锟斤拷   1: 锟届常
*******************************************************************************/
static u8_t Flash_check(void)
{
	const u32_t ref = 0x55aa5aa5;
	u32_t val;
	FlashEquilibria_t check;

	FlashEquilibria_init(&check, FLASH_TEST_ADDR, FLASH_TEST_SIZE, sizeof(val), &val);
	FlashEquilibria_write(&check, &ref);
	FlashEquilibria_read(&check, &val);


	return ((val == ref) ? 0 : 1);
}


/*******************************************************************************
 Brief    : Flash 锟剿讹拷
 Parameter: null
 return   : 0 : 锟斤拷锟斤拷   1: 锟届常
*******************************************************************************/
ServerAddr_t *ServerAdrrGet(u8_t num)
{
	Buffer_t buf;
	Buffer_t portBuf;
	ServerAddr_t *serverAddr = null;
	
	SysPara_Get(num, &buf);	

	if (buf.payload != null && buf.lenght > 5)
	{
		ListHandler_t cmdStr;
		int len;
		char *str = memory_apply(buf.lenght);

		r_memcpy(str, buf.payload, buf.lenght);
		r_strsplit(&cmdStr, str, '.');
		if (cmdStr.count < 3)
		{
			list_delete(&cmdStr);
			memory_release(str);
			return null;
		}
		list_delete(&cmdStr);
		memory_release(str);

		r_strsplit(&cmdStr, (char*)buf.payload, ':');

		if (cmdStr.count > 0)
		{
			len = r_strlen((char*)*(int*)cmdStr.node->payload);
			serverAddr = memory_apply(sizeof(ServerAddr_t) + len);
			r_strcpy(serverAddr->addr, (char*)*(int*)cmdStr.node->payload);

			if(num == SARNATH_SERVER_ADDR)
			{
                SysPara_Get(SARNATH_SERVER_PORT, &portBuf);
                serverAddr->port = Swap_charNum(portBuf.payload);
			}
			else
			{
			    serverAddr->port = 502;
			}
			serverAddr->type = 1;

			if (cmdStr.count > 1)
			{
				serverAddr->port = Swap_charNum((char*)*(int*)cmdStr.node->next->payload);
			}
			if (cmdStr.count > 2)
			{
				if (r_strfind("UDP", (char*)*(int*)cmdStr.node->next->next->payload) >= 0)
				{
					serverAddr->type = 0;
				}
				else if (r_strfind("SSL", (char*)*(int*)cmdStr.node->next->next->payload) >= 0)
				{
					serverAddr->type = 2;
				}
			}
		}
		list_delete(&cmdStr);
	}

	if(serverAddr->port == 0)
	{
	    memory_release(serverAddr);
	    serverAddr = null;
	}
	memory_release(portBuf.payload);
	memory_release(buf.payload);

	return serverAddr;
}


void SysParaErase(void)
{
    Flash_earseArea(FLASH_SYSPARA_ADDR, FLASH_SYSPARA_SIZE);
    Flash_earseArea(0x3E0000, FLASH_SYSPARA_SIZE);
}
/******************************************************************************/



