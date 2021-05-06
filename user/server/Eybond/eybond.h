/**************Copyright(C)锟斤拷2015-2026锟斤拷Shenzhen Eybond Co., Ltd.***************
  *@brief   : Eybodn平台锟斤拷睾锟斤拷锟�
  *@notes   : 2017.12.09 CGQ 锟斤拷锟斤拷   
*******************************************************************************/
#ifndef __EYBOND_H
#define __EYBOND_H	 

#include "typedef.h"
//#include "ql_type.h"

#define EYBOND_TASK		subtask3_id
#define EYBOND_DATA_PROCESS  (0x010001)

#define ESP_PROCESS_FREP        (1000)       //ms
#define EPS_ACK_OVERTIME        (15000)
#define ESP_WAIT_CNT            (EPS_ACK_OVERTIME/ESP_PROCESS_FREP)


typedef enum
{
    EYBOND_HEARTBEAT = 0x01,
    EYBOND_GET_PARA  = 0x02,
    EYBOND_SET_PATA  = 0x03,
    EYBOND_TRANS     = 0x04,
    EYBOND_GET_DEVICE_PARA = 0x11,
    EYBOND_SET_DEVICE_PARA = 0x12,
    EYBOND_GET_DEVICE_HISTORY = 0x13,
    EYBOND_REPORT_SPECIAL       = 0x16, 
    EYBOND_GET_COMMUNICATION = 0x1F,
    EYBOND_FILE_UPDATE        = 0x20,
    EYBOND_SOFT_UPDATE        = 0x21,
	EYBOND_DEVICE_UPDATE      = 0x22,
	EYBOND_DEVICE_UPDATE_STATE = 0x23,
    EYBOND_DEVICE_UPDATE_CANCEL = 0x24,
	EYBOND_UPDATE_INFO          = 0x25,
    EYBOND_UPDATE_DATA_SEND     = 0x26,
	EYBOND_UPDATE_DATA_STATE    = 0x27,
    EYBOND_UPDATE_DATA_CHECK    = 0x28,
	EYBOND_UPDATE_EXIT          = 0x29,
}EybondCode_e;

typedef struct
{
    u16_t serial;   //Serial number
    u16_t code;     //device code  
    u16_t msgLen;    //meseger lenght
    u8_t  addr;      //device addresss
    u8_t  func;      //funcation code
}EybondHeader_t;

typedef void (*AckCh)(Buffer_t *buf);

typedef struct
{
    u16_t PDULen;    //user Data Lenght;
    u16_t waitCnt;  //command wait process time
    AckCh ack;      //commad ack  road function
    EybondHeader_t head;    //protocol head
    u8_t PDU[];             //user Data
}ESP_t;

typedef u8_t (*funCodeExe)(ESP_t *esp);

typedef struct
{
    u16_t code;
    funCodeExe fun;
}funcationTab_t;





void proc_eybond_task(s32 taskId);
u8 ESP_cmd(Buffer_t *buf, AckCh ch);
void ESP_callback(u8 port, Buffer_t *buf);
u8 ESP_check(Buffer_t *buf);
void HistoryDataClean(void);
void specialData_send(void);

#endif //__EYBOND_H
/******************************************************************************/

