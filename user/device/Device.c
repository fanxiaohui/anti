/**************Copyright(C)闁跨喐鏋婚幏锟�2015-2026闁跨喐鏋婚幏绋磆enzhen Eybond Co., Ltd.***************
  *@brief   : Device.c 闁跨喎鈧喎顦柨鐔告灮閹风兘鏁撻幑椋庢閹风兘鏁撻弬銈嗗闁跨噦鎷�
  *@notes   : 2017.12.05 CGQ 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�   
*******************************************************************************/
#include "Device.h"
#include "List.h"
#include "DeviceIO.h"
#include "appTask.h"
#include "ql_system.h"
#include "ql_stdlib.h"
#include "memory.h"
#include "ql_timer.h"
#include "debug.h"
#include "protocol.h"
#include "SysPara.h"
#include "Watchdog.h"
#include "log.h"
#include "selfUpdate.h"
#include "deviceUpdate.h"
#include "UpdateTask.h"
#include "Ql_Gpio.h"
#include "Sineng.h"
#include "clock.h"
#include "CommonServer.h"
#include "Swap.h"

#define DEVICE_LOCK     (0x5A)
#define DEVICE_UNLOCK   (0x00)


ListHandler_t DeviceList;  //闁跨喎鈧喎顦柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�

static DeviceCmd_t *currentCmd;     //闁跨喐鏋婚幏宄板闁跨喎鈧喎顦幐鍥晸閺傘倖瀚�
static u8_t currentStep;            //闁跨喐鏋婚幏鐑芥晸閸欘偉顕滈幏鐑芥晸閺傘倖瀚�
static u16_t DeviceOvertime;		//device overtime cnt
static int watiTime;

static u8_t device_cmp(void *src, void *dest);
static void callback(DeviceAck_e ack);
static void deviceCmdSend(void);
static void deviceLEDInit(void);
static void deviceLEDOn(void);
static void deviceLEDOff(void);


/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void proc_device_task(s32 taskId)
{    
    ST_MSG msg;
	int deviceResetCnt;
	int ret;

	deviceResetCnt = 0;
    currentStep = 0;
	DeviceOvertime = 0;
	watiTime = 5;		//wait 5sec start get device date
	list_init(&DeviceList);
	
	APP_DEBUG("Devce command task run.\r\n");
	deviceLEDInit();
	DeviceIO_init(null);
	Protocol_init();
	
	while (TRUE)
	{	
		Ql_OS_GetMessage(&msg);
		switch(msg.message)
		{ 
			case DEVICE_RESTART_ID:
				if (0 != runTimeCheck(4, 19))
				{
					deviceResetCnt = 0;
					DeviceOvertime = 0;
					watiTime = 10;
					break;
				}
				else if (++deviceResetCnt > 12)
				{
					log_save("Device no command ack reboot!!\r\n");
					Watchdog_stop();
					break;
				}
				else
				{
					log_save("Device modular reset!!\r\n");
					msg.param1 = DEVICE_MONITOR_NUM;
				}
				DeviceOvertime = 0;
			case SYS_PARA_CHANGE:  	//system parameter change 
				if (msg.param1 == DEVICE_MONITOR_NUM || msg.param1 == DEVICE_PROTOCOL)
				{
					DeviceIO_reset();
					Protocol_clean();
					Device_clear();
					memory_trans(Debug_output);
					CommonServer_DeviceInit();
                    Protocol_init();
                    deviceLEDOn();
					
				}
				watiTime = 10;
				break;
			case USER_TIMER_ID:
				if ((watiTime&0x8000) == 0x8000 || --watiTime > 0)
				{
					break;
				}
            case DEVICE_CMD_ID:
				watiTime = 0x8000;
				deviceCmdSend();
				break;
			case DEVICE_UPDATE_READY_ID: //Device update
				Update_ready();
				break;
			case DEVICE_PV_SCAN_ID:    //Device PV scan 
				PV_Scan();
				break;
			case DEVICE_PV_GET_ID:   //Device PV Data Get
				PV_dataGet();
				break;
			default:
				break;
		} 
	}
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t deviceClear(void *payload, void *point)
{
	DeviceCmd_clear((Device_t *)payload);
    return 1;
}
void Device_clear(void)
{
	list_trans(&DeviceList, deviceClear, null);
    list_delete(&DeviceList);
	
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t deviceCmdClear(void *payload, void *point)
{
	DeviceCmd_t * cmd = (DeviceCmd_t*)payload;
	
	memory_release(cmd->ack.payload);
	memory_release(cmd->cmd.payload);
    return 1;
}

void DeviceCmd_clear(Device_t *dev)
{
    list_trans(&dev->cmdList, deviceCmdClear, null);
    list_delete(&dev->cmdList);
	memory_release(dev->explain);
	currentStep = 0;
}     

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void Device_add(Device_t *device)
{
    list_bottomInsert(&DeviceList, device);
    device->lock = DEVICE_UNLOCK;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void Device_inset(Device_t *device)
{
    list_topInsert(&DeviceList, device);
    device->lock = DEVICE_UNLOCK;
	currentStep = 0;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void Device_remove(Device_t *device)
{
    Device_t *dev = list_find(&DeviceList, device_cmp,device);
    
    if (dev != null)
    {
        DeviceCmd_clear(dev);
        list_nodeDelete(&DeviceList, dev);
    }
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t deviceTypeRemove(void *payload, void *point)
{
	Device_t *dev = (Device_t *)payload;

	if (dev->type == *(u16_t*)point)
	{
		DeviceCmd_clear(dev);
        list_nodeDelete(&DeviceList, dev);
	}
	return 1;
}
void Device_removeType(u16_t type)
{
    list_trans(&DeviceList, deviceTypeRemove, &type);
}

/*******************************************************************************
  * @brief  device_cmp
  * @note   None
  * @param  None
  * @retval 
*******************************************************************************/
static u8_t device_cmp(void *src, void *dest)
{
    Device_t *pSrc = (Device_t*)src;
    Device_t *pDest = (Device_t*)dest;
    
    return  Ql_memcmp(&pSrc->callBack, &pDest->callBack, (int)(pSrc + 1) - (int)(&pSrc->callBack));
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
static void deviceCmdSend(void)
{
	static DeviceInfo_t deviceInfo;
	static Device_t *currentDevice;     //闁跨喐鏋婚幏宄板闁跨喎鈧喎顦�
	
    switch (currentStep)
    {
		case 0:
			currentDevice = null;
			currentCmd = null;
			deviceInfo.callback = callback;
        case 1: 
            currentDevice = list_nextData(&DeviceList, currentDevice);
			if (currentDevice == null || currentDevice->lock == DEVICE_LOCK)
			{
				if (++DeviceOvertime > 360)
				{
					log_save("Device 5 min no command reboot!!\r\n");
					Ql_OS_SendMessage(DEVICE_TASK, DEVICE_RESTART_ID, 0, 0);
				}
				watiTime = 2;
			}
			else if (DeviceIO_lockState() == null)
			{
				currentStep = 3;
				currentCmd = null;
				if (currentDevice->cfg != null)
				{
                	DeviceIO_init(currentDevice->cfg);
					watiTime = 1;
				}
                else if (DeviceIO_cfgGet() == null)
                {
                    log_save("Uart No Init!!");
                    currentStep = 1;
                }
			}
			else
			{
				log_save("Device output is lock!!\r\n");
				watiTime = 10;
			}
			break;
        case 2:
            currentDevice->lock = DEVICE_LOCK;
            if (currentDevice->callBack == null
				|| currentDevice->callBack(currentDevice) == 0)
			{
				currentStep = 1;
			}
			else
			{
				currentStep  = 3; //閻犱焦鍎抽ˇ顒勫极閻楀牆绁﹂柛鎰У椤愬ジ寮寸€涙ɑ鐓€闁挎稑鑻·鍐疀椤愶絾娈堕柟璇″枙缁楀倻鐥崸妞诲亾閻斿嘲顔� CGQ 2019.04.30
			}
						
            break;
        case 3:
            currentCmd = list_nextData(&currentDevice->cmdList, currentCmd);
            if (currentCmd == null)
            {
                currentStep--;
                break;
            } 
			currentStep++;
			watiTime = 0;
			break;
        case 4:
            if (null != currentCmd)
            {
				DeviceAck_e e;
                deviceInfo.waitTime = currentCmd->waitTime;
                deviceInfo.buf = &currentCmd->ack;
                if (deviceInfo.buf->size == 0)
                {
                    deviceInfo.buf->size = DEVICE_ACK_SIZE;
                    deviceInfo.buf->payload = memory_apply(DEVICE_ACK_SIZE);
					currentCmd->ack.size = deviceInfo.buf->size;
					currentCmd->ack.payload = deviceInfo.buf->payload;
                }
				
                e = DeviceIO_write(&deviceInfo, currentCmd->cmd.payload, currentCmd->cmd.lenght);
				watiTime = 10;
				if (DEVICE_ACK_FINISH != e)
                {
					//APP_DEBUG("Device command send fail %d.\r\n", e);
                    log_saveAbnormal("Device command send fail ", e);
                    if (e == DEVICE_ACK_HARD_FAULT)
                    {
                        DeviceIO_reset();
                    }
                    currentStep = 1;
				}

				if (++DeviceOvertime > 180)
				{
					Ql_OS_SendMessage(DEVICE_TASK, DEVICE_RESTART_ID, 0, 0);
				}
            }
            break;
        default:
            currentStep = 0;
            break;
    }
	if (watiTime == 0x8000)
	{
		Ql_OS_SendMessage(DEVICE_TASK, DEVICE_CMD_ID, 0, 0);
	}
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void callback(DeviceAck_e ack)
{
	if (ack == DEVICE_ACK_OVERTIME)
	{
		deviceLEDOff();
		#if TEST_RF
		if (++DeviceOvertime > 180)
		{
			Ql_OS_SendMessage(DEVICE_TASK, DEVICE_RESTART_ID, 0, 0);
			return;
		}
		#endif
		if (currentCmd->state != 0)
		{
			currentStep--;
		}
	}
	else
	{
		currentStep--;
		DeviceOvertime = 0;
		deviceLEDOn();
	}
    currentCmd->state = ack;
	Ql_OS_SendMessage(DEVICE_TASK, DEVICE_CMD_ID, 0, 0);
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void deviceLEDInit(void)
{
	Ql_GPIO_Init(DEVICE_LED, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLUP);
	deviceLEDOff();
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void deviceLEDOn(void)
{
	Ql_GPIO_SetLevel(DEVICE_LED, PINLEVEL_LOW);
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static void deviceLEDOff(void)
{
	Ql_GPIO_SetLevel(DEVICE_LED, PINLEVEL_HIGH);
}

/******************************************************************************/

