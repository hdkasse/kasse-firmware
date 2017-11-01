
#include "usbd_customhid.h"
#include "usb_user.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

usb_rx_callback_t user_rx_callback = NULL;

void usb_set_rx_callback(usb_rx_callback_t callback)
{
    user_rx_callback = callback;
}

void usb_tx_helper(uint8_t *message, uint32_t len)
{
    uint8_t tmp_buffer[CUSTOM_HID_EPIN_SIZE] = { 0 };

    memcpy(tmp_buffer, message, MIN(len,CUSTOM_HID_EPIN_SIZE));
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, tmp_buffer, CUSTOM_HID_EPIN_SIZE);

}


void usb_poll(void)
{
    // TBD......
}

