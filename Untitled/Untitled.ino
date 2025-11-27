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

#define INTERVAL 3000  // Number of interrupts before switching modes (~10ms at 48kHz)
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