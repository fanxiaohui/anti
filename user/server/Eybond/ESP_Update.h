/**************Copyright(C)锟斤拷2015-2026锟斤拷Shenzhen Eybond Co., Ltd.***************
  *@brief   : eybond soft update process
  *@notes   : 2017.09.11 CGQ 
*******************************************************************************/
#ifndef __ESP_UPDATE_H
#define __ESP_UPDATE_H	 

#include "typedef.h" 
#include "eybond.h"


typedef struct
{
    u8_t md5[16];
    u8_t slice[32];
    u32_t size;
    u16_t sliceSize;
    u16_t sliceCnt;
}ESPUpdate_t;

u8_t Update_file(ESP_t *esp);
u8_t Update_soft(ESP_t *esp);
u8_t Update_device(ESP_t *esp);
u8_t Update_deviceState(ESP_t *esp);
u8_t Update_deviceCancel(ESP_t *esp);
u8_t Update_info(ESP_t *esp);
u8_t Update_dataRcve(ESP_t *esp);
u8_t Update_rcveState(ESP_t *esp);
u8_t Update_dataCheck(ESP_t *esp);
u8_t Update_exit(ESP_t *esp);



#endif //__ESP_UPDATE_H
/******************************************************************************/

