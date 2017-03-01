unsigned long seconds_into_day = 0;

unsigned long current_second = 0;

const char play_config[] PROGMEM = "!v10";
const char Q1[] PROGMEM = "g#f#e<b.R8";
const char Q2[] PROGMEM = "eg#f#<b.R8 ef#g#e.R8";
const char Q3[] PROGMEM = "g#ef#<b.R8 <bf#g#e.R8 g#f#e<b.R8";
const char Q4[] PROGMEM = "eg#f#<b.R8 ef#g#e.R8 g#ef#<b.R8 <bf#g#e.R8";

#include <AStar32U4.h>

AStar32U4Buzzer buzzer;
AStar32U4ButtonA buttonA;
AStar32U4LCD lcd;

const unsigned SERIAL_BUFFER_LENGTH = 32;
char serial_buffer[SERIAL_BUFFER_LENGTH];
unsigned serial_buffer_index;
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
  if (millis() / 1000 != current_second) {
    current_second = millis() / 1000;

    lcd.clear();
    lcd.print(current_second);
  }
  
  // put your main code here, to run repeatedly:
  if (buttonA.getSingleDebouncedRelease()) {
    playAll();
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

