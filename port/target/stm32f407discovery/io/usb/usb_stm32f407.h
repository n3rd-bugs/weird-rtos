/*
 * usb_stm32f407.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _USB_STM32F407_H_
#define _USB_STM32F407_H_
#include <os.h>

#ifdef CONFIG_USB

/* USB configurations. */
#define STM32F407_USB_FS
//#define STM32F407_USB_HS
#define STM32F407_USB_VBUS_SENSING_ENABLED
#define STM32F407_USB_DEVICE_MODE
//#define STM32F407_USB_HOST_MODE
//#define STM32F407_USB_OTG_MODE

#ifdef STM32F407_USB_FS
#define STM32F407_USB_FS_CORE

/* USB FS core configuration. */
#define RX_FIFO_FS_SIZE                         128
#define TX0_FIFO_FS_SIZE                        64
#define TX1_FIFO_FS_SIZE                        128
#define TX2_FIFO_FS_SIZE                        0
#define TX3_FIFO_FS_SIZE                        0
#define TXH_NP_FS_FIFOSIZ                       96
#define TXH_P_FS_FIFOSIZ                        96

//#define STM32F407_USB_FS_LOW_PWR_MGMT_SUPPORT
//#define STM32F407_USB_FS_SOF_OUTPUT_ENABLED
#elif defined(STM32F407_USB_HS)
#define STM32F407_USB_HS_CORE

/* USB HS core configuration. */
#define RX_FIFO_HS_SIZE                         512
#define TX0_FIFO_HS_SIZE                        512
#define TX1_FIFO_HS_SIZE                        512
#define TX2_FIFO_HS_SIZE                        0
#define TX3_FIFO_HS_SIZE                        0
#define TX4_FIFO_HS_SIZE                        0
#define TX5_FIFO_HS_SIZE                        0
#define TXH_NP_HS_FIFOSIZ                       96
#define TXH_P_HS_FIFOSIZ                        96

//#define STM32F407_USB_HS_LOW_PWR_MGMT_SUPPORT
//#define STM32F407_USB_HS_SOF_OUTPUT_ENABLED

//#define STM32F407_USB_INTERNAL_VBUS_ENABLED
#define STM32F407_USB_EXTERNAL_VBUS_ENABLED

#ifdef STM32F407_USB_ULPI_PHY
#define STM32F407_USB_ULPI_PHY_ENABLED
#endif
#ifdef STM32F407_USB_EMBEDDED_PHY
#define STM32F407_USB_EMBEDDED_PHY_ENABLED
#endif
#ifdef STM32F407_USB_I2C_PHY
#define STM32F407_USB_I2C_PHY_ENABLED
#endif

#define STM32F407_USB_HS_DMA_ENABLED
#define STM32F407_USB_HS_DEDICATED_EP1_ENABLED

#endif

/* Hook-up USB OS stack. */
#define USB_TGT_INIT    usb_stm32f407_init

#include <usb_registers_stm32f407.h>

/* USB definitions. */
#define USB_STM32F407_SPEED_PARAM_HIGH          0
#define USB_STM32F407_SPEED_PARAM_HIGH_IN_FULL  1
#define USB_STM32F407_SPEED_PARAM_FULL          3

#define USB_STM32F407_SPEED_HIGH                0
#define USB_STM32F407_SPEED_FULL                1

#define GAHBCFG_TXFEMPTYLVL_EMPTY               1
#define GAHBCFG_TXFEMPTYLVL_HALFEMPTY           0
#define GAHBCFG_GLBINT_ENABLE                   1
#define GAHBCFG_INT_DMA_BURST_SINGLE            0
#define GAHBCFG_INT_DMA_BURST_INCR              1
#define GAHBCFG_INT_DMA_BURST_INCR4             3
#define GAHBCFG_INT_DMA_BURST_INCR8             5
#define GAHBCFG_INT_DMA_BURST_INCR16            7
#define GAHBCFG_DMAENABLE                       1
#define GAHBCFG_TXFEMPTYLVL_EMPTY               1
#define GAHBCFG_TXFEMPTYLVL_HALFEMPTY           0
#define GRXSTS_PKTSTS_IN                        2
#define GRXSTS_PKTSTS_IN_XFER_COMP              3
#define GRXSTS_PKTSTS_DATA_TOGGLE_ERR           5
#define GRXSTS_PKTSTS_CH_HALTED                 7

#define MODE_HNP_SRP_CAPABLE                    0
#define MODE_SRP_ONLY_CAPABLE                   1
#define MODE_NO_HNP_SRP_CAPABLE                 2
#define MODE_SRP_CAPABLE_DEVICE                 3
#define MODE_NO_SRP_CAPABLE_DEVICE              4
#define MODE_SRP_CAPABLE_HOST                   5
#define MODE_NO_SRP_CAPABLE_HOST                6
#define USB_STM32F407_A_HOST                    1
#define USB_STM32F407_A_SUSPEND                 2
#define USB_STM32F407_A_PERIPHERAL              3
#define USB_STM32F407_B_PERIPHERAL              4
#define USB_STM32F407_B_HOST                    5

#define USB_STM32F407_DEVICE_MODE               0
#define USB_STM32F407_HOST_MODE                 1
#define USB_STM32F407_OTG_MODE                  2

#define DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ      0
#define DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ      1
#define DSTS_ENUMSPD_LS_PHY_6MHZ                2
#define DSTS_ENUMSPD_FS_PHY_48MHZ               3

#define DCFG_FRAME_INTERVAL_80                  0
#define DCFG_FRAME_INTERVAL_85                  1
#define DCFG_FRAME_INTERVAL_90                  2
#define DCFG_FRAME_INTERVAL_95                  3

#define DEP0CTL_MPS_64                          0
#define DEP0CTL_MPS_32                          1
#define DEP0CTL_MPS_16                          2
#define DEP0CTL_MPS_8                           3

#define EP_SPEED_LOW                            0
#define EP_SPEED_FULL                           1
#define EP_SPEED_HIGH                           2

#define EP_TYPE_CTRL                            0
#define EP_TYPE_ISOC                            1
#define EP_TYPE_BULK                            2
#define EP_TYPE_INTR                            3
#define EP_TYPE_MSK                             3

#define STS_GOUT_NAK                            1
#define STS_DATA_UPDT                           2
#define STS_XFER_COMP                           3
#define STS_SETUP_COMP                          4
#define STS_SETUP_UPDT                          6

#define HC_PID_DATA0                            0
#define HC_PID_DATA2                            1
#define HC_PID_DATA1                            2
#define HC_PID_SETUP                            3

#define HPRT0_PRTSPD_HIGH_SPEED                 0
#define HPRT0_PRTSPD_FULL_SPEED                 1
#define HPRT0_PRTSPD_LOW_SPEED                  2

#define HCFG_30_60_MHZ                          0
#define HCFG_48_MHZ                             1
#define HCFG_6_MHZ                              2

#define HCCHAR_CTRL                             0
#define HCCHAR_ISOC                             1
#define HCCHAR_BULK                             2
#define HCCHAR_INTR                             3

#define USB_STM32F407_EP0_IDLE                  0
#define USB_STM32F407_EP0_SETUP                 1
#define USB_STM32F407_EP0_DATA_IN               2
#define USB_STM32F407_EP0_DATA_OUT              3
#define USB_STM32F407_EP0_STATUS_IN             4
#define USB_STM32F407_EP0_STATUS_OUT            5
#define USB_STM32F407_EP0_STALL                 6

#define USB_STM32F407_EP_TX_DIS                 0x0000
#define USB_STM32F407_EP_TX_STALL               0x0010
#define USB_STM32F407_EP_TX_NAK                 0x0020
#define USB_STM32F407_EP_TX_VALID               0x0030

#define USB_STM32F407_EP_RX_DIS                 0x0000
#define USB_STM32F407_EP_RX_STALL               0x1000
#define USB_STM32F407_EP_RX_NAK                 0x2000
#define USB_STM32F407_EP_RX_VALID               0x3000

#define MAX_DATA_LENGTH                         ALLIGN_CEIL(0xFF)

/* USB speed definitions. */
enum USB_STM32F407_SPEED
{
    UNKNOWN = 0,
    LOW,
    FULL,
    HIGH
};

/* USB configuration. */
typedef struct _usb_stm32f407_cfg
{
    uint16_t    max_packet_size;
    uint8_t     host_channels;
    uint8_t     dev_endpoints;
    uint8_t     speed;
    uint8_t     dma_enable;
    uint8_t     sof_output;
    uint8_t     low_power;
} USB_STM32F407_CFG;

typedef struct _usb_stm32f407_handle
{
    USB_STM32F407_CFG           cfg;
    USB_STM32F407_CORE_REGS     regs;
#ifdef STM32F407_USB_DEVICE_MODE
    USB_DEVICE                  device;
#endif
#ifdef STM32F407_USB_HOST_MODE
    HCD_DEV                     host;
#endif
#ifdef STM32F407_USB_OTG_MODE
    OTG_DEV                     otg;
#endif
} USB_STM32F407_HANDLE;

/* Function prototypes. */
void usb_stm32f407_init();
void usb_stm32f407_hw_initilaize(USB_STM32F407_HANDLE *);
void usb_stm32f407_enable_interrupt(USB_STM32F407_HANDLE *);
uint32_t usb_stm32f407_select_core(USB_STM32F407_HANDLE *);
uint32_t usb_stm32f407_core_initialize(USB_STM32F407_HANDLE *);
void usb_stm32f407_enable_common_interrupt(USB_STM32F407_HANDLE *);
uint32_t usb_stm32f407_core_reset(USB_STM32F407_HANDLE *);
uint32_t usb_stm32f407_write_packet(USB_STM32F407_HANDLE *, uint8_t *, uint8_t, uint32_t);
uint32_t usb_stm32f407_read_packet(USB_STM32F407_HANDLE *, uint8_t *, uint32_t);
uint32_t usb_stm32f407_read_otg_interrupt(USB_STM32F407_HANDLE *);
uint32_t usb_stm32f407_read_core_interrupt(USB_STM32F407_HANDLE *);
uint32_t usb_stm32f407_enable_global_interrupt(USB_STM32F407_HANDLE *);
uint32_t usb_stm32f407_disable_global_interrupt(USB_STM32F407_HANDLE *);
uint32_t usb_stm32f407_flush_tx_fifo(USB_STM32F407_HANDLE *, uint32_t);
uint32_t usb_stm32f407_flush_rx_fifo(USB_STM32F407_HANDLE *usb_device);
uint32_t usb_stm32f407_set_current_mode(USB_STM32F407_HANDLE *, uint8_t);
uint32_t usb_stm32f407_get_current_mode(USB_STM32F407_HANDLE *);
uint8_t usb_stm32f407_is_host_mode(USB_STM32F407_HANDLE *);
uint8_t usb_stm32f407_is_device_mode(USB_STM32F407_HANDLE *);

#ifdef USB_FUNCTION
#include <usb_function_stm32f407.h>
#endif

#endif /* CONFIG_USB */
#endif /* _USB_STM32F407_H_ */
