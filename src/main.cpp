#include <Arduino.h>
#include <vector>

// put function declarations here:
int myFunction(int, int);

int MIDI_IN_PIN = 2; // CHANGEME
const int BAUD = 31250; // MIDI requires 31250 baud rate
unsigned long samplingTime = 1000000/BAUD;
std::vector<int> frame; // fixed size frame of 8 bits MSB0->LSB7
std::vector<int> midiMessage;

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

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long loopStartTime;
  unsigned long loopEndTime;
  // This whole loop needs to run every 1/31250 seconds. Check current time with millis
  loopStartTime = micros();
  if(samplingTime >= loopStartTime - loopEndTime){
    int rx0 = digitalRead(MIDI_IN_PIN);

    if(rx0 == LOW && currentFramingState == IDLE){
      nextFramingState = STARTREAD;
      frame.clear();
    }
    else if((rx0 == HIGH && currentFramingState == STARTREAD) && frame.size() == 8){
      nextFramingState = STOPREAD;
      midiMessage = frame;
    }
    else if(currentFramingState == STOPREAD){
      nextFramingState = IDLE;
    }
    else{
      nextFramingState = currentFramingState;
    }


    if(currentFramingState == STARTREAD){
      frame.push_back(rx0);
    }
    currentFramingState = nextFramingState;
    loopEndTime = micros();
  }
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
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

