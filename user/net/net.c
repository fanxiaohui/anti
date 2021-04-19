/**************Copyright(C)��2015-2026��Shenzhen Eybond Co., Ltd.***************
  *@brief   : ������غ���
  *@notes   : 2017.12.06 CGQ ���� 
*******************************************************************************/
#include "net.h"
#include "list.h"
#include "ql_system.h"
#include "ql_stdlib.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_telephony.h"
#include "ril_network.h"
#include "typedef.h"
#include "Debug.h"
#include "M26Net.h"
#include "appTask.h"
#include "memory.h"
#include "Clock.h"
#include "Swap.h"
#include "r_stdlib.h"
#include "Eybond.h"
#include "Watchdog.h"
#include "log.h"
#include "SysPara.h"
#include "Ql_socket.h"
#include "log.h"
#include "Beep.h"
#include "SMS.h"
#include "SSL.h"

typedef struct
{
    u8_t port;
    Buffer_t buf;
}NetSend_t;

#define NET_OVERTIME 		(60*10)
#define MEMOTY_SIZE			(160 * 1024)  //unit 1K

static char testIP[64] = {0};
static u16 testPort;
static u32 NetOvertime;
static char NetOff[] = {"AT+CFUN=0"};
static char NetOn[]  = {"AT+CFUN=1"};
static char CCID[]  = {"AT+CCID"};
static char CSQ[]  = {"AT+CSQ"};
static char CREG_MODE[]  = {"AT+CREG=2"};
static char CREG[]  = {"AT+CREG?"};


static ListHandler_t netSendPakege;

static int netInTest(Buffer_t *buf, void_fun_bufp output);
static void Net_sendData(void);
static s32 ATResponse_Handler(char* line, u32 len, void* userData);
static void Net_OnAck(Buffer_t *buf);
static void Net_OffAck(Buffer_t *buf);
static void Net_CSQ(Buffer_t *buf);
static void Net_CCID(Buffer_t *buf);
static void Net_CREGMode(Buffer_t *buf);
static void Net_CREG(Buffer_t *buf);
static void NetLEDInit(void);
static void GSMLEDInit(void);

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void proc_net_task(s32 taskId)
{
	int ret;
	char gprsManageCount = 0;
	ST_MSG msg;
	Buffer_t *buf;
	void_fun_bufp output;
	int netResetCnt;

	/*variable Initialization*/
	netResetCnt = 0;
	NetOvertime = 0;
	testPort = 0;
	testIP[0] = '\0';
	list_init(&netSendPakege);
	SMS_init();
	Debug_init();
	APP_DEBUG("\r\nEybond GPRS V3.0 Application!!\r\n");
	buf = Ql_MEM_Alloc(MEMOTY_SIZE);
	if (buf == null)
	{
		APP_DEBUG("memory alloc Fail\r\n");
	}
	memory_init(buf, MEMOTY_SIZE);
	NetLEDInit();
	GSMLEDInit();
	
    while(TRUE)
    {
        Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
			case NET_RESTART_ID: //M26 restart 
			    NetOvertime = NET_OVERTIME - 30;
				if ((msg.param1 == 0 || msg.param1 == GPRS_APN_ADDR))
				{
					M26Net_init();
					if (netResetCnt++ > 6)
					{
						log_save("Net err over reboot!!\r\n");
						Watchdog_stop(); //reset system
					}
					else
					{
						log_save("GPRS net off!!\r\n");
						Ql_RIL_SendATCmd((char*)NetOff, sizeof(NetOff), ATResponse_Handler, Net_OffAck, 0);
					}
				}
				else if (msg.param1 == 1)
				{
					log_save("GPRS net on!!\r\n");
					Ql_Sleep(1000);
					Ql_RIL_SendATCmd((char*)NetOn, sizeof(NetOn), ATResponse_Handler, Net_OnAck, 0);
				}
				else
				{
                    log_save("GPRS reset Ok!!\r\n");
                    NetOvertime = 0;
				}
				break;
			case MSG_ID_RIL_READY:
				APP_DEBUG("<-- RIL is ready -->\r\n");
				
				Ql_RIL_Initialize();
				M26Net_init();
				
				break;
	        case MSG_ID_URC_INDICATION:
	            //APP_DEBUG("<-- Received URC: type: %d, -->\r\n", msg.param1);
	            switch (msg.param1)
	            {
	            case URC_SYS_INIT_STATE_IND:
					APP_DEBUG("<-- Sys Init Status %d -->\r\n", msg.param2);
	                if (SYS_STATE_SMSOK == msg.param2)
	                {	// SMS option has been initialized, and application can program SMS
	                    APP_DEBUG("<-- Application can program SMS -->\r\n");
	                }
	                break;
	            case URC_SIM_CARD_STATE_IND:
	                if (SYS_STATE_SMSOK == msg.param2)
                	{
						APP_DEBUG("<-- SIM card is ready -->\r\n");
					}
					else if (0 == msg.param2)
					{
						log_save("SIM card not inserted\r\n");
						Beep_on(5);
					}
					else 
					{
						APP_DEBUG("SIM ERR: %d\r\n", msg.param2);
					}
	                break;
	            case URC_GSM_NW_STATE_IND:
	               	if (NW_STAT_REGISTERED == msg.param2 || NW_STAT_REGISTERED_ROAMING == msg.param2)
	                {
	                    APP_DEBUG("<-- Module has registered to GSM network -->\r\n");
	                }
					else 
					{
						log_saveAbnormal("GSM Not registered, ERR: ", msg.param2);
	                }				
	                break;
	            case URC_GPRS_NW_STATE_IND:
					if (NW_STAT_REGISTERED == msg.param2 || NW_STAT_REGISTERED_ROAMING == msg.param2)
	                {
	                    APP_DEBUG("<-- Module has registered to GPRS network -->\r\n");
						M26Net_ready();
						Ql_RIL_SendATCmd(CCID, sizeof(CCID), ATResponse_Handler, Net_CCID, 0);
						Ql_RIL_SendATCmd(CSQ, sizeof(CSQ), ATResponse_Handler, Net_CSQ, 0);
						Ql_RIL_SendATCmd(CREG_MODE, sizeof(CREG_MODE), ATResponse_Handler, Net_CREGMode, 0);
						Ql_RIL_SendATCmd(CREG, sizeof(CREG), ATResponse_Handler, Net_CREG, 0);
						SSL_init();
	                }
					else
                    {
						APP_DEBUG("GSM Not registered, ERR: %d.\r\n", msg.param2);
						if (NW_STAT_NOT_REGISTERED == msg.param2 && m_GprsActState)
						{
	                        u32 rssi;
	                        u32 ber;
	                        ret = RIL_NW_GetSignalQuality(&rssi, &ber);
	                        APP_DEBUG("<-- Signal strength:%d, BER:%d -->\r\n", rssi, ber);
						}
                    }
	                break;
	            case URC_CFUN_STATE_IND:
	                APP_DEBUG("<-- CFUN Status:%d -->\r\n", msg.param2);
	                break;
	            case URC_COMING_CALL_IND:
	                {
	                    ST_ComingCall* pComingCall = (ST_ComingCall*)msg.param2;
	                    APP_DEBUG("<-- Coming call, number:%s, type:%d -->\r\n", pComingCall->phoneNumber, pComingCall->type);
						Ql_RIL_SendATCmd("ATA", sizeof("ATA"), ATResponse_Handler, null, 0);
						break;
	                }
	            case URC_CALL_STATE_IND:
	                APP_DEBUG("<-- Call state:%d\r\n", msg.param2);
	                break;
	            case URC_NEW_SMS_IND:
	                APP_DEBUG("<-- New SMS Arrives: index=%d\r\n", msg.param2);
					Hdlr_RecvNewSMS(msg.param2);
	                break;
	            case URC_MODULE_VOLTAGE_IND:
	                log_saveAbnormal("<-- VBatt Voltage Ind: type= ", msg.param2);
	                break;
	            default:
	                APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
	                break;
	            }
	            break;
			case AT_CMD_ID:		//�ⲿAT��������
			    buf = (Buffer_t*)msg.param1;
				output = (void_fun_bufp)((void*)msg.param2);
				if (netInTest(buf, output) != 0)
				{
					Ql_memcpy(&buf->payload[buf->lenght], "\r\n", 3);
					buf->lenght += 3;
					
					ret = Ql_RIL_SendATCmd((char*)buf->payload, buf->lenght, ATResponse_Handler, output, 0);
				}
				memory_release(buf->payload);
				buf->payload = null;
				break;
			case USER_TIMER_ID:
				#if TEST_RF
				if (++NetOvertime > NET_OVERTIME)
				{
					log_save("Server connect overtime!!\r\n");
					Ql_OS_SendMessage(NET_TASK, NET_RESTART_ID, 0, 0);
				}
				#else
				APP_DEBUG("RF test!!\r\n");
				#endif

//				if(gprsManageCount++ % 10 == 0)
				{
//				    APP_DEBUG("Enter gprs manage count\r\n");
				    M26Net_manage();
				}
				SIM_send();
				break;
			case NET_SEND_DATA_ID:
				Net_sendData();
				break;
	        default:
	            break;
        }
    }
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static int netInTest(Buffer_t *buf, void_fun_bufp output)
{
	int ret = -1;
	Buffer_t ackBuf;

	if (buf != null && buf->payload != null && buf->lenght > 20)	   
	{		 
		const char AT_TEST[] = "AT+TEST=";		  
		const char AT_ACK[] = "+OK\r\n";			
		int i = r_strfind(AT_TEST, (const char *)buf->payload); 				   
		if ( i >= 0) 		 
		{				
			int port;			   
			char *p = (char*)&buf->payload[i];			   
			i = r_strfind(",", p);			   
			port = Swap_charNum((char*)(p+i+1)); 			 
			if ((i > 0)&&(i < 40) && port < 65535) 			 
			{				  
				p[i] = '\0';
				APP_DEBUG("In test: %s, port: %d\r\n", p, port);
				if (r_strlen(testIP) == 0 || 0 != r_strcmp( p + r_strlen(AT_TEST), testIP)) 			   
				{		
					testPort = port;
					r_strcpy(testIP, p + r_strlen(AT_TEST));			  
					M26Net_closeAll();
					APP_DEBUG("test server: %s, port: %d\r\n", testIP, port);
				}	
				Net_connect(1, testIP, port, ESP_callback);
				ackBuf.lenght = sizeof(AT_ACK);
				ackBuf.payload = AT_ACK;
				output(&ackBuf);		
				ret = 0;
			}		   
		}	  
	}

	return ret;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
u8_t Net_connect(u8 mode, char *ip, u16_t port, NetDataCallback netCallback)
{
	#if TEST_RF
	if (testPort != 0 && testPort != port)
	#endif
	{
		return 0xFF;
	}
	
	return M26Net_open(mode, ip, port, netCallback);
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void Net_close(u8 port)
{
	M26Net_close(port);
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
u8 Net_status(u8 port)
{
	return M26Net_status(port);
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void Net_send(u8_t port, u8_t* pData, u16_t len)
{
    NetSend_t *sendData = list_nodeApply(sizeof(NetSend_t) + len);
    
    if (sendData != null)
    {
        sendData->port = port;
        sendData->buf.lenght = len;
        sendData->buf.payload = (u8_t*)(sendData + 1);
        list_bottomInsert(&netSendPakege, sendData);
        r_memcpy(sendData->buf.payload, pData, len);
		Ql_OS_SendMessage(NET_TASK, NET_SEND_DATA_ID, 0, 0);
    }
    else
    {
        APP_DEBUG("memory apply full!!!");
    }
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
static void Net_sendData(void)
{    
	NetSend_t *send;

	send = (NetSend_t *)list_nextData(&netSendPakege, null);
	if (send == null)
	{
		memory_trans(Debug_output);
		return ;
	}
    else if (send->buf.lenght == 0)
    {
        list_nodeDelete(&netSendPakege, send);
    }
    else
	{
		int ret ;

		ret = M26Net_send(send->port, send->buf.payload, send->buf.lenght);

		if (ret == send->buf.lenght)
		{
			list_nodeDelete(&netSendPakege, send);
			APP_DEBUG("<-- Send socket data successfully. --> \r\n");
			NetOvertime = 0;
		}
		else if (ret > 0 && ret < send->buf.lenght)
		{
			send->buf.lenght -= ret;
			send->buf.payload = send->buf.payload + ret;
		}
		else if (ret == SOC_WOULDBLOCK)
		{
			APP_DEBUG("wait CallBack_socket_write, then send data.\r\n");
			return;
		}
		else if (ret == -520)
		{
			list_nodeDelete(&netSendPakege, send);
			M26Net_close(send->port);
			APP_DEBUG("<-- prot close state. --> \r\n");
		}
		else
		{
			M26Net_close(send->port);
			APP_DEBUG("<-- Fail to connect to server, cause=%d -->\r\n", ret);
			
		}
		
    }

	Ql_OS_SendMessage(NET_TASK, NET_SEND_DATA_ID, 2, 0);

}

static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{
	if (userData != null)
	{
		Buffer_t buf;
		void_fun_bufp output = (void_fun_bufp)(userData);

		buf.lenght = len;
		buf.payload = line;
		output(&buf);
	}
    
    if (Ql_RIL_FindLine(line, len, "OK"))
    {  
        return  RIL_ATRSP_SUCCESS;
    }
    else if (Ql_RIL_FindLine(line, len, "ERROR"))
    {  
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
    {
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
    {
        return  RIL_ATRSP_FAILED;
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}

static void Net_OffAck(Buffer_t *buf)
{
	if (Ql_RIL_FindLine((char*)buf->payload, buf->lenght, "OK"))
    {
		Ql_OS_SendMessage(NET_TASK, NET_RESTART_ID, 1, 0);
	}
}

static void Net_OnAck(Buffer_t *buf)
{
	if (Ql_RIL_FindLine((char*)buf->payload, buf->lenght, "OK"))
    {
		Ql_OS_SendMessage(NET_TASK, NET_RESTART_ID, 2, 0);
	}
}

static void Net_CREGMode(Buffer_t *buf)
{
	if (Ql_RIL_FindLine((char*)buf->payload, buf->lenght, "OK"))
    {
		
	}
}

static void Net_CREG(Buffer_t *buf)
{
	APP_DEBUG("CEGR: %s", buf->payload);
	if (r_strfind("+CREG", (char*)buf->payload) >= 0)
    {
		ListHandler_t subStr;
        Node_t *node;
        
		r_strsplit(&subStr, (char*)buf->payload, ',');
		if (subStr.count > 3)
		{
			Buffer_t writeBuf;
			u16_t base;
			u16_t id;

			char *p =  (char*)*(int*)subStr.node->next->next->payload;
			Swap_charHex((u8_t*)&base, p + 1);
			p = (char*)*(int*)subStr.node->next->next->next->payload;
			Swap_charHex((u8_t*)&id, p + 1);
			
			writeBuf.size = 32;
			writeBuf.payload = memory_apply(writeBuf.size);
			writeBuf.lenght = 0;
			Swap_bigSmallShort(&base);
			Swap_bigSmallShort(&id);
			Swap_numChar((char*)writeBuf.payload, base);
			writeBuf.lenght = r_stradd((char*)writeBuf.payload, ",");
			writeBuf.lenght += Swap_numChar((char*)&writeBuf.payload[writeBuf.lenght], id) + 1;
			SysPara_Set(GPRS_BASE_ID, &writeBuf);
			memory_release(writeBuf.payload);
		}
		list_delete(&subStr);
	}
}

static void Net_CCID(Buffer_t *buf)
{
	int offset = r_strfind(&CCID[2], (char*)buf->payload);
	
	if (offset > 0 && buf->lenght > 30)
	{
		Buffer_t setBuf;

		setBuf.lenght = 20;
		setBuf.payload = &buf->payload[offset+8];
		SysPara_Set(GPRS_CCID_ADDR, &setBuf);
	}
}

static void Net_CSQ(Buffer_t *buf)
{
	int offset = r_strfind("+CSQ", (char*)buf->payload);
	if (offset >= 0)
	{
		log_save((char*)&buf->payload[offset]);
	}
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void NetLEDInit(void)
{
	Ql_GPIO_Init(NET_LED, PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_PULLDOWN);
	NetLEDOff();
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void NetLEDOn(void)
{
	Ql_GPIO_SetLevel(NET_LED, PINLEVEL_LOW);
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void NetLEDOff(void)
{
	Ql_GPIO_SetLevel(NET_LED, PINLEVEL_HIGH);
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void GSMLEDInit(void)
{
	Ql_GPIO_Init(GSM_LED, PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_PULLDOWN);
	GSMLEDOff();
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void GSMLEDOn(void)
{
	Ql_GPIO_SetLevel(GSM_LED, PINLEVEL_LOW);
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void GSMLEDOff(void)
{
	Ql_GPIO_SetLevel(GSM_LED, PINLEVEL_HIGH);
}


/******************************************************************************/

