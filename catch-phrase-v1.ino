#include <stdlib.h>
#include <time.h>
#include <LiquidCrystal.h>
//#include <SPI.h>
#include <avr/sleep.h>
#include "display.h"
#include "button.h"
#include "cat_clues.h"
#include "serial.h"

#define MAX_LONG 4294967295

byte TRANSISTOR_POWER_PIN = 0;
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
// #define SLEEP_HARD_TIME 15000
// #define SLEEP_DIM_TIME 5000

// How long to wait before acknoledging button pushes after a sleep mode
#define SLEEP_HOLD_TIME 100

long end_of_sleep_hold_time = 0;

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

// scottnew
bool camping_mode = false;
bool camping_mode_button_push_registered = false;

bool is_category_displayed_category_selection_mode = true;
int cur_category = 0;
String cur_clue = "";

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

// Track if we've seeded the rng
bool rng_seeded = false;

// File Descriptor for clue file on the SD card
File cluefile;

// scottnew
// | / \ - ...
int spinner_position = 0;

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


// Play the tones, and print them to the console
void play_beep(BEEP_TYPE beep) {

  //scottnew
  if (camping_mode) {
    switch (beep) {
      case BEEP_TIC:
      case BEEP_TOC:
        if (++spinner_position == 4) {
          spinner_position = 0;
        }
        char spinner_char;
        switch (spinner_position) {
          case 0: spinner_char = '|'; break;
          case 1: spinner_char = '/'; break;
          case 2: spinner_char = '-'; break;

          // Fake backslash b/c the char map sucks.
          case 3: spinner_char = 0xA4; break;
        }
        lcd.setCursor(0, 1);
        lcd.print(spinner_char);
        lcd.setCursor(15, 1);
        lcd.print(spinner_char);
        break;
      case BEEP_TIMES_UP:
        lcd.setCursor(0, 1);
        lcd.print("X");
        lcd.setCursor(15, 1);
        lcd.print("X");
    }
    // Quiet!  People are sleeping!
    return;
  }

  print("BEEP - ");  
  switch (beep) {
    case BEEP_TIC:
      println("TIC");
      // Example with PWM:
      // analogWrite(SPEAKER_PIN,2);
      // delay(100);
      // analogWrite(SPEAKER_PIN,0);
      tone(SPEAKER_PIN, 300, 30);
      break;
    case BEEP_TOC:
      println("TOC");
      // Example with PWM:
      // analogWrite(SPEAKER_PIN,2);
      // delay(100);
      // analogWrite(SPEAKER_PIN,0);  
      tone(SPEAKER_PIN, 300, 30);
      break;
    case BEEP_TIMES_UP:
      println("TIMES_UP");
      tone(SPEAKER_PIN, 300, 300);
      delay(300);
      tone(SPEAKER_PIN, 300, 300);
      delay(300);
      tone(SPEAKER_PIN, 300, 300);
      break;
    case BEEP_POWER_ON:
      println("POWER_ON");
      tone(SPEAKER_PIN, 300, 30);
      break;
    case BEEP_CATEGORY_CHANGE:
      println("CATEGORY_CHANGE");
      tone(SPEAKER_PIN, 300, 30);
      break;
    case BEEP_SCORE_CHANGE:
      println("SCORE_CHANGE");
      tone(SPEAKER_PIN, 300, 30);
      break;
    case BEEP_SCORE_RESET:
      println("SCORE_RESET");
      tone(SPEAKER_PIN, 300, 30);
      break;
    case BEEP_WIN_GAME:
      println("WIN_GAME");
      for (int i = 0; i < 3; ++i) {
        tone(SPEAKER_PIN, 300, 250);
        delay(100);
        tone(SPEAKER_PIN, 400, 250);
        delay(100);
        tone(SPEAKER_PIN, 500, 250);
        delay(100);
      }
      break;
    case BEEP_STOP_ROUND:
      println("STOP_ROUND");
      tone(SPEAKER_PIN, 300, 30);
      break;
    case BEEP_EXIT_GAME_DONE_STATE:
      println("EXIT_GAME_DONE_STATE");
      tone(SPEAKER_PIN, 300, 30);
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


// scottnew
void updateDisplay(String displayString, bool clearBottomCorners = true) {

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


  // scottnew
  if (clearBottomCorners) {
    lcd.setCursor(0, 1);
    lcd.print("  ");
  } else {
    lcd.setCursor(2, 1);
  }
  lcd.print(formattedText.substring(12));
  // scottnew
  if (clearBottomCorners) {
    lcd.print("  ");
  }

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
    delay(2000);
  }

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
  digitalWrite(TRANSISTOR_POWER_PIN,LOW);
  sleep_cpu();
  
  doSetup(false);

  end_of_sleep_hold_time = millis() + SLEEP_HOLD_TIME;

  sleep_disable();
}

void setup() {
  doSetup(true);
}

void doSetup(bool firstTime) {

  if (SERIAL_CONSOLE_ENABLE) {
    Serial.begin(9600);
     while (!Serial) {
      delay(10); // wait for serial port to connect. Needed for native USB port only
    }
  }
  //SPI.begin();
  println("Catch Phrase - Power On");
  play_beep(BEEP_POWER_ON);
    
  game_state = CATEGORY_SELECTION;

  pinMode(TRANSISTOR_POWER_PIN,OUTPUT);
  pinMode(TEAM1_PIN,INPUT_PULLUP);
  pinMode(TEAM2_PIN,INPUT_PULLUP);
  pinMode(START_STOP_PIN,INPUT_PULLUP);
  pinMode(NEXT_PIN,INPUT_PULLUP);
  pinMode(CATEGORY_PIN,INPUT_PULLUP);

  digitalWrite(TRANSISTOR_POWER_PIN,HIGH);
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

  if (firstTime) {
    println("Being Read File");
    cluefile = readFile("clues.txt");
  }
  digitalWrite(LCD_PIN_BL,HIGH);
  updateDisplay(categories[cur_category]);
  Button::reset_last_button_press();

  // Initialize the LCD (16 columns, 2 rows)
  game_state = CATEGORY_SELECTION;

  // Reset the game
  cur_category = 0;
  score_team1 = 0;
  score_team2 = 0;

  updateDisplay(categories[cur_category]);
  
  is_category_displayed_category_selection_mode = true;
}

void rotate_category() {
  if(++cur_category >= NUM_CATEGORIES) {
    cur_category = 0;
  }
}

void update_clue() {
  while (true) {
    cur_clue = get_clue_as_string(cur_category,cluefile);
    
    // Performance sin coming up: calling get_display_text()
    // to check if it fits on LCD, and then again to actually
    // display it.  Oops.
    if (get_display_text(cur_clue).length() > 0) {
      break;
    }
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
  update_clue();
  updateDisplay(cur_clue);

  // When we get back to category selection, we want to still
  // show the clue, not the category.
  is_category_displayed_category_selection_mode = false;
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

  // Back into "show the category" mode
  is_category_displayed_category_selection_mode = true;
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

    // If it's too soon after a sleep mode, loop for a little while
    // to allow the start/stop button push edge to be registered without
    // starting a round
    if (millis() < end_of_sleep_hold_time) {
      return;
    }
    

    // Use the time of the first button as a source of entropy to seed
    // the RNG.
    if (!rng_seeded && (
          button_team1.just_pressed() ||
          button_team2.just_pressed() ||
          button_start_stop.just_pressed() ||
          button_next.just_pressed() ||
          button_category.just_pressed()
       )) {
      randomSeed(micros());
      rng_seeded = true;
    }

    switch (game_state) {
      case CATEGORY_SELECTION:
        if (button_start_stop.just_pressed()) {
          start_new_round();

          // Ignore any other button presses since we're now effectively
          // in IN_ROUND state.  There's nothing else that could be going
          // on that we'd want to process.
          break;
        }

        // scottnew
        if (button_next.get_time_pressed() > 1000){
          if (!camping_mode_button_push_registered) {
            if (!camping_mode) {
              camping_mode = true;
              updateDisplay("Camping Mode (Shh Krista)");
            } else {
              camping_mode = false;
              updateDisplay("Krista Mode (Loud)");
            }
            // We've taken action on the long button push, don't do it
            // again until the button is released
            camping_mode_button_push_registered = true;
            break;
          }
        } else {
          // The button hasn't been held long enough, make sure we're ready
          // to notice a long press
          camping_mode_button_push_registered = false;
        }

        if (button_category.just_pressed()) {
          // This button push puts us into category-display mode if we
          // weren't already there.
          is_category_displayed_category_selection_mode = true;

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
        }
        else if (button_team2.just_pressed()) {
          play_beep(BEEP_SCORE_CHANGE);
          ++score_team2;
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
        // Update with clue or category depending on what's needed.
        if (button_team1.just_pressed() || button_team2.just_pressed()) {
          if (is_category_displayed_category_selection_mode) {
            updateDisplay(categories[cur_category]);
          } else {
            updateDisplay(cur_clue);
          }
        }
        
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
          update_clue();
          updateDisplay(cur_clue);
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
    if (subtract_times(millis(), Button::get_last_button_press()) >= SLEEP_HARD_TIME) { 
      // Put the Arduino into power down state
      sleep_power_down();  
    } else if (subtract_times(millis(), Button::get_last_button_press()) >= SLEEP_DIM_TIME && backlight) {
      //Dim the backlight
      println("Turning off Backlight");
      backlight = false;
      digitalWrite(LCD_PIN_BL,LOW);
    } else if (subtract_times(millis(), Button::get_last_button_press()) < SLEEP_DIM_TIME && backlight == false) {
      // Our backlight was dimmed, but we shouldn't be in a dim state anymore
      println("Turning on Backlight");
      backlight = true;
      digitalWrite(LCD_PIN_BL,HIGH);
      
    }

}
