/***************************Copyright QiYi Team 2016.05*************************
��	����    HashMD5.c
˵	����    MD5�����㷨ͷ�ļ�
�ޡ��ģ�	2016.05.19 ���� Unarty
*******************************************************************************/
#ifndef __HASHMD5__
#define __HASHMD5__

#include "typedef.h"

typedef struct {
    u32_t  len;               /* number of _bits_ handled mod 2^32 */
    u32_t  buf[16];               /* input buffer */
    u32_t  md[4];             /* scratch buffer */
}MD5_t;

void Hash_MD5Init(MD5_t *mdContext);
void Hash_MD5Update(MD5_t *mdContext, const u8_t *inBuf, mcu_t inLen);
void Hash_MD5Final(MD5_t *mdContext);
void Hash_Md5Make(u8_t val[], u8_t *buffer, u32_t len); 

#endif // HashMD5 end
