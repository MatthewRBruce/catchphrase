byte TEAM1_PIN = 2;
byte TEAM2_PIN = 3;
byte START_STOP_PIN = 18;
byte NEXT_PIN = 19;
byte CATEGORY_PIN = 20;




class Button {

  public:
    Button(byte PIN);
    byte PIN;
    byte last_advertised_state;
    byte cur_advertised_state;
    long last_change_millis;
    bool just_pressed();
    bool just_released();

  private:
    byte last_read_state;
    void update_advertised_state();
};  

Button::Button (byte PIN) {
    this->PIN = PIN;
    this->last_advertised_state = 1;
    this->last_read_state = 1;
    this->last_change_millis = 0;
}


void Button::update_advertised_state() {
    byte cur_state = digitalRead(this->PIN);
    if (cur_state != last_read_state) {
      last_change_millis = millis();        
    }

    if(millis() - last_change_millis > 50) {
      last_advertised_state = cur_advertised_state;
      cur_advertised_state = cur_state;
    }  
    last_read_state = cur_state;
  }

//is the calcuated stated different from the last advertised state
bool Button::just_pressed() {
    update_advertised_state();
    if(cur_advertised_state != last_advertised_state && cur_advertised_state == 0) {
      return true;
    }
    return false;
  }

bool Button::just_released() {
    update_advertised_state();
    if(cur_advertised_state != last_advertised_state && cur_advertised_state == 1) {
      return true;
    }
    return false;
}

Button button_team1(2);
Button button_team2(3);
Button button_start_stop(18);
Button button_next(19);
Button button_category(20);

int score_team1 = 0;
int score_team2 = 0;

enum GAME_STATES {CATEGORY_SELECTION,IN_ROUND,ROUND_DONE,GAME_DONE};

GAME_STATES game_state = NULL;

String categories[] = {"Everything", "People", "World"};
int NUM_CATEGORIES = 3;
int cur_category = 0;

//bool check_other_button_depressed() {
//  return team1_depressed && team1_depressed && start_stop_depressed && next_depressed && next_depressed;/
//}

void setup() {
  Serial.begin(9600);
  Serial.println("Catch Phrase - Power On");
  delay(2000);// Give reader a chance to see the output.

  game_state = CATEGORY_SELECTION;

  pinMode(TEAM1_PIN,INPUT_PULLUP);
  pinMode(TEAM2_PIN,INPUT_PULLUP);
  pinMode(START_STOP_PIN,INPUT_PULLUP);
  pinMode(NEXT_PIN,INPUT_PULLUP);
  pinMode(CATEGORY_PIN,INPUT_PULLUP);


  digitalWrite(TEAM1_PIN,HIGH);
  digitalWrite(TEAM2_PIN,HIGH);
  digitalWrite(START_STOP_PIN,HIGH);
  digitalWrite(NEXT_PIN,HIGH);
  digitalWrite(CATEGORY_PIN,HIGH);
  
}

void updateDisplay(String displayString) {
  Serial.print(score_team1);
  Serial.print(displayString);
  Serial.println(score_team2);  
}

void rotate_category() {
  if(++cur_category >= NUM_CATEGORIES) {
    cur_category = 0;
  }
}
void start_new_round() {
  //Pick a question
  //Update display
  //set beep frequency
  //play first beep?
  game_state = IN_ROUND;
}


void loop() {

    switch (game_state) {
      case CATEGORY_SELECTION:
        if(button_category.just_pressed()) {
          rotate_category();
          updateDisplay(categories[cur_category]);
        }
        if (button_start_stop.just_pressed()) {
          start_new_round();

        }
        break;
      case IN_ROUND:
        break;
      case ROUND_DONE:
        break;
       case GAME_DONE:
        break;
    }

  // put your main code here, to run repeatedly:

}
