/*
 * spi_stm32f103.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _SPI_STM32F103_H_
#define _SPI_STM32F103_H_
#include <kernel.h>

#ifdef CONFIG_SPI
#include <spi.h>

/* STM32F103 SPI CR1 register definitions. */
#define STM32F103_SPI_CR1_BIDI_SHIFT    (15)
#define STM32F103_SPI_CR1_BIDIOE_SHIFT  (14)
#define STM32F103_SPI_CR1_CRCEN_SHIFT   (13)
#define STM32F103_SPI_CR1_CRCNEXT_SHIFT (12)
#define STM32F103_SPI_CR1_DFF_SHIFT     (11)
#define STM32F103_SPI_CR1_RXONLY_SHIFT  (10)
#define STM32F103_SPI_CR1_SMM_SHIFT     (9)
#define STM32F103_SPI_CR1_SSI_SHIFT     (8)
#define STM32F103_SPI_CR1_LSB_SHIFT     (7)
#define STM32F103_SPI_CR1_SPE_SHIFT     (6)
#define STM32F103_SPI_CR1_BR_SHIFT      (3)
#define STM32F103_SPI_CR1_MSTR_SHIFT    (2)
#define STM32F103_SPI_CR1_CPOL_SHIFT    (1)
#define STM32F103_SPI_CR1_CPHA_SHIFT    (0)

/* STM32F103 SPI CR2 register definitions. */
#define STM32F103_SPI_CR2_TXEIE_SHIFT   (7)
#define STM32F103_SPI_CR1_RXNEIE_SHIFT  (6)
#define STM32F103_SPI_CR1_ERRIE_SHIFT   (5)
#define STM32F103_SPI_CR1_SSOE_SHIFT    (2)
#define STM32F103_SPI_CR1_TXDMAE_SHIFT  (1)
#define STM32F103_SPI_CR1_RXDMAE_SHIFT  (0)

/* STM32F103 SPI I2SCFG register definitions. */
#define STM32F103_SPI_I2SCFG_MOD_SHIFT  (11)

/* STM32F103 status register definitions. */
#define STM32F103_SPI_SR_RXNE           (0x0001)
#define STM32F103_SPI_SR_TXE            (0x0002)
#define STM32F103_SPI_SR_CHSIDE         (0x0004)
#define STM32F103_SPI_SR_UDR            (0x0008)
#define STM32F103_SPI_SR_CRCERR         (0x0010)
#define STM32F103_SPI_SR_MODF           (0x0020)
#define STM32F103_SPI_SR_OVR            (0x0040)
#define STM32F103_SPI_SR_BSY            (0x0080)

#ifndef CMAKE_BUILD
/* SPI Timeout configuration. */
#define STM32F103_SPI_TIMEOUT           (1000)
#endif /* CMAKE_BUILD */

/* SPI device structure. */
typedef struct _stm32f103_spi
{
    /* Physical device ID. */
    uint32_t    device_num;

    /* STM32F103 SPI device register. */
    SPI_TypeDef *reg;

} STM32F103_SPI;

/* Function prototypes. */
void spi_stm32f103_init(SPI_DEVICE *device);
void spi_stm32f103_slave_select(SPI_DEVICE *);
void spi_stm32f103_slave_unselect(SPI_DEVICE *);
int32_t spi_stm32f103_message(SPI_DEVICE *, SPI_MSG *);

#endif /* CONFIG_SPI */
#endif /* _SPI_STM32F103_H_ */
