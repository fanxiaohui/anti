/******************************************************************************           
* name:                  
* introduce:        
* author:           Luee                                     
******************************************************************************/ 
//sdk
#include <stdio.h>
#include <fibo_opencpu.h>

//lib
#include "r_memory.h"
#include "typedef.h"
#include "r_stdlib.h"
//app
#include "Debug.h"
#include "utility.h"


#ifdef  EYBOND_DEBUG_ENABLE
Buffer_t UARTDEBUG_buf;
#endif

void UARTDEBUG_CallBack(hal_uart_port_t uart_port, UINT8 *data, UINT16 len, void *arg) {
  APP_DEBUG("uartapi recv uart_port=%d len=%d, data len=%ld\r\n", uart_port, len, r_strlen((char *)data));
  switch (uart_port) {
    case DEBUG_PORT: {
      if (len > SERIAL_RX_BUFFER_LEN || len == 0) {
        APP_DEBUG("UART get data len is big than %d\r\n", SERIAL_RX_BUFFER_LEN);
        return;
      }
      u16_t nTemp_i = 0;
      for (nTemp_i=0; nTemp_i < len; nTemp_i ++) {
        if (data[nTemp_i] < 10 || data[nTemp_i] > 126) {  // DEBUG串口只接收可见字符
          APP_DEBUG("DEBUG UART get a data less than 0x0A or large than 0x7E\r\n");
          return;
        }
      }
      if (UARTDEBUG_buf.payload != NULL) {
        memory_release(UARTDEBUG_buf.payload);  // clean debug buffer DEBUG串口接收的数据统一在这里释放内存
        UARTDEBUG_buf.payload = NULL;
      }
      UARTDEBUG_buf.payload = memory_apply(SERIAL_RX_BUFFER_LEN);
      if (UARTDEBUG_buf.payload == NULL) {
        //APP_DEBUG("memory alloc Fail\r\n");
        return;
      }
      UARTDEBUG_buf.size = SERIAL_RX_BUFFER_LEN;
      UARTDEBUG_buf.lenght = 0;
      r_memset(UARTDEBUG_buf.payload, '\0', UARTDEBUG_buf.size);
      r_memcpy(UARTDEBUG_buf.payload, data, len);
      UARTDEBUG_buf.lenght = len;

//下面一行暂时关闭，之后再调试 Luee
      //Eybpub_UT_SendMessage(EYBAPP_TASK, APP_DEBUG_MSG_ID, (u32_t)(&UARTDEBUG_buf), (u32_t)((void*)Debug_buffer),0);
      break;
    }
    default:
      break;
  }
}

/*******************************************************************************
 Brief    : Debug_init
 Parameter:
 return   :
*******************************************************************************/
void Debug_init(void) {
  fibo_gpio_mode_set(DEBUG_UART_TXD,6);
  fibo_gpio_cfg(DEBUG_UART_TXD,PINDIRECTION_OUT);
  fibo_gpio_set(DEBUG_UART_TXD,PINLEVEL_HIGH);

  fibo_gpio_mode_set(DEBUG_UART_RXD,6);
  fibo_gpio_cfg(DEBUG_UART_RXD,PINDIRECTION_IN);
  fibo_gpio_set(DEBUG_UART_RXD,PINLEVEL_HIGH);
  
  hal_uart_config_t drvcfg;
  fibo_hal_uart_deinit(DEBUG_PORT);
  fibo_taskSleep(1000);
  r_memset(&drvcfg,0,sizeof(hal_uart_config_t));
  drvcfg.baud = DEBUG_PORT_BITRATE;
  drvcfg.parity = HAL_UART_NO_PARITY;
  drvcfg.data_bits = HAL_UART_DATA_BITS_8;
  drvcfg.stop_bits = HAL_UART_STOP_BITS_1;
  drvcfg.rx_buf_size = UART_RX_BUF_SIZE;
  drvcfg.tx_buf_size = UART_TX_BUF_SIZE;
//  drvcfg.cts_enable = false;
//  drvcfg.rts_enable = false;
  drvcfg.recv_timeout = 500;
  fibo_hal_uart_init(DEBUG_PORT, &drvcfg, UARTDEBUG_CallBack, NULL);

#ifdef  EYBOND_DEBUG_ENABLE
  if (UARTDEBUG_buf.payload != NULL) {
    memory_release(UARTDEBUG_buf.payload);
  }
  UARTDEBUG_buf.payload = NULL;
  UARTDEBUG_buf.size = 0;
  UARTDEBUG_buf.lenght = 0;
#endif  
}
/*******************************************************************************
 Brief    : Debug_printf
 Parameter:
 return   :
*******************************************************************************/
#ifdef  EYBOND_DEBUG_ENABLE
void  Debug_buffer(Buffer_t *buf) {
  fibo_hal_uart_put(DEBUG_PORT, (UINT8 *)buf->payload, buf->lenght);
}

/*******************************************************************************
 Brief    : Debug_output
 Parameter:
 return   :
*******************************************************************************/
void Debug_output(u8_t *p, u16_t len) {
#ifdef EYBOND_TRACE_ENABLE
  fibo_textTrace("%s", (UINT8 *)p);
//  OSI_PRINTFI("%s", (UINT8 *)p);
#else
  fibo_hal_uart_put(DEBUG_PORT, (UINT8 *)p, len);
#endif
}

void Print_output(u8_t *p, u16_t len) {
  fibo_hal_uart_put(DEBUG_PORT, (UINT8 *)p, len);
}

void Debug_trace(u8_t *p, u16_t len) {
  OSI_PRINTFI("%s", (UINT8 *)p);
}
#else
void Print_output(u8_t *p, u16_t len) {
  fibo_hal_uart_put(DEBUG_PORT, (UINT8 *)p, len);
}

void Debug_output(u8_t *p, u16_t len) {
  fibo_hal_uart_put(DEBUG_PORT, (UINT8 *)p, len);
}

#endif

/*********************************FILE END*************************************/

