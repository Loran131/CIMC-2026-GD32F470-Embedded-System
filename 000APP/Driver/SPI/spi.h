
#include "HeaderFiles.h "
#ifndef __SPI_H
#define __SPI_H

/* SPI0_CLK(PB3), SPI0_MISO(PB4), SPI0_MOSI(PB5) */
/* SPI0_CS(PA15) GPIO pin configuration */

/******************************************************************************************/
/* SPI0 引脚 定义 */

#define SPI0_SCK_GPIO_PORT           GPIOB
#define SPI0_SCK_GPIO_PIN            GPIO_PIN_3
#define SPI0_SCK_GPIO_AF             GPIO_AF_5
#define SPI0_SCK_GPIO_CLK            RCU_GPIOB     /* GPIOB时钟使能 */

#define SPI0_MISO_GPIO_PORT          GPIOB
#define SPI0_MISO_GPIO_PIN           GPIO_PIN_4
#define SPI0_MISO_GPIO_AF            GPIO_AF_5
#define SPI0_MISO_GPIO_CLK           RCU_GPIOB     /* GPIOB时钟使能 */

#define SPI0_MOSI_GPIO_PORT          GPIOB
#define SPI0_MOSI_GPIO_PIN           GPIO_PIN_5
#define SPI0_MOSI_GPIO_AF            GPIO_AF_5
#define SPI0_MOSI_GPIO_CLK           RCU_GPIOB     /* GPIOB时钟使能 */

/* SPI0相关定义 */
#define SPI0_SPI                     SPI0
#define SPI0_SPI_CLK                 RCU_SPI0      /* SPI0时钟使能 */
/* CS 引脚 (手动控制片选) */
#define SPI0_CS_GPIO_PORT           GPIOA
#define SPI0_CS_GPIO_PIN            GPIO_PIN_15
#define SPI0_CS_GPIO_CLK            RCU_GPIOA     /* GPIOA时钟使能 */
/******************************************************************************************/
/* SPI3 外设基地址 */
#define SPI3_SPI                    SPI3

/* SPI3 时钟使能 */
#define SPI3_SCK_GPIO_CLK           RCU_GPIOE
#define SPI3_MISO_GPIO_CLK          RCU_GPIOE
#define SPI3_MOSI_GPIO_CLK          RCU_GPIOE
#define SPI3_CS_GPIO_CLK            RCU_GPIOE
#define SPI3_SPI_CLK                RCU_SPI3

/* SPI3 引脚和端口 */
#define SPI3_SCK_GPIO_PORT          GPIOE
#define SPI3_SCK_GPIO_PIN           GPIO_PIN_12
#define SPI3_SCK_GPIO_AF            GPIO_AF_5       /* 查阅数据手册确认 */

#define SPI3_MISO_GPIO_PORT         GPIOE
#define SPI3_MISO_GPIO_PIN          GPIO_PIN_13
#define SPI3_MISO_GPIO_AF           GPIO_AF_5

#define SPI3_MOSI_GPIO_PORT         GPIOE
#define SPI3_MOSI_GPIO_PIN          GPIO_PIN_9
#define SPI3_MOSI_GPIO_AF           GPIO_AF_5

/* CS 引脚（自定义） */
#define SPI3_CS_GPIO_PORT           GPIOE
#define SPI3_CS_GPIO_PIN            GPIO_PIN_11
#define SPI3_CS_LOW()   gpio_bit_reset(SPI3_CS_GPIO_PORT, SPI3_CS_GPIO_PIN)
#define SPI3_CS_HIGH()  gpio_bit_set(SPI3_CS_GPIO_PORT, SPI3_CS_GPIO_PIN)
/* SPI总线速度设置 */
#define SPI_SPEED_2         0
#define SPI_SPEED_4         1
#define SPI_SPEED_8         2
#define SPI_SPEED_16        3
#define SPI_SPEED_32        4
#define SPI_SPEED_64        5
#define SPI_SPEED_128       6
#define SPI_SPEED_256       7


void spi0_init(void);                              /* SPI0初始化 */               
void spi0_set_speed(uint8_t speed);                /* 设置SPI0速度 */
uint8_t spi0_read_write_byte(uint8_t txdata);      /* SPI0读写一个字节 */

void spi3_init(void);                              /* SPI3初始化 */               
void spi3_set_speed(uint8_t speed);                /* 设置SPI3速度 */
uint8_t spi3_read_write_byte(uint8_t txdata);      /* SPI3读写一个字节 */

#endif
























