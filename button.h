#define byte uint8_t
#ifndef BUTTON_H
#define BUTTON_H

class Button {

  public:
    Button(byte PIN);
    byte PIN;
    byte last_advertised_state;
    byte cur_advertised_state;
    long last_change_millis;
    void update_advertised_state();
    bool just_pressed();
    bool just_released();
    bool is_pressed();
    bool is_released();

  private:
    byte last_read_state;
};  

#endif
