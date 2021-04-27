
/******************************************************************************           
* name:                       
* introduce:        头文件
* author:           Luee                                     
******************************************************************************/ 
#ifndef __BLE_TASK_
#define __BLE_TASK_

#define MSG_ID_BLE_START        0x9000

#define  BLE_DEBUG_MSG_ID          MSG_ID_BLE_START+0xFA8  // DEBUG串口输入

void ble_task(void *param);

#endif /* __BLE_TASK_ */