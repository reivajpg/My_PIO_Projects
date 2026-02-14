# Proyecto Básico STM32L153 (PlatformIO)

Este es un proyecto básico para programar un chip **STM32L153** utilizando PlatformIO y el framework Arduino.

## Configuración de Hardware

El proyecto está configurado para utilizar la definición de placa `nucleo_l152re` como base, ya que el **STM32L153** pertenece a la misma familia (STM32L1) y es compatible a nivel de pines y periféricos básicos.

### Ajustes para tu placa:
1. **LED:** Por defecto usa `PA5` (LED_BUILTIN de Nucleo). Si tu placa tiene el LED en otro pin (ej. `PC13`), edita `src/main.cpp`.
2. **Serial:** Usa el puerto Serial por defecto (usualmente TX=PA2, RX=PA3 conectado al programador ST-Link).
3. **Flash:** El STM32L153 suele tener menos memoria flash (256KB) que el L152RE (512KB). Ten cuidado con programas muy grandes, aunque para este ejemplo básico no hay problema.

## Requisitos
- **PlatformIO** (CLI o extensión de IDE).
- **Programador ST-Link** (V2 o V3) para subir el código al chip.

## Cómo compilar y subir

1. **Compilar:**
   ```bash
   pio run
   ```

2. **Subir al microcontrolador:**
   Conecta tu programador ST-Link a los pines SWDIO, SWCLK, GND y 3.3V del STM32L153.
   ```bash
   pio run --target upload
   ```

## Monitor Serial

Para ver los mensajes de depuración ("LED Encendido", etc.):
```bash
pio device monitor
```
Asegúrate de que la velocidad esté en 115200 baudios.
