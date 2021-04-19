/**************Copyright(C)��2015-2026��QIYI TEMP    *************************
  *@brief   : SysPara.h 
  *@notes   : 2017.12.21 CGQ ����   
*******************************************************************************/
#ifndef __SYS_PARA_H
#define __SYS_PARA_H	 

#include "Typedef.h"

#define SYY_PARA_LEN        64
#define SYS_PARA_CHANGE			(0x01F000)

#define BUZER_EN_ADDR			(48)
#define GPRS_CCID_ADDR			(56)
#define GPRS_APN_ADDR			(57)
#define EYBOND_SERVER_ADDR		(21)
#define HANERGY_SERVER_ADDR		(24)
#define STATE_GRID_SERVER_ADDR	(23)
#define SAJ_SERVER_ADDR 		(22)
#define GPRS_BASE_ID            (59)

#define STATE_GRID_SN  			(65)
#define STATE_GRID_USER_NAME    (66)
#define STATE_GRID_PASSWORD     (67)
#define STATE_GRID_REGISTER_ID  (68)

#define TIME_ZONE_ADDR			(25)

#define DEVICE_MONITOR_NUM		(12)
#define DEVICE_PROTOCOL			(14)
#define METER_PROTOCOL			(15)

#define DEVICE_VENDER			(8)

/* Sarnath parameters*/
#define SARNATH_SERVER_ADDR     (71)
#define SARNATH_SERVER_PORT     (72)
#define SARNATH_FACTORY_ID      (73)
#define SARNATH_LOGGER_ID       (74)
#define SARNATH_UPLOAD_TIME     (75)

/* buf :para read or write char point, return : 0(opt OK)*/
typedef int (*paraFun)(Buffer_t *buf);

/*authority 
	bit 0(para read);   0(unable), 1(able)
	bit 1(Para write);	 0(unable),  1(able)
	bit 2(para W authority); 0(No authorized), 1(Be authorized)
	bit 4(R&W point type);  0(string point), 1(funcation point)
*/
typedef enum
{
	P_R = (1<<0),
	PNR = (0<<0),
	P_W = (1<<1),
	PNW = (0<<1),
	P_A = (1<<2),
	PNA = (0<<2),
	P_F = (1<<4),
	P_S = (0<<4),
}SysParaAuth_e;

typedef struct
{
	u16_t number;		//para number
	u8_t authority;		//goto authority
	u8_t offset;		//para inseat offset
	void *read;			//Read
    void *write;		//write 
}SysParaTab_t;

typedef struct 
{
	u16_t code;
	u16_t  def;  //reset Factory setting, 0,invalid,  1: User, 
	char str[28];
}SysDefParaTab_t;

typedef struct
{
	u8_t  type;  //connect type 0: UDP 1:TCP 2:SSL
	u16_t port;
	char addr[2];
}ServerAddr_t;

int SysPara_init(void);
void SysPara_printf(void_fun_bufp output);
void SysPara_auth(void);
void SysPara_default(void);
void SysPara_Get(u8_t num, Buffer_t *buf);
int SysPara_Set(u8_t num, Buffer_t *buf);
void SysParaErase(void);

ServerAddr_t *ServerAdrrGet(u8_t num);



#endif //__SYS_PARA_H

/******************************************************************************/

