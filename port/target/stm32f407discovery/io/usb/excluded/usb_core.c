/**
  ******************************************************************************
  * @file    usb_core.c
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    22-July-2011
  * @brief   USB-OTG Core Layer
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

/* Includes ------------------------------------------------------------------*/
#include <excluded/usb_core.h>


#error "should not be included."

/** @addtogroup USB_OTG_DRIVER
* @{
*/

/** @defgroup USB_CORE 
* @brief This file includes the USB-OTG Core Layer
* @{
*/


/** @defgroup USB_CORE_Private_Defines
* @{
*/ 

/**
* @}
*/ 


/** @defgroup USB_CORE_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 



/** @defgroup USB_CORE_Private_Macros
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USB_CORE_Private_Variables
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USB_CORE_Private_FunctionPrototypes
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USB_CORE_Private_Functions
* @{
*/ 




#ifdef STM32F407_USB_HOST_MODE
/**
* @brief  usb_stm32f407_core_initializeHost : Initializes USB_OTG controller for host mode
* @param  usb_device : Selected device
* @retval status
*/
USB_OTG_STS usb_stm32f407_core_initializeHost(USB_STM32F407_HANDLE *usb_device)
{
  USB_OTG_STS                     status = USB_OTG_OK;
  USB_STM32F407_FSIZ            nptxfifosize;
  USB_STM32F407_FSIZ            ptxfifosize;  
  USB_STM32F407_HCFG            hcfg;
  
#ifdef STM32F407_USB_OTG_MODE
  USB_STM32F407_OTGCTL          gotgctl;
#endif
  
  uint32_t                        i = 0;
  
  nptxfifosize.d32 = 0;  
  ptxfifosize.d32 = 0;
#ifdef STM32F407_USB_OTG_MODE
  gotgctl.d32 = 0;
#endif
  hcfg.d32 = 0;
  
  
  /* configure charge pump IO */
  USB_OTG_BSP_ConfigVBUS(usb_device);
  
  /* Restart the Phy Clock */
  OS_WRITE_REG32(usb_device->regs.PCGCCTL, 0);
  
  /* Initialize Host Configuration Register */
  USB_OTG_InitFSLSPClkSel(usb_device , HCFG_48_MHZ); /* in init phase */
  
  hcfg.d32 = OS_READ_REG32(&usb_device->regs.HREGS->HCFG);
  hcfg.b.fslssupp = 0;
  OS_WRITE_REG32(&usb_device->regs.HREGS->HCFG, hcfg.d32);
  
  /* Configure data FIFO sizes */
  /* Rx FIFO */
#ifdef STM32F407_USB_FS_CORE
  if(usb_device->cfg.coreID == STM32F407_USB_FS_CORE_ID)
  {
    /* set Rx FIFO size */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GRXFSIZ, RX_FIFO_FS_SIZE);
    nptxfifosize.b.startaddr = RX_FIFO_FS_SIZE;   
    nptxfifosize.b.depth = TXH_NP_FS_FIFOSIZ;  
    OS_WRITE_REG32(&usb_device->regs.GREGS->DIEPTXF0_HNPTXFSIZ, nptxfifosize.d32);
    
    ptxfifosize.b.startaddr = RX_FIFO_FS_SIZE + TXH_NP_FS_FIFOSIZ;
    ptxfifosize.b.depth     = TXH_P_FS_FIFOSIZ;
    OS_WRITE_REG32(&usb_device->regs.GREGS->HPTXFSIZ, ptxfifosize.d32);
  }
#endif
#ifdef STM32F407_USB_HS_CORE  
   if (usb_device->cfg.coreID == USB_OTG_HS_CORE_ID)
  {
   /* set Rx FIFO size */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GRXFSIZ, RX_FIFO_HS_SIZE);
    nptxfifosize.b.startaddr = RX_FIFO_HS_SIZE;   
    nptxfifosize.b.depth = TXH_NP_HS_FIFOSIZ;  
    OS_WRITE_REG32(&usb_device->regs.GREGS->DIEPTXF0_HNPTXFSIZ, nptxfifosize.d32);
    
    ptxfifosize.b.startaddr = RX_FIFO_HS_SIZE + TXH_NP_HS_FIFOSIZ;
    ptxfifosize.b.depth     = TXH_P_HS_FIFOSIZ;
    OS_WRITE_REG32(&usb_device->regs.GREGS->HPTXFSIZ, ptxfifosize.d32);
  }
#endif  
  
#ifdef STM32F407_USB_OTG_MODE
  /* Clear Host Set HNP Enable in the USB_OTG Control Register */
  gotgctl.b.hstsethnpen = 1;
  OS_MASK_REG32( &usb_device->regs.GREGS->GOTGCTL, gotgctl.d32, 0);
#endif
  
  /* Make sure the FIFOs are flushed. */
  usb_stm32f407_flush_tx_fifo(usb_device, 0x10 );         /* all Tx FIFOs */
  usb_stm32f407_flush_rx_fifo(usb_device);
  
  
  /* Clear all pending HC Interrupts */
  for (i = 0; i < usb_device->cfg.host_channels; i++)
  {
    OS_WRITE_REG32( &usb_device->regs.HC_REGS[i]->HCINT, 0xFFFFFFFF );
    OS_WRITE_REG32( &usb_device->regs.HC_REGS[i]->HCGINTMSK, 0 );
  }
#ifndef STM32F407_USB_OTG_MODE
  USB_OTG_DriveVbus(usb_device, 1);
#endif
  
  USB_OTG_EnableHostInt(usb_device);
  return status;
}

/**
* @brief  USB_OTG_IsEvenFrame 
*         This function returns the frame number for sof packet
* @param  usb_device : Selected device
* @retval Frame number
*/
uint8_t USB_OTG_IsEvenFrame (USB_STM32F407_HANDLE *usb_device)
{
  return !(OS_READ_REG32(&usb_device->regs.HREGS->HFNUM) & 0x1);
}

/**
* @brief  USB_OTG_DriveVbus : set/reset vbus
* @param  usb_device : Selected device
* @param  state : VBUS state
* @retval None
*/
void USB_OTG_DriveVbus (USB_STM32F407_HANDLE *usb_device, uint8_t state)
{
  USB_STM32F407_HPRT0     hprt0;
  
  hprt0.d32 = 0;
  
  /* enable disable the external charge pump */
  USB_OTG_BSP_DriveVBUS(usb_device, state);
  
  /* Turn on the Host port power. */
  hprt0.d32 = USB_OTG_ReadHPRT0(usb_device);
  if ((hprt0.b.prtpwr == 0 ) && (state == 1 ))
  {
    hprt0.b.prtpwr = 1;
    OS_WRITE_REG32(usb_device->regs.HPRT0, hprt0.d32);
  }
  if ((hprt0.b.prtpwr == 1 ) && (state == 0 ))
  {
    hprt0.b.prtpwr = 0;
    OS_WRITE_REG32(usb_device->regs.HPRT0, hprt0.d32);
  }
  
  USB_OTG_BSP_mDelay(200);
}
/**
* @brief  USB_OTG_EnableHostInt: Enables the Host mode interrupts
* @param  usb_device : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EnableHostInt(USB_STM32F407_HANDLE *usb_device)
{
  USB_OTG_STS       status = USB_OTG_OK;
  USB_STM32F407_GINTMSK  intmsk;
  intmsk.d32 = 0;
  /* Disable all interrupts. */
  OS_WRITE_REG32(&usb_device->regs.GREGS->GINTMSK, 0);
  
  /* Clear any pending interrupts. */
  OS_WRITE_REG32(&usb_device->regs.GREGS->GINTSTS, 0xFFFFFFFF);
  
  /* Enable the common interrupts */
  usb_stm32f407_enable_common_interrupt(usb_device);
  
  if (usb_device->cfg.dma_enable == 0)
  {  
    intmsk.b.rxstsqlvl  = 1;
  }  
  intmsk.b.portintr   = 1;
  intmsk.b.hcintr     = 1;
  intmsk.b.disconnect = 1;  
  intmsk.b.sofintr    = 1;  
  intmsk.b.incomplisoout  = 1; 
  OS_MASK_REG32(&usb_device->regs.GREGS->GINTMSK, intmsk.d32, intmsk.d32);
  return status;
}

/**
* @brief  USB_OTG_InitFSLSPClkSel : Initializes the FSLSPClkSel field of the 
*         HCFG register on the PHY type
* @param  usb_device : Selected device
* @param  freq : clock frequency
* @retval None
*/
void USB_OTG_InitFSLSPClkSel(USB_STM32F407_HANDLE *usb_device , uint8_t freq)
{
  USB_STM32F407_HCFG   hcfg;
  
  hcfg.d32 = OS_READ_REG32(&usb_device->regs.HREGS->HCFG);
  hcfg.b.fslspclksel = freq;
  OS_WRITE_REG32(&usb_device->regs.HREGS->HCFG, hcfg.d32);
}


/**
* @brief  USB_OTG_ReadHPRT0 : Reads HPRT0 to modify later
* @param  usb_device : Selected device
* @retval HPRT0 value
*/
uint32_t USB_OTG_ReadHPRT0(USB_STM32F407_HANDLE *usb_device)
{
  USB_STM32F407_HPRT0  hprt0;
  
  hprt0.d32 = OS_READ_REG32(usb_device->regs.HPRT0);
  hprt0.b.prtena = 0;
  hprt0.b.prtconndet = 0;
  hprt0.b.prtenchng = 0;
  hprt0.b.prtovrcurrchng = 0;
  return hprt0.d32;
}


/**
* @brief  USB_OTG_ReadHostAllChannels_intr : Register PCD Callbacks
* @param  usb_device : Selected device
* @retval Status
*/
uint32_t USB_OTG_ReadHostAllChannels_intr (USB_STM32F407_HANDLE *usb_device)
{
  return (OS_READ_REG32 (&usb_device->regs.HREGS->HAINT));
}


/**
* @brief  USB_OTG_ResetPort : Reset Host Port
* @param  usb_device : Selected device
* @retval status
* @note : (1)The application must wait at least 10 ms (+ 10 ms security)
*   before clearing the reset bit.
*/
uint32_t USB_OTG_ResetPort(USB_STM32F407_HANDLE *usb_device)
{
  USB_STM32F407_HPRT0  hprt0;
  
  hprt0.d32 = USB_OTG_ReadHPRT0(usb_device);
  hprt0.b.prtrst = 1;
  OS_WRITE_REG32(usb_device->regs.HPRT0, hprt0.d32);
  USB_OTG_BSP_mDelay (10);                                /* See Note #1 */
  hprt0.b.prtrst = 0;
  OS_WRITE_REG32(usb_device->regs.HPRT0, hprt0.d32);
  USB_OTG_BSP_mDelay (20);   
  return 1;
}


/**
* @brief  USB_OTG_HC_Init : Prepares a host channel for transferring packets
* @param  usb_device : Selected device
* @param  hc_num : channel number
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_HC_Init(USB_STM32F407_HANDLE *usb_device , uint8_t hc_num)
{
  USB_OTG_STS status = USB_OTG_OK;
  uint32_t intr_enable = 0;
  USB_STM32F407_HCGINTMSK  hcintmsk;
  USB_STM32F407_GINTMSK    gintmsk;
  USB_STM32F407_HCCHAR     hcchar;
  USB_STM32F407_HCINT     hcint;
  
  
  gintmsk.d32 = 0;
  hcintmsk.d32 = 0;
  hcchar.d32 = 0;
  
  /* Clear old interrupt conditions for this host channel. */
  hcint.d32 = 0xFFFFFFFF;
  OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCINT, hcint.d32);
  
  /* Enable channel interrupts required for this transfer. */
  hcintmsk.d32 = 0;
  
  if (usb_device->cfg.dma_enable == 1)
  {
    hcintmsk.b.ahberr = 1;
  }
  
  switch (usb_device->host.hc[hc_num].ep_type)
  {
  case EP_TYPE_CTRL:
  case EP_TYPE_BULK:
    hcintmsk.b.xfercompl = 1;
    hcintmsk.b.stall = 1;
    hcintmsk.b.xacterr = 1;
    hcintmsk.b.datatglerr = 1;
    hcintmsk.b.nak = 1;  
    if (usb_device->host.hc[hc_num].ep_is_in)
    {
      hcintmsk.b.bblerr = 1;
    } 
    else 
    {
      hcintmsk.b.nyet = 1;
      if (usb_device->host.hc[hc_num].do_ping)
      {
        hcintmsk.b.ack = 1;
      }
    }
    break;
  case EP_TYPE_INTR:
    hcintmsk.b.xfercompl = 1;
    hcintmsk.b.nak = 1;
    hcintmsk.b.stall = 1;
    hcintmsk.b.xacterr = 1;
    hcintmsk.b.datatglerr = 1;
    hcintmsk.b.frmovrun = 1;
    
    if (usb_device->host.hc[hc_num].ep_is_in)
    {
      hcintmsk.b.bblerr = 1;
    }
    
    break;
  case EP_TYPE_ISOC:
    hcintmsk.b.xfercompl = 1;
    hcintmsk.b.frmovrun = 1;
    hcintmsk.b.ack = 1;
    
    if (usb_device->host.hc[hc_num].ep_is_in)
    {
      hcintmsk.b.xacterr = 1;
      hcintmsk.b.bblerr = 1;
    }
    break;
  }
  
  
  OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCGINTMSK, hcintmsk.d32);
  
  
  /* Enable the top level host channel interrupt. */
  intr_enable = (1 << hc_num);
  OS_MASK_REG32(&usb_device->regs.HREGS->HAINTMSK, 0, intr_enable);
  
  /* Make sure host channel interrupts are enabled. */
  gintmsk.b.hcintr = 1;
  OS_MASK_REG32(&usb_device->regs.GREGS->GINTMSK, 0, gintmsk.d32);
  
  /* Program the HCCHAR register */
  hcchar.d32 = 0;
  hcchar.b.devaddr = usb_device->host.hc[hc_num].dev_addr;
  hcchar.b.epnum   = usb_device->host.hc[hc_num].ep_num;
  hcchar.b.epdir   = usb_device->host.hc[hc_num].ep_is_in;
  hcchar.b.lspddev = (usb_device->host.hc[hc_num].speed == HPRT0_PRTSPD_LOW_SPEED);
  hcchar.b.eptype  = usb_device->host.hc[hc_num].ep_type;
  hcchar.b.mps     = usb_device->host.hc[hc_num].max_packet;
  if (usb_device->host.hc[hc_num].ep_type == HCCHAR_INTR)
  {
    hcchar.b.oddfrm  = 1;
  }
  OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCCHAR, hcchar.d32);
  return status;
}


/**
* @brief  USB_OTG_HC_StartXfer : Start transfer
* @param  usb_device : Selected device
* @param  hc_num : channel number
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_HC_StartXfer(USB_STM32F407_HANDLE *usb_device , uint8_t hc_num)
{
  USB_OTG_STS status = USB_OTG_OK;
  USB_STM32F407_HCCHAR   hcchar;
  USB_STM32F407_HCTSIZ  hctsiz;
  USB_STM32F407_HNPTXSTS hnptxsts; 
  USB_STM32F407_HPTXSTS  hptxsts; 
  USB_STM32F407_GINTMSK  intmsk;
  uint16_t                 len_words = 0;   
  
  uint16_t num_packets;
  uint16_t max_hc_pkt_count;
  
  max_hc_pkt_count = 256;
  hctsiz.d32 = 0;
  hcchar.d32 = 0;
  intmsk.d32 = 0;
  
  /* Compute the expected number of packets associated to the transfer */
  if (usb_device->host.hc[hc_num].xfer_len > 0)
  {
    num_packets = (usb_device->host.hc[hc_num].xfer_len + \
      usb_device->host.hc[hc_num].max_packet - 1) / usb_device->host.hc[hc_num].max_packet;
    
    if (num_packets > max_hc_pkt_count)
    {
      num_packets = max_hc_pkt_count;
      usb_device->host.hc[hc_num].xfer_len = num_packets * \
        usb_device->host.hc[hc_num].max_packet;
    }
  }
  else
  {
    num_packets = 1;
  }
  if (usb_device->host.hc[hc_num].ep_is_in)
  {
    usb_device->host.hc[hc_num].xfer_len = num_packets * \
      usb_device->host.hc[hc_num].max_packet;
  }
  /* Initialize the HCTSIZn register */
  hctsiz.b.xfersize = usb_device->host.hc[hc_num].xfer_len;
  hctsiz.b.pktcnt = num_packets;
  hctsiz.b.pid = usb_device->host.hc[hc_num].data_pid;
  OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCTSIZ, hctsiz.d32);
  
  if (usb_device->cfg.dma_enable == 1)
  {
    OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCDMA, (unsigned int)usb_device->host.hc[hc_num].xfer_buff);
  }
  
  
  hcchar.d32 = OS_READ_REG32(&usb_device->regs.HC_REGS[hc_num]->HCCHAR);
  hcchar.b.oddfrm = USB_OTG_IsEvenFrame(usb_device);
  
  /* Set host channel enable */
  hcchar.b.chen = 1;
  hcchar.b.chdis = 0;
  OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCCHAR, hcchar.d32);

  if (usb_device->cfg.dma_enable == 0) /* Slave mode */
  {  
    if((usb_device->host.hc[hc_num].ep_is_in == 0) &&
        (usb_device->host.hc[hc_num].xfer_len > 0))
    {
      switch(usb_device->host.hc[hc_num].ep_type)
      {
        /* Non periodic transfer */
      case EP_TYPE_CTRL:
      case EP_TYPE_BULK:
        
        hnptxsts.d32 = OS_READ_REG32(&usb_device->regs.GREGS->HNPTXSTS);
        len_words = (usb_device->host.hc[hc_num].xfer_len + 3) / 4;
        
        /* check if there is enough space in FIFO space */
        if(len_words > hnptxsts.b.nptxfspcavail)
        {
          /* need to process data in nptxfempty interrupt */
          intmsk.b.nptxfempty = 1;
          OS_MASK_REG32( &usb_device->regs.GREGS->GINTMSK, 0, intmsk.d32);
        }
        
        break;
        /* Periodic transfer */
      case EP_TYPE_INTR:
      case EP_TYPE_ISOC:
        hptxsts.d32 = OS_READ_REG32(&usb_device->regs.HREGS->HPTXSTS);
        len_words = (usb_device->host.hc[hc_num].xfer_len + 3) / 4;
        /* check if there is enough space in FIFO space */
        if(len_words > hptxsts.b.ptxfspcavail) /* split the transfer */
        {
          /* need to process data in ptxfempty interrupt */
          intmsk.b.ptxfempty = 1;
          OS_MASK_REG32( &usb_device->regs.GREGS->GINTMSK, 0, intmsk.d32);
        }
        break;
        
      default:
        break;
      }
      
      /* Write packet into the Tx FIFO. */
      usb_stm32f407_write_packet(usb_device,
                          usb_device->host.hc[hc_num].xfer_buff ,
                          hc_num, usb_device->host.hc[hc_num].xfer_len);
    }
  }
  return status;
}


/**
* @brief  USB_OTG_HC_Halt : Halt channel
* @param  usb_device : Selected device
* @param  hc_num : channel number
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_HC_Halt(USB_STM32F407_HANDLE *usb_device , uint8_t hc_num)
{
  USB_OTG_STS status = USB_OTG_OK;
  USB_STM32F407_HNPTXSTS            nptxsts;
  USB_STM32F407_HPTXSTS             hptxsts;
  USB_STM32F407_HCCHAR              hcchar;
  
  nptxsts.d32 = 0;
  hptxsts.d32 = 0;
  hcchar.d32 = OS_READ_REG32(&usb_device->regs.HC_REGS[hc_num]->HCCHAR);
  hcchar.b.chen = 1;
  hcchar.b.chdis = 1;
  
  /* Check for space in the request queue to issue the halt. */
  if (hcchar.b.eptype == HCCHAR_CTRL || hcchar.b.eptype == HCCHAR_BULK)
  {
    nptxsts.d32 = OS_READ_REG32(&usb_device->regs.GREGS->HNPTXSTS);
    if (nptxsts.b.nptxqspcavail == 0)
    {
      hcchar.b.chen = 0;
    }
  }
  else
  {
    hptxsts.d32 = OS_READ_REG32(&usb_device->regs.HREGS->HPTXSTS);
    if (hptxsts.b.ptxqspcavail == 0)
    {
      hcchar.b.chen = 0;
    }
  }
  OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCCHAR, hcchar.d32);
  return status;
}

/**
* @brief  Issue a ping token
* @param  None
* @retval : None
*/
USB_OTG_STS USB_OTG_HC_DoPing(USB_STM32F407_HANDLE *usb_device , uint8_t hc_num)
{
  USB_OTG_STS               status = USB_OTG_OK;
  USB_STM32F407_HCCHAR    hcchar;
  USB_STM32F407_HCTSIZ   hctsiz;  
 
  hctsiz.d32 = 0;
  hctsiz.b.dopng = 1;
  hctsiz.b.pktcnt = 1;
  OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCTSIZ, hctsiz.d32);
  
  hcchar.d32 = OS_READ_REG32(&usb_device->regs.HC_REGS[hc_num]->HCCHAR);
  hcchar.b.chen = 1;
  hcchar.b.chdis = 0;
  OS_WRITE_REG32(&usb_device->regs.HC_REGS[hc_num]->HCCHAR, hcchar.d32);
  return status;  
}

/**
* @brief  Stop the device and clean up fifo's
* @param  None
* @retval : None
*/
void USB_OTG_StopHost(USB_STM32F407_HANDLE *usb_device)
{
  USB_STM32F407_HCCHAR  hcchar;
  uint32_t                i;
  
  OS_WRITE_REG32(&usb_device->regs.HREGS->HAINTMSK , 0);
  OS_WRITE_REG32(&usb_device->regs.HREGS->HAINT,      0xFFFFFFFF);
  /* Flush out any leftover queued requests. */
  
  for (i = 0; i < usb_device->cfg.host_channels; i++)
  {
    hcchar.d32 = OS_READ_REG32(&usb_device->regs.HC_REGS[i]->HCCHAR);
    hcchar.b.chen = 0;
    hcchar.b.chdis = 1;
    hcchar.b.epdir = 0;
    OS_WRITE_REG32(&usb_device->regs.HC_REGS[i]->HCCHAR, hcchar.d32);
  }
  
  /* Flush the FIFO */
  usb_stm32f407_flush_rx_fifo(usb_device);
  usb_stm32f407_flush_tx_fifo(usb_device ,  0x10 );
}
#endif
#ifdef STM32F407_USB_DEVICE_MODE
/*         PCD Core Layer       */
#endif
/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
