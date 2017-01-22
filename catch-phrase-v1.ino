#include <stdlib.h>
#include <time.h>
#include <LiquidCrystal.h>
//#include <SPI.h>
#include <avr/sleep.h>
#include "display.h"
#include "button.h"
#include "cat_clues.h"
#include "serial.h"
#define byte uint8_t
#define MAX_LONG 4294967295

byte START_STOP_PIN = 2;
byte TEAM1_PIN = 3;
byte TEAM2_PIN = 4;
byte NEXT_PIN = 5;
byte CATEGORY_PIN = 6;
byte SPEAKER_PIN = 7;
byte LCD_PIN_RS = 8;
byte LCD_PIN_E  = 9;
byte LCD_PIN_D7 = 18;
byte LCD_PIN_D6 = 19;
byte LCD_PIN_D5 = 20;
byte LCD_PIN_D4 = 21;
byte LCD_PIN_BL = 26;
byte SD_PIN_CS = 10;

#define SLEEP_HARD_TIME 600000
#define SLEEP_DIM_TIME 120000
// Use this to debug sleep code
//#define SLEEP_HARD_TIME 30000
//#define SLEEP_DIM_TIME 15000

bool backlight = true;

extern unsigned long subtract_times(unsigned long t1, unsigned long t2);

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

GAME_STATES game_state = CATEGORY_SELECTION;

int cur_category = 0;

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

// File Descriptor for clue file on the SD card
File cluefile;

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


// Just output for now
void play_beep(BEEP_TYPE beep) {
  print("BEEP - ");  
  switch (beep) {
    case BEEP_TIC:
      analogWrite(SPEAKER_PIN,2);
      delay(100);
      analogWrite(SPEAKER_PIN,0);
      println("TIC");
      break;
    case BEEP_TOC:
      analogWrite(SPEAKER_PIN,2);
      delay(100);
      analogWrite(SPEAKER_PIN,0);           
      println("TOC");
      break;
    case BEEP_TIMES_UP:
      println("TIMES_UP");
      break;
    case BEEP_POWER_ON:
      println("POWER_ON");
      break;
    case BEEP_CATEGORY_CHANGE:
      println("CATEGORY_CHANGE");
      break;
    case BEEP_SCORE_CHANGE:
      println("SCORE_CHANGE");
      break;
    case BEEP_SCORE_RESET:
      println("SCORE_RESET");
      break;
    case BEEP_WIN_GAME:
      println("WIN_GAME");
      break;
    case BEEP_STOP_ROUND:
      println("STOP_ROUND");
      break;
    case BEEP_EXIT_GAME_DONE_STATE:
      println("EXIT_GAME_DONE_STATE");
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

  if (formattedText.length() == 0) {
    print("OOPS! CAN'T DISPLAY: ");
    println(displayString);
    print("formatted: ");
    println(formattedText);
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
  lcd.print("  ");

  print(score_team1);
  print(" ");
  print(formattedText.substring(0,12));
  print(" ");
  println(score_team2); 
  print("  ");
  print(formattedText.substring(12));

}

void wake_interrupt_callback() {
  println("Sleep Pin Interrupt triggered");
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(START_STOP_PIN));

}

void sleep_power_down() {
  println("Going into Power Down State");
  if (SERIAL_CONSOLE_ENABLE) {
    Serial.flush();
    Serial.end();
  }
  delay(2000);
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(START_STOP_PIN), wake_interrupt_callback, LOW);
    
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    
  cli();
  //sleep_bod_disable();
  sei();
  //USBCON |= _BV(FRZCLK);
  //PLLCSR &= ~_BV(PLLE);
  //USBCON &= ~_BV(USBE);

  println("going to sleep");
//  updateDisplay("I'm sooooo tired");
  sleep_cpu();

  // Initialize the LCD (16 columns, 2 rows)
  game_state = CATEGORY_SELECTION;

  // Reset the game
  println("Waking up from Sleep");
  cur_category = 0;
  score_team1 = 0;
  score_team2 = 0;

  updateDisplay(categories[cur_category]);
  Button::reset_last_button_press();

  sleep_disable();
}




void setup() {
  srand(time(0));
  if (SERIAL_CONSOLE_ENABLE) {
    Serial.begin(9600);
     while (!Serial) {
      delay(10); // wait for serial port to connect. Needed for native USB port only
    }
  }
  //SPI.begin();
  println("Catch Phrase - Power On");
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

    print("Initializing SD card...");

  if (!SD.begin(SD_PIN_CS)) {
    println("initialization failed!");
    updateDisplay("SD Card Failure!");
    return; 
  }
  println("initialization done.");
  println("Initialized SD Card Reader");

  println("Being Read File");
  cluefile = readFile("clues.txt");
  digitalWrite(LCD_PIN_BL,HIGH);
  updateDisplay(categories[cur_category]);
  Button::reset_last_button_press();

}

void rotate_category() {
  if(++cur_category >= NUM_CATEGORIES) {
    cur_category = 0;
  }
}

void start_new_round() {
  game_state = IN_ROUND;

  // Set up the timer-related stuff
  cur_beep_interval = 0;
  next_is_tic = true;
  last_tictoc_millis = 0; // want it to tic immediately
  last_beep_speed_change_millis = millis(); // since we just changed the frequency

  // Update the display
  updateDisplay(get_clue_as_string(cur_category,cluefile));
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
          updateDisplay(get_clue_as_string(cur_category,cluefile));
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
        }
        break;
    }

    //Sleep Code
    if (millis() - Button::get_last_button_press() >= SLEEP_HARD_TIME) { 
      // Put the Arduino into power down state
      sleep_power_down();  
    } else if (millis() - Button::get_last_button_press() >= SLEEP_DIM_TIME && backlight) {
      //Dim the backlight
      println("Turning off Backlight");
      backlight = false;
      digitalWrite(LCD_PIN_BL,LOW);
    } else if (millis() - Button::get_last_button_press() < SLEEP_DIM_TIME && backlight == false) {
      // Our backlight was dimmed, but we shouldn't be in a dim state anymore
      println("Turning on Backlight");
      backlight = true;
      digitalWrite(LCD_PIN_BL,HIGH);
      
    }

}
