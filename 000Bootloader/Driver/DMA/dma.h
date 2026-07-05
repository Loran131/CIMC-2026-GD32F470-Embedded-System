/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：dma.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/30     V0.01    original
************************************************************/
#ifndef __DMA_H__
#define __DMA_H__

/************************* 头文件 *************************/
#include "HeaderFiles.h"

/************************* 宏定义 *************************/
//!如下是usart2的接收和发送的DMA通道

#define RCU_DMAX RCU_DMA1

#define DMA_USARTX_ADDR (uint32_t)&USART_DATA(USART0)

#define DMA_TRANSFER_X DMA1
#define DMA_TRANSFER_CHANNEL DMA_CH7

#define DMA_RECEIVE_X DMA1
#define DMA_RECEIVE_CHANNEL DMA_CH5

/************************ 变量定义 ************************/


/************************* 函数定义 *************************/
void my_dma_init(void);

#endif // !__DMA_H__
/****************************End*****************************/
