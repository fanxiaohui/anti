/**************Copyright(C)锟斤拷2015-2026锟斤拷Shenzhen Eybond Co., Ltd.***************
  *@brief   : Hard.c 锟借备锟皆接碉拷元硬锟斤拷锟接口层定锟斤拷
  *@notes   : 2017.04.20 CGQ 锟斤拷锟斤拷   
*******************************************************************************/
#ifndef __DEVICE_H
#define __DEVICE_H	 

#include "Typedef.h"
#include "List.h"
#include "deviceIO.h"

#define DEVICE_LED		(PINNAME_RI)

#define DEVICE_ACK_SIZE (0x400)   //device default data ack data rcveice buffer size
#define DEVICE_CMD_ID			(0x0F8000)
#define DEVICE_RESTART_ID		(0x0F8010)
#define DEVICE_PV_SCAN_ID		(0x0F80F0)
#define DEVICE_PV_GET_ID		(DEVICE_PV_SCAN_ID + 1)


#define DEVICE_TASK				subtask2_id


typedef void (*DeviceCallBack)(ListHandler_t *cmdList);

typedef enum
{
	DEVICE_MOBUS = 0x5433,
	DEVICE_TRANS = 0x5400,
    DEVICE_GRID = 0x5430,
    DEVICE_ARTI = 0x5432,
}DeviceType_e;

typedef struct Device
{
    u8_t lock;        //device lock 
    u16_t type;	      //device type		 
    void *explain;	  //
    ST_UARTDCB *cfg; //device config msg
    u8_t (*callBack)(struct Device *dev);  //command end call back function
    ListHandler_t cmdList;  //device command tab
}Device_t;  //锟借备锟斤拷锟斤拷锟结构锟斤拷

typedef struct DeviceCmd
{
    u8_t  state; 
    u16_t waitTime;
    Buffer_t ack;     //应锟斤拷指锟筋缓锟斤拷
	Buffer_t cmd;    //锟斤拷锟斤拷指锟筋缓锟斤拷
}DeviceCmd_t; //锟借备指锟斤拷峁癸拷锟�

extern ListHandler_t DeviceList;


void Device_add(Device_t *device);
void Device_inset(Device_t *device);
void Device_remove(Device_t *device);
void Device_clear(void);
void DeviceCmd_clear(Device_t *dev);
void Device_removeType(u16_t type);

#endif //__DEVICE_H
/******************************************************************************/

