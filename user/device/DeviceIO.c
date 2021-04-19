/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : DeviceIO.c
 * @Author  : CGQ
 * @Date    : 2017-12-05
 * @Brief   : 
 ******************************************************************************/
#include "DeviceIO.h"
#include "ql_uart.h"
#include "ql_timer.h"
#include "memory.h"
#include "appTask.h"
#include "Device.h"
#include "Debug.h"
#include "r_stdlib.h"
#include "log.h"

static DeviceInfo_t *s_device;
static DeviceInfo_t *s_lockDevice;
static ST_UARTDCB *IOCfg = null;

static Buffer_t rcveBuf;

static void end(DeviceAck_e e);


static void UARTCallBack(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);
static void overtimeCallback(u32 timerId, void* param);

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void DeviceIO_init(ST_UARTDCB *cfg)
{    
	if (cfg == null)
	{
		s_device = null;
		s_lockDevice = null;
		IOCfg = null;
		rcveBuf.payload = null;
		Ql_Timer_Register(DEVICE_OVERTIME_ID, overtimeCallback, null);
		Ql_UART_Register(DEVICE_IO_PORT, UARTCallBack,  NULL);
		Ql_UART_Open(DEVICE_IO_PORT, 9600, FC_NONE);
	}
	else if (IOCfg == null || r_memcmp(IOCfg, cfg, sizeof(ST_UARTDCB)) != 0)
	{
		IOCfg = cfg;
		
		Ql_UART_Close(DEVICE_IO_PORT);
		//Ql_UART_Register(DEVICE_IO_PORT, UARTCallBack,  NULL);
		Ql_UART_OpenEx(DEVICE_IO_PORT, IOCfg);
	}
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void DeviceIO_reset(void)
{
	Ql_UART_Close(DEVICE_IO_PORT);
	s_lockDevice = null;
	IOCfg = null;
	memory_release(rcveBuf.payload);
	rcveBuf.payload = null;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
ST_UARTDCB *DeviceIO_cfgGet(void)
{
	return IOCfg;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
DeviceAck_e DeviceIO_write(DeviceInfo_t *hard, u8_t *pData, mcu_t lenght)
{
	DeviceAck_e result = DEVICE_ACK_FINISH;
	
	if (hard == null || hard->buf->payload == null)
	{
		result = DEVICE_ACK_PRAR_ERR;
	}
	else if (s_device == null || hard == s_device)
	{
		if (lenght == 0 || pData == null)
		{
			result = DEVICE_ACK_PRAR_ERR;
		}
		else//*/
		{  
            int i = Ql_UART_Write(DEVICE_IO_PORT, pData, lenght);

            if (i ==  lenght)
            {
                s_device = hard;
                s_device->buf->lenght = 0;
                Ql_Timer_Start(DEVICE_OVERTIME_ID, hard->waitTime, TRUE);
                result = DEVICE_ACK_FINISH;
            }
            else
            {
                log_saveAbnormal("Uart send fail !!", i);
                result = DEVICE_ACK_HARD_FAULT;
            }
            
		}
	}
	else if (s_lockDevice != null)
	{
		result = DEVICE_ACK_LOCK;
	}
	else
	{
		result = DEVICE_ACK_BUSY;
	}

	return result;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
static void wrtie(Buffer_t *buf)
{
	Uart_write(buf->payload, buf->lenght);
	memory_release(buf->payload);
	buf->payload = null;
}
void Uart_write(u8_t *data, u16_t len)
{
	if (len > 0 && data != null)
	{
		Ql_UART_Write(DEVICE_IO_PORT, data, len);
	}
}
/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void  DeviceIO_lock(DeviceInfo_t *hard)
{
	s_lockDevice = hard;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
DeviceInfo_t * DeviceIO_lockState(void)
{
	return s_lockDevice;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void  DeviceIO_unlock(void)
{
	s_lockDevice = null;
	s_device = null;
}

/*******************************************************************************
  * @brief  device opt end
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
static void end(DeviceAck_e e)
{
	Ql_Timer_Stop(DEVICE_OVERTIME_ID);
	if (null != s_device && s_device->callback != null)
	{
		s_device->callback(e);
	}
	else if (e != DEVICE_ACK_OVERTIME)
	{  
		Ql_OS_SendMessage(APP_TASK, DEVICE_IO_ID, (u32)&rcveBuf, (u32)((void*)wrtie));
	}
	s_device = s_lockDevice;
}

/*******************************************************************************
  * @brief  overtime callback
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
static void overtimeCallback(u32 timerId, void* param)
{
	end(DEVICE_ACK_OVERTIME);
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
static void UARTCallBack(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
	if (port == DEVICE_IO_PORT)
	{
		s32 rdLen = 0;
		Buffer_t *buf;

		if (s_device == null || s_device->buf->payload == null)
		{
			if (rcveBuf.payload != null)
			{
				memory_release(rcveBuf.payload);
			}
			
			rcveBuf.payload = memory_apply(1024);
			rcveBuf.size = 1024;
			buf = &rcveBuf;
		}
		else
		{
			buf = s_device->buf;
		}
		
		//buf->lenght = 0;
		
	    //while (1)
	    {
	     	/*   rdLen = Ql_UART_Read(port, &buf->payload[buf->lenght],  buf->size - buf->lenght);
	        if (rdLen <= 0)  // All data is read out, or Serial Port Error!
	        {
	            break;
	        }
	        buf->lenght += rdLen;*/
	        
	    }
		buf->lenght =  Ql_UART_Read(port, buf->payload,  buf->size);
		Ql_UART_ClrRxBuffer(DEVICE_IO_PORT);
		end(DEVICE_ACK_FINISH);
	}
}

/*********************************FILE END*************************************/

