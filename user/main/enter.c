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



//===========ble================================================================
static gatt_chara_def_short_t ble_mgr_chara = {{
	ATT_CHARA_PROP_READ | ATT_CHARA_PROP_WRITE,
	0,
	0,
	0x2a06&0xff,
	0x2a06>>8     	//GATT Characteristic and Object Type GATT特征和对象类型 
}};

static gatt_chara_def_short_t ble_mgr_chara1 = {{
	ATT_CHARA_PROP_READ | ATT_CHARA_PROP_WRITE,
	0,
	0,
	0x2aa6&0xff,
	0x2aa6>>8		//GATT Characteristic and Object Type GATT特征和对象类型 
}};

#define 		BLE_BUFFER_SIZE    256

//BLE调试
char 			BLE_WriteData[BLE_BUFFER_SIZE];
UINT16 			BLE_WriteLen;
char 			BLE_ReadData[BLE_BUFFER_SIZE];
UINT16 			BLE_ReadLen;
char 			BLE_PublicData[BLE_BUFFER_SIZE];
UINT16 			BLE_PublicLen;

UINT8 data_read_callback(void *param)
{
	memcpy(BLE_ReadData,BLE_PublicData,strlen(BLE_PublicData)); 
    APP_DEBUG("client phone rec:BLE_ReadData=%s,len=%d\r\n",BLE_ReadData,strlen(BLE_ReadData));
	return 0;
}

//手机AT命令接收
UINT8 data_write_callback(void *param)
{
	//将手机发过来的BLE_PublicData拷贝到BLE_WriteData
	memcpy(BLE_WriteData,BLE_PublicData,strlen(BLE_PublicData)); 
	memset(BLE_PublicData,0,sizeof(BLE_PublicData));
	APP_DEBUG("client phone send:BLE_WriteData=%s,len=%d\r\n",BLE_WriteData,strlen(BLE_WriteData));
	BLE_WriteLen = strlen(BLE_WriteData);
	char BLE_RecvData_Copy[BLE_BUFFER_SIZE]={0};
	if(BLE_WriteLen>4)
	{
		memcpy(BLE_RecvData_Copy,BLE_WriteData,BLE_WriteLen); 
		// uart_set_get_para(BLE_RecvData_Copy);	
		//AT_command_analysis(BLE_RecvData_Copy);//AT命令解析
	}
	memset(BLE_WriteData,0,sizeof(BLE_WriteData));
	return 0;
}

//BLE透传
char 			BLE_PASS_WriteData[BLE_BUFFER_SIZE];
UINT16 			BLE_PASS_WriteLen;
char 			BLE_PASS_ReadData[BLE_BUFFER_SIZE];
UINT16 			BLE_PASS_ReadLen;
char 			BLE_PASS_PublicData[BLE_BUFFER_SIZE];
UINT16 			BLE_PASS_PublicLen;

UINT8 data_pass_read_callback(void *param)
{
	memcpy(BLE_PASS_ReadData,BLE_PASS_PublicData,strlen(BLE_PASS_PublicData)); 
    APP_DEBUG("client phone rec:BLE_PASS_ReadData=%s,len=%d\r\n",BLE_PASS_ReadData,strlen(BLE_PASS_ReadData));
	return 0;
}

//手机BLE透传接收
UINT8 data_pass_write_callback(void *param)
{
	memcpy(BLE_PASS_WriteData,BLE_PASS_PublicData,strlen(BLE_PASS_PublicData)); 
	memset(BLE_PASS_PublicData,0,sizeof(BLE_PASS_PublicData));
	APP_DEBUG("client phone send:BLE_PASS_WriteData=%s,len=%d\r\n",BLE_PASS_WriteData,strlen(BLE_PASS_WriteData));
	BLE_PASS_WriteLen = strlen(BLE_PASS_WriteData);
	char BLE_PASS_RecvData_Copy[BLE_BUFFER_SIZE]={0};
	memcpy(BLE_PASS_RecvData_Copy,BLE_PASS_WriteData,BLE_PASS_WriteLen); 
/*
	uart_write(UART1, (UINT8 *)BLE_PASS_RecvData_Copy, BLE_PASS_WriteLen);
	UINT32 serial_waiting_timeout = 1500;
    serial_communication_information();//获取串口信息
    switch(uart1_newcfg.baud)
	{
		case 9600:								    //波特率9600
			serial_waiting_timeout = 2000;          //延时2S
			break;
		case 4800:									//波特率4800
			serial_waiting_timeout = 2500;          //延时2.5S
			break;
		case 2400:									//波特率2400
			serial_waiting_timeout = 3000;          //延时3S
            break;
        case 1200:                                  //波特率1200
            serial_waiting_timeout = 5000;          //延时5S            
            break;
		default:   
            serial_waiting_timeout = 1500;          //延时1.5S         
            break;
	}
    log_d("\r\nbaud is %d serial_waiting_timeout is %d\r\n",uart1_newcfg.baud,serial_waiting_timeout);
    fibo_taskSleep(serial_waiting_timeout);
    memcpy(BLE_PASS_PublicData,uart1_recv_data,uart1_recv_len);
	memset(BLE_PASS_WriteData,0,sizeof(BLE_PASS_WriteData));
*/
	watchdogns(1);
	memcpy(BLE_PASS_PublicData,BLE_PASS_RecvData_Copy,BLE_PASS_WriteLen);
	memset(BLE_PASS_WriteData,0,sizeof(BLE_PASS_WriteData));
	return 0;
}

uint8_t test_service[2] 		= {0x1811&0xff, 0x1811 >> 8};
char wifimgr_ccc_cfg[100] 		= {0};

//服务---》特性---》属性
//Bluetooth_Base_UUID 定义为 00000000-0000-1000-8000-00805F9B34FB 
// < gatt_element >:创建service及characteristic元素数组,见oc_ble_add_service_test .c及蓝牙协议； 
// 结构体里有7个变量 
// typedef struct
// {
//     UINT16                  length;		// 第一个是第五个指针变量数据的长度.
//     UINT16                  permisssion;	// 第二个参数表示当前service或characteristic的读写属性,可配置1可读,2可写.
//     gatt_uuid_t             uuid;		// 第三个参数配置UUID,service或characteristic的配置方法分别见demo.
//     UINT32                  fmt;			// 第四个参数配置该UUID的属性.
//     void                    *attValue;	// 第五个参数是具体的数据，如果是service则是service的UUID,见demo,如果是characteristic则是接收数据的buffer，收发数据共用此buffer 
//     gatt_value_change_cb_t  cb;			// 第六个参数是写数据callback，也就是客户端写数据，我们此接口会被调起来并输出数据 
//     gatt_value_read_cb_t    read_cb;		// 第七个参数是客户端读数据的callback ，客户端读数据时此callback会被执行起来； 
											// ATT_FMT_SHORT_UUI D ---应该是都要的，我们现在都是用的short ATT_FMT_GROUPED-- 定义 primary service 的时候加 ATT_FMT_FIXED_LEN GTH -- 目前除了primary service其他应该都要加ATT_FMT_WRITE_NOT IFY -- 如果下面的那个的callback不为空，且你需要被写的时候callback要call到，需要加上这个 <size>：gatt_element数组元素gatt_element_t的个数，见demo。
// } gatt_element_t;

gatt_element_t config_ble_service[]={//配置蓝牙服务
	{
		//creat service and fill UUID					//创建服务并填写UUID
	    sizeof(test_service),
	   	ATT_PM_READABLE,
	    {ATT_UUID_PRIMARY},								//主要服务
	    ATT_FMT_SHORT_UUID | ATT_FMT_GROUPED,
	    //ATT_FMT_GROUPED,
	    (void *)test_service,
	    NULL,
	    NULL
	},
	{
		//creat chara and fill permission 				创建角色并填写权限
	    sizeof(ble_mgr_chara),
		ATT_PM_READABLE | ATT_PM_WRITEABLE,				//可读可写
	    {ATT_UUID_CHAR},								//0x2803 GATT Declarations 声明 Characteristic 特性
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY | ATT_FMT_FIXED_LENGTH,		//16bit uuid fixed length of value 固定长度的值
	    (void *)&ble_mgr_chara,
		NULL,
		NULL
	},
	{
	    sizeof(BLE_PublicData),
		ATT_PM_READABLE | ATT_PM_WRITEABLE,				//可读可写
	    {0x2a06},										//UUID GATT特征和对象类型 警戒线
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY | ATT_FMT_FIXED_LENGTH, 		//16bit uuid  当客户端调用写入值时，将调用中继服务器回调函数
	    (void *)BLE_PublicData,
	    data_write_callback,							//写数据callback
	    data_read_callback								//读数据callback
	},
	{
	    sizeof(wifimgr_ccc_cfg),
	    ATT_PM_READABLE | ATT_PM_WRITEABLE,
	    {ATT_UUID_CLIENT},								//客户端特征配置
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY|ATT_FMT_FIXED_LENGTH,
	    (void *)wifimgr_ccc_cfg,
	    NULL,
	    NULL
	},
	{
	    sizeof(ble_mgr_chara1),							//创建角色并填写权限
		ATT_PM_READABLE,
	    {ATT_UUID_CHAR},								//特性
	    ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH,
	    (void *)&ble_mgr_chara1,
	    NULL,
	    NULL
	},
	{													//填写chara值
	    sizeof(BLE_PASS_PublicData),
		ATT_PM_READABLE | ATT_PM_WRITEABLE,				//可读可写
	    {0x2aa6},										//UUID GATT特征和对象类型 警戒线
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY | ATT_FMT_FIXED_LENGTH, 		//16bit uuid  当客户端调用写入值时，将调用中继服务器回调函数
	    (void *)BLE_PASS_PublicData,
	    data_pass_write_callback,						//写数据callback
	    data_pass_read_callback							//读数据callback
	},
	{													//des加密
	    sizeof(wifimgr_ccc_cfg),
	    ATT_PM_READABLE | ATT_PM_WRITEABLE,
	    {ATT_UUID_CLIENT},								//客户端特征配置
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY|ATT_FMT_FIXED_LENGTH,
	    (void *)wifimgr_ccc_cfg,
	    NULL,
	    NULL
	}
};


static void prvThreadEntry(void *param)
{
    fibo_taskSleep(1000);//不能删
    INT32 kgret = fibo_bt_onoff(1);//0:关闭蓝牙 1：打开蓝牙
	if(kgret >= 0){
		APP_DEBUG("\r\nkgret is %d bluetooth open success\r\n",kgret);
	}
	else{
		APP_DEBUG("\r\nkgret is %d bluetooth open fail\r\n",kgret);
	}

/*
	char    *PN_buf         	= NULL;//参数PN信息
	UINT16  PN_len        		= 64;  //参数BLE名称长度
	//将BLE蓝牙名称设置为数采器PN号
	for (int j = 0; j < number_of_array_elements; j++)
	{
		if(2 == PDT[j].num)
		{
			PN_buf = fibo_malloc(sizeof(char)*64);
			memset(PN_buf, 0, sizeof(char)*64);
			PDT[j].rFunc(&PDT[j],PN_buf, &PN_len);
			log_d("\r\nPN_buf is %s\r\n",PN_buf); 
			log_d("\r\nPN_len is %d\r\n",PN_len);  
		}
	}
	//设置BLE名称
	INT32 snret = fibo_ble_set_read_name(0,(uint8_t *)PN_buf,0); 	//set ble name 0表示设置 
*/
	INT32 snret = fibo_ble_set_read_name(0,(uint8_t *)"8910_ble",0); 	//set ble name 0表示设置
	if(snret >= 0){
		APP_DEBUG("\r\nsnret is %d set ble name success\r\n",snret);
	}
	else{
		APP_DEBUG("\r\nsnret is %d set ble name fail\r\n",snret);
	}

	//获取 BLE 版本号
	char blever[32]={0};
	INT32 bleverret = fibo_ble_get_ver(blever);
	if(bleverret >= 0){
		APP_DEBUG("\r\nbleverret is %d get ble ver success\r\n",bleverret);
		APP_DEBUG("\r\nblever is %s\r\n",blever);
	}
	else{
		APP_DEBUG("\r\nbleverret is %d get ble ver fail\r\n",bleverret);
	}

	char blepubaddr[18]={0};
	INT32 blepubaddrret = fibo_ble_set_read_addr(1,0,blepubaddr);
	if(blepubaddrret >= 0){
		APP_DEBUG("\r\nblepubaddrret is %d get ble pubaddr success\r\n",blepubaddrret);
		APP_DEBUG("\r\nblepubaddr is %s\r\n",blepubaddr);
	}
	else{
		APP_DEBUG("\r\nnblepubaddrret is %d get ble pubaddr fail\r\n",blepubaddrret);
	}

	UINT32 size;
	size = sizeof(config_ble_service)/sizeof(gatt_element_t);                         //添加服务和特性
	INT32 sechret = fibo_ble_add_service_and_characteristic(config_ble_service,size); //create serive and characteristic
	if(sechret >= 0){
		APP_DEBUG("\r\nsechret is %d ble_add_service_and_characteristic success\r\n",sechret);
	}
	else{
		APP_DEBUG("\r\nsechret is %d ble_add_service_and_characteristic fail\r\n",sechret);
	}
	INT32 gbret = fibo_ble_enable_dev(1);//0关闭广播 1开启广播
	if(gbret >= 0){
		APP_DEBUG("\r\ngbret is %d ble_enable_broadcast success\r\n",gbret);
	}
	else{
		APP_DEBUG("\r\ngbret is %d ble_enable_broadcast fail\r\n",gbret);
	}

	while(1)
	{
        for (int n = 0; n < 10; n++){
            OSI_LOGI(0, "hello world %d", n);
            fibo_taskSleep(500);
            Watchdog_feed();
            fibo_watchdog_feed();
        }
		APP_DEBUG("\r\nblever is %s\r\n",blever);
		APP_DEBUG("\r\nnblepubaddr is %s\r\n",blepubaddr);
	}

    fibo_thread_delete();
}



void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}


//============ble===============================================================














//static void prvInvokeGlobalCtors(void);

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
/*
static void prvThreadEntry(void *param)
{
    UINT32 size;
    OSI_LOGI(0, "application thread enter, param 0x%x", param);
    //srand(100);

    //ble
    //fibo_taskSleep(10000);

    for (int n = 0; n < 10; n++)
    {
        OSI_LOGI(0, "hello world %d", n);
        APP_DEBUG("Luee test by debug print %d\r\n",n);
        fibo_taskSleep(500);
        Watchdog_feed();
        fibo_watchdog_feed();
    }

    //fibo_taskSleep(10000);

    fibo_bt_onoff(1);
	fibo_taskSleep(2000);
	//fibo_bt_onoff(0);
	//fibo_taskSleep(2000);
	//fibo_bt_onoff(1);
	//fibo_taskSleep(2000);
	fibo_ble_set_read_name(0,(uint8_t *)"8910_ble",0); // set ble name 
	size = sizeof(config_wifi_service)/sizeof(gatt_element_t);
	fibo_ble_add_service_and_characteristic(config_wifi_service,size); //create serive and characteristic
	fibo_taskSleep(2000);

	fibo_ble_scan_enable(1);

	//fibo_taskSleep(20000);
    for (int n = 0; n < 20; n++)
    {
        OSI_LOGI(0, "hello world %d", n);
        APP_DEBUG("Luee test by debug print %d\r\n",n);
        fibo_taskSleep(500);
        Watchdog_feed();
        fibo_watchdog_feed();
    }

	fibo_ble_enable_dev(1); // open broadcast
	//fibo_taskSleep(20000);
    for (int n = 0; n < 20; n++)
    {
        OSI_LOGI(0, "hello world %d", n);
        APP_DEBUG("Luee test by debug print %d\r\n",n);
        fibo_taskSleep(500);
        Watchdog_feed();
        fibo_watchdog_feed();
    }
#if 0
	fibo_send_notify(3,"abc");

	fibo_taskSleep(20000);

	fibo_send_notify(3,"WER");

	fibo_taskSleep(20000);

	fibo_ble_enable_dev(0); // close broadcast

	fibo_taskSleep(2000);

	char *adv_data = NULL;
	adv_data = fibo_malloc(20);
	memcpy(adv_data,"0406090102",10);

	fibo_ble_set_dev_data(5,adv_data);

	fibo_ble_enable_dev(1); // open broadcast



    fibo_taskSleep(20000);
	fibo_ble_enable_dev(0); // close broadcast
    memcpy(adv_data,"050209010205",12);
	fibo_ble_set_dev_data(6,adv_data); // set broadcast data

	fibo_ble_enable_dev(1); // open broadcast



	fibo_taskSleep(20000);
	fibo_ble_enable_dev(0); // close broadcast
    fibo_ble_set_dev_param(6,60,80,0,0,-1,7,0,NULL);
	fibo_ble_enable_dev(1); // open broadcast
#endif

    //ble

    while(1){
    for (int n = 0; n < 10; n++)
    {
        OSI_LOGI(0, "hello world %d", n);
        APP_DEBUG("Luee test by debug print %d\r\n",n);
        fibo_taskSleep(500);
        Watchdog_feed();
        fibo_watchdog_feed();
    }

    char *pt = (char *)fibo_malloc(512);
    if (pt != NULL)
    {
        OSI_LOGI(0, "malloc address %u", (unsigned int)pt);
        fibo_free(pt);
    }
    }

	//test_printf();
    fibo_thread_delete();
}
*/
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

  INT32 enret = fibo_watchdog_enable(30);  // 60秒=1分钟 无任何语句执行则重启  
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
	//fibo_thread_create(fibo_ble_task, "fibo_ble_task", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    //return (void *)&user_callback;

  return 0;

}

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
/*
static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}
*/

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
/*
void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}
*/