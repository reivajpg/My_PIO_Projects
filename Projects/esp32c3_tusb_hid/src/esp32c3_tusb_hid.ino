#define CFG_TUSB_MCU OPT_MCU_ESP32C3
#include <Adafruit_TinyUSB.h>

Adafruit_USBD_HID usb_hid;

void setup() {
    usb_hid.begin();
    delay(1000);
}

void loop() {
    uint8_t keycode[6] = {0};
    keycode[0] = HID_KEY_A;  // Enviar la tecla 'A'

    usb_hid.keyboardReport(0, 0, keycode);
    delay(500);
    
    usb_hid.keyboardRelease();
    delay(1000);
}
