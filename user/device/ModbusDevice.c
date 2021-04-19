/**************Copyright(C)，2015-2026，Shenzhen Eybond Co., Ltd.***************
  *@brief   : ModbusDevice.c  MOSOEC Modbus
  *@notes   : 2017.08.04 CGQ   
*******************************************************************************/
#include "ModbusDevice.h"
#include "modbus.h"
#include "Protocol.h"
#include "list.h"
#include "r_stdlib.h"
#include "memory.h"
#include "Debug.h"


ListHandler_t onlineDeviceList;      //sucess online device tab


static void addrFind(void);
static u8_t addrFindProcess(Device_t *dev);
static void protocolFind(Device_t *dev);
static u8_t protocolFindProcess(Device_t *dev);
static u8_t ackProcess(Device_t *dev);
static u8_t cmdTabCheck(void *load, void *changeData);
static void onlineDeviceAddr(Device_t *dev);
static u8_t onlineDeviceRemove(void *dev, void *point);
static u8_t onlineDeviceCmdAdd(void *load, void *changePoint);
static u8_t onlineDeviceCmdRemove(void *load, void *changePoint);
static u8_t onlineDeviceCmp(void *src, void *dest);
static u8_t onlineDeviceCmdCmp(void *src, void *dest);
static void onlineDevicePrintf(DeviceOnlineHead_t *head);
static u8_t onlineCmdTabPrintf(void *load, void *changeData);


/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void ModbusDevice_init(void)
{
	list_init(&onlineDeviceList);

	if (ModbusDevice.head != null || MeterDevice.head != null)
	{
		addrFind();
	}
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void ModbusDevice_clear(void)
{
    list_trans(&onlineDeviceList, onlineDeviceRemove, null);
    list_delete(&onlineDeviceList);
    memory_release(ModbusDevice.cfg);
    memory_release(MeterDevice.cfg);
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t addrCmp(void *load, void *val)
{
	Device_t *dev = load;

	if (dev->type == DEVICE_MOBUS)
	{
		int i;
		DeviceExplain_t *exp = dev->explain;
		u8_t *addrTab = val;
		for (i = 0; *addrTab != 0; i++, addrTab++)
		{
			if (exp->addr == *addrTab)
			{
				*addrTab = 0xFF;
				break;
			}
		}
	}
	 
	return 1;
}

static void addrFind(void)
{
	int i; 
	Device_t *dev;
	DeviceExplain_t *exp;
	u8_t addrTab[64];

	r_memset(addrTab, 0, sizeof(addrTab));
	r_memcpy(addrTab, ModbusDevice.addrTab, sizeof(addrTab));

	/* 如果查找到指定地址，本地址对应数组数据为0xFF */
	list_trans(&DeviceList, addrCmp, addrTab);

	for (i = 0; addrTab[i] != 0; i++)
	{
        if (addrTab[i] != 0xFF)
        {
            ModbusGetCmd_t *cmd = null;
            exp = memory_apply(sizeof(DeviceExplain_t));

			if (MeterDevice.head != null && addrTab[i] >= MeterDevice.startAddr && addrTab[i] <= MeterDevice.endAddr)
            {
                dev = list_nodeApply(sizeof(Device_t));
                cmd = MeterDevice.head->findCmd;
                exp->head = MeterDevice.head;
                if (MeterDevice.cfg != null)
                {
                    dev->cfg = MeterDevice.cfg;
                }
                else
                {
                    dev->cfg = MeterDevice.head->hardCfg;
                }
                APP_DEBUG("add meter \r\n");
            }
            else if (ModbusDevice.head != null)
            {
                dev = list_nodeApply(sizeof(Device_t));
                cmd = ModbusDevice.head->findCmd;
                exp->head = ModbusDevice.head;
                if (ModbusDevice.cfg == null)
                {
                    dev->cfg = ModbusDevice.head->hardCfg;
                }
                else
                {
                    dev->cfg = ModbusDevice.cfg; 
                }
            }

            if (cmd != null)
            {
                exp->addr = addrTab[i];
                dev->type = DEVICE_MOBUS;
                dev->callBack = addrFindProcess;
                dev->explain = exp;
                list_init(&dev->cmdList);
                
                APP_DEBUG("add cmd %d, %d, %d \r\n", cmd->fun, cmd->start, cmd->end);

                /* 为底层命令对列只添加了对应的一条命令，原因是长度为sizeof(ModbusGetCMD_t)这里的限制*/
                Modbus_GetCmdAdd(&dev->cmdList, addrTab[i], cmd, sizeof(ModbusGetCmd_t));
                Device_add(dev);
            }
		}
	}
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t addrFindProcess(Device_t *dev)
{
    u8_t ret = 0;
    DeviceCmd_t *cmd = (DeviceCmd_t *)(dev->cmdList.node->payload);
    
    dev->lock = 0;

    if (ModbusDevice.monitorCount <= onlineDeviceList.count)
    {
        log_save("Monitor num over!!\r\n");
        Device_remove(dev);
    }
	else if (0 <= Modbus_CmdCheck(&cmd->cmd, &cmd->ack))
	{
		protocolFind(dev);
        ret = 1;
    }   

    return ret;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void protocolFind(Device_t *dev)
{
    DeviceExplain_t *exp =(DeviceExplain_t *)(dev->explain);

    if (exp->head->procmd != null && exp->head->procmd->code == 0)
    {
        APP_DEBUG("Add the protocol adapjust!!!!!!!!!!!!!!!!!!\r\n");
        dev->explain = null;
		DeviceCmd_clear(dev);

        dev->callBack = protocolFindProcess;
		dev->explain = exp;

		Modbus_GetCmdAdd(&dev->cmdList, exp->addr, exp->head->procmd->cmdTab,\
						sizeof(exp->head->procmd->cmdTab[0])* exp->head->procmd->count);
    }
    else
    {
        protocolFindProcess(dev);
    }
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t protocolFindProcess(Device_t *dev)
{   
    u8_t ret = 0;
    ModbusGetCmdTab_t *cmdTab = null;
    DeviceExplain_t *exp =(DeviceExplain_t *)(dev->explain);

    dev->lock = 0;
	if (exp->head->devicePro != null)
	{
    	list_trans(&dev->cmdList, exp->head->devicePro, &cmdTab);
	}
	else
	{
		cmdTab = exp->head->procmd;
	}
   

	if (cmdTab == null)
	{
		Device_remove(dev);
		addrFind();
	}
    else if (ModbusDevice.monitorCount > onlineDeviceList.count)
    {
        DeviceExplain_t *exp;
        
        ret = 1;
        exp = (DeviceExplain_t *)(dev->explain);
        dev->explain = null;
		DeviceCmd_clear(dev);
        dev->callBack = ackProcess;
		dev->explain = exp;
        exp->code = cmdTab->code;
		exp->flag = cmdTab->flag&(~(1<<7));

		if (cmdTab->flag & (1<<7)) //user to must protocol
		{
			int i; 
			
			for (i = cmdTab->count, cmdTab = (ModbusGetCmdTab_t*)(cmdTab->cmdTab);\
					i > 0 && exp->code == cmdTab->code; i--, cmdTab++)
			{
				if (cmdTab->flag == 0)
				{
					Modbus_GetCmdAdd(&dev->cmdList, exp->addr, cmdTab->cmdTab,\
							sizeof(cmdTab->cmdTab[0]) * cmdTab->count);
				}
				else
				{
					Modbus_GetCmdAdd(&dev->cmdList, cmdTab->flag, cmdTab->cmdTab,\
							sizeof(cmdTab->cmdTab[0]) * cmdTab->count);
				}
			}
		}
		else
		{
			exp->flag = cmdTab->flag;
        	Modbus_GetCmdAdd(&dev->cmdList, exp->addr, cmdTab->cmdTab,\
							sizeof(cmdTab->cmdTab[0]) * cmdTab->count);
		}
    }
    else
    {
        log_save("Monitor num over!!\r\n");
       Device_remove(dev);
    }

    return ret;
}

// static Buffer_t memStateBuf;
// static void rcveMemStatic(u8 *data, u16 len)
// {
//     if (memStateBuf.size > memStateBuf.lenght && memStateBuf.payload != null)
//     {
//         len = memStateBuf.size - memStateBuf.lenght > len ? len : memStateBuf.lenght;

//         r_memcpy(&memStateBuf.payload[memStateBuf.lenght], data, len);
//         memStateBuf.lenght += len;
//     }
// }

// static void memoryLog(void)
// {
//     memStateBuf.size = 1024;
//     memStateBuf.lenght = 0;
//     memStateBuf.payload = memory_apply(memStateBuf.size);

//     memory_trans(rcveMemStatic);
//     memStateBuf.payload[memStateBuf.lenght] = '\0';
//     log_save( memStateBuf.payload);
//     memory_release(memStateBuf.payload);
// }
/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t ackProcess(Device_t *dev)
{
    u8_t checkResult = 0;

    DeviceExplain_t *exp = (DeviceExplain_t *)dev->explain;
    
    dev->lock = 0;
    
    if (exp->head->ackDataPro != null)
    {
        list_trans(&dev->cmdList, exp->head->ackDataPro, &checkResult);
    }
	else
	{
		list_trans(&dev->cmdList, cmdTabCheck, &checkResult);
	}
    
    if (checkResult == 0)  //Device ack Data OK
    {
        exp->tryCnt = 0;
        onlineDeviceAddr(dev);	
    }
    else if (exp->tryCnt++ > 10)
    {
		DeviceOnlineHead_t *head = list_find(&onlineDeviceList, onlineDeviceCmp, dev);
		if (head != null)
		{
			log_saveAbnormal("Device unline: ", exp->addr);
			onlineDeviceRemove(head, null);
			list_nodeDelete(&onlineDeviceList, head);
		}
        log_saveAbnormal("Device remove: ", exp->addr);
		Device_remove(dev);
		addrFind();  
        //memoryLog();
    }

    return 0;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t cmdTabCheck(void *load, void *changeData)
{
	DeviceCmd_t *cmd= (DeviceCmd_t*)load;
	int ret;
    
    
    if (0 == cmd->state)
    {
		ret = Modbus_CmdCheck(&cmd->cmd, &cmd->ack);
		cmd->state = (u8_t)ret;

		if (ret == 0)
		{}
		else if (ret >  0)
		{
			*((u8_t*)changeData) |= (u8_t)ret;
		}
		else if (ret < 0)
		{
			goto ERR;
		}
    }
    else
    {
        goto ERR;
    }
    
    return 1;
ERR:
	*((u8_t*)changeData) |= 0x10;

	return 0;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void onlineDeviceAddr(Device_t *dev)
{
    DeviceOnlineHead_t *head = list_find(&onlineDeviceList, onlineDeviceCmp, dev);

    //memoryLog();
    if (head == null)
    {
		if (ModbusDevice.monitorCount > onlineDeviceList.count )
		{
			DeviceExplain_t *exp = dev->explain;
	        head = list_nodeApply(sizeof(DeviceOnlineHead_t));
			list_bottomInsert(&onlineDeviceList, head);
			head->addr = exp->addr;
			head->flag = exp->flag;
			head->code = exp->code;
			list_init(&head->cmdList);
			log_saveAbnormal("Device online: ", exp->addr);
		}
		else
		{
			log_save("Monitor num over!!\r\n");
			Device_remove(dev);
			return;
		}
    }
    
    list_trans(&dev->cmdList, onlineDeviceCmdAdd, &head->cmdList);
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t onlineDeviceRemove(void *dev, void *point)
{
    DeviceOnlineHead_t *head = dev;

    list_trans(&head->cmdList, onlineDeviceCmdRemove, null);
    list_delete(&head->cmdList);
    
    return 1;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t onlineDeviceCmdAdd(void *load, void *changePoint)
{
    u16_t crc;
    ListHandler_t *head = changePoint;
    DeviceCmd_t *cmd = (DeviceCmd_t*)load;
    
    CmdBuf_t *cmdBuf = list_find(head, onlineDeviceCmdCmp, cmd->cmd.payload);
    r_memcpy(&crc, &cmd->ack.payload[cmd->ack.lenght-2], 2);
    
    if (cmdBuf == null)
    {
		
        cmdBuf = list_nodeApply(sizeof(CmdBuf_t));
        list_bottomInsert(head, cmdBuf);
        cmdBuf->state = 0;
        cmdBuf->fun = cmd->cmd.payload[1];
        cmdBuf->startAddr = (cmd->cmd.payload[2]<<8) | cmd->cmd.payload[3];
        cmdBuf->endAddr = cmdBuf->startAddr + cmd->cmd.payload[5] - 1;
        cmdBuf->crc = crc;
        cmdBuf->buf.size = cmd->ack.lenght > 5 ? (cmd->ack.lenght - 5) : 0;
        cmdBuf->buf.lenght = cmdBuf->buf.size;
        cmdBuf->buf.payload = memory_apply(cmdBuf->buf.size);
        r_memcpy(cmdBuf->buf.payload, &cmd->ack.payload[3], cmdBuf->buf.size);
    }
    
    if (crc != cmdBuf->crc && cmd->ack.lenght > 5)
    {
        int dataLenght = cmd->ack.lenght - 5;

        if (dataLenght > cmdBuf->buf.size)
        {
            memory_release(cmdBuf->buf.payload);
            cmdBuf->buf.size = dataLenght;
            cmdBuf->buf.lenght = cmdBuf->buf.size;
            cmdBuf->buf.payload = memory_apply(cmdBuf->buf.size);
        }
        cmdBuf->buf.lenght = dataLenght;
        r_memcpy(cmdBuf->buf.payload, &cmd->ack.payload[3], dataLenght);
        cmdBuf->state = 0;
        cmdBuf->crc = crc;
    }
    
    // cmdBuf->buf.lenght = cmd->ack.lenght - 5;
    // cmdBuf->buf.payload =  &cmd->ack.payload[3];
    
    return 1;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t onlineDeviceCmdRemove(void *load, void *changePoint)
{
    CmdBuf_t *cmdBuf = load;

     memory_release(cmdBuf->buf.payload);

    return 1;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t onlineDeviceCmp(void *src, void *dest)
{
    Device_t *dev = dest;
    DeviceOnlineHead_t *head = src;
    DeviceExplain_t *exp = dev->explain;
    
    if (exp->addr == head->addr && exp->code == head->code)
    {
        return 0;
    }
    
    return 1;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t onlineDeviceCmdCmp(void *src, void *dest)
{
	u8_t *para = (u8_t*)dest;
    u16_t srcAddr = ((para[2]<<8)|para[3]);
    CmdBuf_t *cmd = (CmdBuf_t*)src;
    
    if (cmd->startAddr == srcAddr && para[1] == cmd->fun)
    {
        return 0;
    }
    
    return 1;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t cmdTabPrintf(void *load, void *changeData)
{
	DeviceCmd_t *cmd;

    cmd = (DeviceCmd_t*)load;
    
    if (cmd->ack.lenght > 0)
    {
        char *buf = memory_apply(cmd->ack.lenght*2+1);
        Swap_hexChar(buf, cmd->ack.payload, cmd->ack.lenght, ' ');
        APP_DEBUG(buf);
        memory_release(buf);
        APP_DEBUG("\r\n");
    }
    
    return 1;
}

/******************************************************************************/

