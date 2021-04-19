/*******************************************************************************
 * 锟斤拷 锟侥硷拷锟斤拷锟斤拷	list.h
 * 锟斤拷 锟斤拷锟斤拷	锟斤拷	锟斤拷锟斤拷锟斤拷锟斤拷头锟侥硷拷
 * 锟斤拷 锟斤拷锟斤拷锟竭ｏ拷	UnartyChen
 * 锟斤拷 锟芥本	锟斤拷	V1.0
 * 锟斤拷 锟斤拷锟斤拷	锟斤拷	2015.03.13
 * 锟斤拷 锟睫革拷	锟斤拷  锟斤拷锟斤拷	
 * ****************************************************************************/
#ifndef __LIST_H
#define	__LIST_H

#include "typedef.h"

typedef struct node_t
{
	struct node_t *front;
	struct node_t *next;
	u8_t   payload[];
}Node_t;


typedef struct
{
    s16_t   count;  
    u16_t   rsv;
	Node_t	*node;
}ListHandler_t;

typedef u8_t (*cmpFun)(void *src, void *dest);	//锟斤拷锟捷比较猴拷锟斤拷
typedef u8_t (*processorFun)(void *listPaycoad, void *chagePoint);			//锟斤拷锟捷憋拷锟斤拷


void    list_init(ListHandler_t *head);
int 	list_topInsert(ListHandler_t *head, void *payload);
int 	list_bottomInsert(ListHandler_t *head, void *payload);
int 	list_ConfInsert(ListHandler_t *head, cmpFun cmp, void *payload);
void 	*list_nodeApply(mcu_t payloadSize);
void    *list_nextData(ListHandler_t *head, void *payload); // get next node data
void 	*list_find(ListHandler_t *head, cmpFun cmp, void *conVal);

void    list_trans(ListHandler_t *head, processorFun fun, void *optPoint);
void 	list_nodeDelete(ListHandler_t *head, void *payload);
void	list_delete(ListHandler_t *head);



#endif//list.h end 
/******************************************************************************/

