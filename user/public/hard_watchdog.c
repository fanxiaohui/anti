/******************************************************************************           
* name:             enter.c        
* introduce:        user app enter
* author:           Luee                                     
******************************************************************************/ 
//sdk
#include <fibo_opencpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lib

//app
#include "Debug.h"
#include "utility.h"
#include "hard_watchdog.h"

u8_t FeedFlag=0;

void Watchdog_init(void) {    
  FeedFlag = 0;
  s32_t ret = 0;
  ret = fibo_gpio_mode_set(WATCHDOG_PIN, 0);
  ret = fibo_gpio_cfg(WATCHDOG_PIN,PINDIRECTION_OUT);
  ret = fibo_gpio_set(WATCHDOG_PIN,PINLEVEL_HIGH);
}

/*******************************************************************************
 Brief    : Watchdog_feed
 Parameter: 
 return   : 
*******************************************************************************/
void Watchdog_feed(void) {
  if (FeedFlag == 0) {
    s32_t ret = 0;
    ret = fibo_gpio_set(WATCHDOG_PIN,PINLEVEL_LOW);
    fibo_taskSleep(500);  //500ms翻转一次
	ret = fibo_gpio_set(WATCHDOG_PIN,PINLEVEL_HIGH);
  }
}

/*******************************************************************************
 Brief    : Watchdog_stop
 Parameter: 
 return   : 
*******************************************************************************/
void Watchdog_stop(void) {
  int i;
  for (i = 0; i < 6; i++) {
    //Clock_Add();      //Luee 暂时去掉
  }
  FeedFlag = 1;
  s32_t ret = 0;
  ret = fibo_gpio_set(WATCHDOG_PIN,PINLEVEL_HIGH);
  //Clock_save();       //Luee 暂时去掉
  APP_DEBUG("feed stop!!\r\n");
  //log_save("wdt feed stop!!!\r\n");       //Luee 暂时去掉
}