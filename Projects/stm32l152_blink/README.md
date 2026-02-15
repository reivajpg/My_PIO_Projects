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

## Solución de Problemas de Subida (Upload Troubleshooting)

Si has podido subir el código manualmente con **STM32CubeProgrammer** pero falla desde PlatformIO, prueba lo siguiente:

### Opción 1: Usar el entorno `nucleo_l152re_cube`

PlatformIO soporta directamente la herramienta de línea de comandos de STM32CubeProgrammer. Si ya tienes instalada esta herramienta y añadida a tu PATH del sistema, puedes usar este entorno.

**Compilar y subir con STM32CubeProgrammer:**
```bash
pio run -e nucleo_l152re_cube --target upload
```

### Opción 2: Ajustar OpenOCD (Clones ST-Link V2)

Si usas un clon chino de ST-Link V2 y obtienes errores como `Error: init mode failed` o `target not halted`, puedes intentar ajustar la configuración de OpenOCD en `platformio.ini` (entorno `nucleo_l152re_stlink`).

Descomenta estas líneas en `platformio.ini`:
```ini
; Forzar modo HLA (High Level Adapter) que funciona mejor con clones:
upload_flags = -c transport select hla_swd
```
O también probar forzando el ID del chip si no lo detecta automáticamente:
```ini
upload_flags = -c set CPUTAPID 0x2ba01477
```

**Compilar y subir con OpenOCD (por defecto):**
```bash
pio run -e nucleo_l152re_stlink --target upload
```

## Monitor Serial

```bash
pio device monitor
```
