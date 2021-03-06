#include "button.h"

unsigned long subtract_times(unsigned long t1, unsigned long t2);

unsigned long Button::last_button_press = 0;

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

  // scottnew
  last_advertised_state = cur_advertised_state;
	if(subtract_times(now,last_change_millis) > 50) {
		cur_advertised_state = cur_state;
	}	
	last_read_state = cur_state;
}

//is the calcuated stated different from the last advertised state
bool Button::just_pressed() {
	if(cur_advertised_state != last_advertised_state && is_pressed()) {
		last_button_press = millis();
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
// scottnew
long Button::get_time_pressed() {
  if (is_pressed()) {
    return millis() - last_change_millis;
  }
  return 0;
}

unsigned long Button::get_last_button_press() {
	return last_button_press; 
}

void Button::reset_last_button_press() {
	last_button_press = millis();
}

