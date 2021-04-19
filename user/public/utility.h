/******************************************************************************           
* name:             grid_tool.h           
* introduce:        头文件
* author:           Luee                                     
******************************************************************************/ 
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "oc_uart.h"

#define NET_LED			(24)
#define GSM_LED			(26)
#define DEVICE_LED		(30)
#define BEEP_PORT		(27)

#define KEY_PORT		(31)
#define WATCHDOG_PIN	(25)
#define ADC0_PORT	    (45)

#define SERIAL_RX_BUFFER_LEN  1024
#define MAX_NET_BUFFER_LEN    1024

#define DEBUG_UART_TXD       28
#define DEBUG_UART_RXD       29

#define DEVICE_UART_TXD      67
#define DEVICE_UART_RXD      68


#define PINDIRECTION_IN   0
#define PINDIRECTION_OUT  1

#define PINLEVEL_LOW      0
#define PINLEVEL_HIGH     1


#endif /* _UTILITY_H_ */