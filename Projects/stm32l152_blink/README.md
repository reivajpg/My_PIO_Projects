# Proyecto Básico STM32L152RCT6 (PlatformIO)

Este es un proyecto básico para programar un chip **STM32L152RCT6** (256KB Flash / 32KB RAM) utilizando PlatformIO y el framework Arduino.

## Configuración de Hardware

Este proyecto usa la definición de la placa `nucleo_l152re` como base (familia STM32L1), pero se ha configurado en `platformio.ini` para que compila y suba correctamente al **STM32L152RCT6**.

### Ajustes importantes:
1. **Microcontrolador:** Se ha forzado a `board_build.mcu = stm32l152rct6`.
2. **Memoria:** Se han ajustado los límites de Flash (256KB) y RAM (32KB).
3. **LED:** Por defecto usa el pin definido en la variante de Nucleo (PA5). Si tu placa personalizada tiene el LED en otro pin (ej. `PC13`), edítalo en `src/main.cpp`.

## Cómo conectar el ST-Link V2

Para programar el chip, necesitas conectar un programador ST-Link V2 (original o clon) a los siguientes pines del STM32L152:

| ST-Link V2 | STM32L152RCT6 | Notas |
| :--- | :--- | :--- |
| **3.3V** | VCC / 3.3V | **¡IMPORTANTE!** No conectes 5V. |
| **GND** | GND | Tierra común. |
| **SWDIO** | PA13 | Pin de datos SWD. |
| **SWCLK** | PA14 | Pin de reloj SWD. |
| **RST** | NRST | (Opcional pero recomendado) Pin de Reset. |

> **Nota:** Si tu programador es un clon chino y no tiene pin de Reset, conecta solo SWDIO, SWCLK, GND y VCC. Si falla la subida, intenta mantener presionado el botón de Reset de tu placa, inicia la subida y suéltalo justo cuando veas "Open On-Chip Debugger" en la terminal.

## Solución de Problemas de Subida (Upload Issues)

Si obtienes errores como `Error: init mode failed` o `target not halted`:

1. **Revisa las conexiones:** Asegúrate de que SWDIO y SWCLK no estén intercambiados.
2. **Pin de Reset:** Si es posible, conecta el pin NRST del chip al RST del ST-Link.
3. **Configuración de OpenOCD:**
   Si usas un clon de ST-Link V2, es posible que necesites cambiar la configuración de subida en `platformio.ini`. Descomenta las líneas de `upload_flags` sugeridas en el archivo `platformio.ini`.

   Ejemplo común para clones:
   ```ini
   upload_flags = -c transport select hla_swd
   ```

4. **Boot0:** Asegúrate de que el pin BOOT0 esté conectado a GND a través de una resistencia (o directamente) para arrancar desde la Flash principal.

## Cómo compilar y subir

1. **Compilar:**
   ```bash
   pio run
   ```

2. **Subir:**
   ```bash
   pio run --target upload
   ```

3. **Monitor Serial:**
   ```bash
   pio device monitor
   ```
