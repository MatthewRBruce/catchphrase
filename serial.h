
#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_CONSOLE_ENABLE 0

void print(String msg) {
  if (SERIAL_CONSOLE_ENABLE) {
    Serial.print(msg);
  }
}
void println(String msg) {
  if (SERIAL_CONSOLE_ENABLE) {
    Serial.println(msg);
  }
}
void print(int num) {
  if (SERIAL_CONSOLE_ENABLE) {
    Serial.print(num);
  }
}
void println(int num) {
  if (SERIAL_CONSOLE_ENABLE) {
    Serial.println(num);
  }
}

#endif

