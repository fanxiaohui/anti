/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : M26Net.c
 * @Author  : CGQ
 * @Date    : 2017-12-08
 * @Brief   : 
 ******************************************************************************/
#include "M26Net.h"
#include "Ql_stdlib.h"
#include "ql_socket.h"
#include "ql_gprs.h"
#include "Debug.h"
#include "memory.h"
#include "Syspara.h"
#include "SSL.h"
#include "Clock.h"

s32 m_GprsActState;  // GPRS PDP activation state, 0= not activated, 1=activated
static M26Net_t netManage[6];
static s32 M26pdpCntxtId;  
static u8 registe;
static s32 customPara = 0;


/************************************************************************/
/* Definition for GPRS PDP context                                      */
/************************************************************************/
static ST_GprsConfig m_GprsConfig = {
    "",    // APN name
    "",         // User name for APN
    "",         // Password for APN
    0,
    NULL,
    NULL,
};

static void ipCallback(u8 contexId, u8 requestId, s32 errCode,  u32 ipAddrCnt, u32* ipAddr);
static void Callback_Socket_Connect(u8 contextId, s32 errCode, void* customParam );
static void Callback_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam );
static void Callback_Socket_Close(s32 socketId, s32 errCode, void* customParam );
static void Callback_Socket_Read(s32 socketId, s32 errCode, void* customParam );
static void Callback_Socket_Write(s32 socketId, s32 errCode, void* customParam);

static void socketClose(s32 socketid);

static ST_PDPContxt_Callback callback_gprs_func = {
    NULL,
    Callback_GPRS_Deactived
};

static ST_SOC_Callback callback_soc_func = {
    Callback_Socket_Connect,
    Callback_Socket_Close,
    NULL,
    Callback_Socket_Read,    
    Callback_Socket_Write
};

/*******************************************************************************
 Brief    : M26Net para init
 Parameter: 
 return   : 
*******************************************************************************/
void M26Net_init(void)
{
	Buffer_t buf;

	SysPara_Get(GPRS_APN_ADDR, &buf);

	if (buf.lenght > 2 && buf.payload != null)
	{
		r_strcpy(m_GprsConfig.apnName, buf.payload);
		memory_release(buf.payload);
	}
	
	m_GprsActState = 0;
	M26pdpCntxtId = 0;
	registe = 0;
	NetLEDOff();
	GSMLEDOff();
	Ql_memset(netManage, 0, sizeof(netManage));
}

/*******************************************************************************
 Brief    : M26Net_open
 Parameter: 
 return   : connect number 0~ 5; oxFF: full no space
*******************************************************************************/
u8 M26Net_open(u8 mode, char *ip, u16 port, NetDataCallback netCallback)
{
	int i;

	for (i = 0; i < sizeof(netManage)/sizeof(netManage[0]); i++)
	{
		if (netManage[i].flag == 1 
			&& netManage[i].port == port 
			&& 0 == Ql_strcmp(ip, netManage[i].ipStr))
		{
			netManage[i].flag = 1;
			netManage[i].mode = mode;
			netManage[i].callback = netCallback;
			break;
		}
		else if (netManage[i].flag == 0)
		{
			netManage[i].flag = 1;
			netManage[i].mode = mode;
			netManage[i].port = port;
			netManage[i].callback = netCallback;
			netManage[i].status = M26_WAIT;
			netManage[i].socketID = -1;
			netManage[i].ip = 0;
			Ql_strcpy(netManage[i].ipStr, ip);
			break;
		}
	}	

	return ((i >= sizeof(netManage)/sizeof(netManage[0])) ? 0xFF : i);
}

/*******************************************************************************
 Brief    : M26Net_close
 Parameter: 
 return   : 
*******************************************************************************/
u8 M26Net_status(u8 port)
{
	u8 ret = 0xff;

	if (port < sizeof(netManage)/sizeof(netManage[0]))
	{
		if (netManage[port].flag == 1)
		{
			ret = netManage[port].status;
			if (ret == M26_SUCCESS)
			{
				NetLEDOn();
			}
			else
			{
				NetLEDOff();
			}
		}
	}

	return ret;
}

/*******************************************************************************
 Brief    : M26Net_close
 Parameter: 
 return   : 
*******************************************************************************/
void M26Net_closeAll(void)
{
	int i;

	for (i = 0; i < sizeof(netManage)/sizeof(netManage[0]); i++)
	{
		M26Net_close(i);
	}
}

/*******************************************************************************
 Brief    : M26Net_close
 Parameter: 
 return   : 
*******************************************************************************/
void M26Net_close(u8 port)
{
	if (port < sizeof(netManage)/sizeof(netManage[0]))
	{
		if (netManage[port].flag != 0)
		{
//			if (netManage[port].status == M26_SUCCESS)
			{
				Ql_SOC_Close(netManage[port].socketID);
				NetLEDOff();
			}
		}
		netManage[port].flag = 0;
		netManage[port].socketID = -1;
		netManage[port].ip = 0;
		netManage[port].status = M26_CLOSE;
	}
}

/*******************************************************************************
 Brief    : M26Net_close
 Parameter: 
 return   : 
*******************************************************************************/
void M26Net_ready(void)
{
	s32 ret;

	registe = 0;
	
	M26pdpCntxtId = Ql_GPRS_GetPDPContextId();
    if (GPRS_PDP_ERROR == M26pdpCntxtId)
    {
        log_save("No PDP context is available\r\n");
        return;
    }
    ret = Ql_GPRS_Register(M26pdpCntxtId, &callback_gprs_func, NULL);
    if (GPRS_PDP_SUCCESS == ret || GPRS_PDP_ALREADY == ret)
    {
        APP_DEBUG("<-- Register GPRS callback function -->\r\n");
    }
	else
	{
        log_saveAbnormal("<-- Fail to register GPRS, ", ret);
        return;
    }
	
	ret = Ql_GPRS_Config(M26pdpCntxtId, &m_GprsConfig);
	if (GPRS_PDP_SUCCESS == ret)
	{
		APP_DEBUG("<--- Configure PDP context ->>\r\n");
	}
	else
	{
		APP_DEBUG("<--- Fail to configure GPRS PDP, cause=%d. ->>\r\n", ret);
		return;
	}

	ret = Ql_GPRS_ActivateEx(M26pdpCntxtId, TRUE);
    if (ret == GPRS_PDP_SUCCESS)
    {
        m_GprsActState = 1;
		GSMLEDOn();
        APP_DEBUG("<-- Activate GPRS successfully. -->\r\n\r\n");
    }
	else
	{
        log_saveAbnormal("<-- Fail to activate GPRS, ", ret);
        return;
    }

    //4. Register Socket callback
    ret = Ql_SOC_Register(callback_soc_func, &customPara);
    if (SOC_SUCCESS == ret)
    {
		registe = 1;
        APP_DEBUG("<-- Register socket callback function -->\r\n");
    }
	else
	{
		log_saveAbnormal(" Fail to register socket callback, ", ret);
    }	
}
int findEybondIP(int port)
{
    if(0 == Ql_strcmp(netManage[port].ipStr, "www.shinemonitor.com"))
    {
        return 1;
    }

    return 0;
}

/*******************************************************************************
 Brief    : M26Net_close
 Parameter: 
 return   : 
*******************************************************************************/
void M26Net_manage(void)
{
	static s32 offset = 0;
	int ret;
	
	if (registe)
	{
		if (offset < sizeof(netManage)/sizeof(netManage[0]))
		{
			if (netManage[offset].flag == 1 && netManage[offset].status != M26_SUCCESS && netManage[offset].status != M26_CONNECTING)
			{
				if (netManage[offset].socketID < 0)
				{
					if (netManage[offset].mode == 1 || netManage[offset].mode == 2)
					{
						netManage[offset].socketID = Ql_SOC_Create(M26pdpCntxtId, SOC_TYPE_TCP);
						APP_DEBUG("TCP %s:%d!\r\n", netManage[offset].ipStr,  netManage[offset].port);
					}
					else
					{
						netManage[offset].socketID = Ql_SOC_Create(M26pdpCntxtId, SOC_TYPE_UDP);
						APP_DEBUG("UDP %s:%d!!\r\n", netManage[offset].ipStr,  netManage[offset].port);
					}
					
				    if (netManage[offset].socketID < 0)
				    {
						netManage[offset].status = M26_SOCKET_FAIL;
				        APP_DEBUG("<-- Fail to create socket, cause=%d. -->\r\n", netManage[offset].socketID);
//						Ql_OS_SendMessage(NET_TASK, NET_RESTART_ID, 0, 1);
						return ;
					}
					APP_DEBUG("socket id %d\r\n", netManage[offset].socketID);
					APP_DEBUG("offset %d\r\n", offset);
				}

				/* DNS做判断 */
				if (netManage[offset].ip == 0 && netManage[offset].status != M26_DNS_FAIL)
				{
					ret = Ql_IpHelper_ConvertIpAddr(netManage[offset].ipStr, (u32*)netManage[offset].ip);
			        if (SOC_SUCCESS != ret) // ip address is xxx.xxx.xxx.xxx
			        {
			            ret = Ql_IpHelper_GetIPByHostName(M26pdpCntxtId, offset, netManage[offset].ipStr, ipCallback);
						if (ret != 0)
						{
//							log_saveAbnormal("DNS API Err: ", ret);
							APP_DEBUG("DNS API Err: %d\r\n", ret);
							if (ret == -3)
							{
							    APP_DEBUG("Send net relink Message!!!!!!!!!!!\r\n");
//								Ql_OS_SendMessage(NET_TASK, NET_RESTART_ID, 0, findEybondIP(offset));
                                M26Net_close(offset);
                                return;

							}
						}
					}
					else  
					{
						netManage[offset].status = M26_IP_OK;
					}
					
				}
				else if(netManage[offset].status == M26_DNS_FAIL){
				    APP_DEBUG("DNS faile, close socket %d\r\n", offset);
				    M26Net_close(offset);
				}
				
				if (netManage[offset].status == M26_IP_OK)
				{
					APP_DEBUG("IP: %x\r\n", netManage[offset].ip )
					log_saveAbnormal(netManage[offset].ipStr, netManage[offset].port);
					if (netManage[offset].mode == 2)
					{
						ret = SSL_Open(&netManage[offset]);
					}
					else
					{
						ret = Ql_SOC_ConnectEx(netManage[offset].socketID, (u32)&netManage[offset].ip, netManage[offset].port, FALSE);
						APP_DEBUG("connet server ret = %d\r\n", ret);
					}

					if (SOC_SUCCESS == ret)
			        {
						if (netManage[offset].mode != 2)
						{
							netManage[offset].status = M26_SUCCESS;
							// log_save("Connect to server successfully\r\n");
						}
						else
						{
                            APP_DEBUG("connet server M26_CONNECTING\r\n", ret);
                            netManage[offset].status = M26_CONNECTING;
						}
			        }
					else if(SOC_WOULDBLOCK == ret)
					{
					    customPara = offset;
                        APP_DEBUG("connet server M26_CONNECTING -2\r\n", ret);
                        netManage[offset].status = M26_CONNECTING;
					}
					else
					{
						netManage[offset].status = M26_CONNECT_FAIL;
			            log_saveAbnormal("Fail to connect to server, cause=", ret);
			            M26Net_close(offset);
			        }
				}
			}
			offset++;
        }
		else 
		{
			offset = 0;
		}
	}//*/
}

static void ipCallback(u8 contexId, u8 requestId, s32 errCode,  u32 ipAddrCnt, u32* ipAddr)
{
	static int redns_count = 0;
	if (errCode)
	{
		APP_DEBUG("DEMO Get IP Fail: %d\r\n", errCode);
		netManage[requestId].status = M26_DNS_FAIL;
        log_saveAbnormal("DNS API Err: ", errCode);
		if(0 == (runTimeCheck(5, 21))&&(0==(++redns_count)%6))
		{
			redns_count = 0;
			log_save("feed stop!!\r\n");	
			Watchdog_stop();
		}
	}
	else
	{
		netManage[requestId].status = M26_IP_OK;
		Ql_memcpy(&netManage[requestId].ip, ipAddr, 4);
	}
}


/*******************************************************************************
 Brief    : socketClose
 Parameter: 
 return   : 
*******************************************************************************/
static void socketClose(s32 socketid)
{
	u8 port;
	M26Net_t *socke = M26Socket(socketid, &port);

	if (socke != null)
	{
		M26Net_close(port);
	}
}

/*******************************************************************************
 Brief    : M26Socket
 Parameter: 
 return   : 
*******************************************************************************/
M26Net_t *M26Socket(s32 socketid, u8 *offset)
{
	int i;

	for (i = 0; i < sizeof(netManage)/sizeof(netManage[0]); i++)
	{
		if (netManage[i].flag == 1 && netManage[i].socketID == socketid)
		{
			*offset = i;
			return &netManage[i];
		}
	}
	
	return null;
}

/*******************************************************************************
 Brief    : M26Socket
 Parameter: 
 return   : 
*******************************************************************************/
int M26Net_send(u8 port, u8* data, u16 len)
{
	int ret = -520;

	if ((port < sizeof(netManage)/sizeof(netManage[0]))
		&& netManage[port].flag == 1
		&& netManage[port].status == M26_SUCCESS
		)
	{
		if (netManage[port].mode == 2)
		{
			ret = SSL_Send(netManage[port].socketID, (u8*)data, len);
		}
		else
		{
			ret = Ql_SOC_Send(netManage[port].socketID, (u8*)data, len);
		}
	}

	return ret;
}


/*******************************************************************************
 Brief    : M26Net_close
 Parameter: 
 return   : 
*******************************************************************************/
static void Callback_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: deactivated GPRS successfully.-->\r\n"); 
    }
	else
	{
        APP_DEBUG("<--CallBack: fail to deactivate GPRS, cause=%d)-->\r\n", errCode); 
    }
	
    if (1 == m_GprsActState)
    {
        m_GprsActState = 0;
        APP_DEBUG("<-- GPRS drops down -->\r\n"); 
    }
}

static void Callback_Socket_Connect(u8 contextId, s32 errCode, void* customParam )
{
    s32 offset = *((s32 *)customParam);
    if (errCode == SOC_SUCCESS)
    {
        netManage[offset].status = M26_SUCCESS;
        // log_save("Connect to server successfully\r\n");
        APP_DEBUG("CallBack Connect to server successfully %d\r\n", *((s32 *)customParam));
    }
	else if(errCode == SOC_WOULDBLOCK)
	{
        APP_DEBUG("CallBack Connecting server %d\r\n", *((s32 *)customParam));
    }
	else
	{
	    netManage[offset].status = M26_CONNECT_FAIL;
        APP_DEBUG("CallBack Connect to server failed %d\r\n", *((s32 *)customParam));
        log_saveAbnormal("Fail to connect to server, cause=", errCode);
        M26Net_close(offset);

	}


}
/*******************************************************************************
 Brief    : M26Net_close
 Parameter: 
 return   : 
*******************************************************************************/
static void Callback_Socket_Close(s32 socketId, s32 errCode, void* customParam )
{	
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: close socket successfully.-->\r\n"); 
    }
    else if(errCode == SOC_BEARER_FAIL)
    {   
        APP_DEBUG("<--CallBack: fail to close socket,(socketId=%d,error_cause=%d)-->\r\n", socketId, errCode); 
    }
	else
	{
        APP_DEBUG("<--CallBack: close socket failure,(socketId=%d,error_cause=%d)-->\r\n", socketId, errCode); 
    }


    APP_DEBUG("<-- Close socket at module side -->\r\n"); 
    socketClose(socketId);   

}


/*******************************************************************************
 Brief    : Callback_Socket_Read
 Parameter: 
 return   : 
*******************************************************************************/
static void Callback_Socket_Read(s32 socketId, s32 errCode, void* customParam )
{
    s32 ret;
	u8 port;
	M26Net_t *socket = M26Socket(socketId, &port);
	Buffer_t *buf;
	
    if (errCode)
    {
        APP_DEBUG("<-- Close socket -->\r\n");
        socketClose(socketId);
        return;
    }
	buf = memory_apply(1560);
	if (buf == null )
	{
		APP_DEBUG("Socket Data Read memory alloc fail!!");
		return;
	}
	else if (socket == null)
	{
		APP_DEBUG("Rcveice net data but no find Socket connect.\r\n");
		return;
	}
	buf->lenght = 0;
	buf->size = 1560 - sizeof(Buffer_t);
	buf->payload = (u8*)(buf + 1);
	socket->status = M26_SUCCESS;
    do
    {
        ret = Ql_SOC_Recv(socket->socketID, buf->payload + buf->lenght, buf->size - buf->lenght);
        if((ret < SOC_SUCCESS) && (ret != SOC_WOULDBLOCK))
        {
            APP_DEBUG("<-- Fail to receive data, cause=%d.-->\r\n",ret);
            APP_DEBUG("<-- Close socket.-->\r\n");
            socketClose(socketId);
			memory_release(buf);
            break;
        }
        else if(SOC_WOULDBLOCK == ret)  // Read finish
        {
		END:
			socket->callback(port, buf);
            break;
        }
        else // Continue to read...
        {
			buf->lenght += ret;
            if (buf->size == buf->lenght)  // buffer if full
            {
                goto END;
            }
        }
    } while (TRUE);
}


/*******************************************************************************
 Brief    : Callback_Socket_Write
 Parameter: 
 return   : 
*******************************************************************************/
static void Callback_Socket_Write(s32 socketId, s32 errCode, void* customParam)
{
    if (errCode < 0)
    {
        APP_DEBUG("<-- Socket error(error code:%d), close socket.-->\r\n", errCode);
		
        socketClose(socketId);
    }
	else
	{
		Ql_OS_SendMessage(NET_TASK, NET_SEND_DATA_ID, 2, 0);
        APP_DEBUG("<-- You can continue to send data to socket -->\r\n");
    }
}

/*********************************FILE END*************************************/

