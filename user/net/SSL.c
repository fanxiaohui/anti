/**********************    COPYRIGHT 2014-2100,  Eybond ************************ 
 * @File    : SSL.c
 * @Author  : CGQ
 * @Date    : 2018-10-25
 * @Brief   : 
 ******************************************************************************/
#include "SSL.h"
#include "custom_feature_def.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_sim.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_system.h"
#include "ql_trace.h"
#include "Debug.h"
#include "memory.h"
#include "Syspara.h"
#include "file.h"
#include "r_stdlib.h"
#include "log.h"
#include "Clock.h"


M26Net_t *SSLNet;


/*******************************************************************************
 Brief    : SSL Ca file write
 Parameter: 
 return   : 
*******************************************************************************/
static s32 CAFileWrite_Handler(char* line, u32 len, void* userData)
{
    Buffer_t *buf = userData;
    char *head = null;

    APP_DEBUG(line);
    head = Ql_RIL_FindString(line, len, "CONNECT"); //continue wait
    if (head)
    {
        Ql_RIL_WriteDataToCore(buf->payload, buf->lenght);
        return RIL_ATRSP_CONTINUE;
    }
    head = Ql_RIL_FindString(line, len, "OK"); 
    if (head)
    {
        return RIL_ATRSP_SUCCESS;
    }
    head = Ql_RIL_FindString(line, len, "ERROR"); 
    if (head)
    {
        return RIL_ATRSP_FAILED;
    }
    head = Ql_RIL_FindString(line, len, "+CME ERROR"); 
    if (head)
    {
        return RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE;
}

int SSL_CAFileWrite(void)
{
    char *at;
    Buffer_t buf;
    int ret = RIL_AT_FAILED;
    File_t *file = memory_apply(sizeof(File_t));

    File_state(file, CA_FILE_ADDR);

    if ((file->size > 0xFFFF)
        || (File_Check(file) != 0))
    {
        log_save("CA file get fail\r\n");
        return;
    } 
    at = memory_apply(200);
    r_memset(at, 0, 200);
    buf.size = file->size;
    buf.payload = memory_apply(buf.size);
    buf.lenght = File_read(file, buf.payload, buf.size);
    Ql_sprintf(at, "AT+QSECWRITE=\"RAM:CA.crt\",%d,%d\r\n", file->size, 100);
    APP_DEBUG(at);
    ret = Ql_RIL_SendATCmd(at, r_strlen(at),  CAFileWrite_Handler, &buf, 0);
    memory_release(buf.payload);
    memory_release(at);
    memory_release(file);
    if (ret != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal("CA file write fail :", ret);
    }
    return ret;
}

static s32 CfgAck_Handler(char* line, u32 len, void* userData)
{
    char *head;
    
    APP_DEBUG(line);

    head = Ql_RIL_FindString(line, len, "OK"); //continue wait
    if (head)
    {
        return RIL_ATRSP_SUCCESS;
    }

    head = Ql_RIL_FindString(line, len, "ERROR"); 
    if (head)
    {
        return RIL_ATRSP_FAILED;
    }
    head = Ql_RIL_FindString(line, len, "+CME ERROR"); 
    if (head)
    {
        return RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE;
}

static s32 CAFileRead_Handler(char* line, u32 len, void* userData)
{
    char *head = null; 

    APP_DEBUG(line);

    head = Ql_RIL_FindString(line, len, "OK"); //continue wait
    if (head)
    {
        return RIL_ATRSP_SUCCESS;
    }
    head =  Ql_RIL_FindString(line, len, "+CME ERROR:"); //continue wait
    if (head)
    {
        return RIL_ATRSP_FAILED;
    }
    

    return RIL_ATRSP_CONTINUE;
}

int SSL_modeCfg(void)
{
    int result;
    static const char cfg0[] = "AT+QIFGCNT=1\r\n";
    static const char cfg1[] = "AT+QIMUX=1\r\n";
    static const char cfg2[] = "AT+QIREGAPP\r\n";

    APP_DEBUG(cfg0);
    result = Ql_RIL_SendATCmd(cfg0, sizeof(cfg0), CfgAck_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal(cfg0, result);
        return result;
    }
    APP_DEBUG(cfg1);
    result = Ql_RIL_SendATCmd(cfg1, sizeof(cfg1), CfgAck_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal(cfg1, result);
        return result;
    }
    APP_DEBUG(cfg2);
    result = Ql_RIL_SendATCmd(cfg2, sizeof(cfg2), CfgAck_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal(cfg2, result);
        return result;
    }

    return result;
}

void SSL_init(void)
{
    static const char cfg1[] = "AT+QSSLCFG=\"sslversion\",0,4\r\n";
    static const char cfg3[] = "AT+QSSLCFG=\"ciphersuite\",1,\"0X0035\"\r\n";
    static const char cfg2[] = "AT+QSSLCFG=\"seclevel\",0,1\r\n";
    static const char cfg4[] = "AT+QSSLCFG=\"ignorertctime\",1\r\n";
    static const char readCA[] = "AT+QSECREAD=\"RAM:CA.crt\"\r\n";
    static const char cfg5[] = "AT+QSSLCFG=\"cacert\",0,\"RAM:CA.crt\"\r\n";
    int result;

    SSLNet = null;
    result = Ql_RIL_SendATCmd(readCA, sizeof(readCA), CAFileRead_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        result = SSL_CAFileWrite();
        if (result != RIL_ATRSP_SUCCESS)
        {
            log_saveAbnormal("CA file write fail ", result);
            return ;
        }
    }

    result = SSL_modeCfg();
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal("Config AT mode fail ", result);
        return;
    }

    APP_DEBUG(cfg1);
    result = Ql_RIL_SendATCmd(cfg1, sizeof(cfg1), CfgAck_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal("CA cfg1 write fail ", result);
        return;
    }
    APP_DEBUG(cfg2);
    result = Ql_RIL_SendATCmd(cfg2, sizeof(cfg2), CfgAck_Handler,null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal("CA cfg2 write fail ", result);
        return;
    }
    APP_DEBUG(cfg3);
    result = Ql_RIL_SendATCmd(cfg3, sizeof(cfg3), CfgAck_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal("CA cfg3 write fail ", result);
        return;
    }
    APP_DEBUG(cfg4);
    result = Ql_RIL_SendATCmd(cfg4, sizeof(cfg4), CfgAck_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal("CA cfg4 write fail ", result);
        return;
    }
    APP_DEBUG(readCA);
    result = Ql_RIL_SendATCmd(readCA, sizeof(readCA), CAFileRead_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal("CA file read fail ", result);
        return;
    }
    APP_DEBUG(cfg5);
    result = Ql_RIL_SendATCmd(cfg5, sizeof(cfg5), CfgAck_Handler, null, 0);
    if (result != RIL_ATRSP_SUCCESS)
    {
        log_saveAbnormal("CA cfg5 write fail ", result);
        return;
    }
}

int SSL_Open(M26Net_t *net)
{
    int ret;
    char *at;

    SSLNet = net;
    at = memory_apply(100);

    r_memset(at, 0, 100);

    Ql_sprintf(at, "AT+QSSLOPEN=%d,0,\"%d.%d.%d.%d\",%d,0\r\n", SSLNet->socketID, 
                                                                (SSLNet->ip >> 0)&(0xFF),
                                                                (SSLNet->ip >> 8)&(0xFF),
                                                                (SSLNet->ip >> 16)&(0xFF), 
                                                                (SSLNet->ip >> 24)&(0xFF), 
                                                                SSLNet->port);
    
    ret = Ql_RIL_SendATCmd(at, r_strlen(at), CfgAck_Handler, null, 0);
    APP_DEBUG("%s, %d\r\n", at, ret);
    memory_release(at);
    
    return ret;
}

/*******************************************************************************
 Brief    : SSL Ca file write
 Parameter: 
 return   : 
*******************************************************************************/
static s32 dataSend_Handler(char* line, u32 len, void* userData)
{
    Buffer_t *buf = userData;
    char *head = null;

    APP_DEBUG(line);
    head = Ql_RIL_FindString(line, len, ">"); //continue wait
    if (head)
    {
        Ql_RIL_WriteDataToCore(buf->payload, buf->lenght);
        return RIL_ATRSP_CONTINUE;
    }
    head = Ql_RIL_FindString(line, len, "SEND OK"); 
    if (head)
    {
        return RIL_ATRSP_SUCCESS;
    }
    head = Ql_RIL_FindString(line, len, "ERROR"); 
    if (head)
    {
        return RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE;
}

int SSL_Send(s32_t socketId, u8_t* pData, s32_t dataLen)
{
    char *at;
    int ret;
    int offset;
    Buffer_t buf;

    at = memory_apply(100);
    if (at != null)
    {
        offset = r_strcpy(at, "AT+QSSLSEND=");

        Swap_numChar(&at[offset], socketId);
        offset = r_stradd(at, ",");
        Swap_numChar(&at[offset], dataLen);
        buf.lenght = dataLen;
        buf.payload = pData;
        APP_DEBUG(at);
        ret =  Ql_RIL_SendATCmd(at, r_strlen(at),  dataSend_Handler, &buf, 0);

        memory_release(at);

        if (ret == RIL_ATRSP_SUCCESS)
        {
            ret = dataLen;
        }
    }
    else
    {
        APP_DEBUG("Memory full\r\n");
    }

    return ret;
}

/*******************************************************************************
  * @note   SLL TCP connect callbback
  * @param  None
  * @retval None
*******************************************************************************/
void SSLHandler_TCPConnect(const char* strURC, void* reserved)
{
    static int reconnect_count = 0;
    APP_DEBUG(strURC);
    if (r_strfind("0", strURC)>=0)
    {
        SSLNet->status = M26_SUCCESS;
    }
    else if (r_strfind("-1", strURC)>=0)
    {
        SSLNet->status = M26_CONNECT_FAIL;
        log_save("SSL Connect Fail\r\n");
        if(0 == (runTimeCheck(5, 21))&&(0==(++reconnect_count)%6))
        {
            reconnect_count = 0;
            log_save("feed stop!!\r\n");	
            Watchdog_stop();
        }
    }
}

/*******************************************************************************
 Brief    : SSL Ca file write
 Parameter: 
 return   : 
*******************************************************************************/
static s32 dataRcve_Handler(char* line, u32 len, void* userData)
{
    static Buffer_t dataBuf = {0};
    char *head = null;
    int offset;

    APP_DEBUG(line);
    if ((offset = r_strfind("QSSLRECV", line)) >= 0)
    {
        offset += r_strfind("TCP,", (char*)&line[offset]);
        dataBuf.size = Swap_charNum((char*)&line[offset + 4]);
        dataBuf.lenght = 0;
        dataBuf.payload = memory_apply(dataBuf.size);
        offset += r_strfind("\r\n", (char*)&line[offset]);
        offset += r_strlen("\r\n");

        return RIL_ATRSP_CONTINUE;
    }
    else if (dataBuf.payload != null)
    {
        offset = dataBuf.size - dataBuf.lenght;
        offset = offset > len ? len : offset;
        r_memcpy(&dataBuf.payload[dataBuf.lenght], line, offset);
        dataBuf.lenght += offset;
        if (dataBuf.lenght == dataBuf.size)
        {
            u8_t  port;

            M26Socket(SSLNet->socketID, &port);
            SSLNet->callback(port, &dataBuf);
            memory_release(dataBuf.payload);
            dataBuf.payload = null;
            return RIL_ATRSP_SUCCESS;
        }
    
        return RIL_ATRSP_CONTINUE;
    }
    else if (r_strfind("OK", line) >= 0)
    {
        memory_release(dataBuf.payload);
        dataBuf.payload = null;
        return RIL_ATRSP_SUCCESS;
    }

    head = Ql_RIL_FindString(line, len, "ERROR"); 
    if (head)
    {
        memory_release(dataBuf.payload);
        dataBuf.payload = null;
        return RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE;
}

/*******************************************************************************
  * @note   SLL data rcveice Callback
  * @param  None
  * @retval None
*******************************************************************************/
void SSLHandler_DataRcve(const char* strURC, void* reserved)
{
    int offset;
    APP_DEBUG(strURC);

    if ((offset = r_strfind("recv", strURC)) >= 0)
    {
        char *at;
        char str[8];
        
        offset += r_strfind(",", (char*)&strURC[offset]);
        at = (char*)&strURC[offset + 1];
        offset = r_strfind("\r\n", (char*)&strURC[offset + 1]);
        r_memset(str, 0, sizeof(str));
        r_memcpy(str, at, offset);

        at = memory_apply(100);
        offset = r_strcpy(at, "AT+QSSLRECV=");
        r_stradd(at, str);
        r_stradd(at, ",1500");
        APP_DEBUG(at);
        Ql_RIL_SendATCmd(at, r_strlen(at),  dataRcve_Handler, null, 0);

        memory_release(at);
    }
    else if ((offset = r_strfind("closed", strURC)) >= 0)
    {
        log_save(strURC);
        SSLNet->status = M26_CLOSE;
    }
}
/*********************************FILE END*************************************/

