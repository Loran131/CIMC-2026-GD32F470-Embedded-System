
/************************* 头文件 *************************/
#include "485.h"
#include "LED.h""
/************************* 宏定义 *************************/



/************************ 变量定义 ************************/
static uint8_t Buf_Send[128];
__IO static uint8_t usart_send_len = 0;
__IO static uint8_t usart_send_index = 0;
__IO uint8_t usart_recvSuccess_flag = 1;


uint8_t recv_485_len = 0;
uint8_t recv_485_real_len = 0;
uint8_t recv_485_flag = 0;
static uint16_t g_ascii_index = 0;      //读取位置的索引
__IO static uint8_t send_busy = 0;   

RingBuffer recv_485_rb;              // 环形缓冲区对象
#define     MAX_485_FRAME_LEN  256          // 最大报文长度
uint8_t     temp_line_buf[MAX_485_FRAME_LEN]; 
void TIM1_ResetCounter(void)
{
    timer_counter_value_config(TIMER3, 0);  /* 计数器归零，重新计时 */
}

/************************ 函数定义 ************************/
void usart_485_CS(uint8_t cs);

void usart_485_init(uint32_t baudrate)
{
    nvic_irq_enable(USART1_IRQn, 5, 0);
    rcu_periph_clock_enable(USARTX_485_RCU);
    rcu_periph_clock_enable(USART_485_PIN_RCU);
    rcu_periph_clock_enable(USART_485_CS_RCU);

    gpio_mode_set(USART_485_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, USARTX_485_CS_Pin);
    gpio_output_options_set(USART_485_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USARTX_485_CS_Pin);

    gpio_af_set(USART_485_PORT, GPIO_AF_7, USART_485_TX_Pin);
    gpio_mode_set(USART_485_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART_485_TX_Pin);
    gpio_output_options_set(USART_485_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART_485_TX_Pin);

    gpio_af_set(USART_485_PORT, GPIO_AF_7, USART_485_RX_Pin);
    gpio_mode_set(USART_485_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART_485_RX_Pin);
    gpio_output_options_set(USART_485_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART_485_RX_Pin);

    usart_deinit(USART_485_);
    usart_baudrate_set(USART_485_, baudrate);
    usart_transmit_config(USART_485_, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART_485_, USART_RECEIVE_ENABLE);

		/* RS485 默认进入接收态 */
		usart_485_CS(USARTX_485_Receive);

		/* 清掉上电/重初始化残留的 IDLE/RBNE 状态 */
		(void)USART_STAT0(USART_485_);
		(void)USART_DATA(USART_485_);
		
    usart_interrupt_enable(USART_485_, USART_INT_RBNE);
    usart_interrupt_enable(USART_485_, USART_INT_IDLE);
    usart_enable(USART_485_);
}

void usart_485_send_str(uint8_t* str, uint8_t len)
{
    if (len > sizeof(Buf_Send))
    {
        return;
    }

    while (send_busy);
    send_busy = 1;

    usart_485_CS(USARTX_485_Send);

    usart_interrupt_disable(USART_485_, USART_INT_RBNE);
    usart_interrupt_disable(USART_485_, USART_INT_IDLE);
    memcpy(Buf_Send, str, len);
    usart_send_len = len;
    usart_interrupt_enable(USART_485_, USART_INT_TBE);
}

void usart_485_recv_buf(void)
{
    if (recv_485_flag)
    {
        usart_485_send_str(temp_line_buf, recv_485_real_len);
        recv_485_flag = 0;
    }
}

void usart_485_CS(uint8_t cs)
{
    if (cs == 1)
    {
        gpio_bit_set(USART_485_CS_PORT, USARTX_485_CS_Pin);
    }
    else
    {
        gpio_bit_reset(USART_485_CS_PORT, USARTX_485_CS_Pin);
    }
}

void USART1_IRQHandler(void)
{
    if (usart_interrupt_flag_get(USART_485_, USART_INT_FLAG_TBE))
    {
        if (usart_send_index < usart_send_len)
        {
            usart_data_transmit(USART_485_, Buf_Send[usart_send_index]);
            usart_send_index++;
        }
        else
        {
            Buf_Send[usart_send_index] = '\0';
            usart_send_index = 0;
            usart_send_len = 0;
            usart_interrupt_disable(USART_485_, USART_INT_TBE);
            usart_interrupt_enable(USART_485_, USART_INT_TC);
        }
    }

    if (usart_interrupt_flag_get(USART_485_, USART_INT_FLAG_TC))
    {
        usart_interrupt_flag_clear(USART_485_, USART_INT_FLAG_TC);
        usart_interrupt_disable(USART_485_, USART_INT_TC);
        usart_485_CS(USARTX_485_Receive);
        usart_interrupt_enable(USART_485_, USART_INT_RBNE);
        usart_interrupt_enable(USART_485_, USART_INT_IDLE);
        send_busy = 0;   
    }

//uint32_t stat0 = USART_STAT0(USART_485_);
//uint32_t ctl0  = USART_CTL0(USART_485_);
//if ((stat0 & USART_STAT0_IDLEF) && (ctl0 & USART_CTL0_IDLEIE))
//{
//    if (stat0 & USART_STAT0_RBNE)
//    {   
//        uint8_t data = (uint8_t)USART_DATA(USART_485_);
//        RingBuffer_Write(&recv_485_rb, &data, 1);
//        recv_485_len++;
//    }
//    else
//    {
//        (void)USART_DATA(USART_485_);
//    }
//    if (recv_485_len != 0)
//    {
//        recv_485_len = 0;
//        recv_485_flag = 1;
//        usart_recvSuccess_flag = 1;
//    }
//}
//else if ((stat0 & USART_STAT0_RBNE) && (ctl0 & USART_CTL0_RBNEIE))
//{   
//    usart_recvSuccess_flag = 0;
//    uint8_t data = (uint8_t)USART_DATA(USART_485_);
//    RingBuffer_Write(&recv_485_rb, &data, 1);
//    recv_485_len++;
//}

if (usart_interrupt_flag_get(USART_485_, USART_INT_FLAG_IDLE) != RESET)
{
    /* IDLE 必须无条件清除：先读 STAT0，再读 DATA */
    (void)USART_STAT0(USART_485_);
    (void)USART_DATA(USART_485_);

    if (recv_485_len != 0)
    {
        recv_485_len = 0;
        recv_485_flag = 1;
        usart_recvSuccess_flag = 1;
    }
}		if (usart_interrupt_flag_get(USART_485_, USART_INT_FLAG_RBNE) != RESET) {
			usart_recvSuccess_flag = 0;
     	TIM1_ResetCounter();
        uint8_t data = usart_data_receive(USART_485_);  // 读取并清除标志
        RingBuffer_Write(&recv_485_rb, &data, 1);
        recv_485_len++; 
    }

}
void timer1_init(){
    rcu_periph_clock_enable(RCU_TIMER1);
		timer_parameter_struct timer_init_struct;
		timer_struct_para_init(&timer_init_struct);
		timer_init_struct.prescaler = 8399; 
		timer_init_struct.alignedmode = TIMER_COUNTER_UP;
		timer_init_struct.counterdirection = TIMER_COUNTER_UP;
		timer_init_struct.period = 9999; 
		timer_init_struct.clockdivision = TIMER_CKDIV_DIV1;
		timer_init(TIMER1, &timer_init_struct);
	  timer_flag_clear(TIMER1, TIMER_FLAG_UP);
    timer_interrupt_enable(TIMER1, TIMER_INT_UP);
    nvic_irq_enable(TIMER1_IRQn, 1, 0);
	  timer_enable(TIMER1);
}
void TIMER1_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER1, TIMER_INT_FLAG_UP) != RESET)
    {   LED4_OFF();
        recv_485_len = 0;
        recv_485_flag = 1;
        usart_recvSuccess_flag = 1;
        timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
    }
}
