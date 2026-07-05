/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "sdcard.h"
#include <stdio.h>
#include "SPI_FLASH\SPI_FLASH.h"
/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/*-----------------------------------------------------------------------*/
#define BLOCKSIZE   512
#define BUSMODE_4BIT
#define DMA_MODE

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE drv) {
    DSTATUS status = STA_NOINIT;
    
    switch (drv) {
        case 0: // SD卡物理驱动器0
        {
            sd_error_enum ret;
            sd_card_info_struct sd_cardinfo;
            uint32_t cardstate = 0;
            
            ret = sd_init();
            if (SD_OK != ret) break;
            
            ret = sd_card_information_get(&sd_cardinfo);
            if (SD_OK != ret) break;
            
            ret = sd_card_select_deselect(sd_cardinfo.card_rca);
            if (SD_OK != ret) break;
            
            ret = sd_cardstatus_get(&cardstate);
            if (SD_OK != ret) break;
            
            if (cardstate & 0x02000000) {
                // 卡处于错误状态，需要复位
                break;
            }
            
            // 配置总线模式
#ifdef BUSMODE_4BIT
            ret = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
#else
            ret = sd_bus_mode_config(SDIO_BUSMODE_1BIT);
#endif
            if (SD_OK != ret) break;
            
            // 配置传输模式
#ifdef DMA_MODE
            ret = sd_transfer_mode_config(SD_DMA_MODE);
#else
            ret = sd_transfer_mode_config(SD_POLLING_MODE);
#endif
            if (SD_OK == ret) {
                status = 0; // 初始化成功
            }
            break;
        }
        
        case 1: // SPI Flash物理驱动器1
        { 		spi_flash_init();
							uint32_t flash_id = spi_flash_read_id();
				    	if (flash_id == SFLASH_ID) {
                status = 0; // 初始化成功
            }
					    break;
        }
        
        default:
            status = STA_NOINIT;
            break;
    }
    
    return status;
}


/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE drv         /* Physical drive nmuber (0..) */
)
{
    switch (drv) {
        case 0: // SD卡
        case 1: // SPI Flash
            return 0; 
        default:
            return STA_NOINIT;}
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE drv,          /* Physical drive nmuber (0..) */
    BYTE *buff,        /* Data buffer to store read data */
    DWORD sector,      /* Sector address (LBA) */
    BYTE count         /* Number of sectors to read (1..255) */
)
{     switch (drv) {
     case 0: // SD卡读取
				{sd_error_enum status = SD_ERROR;
           if(NULL == buff){
								return RES_PARERR;
						}
						if(!count){
								return RES_PARERR;
						}

						if(0 == drv){
								if(1 == count){
										status = sd_block_read((uint32_t *)(&buff[0]), (uint32_t)(sector<<9), BLOCKSIZE);
								}else{
										status = sd_multiblocks_read((uint32_t *)(&buff[0]), (uint32_t)(sector<<9), BLOCKSIZE, (uint32_t)count);
								}
						}
						if(SD_OK == status){
								return RES_OK;
						}
						return RES_ERROR;}
		case 1: // SPI Flash读取
            for (UINT i = 0; i < count; i++) {
                uint32_t addr = (sector + i) * 4096; 
                spi_flash_read_byte();
            }
            return RES_OK;
            
        default:
            return RES_PARERR;
}
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/* The FatFs module will issue multiple sector transfer request
/  (count > 1) to the disk I/O layer. The disk function should process
/  the multiple sector transfer properly Do. not translate it into
/  multiple single sector transfers to the media, or the data read/write
/  performance may be drasticaly decreased. */

#if _READONLY == 0
DRESULT disk_write (
    BYTE drv,            /* Physical drive nmuber (0..) */
    const BYTE *buff,    /* Data to be written */
    DWORD sector,        /* Sector address (LBA) */
    BYTE count           /* Number of sectors to write (1..255) */
)
{   switch (drv) {
        case 0: // SD卡写入
		{
    sd_error_enum status = SD_ERROR;
    if(NULL == buff){
        return RES_PARERR;
    }
    if(!count){
        return RES_PARERR;
    }

    if(0 == drv){
        if(1 == count){
            status = sd_block_write((uint32_t *)buff, sector<<9, BLOCKSIZE);
        }else{
            status = sd_multiblocks_write((uint32_t *)buff, sector<<9, BLOCKSIZE, (uint32_t)count);
        }
    }
    if(SD_OK == status){
        return RES_OK;
    }
    return RES_ERROR;
}case 1: // SPI Flash写入
     {uint32_t addr = sector * SPI_FLASH_SECTOR_SIZE;
    uint32_t bytes = count * SPI_FLASH_SECTOR_SIZE;
    spi_flash_sector_erase( FLASH_WRITE_ADDRESS);
    for(uint32_t i = 0; i < count; i++) {
        uint32_t sec_addr = (sector + i) * SPI_FLASH_SECTOR_SIZE;
        
        // 擦除扇区
        spi_flash_sector_erase(sec_addr);
        
        // 写入数据
        spi_flash_buffer_write((uint8_t*)buff + (i * SPI_FLASH_SECTOR_SIZE), 
                               sec_addr, SPI_FLASH_SECTOR_SIZE);
    }
    
    return RES_OK;
    }
   default:
            return RES_ERROR;}}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE drv,         /* Physical drive nmuber (0..) */
    BYTE ctrl,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    
   if(drv !=1) return RES_PARERR;
    
    switch(ctrl) {
        case GET_SECTOR_SIZE:
            *(WORD*)buff = SPI_FLASH_SECTOR_SIZE;    // 扇区大小 4096
            return RES_OK;
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;                   // 擦除块大小（以扇区为单位）
            return RES_OK;
        case CTRL_SYNC:
            spi_flash_wait_for_write_end();      // 等待写操作完成
            return RES_OK;
        default:
            return RES_PARERR;
    }
}
 
/*-----------------------------------------------------------------------*/
/* Get current time                                                      */
/*-----------------------------------------------------------------------*/ 
DWORD get_fattime(void)
{

  return 0;

}
