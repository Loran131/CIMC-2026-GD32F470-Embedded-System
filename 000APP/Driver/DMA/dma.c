/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：dma.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/30     V0.01    original
************************************************************/


/************************* 头文件 *************************/
#include "dma.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/
//! 接收数据的数组
extern uint8_t recv_real_buf[512] ;
extern uint16_t recv_real_len ;
//! 接收数据的标记位
extern uint8_t recv_flag ;
uint8_t g_transfer_buf[128];
//extern uint8_t recv_real_buf[512];

/************************ 函数定义 ************************/
void dma_transfer_config(void);
void dma_receive_config(void);

/************************************************************
 * Function :       my_dma_init
 * Comment  :       用于初始化DMA
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-30 V0.1 original
************************************************************/
void my_dma_init(void)
{
	rcu_periph_clock_enable(RCU_DMAX);	// 使能DMA1时钟
	dma_transfer_config();	// 初始化DMA传输通道
	dma_receive_config();	// 初始化DMA接收通道
}
/************************************************************
 * Function :       dma_transfer_config
 * Comment  :       用于初始化DMA传输通道
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-30 V0.1 original
************************************************************/
void dma_transfer_config(void)
{
	dma_single_data_parameter_struct dma_init_struct;
	/* enable DMA0 */

	/* deinitialize DMA channel6(USART1 tx) */
	dma_deinit(DMA_TRANSFER_X , DMA_TRANSFER_CHANNEL); // Tx对应DMA1 通道7
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;	//内存到外设
	dma_init_struct.memory0_addr = (uint32_t)g_transfer_buf;	// 内存地址为传输缓冲区
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;	// 内存地址增加
	dma_init_struct.number = sizeof(g_transfer_buf);	// 传输数据数量为传输缓冲区大小
	dma_init_struct.periph_addr = DMA_USARTX_ADDR;	// USART0发送数据寄存器地址
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;	// 外设地址不增加
	dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;	// 外设数据宽度为8位
	dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;	// 优先级
	dma_single_data_mode_init(DMA_TRANSFER_X , DMA_TRANSFER_CHANNEL , &dma_init_struct);	// 初始化DMA传输通道
	dma_channel_subperipheral_select(DMA_TRANSFER_X , DMA_TRANSFER_CHANNEL , DMA_SUBPERI4);	// DMA通道选择
	// configure DMA mode
	dma_circulation_disable(DMA_TRANSFER_X , DMA_TRANSFER_CHANNEL);	// 循环模式禁用
}
/************************************************************
 * Function :       dma_receive_config
 * Comment  :       用于初始化DMA接收通道
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-30 V0.1 original
************************************************************/
void dma_receive_config(void)
{
    dma_single_data_parameter_struct dma_parameter;

    dma_deinit(DMA_RECEIVE_X , DMA_RECEIVE_CHANNEL); // Rx对应DMA1 通道5
    dma_parameter.direction = DMA_PERIPH_TO_MEMORY;  // 外设到内存
    dma_parameter.periph_addr = DMA_USARTX_ADDR;     // USART 接收寄存器地址
    dma_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_parameter.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_parameter.memory0_addr = (uint32_t)recv_real_buf;         // 指向接收缓冲区
    dma_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_parameter.number = sizeof(recv_real_buf);                 // 接收字节数
    dma_parameter.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_parameter.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    dma_single_data_mode_init(DMA_RECEIVE_X , DMA_RECEIVE_CHANNEL , &dma_parameter);

    dma_channel_subperipheral_select(DMA_RECEIVE_X , DMA_RECEIVE_CHANNEL , DMA_SUBPERI4);
    dma_channel_enable(DMA_RECEIVE_X , DMA_RECEIVE_CHANNEL);
}
/****************************End*****************************/
