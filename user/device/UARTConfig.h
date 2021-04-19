/**********************************************
  *@brief   : L610_UARTconfig.h L610 UART config
**********************************************************************************/
#ifndef __L610_UARTCONFIG_H
#define __L610_UARTCONFIG_H

#include "typedef.h"

#include "oc_uart.h"
typedef enum {
    FC_NONE=1,  // None Flow Control
    FC_HW,      // Hardware Flow Control 
    FC_SW       // Software Flow Control
} Enum_FlowCtrl;

typedef struct {
    u32_t                      baudrate; 
    hal_uart_data_bits_t       dataBits;
    hal_uart_stop_bits_t       stopBits;
    hal_uart_parity_t          parity;
    Enum_FlowCtrl              flowCtrl;
}ST_UARTDCB;

#endif //__L610_UARTCONFIG_H end
/******************************************************************************/

