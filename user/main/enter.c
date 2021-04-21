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

static void prvInvokeGlobalCtors(void);

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
static void prvThreadEntry(void *param)
{
    OSI_LOGI(0, "application thread enter, param 0x%x", param);
    //srand(100);

    for (int n = 0; n < 80; n++)
    {
        OSI_LOGI(0, "hello world %d", n);
        APP_DEBUG("Luee test by debug print %d\r\n",n);
        fibo_taskSleep(1000);
    }

    char *pt = (char *)fibo_malloc(512);
    if (pt != NULL)
    {
        OSI_LOGI(0, "malloc address %u", (unsigned int)pt);
        fibo_free(pt);
    }

	//test_printf();
    fibo_thread_delete();
}

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
void * appimg_enter(void *param) {
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
  INT32 enret = fibo_watchdog_enable(60);  // 60秒=1分钟 无任何语句执行则重启      
  if(0 == enret) {
    APP_DEBUG("ninside watchdog enable success\r\n"); 
  }
  if(enret < 0) {
    APP_DEBUG("ninside watchdog enable fail\r\n"); 
  }

    fibo_thread_create(prvThreadEntry, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return 0;

}

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

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}
