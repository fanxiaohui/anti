/******************************************************************************           
* name:             grid_tool.h           
* introduce:        头文件
* author:           Luee                                     
******************************************************************************/ 
#ifndef _GRID_TOOL_H_
#define _GRID_TOOL_H_

#include "typedef.h"

/*
#include "eyblib_typedef.h"
#include "eyblib_memory.h"
#include "fibo_opencpu.h"
#include "eyblib_r_stdlib.h"
#include "eyblib_swap.h"
#include "eybpub_utility.h"
#include "eybpub_Debug.h"
#include "eybpub_SysPara_File.h"
#include "eybpub_data_collector_parameter_table.h"
#include "eybpub_parameter_number_and_value.h"
*/

#define GETANTIBUF_FROMACK 1



#define log_d APP_DEBUG

void print_buf(u8_t *buf,u16_t lenght);
void SysPara_Get(u8_t num, Buffer_t *databuf);
int SysPara_Set(u8_t num, Buffer_t *buf);

#endif /* _GRID_TOOL_H_ */