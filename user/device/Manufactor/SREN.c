/**************Copyright(C)，2015-2026，Shenzhen Eybond Co., Ltd.***************
  *@brief   : SREN.c
  *@notes   : 2018.05.11 CGQ   
*******************************************************************************/
#include "Protocol.h"
#include "typedef.h"
#include "Modbus.h"
#include "Device.h"

static const ModbusGetCmd_t sren090D[] = {
    {0x03, 0x000A , 0x001A},
    {0x03, 0x0100 , 0x011f},
    {0x03, 0x0120 , 0x0122},
};
static const ModbusGetCmdTab_t sren090DProtocol = PROTOCOL_TABER(sren090D, 0, 0x090D);
const ModbusDeviceHead_t SRENDevice = {
    &UART_9600_N1,
    &sren090D[0],
    &sren090DProtocol,
    null,
    null,
};
static const ModbusGetCmd_t srne[] = {
    {0x03, 0x7000, 0x701F},
    {0x03, 0x7020, 0x703D},
    {0x03, 0x703E, 0x7043},
    {0x03, 0xDF00, 0xDF00},
    {0x03, 0xE800, 0xE81F},
    {0x03, 0xE820, 0xE83F},
    {0x03, 0xE840, 0xE855},
    {0x03, 0xE900, 0xE917},
    {0x03, 0xE999, 0xE99C},
};
static const ModbusGetCmdTab_t srneProtocol = PROTOCOL_TABER(srne, ((32/8)<<1)|(1<<0), 0x02C5);
const ModbusDeviceHead_t SrneDevice = {
	&UART_9600_N1,
	&srne[0],
	&srneProtocol,
	null,
	null,
}; 

static const ModbusGetCmd_t srne0920[] = {
    {0x03, 0x000B, 0x002A},
    {0x03, 0x002B, 0x0043},
    {0x03, 0x0100, 0x0102},
    {0x03, 0x0107, 0x010F},
    {0x03, 0x0204, 0x0223},
    {0x03, 0x0224, 0x0225},
    {0x03, 0xF000, 0xF006},
    {0x03, 0xF007, 0xF00D},
    {0x03, 0xF00E, 0xF014},
    {0x03, 0xF015, 0xF01B},
    {0x03, 0xF01C, 0xF022},
    {0x03, 0xF023, 0xF029},
    {0x03, 0xF02D, 0xF031},
    {0x03, 0xF034, 0xF03D},
    {0x03, 0xF040, 0xF04B},
};
static const ModbusGetCmdTab_t srne0920Protocol = PROTOCOL_TABER(srne0920, ((32/8)<<1)|(1<<0), 0x0920);
const ModbusDeviceHead_t Srne0920Device = {
	&UART_9600_N1,
	&srne0920[0],
	&srne0920Protocol,
	null,
	null,
}; 

static const ModbusGetCmd_t srne091F[] = {
    {0x03, 0x000B, 0x002A},
    {0x03, 0x002B, 0x0043},
    {0x03, 0x0100, 0x0102},
    {0x03, 0x0107, 0x010F},
    {0x03, 0x0204, 0x0223},
    {0x03, 0x0224, 0x0225},
    {0x03, 0xF000, 0xF006},
    {0x03, 0xF007, 0xF00D},
    {0x03, 0xF00E, 0xF014},
    {0x03, 0xF015, 0xF01B},
    {0x03, 0xF01C, 0xF022},
    {0x03, 0xF023, 0xF029},
    {0x03, 0xF02D, 0xF031},
    {0x03, 0xF034, 0xF03D},
    {0x03, 0xF040, 0xF04B},
};
static const ModbusGetCmdTab_t srne091FProtocol = PROTOCOL_TABER(srne091F, ((32/8)<<1)|(1<<0), 0x091F);
const ModbusDeviceHead_t Srne091FDevice = {
	&UART_9600_N1,
	&srne091F[0],
	&srne091FProtocol,
	null,
	null,
}; 


/******************************************************************************/

