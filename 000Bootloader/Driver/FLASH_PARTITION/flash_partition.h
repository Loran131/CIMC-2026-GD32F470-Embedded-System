#ifndef FLASH_PARTITION_H
#define FLASH_PARTITION_H

#include "gd32f4xx.h" 

/* MCU内部Flash空间划分 */
#define FLASH_BASE_ADDR                  ((uint32_t)0x08000000UL)

/* Bootloader区域 */
#define BOOTLOADER_AREA_BASE_ADDR        FLASH_BASE_ADDR
#define BOOTLOADER_AREA_SIZE             ((uint32_t)0x00010000UL) // 64KB
#define BOOTLOADER_AREA_END_ADDR         (BOOTLOADER_AREA_BASE_ADDR + BOOTLOADER_AREA_SIZE - 1UL)

/* 参数区域 */
#define PARAM_AREA_BASE_ADDR             ((uint32_t)0x08010000UL)
#define PARAM_AREA_SIZE                  ((uint32_t)0x00001000UL) // 4KB
#define PARAM_AREA_END_ADDR              (PARAM_AREA_BASE_ADDR + PARAM_AREA_SIZE - 1UL)

/* App区域 */
#define APP_AREA_BASE_ADDR               ((uint32_t)0x08011000UL)
#define APP_AREA_SIZE                    ((uint32_t)0x00020000UL) // 128KB
#define APP_AREA_END_ADDR                (APP_AREA_BASE_ADDR + APP_AREA_SIZE - 1UL)

/* App备份区域 */
#define APP_BAK_AREA_BASE_ADDR           ((uint32_t)0x08031000UL)
#define APP_BAK_AREA_SIZE                ((uint32_t)0x00020000UL) // 128KB
#define APP_BAK_AREA_END_ADDR            (APP_BAK_AREA_BASE_ADDR + APP_BAK_AREA_SIZE - 1UL)

/* 固件暂存区域 */
#define FW_STAGING_AREA_BASE_ADDR        ((uint32_t)0x08051000UL)
#define FW_STAGING_AREA_SIZE             ((uint32_t)0x00020000UL) // 128KB
#define FW_STAGING_AREA_END_ADDR         (FW_STAGING_AREA_BASE_ADDR + FW_STAGING_AREA_SIZE - 1UL)

#endif /* FLASH_PARTITION_H */
