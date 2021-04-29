
/******************************************************************************           
* name:                       
* introduce:        头文件
* author:           Luee                                     
******************************************************************************/ 
#ifndef __BLE_TASK_
#define __BLE_TASK_

#define MSG_ID_BLE_START        0x9000

#define  BLE_DEBUG_MSG_ID           MSG_ID_BLE_START+0xFA8   // DEBUG串口输入
#define  BLE_DEBUG_INPUT_PROCESS    MSG_ID_BLE_START+0xFA9   // DEBUG INPUT DATA handle

extern u8 ble_rec_delay;

void ble_task(void *param);

#endif /* __BLE_TASK_ */