#include <AStar32U4.h>

AStar32U4Buzzer buzzer;
AStar32U4ButtonA buttonA;
AStar32U4ButtonB buttonB;
AStar32U4ButtonC buttonC;
AStar32U4LCD lcd;

byte clock_hour = 0;
byte clock_minute = 0;
byte clock_second = 1; // to prevent playing long sequence on startup

// dedicated C string for display formatting
char time_string[] = "00:00:00";

// millis value for the start of the next second
unsigned long next_second = 1000;

unsigned set_time_mode = 0;
const unsigned SET_TIME_MODE_HOUR = 1;
const unsigned SET_TIME_MODE_MINUTE = 2;
const unsigned SET_TIME_MODE_SECOND = 3;

const char play_config[] PROGMEM = "!v8";
const char Q1[] PROGMEM = "g#f#e<b.R8";
const char Q2[] PROGMEM = "eg#f#<b.R8 ef#g#e.R8";
const char Q3[] PROGMEM = "g#ef#<b.R8 <bf#g#e.R8 g#f#e<b.R8";
const char Q4[] PROGMEM = "eg#f#<b.R8 ef#g#e.R8 g#ef#<b.R8 <bf#g#e.R8";

unsigned play_strikes = 0;

const unsigned SERIAL_BUFFER_LENGTH = 32;
char serial_buffer[SERIAL_BUFFER_LENGTH];
unsigned serial_buffer_index = 0;
boolean bad_serial_buffer = false; // used if input exceeds buffer length

// variables for the play-all-sequences feature
boolean playing_all = false;
unsigned current_play_all_sequence = 0;
boolean play_all_resting = false;
unsigned long play_all_next_start = 0;

void setup() {
  // put your setup code here, to run once:

  lcd.clear();

  for (unsigned i = 0; i < SERIAL_BUFFER_LENGTH; i++) {
    serial_buffer[i] = 0;
  }

  buzzer.playFromProgramSpace(play_config);
}

void loop() {
  if (millis() >= next_second) {
    next_second += 1000;

    // update the time variables
    clock_second++;

    display_time();
  }
  
  if (playing_all && !buzzer.isPlaying()) {
    if (!play_all_resting) {
      // start inter-sequence delay
      play_all_resting = true;
      play_all_next_start = millis() + 4000;
    } else if (millis() > play_all_next_start) {
      // end inter-sequence delay
      play_all_resting = false;
      playNext();
    }
  }

  while (Serial.available()) {
    Serial.println(Serial.available());
    
    int next_char = Serial.read();

    Serial.println((byte) next_char);
    Serial.println();

    if (next_char == 13) {
      // consume the line-feed character if present
      if (Serial.available() && Serial.peek() == 10) {
        Serial.read();
        Serial.println("found CR+LF");
      }

      process_serial_line();
    } else if (next_char == 10) {
      process_serial_line();
    } else {
      // some character other than a new-line
 
      if (serial_buffer_index >= SERIAL_BUFFER_LENGTH - 1) {
        bad_serial_buffer = true;
      } else {
        Serial.println("placing in serial buffer");
        serial_buffer[serial_buffer_index++] = next_char;
      }
    }
  }

  if (play_strikes > 0 && !buzzer.isPlaying()) {
    play_strikes--;
    buzzer.play("<E1R");
  }

  switch (set_time_mode) {
    case 0:
    {
      if (buttonA.getSingleDebouncedRelease()) {
        //playAll();
      }
  
      if (buttonB.getSingleDebouncedPress() or buttonC.getSingleDebouncedPress()) {
        set_time_mode = SET_TIME_MODE_HOUR;
        // TODO: start blinking hour indicator
      }

      // check if it's time to play a sequence
      if (!buzzer.isPlaying() && clock_second == 0) {
        switch (clock_minute) {
          case 0:
          // hour
          buzzer.playFromProgramSpace(Q4);

          if (clock_hour == 0) {
            play_strikes = 12;
          } else if (clock_hour > 12) {
            play_strikes = clock_hour - 12;
          } else {
            play_strikes = clock_hour;
          }
 
          break;

          case 15:
          buzzer.playFromProgramSpace(Q1);
          break;

          case 30:
          buzzer.playFromProgramSpace(Q2);
          break;

          case 45:
          buzzer.playFromProgramSpace(Q3);
          break;
        }
      }
    }
    break;

    case SET_TIME_MODE_HOUR:
    {
      if (buttonB.getSingleDebouncedPress()) {
        clock_hour++;
        display_time();
      } else if (buttonC.getSingleDebouncedPress()) {
        clock_hour--;
        display_time();
      } else if (buttonA.getSingleDebouncedPress()) {
        set_time_mode++;
      }
    }
    break;

    case SET_TIME_MODE_MINUTE:
    {
      if (buttonB.getSingleDebouncedPress()) {
        clock_minute++;
        display_time();
      } else if (buttonC.getSingleDebouncedPress()) {
        clock_minute--;
        display_time();
      } else if (buttonA.getSingleDebouncedPress()) {
        set_time_mode++;
      }
    }
    break;

    case SET_TIME_MODE_SECOND:
    {
      // if the second is changed, consider the button press as marking the start of the second
      if (buttonB.getSingleDebouncedPress()) {
        clock_second++;
        next_second = millis() + 1000;
        display_time();
      } else if (buttonC.getSingleDebouncedPress()) {
        clock_second--;
        next_second = millis() + 1000;
        display_time();
      } else if (buttonA.getSingleDebouncedPress()) {
        set_time_mode = 0;
      }
    }
    break;

  }
}


void playAll() {
  playing_all = true;
  
  current_play_all_sequence = 1;
  buzzer.playFromProgramSpace(Q1);
}

void playNext() {
  current_play_all_sequence++;
  
  switch(current_play_all_sequence) {
    case 2:
    buzzer.playFromProgramSpace(Q2);
    break;

    case 3:
    buzzer.playFromProgramSpace(Q3);
    break;

    case 4:
    buzzer.playFromProgramSpace(Q4);
    break;

    default:
    playing_all = false;
  }
}


void process_serial_line() {
  Serial.println("processing serial buffer");
  
  if (bad_serial_buffer) {
    // ignore buffer contents
    Serial.println("Buffer overflow; ignoring line.");
    bad_serial_buffer = false;
  } else {
  }

  Serial.print("Buffer contents: ");
  Serial.println(serial_buffer);

  // clear the buffer
  for (unsigned i = 0; i < serial_buffer_index; i++) {
    serial_buffer[i] = 0;
  }

  serial_buffer_index = 0;
}

void fix_time() {
  // assume that if a value is outside its bound, only one correction is necessary
  if (clock_second >= (256 - 60)) {
    clock_second += 60;
    clock_minute--;
  } else if (clock_second >= 60) {
    clock_second -= 60;
    clock_minute++;
  }

  if (clock_minute >= (256 - 60)) {
    clock_minute += 60;
    clock_hour--;
  } else if (clock_minute >= 60) {
    clock_minute -= 60;
    clock_hour++;
  }

  if (clock_hour >= (256 - 24)) {
    clock_hour += 24;
  } else if (clock_hour >= 24) {
    clock_hour -= 24;
  }
}

void display_time() {
  fix_time();

  time_string[0] = '0' + clock_hour / 10;
  time_string[1] = '0' + clock_hour % 10;

  time_string[3] = '0' + clock_minute / 10;
  time_string[4] = '0' + clock_minute % 10;

  time_string[6] = '0' + clock_second / 10;
  time_string[7] = '0' + clock_second % 10;

  lcd.clear();
  lcd.print(time_string);

  // handle cursor blinking if currently in a set-time mode
  if (set_time_mode && millis() % 2000 < 1000) {
    switch (set_time_mode) {
      case SET_TIME_MODE_HOUR:
      lcd.setCursor(1,0);
      break;

      case SET_TIME_MODE_MINUTE:
      lcd.setCursor(4,0);
      break;

      case SET_TIME_MODE_SECOND:
      lcd.setCursor(7,0);
      break;
    }
    
    lcd.cursorSolid();
  }
}


