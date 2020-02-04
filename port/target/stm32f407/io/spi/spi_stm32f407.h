/*
 * spi_stm32f407.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _SPI_STM32F407_H_
#define _SPI_STM32F407_H_
#include <kernel.h>

#ifdef CONFIG_SPI
#include <spi.h>
#include <spi_stm32_config.h>

/* STM32F407 SPI CR1 register definitions. */
#define STM32F407_SPI_CR1_BIDI_SHIFT    (15)
#define STM32F407_SPI_CR1_BIDIOE_SHIFT  (14)
#define STM32F407_SPI_CR1_CRCEN_SHIFT   (13)
#define STM32F407_SPI_CR1_CRCNEXT_SHIFT (12)
#define STM32F407_SPI_CR1_DFF_SHIFT     (11)
#define STM32F407_SPI_CR1_RXONLY_SHIFT  (10)
#define STM32F407_SPI_CR1_SMM_SHIFT     (9)
#define STM32F407_SPI_CR1_SSI_SHIFT     (8)
#define STM32F407_SPI_CR1_LSB_SHIFT     (7)
#define STM32F407_SPI_CR1_SPE_SHIFT     (6)
#define STM32F407_SPI_CR1_BR_SHIFT      (3)
#define STM32F407_SPI_CR1_MSTR_SHIFT    (2)
#define STM32F407_SPI_CR1_CPOL_SHIFT    (1)
#define STM32F407_SPI_CR1_CPHA_SHIFT    (0)

/* STM32F407 SPI CR2 register definitions. */
#define STM32F407_SPI_CR2_TXEIE_SHIFT   (7)
#define STM32F407_SPI_CR1_RXNEIE_SHIFT  (6)
#define STM32F407_SPI_CR1_ERRIE_SHIFT   (5)
#define STM32F407_SPI_CR1_FRF_SHIFT     (4)
#define STM32F407_SPI_CR1_SSOE_SHIFT    (2)
#define STM32F407_SPI_CR1_TXDMAE_SHIFT  (1)
#define STM32F407_SPI_CR1_RXDMAE_SHIFT  (0)

/* STM32F407 SPI I2SCFG register definitions. */
#define STM32F407_SPI_I2SCFG_MOD_SHIFT  (11)

/* STM32F407 status register definitions. */
#define STM32F407_SPI_SR_RXNE           (0x1)
#define STM32F407_SPI_SR_TXE            (0x2)
#define STM32F407_SPI_SR_CHSIDE         (0x4)
#define STM32F407_SPI_SR_UDR            (0x8)
#define STM32F407_SPI_SR_CRCERR         (0x10)
#define STM32F407_SPI_SR_MODF           (0x20)
#define STM32F407_SPI_SR_OVR            (0x40)
#define STM32F407_SPI_SR_BSY            (0x80)
#define STM32F407_SPI_SR_FRE            (0x100)

/* SPI device structure. */
typedef struct _stm32f407_spi
{
    /* Physical device ID. */
    uint32_t    device_num;

    /* STM32F407 SPI device register. */
    SPI_TypeDef *reg;

} STM32F407_SPI;

/* Function prototypes. */
void spi_stm32f407_init(SPI_DEVICE *device);
void spi_stm32f407_slave_select(SPI_DEVICE *);
void spi_stm32f407_slave_unselect(SPI_DEVICE *);
int32_t spi_stm32f407_message(SPI_DEVICE *, SPI_MSG *);

#endif /* CONFIG_SPI */
#endif /* _SPI_STM32F407_H_ */
