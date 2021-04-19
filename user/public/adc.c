/**
 * @file adc.c
 * @brief 
 * @author Derek (dongjiemail@163.com)
 * @date 2020年1月15日
 * @copyright Copyright (c) 2019
 * @note 
 */

#include "Ql_system.h"
#include "log.h"
#include "Debug.h"
#include "ql_adc.h"
#include "eybond.h"
#include "memory.h"

u8_t g_ret_flag = 1;

static void callBackADCSampling(Enum_ADCPin adcPin, u32 adcValue, void *customParam)
{
    static int count_1 = 0;
    static int count_2 = 0;

    APP_DEBUG("\r\nadcValue is %d\r\n",adcValue);

    if((adcValue < 700)&&(1 == g_ret_flag)&&(0==(++count_1)%3))
    {
        APP_DEBUG("power_interrupted\r\n");
        specialData_send();
        count_1 = 0;
    }

    if((adcValue > 700)&&(0==(++count_2)%3))
    {
        g_ret_flag = 1;
        count_2 = 0;
    }
}

void ADC_Init(void)
{
    APP_DEBUG("ADC init...\r\n");
    Ql_ADC_Register(PIN_ADC0, callBackADCSampling ,NULL);
    Ql_ADC_Init(PIN_ADC0, 5, 100);
    Ql_ADC_Sampling(PIN_ADC0, TRUE);
}
