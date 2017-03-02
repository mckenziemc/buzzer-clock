byte time_hour = 0;
byte time_minute = 0;
byte time_second = 0;

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

#include <AStar32U4.h>

AStar32U4Buzzer buzzer;
AStar32U4ButtonA buttonA;
AStar32U4ButtonB buttonB;
AStar32U4ButtonC buttonC;
AStar32U4LCD lcd;

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
    time_second++;

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

  switch (set_time_mode) {
    case 0:
    {
      if (buttonA.getSingleDebouncedRelease()) {
        playAll();
      }
  
      if (buttonB.getSingleDebouncedPress() or buttonC.getSingleDebouncedPress()) {
        set_time_mode = SET_TIME_MODE_HOUR;
        // TODO: start blinking hour indicator
      }
    }
    break;

    case SET_TIME_MODE_HOUR:
    {
      if (buttonB.getSingleDebouncedPress()) {
        time_hour++;
        display_time();
      } else if (buttonC.getSingleDebouncedPress()) {
        time_hour--;
        display_time();
      } else if (buttonA.getSingleDebouncedPress()) {
        set_time_mode++;
      }
    }
    break;

    case SET_TIME_MODE_MINUTE:
    {
      if (buttonB.getSingleDebouncedPress()) {
        time_minute++;
        display_time();
      } else if (buttonC.getSingleDebouncedPress()) {
        time_minute--;
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
        time_second++;
        next_second = millis() + 1000;
        display_time();
      } else if (buttonC.getSingleDebouncedPress()) {
        time_second--;
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
  if (time_second >= (256 - 60)) {
    time_second += 60;
    time_minute--;
  } else if (time_second >= 60) {
    time_second -= 60;
    time_minute++;
  }

  if (time_minute >= (256 - 60)) {
    time_minute += 60;
    time_hour--;
  } else if (time_minute >= 60) {
    time_minute -= 60;
    time_hour++;
  }

  if (time_hour >= (256 - 24)) {
    time_hour += 24;
  } else if (time_hour >= 24) {
    time_hour -= 24;
  }
}

void display_time() {
  fix_time();

  String hour_string;
  String minute_string;
  String second_string;
  
  if (time_hour < 10) {
    hour_string = "0";
  } else {
    hour_string = "";
  }
  
  hour_string += time_hour;

  if (time_minute < 10) {
    minute_string = "0";
  } else {
    minute_string = "";
  }
  
  minute_string += time_minute;
  
  if (time_second < 10) {
    second_string = "0";
  } else {
    second_string = "";
  }

  second_string += time_second;

  String time_string = hour_string + ":" + minute_string + ":" + second_string;

  lcd.clear();
  lcd.print(time_string);
}


