/*!
    \file    spi_port.h
    \brief   definitions of platform port for the gd30ad3344
    
    \version 2024-6-27, V1.0.0, firmware for GD30AD3344
*/

#ifndef __SPI_PORT__H
#define __SPI_PORT__H

#include "gd32f4xx.h"
#include "HeaderFiles.h"
#include "gd32f4xx_rcu.h"
#include "gd32f4xx_gpio.h"
#include "gd32f4xx_exti.h"
#include "gd32f4xx_misc.h"
#include "gd32f4xx_spi.h"

#define SPI_SET_CS()  SPI3_CS_HIGH()
#define SPI_CLR_CS()  SPI3_CS_LOW()

/* SPI transmit and receive 16 bit data */
uint16_t ad3344_spi_txrx16bit(uint16_t tx_byte);

#endif 
