/***************************Copyright QYKJ   2015-06-18*************************
//
*******************************************************************************/    
#ifndef __TYPEDEF_H
#define __TYPEDEF_H

#ifndef null
#define null				((void*)0)
#endif
#ifndef NULL
#define NULL				((void*)0)
#endif

#define mcu_t				u32_t		

#define MAX(a, b)			((a) > (b) ? (a) : (b))
#define MIN(a, b)			((a) > (b) ? (b) : (a))
#define ABS(a, b)			(((a) > (b)) ? ((a) - (b)) : ((b) - (a)))

#define SIZE(n)             (sizeof(n)/sizeof(n[0]))

#define ERRR(conf, ret)     do               \
                            {                \
                                if (conf)    \
                                {            \
                                    ret;     \
                                }            \
                            } while(0) 



typedef signed long long   s64_t;                            
typedef signed long  s32_t;
typedef signed short s16_t;
typedef signed char  s8_t;

typedef const signed long  sc32_t;  /*!< Read Only */
typedef const signed short sc16_t;  /*!< Read Only */
typedef const signed char  sc8_t;   /*!< Read Only */

typedef  volatile signed long   vs32_t;
typedef  volatile signed short  vs16_t;
typedef  volatile signed char   vs8_t;

typedef volatile const signed long  vsc32_t;  /*!< Read Only */
typedef volatile const signed short vsc16_t;  /*!< Read Only */
typedef volatile const signed char  vsc8_t;   /*!< Read Only */

typedef unsigned long long u64_t;
typedef unsigned long  u32_t;
typedef unsigned short u16_t;
typedef unsigned char  u8_t;

typedef const unsigned long  uc32_t;  /*!< Read Only */
typedef const unsigned short uc16_t;  /*!< Read Only */
typedef const unsigned char  uc8_t;   /*!< Read Only */

typedef volatile unsigned long  vu32_t;
typedef volatile unsigned short vu16_t;
typedef volatile unsigned char  vu8_t;

typedef volatile const unsigned long vuc32_t;  /*!< Read Only */
typedef volatile const unsigned long vuc16_t;  /*!< Read Only */
typedef volatile const unsigned long vuc8_t;   /*!< Read Only */

typedef signed long long   s64;                            
typedef signed long  s32;
typedef signed short s16;
typedef signed char  s8;

typedef const signed long  sc32;  /*!< Read Only */
typedef const signed short sc16;  /*!< Read Only */
typedef const signed char  sc8;   /*!< Read Only */

typedef  volatile signed long   vs32;
typedef  volatile signed short  vs16;
typedef  volatile signed char   vs8;

typedef volatile const signed long  vsc32;  /*!< Read Only */
typedef volatile const signed short vsc16;  /*!< Read Only */
typedef volatile const signed char  vsc8;   /*!< Read Only */

typedef unsigned long long u64;
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef const unsigned long  uc32;  /*!< Read Only */
typedef const unsigned short uc16;  /*!< Read Only */
typedef const unsigned char  uc8;   /*!< Read Only */

typedef volatile unsigned long  vu32;
typedef volatile unsigned short vu16;
typedef volatile unsigned char  vu8;

typedef volatile const unsigned long vuc32;  /*!< Read Only */
typedef volatile const unsigned long vuc16;  /*!< Read Only */
typedef volatile const unsigned long vuc8;   /*!< Read Only */

#ifndef bool
#define bool    u8
#endif

typedef struct
{
    u16_t size;
	u16_t lenght;
	u8_t *payload;
}Buffer_t;

typedef void 	(*void_fun_void)(void); 
typedef u8_t   (*u8_fun_void)(void);
typedef u16_t  (*u16_fun_void)(void);
typedef void 	(*void_fun_u8)(u8_t val);
typedef void 	(*void_fun_u8p_u16)(u8_t *val, u16_t len);
typedef void   (*void_fun_bufp)(Buffer_t *buf);

typedef void (*void_fun_u32_u8p_u32)(u32_t src, u8_t *data, u32_t len);


#define TEST_RF		(1)


#endif /* typedef.h */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

