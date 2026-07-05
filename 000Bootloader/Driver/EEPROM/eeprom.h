/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：Function.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/
#ifndef __EEPROM_H__
#define __EEPROM_H__

/************************* 头文件 *************************/

#include "HeaderFiles.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

void my_eeprom_init(void);

void EEPROM_WriteStr(uint8_t addr, uint8_t *data, uint16_t len);

void EEPROM_ReadStr(uint8_t addr, uint8_t *data, uint16_t len);

#endif // !__EEPROM_H__

/****************************End*****************************/
