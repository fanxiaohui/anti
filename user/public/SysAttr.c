/**************Copyright(C)��2015-2026��QIYI Temp    Co., Ltd.******************
  *@brief   : SysAttr.c 
  *@notes   : 2017.12.21 CGQ ����   
  *@notes   : 防逆流相关逻辑
*******************************************************************************/
#include "Ql_system.h"
#include "r_stdlib.h"
#include "Swap.h"
#include "Ril_network.h"
#include "Ril_system.h"
#include "ModbusDevice.h"
#include "DeviceIO.h"
#include "Clock.h"
#include "memory.h"
#include "typedef.h"
#include "list.h"
#include "SysPara.h"
#include "Watchdog.h"
#include "net.h"
#include "device.h"
#include "Ql_system.h"
#include "log.h"
#include "Eybond.h"
#include "CommonServer.h"
#include "Protocol.h"
#include "Debug.h"

/*******************************************************************************
  * @brief	
  * @param	None
  * @retval None
*******************************************************************************/
int Attr_onlineCnt(Buffer_t *buf)
{ 
	
	buf->size = 8;
	buf->payload = memory_apply(buf->size);
	buf->lenght = Swap_numChar((char*)buf->payload, onlineDeviceList.count);
	
	return 0;
}

/*******************************************************************************
  * @brief	
  * @param	None
  * @retval None
*******************************************************************************/	
int Attr_timeGet(Buffer_t *buf)
{
	int offset;
	int timeZone;
	int day;
	int hour;
	Clock_t clock;
	
	Clock_timeZone(&clock);
	buf->size = 40;
	buf->payload = memory_apply(buf->size);
	
    offset = Swap_numChar(&buf->payload[offset], clock.year);
    buf->payload[offset++] = '-';
    offset += Swap_numChar(&buf->payload[offset], clock.month);
    buf->payload[offset++] = '-';
    offset += Swap_numChar(&buf->payload[offset], clock.day);
    buf->payload[offset++] = ' ';
    offset += Swap_numChar(&buf->payload[offset], clock.hour);
    buf->payload[offset++] = ':';
    offset += Swap_numChar(&buf->payload[offset], clock.min);
    buf->payload[offset++] = ':';
    offset += Swap_numChar(&buf->payload[offset], clock.secs);
	
	buf->lenght = offset;
	
	return 0;
}
		
/*******************************************************************************
  * @brief	
  * @param	None
  * @retval None
*******************************************************************************/
int Attr_option(Buffer_t *buf)
{
	if (buf->lenght > 0 && buf->payload != null)
	{
		switch (buf->payload[0])
		{
			case '1': //soft reboot
				log_save("Command soft reboot!!\r\n");
				Watchdog_stop();
				break;
			case '2': // reset to default val
			    log_save("Command reset to default value!!\r\n");
			    SysPara_default();
				HistoryDataClean();
				CommonServerData_clean();
				break;
			case '3': // reset device Modular
			    log_save("Command reset device module!!\r\n");
				Ql_OS_SendMessage(DEVICE_TASK, DEVICE_RESTART_ID, 0, 0);
				break;
			case '4': // reset GPRS Modular
				log_save("Command reset GPRS module!!\r\n");
			    Ql_OS_SendMessage(NET_TASK, NET_RESTART_ID, 0, 0);
				break;
			case '9':
			    SysParaErase();
		}
	}
	
	return 0;
}

/*******************************************************************************
  * @brief	
  * @param	None
  * @retval None
*******************************************************************************/
int Attr_Uart_Config_Get(Buffer_t *buf)
{
	ST_UARTDCB cfg;
	int ret;

	ret = Ql_UART_GetDCBConfig(DEVICE_IO_PORT, &cfg);
	u16_t offset = 0;
	if (ret == 0)
	{
		buf->size = 13;												//最长13
		buf->payload = memory_apply(buf->size);						//设置串口1000000-8-1-0
		offset = Swap_numChar((char*)buf->payload, cfg.baudrate);	//波特率
		buf->payload[offset++] = '-';
		offset += Swap_numChar(&buf->payload[offset], cfg.dataBits);//数据位
		buf->payload[offset++] = '-';
		offset += Swap_numChar(&buf->payload[offset], cfg.stopBits);//停止位
		buf->payload[offset++] = '-';
		offset += Swap_numChar(&buf->payload[offset], cfg.parity);	//校验位
		buf->lenght = offset;	
	}
	else 
	{
		buf->lenght = 0;
		buf->payload = null;
	}

	return ret;
}

/*******************************************************************************
  * @brief	
  * @param	None
  * @retval None
*******************************************************************************/
int Attr_Uart_Config_Set(Buffer_t *buf)
{
	int ret = 0;
	ListHandler_t uartCfgStr;
	r_strsplit(&uartCfgStr, (char*)buf->payload, '-');
	int tab[4];
	int i = 0;
	Node_t *node;
	node = uartCfgStr.node;
	do {
		tab[i++] = Swap_charNum((char*)*(int*)node->payload);
		APP_DEBUG("\r\nBand %d, %d.\r\n", i, tab[i-1]);
		node = node->next;			
	} while(node != uartCfgStr.node && i < 4);
	if (	(tab[0] >  2000 && tab[0] < 1000000) 	//波特率
		&&  (tab[1] >  4    && tab[1] < 9)			//数据位
		&&  (tab[2] >  0    && tab[2] < 4)  		//停止位
		&&  (tab[3] >= 0    && tab[3] < 5)			//校验位
		)
	{
		ProtocolAttr_t 	attr;
        ST_UARTDCB 		uart;
        char str[64]  = {0};
        int offset 	  =  0;
        Buffer_t buf;
        
        attr.code 		= 0x02FF;
        attr.startAddr 	= 1;
        attr.endAddr 	= 1;
        attr.cfg 		= null;

        uart.baudrate 	= tab[0];
        uart.dataBits 	= tab[1];
        uart.stopBits 	= tab[2];
        uart.parity 	= tab[3];
        uart.flowCtrl 	= FC_NONE;

        protocolAttrGet(DEVICE_PROTOCOL, &attr);//14号参数

        if (attr.cfg != null)
        {
            memory_release(attr.cfg);
        }
        attr.cfg = &uart;

        Swap_numHexChar(str, attr.code);
        offset = r_stradd(str, ",");
        Swap_numChar(&str[offset], attr.startAddr);
        offset = r_stradd(str, ",");
        Swap_numChar(&str[offset], attr.endAddr);
        offset = r_stradd(str, ",#");
        Swap_numChar(&str[offset], attr.cfg->baudrate);
        offset = r_stradd(str, "-");
        Swap_numChar(&str[offset], attr.cfg->dataBits);
        offset = r_stradd(str, "-");
        Swap_numChar(&str[offset], attr.cfg->stopBits);
        offset = r_stradd(str, "-");
        Swap_numChar(&str[offset], attr.cfg->parity);
        buf.lenght = r_stradd(str, "#");
        buf.payload = (u8_t*)str;

        ret = SysPara_Set(DEVICE_PROTOCOL, &buf);
	}
	else if (tab[0]>2000 && tab[0]<1000000) 	//波特率
	{
		ProtocolAttr_t attr;
        ST_UARTDCB uart;
        char str[64]  = {0};
        int offset = 0;
        Buffer_t buf;
        

        attr.code = 0x02FF;
        attr.startAddr = 1;
        attr.endAddr = 1;
        attr.cfg = null;

        uart.baudrate = tab[0];
        uart.dataBits = DB_8BIT;
        uart.stopBits = SB_ONE;
        uart.parity = PB_NONE;
        uart.flowCtrl = FC_NONE;

        protocolAttrGet(DEVICE_PROTOCOL, &attr);

        if (attr.cfg != null)
        {
            memory_release(attr.cfg);
        }
        attr.cfg = &uart;

        Swap_numHexChar(str, attr.code);
        offset = r_stradd(str, ",");
        Swap_numChar(&str[offset], attr.startAddr);
        offset = r_stradd(str, ",");
        Swap_numChar(&str[offset], attr.endAddr);
        offset = r_stradd(str, ",#");
        Swap_numChar(&str[offset], attr.cfg->baudrate);
        offset = r_stradd(str, "-");
        Swap_numChar(&str[offset], attr.cfg->dataBits);
        offset = r_stradd(str, "-");
        Swap_numChar(&str[offset], attr.cfg->stopBits);
        offset = r_stradd(str, "-");
        Swap_numChar(&str[offset], attr.cfg->parity);
        buf.lenght = r_stradd(str, "#");
        buf.payload = (u8_t*)str;

        ret = SysPara_Set(DEVICE_PROTOCOL, &buf);
	}
	else 
	{
		ret = -1;
	}
	
	return ret;
}


/*******************************************************************************
  * @brief	
  * @param	None
  * @retval None
*******************************************************************************/
int Attr_logGet(Buffer_t *buf)
{
	int ret;
    Buffer_t getBuf;
	
	buf->size = 4096;
	buf->payload = memory_apply(buf->size);
	buf->lenght = 0;

	while (buf->payload != null)
	{
        getBuf.size = buf->size - buf->lenght;
        getBuf.payload = &buf->payload[buf->lenght];
		ret = log_get(&getBuf);
        
        if (ret > 0)
        {
            buf->lenght += ret;
            if (buf->lenght > 900)
            {
                break;
            }
			else
			{
				buf->lenght--;
			}
        }
        else
        {
            break;
        }
	}

	return 0;
}

/*******************************************************************************
  * @brief	
  * @param	None
  * @retval None
*******************************************************************************/
int Attr_logSet(Buffer_t *buf)
{
	log_clear();
	return 0;
}


/*******************************************************************************
  * @brief	
  * @param	None
  * @retval None
*******************************************************************************/
static u8_t cmdCode(void *load, void *dest)
{
	int offset;
	DeviceCmd_t *cmd = (DeviceCmd_t *)load;
	char *str = (char*)dest;

	offset = r_strlen(str);
	Swap_numChar(&str[offset], cmd->state);
	offset = r_stradd(str, ", ");
	Swap_hexChar(&str[offset], cmd->cmd.payload, cmd->cmd.lenght, 0);
	if (cmd->ack.payload != null && cmd->ack.size != cmd->ack.lenght)
	{
		offset = r_stradd(str, "<");
		Swap_hexChar(&str[offset], cmd->ack.payload, cmd->ack.lenght, 0);
	}
	offset = r_stradd(str, ";");
	return 1;
}
static u8_t addrCode(void *load, void *dest)
{
	int offset;
	char *str = (char*)dest;
	Device_t *device = (Device_t *)load;

	if (device->type == DEVICE_MOBUS)
	{
		DeviceExplain_t *exp = device->explain;
		
		offset = r_stradd(str, "[");
		Swap_numChar(&str[offset], exp->addr);
		offset = r_stradd(str, ", ");
		Swap_numHexChar(&str[offset], exp->code);
		offset = r_stradd(str, "]");
	}
	else if (device->type == DEVICE_GRID)
	{
		r_stradd(str, "[State Grid]");
	}
	else if (device->type == DEVICE_ARTI)
	{
		r_stradd(str, "[Adj power]");
	}
	else
	{
		r_stradd(str, "[TRANS]");
	}
	list_trans(&device->cmdList, cmdCode, str);
	offset = r_strlen(str);
	
	return (offset > 3072 ? 0 : 1);
}
int Attr_onlineType(Buffer_t *buf)
{
	buf->size = 4096;
	buf->payload = memory_apply(buf->size);
	r_strcpy((char *)buf->payload, "");
	
	list_trans(&DeviceList, addrCode, buf->payload);

	buf->lenght = r_strlen((const char *)buf->payload);
	
	return 0;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/	
int Attr_CSQ(Buffer_t *buf)
{
	int ret;
	u32 rssi;
	u32 ber;
	
	ret = RIL_NW_GetSignalQuality(&rssi, &ber);
	if (ret == 0)
	{
		buf->size = 8;
		buf->payload = memory_apply(buf->size);
		buf->lenght = Swap_numChar((char*)buf->payload, rssi);
	}
	else
	{
		buf->payload = null;
		buf->lenght = 0;
	}
	
	return ret;
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/	
int Attr_CPUID(Buffer_t *buf)
{
	int ret;
	char str[64];
	
	/*
	u8_t buf[12];
	
	Ql_GetUID(buf, sizeof(buf));
	Swap_hexChar(str, buf, sizeof(buf), 0);//*/

	ret = RIL_GetIMEI(str);

	if (ret != 0)
	{
		buf->lenght = 0;
		buf->payload = null;
	}
	else
	{
		char *p = str;
		buf->size = 64;
		buf->payload = memory_apply(buf->size);
		while (*p != '\0')
		{
			if (*p <= '9' && *p >= '0')
			{
				buf->payload[ret++] = *p;
			}
			
			p++;
		}
		
		buf->lenght = 15;
		buf->payload[15] = '\0';
	}
	return ret;
}


/******************************************************************************/

