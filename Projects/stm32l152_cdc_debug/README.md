# Proyecto Base STM32L152RCT6 con USB CDC (Virtual COM) - TempTale Ultra

Este proyecto sirve como plantilla robusta para el desarrollo con el microcontrolador **STM32L152RCT6** (256KB Flash / 32KB RAM) utilizando PlatformIO y el framework STM32Cube.

**Origen del Hardware:** Este proyecto se basa en la reutilización del dispositivo **TempTale Ultra** de **SENSITECH**. Estos monitores de temperatura son comúnmente desechados tras su uso, por lo que este proyecto busca aprovechar esta "basura electrónica" (e-waste) convirtiéndolos en placas de desarrollo funcionales.

La característica principal es la implementación funcional del puerto **USB CDC (Virtual COM Port)** para depuración mediante `printf`, resolviendo problemas comunes de bloqueo y gestión de memoria.

## Características Implementadas

1.  **USB CDC (Device):**
    *   Integración del Middleware USB de ST mediante script de compilación (`add_usb_middleware.py`).
    *   Configuración de reloj para soportar USB (PLL a 96MHz / 2 = 48MHz).
2.  **Depuración por Puerto Serie:**
    *   Redirección de la función `printf` al puerto USB.
    *   Implementación de `__io_putchar` con **buffer intermedio** y **timeout** para evitar bloqueos si el cable se desconecta o el host no lee.
3.  **Manejo de Entradas (Botones):**
    *   Lógica de lectura por **Polling** (encuesta) en el bucle principal.
    *   Se evita el uso de interrupciones externas (EXTI) para los botones para prevenir conflictos de prioridad y "deadlocks" con las interrupciones del USB.

## Estructura del Proyecto

*   `src/main.c`: Bucle principal que lee los botones (Start/Stop), controla LEDs y envía mensajes por USB.
*   `src/usb_device.c` & `src/usbd_cdc_if.c`: Configuración y callbacks del dispositivo USB.
*   `add_usb_middleware.py`: Script de Python para incluir las librerías USB de STM32Cube en la compilación de PlatformIO.
*   `platformio.ini`: Configuración del entorno, flags de compilación y scripts extra.

## Configuración de Hardware

Este proyecto está configurado para el chip **STM32L152RCT6** presente en el TempTale Ultra.

### Conexión ST-Link V2 (Programación)

| ST-Link V2 | STM32L152RCT6 | Notas |
| :--- | :--- | :--- |
| **3.3V** | VCC / 3.3V | **¡IMPORTANTE!** No conectes 5V. |
| **GND** | GND | Tierra común. |
| **SWDIO** | PA13 | Pin de datos SWD. |
| **SWCLK** | PA14 | Pin de reloj SWD. |
| **RST** | NRST | Recomendado. |

### Periféricos

*   **USB:** Pines PA11 (DM) y PA12 (DP).
*   **LED Verde:** PB3 (Configurable en `main.c`).
*   **LED Rojo:** PB4 (Configurable en `main.c`).
*   **Botón Start:** PA0 (Pull-up interno).
*   **Botón Stop:** PA1 (Pull-up interno).

## Cómo compilar y usar

1.  **Clonar y Abrir:** Abre la carpeta del proyecto en VSCode con PlatformIO.
2.  **Compilar:**
    ```bash
    pio run
    ```
    *El script `add_usb_middleware.py` se ejecutará automáticamente para incluir las librerías USB.*
3.  **Subir:**
    Conecta el ST-Link y ejecuta:
    ```bash
    pio run --target upload
    ```
4.  **Probar:**
    *   Conecta el puerto USB del STM32 a la PC.
    *   Abre un monitor serie (ej. `pio device monitor` o Putty) en el puerto COM generado.
    *   Presiona los botones y observa los mensajes de depuración.
