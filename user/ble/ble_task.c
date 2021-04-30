/******************************************************************************           
* name:             ble_task.c        
* introduce:        ble
* author:           Luee                                     
******************************************************************************/ 
//sdk
#include <fibo_opencpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lib
#include "r_memory.h"
#include "typedef.h"
#include "r_stdlib.h"
#include "list.h"
//app
#include "Debug.h"
#include "hard_watchdog.h"
#include "ble_task.h"
#include "utility.h"
#include "history.h"

static Buffer_t *debug_input_buf;
static ListHandler_t debug_input_list;

u8 ble_rec_delay=0;

static void debug_input_data_to_BLE(void);
static void debug_input_data_process(u8 *blebuf);
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
char 			BLE_PublicData[BLE_BUFFER_SIZE]={0};
UINT16 			BLE_PublicLen;

//手机AT命令发送
//发送结束，会回调两次此函数
UINT8 data_read_callback(void *param)
{
	u16 len=0;

	r_memcpy(BLE_ReadData,BLE_PublicData,strlen(BLE_PublicData)); 
	//BLE_ReadData 空，则处理DEBUG串口发过来的链表数据
	len=r_strlen(BLE_ReadData);
	if(len==0){
		Ql_OS_SendMessage(BLE_TASK, BLE_DEBUG_INPUT_PROCESS, 0, 0,0);
		//debug_input_data_process((u8 *)BLE_ReadData);
		return 0;
	}else{
		r_memset(BLE_PublicData,0,sizeof(BLE_PublicData));
    	APP_DEBUG("client phone rec:BLE_PublicData=%s,len=%d\r\n",BLE_ReadData,strlen(BLE_ReadData));
		r_memset(BLE_ReadData,0,sizeof(BLE_ReadData));
		return 0;
	}
}

//手机AT命令接收
UINT8 data_write_callback(void *param)
{
	//将手机发过来的BLE_PublicData拷贝到BLE_WriteData
	memcpy(BLE_WriteData,BLE_PublicData,strlen(BLE_PublicData)); 
	memset(BLE_PublicData,0,sizeof(BLE_PublicData));
	
	APP_DEBUG("client phone send len=%d,data:\r\n",strlen(BLE_WriteData));
	APP_DEBUG("%s\r\n",BLE_WriteData);
	BLE_WriteLen = strlen(BLE_WriteData);
	char BLE_RecvData_Copy[BLE_BUFFER_SIZE]={0};
	if(BLE_WriteLen>4)
	{
		memcpy(BLE_RecvData_Copy,BLE_WriteData,BLE_WriteLen); 
		// 
        if (BLE_WriteLen > 2 && 0 == r_strncmp((char *)BLE_RecvData_Copy, "AT", 2)) {
          //r_memcpy(&debug_input_buf->payload[debug_input_buf->lenght], "\r\n", 3);
          //debug_input_buf->lenght += 3;
		  APP_DEBUG("phone input AT cmd\r\n");
          //fibo_at_send(buf->payload, buf->lenght);
        } else {
          //strCmp(buf, (void_fun_bufp)((void *)msg.param2));
        }

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
	//将手机发送的数据返回给手机
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

static void debug_input_data_to_BLE(void)
{
	if(r_strlen(BLE_PublicData)==0){
		//将DEBUG输入数据发送给手机
		r_memcpy(BLE_PublicData,debug_input_buf->payload,debug_input_buf->lenght);
    	APP_DEBUG("debug input data put to BLE buf:%s %d\r\n", (char *) debug_input_buf->payload, debug_input_buf->lenght);
    	if (debug_input_buf->lenght > 2 && 0 == r_strncmp((char *)debug_input_buf->payload, "AT", 2)) {
		  APP_DEBUG("decode debug input AT cmd\r\n");
		}
    } 
}

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
static void debug_input_data_process(u8 *blebuf) {
  if(r_strlen((char *)blebuf)==0){
		//将DEBUG输入数据发送给手机
		buf_t *buf=(buf_t *)list_nextData(&debug_input_list,null);
  		if (null == buf) {
			list_delete(&debug_input_list);
			list_init(&debug_input_list);
    		return;
  		}
		r_memcpy(BLE_PublicData,buf->payload,buf->lenght);
    	APP_DEBUG("debug input data put to BLE buf:%s len=%d\r\n", buf->payload, strlen((char *)buf->payload));
    	if (buf->lenght > 2 && 0 == r_strncmp((char *)buf->payload, "AT", 2)) {
		  APP_DEBUG("decode debug input AT cmd\r\n");
		}
		list_nodeDelete(&debug_input_list, buf);
    } 
}

/*******************************************************************************            
* introduce:        
* parameter:                       
* return:                 
* author:           Luee                                                    
*******************************************************************************/
void ble_task(void *param)
{
	ST_MSG msg;

	list_init(&debug_input_list);
	
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

	//Buffer_t *parabuf=null;
	//SysPara_Get(2,parabuf);		//得到PN值
	////设置BLE名称
	//INT32 snret = fibo_ble_set_read_name(0,(uint8_t *)parabuf->payload,0); 	//set ble name 0表示设置 
	//memory_release(parabuf->payload);
	
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

	while(1){
		//APP_DEBUG("\r\nblever is %s\r\n",blever);
		APP_DEBUG("\r\nnblepubaddr is %s\r\n",blepubaddr);
		watchdogns(1);
		
		u32 msg_ret=fibo_queue_get(BLE_TASK, (void *)&msg, 0);
		//if(msg_ret==0){
			switch(msg.message){
				case BLE_DEBUG_MSG_ID:
				//得到debug输入buf:debug_input_buf									
				debug_input_buf = (Buffer_t *)msg.param1;
				
				//如果BLE buf:是空的，则放入buf,否则将数据放入链表
				if(r_strlen(BLE_PublicData)==0){
					//将DEBUG输入数据发送给手机
					r_memcpy(BLE_PublicData,debug_input_buf->payload,debug_input_buf->lenght);
        			APP_DEBUG("debug input data put to BLE buf:%s %d\r\n", (char *) debug_input_buf->payload, debug_input_buf->lenght);
        			if (debug_input_buf->lenght > 2 && 0 == r_strncmp((char *)debug_input_buf->payload, "AT", 2)) {
					  APP_DEBUG("decode debug input AT cmd\r\n");
					}
				}else{
					buf_t *list_buf=list_nodeApply(debug_input_buf->lenght +4);		//长度指的是整个结构体的长度
			    	if (list_buf == null) {
			    	  //log_save("list_buf memory apply fail!!");
					  APP_DEBUG("list_buf memory apply fail!!\r\n")
					  break;
			    	} else {
			    	  	list_buf->lenght = debug_input_buf->lenght; 
			    	  	r_memcpy(list_buf->payload, debug_input_buf->payload, debug_input_buf->lenght); // 把指令放入列表节点的内存
			    	  	list_bottomInsert(&debug_input_list, list_buf);   // 把debug输入插入列表节点
						APP_DEBUG("debug input data put to list\r\n");
						APP_DEBUG("%s\r\n",debug_input_buf->payload);
					}
				}
				break;
				
				case BLE_DEBUG_INPUT_PROCESS:
				debug_input_data_process((u8 *)BLE_PublicData);									
				break;

				default :
				break;
			}
		//}	
	}
	fibo_thread_delete();
}



//============ble===============================================================