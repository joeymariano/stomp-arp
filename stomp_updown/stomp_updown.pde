/*
stomp_updown.pde
guest openmusiclabs.com 7.15.13
this program plays through a sample buffer, first forward at
double rate, and then backwards at single rate.  it changes
direction at the buffer boundary.  the rotary encoder does
nothing.
*/

#include "StompShield.h"

#define SIZE 1000 // buffer size, make lower if it clicks
int buffer[SIZE]; // data buffer

unsigned int location = 0; // current buffer location
unsigned int offset = 0; // distance to current location
byte dir = 0; // direction of travel in buffer
int data_buffer = 0x8000;

void setup() {
  StompShield_init(); // setup the arduino for the shield
}

void loop() {
}

ISR(TIMER1_OVF_vect) { // all processing happens here

  // output the last value calculated
  OCR1AL = ((data_buffer + 0x8000) >> 8); // convert to unsigned, send out high byte
  OCR1BL = data_buffer; // send out low byte
  
  // get ADC data
  byte temp1 = ADCL; // you need to fetch the low byte first
  byte temp2 = ADCH; // yes it needs to be done this way
  int input = ((temp2 << 8) | temp1) + 0x8000; // make a signed 16b value
  
  buffer[location] = input; // store current sample
  location++; // go to next location
  if (location >= SIZE) location = 0; // deal with boundary
  unsigned int temp = location + offset; // find playback location
  if (temp >= SIZE) temp -= SIZE; // boundary wrap
  data_buffer = buffer[temp]; // fetch sample
  if (dir) { // increment until at buffer boundary
    if (offset >= (SIZE - 4)) {
      dir = 0;
      offset--;
    }
    else offset++;
  }
  else { // decrement till reaching boundary from other side
   if (offset <= 4) {
     dir = 1;
     offset--;
   }
   else offset -= 2;
  }
}


