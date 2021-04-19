/**************Copyright(C)��2015-2026��QIYI Temp    Co., Ltd.******************
  *@brief   : file option
  *@notes   : 2017.05.12 CGQ ����   
*******************************************************************************/
#include "File.h"
#include "r_stdlib.h"
#include "x25qxx.h"
#include "HashMD5.h"
#include "crc.h"
#include "memory.h"
#include "Debug.h"
#include "watchdog.h"
#include "log.h"

#define FILE_FLAG	(0xAA550088)

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
File_t *File_init(u32_t addr, u16_t sliceCnt, u16_t sliceSize)
{
    File_t *file;
    u16_t len;
    
    len = (sliceCnt>>3) + ((sliceCnt&0x07) ? 1 : 0);
    file = memory_apply(sizeof(File_t) + len + 8);
    
    if (file != null)
    {
        int i;

        file->seat = 0;
        file->addr = addr;
		file->flag = FILE_FLAG;
        file->sliceCnt = sliceCnt;
        file->sliceSize = sliceSize;
        file->sliceStateLen = len;
        r_memset(file->sliceState, 0, len);
        
        for (i = (file->sliceCnt&0x07); i < 8 && i != 0; i++)
        {
            file->sliceState[len - 1] |= (1<<i);
        }
    }
    
    return file;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
u8_t File_rcve(File_t *file, u16_t offset, u8_t *data, u16_t len)
{
    if (((file->sliceSize == len) || ((offset + 1) == file->sliceCnt ))
        && (file->sliceCnt > offset)
    )
    {
        u32_t saveAddr = file->addr + (offset * file->sliceSize);
       
        file->sliceState[offset>>3] |= (1<<(offset&0x07));
        
        if (0 == (saveAddr&(X25Q_SECTOR_SIZE - 1)))
        {
			APP_DEBUG("Save area earse addr: %x \r\n", saveAddr);
            x25Qxx_earse(saveAddr);
        }
      	x25Qxx_wrtie(saveAddr, len, data);
        
        return 0;
    }
    
    APP_DEBUG("file size %d - %d, slice %d - %d  \r\n ", file->sliceSize, len, file->sliceCnt, offset);
    return 1;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
s16_t File_read(File_t *file, u8_t *data, u16_t len)
{
	static MD5_t *md5 = null;
    int readLen;

	if (file->seat == 0)
	{
		memory_release(md5);
		md5 = memory_apply(sizeof(MD5_t));
		Hash_MD5Init(md5);
		APP_DEBUG("File Satrt read\r\n");
	}
    readLen = (file->size - file->seat) > len ? len : (file->size - file->seat);

    x25Qxx_read(file->addr + file->seat, readLen, data);
    file->seat += readLen;

	if (md5 != null)
	{
		Hash_MD5Update(md5, data, readLen);
		if (file->size == file->seat)
		{
			Hash_MD5Final(md5);
	    
	    	if (r_memcmp(file->md5, md5->md, 16) != 0)
	    	{
				readLen = -1;
				log_save("File read rusult fail\r\n");
	    	}
			memory_release(md5);
			md5 = null;
		}

		if (0 == (file->seat&0x001FFFF))
		{
			Watchdog_feed();
		}
	}
    else 
    {
        readLen = 0;
        log_save("MD5 memory apply fail");
    }
	
    return readLen;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
u8_t File_Check(File_t *file)
{
    MD5_t *md5;
    u8_t *buf;
    int i;
    u16_t len;

	//Watchdog_feed();
    md5 = memory_apply(sizeof(MD5_t));
    buf = memory_apply(X25Q_READ_SIZE);
    
    Hash_MD5Init(md5);
    
    for (i = 0; i < file->size; i += len)
    {
        len = (file->size - i) > X25Q_READ_SIZE ? X25Q_READ_SIZE : (file->size - i);
        
        x25Qxx_read(file->addr + i, len, buf);
        Hash_MD5Update(md5, buf, len);
		if (0 == (i&0x001FFFF))
		{
			Watchdog_feed();
		}
    }
    
    Hash_MD5Final(md5);
	
    i = (r_memcmp(file->md5, md5->md, 16) == 0) ? 0 : 1;
    memory_release(md5);
    memory_release(buf);
    
    return i;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void File_state(File_t *file, u32_t addr)
{
    x25Qxx_read(addr, sizeof(File_t), (u8_t*)file);
	file->seat = 0;
}

/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
u8_t File_validCheck(File_t *file)
{
    if ((file != null)
		&& (file->flag == FILE_FLAG)
		//&& (file->addr < 0x100000)
		&& (file->size < 0x400000)
	)
    {
		return 0;
    }
	return 1;
}


/*******************************************************************************
  * @brief  
  * @note   None
  * @param  None
  * @retval None
*******************************************************************************/
void File_save(File_t *file, u32_t addr)
{
    x25Qxx_earse(addr);
    x25Qxx_wrtie(addr, sizeof(File_t), (u8_t*)file);
}

/******************************************************************************/

