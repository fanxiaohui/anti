/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : M26Net.h
 * @Author  : CGQ
 * @Date    : 2017-12-08
 * @Brief   : 
 ******************************************************************************/
#ifndef __M26_NET_
#define __M26_NET_

#include "net.h"

typedef enum
{
	M26_SUCCESS,
	M26_WAIT,
	M26_DNS_FAIL,
	M26_IP_OK,
	M26_SOCKET_FAIL,
	M26_CONNECT_FAIL,
    M26_CONNECTING,
	M26_CLOSE,
}M26NetState_e;

typedef struct 
{
    u8    flag;     // link user flag
    u8    mode;     // link connect mode
    u16   port;		// net port
    u32   ip;    // link inter net IP -> ___.___.___.___
    s32   socketID;	// socket id 
    M26NetState_e status; // link status
    char  ipStr[52]; 		// user connet IP/DNS, lake
    NetDataCallback callback;  //rcveData callback fun
}M26Net_t;

extern s32 m_GprsActState;

void M26Net_init(void);
u8 M26Net_open(u8 mode, char *ip, u16 port, NetDataCallback netCallback);
void M26Net_close(u8 port);
int M26Net_send(u8 port, u8* data, u16 len);
void M26Net_closeAll(void);
void M26Net_manage(void);
u8 M26Net_status(u8 port);
void M26Net_ready(void);
M26Net_t *M26Socket(s32 socketid, u8 *offset);


#endif //__M26_NET_
/******************************************************************************/
