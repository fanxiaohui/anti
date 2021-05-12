/******************************************************************************           
* name:                       
* introduce:        
* author:           Luee                                     
******************************************************************************/ 
//标注有 "Luee 之后再加入",需要后面重新加入
//sdk
#include <fibo_opencpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lib
#include "typedef.h"
#include "Swap.h"
#include "r_stdlib.h"
#include "r_memory.h"
#include "list.h"
#include "HashMD5.h"
#include "ieee754_float.h"
#include "char2negative.h"
#include "struct_type.h"
//app
#include "Debug.h"
#include "utility.h"
#include "data_collector_parameter_table.h"
#include "struct_type.h"
#include "parameter_number_and_value.h"
#include "run_log.h"
#include "SysPara_File.h"
#include "hard_watchdog.h"
#include "grid_tool.h"
#include "DeviceIO.h"
#include "Device.h"
#include "Protocol.h"
#include "eybond.h"
#include "CommonServer.h"
#include "ModbusDevice.h"

#include "DeviceIO.h"
#include "Device.h"
#include "Protocol.h"
#include "eybond.h"
#include "CommonServer.h"
#include "grid_tool.h"
#include "ModbusDevice.h"
#include "ieee754_float.h"
#include "char2negative.h"
#include "anti_reflux.h"

static u8_t Para_Init_flag = 0;
char produc_save_flag = 0;
// static char para_value[64] = {0};  // 参数值


#ifdef _PLATFORM_L610_
void parametr_get(u32_t number, Buffer_t *databuf) {
  char ch1[]={"-123.543"}; 
  char ch2[]={"-200"}; 
  char ch3[]={"123"}; 


  char *buf_value = NULL;
  u16_t len = 0;
  Buffer_t logbuf;
  u8 parageting=0;    //=0 got para finish =1 get paraing

  int j = 0;


 // if (databuf == NULL||device_data_geting) {
  if (databuf == NULL) {

    //parageting=0;   //参数获取完成
    //return;
    len=0;
    goto GETFAIL;
  }


 
  for (j = 0; j < number_of_array_elements; j++) {
    if (number == PDT[j].num) {
      

      APP_DEBUG("para_meter[%d]num = %ld \r\n", j, number);
      buf_value = memory_apply(sizeof(char) * 64);
      if (buf_value == NULL) {
        APP_DEBUG("MEM Alloc Error\r\n");
        return;
      }
      r_memset(buf_value, 0, sizeof(char) * 64);
      if (number == 5 || number == 6 || number == 11 || number == 16\
          || number == 49 || number == 50 || number == 51 || number == 52\
          || number == 55 || number == 56 || number == 58 ||  number == 54 || number == 98) {
        switch (number) {
          case 5:  // 软件版本号
            r_strcpy(buf_value, defaultPara[j].para);
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            PDT[j].wFunc(&PDT[j], buf_value, &len);
            break;
          case 6:  // 硬件版本号
            r_strcpy(buf_value, defaultPara[j].para);
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            PDT[j].wFunc(&PDT[j], buf_value, &len);
            break;
          case 11:  // 是否有设备在线
            //test float
            //get_float();
            //get_ieee754();

                 
            APP_DEBUG("char2negative:%f\n",my_atof(ch1)); 
            APP_DEBUG("char2negative:%f\n",my_atof(ch2)); 
            APP_DEBUG("char2negative:%f\n",my_atof(ch3)); 

            //将在线设备数量转为字符并存在buf_value
            Swap_numChar(buf_value,onlineDeviceList.count);
            //r_strcpy(buf_value, "1");
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            PDT[j].wFunc(&PDT[j], buf_value, &len);
            break;
          case 16: {  // 查询IP地址
            break;
          }
          case 49:  // 查询网络注册信息
            break;
          case 50:  // GPRS 网络注册状态
            break;
          case 51:  // 固件编译日期
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            PDT[j].wFunc(&PDT[j], MAKE_DATE, &len);
            break;
          case 52:  // 固件编译时间
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            PDT[j].wFunc(&PDT[j], MAKE_TIME2, &len);
            break;
          case 54:  // 获取日志
            if(parageting==0){
              parageting=1;
              logbuf.payload=fibo_malloc(64);
              len=log_get(&logbuf);
              if(len==0){
                APP_DEBUG("\r\n-->log:get log fail\r\n");
                fibo_free(logbuf.payload);
                break;
              }
              r_memcpy(buf_value, logbuf.payload, len);
              fibo_free(logbuf.payload);
              //r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
              len = r_strlen(buf_value);
              //PDT[j].wFunc(&PDT[j], buf_value, &len);
              break;
            }else{
              r_strcpy(buf_value,"system busying!!!\r\n");
              len = r_strlen(buf_value);
              break;
            }  
          case 55:  {  // 获取CSQ值
            s8_t nrssi = 0, nber = 0;
            fibo_get_csq((INT32 *)&nrssi, (INT32 *)&nber);
            snprintf(buf_value, 64, "%d", nrssi);
            len = r_strlen(buf_value);
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            PDT[j].wFunc(&PDT[j], buf_value, &len);
            break;
          }
          case 56:  // 通信卡CCID
            fibo_get_ccid((u8_t *)buf_value);
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            len = r_strlen(buf_value);
            PDT[j].wFunc(&PDT[j], buf_value, &len);
            break;
          case 58:  // CPUID IMEI
            fibo_get_imei((u8_t *)buf_value, 0);
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            len = r_strlen(buf_value);
            PDT[j].wFunc(&PDT[j], buf_value, &len);
            break;
		  case 98:  // 锦浪版本号
            r_strcpy(buf_value, defaultPara[j].para);
            r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
            PDT[j].wFunc(&PDT[j], buf_value, &len);
            break;
          default:
            break;
        }
      }
      //获取日志,从日志中读取，不需从参数中读取
      if(number==54){
        
      }
      //其它从参数文件中读取
      else{        
        r_memset(buf_value, 0, sizeof(char) * 64);    
        PDT[j].rFunc(&PDT[j], buf_value, &len);
      }
 

GETFAIL:
      if(len==0){
        r_strcpy(buf_value,"para get fail!\r\n");
        len = r_strlen(buf_value);
      }

//      APP_DEBUG("%d=%s len=%d\r\n", PDT[j].num, buf_value, len);
      if (len > 0) {
//      APP_DEBUG("%d=%s len %d\r\n", PDT[j].num, buf_value, len);
        databuf->size = len;
        databuf->lenght = len;
        databuf->payload = memory_apply(databuf->size + 1);  // 将获取的参数传递出去 申请后需要在函数外部释放
        if (databuf->payload == NULL) {
          APP_DEBUG("sys para databuf payload apply fail\r\n");
        } else {
          r_memset(databuf->payload, 0, databuf->size + 1);
          r_memcpy(databuf->payload, buf_value, databuf->lenght);
          APP_DEBUG("\r\n-->tcp lose:get para num= %ld,value=%s,len=%d\r\n",number,databuf->payload,databuf->lenght);
        }
      } else {
        APP_DEBUG("para fail!\r\n");
      }
      memory_release(buf_value);
      parageting=0;   //参数获取完成
      break;
    }
    
  }

 
  parageting=0;   //参数获取完成
//  APP_DEBUG("index: %d totle: %d\r\n", j, number_of_array_elements);
  if (j >= number_of_array_elements) {
    databuf->size = 0;
    databuf->lenght = 0;
    databuf->payload = NULL;
  }
}

u8_t parametr_set(u32_t number, Buffer_t *data) {
  int j = 0;
  u8_t ret = 0x00;
  char *str = NULL;
  for (j = 0; j < number_of_array_elements; j++) {
    if (number == PDT[j].num) {
      APP_DEBUG("para[%d] number is %ld Old value is %s\r\n", j, number, PDT[j].a);
//    APP_DEBUG("para[%d] number is %ld New value will be %s\r\n", j, number, data->payload);
      str = memory_apply(sizeof(char) * 64);
      if (str == NULL) {
        APP_DEBUG("MEM Alloc Error\r\n");
        ret = 0x03;
        return ret;
      }
      r_memset(str, '\0', sizeof(char) * 64);
      r_strncpy(str, (char *)data->payload, r_strlen((char *)data->payload));
      APP_DEBUG("\r\n-->para[%d] number is %ld str %s\r\n", j, number, str);
      switch (number) {  // 01/02/03/07/08/12/14/29/34/57 生产是必须写的参数
        case 0:
        case 4:
        case 5:
        case 6:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 27:
        case 28:
        case 49:
        //case 50:
        case 51:
        case 52:
        case 55:
        case 56:
        case 58:
          APP_DEBUG("para_meter can't be set \r\n");
          ret = 0x02;
          break;
        case 14: {
          break;
        }
        case 29:
          APP_DEBUG("\r\n-->number is %ld str %s\r\n",number, data->payload);
          switch (*(data->payload)) {
            case '1':  // 采集器硬重启代替软重启
              soft_reset_en();              
              break;
            case '2':  // 恢复出厂设置 -- 实测若加入恢复参数为出厂设置，则生产时14号参数不能被写入
              //log_save("System Para Reset!");
              //parametr_default();
              //设备重置
              //Eybpub_UT_SendMessage(EYBDEVICE_TASK, SYS_PARA_CHANGE, number, 0,0);  //Luee 之后再加入
              break;
            case '3':   //软重启
              fibo_softReset();
              break;
            case '6':  // 看门狗停狗重启
              log_save("System Hardware Reset!");
              Watchdog_stop();
              break;
            case '7':  
              log_clean();    //Luee 日志清除
              break;
            case '8':  
              para_init();    //Luee 参数初始化
              Watchdog_stop();
              break;

            #if 0
            case '4':  // 删除a文件测试
              log_save("delete a file success!");
              rm_file_A ();
              break;
            #endif
            default:
              break;
          }
          ret = 0x00;
          APP_DEBUG("PARA %d=%s\r\n", PDT[j].num, data->payload);
          return ret;
          break;
        case 34: {
          //生产时不修改14号参数
          if(produc_save_flag)
          break;
          ListHandler_t uartCfgStr;
          r_strsplit(&uartCfgStr, str, '-');
          int tab[4] = {0};
          int i = 0;
          Node_t *node;
          node = uartCfgStr.node;
          do {
            tab[i++] = Swap_charNum((char *) * (int *)node->payload);
            APP_DEBUG("\r\nBand %d, %d.\r\n", i, tab[i - 1]);
            node = node->next;
          } while (node != uartCfgStr.node && i < 4);
          ST_UARTDCB uart;
          if ((tab[0] > 2000 && tab[0] < 1000000)   // 波特率
              && (tab[1] > 4 && tab[1] < 9)  // 数据位
              && (tab[2] > 0 && tab[2] < 4)  // 停止位
              && (tab[3] >= 0 && tab[3] < 5)) {   // 校验位
            uart.baudrate = tab[0];
            uart.dataBits = tab[1];
            uart.stopBits = tab[2];
            uart.parity = tab[3];
            uart.flowCtrl = FC_NONE;
          } else if (tab[0] > 2000 && tab[0] < 1000000) {
            uart.baudrate = tab[0];
            uart.dataBits = HAL_UART_DATA_BITS_8;
            uart.stopBits = HAL_UART_STOP_BITS_1;
            uart.parity = HAL_UART_NO_PARITY;
            uart.flowCtrl = FC_NONE;
          } else {
            ret = 0x02;
            break;
          }
          list_delete(&uartCfgStr);   // mike 20200922
          char str_temp_1[64] = {0};
          char str_temp_2[64] = {0};
          u16_t len = 0;
          char *P1 = NULL;
          r_memset(str, '\0', sizeof(char) * 64);
          r_strncpy(str, (char *)data->payload, r_strlen((char *)data->payload));
          r_memset(str_temp_1, 0, sizeof(str_temp_1));
          r_memset(str_temp_2, 0, sizeof(str_temp_2));
          snprintf(str_temp_2, sizeof(str_temp_2), "#%ld-%d-%d-%d#", uart.baudrate, uart.dataBits, uart.stopBits, uart.parity);
          for (i = 0; i < number_of_array_elements; i++) {
            if (14 == PDT[i].num) {
              PDT[i].rFunc(&PDT[i], str_temp_1, &len);
              P1 = r_strstr(str_temp_1, "#");
              if (P1) {
                *P1 = '\0';   // 去掉#号
                len = r_strlen(str_temp_1);
                APP_DEBUG("%s len:%d\r\n", str_temp_1, len);
                r_strcat(str_temp_1, str_temp_2);
                len = r_strlen(str_temp_1);
                APP_DEBUG("%s len:%d\r\n", str_temp_1, len);
                r_memset((&PDT[i])->a, 0, sizeof((&PDT[i])->a));
                PDT[i].wFunc(&PDT[i], str_temp_1, &(len));
              }
            }
          }
          break;
        }
        default:
          break;
      }

      if (ret == 0x00) {
        //这里只是写入成功，但最后是不是更新成功未知
        ret = 0x00;
        //防止写入字符串过长
        if (data->lenght >= sizeof((&PDT[j])->a)) {
          APP_DEBUG("para so long!\r\n");
          ret = 0x03;
          break;
        }
        r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
        PDT[j].wFunc(&PDT[j], str, &(data->lenght));

        if (number == 21) {
          log_save("set para 21: %s", (&PDT[j])->a);
        }
        APP_DEBUG("PARA %d=%s\r\n", PDT[j].num, str);
        break;
      }
      memory_release(str);
    }
    if (j >= number_of_array_elements) {
      APP_DEBUG("para_meter %ld is wrongth\r\n", number);
      ret = 0x03;
    }
  }
  // start_timer();
  if (ret == 0x00 && number != LOCAL_TIME) {  // mike 心跳时间不存到参数系统
    parameter_a_module();
    a_copy_to_b();
    // 将生产参数存进c文件
    if (1 == produc_save_flag) {
      APP_DEBUG("begin to backup parameter\r\n");
      a_copy_to_c();
    }
    parameter_init();  // 保持统一
    if (number == DEVICE_MONITOR_NUM || number == DEVICE_PROTOCOL || number == DEVICE_UART_SETTING) {
      //Eybpub_UT_SendMessage(EYBDEVICE_TASK, SYS_PARA_CHANGE, number, 0,0);  //Luee  之后再加入
    } else if (number == DEVICE_PNID || number == EYBOND_SERVER_ADDR) {
      //Eybpub_UT_SendMessage(EYBNET_TASK, NET_CMD_RESTART_ID, 0, 0,0);       //Luee  之后再加入
    }
  }
  return ret ;
}

void main_parametr_update(void) { // 由于APP固件升级会让系统保存的参数不匹配，需要实时更新
  u16_t len = 0;
  int j = 0;
  int number = 0;
  char *buf_value = NULL;
  buf_value = memory_apply(sizeof(char) * 64);
  if (buf_value == NULL) {
    APP_DEBUG("MEM Alloc Error\r\n");
    return;
  }
//  r_memset(buf_value, 0, sizeof(char) * 64);

  APP_DEBUG("main_parametr_update!\r\n");
  for (j = 0; j < number_of_array_elements; j++) {
    r_memset(buf_value, 0, sizeof(char) * 64);
    number = PDT[j].num;
    if (number == 5 || number == 6 || number == 11 || number == 49 || number == 50 \
        || number == 51 || number == 52 || number == 55 || number == 56 || number == 58 || number == 98 ) {
      switch (number) {
        case 5:  // 软件版本号
          r_strcpy(buf_value, defaultPara[j].para);
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          len = r_strlen(buf_value);
          PDT[j].wFunc(&PDT[j], buf_value, &len);
          break;
        case 6:  // 硬件版本号
          r_strcpy(buf_value, defaultPara[j].para);
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          len = r_strlen(buf_value);
          PDT[j].wFunc(&PDT[j], buf_value, &len);
          break;
        case 11:  // 是否有设备在线
          r_strcpy(buf_value, "1");
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          len = r_strlen(buf_value);
          PDT[j].wFunc(&PDT[j], buf_value, &len);
          break;
        case 49:  // 查询网络注册信息
          // cm_gprs_getcregstate(&creg);
          // cm_itoa(creg,creg_char,10);
          // r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          // PDT[j].wFunc(&PDT[j],creg_char, &len);
          break;
        case 50:  // GPRS 网络注册状态
          // cm_gprs_getcgregstate(&cgreg);
          // cm_itoa(cgreg,cgreg_char,10);
          // r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          // PDT[j].wFunc(&PDT[j],cgreg_char, &len);
          break;
        case 51:  // 固件编译日期
          r_strcpy(buf_value, defaultPara[j].para);
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          len = r_strlen(defaultPara[j].para);
          PDT[j].wFunc(&PDT[j], buf_value, &len);
          break;
        case 52:  // 固件编译时间
          r_strcpy(buf_value, defaultPara[j].para);
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          len = r_strlen(defaultPara[j].para);
          PDT[j].wFunc(&PDT[j], buf_value, &len);
          break;
        case 55: {  // 获取CSQ值
          s8_t nrssi = 0, nber = 0;
          fibo_get_csq((INT32 *)&nrssi, (INT32 *)&nber);
          snprintf(buf_value, 64, "%d", nrssi);
          len = r_strlen(buf_value);
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          PDT[j].wFunc(&PDT[j], buf_value, &len);
          break;
        }
        case 56:  // 通信卡CCID
          fibo_get_ccid((u8_t *)buf_value);
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          len = r_strlen(buf_value);
          PDT[j].wFunc(&PDT[j], buf_value, &len);
          break;
        case 58:  // CPUID IMEI
          fibo_get_imei((u8_t *)buf_value, 0);
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          len = r_strlen(buf_value);
          PDT[j].wFunc(&PDT[j], buf_value, &len);
          break;
	    case 98:  // 锦浪软件版本号
          r_strcpy(buf_value, defaultPara[j].para);
          r_memset((&PDT[j])->a, 0, sizeof((&PDT[j])->a));
          len = r_strlen(buf_value);
          PDT[j].wFunc(&PDT[j], buf_value, &len);
		  APP_DEBUG("default 98 write = %s\r\n",buf_value);
          break;
        default:
          break;
      }
    }
  }
  memory_release(buf_value);
  parameter_a_module();
  a_copy_to_b();
  parameter_init();  // 保持统一
}
#endif

ServerAddr_t *ServerAdrrGet(u8_t num) {
  
  Buffer_t buf;
  Buffer_t portBuf;
  ServerAddr_t *serverAddr = null;

  r_memset(&buf, 0, sizeof(Buffer_t));
  r_memset(&portBuf, 0, sizeof(Buffer_t));
  parametr_get(num, &buf);  // TODO获取的buf长度有问题?


  if (buf.payload != null && buf.lenght > 5) {
    ListHandler_t cmdStr;
    int len = 0;

    r_strsplit(&cmdStr, (char *) buf.payload, ':');

    if (cmdStr.count > 0) {
      len = r_strlen((char *) * (int *) cmdStr.node->payload);
      serverAddr = memory_apply(sizeof(ServerAddr_t) + len);
      r_strcpy(serverAddr->addr, (char *) * (int *) cmdStr.node->payload);
      serverAddr->type = 1;
      if (num == SARNATH_SERVER_ADDR) {
        parametr_get(SARNATH_SERVER_PORT, &portBuf);
        serverAddr->port = Swap_charNum((char *)portBuf.payload);
      } else {
        serverAddr->port = 502;
      }

      if (num == HANERGY_SERVER_ADDR) {
        serverAddr->port = 8081;
      } else {
        serverAddr->port = 502;
      }

      if (cmdStr.count > 1) {
        serverAddr->port = Swap_charNum((char *) * (int *) cmdStr.node->next->payload);
        // TODO 有问题，一直会进入这里
      }

      if (cmdStr.count > 2) {
        if (r_strstr((char *) * (int *) cmdStr.node->next->next->payload, "UDP") != NULL) {
          serverAddr->type = 0;
        } else if (r_strstr((char *) * (int *) cmdStr.node->next->next->payload, "SSL") != NULL) {
          serverAddr->type = 2;
        }
      }
    }
    list_delete(&cmdStr);
  }

  if (serverAddr->port == 0) {
    if (serverAddr != NULL) {
      memory_release(serverAddr);
      serverAddr = null;
    }
  }

  memory_release(portBuf.payload);
  portBuf.size = 0;
  portBuf.lenght = 0;
  memory_release(buf.payload);
  buf.lenght = 0;
  buf.size = 0;

  return serverAddr;

}

// void SysPara_init(void) {
//   APP_DEBUG("SysPara_init\r\n");
//   Para_Init_flag = 0;
//   live_a_and_b();
//   main_parametr_update();  //
// }

int SysPara_init(void) {
  APP_DEBUG("SysPara_init\r\n");
  Para_Init_flag = 0;
  // 参数初始化失败
  if(live_a_and_b()) {
    log_save("sys para init fail.");
    return -1;
  }
  main_parametr_update();
  return 0;
}

u8_t SysPara_Get_State(void) {
  return Para_Init_flag;
}

void parametr_default(void) {   // mike 依据default配置重新生成PDT表
  APP_DEBUG("parametr_default\r\n");

  u16_t len = 0;
  int i = 0;
  for (i = 0; i < number_of_array_elements; i++) {
    if (r_strlen(defaultPara[i].para) <= 0) {
      continue;
    }
    r_memset((&PDT[i])->a, 0, sizeof((&PDT[i])->a));
    len = r_strlen(defaultPara[i].para);
    PDT[i].wFunc(&PDT[i], (void *)defaultPara[i].para, &len);
  }
  parameter_a_module();
  a_copy_to_b();
  parameter_init();  // 保持统一
}

void GET_ALL_data(void) {
  int j = 0;
  char *buf  = NULL;
  u16_t len = 64;

  for (j = 0; j < number_of_array_elements; j++) {
    //日志系统
    if (PDT[j].num == 54) {
      continue;
    }

    buf = memory_apply(sizeof(char) * 64);
    r_memset(buf, 0, sizeof(char) * 64);
    PDT[j].rFunc(&PDT[j], buf, &len);
    if (r_strlen(buf) > 0) {
      APP_PRINT("%d=%s\r\n", PDT[j].num, buf);
    }
    memory_release(buf);
  }
  Para_Init_flag = 1;
}

/******************************************************************************/
