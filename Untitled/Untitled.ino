/*
stomp_updown_mod.pde
Modified for slower effect swapping:
- alternates between normal and double-speed playback every few ms
*/

#include "StompShield.h"

#define SIZE 1000 // buffer size
int buffer[SIZE];

unsigned int location = 0;      // write pointer
unsigned int playLocation = 0;  // read pointer

int data_buffer = 0x8000;

/*
stomp_updown_rotary_mod.pde
Effect alternates between normal and double speed playback.
Rotary encoder on pedal sets alternation interval (rate).
Hardware: StompShield for Arduino (pins 2/4 encoder, pin 7 button)
*/

#include "StompShield.h"

#define SIZE 1000 // buffer size
int buffer[SIZE];

unsigned int location = 0;     // write pointer
unsigned int playLocation = 0; // read pointer

int data_buffer = 0x8000;

// Rotary Encoder Pins
#define ROTARY_A 2
#define ROTARY_B 4
#define ROTARY_BTN 7

volatile int encoderValue = 48;     // Initial value (will be mapped to interval)
byte lastA;
unsigned long lastPoll = 0;

// Interval mapping (ms = intervalTicks / 48, since interrupt at ~48kHz)
#define MIN_INTERVAL_TICKS 96      // ~2ms (fast alternation)
#define MAX_INTERVAL_TICKS 4800    // ~100ms (slow alternation)
unsigned int INTERVAL = 480;       // Default interval (~10ms)
unsigned int intervalCount = 0;
bool doubleSpeed = false;

// Rotary Encoder polling
void pollEncoder() {
  byte currentA = digitalRead(ROTARY_A);
  byte currentB = digitalRead(ROTARY_B);

  if (currentA != lastA) { // A changed
    if (currentA == LOW) { // falling edge
      if (currentB == LOW) encoderValue++; // clockwise
      else encoderValue--;                 // counterclockwise
      // constrain encoder value for sensible range
      encoderValue = constrain(encoderValue, 2, 100);
      // Map to interval ticks (higher = slower)
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
  // Poll the rotary encoder about every 1ms (optional debounce)
  if (millis() - lastPoll > 1) {
    pollEncoder();
    lastPoll = millis();
  }
  // Add button control if desired (digitalRead(ROTARY_BTN))
}

// Audio ISR
ISR(TIMER1_OVF_vect) {
  // Output last value
  OCR1AL = ((data_buffer + 0x8000) >> 8);
  OCR1BL = data_buffer;

  // Get ADC sample
  byte temp1 = ADCL;
  byte temp2 = ADCH;
  int input = ((temp2 << 8) | temp1) + 0x8000;
  buffer[location] = input;

  location++;
  if (location >= SIZE) location = 0;

  // Play the sample at playLocation
  data_buffer = buffer[playLocation];

  // Step playLocation forward:
  if (doubleSpeed) {
    playLocation += 2;
  } else {
    playLocation += 1;
  }
  if (playLocation >= SIZE) playLocation -= SIZE;

  // Alternate modes every INTERVAL ISR calls
  intervalCount++;
  if (intervalCount >= INTERVAL) {
    doubleSpeed = !doubleSpeed; // Swap playback mode
    intervalCount = 0;
  }
} // Number of interrupts before switching modes (~10ms at 48kHz)
unsigned int intervalCount = 0; // Counts ISR cycles for alternation
bool doubleSpeed = false;       // double speed mode (true = double speed, false = normal speed)

void setup() {
  StompShield_init();
}

void loop() {
}

ISR(TIMER1_OVF_vect) {
  // Output last value
  OCR1AL = ((data_buffer + 0x8000) >> 8);
  OCR1BL = data_buffer;

  // Get ADC sample
  byte temp1 = ADCL;
  byte temp2 = ADCH;
  int input = ((temp2 << 8) | temp1) + 0x8000;
  buffer[location] = input;

  location++;
  if (location >= SIZE) location = 0;

  // Play the sample at playLocation
  data_buffer = buffer[playLocation];

  // Step playLocation forward:
  if (doubleSpeed) {
    playLocation += 2; // Double speed: skips every other sample
  } else {
    playLocation += 1; // Normal speed: sequential sample
  }
  if (playLocation >= SIZE) playLocation -= SIZE;

  // Alternate modes every INTERVAL ISR calls
  intervalCount++;
  if (intervalCount >= INTERVAL) {
    doubleSpeed = !doubleSpeed; // Swap playback mode
    intervalCount = 0;
  }
  
}