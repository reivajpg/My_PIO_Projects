#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tusb_hid.h"
#include "tinyusb.h"

void app_main() {
    // Configuración de TinyUSB
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    // Esperar a que la conexión USB esté lista
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1) {
        uint8_t keycode[6] = {0};
        keycode[0] = HID_KEY_A;  // Enviar la tecla 'A'

        tud_hid_keyboard_report(0, 0, keycode);
        vTaskDelay(pdMS_TO_TICKS(500));

        tud_hid_keyboard_report(0, 0, NULL);  // Soltar la tecla
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
