/**************Copyright(C)，2015-2026，Shenzhen Eybond Co., Ltd.***************
  *@brief   : 网络相关函数
  *@notes   : 2017.12.06 CGQ 创建   
*******************************************************************************/
#ifndef __NET_H
#define __NET_H	 

#include "typedef.h"
#include "ql_type.h"

#define NET_LED			(PINNAME_NETLIGHT)
#define GSM_LED			(PINNAME_RTS)

#define NET_TASK		main_task_id
#define NET_SEND_DATA_ID		0x00F900
#define NET_RESTART_ID			0x00F901


typedef void (*NetDataCallback)(u8 port, Buffer_t *buf);


void proc_net_task(s32 taskId);
u8 Net_connect(u8 mode, char *ip, u16 port, NetDataCallback netCallback);
void Net_close(u8 port);
void Net_send(u8 port, u8 * pData, u16 len);
u8 Net_status(u8 port);
void NetLEDOn(void);
void NetLEDOff(void);
void GSMLEDOn(void);
void GSMLEDOff(void);


#endif //__NET_H
/******************************************************************************/

