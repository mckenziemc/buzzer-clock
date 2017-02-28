const char Q1[] PROGMEM = "! g#f#e<b.R8";
const char Q2[] PROGMEM = "eg#f#<b.R8 ef#g#e.R8";
const char Q3[] PROGMEM = "g#ef#<b.R8 <bf#g#e.R8 g#f#e<b.R8";
const char Q4[] PROGMEM = "eg#f#<b.R8 ef#g#e.R8 g#ef#<b.R8 <bf#g#e.R8";

#include <AStar32U4.h>

AStar32U4Buzzer buzzer;
AStar32U4ButtonA buttonA;

void setup() {
  // put your setup code here, to run once:
  delay(5000);
  playAll();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (buttonA.getSingleDebouncedRelease()) {
    playAll();
  }
}


void playAll() {
  buzzer.playFromProgramSpace(Q1);

  delay(5000);
  buzzer.playFromProgramSpace(Q2);

  delay(8000);
  buzzer.playFromProgramSpace(Q3);

  delay(10000);
  buzzer.playFromProgramSpace(Q4);
}

