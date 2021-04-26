/******************************************************************************           
* name:             enter.c        
* introduce:        user app enter
* author:           Luee                                     
******************************************************************************/ 
//sdk
#include <fibo_opencpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lib

//app
#include "Debug.h"
#include "hard_watchdog.h"
#include "appTask.h"
#include "ble_task.h"




static void prvThreadEntry(void *param)
{
    

	while(1)
	{
        for (int n = 0; n < 10; n++){
            OSI_LOGI(0, "hello world %d", n);
            fibo_taskSleep(500);
            Watchdog_feed();
            fibo_watchdog_feed();
        }
	}

    fibo_thread_delete();
}



void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}


//============ble===============================================================

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

void * appimg_enter(void *param) {
  UINT32 net_thread_id = 0;
  UINT32 app_thread_id = 0;
  UINT32 dev_thread_id = 0;
  UINT32 eyb_thread_id = 0;
  UINT32 ali_thread_id = 0;
  UINT32 fota_thread_id = 0;
  UINT32 upd_thread_id = 0;
  UINT32 com_thread_id = 0;
  UINT32 anti_thread_id=0;


  OSI_LOGI(0, "application image enter");
  prvInvokeGlobalCtors();

  Debug_init(); // 上电配置DEBUG串口
  fibo_taskSleep(1000);

  u8_t bootcase = fibo_getbootcause();
  switch (bootcase) {
    case 0:
      APP_PRINT("Booting from Soft reboot!!\r\n");
      break;
    case 1:
      APP_PRINT("Booting from RST reboot!!\r\n");
      break;
    case 2:
      APP_PRINT("Booting from Power on!!\r\n");
      break;
    case 3:
      APP_PRINT("Booting from USB plug!!\r\n");
      break;
    default:
      APP_PRINT("Booting from unkonw mode!!\r\n");
      break;
  }

  APP_PRINT("Version: %s\r\n", FWVERSION);
  APP_PRINT("Time: %s\r\n", MAKE_TIME);
  //fibo_set_app_version(FWVERSION);

  INT8 *hardware_version = NULL;
  INT8 *software_version = NULL;

  fibo_at_send((UINT8 *)"AT+MSTART=0,0\r\n", r_strlen("AT+MSTART=0,0\r\n"));  // 关闭Start message notification

  hardware_version   = fibo_get_hw_verno();  // 获取当前的硬件版本(客户定制)
  software_version   = fibo_get_sw_verno();  // 获取当前的软件版本(客户定制)
  APP_PRINT("hardware_version %s\r\n",hardware_version);
  APP_PRINT("SDK version %s\r\n",software_version);
  fibo_setSleepMode(0);   // Disable sleep mode.  

  INT32 enret = fibo_watchdog_enable(120);  // 120秒=2分钟 无任何语句执行则重启  
  if(0 == enret) {
    APP_DEBUG("ninside watchdog enable success\r\n"); 
  }
  if(enret < 0) {
    APP_DEBUG("ninside watchdog enable fail\r\n"); 
  }


  //fibo_watchdog_disable();

  Watchdog_init();

  //fibo_thread_create(prvThreadEntry, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
	// return (void *)&user_callback;
//BLE
   //if(g_fibo_ble_queue == 0)
   // {
	//	g_fibo_ble_queue = fibo_queue_create(20, sizeof(int));
	//}

    fibo_thread_create(prvThreadEntry, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
	fibo_thread_create(ble_task, "ble_task", 1024*4, NULL, OSI_PRIORITY_NORMAL);
	//fibo_thread_create(fibo_ble_task, "fibo_ble_task", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    //return (void *)&user_callback;

  return 0;

}

