/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：usart.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/31     V0.01    original
************************************************************/
#ifndef __USART_H__
#define __USART_H__

/************************* 头文件 *************************/

#include "HeaderFiles.h"

/************************* 宏定义 *************************/
#define USART_PORT GPIOA
#define USART USART0
#define USART_TX_Pin GPIO_PIN_9
#define USART_RX_Pin GPIO_PIN_10
#define USART_RCU RCU_USART0
#define USART_PIN_RCU RCU_GPIOA

/************************ 变量定义 ************************/

//! 接收数据的数组
extern uint8_t recv_real_buf[512] ;
extern uint16_t recv_real_len ;
//! 接收数据的标记位
extern uint8_t recv_flag ;

/************************ 函数定义 ************************/


/************************ 函数定义 ************************/

void usart_init(void);

void usart_recv_buf(void);

#endif // !__USART_H__
/****************************End*****************************/
