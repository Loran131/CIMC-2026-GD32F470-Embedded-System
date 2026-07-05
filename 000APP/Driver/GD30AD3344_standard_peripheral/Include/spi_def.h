/*!
    \file    spi_def.h
    \brief   definitions of platform port for the gd30ad3344
    
    \version 2024-10-08, V1.0.0, firmware for GD30AD3344
*/

#ifndef __SPI_DEF__H
#define __SPI_DEF__H

#include "spi_port.h"

#define AD3344_SPI_TX_DUMMY         0x0
#define AD3344_SPI_TX_HIGH          0x4
#define AD3344_SPI_TX_LOW           0x8B

#define R_MMR_KEY_HIGH   0x81
#define R_MMR_KEY_LOW    0x6
#define W_MMR_KEY_HIGH   0x81
#define W_MMR_KEY_LOW    0x0

#endif 

