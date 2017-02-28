unsigned long seconds_into_day = 0;

unsigned long current_second = 0;

const char Q1[] PROGMEM = "! g#f#e<b.R8";
const char Q2[] PROGMEM = "eg#f#<b.R8 ef#g#e.R8";
const char Q3[] PROGMEM = "g#ef#<b.R8 <bf#g#e.R8 g#f#e<b.R8";
const char Q4[] PROGMEM = "eg#f#<b.R8 ef#g#e.R8 g#ef#<b.R8 <bf#g#e.R8";

#include <AStar32U4.h>

AStar32U4Buzzer buzzer;
AStar32U4ButtonA buttonA;
AStar32U4LCD lcd;

void setup() {
  // put your setup code here, to run once:

  lcd.clear();
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
}


void playAll() {
  // FIXME: advance asycronously to allow handling other tasks
  buzzer.playFromProgramSpace(Q1);

  delay(5000);
  buzzer.playFromProgramSpace(Q2);

  delay(8000);
  buzzer.playFromProgramSpace(Q3);

  delay(10000);
  buzzer.playFromProgramSpace(Q4);
}

