#include <BleGamepad.h>

BleGamepad bleGamepad("Mando Retro C3", "Fabricante DIY", 100);

// Definición de pines
const int pinUp = 9;
const int pinDown = 1;
const int pinLeft = 2;
const int pinRight = 3;

void setup() {
  pinMode(pinUp, INPUT_PULLUP);
  pinMode(pinDown, INPUT_PULLUP);
  pinMode(pinLeft, INPUT_PULLUP);
  pinMode(pinRight, INPUT_PULLUP);
  
  bleGamepad.begin();
}

void loop() {
  if (bleGamepad.isConnected()) {
    bool up = !digitalRead(pinUp);
    bool down = !digitalRead(pinDown);
    bool left = !digitalRead(pinLeft);
    bool right = !digitalRead(pinRight);

    // Lógica para el Hat Switch (Cruceta)
    if (up && right) bleGamepad.setHat1(DPAD_UP_RIGHT);
    else if (up && left) bleGamepad.setHat1(DPAD_UP_LEFT);
    else if (down && right) bleGamepad.setHat1(DPAD_DOWN_RIGHT);
    else if (down && left) bleGamepad.setHat1(DPAD_DOWN_LEFT);
    else if (up) bleGamepad.setHat1(DPAD_UP);
    else if (down) bleGamepad.setHat1(DPAD_DOWN);
    else if (left) bleGamepad.setHat1(DPAD_LEFT);
    else if (right) bleGamepad.setHat1(DPAD_RIGHT);
    else bleGamepad.setHat1(DPAD_CENTERED);

    bleGamepad.sendReport();
    delay(10); // Pequeña pausa para estabilidad
  }
}
