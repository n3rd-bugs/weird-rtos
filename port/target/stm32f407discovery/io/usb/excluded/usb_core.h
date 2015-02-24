/**
  ******************************************************************************
  * @file    usb_core.h
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    22-July-2011
  * @brief   Header of the Core Layer
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CORE_H__
#define __USB_CORE_H__

/* Includes ------------------------------------------------------------------*/
#include <os.h>
#error "should not be included."

/** @addtogroup USB_OTG_DRIVER
  * @{
  */
  
/** @defgroup USB_CORE
  * @brief usb otg driver core layer
  * @{
  */ 


/** @defgroup USB_CORE_Exported_Defines
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Types
  * @{
  */ 


typedef enum {
  USB_OTG_OK = 0,
  USB_OTG_FAIL
}USB_OTG_STS;

typedef enum {
  HC_IDLE = 0,
  HC_XFRC,
  HC_HALTED,
  HC_NAK,
  HC_NYET,
  HC_STALL,
  HC_XACTERR,
  HC_BBLERR,
  HC_DATATGLERR,
}HC_STATUS;

typedef enum {
  URB_IDLE = 0,
  URB_DONE,
  URB_NOTREADY,
  URB_ERROR,
  URB_STALL
}URB_STATE;

typedef enum {
  CTRL_START = 0,
  CTRL_XFRC,
  CTRL_HALTED,
  CTRL_NAK,
  CTRL_STALL,
  CTRL_XACTERR,  
  CTRL_BBLERR,   
  CTRL_DATATGLERR,  
  CTRL_FAIL
}CTRL_STATUS;


typedef struct USB_OTG_hc
{
    uint8_t     *xfer_buff;
    uint32_t    dma_addr;
    uint32_t    xfer_len;
    uint32_t    xfer_count;
    uint16_t    max_packet;
    uint8_t     dev_addr ;
    uint8_t     ep_num;
    uint8_t     ep_is_in;
    uint8_t     speed;
    uint8_t     do_ping;
    uint8_t     ep_type;
    uint8_t     data_pid;
    uint8_t     toggle_in;
    uint8_t     toggle_out;
    uint8_t     pad[1];
}
USB_OTG_HC , *PUSB_OTG_HC;

typedef struct USB_OTG_hPort
{
  void (*Disconnect) (void *phost);
  void (*Connect) (void *phost); 
  uint8_t ConnStatus;
  uint8_t DisconnStatus;
  uint8_t ConnHandled;
  uint8_t DisconnHandled;
} USB_OTG_hPort_TypeDef;


typedef struct _HCD
{
    USB_OTG_hPort_TypeDef   *port_cb;
  uint8_t                   Rx_Buffer [MAX_DATA_LENGTH];
  volatile uint32_t             ConnSts;
  volatile uint32_t             ErrCnt[USB_MAX_FIFO];
  volatile uint32_t             XferCnt[USB_MAX_FIFO];
  volatile HC_STATUS            HC_Status[USB_MAX_FIFO];
  volatile URB_STATE            URB_State[USB_MAX_FIFO];
  uint16_t                  channel [USB_MAX_FIFO];
  USB_OTG_HC                hc [USB_MAX_FIFO];
}
HCD_DEV , *USB_OTG_USBH_PDEV;


typedef struct _OTG
{
  uint8_t    OTG_State;
  uint8_t    OTG_PrevState;  
  uint8_t    OTG_Mode;    
}
OTG_DEV , *USB_OTG_USBO_PDEV;

/**
  * @}
  */ 


/** @defgroup USB_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Variables
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_FunctionsPrototype
  * @{
  */ 
USB_OTG_STS  USB_OTG_PhyInit         (USB_STM32F407_HANDLE *pdev);

/*********************** HOST APIs ********************************************/
#ifdef STM32F407_USB_HOST_MODE
USB_OTG_STS  usb_stm32f407_core_initializeHost    (USB_STM32F407_HANDLE *pdev);
USB_OTG_STS  USB_OTG_EnableHostInt   (USB_STM32F407_HANDLE *pdev);
USB_OTG_STS  USB_OTG_HC_Init         (USB_STM32F407_HANDLE *pdev, uint8_t hc_num);
USB_OTG_STS  USB_OTG_HC_Halt         (USB_STM32F407_HANDLE *pdev, uint8_t hc_num);
USB_OTG_STS  USB_OTG_HC_StartXfer    (USB_STM32F407_HANDLE *pdev, uint8_t hc_num);
USB_OTG_STS  USB_OTG_HC_DoPing       (USB_STM32F407_HANDLE *pdev , uint8_t hc_num);
uint32_t     USB_OTG_ReadHostAllChannels_intr    (USB_STM32F407_HANDLE *pdev);
uint32_t     USB_OTG_ResetPort       (USB_STM32F407_HANDLE *pdev);
uint32_t     USB_OTG_ReadHPRT0       (USB_STM32F407_HANDLE *pdev);
void         USB_OTG_DriveVbus       (USB_STM32F407_HANDLE *pdev, uint8_t state);
void         USB_OTG_InitFSLSPClkSel (USB_STM32F407_HANDLE *pdev ,uint8_t freq);
uint8_t      USB_OTG_IsEvenFrame     (USB_STM32F407_HANDLE *pdev) ;
void         USB_OTG_StopHost        (USB_STM32F407_HANDLE *pdev);
#endif
/********************* DEVICE APIs ********************************************/
/**
  * @}
  */ 

#endif  /* __USB_CORE_H__ */


/**
  * @}
  */ 

/**
  * @}
  */ 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

