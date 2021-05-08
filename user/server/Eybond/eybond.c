/******************************************************************************           
* name:                  
* introduce:        
* author:           Luee                                     
******************************************************************************/ 
//sdk
#include <fibo_opencpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lib
#include "r_memory.h"
#include "typedef.h"
#include "r_stdlib.h"
#include "list.h"
#include "algorithm.h"
//app
#include "Debug.h"
#include "hard_watchdog.h"
#include "ble_task.h"
#include "utility.h"
#include "history.h"
#include "grid_tool.h"
#include "run_log.h"
#include "appTask.h"
#include "utility.h"
#include "Status.h"
#include "SysPara_File.h"
#include "eybond.h"
#include "Device.h"
#include "4G_net.h"
#include "ESP_Update.h"
#include "Sineng.h"
#include "ModbusDevice.h"
#include "Clock.h"

#define MAX_CMD_LEN				(0xA00)

#define HISTORY_SPACE_ADDR		(FLASH_EYBOND_HISTORY_ADDR)
#define HISTORY_SPACE_SIZE		(4096)
#define HISTORY_AREA_ADDR		(HISTORY_SPACE_ADDR + HISTORY_SPACE_SIZE)
#define HISTORY_AREA_SIZE 		(FLASH_EYBOND_HISTORY_SIZE - HISTORY_SPACE_SIZE)


static char EybondServer[] = "www.shinemonitor.com:502";
static ListHandler_t rcveList;
static u8 sPort;
static u16 overtime;
static u16 relinkCnt;
static u16 histprySaveSpace;	
static int historySaveCnt;

static void NetMsgAck_Eybond(Buffer_t *buf);
static void output(Buffer_t *buf);

static void ESP_process(void);
static u8_t espCmp(void *src, void *dest);
static u8_t heartbeat(ESP_t *esp);
static u8_t paraGet(ESP_t *esp);
static u8_t paraSet(ESP_t *esp);
static u8_t trans(ESP_t *esp);
static u8_t transAck(Device_t *dev);
static u8_t deviceDataGet(ESP_t *esp);
static u8_t historyData(ESP_t *esp);
static u8_t specialData_receive(ESP_t *esp);
static u8_t commnuicationData(ESP_t *esp);
//static void historySave(void);
  

const funcationTab_t funTab[] = {
    {EYBOND_HEARTBEAT,  heartbeat},
    {EYBOND_GET_PARA,   paraGet},
    {EYBOND_SET_PATA,   paraSet},
    {EYBOND_TRANS,      trans},
    {EYBOND_GET_DEVICE_PARA, deviceDataGet},
    {EYBOND_GET_DEVICE_HISTORY, historyData},
    {EYBOND_REPORT_SPECIAL,         specialData_receive}, 
    {EYBOND_GET_COMMUNICATION, commnuicationData},
    {EYBOND_FILE_UPDATE,    Update_file}, 
    {EYBOND_SOFT_UPDATE,    Update_soft},
    {EYBOND_DEVICE_UPDATE,  Update_device},
    {EYBOND_DEVICE_UPDATE_STATE, Update_deviceState},
    {EYBOND_DEVICE_UPDATE_CANCEL, Update_deviceCancel},
    {EYBOND_UPDATE_INFO, Update_info},
    {EYBOND_UPDATE_DATA_SEND, Update_dataRcve},
    {EYBOND_UPDATE_DATA_STATE, Update_rcveState},
    {EYBOND_UPDATE_DATA_CHECK, Update_dataCheck},
    {EYBOND_UPDATE_EXIT, Update_exit},
};

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
void proc_eybond_task(s32 taskId)
{
	u8 ret;
	ST_MSG msg;
	u8 count = 0;
	APP_DEBUG("Eybond task run!!\r\n");
	relinkCnt = 0;
	sPort = 0xff;
    r_memset(&msg,0,sizeof(ST_MSG));
	list_init(&rcveList);
		
    while(TRUE)
    {
        fibo_queue_get(EYBOND_TASK, (void *)&msg, 0);
        switch(msg.message)
        {
			case SYS_PARA_CHANGE:
                Net_close(sPort);
				sPort = 0xff;
			case APP_MSG_DEVTIMER_ID:
				ret = Net_status(sPort);
				if (ret == 0xFF)
				{
                    APP_DEBUG("Eybond start relink socket\r\n");
					ServerAddr_t *eybondServer = ServerAdrrGet(EYBOND_SERVER_ADDR);
					if (eybondServer != null)
					{
                        APP_DEBUG("Eybond Connect sever socket\r\n");
//			  			overtime = 0;
						sPort = Net_connect(1, eybondServer->addr, eybondServer->port, ESP_callback);
						memory_release(eybondServer);
					}
                    else
                    {
                        Buffer_t buf;
                        buf.lenght = sizeof(EybondServer);
                        buf.payload = (u8*)EybondServer;
                        //SysPara_Set(EYBOND_SERVER_ADDR, &buf);
                        parametr_set(EYBOND_SERVER_ADDR, &buf);
                    }
				}

                if (overtime++ > (75 * 2))
                {
                    relinkCnt++;
                    overtime = 0;
                    Net_close(sPort);
                    sPort = 0xFF;
                    APP_DEBUG("Eybond Close socket %d, relinkCnt = %d\r\n", sPort, relinkCnt);
                }

                if(relinkCnt > 10){
                    APP_DEBUG(" relinkCont over and reboot!!!!!!!!!!!!!!!!\r\n");
                    Watchdog_stop();
                    relinkCnt = 0;
                }


				//if ((runTimeCheck(4, 19) == 0 ) &&(0 >= (historySaveCnt--))){
				//	historySave();
				//}
            case EYBOND_DATA_PROCESS:
                    ESP_process();
                break;
	        default:
	            break;
        }
    }
}

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
static void ESP_process(void)
{
    ESP_t *esp = (ESP_t *)list_nextData(&rcveList, null);
    
    if (null == esp){
        return;
    }
    
    while (esp != null && esp->waitCnt != 0)
    {
		if (++esp->waitCnt > ESP_WAIT_CNT)  //wait prcesso overtime
        {
			list_nodeDelete(&rcveList, esp);
            return;
        }
		esp = (ESP_t *)list_nextData(&rcveList, esp);
    }
    
    if (esp != null)
    {		
        overtime = 0;
        relinkCnt = 0;
        funcationTab_t *exe = (funcationTab_t*)ALG_binaryFind(esp->head.func, (void*)funTab, (void*)(&funTab[sizeof(funTab)/sizeof(funTab[0])]), sizeof(funTab[0]));
        if (exe != null)
        {
            if (0 != exe->fun(esp))
            {
                esp->waitCnt++;
                return;
            }
        }
    }
    list_nodeDelete(&rcveList, esp);
}

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
static u8_t commnuicationData(ESP_t *esp)
{
#pragma pack(1)
    typedef struct
    {
		u8_t state;
    }rcve_t;
#pragma pack() 	
	EybondHeader_t *ack;
	Buffer_t buf;
	u16_t code;
    rcve_t *para = (rcve_t*)(esp->PDU);
    

	Sineng_PVData(esp->head.addr, para->state, &code, &buf);
	if (buf.lenght == 0 || buf.payload == null)
	{
		ack = &esp->head;
		ack->msgLen = ENDIAN_BIG_LITTLE_16(3);
		para->state = 0x11;
		buf.lenght = sizeof(EybondHeader_t) + 1;
		buf.payload = (u8_t*)ack;
	}
	else
	{
		ack = memory_apply(sizeof(EybondHeader_t) + buf.lenght);
		r_memcpy(ack, &esp->head, sizeof(EybondHeader_t));
		r_memcmp(ack + 1, buf.payload, buf.lenght);
		ack->code = code;
		ack->msgLen = buf.lenght + 2;
		memory_release(buf.payload);
		buf.lenght += sizeof(EybondHeader_t);
		buf.payload = (u8_t*)ack;
	}
	
    esp->ack(&buf); 
    memory_release(buf.payload);
    return 1;
}

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
static u8_t deviceDataGet(ESP_t *esp)
{
    static u8_t overTime = 0;

    u8_t *ackBuf;
    Buffer_t buf;
    u8_t *endAddrH = null;
	u8_t *endAddrL = null;
	u16_t regAddr;
    EybondHeader_t *ackHead = null;
	Node_t *tail;
    DeviceOnlineHead_t *head;
    
    overTime++;
    buf.lenght = 0;
    buf.size = MAX_CMD_LEN;
    buf.payload = memory_apply(buf.size);

    tail = onlineDeviceList.node;
    
    if (tail != null && buf.payload != null)
    {
        APP_DEBUG("enter the upload device data\r\n");
        do {
            Node_t *node;
            
            head = (DeviceOnlineHead_t*)tail->payload;
            node = head->cmdList.node;
            tail = tail->next;
            if (node != null)
            {
                CmdBuf_t *cmd;
                u8_t *para;
                u16_t loadLen=0;
				u8_t cmdLen = 0;
                u8_t code = 0;
                u16_t updateFlag = 0;
                
                ackBuf = &buf.payload[buf.lenght];
                do {
                    cmd = (CmdBuf_t*)node->payload;
                    node = node->next;

                    /* 妫€娴嬫暟鎹槸鍙樺寲 */
                    updateFlag++;

                    if ((cmd->state&(1<<0)) == 0)
                    {
                       cmd->state |= (1<<0);
                    }

                    if (code != cmd->fun)
                    { 
                        code = cmd->fun;   
                        if (ackHead != null)
                        {
                            r_memcpy(ackHead, &esp->head, sizeof(EybondHeader_t));
                            ackHead->addr = head->addr;
                            ackHead->code = ENDIAN_BIG_LITTLE_16(head->code);
                            ackHead->msgLen = ENDIAN_BIG_LITTLE_16(loadLen + 4);
                            para = (u8_t*)(ackHead+1);
                            *para++ = 0x01;
                            *para++ = 0x02;
                        }
                        ackHead = (EybondHeader_t*)ackBuf;
                        ackBuf += sizeof(EybondHeader_t) + 2;
                        loadLen = 0;
                    }
                    if (((head->flag&0x01) == 0 || ((head->flag>>1) != 0 && ((cmdLen>>4) == (head->flag>>1))))
							&& endAddrH != null && ((((*endAddrH<<8) | *endAddrL) + 1) == cmd->startAddr))
                    {
                        *endAddrH = cmd->endAddr>>8;
						*endAddrL = cmd->endAddr;
                    }
                    else
                    {
						regAddr = ENDIAN_BIG_LITTLE_16(cmd->startAddr);
                        r_memcpy(ackBuf, &regAddr, 2);
						ackBuf += 2;
						regAddr = ENDIAN_BIG_LITTLE_16(cmd->endAddr);
						r_memcpy(ackBuf, &regAddr, 2);
                        endAddrH = ackBuf++;
						endAddrL = ackBuf++;
                        loadLen += 4;
                    }
					cmdLen = (cmd->endAddr - cmd->startAddr + 1)<<1; // User modbus 01 02 code
                    r_memcpy(ackBuf, cmd->buf.payload, cmd->buf.lenght);
					r_memset(&ackBuf[cmd->buf.lenght], 0, cmdLen - cmd->buf.lenght);
                    ackBuf += cmdLen;
                    loadLen += cmdLen; 
                } while (node != head->cmdList.node && head->cmdList.node != null);
                
                if (updateFlag != 0)
                {
                    r_memcpy(ackHead, &esp->head, sizeof(EybondHeader_t));
                    ackHead->addr = head->addr;
                    ackHead->code = ENDIAN_BIG_LITTLE_16(head->code);
                    ackHead->msgLen = ENDIAN_BIG_LITTLE_16(loadLen + 4);
                    para = (u8_t*)(ackHead+1);
                    if (tail == onlineDeviceList.node)
                    {
                        *para++ = 0x00;
                    }
                    else
                    {
                        *para++ = 0x01;
                    }
                    if ((int)ackHead == (int)&buf.payload[buf.lenght])
                    {
                        *para++ = 0x00;
                    }
                    else
                    {
                        *para++ = 0x01;
                    }
                    ackHead = null;
                    buf.lenght += ackBuf - &buf.payload[buf.lenght];
                }
                else
                {
                    r_memcpy(ackHead, &esp->head, sizeof(EybondHeader_t));
                    ackHead->addr = head->addr;
                    ackHead->code = ENDIAN_BIG_LITTLE_16(head->code);
                    ackHead->msgLen = ENDIAN_BIG_LITTLE_16(4);
                    para = (u8_t*)(ackHead+1);
                    if (tail == onlineDeviceList.node)
                    {
                        *para++ = 0x00;
                    }
                    else
                    {
                        *para++ = 0x01;
                    }
                    *para++= 0;
                    ackHead = null;
                    buf.lenght = para - buf.payload;
                }

                if (buf.lenght > 800)
                {
                    esp->ack(&buf); 
                    buf.lenght = 0;
                }
                overTime = 0;
            } 
        }while (tail != onlineDeviceList.node && onlineDeviceList.node != null);
		historySaveCnt = histprySaveSpace + 15;
    }
    else
    {
        u8_t *para;
        APP_DEBUG("buf malloc failed!!\r\n");
        buf.lenght = sizeof(EybondHeader_t) + 2;
        //buf.payload = memory_apply(buf.lenght);
        ackHead = (EybondHeader_t*)buf.payload;
        r_memcpy(ackHead, &esp->head, sizeof(EybondHeader_t));
        ackHead->msgLen = ENDIAN_BIG_LITTLE_16(4);
        para = (u8_t*)(ackHead + 1);
        para[0] = 2;
        para[1] = 0;
    }
    esp->ack(&buf); 
    memory_release(buf.payload);

    if (overTime > 5)
    {
        log_save("No device online reboot!\r\n");
        //Ql_OS_SendMessage(DEVICE_TASK, DEVICE_RESTART_ID, 0, 0);      //Luee 之后再加入
        overTime = 0;
    }
    
    return 0;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
*******************************************************************************/
static u8_t heartbeat(ESP_t *esp) {
#pragma pack(1)
  typedef struct {
    u8_t year;
    u8_t month;
    u8_t day;
    u8_t hour;
    u8_t min;
    u8_t secs;
  } rcve_t;
#pragma pack()

  rcve_t *para = (rcve_t *)(esp->PDU);
  //EybondHeader_t *ack;
  u8 ack_buf[sizeof(EybondHeader_t) + 18]={0};
  EybondHeader_t *ack=(EybondHeader_t *)ack_buf;
  Buffer_t buf;
  Clock_t time;

  time.year = para->year;
  time.month = para->month;
  time.day = para->day;
  time.hour = para->hour;
  time.min = para->min;
  time.secs = para->secs;
  APP_DEBUG("ESP time: %d-%d-%d %d:%d:%d\r\n", time.year, time.month, time.day, time.hour, time.min, time.secs);

  Clock_Set(&time);

  parametr_get(DEVICE_PNID, &buf);
  //ack = memory_apply(sizeof(EybondHeader_t) + 18);
  r_memset(ack, 0, sizeof(EybondHeader_t) + 18);
  //if (ack != null) {
  if (1) {
    r_memcpy(ack, &esp->head, sizeof(EybondHeader_t));    //得到回复头
    if (buf.payload == null || (buf.lenght != 14 &&  buf.lenght != 18)) {
      APP_DEBUG("PN ERR: %d, %s\r\n", buf.lenght, buf.payload);
      buf.lenght = 14;
      r_memcpy(ack + 1, "EYBONDERR00000", 14);
    } else {
      r_memcpy(ack + 1, buf.payload,  buf.lenght);
    }
    memory_release(buf.payload);

    ack->msgLen = ENDIAN_BIG_LITTLE_16(buf.lenght + 2);
    buf.lenght = sizeof(EybondHeader_t) + buf.lenght;
    buf.payload = (u8_t *)ack;
    esp->ack(&buf);
  }
  memory_release(buf.payload);
  return 0;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
*******************************************************************************/
static u8_t paraGet(ESP_t *esp)
{
    u8_t i;
    u16_t len;
	Buffer_t *paraBufTab;
    Buffer_t *ackBufTab;
    Buffer_t buf;
    u8_t *ackBuf;
    EybondHeader_t *ackHead;
    
    u8_t *para = esp->PDU;
    
    i = esp->PDULen - sizeof(EybondHeader_t);
    
    ackBufTab = memory_apply(i * sizeof(Buffer_t) + 8);
    ackBuf = (u8_t*)ackBufTab;
	paraBufTab = ackBufTab;
    len = 0;
    
    while (i--)
    {
		buf.lenght = 0;
		buf.payload = null;
		
		SysPara_Get(*para, &buf);
        len += buf.lenght;
        r_memcpy(ackBuf, &buf, sizeof(Buffer_t));
        ackBuf += sizeof(Buffer_t);
		para++;
    }
    
    i = esp->PDULen - sizeof(EybondHeader_t);
	
    buf.lenght = i*2 + len + (i*sizeof(EybondHeader_t));
    buf.payload = memory_apply(buf.lenght + 20);
    ackBuf = buf.payload;
    para = esp->PDU;
    
    
    while (i--)
    {  
        ackHead = (EybondHeader_t*)ackBuf;
        r_memcpy(ackHead, &esp->head, sizeof(EybondHeader_t));
        ackBuf += sizeof(EybondHeader_t);
        if (ackBufTab->lenght > 0 && ackBufTab->payload != null)
        {
            *ackBuf++ = 0;
            *ackBuf++ = *para++;
			
            r_memcpy(ackBuf, ackBufTab->payload, ackBufTab->lenght);
            ackBuf += ackBufTab->lenght;
        }
        else
        {
            *ackBuf++ = 1;
            *ackBuf++ = *para++;
			ackBufTab->lenght = 0;
        }
		//byte alignment/*
		*(((u8*)ackHead) + 4) = (ackBufTab->lenght + 4) >> 8;
		*(((u8*)ackHead) + 5) = (ackBufTab->lenght + 4); //*/

		//ackHead->msgLen = ENDIAN_BIG_LITTLE_16(ackBufTab->lenght + 4);
        memory_release(ackBufTab->payload);
        ackBufTab++; 
    }
    esp->ack(&buf); 
	memory_release(paraBufTab);
    memory_release(buf.payload);
    
    return 0;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
*******************************************************************************/
static u8_t paraSet(ESP_t *esp)
{
    Buffer_t buf;
	int ret;

    EybondHeader_t *ackHead;
    u8_t *para = esp->PDU;

    buf.lenght = ENDIAN_BIG_LITTLE_16(esp->head.msgLen) - 3;
    buf.payload = para + 1;
	ret = SysPara_Set(*para, &buf);
    
    ackHead = &esp->head;
    ackHead->msgLen = ENDIAN_BIG_LITTLE_16(4);
    para[1] = para[0];
    para[0] = ret;
	
    buf.lenght = sizeof(EybondHeader_t) + 2;
    buf.payload = (u8_t*)ackHead;
    esp->ack(&buf); 
    memory_release(buf.payload);
	
    return 0;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
*******************************************************************************/
static u8_t trans(ESP_t *esp)
{
    Device_t *dev;
    DeviceCmd_t *cmd;
    
    dev = list_nodeApply(sizeof(Device_t));
    cmd = list_nodeApply(sizeof(DeviceCmd_t));
    
    cmd->waitTime = 1500;
    cmd->state = 0;
    cmd->ack.size = 0;
    cmd->ack.payload = null;
    cmd->cmd.size = ENDIAN_BIG_LITTLE_16(esp->head.msgLen) - 2;
    cmd->cmd.lenght = cmd->cmd.size;
    cmd->cmd.payload = memory_apply(cmd->cmd.size);
    r_memcpy(cmd->cmd.payload, esp->PDU, cmd->cmd.size);
    
    dev->cfg = null;
    dev->callBack = transAck;
    dev->explain = esp;
	dev->type = DEVICE_TRANS;
    
    list_init(&dev->cmdList);
    list_bottomInsert(&dev->cmdList, cmd);
    Device_inset(dev);

    return 1;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
*******************************************************************************/
static u8_t transAck(Device_t *dev)
{
    ESP_t *esp;
    EybondHeader_t *ackHead;
    DeviceCmd_t *cmd;
    Buffer_t buf;
    
    esp = list_find(&rcveList, espCmp, dev->explain);

    if (esp != null)
    {
        cmd = (DeviceCmd_t*)dev->cmdList.node->payload;
        
        buf.lenght = cmd->ack.lenght + sizeof(EybondHeader_t);
        buf.payload = memory_apply(buf.lenght);

        ackHead = (EybondHeader_t*)buf.payload;
        r_memcpy(ackHead, &esp->head, sizeof(EybondHeader_t));
        r_memcpy(ackHead+1, cmd->ack.payload, cmd->ack.lenght);
        ackHead->msgLen = ENDIAN_BIG_LITTLE_16(cmd->ack.lenght + 2);
        esp->ack(&buf);
        memory_release(buf.payload);
        list_nodeDelete(&rcveList, esp);
    }
    Device_remove(dev);

    return 0;
}

extern u8_t g_ret_flag; 
/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
*******************************************************************************/
static u8_t specialData_receive(ESP_t *esp)
{
    EybondHeader_t *head = &esp->head;
    u8_t send_buf[9]={0xF0,0x01,0xFF,0xFF,0x00,0x03,0xFF,0x16,0x01};
    g_ret_flag = r_memcmp(head,send_buf,9);
    APP_DEBUG("<-- g_ret_flag = %d -->\r\n",g_ret_flag);
    return 0;
}

/*******************************************************************************
  * @brief
  * @note   None
  * @param  None
*******************************************************************************/
static u8_t espCmp(void *src, void *dest) {
  return r_memcmp(src, dest, sizeof(ESP_t));
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
*******************************************************************************/
static u8_t historyData(ESP_t *esp)
{
#pragma pack(1)
    typedef struct
    {
		u8_t flag;
        u8_t spaceH;
        u8_t spaceL;
    }rcve_t;
#pragma pack() 

    Buffer_t buf;
    EybondHeader_t *ackHead;
	rcve_t *para = (rcve_t*)(esp->PDU);
/*
	if (histprySaveSpace != ((para->spaceH<<8) | para->spaceL))
	{
		histprySaveSpace = ((para->spaceH<<8) | para->spaceL);
		FlashEquilibria_write(&saveSpace, &histprySaveSpace);
	}
	
	buf.size = MAX_CMD_LEN;
	buf.payload = memory_apply(buf.size);
	ackHead = (EybondHeader_t *)buf.payload;
	while (buf.payload)
	{
        buf.lenght = FlashFIFO_get(&historyHead, &buf);

		if (buf.lenght) 
		{
			if (crc16_standard(CRC_RTU, buf.payload, buf.lenght) == 0)	 
			{
				ackHead->serial = esp->head.serial;
				ackHead->func = esp->head.func;
				buf.payload[8] = 0x01;
				buf.lenght -= 2;
				
				break;
			}
		}
		else
		{
			r_memcpy(ackHead, &esp->head, sizeof(EybondHeader_t));
			ackHead->msgLen = ENDIAN_BIG_LITTLE_16(3);
			buf.payload[8] = 0x00;
			buf.lenght = sizeof(EybondHeader_t) + 1;
            break;
		}
	}
*/
      //没有历史数据直接回复  Luee
      buf.size = MAX_CMD_LEN;
	    buf.payload = memory_apply(buf.size);
	    ackHead = (EybondHeader_t *)buf.payload;

      r_memcpy(ackHead, &esp->head, sizeof(EybondHeader_t));
			ackHead->msgLen = ENDIAN_BIG_LITTLE_16(3);
			buf.payload[8] = 0x00;
			buf.lenght = sizeof(EybondHeader_t) + 1;
      //以上几行为新增

    esp->ack(&buf);
	  memory_release(buf.payload);
    return 0;
}

