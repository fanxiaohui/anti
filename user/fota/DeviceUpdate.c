/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : DeviceUpdate.c
 * @Author  : CGQ
 * @Date    : 2017-12-14
 * @Brief   : 
 ******************************************************************************/
#include "DeviceUpdate.h"
#include "DeviceIO.h"
#include "device.h"
#include "memory.h"
#include "MODBUS.h"
#include "list.h"
#include "ql_system.h"
#include "UpdateTask.h"
#include "Device.h"
#include "XmodemTrans.h"
#include "r_stdlib.h"
#include "Debug.h"
#include "log.h"

static DeviceInfo_t updateDeviceHead;

ListHandler_t updateList = {0, 0, null};
ST_UARTDCB *defaultCfg = null;
ST_UARTDCB *updateCfg = null;
static u8 cnt = 0;
static Buffer_t buf;
static u8 updateState = 0xFF;

static u8_t readyAck(DeviceAck_e e);
static u8_t XmodemAck(DeviceAck_e e);

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
int Update_startDevice(File_t *file, DeviceType_t *type)
{
	u8 buf[4];
	DeviceCmd_t *cmd;
	
	buf[0] = 0x00;
	buf[1] = type->type;
	buf[2] = type->bandrate >> 8;
	buf[3] = type->bandrate;

	list_init(&updateList);
	Modbus_16SetCmd(&updateList, type->addr, 0x0438, 0x0439, buf);
	cmd = list_nextData(&updateList, null);
	if (cmd != null)
	{
		cnt = 0;
		updateState = 0;
		updateDeviceHead.buf = &cmd->ack;
		updateDeviceHead.callback = readyAck;
		updateDeviceHead.waitTime = 6000;
		defaultCfg = DeviceIO_cfgGet();
		updateCfg = memory_apply(sizeof(ST_UARTDCB));
		if (defaultCfg == null || updateCfg == null)
		{
			memory_release(cmd->ack.payload);
			memory_release(cmd->cmd.payload);
			list_delete(&updateList);
			memory_release(updateCfg);
			updateCfg = null;
		}
		else
		{
			r_memcpy(updateCfg, defaultCfg, sizeof(ST_UARTDCB));
			if (type->bandrate != 0)
			{
				updateCfg->baudrate = type->bandrate * 100;
			}
			DeviceIO_lock(&updateDeviceHead);
			Xmodem_start(file);
			memory_release(type);
			
			return 0;
		}
	}
	memory_release(file);
	memory_release(type);
	updateState = 0xFE;
	
	return -1;
	
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
u8 Update_state(void)
{
	if (updateState)
	{
		return updateState;
	}
	else
	{
		return Xmodem_progress();
	}
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void Update_end(void)
{
	DeviceCmd_t *cmd;

	cmd = list_nextData(&updateList, null);
	if (cmd != null)
	{
		memory_release(cmd->ack.payload);
		memory_release(cmd->cmd.payload);
		list_delete(&updateList);
	}
	
	memory_release(buf.payload);
	buf.payload = null;
	buf.lenght = 0;
	buf.size = 0;
	cnt = 0;
	
	DeviceIO_unlock();
	if (updateCfg != null)
	{
		DeviceIO_init(defaultCfg);
		memory_release(updateCfg);
		defaultCfg = null;
		updateCfg = null;
	}
	Xmodem_end();
	log_save("Device Update cancel!!\r\n");
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void Update_ready(void)
{
	readyAck(DEVICE_ACK_OVERTIME);
}
static u8_t readyAck(DeviceAck_e e)
{
	DeviceCmd_t *cmd;

	cmd = list_nextData(&updateList, null);
	if ((e == DEVICE_ACK_FINISH) 
		&& (cmd != null)
		&& (0 == Modbus_CmdCheck(&cmd->cmd, &cmd->ack))
		)
	{		
		cnt = 0;
		memory_release(cmd->ack.payload);
		memory_release(cmd->cmd.payload);
		list_delete(&updateList);
		buf.lenght = 0;
		buf.payload = null;
		updateDeviceHead.callback = XmodemAck;
		updateDeviceHead.waitTime = 10000;
		updateDeviceHead.buf = memory_apply(sizeof(Buffer_t));
		updateDeviceHead.buf->size = 512;
		updateDeviceHead.buf->lenght = 0;
		updateDeviceHead.buf->payload = memory_apply(updateDeviceHead.buf->size);
		DeviceIO_init(updateCfg);
		DeviceIO_lock(&updateDeviceHead);
		DeviceIO_write(&updateDeviceHead, buf.payload, buf.lenght);
		log_save("Device Update in X-modem.\r\n");
	}
	else if (cmd == null || cnt++ > 10)
	{
		Update_end();
		updateState = 0xFE;
	}
	else
	{
		int ret;
		log_saveAbnormal("Update Ack : ", e);
        if (e == DEVICE_ACK_FINISH)
        {
            log_saveBuf(&cmd->ack);
        }
		ret = DeviceIO_write(&updateDeviceHead, cmd->cmd.payload, cmd->cmd.lenght);
		if (ret != 0)
		{ 
			log_saveAbnormal("Device Update command fail: ", ret);
			Ql_OS_SendMessage(DEVICE_TASK, DEVICE_UPDATE_READY_ID, 0, 0);
		}
		else
		{
			APP_DEBUG("Device Update command send\r\n")
		}
	}

	return 0;
}

static u8_t XmodemAck(DeviceAck_e e)
{	
	if (e == DEVICE_ACK_FINISH)
	{
		int ret = Xmodem_ack(&buf, updateDeviceHead.buf);

		if (ret < 0)
		{
			updateState = 0xFE;
			log_save("Device update fail!!\r\n");
			goto END;
		}
		else if (ret == 1)
		{
			log_save("Device update Ok!!\r\n");
			updateState = 100;
			goto END;
		}
		cnt = 0;
	}
	else if (e == DEVICE_ACK_OVERTIME)
	{
		if (cnt++ > 10)
		{
			log_save("Device update wati overtime!!\r\n");
			updateState = 0xFE;
			goto END;
		}
	}
	
	DeviceIO_write(&updateDeviceHead, buf.payload, buf.lenght);
	return 0;
	
END:
	Update_end();

	return 0;
}
/*********************************FILE END*************************************/
