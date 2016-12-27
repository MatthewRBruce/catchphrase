
#include <LiquidCrystal.h>
#include "display.h"

#define MAX_LONG 4294967295

byte TEAM1_PIN = 2;
byte TEAM2_PIN = 3;
byte START_STOP_PIN = 18;
byte NEXT_PIN = 19;
byte CATEGORY_PIN = 20;
byte SPEAKER_PIN = 7;

// Picked these randomly - Scott
byte LCD_PIN_RS = 10;
byte LCD_PIN_E  = 11;
byte LCD_PIN_D4 = 12;
byte LCD_PIN_D5 = 13;
byte LCD_PIN_D6 = 14;
byte LCD_PIN_D7 = 15;

unsigned long subtract_times(unsigned long t1, unsigned long t2);

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

Button button_team1(TEAM1_PIN);
Button button_team2(TEAM2_PIN);
Button button_start_stop(START_STOP_PIN);
Button button_next(NEXT_PIN);
Button button_category(CATEGORY_PIN);

// Pins: RS,E,D4,D5,D6,D7
LiquidCrystal lcd(
  LCD_PIN_RS,
  LCD_PIN_E,
  LCD_PIN_D4,
  LCD_PIN_D5,
  LCD_PIN_D6,
  LCD_PIN_D7);

int score_team1 = 0;
int score_team2 = 0;

enum GAME_STATES {CATEGORY_SELECTION,IN_ROUND,GAME_DONE};

GAME_STATES game_state = NULL;

String categories[] = {"Everything", "People", "World"};
int NUM_CATEGORIES = 3;
int cur_category = 0;

// For now, just have some clues
String clues[] = {"Big Sur", "Chocolate cake", "Butterfly bandage"};
int NUM_CLUES = 3;
int cur_clue = 0;

// Tic-toc related constants and state variables

// How long does each beep frequency last?
unsigned long beep_frequency_change_interval_millis = 15000;

// What are the waits between each beep?
unsigned long beep_interval_millis[] = {500, 500, 300, 200};

// The number of beep speeds 
int NUM_BEEP_INTERVALS = 4;

// Which speed are we on?
int cur_beep_interval = 0;

// Track whether to tic or toc next
bool next_is_tic = true;

// Last time we did a tic or toc
unsigned long last_tictoc_millis = 0;

// Last time we sped up the tic/toc
unsigned long last_beep_speed_change_millis = 0;


// BEEP!
enum BEEP_TYPE {
  BEEP_TIC,
  BEEP_TOC,
  BEEP_TIMES_UP,
  BEEP_POWER_ON,
  BEEP_CATEGORY_CHANGE,
  BEEP_SCORE_CHANGE,
  BEEP_SCORE_RESET,
  BEEP_WIN_GAME,
  BEEP_STOP_ROUND,
  BEEP_EXIT_GAME_DONE_STATE,
};


//440 -> 490 -> 440 for score reset?

// Just output for now
void play_beep(BEEP_TYPE beep) {
  Serial.print("BEEP - ");  
  switch (beep) {
    case BEEP_TIC:
      analogWrite(SPEAKER_PIN,2);
      delay(100);
      analogWrite(SPEAKER_PIN,0);
      Serial.println("TIC");
      break;
    case BEEP_TOC:
      analogWrite(SPEAKER_PIN,2);
      delay(100);
      analogWrite(SPEAKER_PIN,0);           
      Serial.println("TOC");
      break;
    case BEEP_TIMES_UP:
      Serial.println("TIMES_UP");
      break;
    case BEEP_POWER_ON:
      Serial.println("POWER_ON");
      break;
    case BEEP_CATEGORY_CHANGE:
      Serial.println("CATEGORY_CHANGE");
      break;
    case BEEP_SCORE_CHANGE:
      Serial.println("SCORE_CHANGE");
      break;
    case BEEP_SCORE_RESET:
      Serial.println("SCORE_RESET");
      break;
    case BEEP_WIN_GAME:
      Serial.println("WIN_GAME");
      break;
    case BEEP_STOP_ROUND:
      Serial.println("STOP_ROUND");
      break;
    case BEEP_EXIT_GAME_DONE_STATE:
      Serial.println("EXIT_GAME_DONE_STATE");
      break;
  }
}

unsigned long subtract_times(unsigned long t1, unsigned long t2) {
  //If t1 is less than t2 then we assume t1 has overflowed
  if (t1 < t2) {
    return (MAX_LONG - t2) + t1;
  }
  return t1 - t2;
}


void updateDisplay(String displayString) {

  String formattedText = get_display_text(displayString);

  // TODO: when integrated with SD, check this earlier.  At this point
  // we can't handle it by picking the next clue
  if (formattedText.length() == 0) {
    Serial.print("OOPS! CAN'T DISPLAY: ");
    Serial.println(displayString);
    Serial.print("formatted: ");
    Serial.println(formattedText);
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print(score_team1);
  lcd.print(" ");
  lcd.print(formattedText.substring(0,12));
  lcd.print(" ");
  lcd.print(score_team2); 
  lcd.setCursor(0, 1);
  lcd.print("  ");
  lcd.print(formattedText.substring(12));

  Serial.print(score_team1);
  Serial.print(" ");
  Serial.print(formattedText.substring(0,12));
  Serial.print(" ");
  Serial.println(score_team2); 
  Serial.print("  ");
  Serial.print(formattedText.substring(12));
}


void setup() {
  Serial.begin(9600);
  Serial.println("Catch Phrase - Power On");
  play_beep(BEEP_POWER_ON);
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

  // Initialize the LCD (16 columns, 2 rows)
  lcd.begin(16, 2);
  updateDisplay(categories[cur_category]);
}

void rotate_category() {
  if(++cur_category >= NUM_CATEGORIES) {
    cur_category = 0;
  }
}

// TODO: this has to read from micro SD
void rotate_clue () {
  if(++cur_clue>= NUM_CLUES) {
    cur_clue = 0;
  }
}


void start_new_round() {
  rotate_clue();
  game_state = IN_ROUND;

  // Set up the timer-related stuff
  cur_beep_interval = 0;
  next_is_tic = true;
  last_tictoc_millis = 0; // want it to tic immediately
  last_beep_speed_change_millis = millis(); // since we just changed the frequency

  // Update the display
  updateDisplay(clues[cur_clue]);
}

void end_current_round() {
  play_beep(BEEP_TIMES_UP);
  game_state = CATEGORY_SELECTION;
  // Leave the last clue on the screen
}

// Requires: score_team1 == 7 or score_team2 == 7
void end_game() {
  play_beep(BEEP_WIN_GAME);
  if (score_team1 == 7)
  {
    updateDisplay("Blondes Win!");
  } else {
    updateDisplay("Brunettes Win!");
  }
  game_state = GAME_DONE;
}


void do_tic_toc()
{
  unsigned long now = millis();
  // update frequency and end game if needed
  
  
  if (subtract_times(now, last_beep_speed_change_millis) > beep_frequency_change_interval_millis) {
    last_beep_speed_change_millis = now;
    // Handle the time up case
    if (++cur_beep_interval >= NUM_BEEP_INTERVALS) {
      end_current_round();
      return;
    }
  }

  // Beep if needed
  if (subtract_times(now,last_tictoc_millis) > beep_interval_millis[cur_beep_interval]) {
    if (next_is_tic) {
      play_beep(BEEP_TIC);
    } else {
      play_beep(BEEP_TOC);
    }
    next_is_tic = !(next_is_tic);
    last_tictoc_millis = now;
  }
}

bool is_score_reset_needed() {
  if ( (button_team1.just_pressed() && button_team2.is_pressed()) ||
       (button_team2.just_pressed() && button_team1.is_pressed()) ) {
    return true;
  }
  return false;
}


void loop() {
  
    // Update all the buttons state
    button_team1.update_advertised_state();
    button_team2.update_advertised_state();
    button_start_stop.update_advertised_state();
    button_next.update_advertised_state();
    button_category.update_advertised_state();

    switch (game_state) {
      case CATEGORY_SELECTION:
        if (button_start_stop.just_pressed()) {
          start_new_round();

          // Ignore any other button presses since we're now effectively
          // in IN_ROUND state.  There's nothing else that could be going
          // on that we'd want to process.
          break;
        }

        if (button_category.just_pressed()) {
          play_beep(BEEP_CATEGORY_CHANGE);
          rotate_category();
          updateDisplay(categories[cur_category]);
        }

        // If we get new team1/2 button push, and the other one is pressed
        // zero the scores; otherwise increment the team score as needed
        if (is_score_reset_needed()) {
          // Note we're deviating from standard catch-phrase here to
          // reset the scores immediately rather than after a delay
          int score_team1 = 0;
          int score_team2 = 0;

          play_beep(BEEP_SCORE_RESET);
        }
        else if (button_team1.just_pressed()) {
          play_beep(BEEP_SCORE_CHANGE);
          ++score_team1;
          updateDisplay(categories[cur_category]);
        }
        else if (button_team2.just_pressed()) {
          play_beep(BEEP_SCORE_CHANGE);
          ++score_team2;
          updateDisplay(categories[cur_category]);
        }
        if (score_team1 == 7 || score_team2 == 7) \
        {
          end_game();
          break;
        }

        // We can do this once at the end since all the buttons that might
        // have been pushed will update the display.  Note the the start/stop
        // button push won't end up here since we break in that case.  Same with the
        // game over case.
        
        break;
      case IN_ROUND:
        if (button_start_stop.just_pressed()) {
          // Just get out
          // Beeping will stop since we're changing the state
          // The display will keep showing the clue (good I think).
          // So beep and exit
          play_beep(BEEP_STOP_ROUND);
          game_state = CATEGORY_SELECTION;
          break;
        }
        if (button_next.just_pressed()) {
          // No sound on this event
          rotate_clue();
          updateDisplay(clues[cur_clue]);
        }
        do_tic_toc();
        break;
      case GAME_DONE:
        // The actions are similar whether we get a category button, start/stop button
        // or simultaneous team1/team2 press (sounds differ).
        if (button_start_stop.just_pressed() ||
            button_category.just_pressed()) {
          game_state = CATEGORY_SELECTION;
          score_team1 = 0;
          score_team2 = 0;

          play_beep(BEEP_EXIT_GAME_DONE_STATE);
          updateDisplay(categories[cur_category]);
          
          // Let the next loop() handle the display update
        }
        break;
    }

}

