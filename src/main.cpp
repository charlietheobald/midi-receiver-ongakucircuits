// To fix:
// Running Status needs to be implemented separately for every MIDI channel

#include <Arduino.h>

#define MIDI_IN_PIN 4

struct midiEvent{
  uint8_t status1;
  uint8_t param1;
  uint8_t param2;
};

// MIDI MESSAGE RESPONSE STATE TRANSITIONS

// nb it might be better to put everything into a single enum that way we have just one status to detect
// type1 for two-parameter messages
// in either Normal or Running Status mode
// Example commands are Note On / Note Off / Polyphonic Key Pressure / Control Change / Channel Mode

// type2 for one-parameter messages
// in either Normal or Running Status mode
// Example commands are Program Change / Channel Pressure


// type3 for messages without parameters
// irrelevant of Normal or Running Status
// Example commands are Tune Request / Timing Clock / Active Sensing / Reset

enum midiReceiverState{
  MIDI_STATUS, CHANNEL, VELOCITY, PARAMETER, RUNNINGSTATUSCHECK
};

int messageType;
uint8_t lastStatus;




void setup() {
  Serial.begin(115200);
  Serial1.begin(31250, SERIAL_8N1, MIDI_IN_PIN, -1, true); 
  delay(100);
  while (Serial1.available()) Serial1.read();

  Serial.println("Ready"); 
}



void loop() {
  // Use a while loop to instantly empty the hardware buffer without delays
  while (Serial1.available() > 0) {
    uint8_t midiData = Serial1.read();
    
    static midiReceiverState currentState;
    static midiReceiverState nextState;
    static midiEvent midievent;

    // Print a leading zero for single-digit hex values (e.g., 02 instead of 2)
    if (midiData < 0x10){
      Serial.print("0");
    }
    Serial.print(midiData, HEX);
    Serial.print(" ");


    uint8_t midiData_top4 = midiData >> 4; // the top 4 bits of midiData, representing the status message
    uint8_t midiData_bottom4 = midiData & (0x0F); // the bottom 4 bits of midiData, representing the MIDI chnannel
    bool realTime = false; // realTime init to false. Used to allow running status to continue even with a "1" as MSB in a Real-Time Message

    if(midiData >> 7 == 1 && currentState != RUNNINGSTATUSCHECK){
      currentState = MIDI_STATUS;
    }
    // else we are receiving data of the same type as before

    switch(currentState){
      case(MIDI_STATUS):
        // If we're in one of the STATUS states
        // Record the event type in latestStatus
        // nextState always a channel (type1) or a parameter (type2) or another status (type3)
        switch(midiData_top4){
          case(0x9): // Note On
            messageType = 1;
            nextState = CHANNEL;
            break;
          case(0x8): // Note Off
            messageType = 1;
            nextState = CHANNEL;
            break;
          case(0xA): // Polyphonic Key Pressure
            messageType = 1;
            nextState = CHANNEL;
            break;
          case(0xB): // Control Change / Channel Mode depending on the channel bits
            messageType = 1;
            nextState = CHANNEL;
            break;
          case(0xE): // Pitch Bend Change
            messageType = 1;
            nextState = CHANNEL;
            break;
      

          case(0xC): // Program Change
            messageType = 2;
            nextState = PARAMETER;
            break;
          case(0xD): // Channel Pressure
            messageType = 2;
            nextState = PARAMETER;
            break;

          case(0xF):
            // This is the bunch of weird type3 messages
            switch(midiData_bottom4){
              // System Common Messages - ignored for now

              // Real-Time Messages
              case(0x8): // Timing Clock
                break;
              case(0x9): // Undefined
                break;
              case(0xA): // Start
                break;
              case(0xB): // Continue
                break;
              case(0xC): // Stop
                break;
              case(0xD): // Undefined
                break;
              case(0xE): // Active Sensing (connection alive every 300ms)
                break;
              case(0xF): // Reset
                break;
            }
            realTime = true;
            messageType = 3;
            nextState = MIDI_STATUS;
          
        }
        lastStatus = midiData;
        break;
      case(CHANNEL):
        nextState = VELOCITY;
        break;
      case(VELOCITY):
        nextState = RUNNINGSTATUSCHECK;
        break;
      case(PARAMETER):
        nextState = RUNNINGSTATUSCHECK;
        break;
      case(RUNNINGSTATUSCHECK):

        // if a 1 is detected we are not in Running Status
        // FIX: Real-Time Messages should not affect Running Status.
        if(midiData >> 7 == 1 && !realTime){
          nextState = MIDI_STATUS;
        }

        // Running Status triggered
        else{
          if(messageType == 1){
            nextState = CHANNEL;
          }
          else if(messageType == 2){
            nextState = PARAMETER;
          }
          else if(messageType == 3){
            nextState = MIDI_STATUS;
          }
        break;
      }
    }



    // Other logic

    // FIX: additional logic required to regroup Running Status with data bytes
    currentState = nextState;
    midievent.status1 = lastStatus;
    //midievent.param1 = channel;
    //midievent.param2 = velocity;
  }
}

