/***************************Copyright QYKJ   2015-06-18*************************
��	����	aLgorithm.h
˵	����	�㷨ʵ�ֺ���
�ޡ��ģ�	2015.06.23 Unarty establish
*******************************************************************************/
#ifndef __ALGORITHM_H
#define __ALGORITHM_H

#include "typedef.h"


#define BINARY_FIND(addr)       (void*)addr, (void*)(&addr + 1), sizeof(addr[0])    //

void *ALG_binaryFind(u16_t arm, void *srcStart, void *srcEnd, u16_t onceSize);

#endif //algorithm.h end
/*******************************Copyright QYKJ*********************************/

