/**********************    COPYRIGHT 2014-2100, QIYI    ************************ 
 * @File    : log.c
 * @Author  : CGQ
 * @Date    : 2017-08-17
 * @Brief   : 
 ******************************************************************************/
#include "log.h"
#include "FlashFIFO.h"
#include "memory.h"
#include "Swap.h"
#include "r_stdlib.h"
#include "Clock.h"
#include "debug.h"
#include "FlashHard.h"


static FlashFIFOHead_t s_logeHead;

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void log_init(void)
{
    FlashFIFO_init(&s_logeHead, FLASH_LOG_DATA_ADDR, FLASH_LOG_DATA_SIZE);
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void log_clear(void)
{
   FlashFIFO_clear(&s_logeHead);
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void log_saveAbnormal(char const * note, int num)
{
	char str[64];
	int offset;

	offset = r_strcpy(str, note);
	Swap_signedString(&str[offset], num);
	r_stradd(str, ".\r\n");
	log_save(str);
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void log_saveBuf(Buffer_t *buf)
{
    if (buf != null && buf->payload != null)
    {
        char *str = memory_apply(buf->lenght * 3 + 8);
        if (str != null)
        {
            int l = Swap_hexChar(str, buf->payload, buf->lenght, 0);
            log_save(str);
            memory_release(str);
        }	
    }
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
void log_save(char const * note)
{
    Buffer_t buf;
    u16_t offset = r_strlen(note);

    if (buf.lenght < 64 && null != (buf.payload = memory_apply(offset + 32)))
    {
        Clock_t clock;

		Clock_timeZone(&clock);
        offset = Swap_numChar(buf.payload, clock.year);
        buf.payload[offset++] = '-';
        offset += Swap_numChar(&buf.payload[offset], clock.month);
        buf.payload[offset++] = '-';
        offset += Swap_numChar(&buf.payload[offset], clock.day);
        buf.payload[offset++] = ' ';
        
        offset += Swap_numChar(&buf.payload[offset], clock.hour);
        buf.payload[offset++] = ':';
        offset += Swap_numChar(&buf.payload[offset], clock.min);
        buf.payload[offset++] = ':';
        offset += Swap_numChar(&buf.payload[offset], clock.secs);
        buf.payload[offset++] = '>';
        buf.payload[offset++] = ' ';
        
        offset += r_strcpy(&buf.payload[offset], note);
		
        buf.lenght = offset + 1;
		FlashFIFO_put(&s_logeHead, &buf);
        APP_DEBUG("Log:  %s\r\n", buf.payload);
		memory_release(buf.payload);
    }
	else 
	{
        buf.lenght = offset;
        buf.payload = (u8_t*)note;
		FlashFIFO_put(&s_logeHead, &buf);
	}
	
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
u16_t log_get(Buffer_t *buf)
{
    return FlashFIFO_get(&s_logeHead, buf);
}

/*******************************************************************************
 Brief    : void
 Parameter: 
 return   : 
*******************************************************************************/
u16_t log_see(Buffer_t *buf)
{
    return FlashFIFO_see(&s_logeHead, buf);
}

/******************************************************************************/
