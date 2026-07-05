
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

extern RingBuffer recv_485_rb;
/************************ 函数定义 ************************/

void timer1_init(void);
void TIMER1_IRQHandler(void);
void usart_485_init(uint32_t baudrate);
void usart_485_send_str(uint8_t* str, uint8_t len);
void usart_485_recv_buf(void);
void reset_get_ascii_byte(void);
uint8_t get_ascii_byte(void);
#endif 
/****************************End*****************************/
