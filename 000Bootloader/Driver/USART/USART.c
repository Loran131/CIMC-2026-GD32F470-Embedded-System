/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：usart.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/30     V0.01    original
************************************************************/


/************************* 头文件 *************************/
#include "usart.h"
#include "DMA/dma.h"

/************************* 宏定义 *************************/


//! 接收数据的数组
uint8_t recv_real_buf[512] = { 0 };
uint16_t recv_real_len = 0;
//! 接收数据的标记位
uint8_t recv_flag = 0;

/************************ 函数定义 ************************/

void usart_dma_send(uint8_t* buf, uint16_t len);

/************************************************************
 * Function :       usart_init
 * Comment  :       用于初始化USART
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-30 V0.1 original
************************************************************/
void usart_init(void)
{
    //! 首先开启对应的中断处理函数     
    nvic_irq_enable(USART0_IRQn, 3, 2);

    //! 使能USART时钟
    rcu_periph_clock_enable(USART_RCU);
    rcu_periph_clock_enable(USART_PIN_RCU);

    // 对TX配置成为复用模式
    gpio_af_set(USART_PORT, GPIO_AF_7, USART_TX_Pin | USART_RX_Pin);

    // 设置GPIO引脚模式
    gpio_mode_set(USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART_TX_Pin | USART_RX_Pin);
    // 设置GPIO引脚参数
    gpio_output_options_set(USART_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART_TX_Pin | USART_RX_Pin);

    // 配置USART
    usart_deinit(USART);
    usart_baudrate_set(USART, 19200U);
    usart_transmit_config(USART, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART, USART_RECEIVE_ENABLE);
    usart_enable(USART);

    //! 启动串口中断
    usart_interrupt_enable(USART, USART_INT_RBNE);
    usart_interrupt_enable(USART, USART_INT_IDLE);

    //! 配置USART2的DMA TX_RX
    usart_dma_transmit_config(USART, USART_TRANSMIT_DMA_ENABLE);
    usart_dma_receive_config(USART, USART_RECEIVE_DMA_ENABLE);

    //! 初始化DMA
    my_dma_init();
}

/************************************************************
 * Function :       usart_recv_buf
 * Comment  :       用于处理USART接收数据
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-30 V0.1 original
************************************************************/
void usart_recv_buf(void)
{
    if (recv_flag)
    {
        printf("recv_buf:%s, recv_len: %d\r\n", recv_real_buf, recv_real_len);
		//! 处理完数据后需要将接收缓冲区清零，并将标记位置0
		memset(recv_real_buf, 0, recv_real_len);
		recv_real_len = 0;
        recv_flag = 0;
    }
}
/************************************************************
 * Function :       fputc
 * Comment  :       用于重载printf函数
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-30 V0.1 original
************************************************************/
int fputc(int ch, FILE* f)
{
    usart_dma_send((uint8_t*)&ch, 1);
    return ch;
}
/************************************************************
 * Function :       usart_dma_send
 * Comment  :       用于发送数据通过USART DMA
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-30 V0.1 original
************************************************************/
void usart_dma_send(uint8_t* buf, uint16_t len)
{
	//在每次发送前先对DMA通道进行初始化(复位)
    dma_channel_disable(DMA_TRANSFER_X, DMA_TRANSFER_CHANNEL);	// 禁用DMA通道
    dma_flag_clear(DMA_TRANSFER_X, DMA_TRANSFER_CHANNEL, DMA_FLAG_FTF);	// 清除DMA通道完成标志
    dma_memory_address_config(DMA_TRANSFER_X, DMA_TRANSFER_CHANNEL, DMA_MEMORY_0, (uint32_t)buf);	// 配置DMA通道内存地址
    dma_transfer_number_config(DMA_TRANSFER_X, DMA_TRANSFER_CHANNEL, len);	// 配置DMA通道传输数量
    dma_channel_enable(DMA_TRANSFER_X, DMA_TRANSFER_CHANNEL);	// 启用DMA通道
    while (!dma_flag_get(DMA_TRANSFER_X, DMA_TRANSFER_CHANNEL, DMA_FLAG_FTF))	// 等待DMA通道完成传输
        ;
}

/************************************************************
 * Function :       USART0_IRQHandler
 * Comment  :       用于处理USART中断
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-30 V0.1 original
************************************************************/
void USART0_IRQHandler(void)
{
    //! 判断是否是空闲帧
    recv_real_len = 0;
    if (usart_interrupt_flag_get(USART, USART_INT_FLAG_IDLE) != RESET)
    {
        //! 清除中断标志
        usart_data_receive(USART);
        //! 关闭DMA通道
        dma_channel_disable(DMA_RECEIVE_X, DMA_RECEIVE_CHANNEL);

        //! 进行计算DMA中接收的字节数
        recv_real_len = sizeof(recv_real_buf) - dma_transfer_number_get(DMA_RECEIVE_X, DMA_RECEIVE_CHANNEL);

        if (recv_real_len != 0 && recv_real_len < sizeof(recv_real_buf))
        {
            //! 此时重新配置DMA接收
            dma_memory_address_config(DMA_RECEIVE_X, DMA_RECEIVE_CHANNEL, DMA_MEMORY_0, (uint32_t)recv_real_buf);
            dma_transfer_number_config(DMA_RECEIVE_X, DMA_RECEIVE_CHANNEL, sizeof(recv_real_buf));
            dma_flag_clear(DMA_RECEIVE_X, DMA_RECEIVE_CHANNEL, DMA_FLAG_FTF);
            dma_channel_enable(DMA_RECEIVE_X, DMA_RECEIVE_CHANNEL);
            recv_flag = 1;
        }
        else
        {
            memset(recv_real_buf, 0, sizeof(recv_real_buf));
        }
    }
}

/****************************End*****************************/
