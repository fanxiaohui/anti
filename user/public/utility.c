/******************************************************************************           
* name:                  
* introduce:        
* author:           Luee                                     
******************************************************************************/ 

//#include "ql_stdlib.h"
//#include "ql_trace.h"

//sdk
#include <fibo_opencpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lib
#include "r_stdlib.h"
//app
#include "utility.h"

/*

//check Symbol completeness
static bool Check_Separator(char* pCfgStr)
{
    u32 i=0,j=0,k=0;
    char* pChar1;
    char* pChar2;
    char* pConnCfg = pCfgStr;
    
    while (pChar1 = Ql_strstr(pConnCfg, "<"))
    {
        i++;
        pConnCfg = pChar1 + 1;
    }

    pConnCfg = pCfgStr;
    while (pChar1 = Ql_strstr(pConnCfg, ">"))
    {
        j++;
        pConnCfg = pChar1 + 1;
    }
    
    pConnCfg = pCfgStr;
    while (pChar1 = Ql_strstr(pConnCfg, ">,<"))
    {
        k++;
        pConnCfg = pChar1 + 1;
    }
    
    if (!((i == j)&&(i == k + 1)))
    {
        return FALSE;
    }
    return TRUE;
}
*/
/*****************************************************************************
* Function:     Analyse_Command
*
* Description:
*               Analyse command string
*
* Parameters:
*               src_str:
*                    [in]point to string which need to analyse.
*               symbol_num:
*                    [in]symbol number, the data which want to get in the front
*                        of the symbol.
*               symbol:
*                    [in]symbol ">"
*               dest_buf:
*                    [out]Point to the buffer that save the analysed data .
* Return:
*               None
*****************************************************************************/
/*
s32 Analyse_Command(u8* src_str,s32 symbol_num,u8 symbol, u8* dest_buf)
{
    s32 i = 0;
    u8 *p[30];
    u8 *q;
    s32 result = -1;

    if (!Check_Separator((char*)src_str))
    {
        return result;
    }
    
    if (q = (u8*)Ql_strstr((char*)src_str,"\r\n"))//remove\r\n
    {
        *q = '\0';
    }

    if (!(q = (u8*)Ql_strstr((char*)src_str,"<")))//find first'<'
    {
        return result;
    }
    p[0] = q + 1;//remove first'<'
    
    switch(symbol)
    {
        case '>':
            
            for(i=0;i<symbol_num;i++)
            {
                if (p[i+1] = (u8*)Ql_strstr((char*)p[i],">"))
                {
                    p[i+1] += 3;
                    if (i == symbol_num - 1)
                    {
                        result = 0;
                    }
                }else
                {
                    break;
                }      
            }

            if (!result)
            {
                Ql_strncpy((char*)dest_buf,(const char *)(p[i-1]),p[i]-p[i-1]-3);
            }
            break;

        default:
            result = -1;
            break;
    }

    return result;
}
*/
s32_t Ql_OS_SendMessage(s32_t destTaskId, u32_t msgId, u32_t param1, u32_t param2, u32_t param3) {
  ST_MSG msg;
  u32_t overtime=0;
  r_memset(&msg, 0, sizeof(ST_MSG));
  msg.message = msgId;
  msg.param1 = param1;
  msg.param2 = param2;

  overtime=param3;
  if(param3==0)
    overtime=10000;
  return fibo_queue_put(destTaskId, &msg, overtime);    //Luee 
}
