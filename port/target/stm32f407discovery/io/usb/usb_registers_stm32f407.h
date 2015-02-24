/*
 * usb_registers_stm32f407.h
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
#ifndef _USB_REGISTERS_STM32F407_H_
#define _USB_REGISTERS_STM32F407_H_

#include <os.h>

#ifdef CONFIG_USB

/* USB definitions.  */
#define USB_STM32F407_HS_BASE_ADDR          0x40040000
#define USB_STM32F407_FS_BASE_ADDR          0x50000000

#define USB_STM32F407_CORE_GLOBAL_OFFSET    0x000
#define USB_STM32F407_DEV_GLOBAL_OFFSET     0x800
#define USB_STM32F407_DEV_IN_EP_OFFSET      0x900
#define USB_STM32F407_EP_OFFSET             0x20
#define USB_STM32F407_DEV_OUT_EP_OFFSET     0xB00
#define USB_STM32F407_HOST_GLOBAL_OFFSET    0x400
#define USB_STM32F407_HOST_PORT_OFFSET      0x440
#define USB_STM32F407_HOST_CHAN_OFFSET      0x500
#define USB_STM32F407_CHAN_OFFSET           0x20
#define USB_STM32F407_PCGCCTL_OFFSET        0xE00
#define USB_STM32F407_DATA_FIFO_OFFSET      0x1000
#define USB_STM32F407_DATA_FIFO_SIZE        0x1000

#define USB_STM32F407_HS_MAX_PACKET_SIZE    512
#define USB_STM32F407_FS_MAX_PACKET_SIZE    64
#define USB_STM32F407_MAX_EP0_SIZE          64

/* USB General Registers. */
typedef struct _usb_stm32f407_gregs
{
    volatile uint32_t GOTGCTL;      /* USB OTG Control and Status Register  000h */
    volatile uint32_t GOTGINT;      /* USB OTG Interrupt Register           004h */
    volatile uint32_t GAHBCFG;      /* Core AHB Configuration Register      008h */
    volatile uint32_t GUSBCFG;      /* Core USB Configuration Register      00Ch */
    volatile uint32_t GRSTCTL;      /* Core Reset Register                  010h */
    volatile uint32_t GINTSTS;      /* Core Interrupt Register              014h */
    volatile uint32_t GINTMSK;      /* Core Interrupt Mask Register         018h */
    volatile uint32_t GRXSTSR;      /* Receive status Q Read Register       01Ch */
    volatile uint32_t GRXSTSP;      /* Receive status Q Read & POP Register             020h */
    volatile uint32_t GRXFSIZ;      /* Receive FIFO Size Register                       024h */
    volatile uint32_t DIEPTXF0_HNPTXFSIZ;   /* EP0 / Non Periodic TX FIFO Size Register 028h */
    volatile uint32_t HNPTXSTS;     /* Non Periodic TX FIFO/Queue status Register       02Ch */
    volatile uint32_t GI2CCTL;      /* I2C Access Register                  030h */
    uint32_t res34;                 /* PHY Vendor Control Register          034h */
    volatile uint32_t GCCFG;        /* General Purpose IO Register          038h */
    volatile uint32_t CID;          /* User ID Register                     03Ch */
    uint32_t res40[48];             /* Reserved                        040h-0FFh */
    volatile uint32_t HPTXFSIZ;     /* Host Periodic TX FIFO Size Register  100h */
    volatile uint32_t DIEPTXF[USB_MAX_FIFO];  /* Device Periodic Transmit FIFO. */
} USB_STM32F407_GREGS;

/* USB Device Registers. */
typedef struct _usb_stm32f407_dregs
{
    volatile uint32_t DCFG;         /* Device Configuration Register        800h */
    volatile uint32_t DCTL;         /* Device Control Register              804h */
    volatile uint32_t DSTS;         /* Device Status Register (RO)          808h */
    uint32_t res0C;                 /* Reserved                             80Ch */
    volatile uint32_t DIEPMSK;      /* Device IN Endpoint Mask              810h */
    volatile uint32_t DOEPMSK;      /* Device OUT Endpoint Mask             814h */
    volatile uint32_t DAINT;        /* Device All Endpoints Interrupt Register  818h */
    volatile uint32_t DAINTMSK;     /* Device All Endpoints Interrupt Mask      81Ch */
    uint32_t res20;                 /* Reserved                             820h */
    uint32_t res24;                 /* Reserved                             824h */
    volatile uint32_t DVBUSDIS;     /* Device VBUS discharge Register       828h */
    volatile uint32_t DVBUSPULSE;   /* Device VBUS Pulse Register           82Ch */
    volatile uint32_t DTHRCTL;      /* Device Threshold Register            830h */
    volatile uint32_t DIEPEMPMSK;   /* Device empty Mask                    834h */
    volatile uint32_t DEACHINT;     /* Dedicated EP interrupt               838h */
    volatile uint32_t DEACHMSK;     /* Dedicated EP Mask                    83Ch */
    uint32_t res40;                 /* Dedicated EP Mask                    840h */
    volatile uint32_t DINEP1MSK;    /* Dedicated EP Mask                    844h */
    uint32_t res44[15];             /* Reserved                         844-87Ch */
    volatile uint32_t DOUTEP1MSK;   /* Dedicated EP Mask                    884h */
} USB_STM32F407_DREGS;

/* USB IN Endpoint Registers. */
typedef struct _usb_stm32f407_inepregs
{
    volatile uint32_t DIEPCTL;      /* Device IN Endpoint Control Register      900h + (ep_num * 20h) + 00h */
    uint32_t res04;                 /* Reserved                                 900h + (ep_num * 20h) + 04h */
    volatile uint32_t DIEPINT;      /* Device IN Endpoint Interrupt Register    900h + (ep_num * 20h) + 08h */
    uint32_t res0C;                 /* Reserved                                 900h + (ep_num * 20h) + 0Ch */
    volatile uint32_t DIEPTSIZ;     /* IN Endpoint Transfer Size                900h + (ep_num * 20h) + 10h */
    volatile uint32_t DIEPDMA;      /* IN Endpoint DMA Address Register         900h + (ep_num * 20h) + 14h */
    volatile uint32_t DTXFSTS;      /* IN Endpoint TX FIFO Status Register      900h + (ep_num * 20h) + 18h */
    uint32_t res18;                 /* Reserved   900h + (ep_num * 20h) + 1Ch - 900h + (ep_num * 20h) + 1Ch */
} USB_STM32F407_INEPREGS;

/* USB OUT Endpoint Registers. */
typedef struct _usb_stm32f407_outepregs
{
    volatile uint32_t DOEPCTL;      /* Device OUT Endpoint Control Register     B00h + (ep_num * 20h) + 00h */
    volatile uint32_t DOUTEPFRM;    /* Device OUT Endpoint Frame number         B00h + (ep_num * 20h) + 04h */
    volatile uint32_t DOEPINT;      /* Device OUT Endpoint Interrupt Register   B00h + (ep_num * 20h) + 08h */
    uint32_t res0C;                 /* Reserved                                 B00h + (ep_num * 20h) + 0Ch */
    volatile uint32_t DOEPTSIZ;     /* Device OUT Endpoint Transfer Size        B00h + (ep_num * 20h) + 10h */
    volatile uint32_t DOEPDMA;      /* Device OUT Endpoint DMA Address          B00h + (ep_num * 20h) + 14h */
    uint32_t res18[2];              /* Reserved   B00h + (ep_num * 20h) + 18h - B00h + (ep_num * 20h) + 1Ch */
} USB_STM32F407_OUTEPREGS;

/* USB Host Registers. */
typedef struct _usb_stm32f407_hregs
{
    volatile uint32_t HCFG;         /* Host Configuration Register          400h */
    volatile uint32_t HFIR;         /* Host Frame Interval Register         404h */
    volatile uint32_t HFNUM;        /* Host Frame Number/Frame Remaining    408h */
    uint32_t res0C;                 /* Reserved                             40Ch */
    volatile uint32_t HPTXSTS;      /* Host Periodic TX FIFO/ Queue Status  410h */
    volatile uint32_t HAINT;        /* Host All Channels Interrupt Register 414h */
    volatile uint32_t HAINTMSK;     /* Host All Channels Interrupt Mask     418h */
} USB_STM32F407_HREGS;

/* USB Host Channel Registers. */
typedef struct _usb_stm32f407_hc_regs
{
    volatile uint32_t HCCHAR;
    volatile uint32_t HCSPLT;
    volatile uint32_t HCINT;
    volatile uint32_t HCGINTMSK;
    volatile uint32_t HCTSIZ;
    volatile uint32_t HCDMA;
    uint32_t res[2];
} USB_STM32F407_HC_REGS;

/* USB Core Registers. */
typedef struct _usb_stm32f407_core_regs
{
    USB_STM32F407_GREGS *GREGS;
    USB_STM32F407_DREGS *DREGS;
    USB_STM32F407_HREGS *HREGS;
    USB_STM32F407_INEPREGS *INEP_REGS[USB_MAX_FIFO];
    USB_STM32F407_OUTEPREGS *OUTEP_REGS[USB_MAX_FIFO];
    USB_STM32F407_HC_REGS *HC_REGS[USB_MAX_FIFO];
    volatile uint32_t *HPRT0;
    volatile uint32_t *DFIFO[USB_MAX_FIFO];
    volatile uint32_t *PCGCCTL;
} USB_STM32F407_CORE_REGS;

/* USB register bits definitions. */

typedef union _usb_stm32f407_otgctl
{
    uint32_t d32;
    struct
    {
        uint32_t sesreqscs      : 1;
        uint32_t sesreq         : 1;
        uint32_t res2           : 6;
        uint32_t hstnegscs      : 1;
        uint32_t hnpreq         : 1;
        uint32_t hstsethnpen    : 1;
        uint32_t devhnpen       : 1;
        uint32_t res12_15       : 4;
        uint32_t conidsts       : 1;
        uint32_t res17          : 1;
        uint32_t asesvld        : 1;
        uint32_t bsesvld        : 1;
        uint32_t currmod        : 1;
        uint32_t res21_31       : 11;
    } b;
} USB_STM32F407_OTGCTL;

typedef union _usb_stm32f407_gotgint
{
    uint32_t d32;
    struct
    {
        uint32_t res0_1         : 2;
        uint32_t sesenddet      : 1;
        uint32_t res3_7         : 5;
        uint32_t sesreqsucstschng   : 1;
        uint32_t hstnegsucstschng   : 1;
        uint32_t res10_16       : 7;
        uint32_t hstnegdet      : 1;
        uint32_t adevtoutchng   : 1;
        uint32_t debdone        : 1;
        uint32_t res20_31       : 12;
    } b;
} USB_STM32F407_GOTGINT;

typedef union _usb_stm32f407_gahbcfg
{
    uint32_t d32;
    struct
    {
        uint32_t glblintrmsk    : 1;
        uint32_t hburstlen      : 4;
        uint32_t dmaenable      : 1;
        uint32_t res       : 1;
        uint32_t nptxfemplvl_txfemplvl  : 1;
        uint32_t ptxfemplvl     : 1;
        uint32_t res9_31        : 23;
    } b;
} USB_STM32F407_GAHBCFG;

typedef union _usb_stm32f407_gusbcfg
{
    uint32_t d32;
    struct
    {
        uint32_t toutcal        : 3;
        uint32_t phyif          : 1;
        uint32_t ulpi_utmi_sel  : 1;
        uint32_t fsintf         : 1;
        uint32_t physel         : 1;
        uint32_t ddrsel         : 1;
        uint32_t srpcap         : 1;
        uint32_t hnpcap         : 1;
        uint32_t usbtrdtim      : 4;
        uint32_t nptxfrwnden    : 1;
        uint32_t phylpwrclksel  : 1;
        uint32_t otgutmifssel   : 1;
        uint32_t ulpi_fsls      : 1;
        uint32_t ulpi_auto_res  : 1;
        uint32_t ulpi_clk_sus_m : 1;
        uint32_t ulpi_ext_vbus_drv          : 1;
        uint32_t ulpi_int_vbus_indicator    : 1;
        uint32_t term_sel_dl_pulse          : 1;
        uint32_t res            : 6;
        uint32_t force_host     : 1;
        uint32_t force_dev      : 1;
        uint32_t corrupt_tx     : 1;
    } b;
} USB_STM32F407_GUSBCFG;

typedef union _usb_stm32f407_grstctl
{
    uint32_t d32;
    struct
    {
        uint32_t csftrst        : 1;
        uint32_t hsftrst        : 1;
        uint32_t hstfrm         : 1;
        uint32_t intknqflsh     : 1;
        uint32_t rxfflsh        : 1;
        uint32_t txfflsh        : 1;
        uint32_t txfnum         : 5;
        uint32_t res11_29       : 19;
        uint32_t dmareq         : 1;
        uint32_t ahbidle        : 1;
    } b;
} USB_STM32F407_GRSTCTL;

typedef union _usb_stm32f407_gintmsk
{
    uint32_t d32;
    struct
    {
        uint32_t res0           : 1;
        uint32_t modemismatch   : 1;
        uint32_t otgintr        : 1;
        uint32_t sofintr        : 1;
        uint32_t rxstsqlvl      : 1;
        uint32_t nptxfempty     : 1;
        uint32_t ginnakeff      : 1;
        uint32_t goutnakeff     : 1;
        uint32_t res8           : 1;
        uint32_t i2cintr        : 1;
        uint32_t erlysuspend    : 1;
        uint32_t usbsuspend     : 1;
        uint32_t usbreset       : 1;
        uint32_t enumdone       : 1;
        uint32_t isooutdrop     : 1;
        uint32_t eopframe       : 1;
        uint32_t res16          : 1;
        uint32_t epmismatch     : 1;
        uint32_t inepintr       : 1;
        uint32_t outepintr      : 1;
        uint32_t incomplisoin   : 1;
        uint32_t incomplisoout  : 1;
        uint32_t res22_23       : 2;
        uint32_t portintr       : 1;
        uint32_t hcintr         : 1;
        uint32_t ptxfempty      : 1;
        uint32_t res27          : 1;
        uint32_t conidstschng   : 1;
        uint32_t disconnect     : 1;
        uint32_t sessreqintr    : 1;
        uint32_t wkupintr       : 1;
    } b;
} USB_STM32F407_GINTMSK;

typedef union _usb_stm32f407_gintsts
{
    uint32_t d32;
    struct
    {
        uint32_t curmode        : 1;
        uint32_t modemismatch   : 1;
        uint32_t otgintr        : 1;
        uint32_t sofintr        : 1;
        uint32_t rxstsqlvl      : 1;
        uint32_t nptxfempty     : 1;
        uint32_t ginnakeff      : 1;
        uint32_t goutnakeff     : 1;
        uint32_t res8           : 1;
        uint32_t i2cintr        : 1;
        uint32_t erlysuspend    : 1;
        uint32_t usbsuspend     : 1;
        uint32_t usbreset       : 1;
        uint32_t enumdone       : 1;
        uint32_t isooutdrop     : 1;
        uint32_t eopframe       : 1;
        uint32_t intimerrx      : 1;
        uint32_t epmismatch     : 1;
        uint32_t inepint        : 1;
        uint32_t outepintr      : 1;
        uint32_t incomplisoin   : 1;
        uint32_t incomplisoout  : 1;
        uint32_t res22_23       : 2;
        uint32_t portintr       : 1;
        uint32_t hcintr         : 1;
        uint32_t ptxfempty      : 1;
        uint32_t res27          : 1;
        uint32_t conidstschng   : 1;
        uint32_t disconnect     : 1;
        uint32_t sessreqintr    : 1;
        uint32_t wkupintr       : 1;
    } b;
} USB_STM32F407_GINTSTS;

typedef union _usb_stm32f407_drxsts
{
    uint32_t d32;
    struct
    {
        uint32_t epnum          : 4;
        uint32_t bcnt           : 11;
        uint32_t dpid           : 2;
        uint32_t pktsts         : 4;
        uint32_t fn             : 4;
        uint32_t res            : 7;
    } b;
} USB_STM32F407_DRXSTS;

typedef union _usb_stm32f407_grxsts
{
    uint32_t d32;
    struct
    {
        uint32_t chnum          : 4;
        uint32_t bcnt           : 11;
        uint32_t dpid           : 2;
        uint32_t pktsts         : 4;
        uint32_t res            : 11;
    } b;
} USB_STM32F407_GRXSTS;

typedef union _usb_stm32f407_fsiz
{
    uint32_t d32;
    struct
    {
        uint32_t startaddr      : 16;
        uint32_t depth          : 16;
    } b;
} USB_STM32F407_FSIZ;

typedef union _usb_stm32f407_hnptxsts
{
    uint32_t d32;
    struct
    {
        uint32_t nptxfspcavail  : 16;
        uint32_t nptxqspcavail  : 8;
        uint32_t nptxqtop_terminate     : 1;
        uint32_t nptxqtop_timer : 2;
        uint32_t nptxqtop       : 2;
        uint32_t chnum          : 2;
        uint32_t res            : 1;
    } b;
} USB_STM32F407_HNPTXSTS;

typedef union _usb_stm32f407_dtxfsts
{
    uint32_t d32;
    struct
    {
        uint32_t txfspcavail    : 16;
        uint32_t res            : 16;
    } b;
} USB_STM32F407_DTXFSTS;

typedef union _usb_stm32f407_gi2cctl
{
    uint32_t d32;
    struct
    {
        uint32_t rwdata         : 8;
        uint32_t regaddr        : 8;
        uint32_t addr           : 7;
        uint32_t i2cen          : 1;
        uint32_t ack            : 1;
        uint32_t i2csuspctl     : 1;
        uint32_t i2cdevaddr     : 2;
        uint32_t dat_se0        : 1;
        uint32_t res            : 1;
        uint32_t rw             : 1;
        uint32_t bsydne         : 1;
    } b;
} USB_STM32F407_GI2CCTL;

typedef union _usb_stm32f407_gccfg
{
    uint32_t d32;
    struct
    {
        uint32_t res_in         : 16;
        uint32_t pwdn           : 1;
        uint32_t i2cifen        : 1;
        uint32_t vbussensingA   : 1;
        uint32_t vbussensingB   : 1;
        uint32_t sofouten       : 1;
        uint32_t disablevbussensing     : 1;
        uint32_t res_out        : 10;
    } b;
} USB_STM32F407_GCCFG;

typedef union _usb_stm32f407_dcfg
{
    uint32_t d32;
    struct
    {
        uint32_t devspd         : 2;
        uint32_t nzstsouthshk   : 1;
        uint32_t res3           : 1;
        uint32_t devaddr        : 7;
        uint32_t perfrint       : 2;
        uint32_t res13_17       : 5;
        uint32_t epmscnt        : 4;
        uint32_t pad            : 10;
    } b;
} USB_STM32F407_DCFG;

typedef union _usb_stm32f407_dctl
{
    uint32_t d32;
    struct
    {
        uint32_t rmtwkupsig     : 1;
        uint32_t sftdiscon      : 1;
        uint32_t gnpinnaksts    : 1;
        uint32_t goutnaksts     : 1;
        uint32_t tstctl         : 3;
        uint32_t sgnpinnak      : 1;
        uint32_t cgnpinnak      : 1;
        uint32_t sgoutnak       : 1;
        uint32_t cgoutnak       : 1;
        uint32_t res            : 21;
    } b;
} USB_STM32F407_DCTL;

typedef union _usb_stm32f407_dsts
{
    uint32_t d32;
    struct
    {
        uint32_t suspsts        : 1;
        uint32_t enumspd        : 2;
        uint32_t errticerr      : 1;
        uint32_t res4_7         : 4;
        uint32_t soffn          : 14;
        uint32_t res22_31       : 10;
    } b;
} USB_STM32F407_DSTS;

typedef union _usb_stm32f407_diepint
{
    uint32_t d32;
    struct
    {
        uint32_t xfercompl      : 1;
        uint32_t epdisabled     : 1;
        uint32_t ahberr         : 1;
        uint32_t timeout        : 1;
        uint32_t intktxfemp     : 1;
        uint32_t intknepmis     : 1;
        uint32_t inepnakeff     : 1;
        uint32_t emptyintr      : 1;
        uint32_t txfifoundrn    : 1;
        uint32_t res08_31       : 23;
    } b;
} USB_STM32F407_DIEPINT;

typedef union _usb_stm32f407_diepint USB_STM32F407_DIEPMSK;

typedef union _usb_stm32f407_doepint
{
    uint32_t d32;
    struct
    {
        uint32_t xfercompl      : 1;
        uint32_t epdisabled     : 1;
        uint32_t ahberr         : 1;
        uint32_t setup          : 1;
        uint32_t res04_31       : 28;
    } b;
} USB_STM32F407_DOEPINT;

typedef union _usb_stm32f407_doepint   USB_STM32F407_DOEPMSK;

typedef union _usb_stm32f407_daint
{
    uint32_t d32;
    struct
    {
        uint32_t in             : 16;
        uint32_t out            : 16;
    } ep;
} USB_STM32F407_DAINT;

typedef union _usb_stm32f407_dthrctl
{
    uint32_t d32;
    struct
    {
        uint32_t non_iso_thr_en : 1;
        uint32_t iso_thr_en     : 1;
        uint32_t tx_thr_len     : 9;
        uint32_t res11_15       : 5;
        uint32_t rx_thr_en      : 1;
        uint32_t rx_thr_len     : 9;
        uint32_t res26_31       : 6;
    } b;
} USB_STM32F407_DTHRCTL;

typedef union _usb_stm32f407_depctl
{
    uint32_t d32;
    struct
    {
        uint32_t mps            : 11;
        uint32_t res            : 4;
        uint32_t usbactep       : 1;
        uint32_t dpid           : 1;
        uint32_t naksts         : 1;
        uint32_t eptype         : 2;
        uint32_t snp            : 1;
        uint32_t stall          : 1;
        uint32_t txfnum         : 4;
        uint32_t cnak           : 1;
        uint32_t snak           : 1;
        uint32_t setd0pid       : 1;
        uint32_t setd1pid       : 1;
        uint32_t epdis          : 1;
        uint32_t epena          : 1;
    } b;
} USB_STM32F407_DEPCTL;

typedef union _usb_stm32f407_depxfrsiz
{
    uint32_t d32;
    struct
    {
        uint32_t xfersize       : 19;
        uint32_t pktcnt         : 10;
        uint32_t mc             : 2;
        uint32_t res            : 1;
    } b;
} USB_STM32F407_DEPXFRSIZ;

typedef union _usb_stm32f407_dep0rfrsiz
{
    uint32_t d32;
    struct
    {
        uint32_t xfersize       : 7;
        uint32_t res7_18        : 12;
        uint32_t pktcnt         : 2;
        uint32_t res20_28       : 9;
        uint32_t supcnt         : 2;
    } b;
} USB_STM32F407_DEP0XFRSIZ;

typedef union _usb_stm32f407_hcfg
{
    uint32_t d32;
    struct
    {
        uint32_t fslspclksel    : 2;
        uint32_t fslssupp       : 1;
        uint32_t pad            : 29;
    } b;
} USB_STM32F407_HCFG;

typedef union _usb_stm32f407_hfrminintrvl
{
    uint32_t d32;
    struct
    {
        uint32_t frint          : 16;
        uint32_t res            : 16;
    } b;
} USB_STM32F407_HFRMINTRVL;

typedef union _usb_stm32f407_hfnum
{
    uint32_t d32;
    struct
    {
        uint32_t frnum          : 16;
        uint32_t frrem          : 16;
    } b;
} USB_STM32F407_HFNUM;

typedef union _usb_stm32f407_hptxsts
{
    uint32_t d32;
    struct
    {
        uint32_t ptxfspcavail   : 16;
        uint32_t ptxqspcavail   : 8;
        uint32_t ptxqtop_terminate  : 1;
        uint32_t ptxqtop_timer  : 2;
        uint32_t ptxqtop        : 2;
        uint32_t chnum          : 2;
        uint32_t ptxqtop_odd    : 1;
    } b;
} USB_STM32F407_HPTXSTS;

typedef union _usb_stm32f407_hprt0
{
    uint32_t d32;
    struct
    {
        uint32_t prtconnsts     : 1;
        uint32_t prtconndet     : 1;
        uint32_t prtena         : 1;
        uint32_t prtenchng      : 1;
        uint32_t prtovrcurract  : 1;
        uint32_t prtovrcurrchng : 1;
        uint32_t prtres         : 1;
        uint32_t prtsusp        : 1;
        uint32_t prtrst         : 1;
        uint32_t res9           : 1;
        uint32_t prtlnsts       : 2;
        uint32_t prtpwr         : 1;
        uint32_t prttstctl      : 4;
        uint32_t prtspd         : 2;
        uint32_t res19_31       : 13;
    } b;
} USB_STM32F407_HPRT0;

typedef union _usb_stm32f407_haint
{
    uint32_t d32;
    struct
    {
        uint32_t chint          : 16;
        uint32_t res            : 16;
    } b;
} USB_STM32F407_HAINT;

typedef union _usb_stm32f407_haintmsk
{
    uint32_t d32;
    struct
    {
        uint32_t chint          : 16;
        uint32_t res            : 16;
    } b;
} USB_STM32F407_HAINTMSK;

typedef union _usb_stm32f407_hcchar
{
    uint32_t d32;
    struct
    {
        uint32_t mps            : 11;
        uint32_t epnum          : 4;
        uint32_t epdir          : 1;
        uint32_t res            : 1;
        uint32_t lspddev        : 1;
        uint32_t eptype         : 2;
        uint32_t multicnt       : 2;
        uint32_t devaddr        : 7;
        uint32_t oddfrm         : 1;
        uint32_t chdis          : 1;
        uint32_t chen           : 1;
    } b;
} USB_STM32F407_HCCHAR;

typedef union _usb_stm32f407_hcsplt
{
    uint32_t d32;
    struct
    {
        uint32_t prtaddr        : 7;
        uint32_t hubaddr        : 7;
        uint32_t xactpos        : 2;
        uint32_t compsplt       : 1;
        uint32_t res            : 14;
        uint32_t spltena        : 1;
    } b;
} USB_STM32F407_HCSPLT;

typedef union _usb_stm32f407_hcint
{
    uint32_t d32;
    struct
    {
        uint32_t xfercompl      : 1;
        uint32_t chhltd         : 1;
        uint32_t ahberr         : 1;
        uint32_t stall          : 1;
        uint32_t nak            : 1;
        uint32_t ack            : 1;
        uint32_t nyet           : 1;
        uint32_t xacterr        : 1;
        uint32_t bblerr         : 1;
        uint32_t frmovrun       : 1;
        uint32_t datatglerr     : 1;
        uint32_t res            : 21;
    } b;
} USB_STM32F407_HCINT;

typedef union _usb_stm32f407_hctsiz
{
    uint32_t d32;
    struct
    {
        uint32_t xfersize       : 19;
        uint32_t pktcnt         : 10;
        uint32_t pid            : 2;
        uint32_t dopng          : 1;
    } b;
} USB_STM32F407_HCTSIZ;

typedef union _usb_stm32f407_hcgintmsk
{
    uint32_t d32;
    struct
    {
        uint32_t xfercompl      : 1;
        uint32_t chhltd         : 1;
        uint32_t ahberr         : 1;
        uint32_t stall          : 1;
        uint32_t nak            : 1;
        uint32_t ack            : 1;
        uint32_t nyet           : 1;
        uint32_t xacterr        : 1;
        uint32_t bblerr         : 1;
        uint32_t frmovrun       : 1;
        uint32_t datatglerr     : 1;
        uint32_t res            : 21;
    } b;
} USB_STM32F407_HCGINTMSK;

typedef union _usb_stm32f407_pcgcctl
{
    uint32_t d32;
    struct
    {
        uint32_t stoppclk       : 1;
        uint32_t gatehclk       : 1;
        uint32_t res            : 30;
    } b;
} USB_STM32F407_PCGCCTL;

#endif /* CONFIG_USB */
#endif /* _USB_REGISTERS_STM32F407_H_  */
