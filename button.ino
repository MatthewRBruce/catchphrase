#include "button.h"

unsigned long subtract_times(unsigned long t1, unsigned long t2);

Button::Button (byte PIN) {
    this->PIN = PIN;
    this->last_advertised_state = 1;
    this->last_read_state = 1;
    this->last_change_millis = 0;
}


void Button::update_advertised_state() {
    byte cur_state = digitalRead(this->PIN);
    long now = millis();
    if (cur_state != last_read_state) {
      last_change_millis = now;        
    }

    if(subtract_times(now,last_change_millis) > 50) {
      last_advertised_state = cur_advertised_state;
      cur_advertised_state = cur_state;
    }  
    last_read_state = cur_state;
  }

//is the calcuated stated different from the last advertised state
bool Button::just_pressed() {
    if(cur_advertised_state != last_advertised_state && is_pressed()) {
      return true;
    }
    return false;
  }

bool Button::just_released() {
    if(cur_advertised_state != last_advertised_state && is_released()) {
      return true;
    }
    return false;
}

bool Button::is_pressed() {
    return cur_advertised_state == 0;
}
bool Button::is_released() {
    return cur_advertised_state == 1;
}

