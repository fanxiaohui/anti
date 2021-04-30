/******************************************************************************           
* name:                      
* introduce:        
* author:           Luee                                     
******************************************************************************/ 
#ifdef _PLATFORM_L610_
#include "fibo_opencpu.h"
#endif

#include "utility.h"
#include "Status.h"
#include "Debug.h"
//#include "eybpub_SysPara_File.h"
#include "appTask.h"

static char beepCnt = 0;

#ifdef _PLATFORM_L610_
#ifndef BEEP_PORT
#define BEEP_PORT (0)
#endif

#ifndef NET_LED
#define NET_LED (0)
#endif

#ifndef GSM_LED
#define GSM_LED (0)
#endif

#ifndef DEVICE_LED
#define DEVICE_LED (0)
#endif

/*******************************************************************************
 Brief    : void
 Parameter:
 return   :
*******************************************************************************/
/*
void Beep_Init(void) {
  s32_t ret = 0;
  beepCnt = 0;
  fibo_gpio_mode_set(BEEP_PORT, 0);
  fibo_gpio_cfg(BEEP_PORT,PINDIRECTION_OUT);
  fibo_gpio_set(BEEP_PORT,PINLEVEL_HIGH);
  UINT8 nBeep_Level = 0;
  ret = fibo_gpio_get(BEEP_PORT, &nBeep_Level);
//  APP_DEBUG("BEEP PORT GPIO Init ret:%ld direction: %d\r\n", ret, nBeep_Level);
  Beep_On(1);
}
*/
/*******************************************************************************
 Brief    : void
 Parameter:
 return   :
*******************************************************************************/
/*
void Beep_On(char cnt) {
  if (cnt < 10 && cnt > 0) {
    beepCnt = cnt;
    Ql_OS_SendMessage(APP_TASK, APP_CMD_BEEP_ID, 0, 0,0);
  }
}
*/
/*******************************************************************************
 Brief    :void
 Parameter:
 return   :
*******************************************************************************/
void Beep_Run(void) {
  s32_t ret = 0;
  ret = fibo_gpio_set(BEEP_PORT, PINLEVEL_HIGH);
  fibo_taskSleep(500);
  fibo_gpio_set(BEEP_PORT, PINLEVEL_LOW);  
  if (--beepCnt > 0) {
    fibo_taskSleep(500);
    Ql_OS_SendMessage(APP_TASK, APP_CMD_BEEP_ID, 0, 0,0);
  }
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void NetLED_Init(void) {
  s32_t ret = 0;
  fibo_gpio_mode_set(NET_LED, 0);
  fibo_gpio_cfg(NET_LED,PINDIRECTION_OUT);
  fibo_gpio_set(NET_LED,PINLEVEL_HIGH);
  NetLED_On();
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void NetLED_On(void) {
  s32_t ret = 0;
  fibo_gpio_set(NET_LED,PINLEVEL_LOW);
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void NetLED_Off(void) {
  s32_t ret = 0;
  fibo_gpio_set(NET_LED,PINLEVEL_HIGH);
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void GSMLED_Init(void) {
  s32_t ret = 0;
  fibo_gpio_mode_set(GSM_LED, 0);
  fibo_gpio_cfg(GSM_LED,PINDIRECTION_OUT);
  fibo_gpio_set(GSM_LED,PINLEVEL_HIGH);
  GSMLED_On();
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void GSMLED_On(void) {
  s32_t ret = 0;
  fibo_gpio_set(GSM_LED,PINLEVEL_LOW);
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void GSMLED_Off(void) {
  s32_t ret = 0;
  fibo_gpio_set(GSM_LED,PINLEVEL_HIGH);
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void deviceLEDInit(void) {
  s32_t ret = 0;
  fibo_gpio_mode_set(DEVICE_LED, 1);
  fibo_gpio_cfg(DEVICE_LED,PINDIRECTION_OUT);
  fibo_gpio_set(DEVICE_LED,PINLEVEL_HIGH);
  deviceLEDOn();
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void deviceLEDOn(void) {
  s32_t ret = 0;
  fibo_gpio_set(DEVICE_LED,PINLEVEL_LOW);
}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
*******************************************************************************/
void deviceLEDOff(void) {
  s32_t ret = 0;
  fibo_gpio_set(DEVICE_LED,PINLEVEL_HIGH);
}

#endif
/******************************************************************************/
