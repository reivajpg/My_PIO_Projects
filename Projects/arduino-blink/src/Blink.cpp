/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>

// Set LED_BUILTIN if it is not defined by Arduino framework
#ifndef LED_BUILTIN
    #define LED_BUILTIN 8
#endif

#define LED 8

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED, OUTPUT);

  Serial.begin(115200);
}

void loop()
{
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED, HIGH);
  Serial.println("turn the LED on (HIGH is the voltage level)");
  // wait for a second
  delay(1000);
  Serial.println("wait for a second");
  // turn the LED off by making the voltage LOW
  digitalWrite(LED, LOW);
  Serial.println("turn the LED off by making the voltage LOW");
   // wait for a second
  delay(1000);
  Serial.println("wait for a second");
}
