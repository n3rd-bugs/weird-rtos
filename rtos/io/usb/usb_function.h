/*
 * usb_function.h
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
#ifndef _USB_FUNCTION_H_
#define _USB_FUNCTION_H_

#include <os.h>

#ifdef USB_FUNCTION

/* USB function configurations. */
//#define USB_FUNCTION_CDC_ACM

/* USB Configuration. */
//#define USB_SUPPORT_STRING_DESC

/* USB function includes. */
#include <usb_fun_device.h>
#include <usb_fun_request.h>
#include <usb_fun_io_request.h>

/* Macro definitions. */
#define USB_LEN_DEV_QUALIFIER_DESC      0x0A
#define USB_LEN_DEV_DESC                0x12
#define USB_LEN_CFG_DESC                0x09
#define USB_LEN_IF_DESC                 0x09
#define USB_LEN_EP_DESC                 0x07
#define USB_LEN_OTG_DESC                0x03

#define USB_DEV_IDX_LANGID_STR          0x00
#define USB_DEV_IDX_MFC_STR             0x01
#define USB_DEV_IDX_PRODUCT_STR         0x02
#define USB_DEV_IDX_SERIAL_STR          0x03
#define USB_DEV_IDX_CONFIG_STR          0x04
#define USB_DEV_IDX_INTERFACE_STR       0x05

#define USB_REQ_TYPE_STANDARD           0x00
#define USB_REQ_TYPE_CLASS              0x20
#define USB_REQ_TYPE_VENDOR             0x40
#define USB_REQ_TYPE_MASK               0x60

#define USB_REQ_RECIPIENT_DEVICE        0x00
#define USB_REQ_RECIPIENT_INTERFACE     0x01
#define USB_REQ_RECIPIENT_ENDPOINT      0x02
#define USB_REQ_RECIPIENT_MASK          0x03

#define USB_REQ_GET_STATUS              0x00
#define USB_REQ_CLEAR_FEATURE           0x01
#define USB_REQ_SET_FEATURE             0x03
#define USB_REQ_SET_ADDRESS             0x05
#define USB_REQ_GET_DESCRIPTOR          0x06
#define USB_REQ_SET_DESCRIPTOR          0x07
#define USB_REQ_GET_CONFIGURATION       0x08
#define USB_REQ_SET_CONFIGURATION       0x09
#define USB_REQ_GET_INTERFACE           0x0A
#define USB_REQ_SET_INTERFACE           0x0B
#define USB_REQ_SYNCH_FRAME             0x0C

#define USB_DESC_TYPE_DEVICE            1
#define USB_DESC_TYPE_CONFIG            2
#define USB_DESC_TYPE_STRING            3
#define USB_DESC_TYPE_INTERFACE         4
#define USB_DESC_TYPE_ENDPOINT          5
#define USB_DESC_TYPE_DEVICE_QUALIFIER  6
#define USB_DESC_TYPE_SPEED_CONFIG      7

#define USB_CONFIG_REMOTE_WAKEUP        2
#define USB_CONFIG_SELF_POWERED         1

#define USB_FEATURE_EP_HALT             0
#define USB_FEATURE_REMOTE_WAKEUP       1
#define USB_FEATURE_TEST_MODE           2

/* User configurations. */
#define USB_FUN_CFG_MAX_NUM             1
#define USB_FUN_ITF_MAX_NUM             1

/* Helper macros. */
#define SWAPBYTE(addr)                  ((uint16_t)(*((uint8_t *)(addr)) + (*((uint8_t *)(addr) + 1) << 8)))
#define LOBYTE(x)                       ((uint8_t)(x & 0x00FF))
#define HIBYTE(x)                       ((uint8_t)((x & 0xFF00) >>8))

/* USB function descriptor callbacks. */
typedef struct _usb_fun_desc
{
  uint8_t *(*get_device_desc)(uint8_t, uint16_t *);
  uint8_t *(*get_lang_id_desc)(uint8_t, uint16_t *);
  uint8_t *(*get_mfg_str_desc)(uint8_t, uint8_t *, uint16_t *);
  uint8_t *(*get_product_str_desc)(uint8_t, uint8_t *, uint16_t *);
  uint8_t *(*get_serial_number_str_desc)(uint8_t, uint8_t *, uint16_t *);
  uint8_t *(*get_cfg_str_desc)(uint8_t, uint8_t *, uint16_t *);
  uint8_t *(*get_iface_str_desc)(uint8_t, uint8_t *, uint16_t *);
} USB_FUN_DESC_CB;

/* USB function device callbacks. */
typedef struct _usb_fun_cb
{
    uint32_t (*init)(void *, uint8_t);
    uint32_t (*deinit)(void *, uint8_t);

    /* Control endpoints. */
    uint32_t (*setup)(void *, USB_SETUP_REQ  *);
    uint32_t (*ep0_tx_sent)(void *);
    uint32_t (*ep0_rx_ready)(void *);

    /* Class Specific endpoints. */
    uint32_t (*data_in)(void *, uint8_t);
    uint32_t (*data_out)(void *, uint8_t);
    uint32_t (*sof)(void *);
    uint32_t (*iso_in_incomplete)(void *);
    uint32_t (*iso_out_incomplete)(void *);

    uint8_t *(*get_config_descriptor)(uint8_t, uint16_t *);
    uint8_t *(*get_other_config_descriptor)(uint8_t, uint16_t *);
#ifdef USB_SUPPORT_STRING_DESC
    uint8_t *(*get_usr_str_descriptor)(uint8_t, uint16_t, uint16_t *);
#endif

    /* Descriptor callbacks. */
    USB_FUN_DESC_CB     *desc_cb;
} USB_FUN_CB;

/* User callback definition. */
typedef struct _usb_fun_user_cb
{
    void (*init)(void *);
    void (*reset)(void *, uint8_t);
    void (*configured)(void *);
    void (*suspended)(void *);
    void (*resumed)(void *);

    void (*connected)(void *);
    void (*disconnected)(void *);
} USB_FUN_USER_CB;

/* USB function request callbacks. */
extern USB_FUN_REQ_CB *usb_function_requset_cb;

/* Function prototypes. */
void usb_function_init(USB_STM32F407_HANDLE *, USB_FUN_CB *);

#ifdef USB_FUNCTION_CDC_ACM
#include <usb_fun_cdc_acm.h>
#endif

#endif /* USB_FUNCTION */
#endif /* _USB_FUNCTION_H_ */
