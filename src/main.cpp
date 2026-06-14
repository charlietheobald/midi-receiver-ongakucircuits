#include <Arduino.h>
#include <vector>

// put function declarations here:
int myFunction(int, int);

int MIDI_IN_PIN = 2; // CHANGEME
const int BAUD = 31250; // MIDI requires 31250 baud rate

// should be 32us. The sample period of incoming data for correct framing
const unsigned long samplingTime = 32;

// should be 48us. This delay is triggered immediately when the start bit is pulled low
// and is 1.5 times the sample period to centre all subsequent samples in the centre of each pulse
const unsigned long centreSamplingDelay = 48;
uint8_t frame; // fixed size frame of 8 bits MSB0->LSB7
uint8_t midiMessage;

enum framingState{
  IDLE, STARTREAD, STOPREAD
};
framingState currentFramingState = IDLE;
framingState nextFramingState = currentFramingState;


void setup() {
  // put your setup code here, to run once:
  pinMode(MIDI_IN_PIN, INPUT);
  Serial.begin(BAUD);
  int loopStartTime = 0;
  int loopEndTime = 0;
}

// ISSUES:
// Should use better loop interrupts and sample at the centre of each MIDI pulse (as per UART)
// Now uses bitwise operators for greater efficiency
void loop() {
  
  unsigned long currentTime = micros();
  static unsigned long nextSampleTime;
  static int bitCount = 0;
  char rx0 = 0;

  if(currentTime >= nextSampleTime){
    rx0 = digitalRead(MIDI_IN_PIN);

    switch(currentFramingState){
      case IDLE:
      if(rx0 == LOW){
        nextFramingState = STARTREAD;
        nextSampleTime = currentTime + centreSamplingDelay;
        frame = 0;
        bitCount = 0;
      }
      break;

      case STARTREAD:
      frame = frame << 1 | (rx0 & 1);
      bitCount++;
      if(bitCount >= 8){
        nextFramingState = STOPREAD;
      }
      else{
        nextFramingState = STARTREAD;
      }
      nextSampleTime = currentTime + samplingTime;
      break;

      case STOPREAD:
      if(rx0 == HIGH){
        midiMessage = frame;
      }
      nextFramingState = IDLE;
      nextSampleTime = currentTime;
      break;

      
      default: nextFramingState = currentFramingState;
      break;
    }
  
    currentFramingState = nextFramingState;
  }
}

// put function definitions here:
int reverseTransmittedBits(unsigned int &midiMessage){
  
}

// Basic functionality - Take MIDI messages from the MIDI sockets and pass them through an optocoupler.
// MIDI input comes from pins 4 and 5. 

/*
HOW ARE MIDI NOTE ON / OFF EVENTS DEFINED?
8-bit messages.
Start bit 0, end bit 1
STATUS bytes always start with 1
DATA bytes always start with 0
Channel mode messages







*/

