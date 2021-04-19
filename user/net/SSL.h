/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : SSL.h
 * @Author  : CGQ
 * @Date    : 2018-10-25
 * @Brief   : 
 ******************************************************************************/
#ifndef __SSL_
#define __SSL_

#include "typedef.h"
#include "M26Net.h"

#define CA_FILE_ADDR        (0x00390000)

void SSL_init(void);
int SSL_Open(M26Net_t *net);
void SSLHandler_TCPConnect(const char* strURC, void* reserved);
int SSL_Send(s32_t socketId, u8_t* pData, s32_t dataLen);
void SSLHandler_DataRcve(const char* strURC, void* reserved);

#endif //__SSL_
/******************************************************************************/
