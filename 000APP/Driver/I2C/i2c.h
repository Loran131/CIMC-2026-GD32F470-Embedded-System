/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：Function.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/5     V0.01    original
************************************************************/
#ifndef __I2C_H__
#define __I2C_H__

/************************* 头文件 *************************/

#include "HeaderFiles.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

void my_I2C_Init(void);

uint8_t my_I2C_Send_Byte(uint8_t dat);

uint8_t my_I2C_Read_Byte(void);

void I2C_Start(void);

void I2C_Stop(void);
void IIC_delay(void);
void I2C_Respond(unsigned char ACKSignal);


// void test_send(void);

#endif // !__I2C_H__

/****************************End*****************************/
