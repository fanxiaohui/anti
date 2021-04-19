/**************Copyright(C)，2015-2026，Shenzhen Eybond Co., Ltd.***************
  *@brief   : TransDevice.c  Eybond transparent transmission monitor
  *@notes   : 2017.12.28 CGQ   
*******************************************************************************/
#include "TransDevice.h"
#include "list.h"
#include "r_stdlib.h"
#include "Device.h"
#include "protocol.h"


static const TransCfgTab_t TransCfgTab[] = {
	{"07", &UART_9600_N1},
	{"A3", &UART_9600_E1},
};

static u8_t callback(Device_t *dev);


/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void TransDevice_init(char *str, ST_UARTDCB *cfg)
{
	int i;
	Device_t * dev;

	for (i = 0; i < sizeof(TransCfgTab)/sizeof(TransCfgTab[0]); i++)
	{
		if (r_strfind(str, TransCfgTab[i].narration) >= 0)
		{
			break;
		}
	}//*/
	
	if (i >= sizeof(TransCfgTab)/sizeof(TransCfgTab[0]))
	{
		i = 0;
	}
	
	dev = list_nodeApply(sizeof(Device_t));
	dev->type = DEVICE_TRANS;
	if (cfg == null)
	{
    	dev->cfg = TransCfgTab[i].cfg;
	}
	else
	{
		dev->cfg = cfg;
	}
    dev->callBack = callback;
    dev->explain = null;
    list_init(&dev->cmdList);
	
    Device_add(dev); 
}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
void TransDevice_clear(void)
{

}

/*******************************************************************************
  * @brief  
  * @param  None
  * @retval None
*******************************************************************************/
static u8_t  callback(Device_t *dev)
{
	return 0;
}

/******************************************************************************/

