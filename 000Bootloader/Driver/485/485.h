
#ifndef __485_H__
#define __485_H__

/************************* 头文件 *************************/
#include "HeaderFiles.h"
#include "ringbuffer.h"
/************************* 宏定义 *************************/
#define USART_485_PORT GPIOD
#define USART_485_ USART1
#define USART_485_TX_Pin GPIO_PIN_5
#define USART_485_RX_Pin GPIO_PIN_6
#define USARTX_485_RCU RCU_USART1
#define USART_485_PIN_RCU RCU_GPIOD


#define USART_485_CS_RCU RCU_GPIOE
#define USART_485_CS_PORT GPIOE
#define USARTX_485_CS_Pin GPIO_PIN_8

#define USARTX_485_Send 1
#define USARTX_485_Receive 0


/************************ 变量定义 ************************/

extern volatile uint16_t g_recv_len;   // 保存最新一帧的实际字节数
extern uint8_t recv_485_flag;
extern RingBuffer recv_485_rb;
#define     MAX_485_FRAME_LEN  256          // 最大报文长度
extern uint8_t     temp_line_buf[MAX_485_FRAME_LEN];
/************************ 函数定义 ************************/

void usart_485_send_str(uint8_t* str, uint8_t len);
void usart_485_recv_buf(void);
void reset_get_ascii_byte(void);
uint8_t get_ascii_byte(void);void usart_485_init(uint32_t baudrate);
#endif 
/****************************End*****************************/
