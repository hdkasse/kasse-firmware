
#ifndef __usb_user_H
#define __usb_user_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "usbd_customhid.h"

#define MAX_MESSAGE_SIZE CUSTOM_HID_EPIN_SIZE

typedef struct
{
    uint32_t len;
    uint8_t message[MAX_MESSAGE_SIZE];
} UsbMessage;

typedef void (*usb_rx_callback_t)(UsbMessage* msg);

void usb_set_rx_callback(usb_rx_callback_t callback);
void usb_tx_helper(uint8_t *message, uint32_t len);

void usb_poll(void);

#ifdef __cplusplus
}
#endif
#endif /*__usb_user_H */

