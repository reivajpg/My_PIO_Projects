#define CFG_TUSB_MCU OPT_MCU_ESP32C3
#include <Adafruit_TinyUSB.h>

Adafruit_USBD_HID usb_hid;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    usb_hid.begin();
    Serial.println("USB HID inicializado");

    // Esperar a que el dispositivo USB sea reconocido
    while (!TinyUSBDevice.mounted()) {
        Serial.println("Esperando conexión USB...");
        delay(500);
    }
    Serial.println("USB listo!");
}

void loop() {
    uint8_t keycode[6] = {0};
    keycode[0] = HID_KEY_A;  // Enviar la tecla 'A'

    usb_hid.keyboardReport(0, 0, keycode);
    delay(500);
    
    usb_hid.keyboardRelease();
    delay(1000);
}
