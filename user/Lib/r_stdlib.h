/***************************Copyright QYKJ   2015-06-18*************************
文	件：	r_stdlib.h
说	明：	重写C标准库函数
修　改：	2015.06.20 Unarty establish
*******************************************************************************/
#ifndef __R_STDLIB_H
#define __R_STDLIB_H

#include "typedef.h"
                                
void *r_memcpy(void *trg, const void *src, u32_t num);  // 内存考备
u32_t r_memcmp(void *trg,  const void *src, u32_t num);  // 内存比较
void *r_memmove(void *trg,  const void *src, u32_t num);  // 内存移动
void r_memset(void *trg, u8_t const val, u32_t num);  // 内存设置
void *r_memfind(void *trg, u32_t trgLen, const void *src, u32_t srcLen);

u32_t r_strlen(const char *src);
// u32_t r_strcpy(char *trg, const char *src);
char *r_strcpy(char *trg, const char *src);
u32_t r_strcpy_len(char *trg, const char *src);
char *r_strncpy(char *trg, const char *src, u32_t srcLen);
u32_t r_stradd(char *trg, const char *src);
char *r_strcat(char *trg, const char *src);
char *r_strncat(char *trg, const char *src, u32_t srcLen);
u32_t r_strcmp(const char *trg, const char *src);
u32_t r_strncmp(const char *trg, const char *src, u32_t srcLen); // 定长字符串比较
u32_t r_strcmpformat(const char *trg, const char *format);  //**:**:**
int r_strfind(const char *trg, const char *src);
char *r_strstr(const char *trg, const char *src);   // 字符串查找
void r_strSub(char *trg, const char *src, char c);

u16_t r_htons(u16_t n);
u32_t r_htonl(u32_t n);

#endif //r_stdlib.h end
/*******************************Copyright QYKJ*********************************/
							
