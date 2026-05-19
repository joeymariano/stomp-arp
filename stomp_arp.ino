/*
stomp_updown_rotary_mod.ino
Alternates playback between normal speed and double speed.
Rotary encoder on the StompShield sets the alternation interval (rate).
Hardware: StompShield for Arduino (encoder A/B on pins 2/4, button on pin 7)
*/

#include "StompShield.h"

#define SIZE 1000 // buffer size
int buffer[SIZE];

unsigned int location = 0;     // write pointer
unsigned int playLocation = 0; // read pointer

int data_buffer = 0x8000;

// Rotary encoder pins
#define ROTARY_A 2
#define ROTARY_B 4
#define ROTARY_BTN 7

volatile int encoderValue = 48; // mapped to INTERVAL below
byte lastA;
unsigned long lastPoll = 0;

// Interval mapping. The ISR runs at ~48 kHz, so 48 ticks ~= 1 ms.
#define MIN_INTERVAL_TICKS 96   // ~2 ms (fast alternation)
#define MAX_INTERVAL_TICKS 4800 // ~100 ms (slow alternation)
volatile unsigned int INTERVAL = 480; // default ~10 ms
unsigned int intervalCount = 0;
bool doubleSpeed = false;

void pollEncoder() {
  byte currentA = digitalRead(ROTARY_A);
  byte currentB = digitalRead(ROTARY_B);

  if (currentA != lastA) {
    if (currentA == LOW) { // falling edge on A
      if (currentB == LOW) encoderValue++; // clockwise
      else                 encoderValue--; // counterclockwise
      encoderValue = constrain(encoderValue, 2, 100);
      INTERVAL = map(encoderValue, 2, 100, MIN_INTERVAL_TICKS, MAX_INTERVAL_TICKS);
    }
    lastA = currentA;
  }
}

void setup() {
  StompShield_init();
  pinMode(ROTARY_A, INPUT_PULLUP);
  pinMode(ROTARY_B, INPUT_PULLUP);
  pinMode(ROTARY_BTN, INPUT_PULLUP);
  lastA = digitalRead(ROTARY_A);
}

void loop() {
  if (millis() - lastPoll > 1) {
    pollEncoder();
    lastPoll = millis();
  }
}

ISR(TIMER1_OVF_vect) {
  // Output last computed sample
  OCR1AL = ((data_buffer + 0x8000) >> 8);
  OCR1BL = data_buffer;

  // Read new ADC sample (low byte first)
  byte temp1 = ADCL;
  byte temp2 = ADCH;
  int input = ((temp2 << 8) | temp1) + 0x8000;
  buffer[location] = input;

  location++;
  if (location >= SIZE) location = 0;

  // Play sample at playLocation
  data_buffer = buffer[playLocation];

  // Advance playback pointer
  if (doubleSpeed) playLocation += 2;
  else             playLocation += 1;
  if (playLocation >= SIZE) playLocation -= SIZE;

  // Flip between normal and double speed every INTERVAL ticks
  intervalCount++;
  if (intervalCount >= INTERVAL) {
    doubleSpeed = !doubleSpeed;
    intervalCount = 0;
  }
}
