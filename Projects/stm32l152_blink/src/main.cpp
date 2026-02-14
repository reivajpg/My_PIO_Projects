#include <Arduino.h>

/*
 * Ejemplo básico para STM32L152RCT6 (Configurado como variante de Nucleo L152RE)
 * Este programa hace parpadear un LED conectado al pin definido como LED_BUILTIN.
 *
 * NOTA IMPORTANTES SOBRE EL HARDWARE:
 * 1. La placa 'nucleo_l152re' define LED_BUILTIN como PA5 (Pin D13 en formato Arduino).
 * 2. Si estás usando un chip STM32L152RCT6 en una placa personalizada y tu LED está en otro pin
 *    (por ejemplo, PC13, PB6, etc.), debes cambiar `LED_BUILTIN` por el número de pin correspondiente.
 *    Ejemplo: #define MI_LED PC13
 */

// Si necesitas definir un pin específico para tu placa, descomenta y edita la siguiente línea:
// #define LED_PIN PC13
// Y usa LED_PIN en lugar de LED_BUILTIN en el código.

#ifndef LED_BUILTIN
  #define LED_BUILTIN PA5 // Valor por defecto si no está definido (usualmente PA5 o PC13)
#endif

void setup() {
  // Inicializar el pin del LED como salida
  pinMode(LED_BUILTIN, OUTPUT);

  // Inicializar comunicación serial para depuración (UART por defecto)
  // En Nucleo L152RE esto suele ser TX=PA2, RX=PA3 conectado al ST-Link VCP
  Serial.begin(115200);
  while (!Serial) {
    ; // Esperar a que el puerto serial se conecte (solo necesario para USB nativo)
  }

  Serial.println("Iniciando ejemplo Blink en STM32L152RCT6...");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // Encender LED (Voltaje Alto)
  // Nota: En algunas placas el LED se enciende con LOW. Si es tu caso, invierte HIGH/LOW.

  Serial.println("LED Encendido");
  delay(1000);                      // Esperar 1000 milisegundos (1 segundo)

  digitalWrite(LED_BUILTIN, LOW);   // Apagar LED (Voltaje Bajo)

  Serial.println("LED Apagado");
  delay(1000);                      // Esperar 1 segundo
}
