#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MIDI.h>
#include <deque>

SoftwareSerial midiSerial(4, 3); // RX = pin 4, TX = pin 3 (unused but required)

MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, MIDI);

int iFindIndexInDeque(int ipitchNum, std::deque<int> &dqiPolyphonyFIFO){
    bool bfound = false;
    for(int i = 0; i < dqiPolyphonyFIFO.size(); i++){
        if(dqiPolyphonyFIFO.at(i) == ipitchNum){
            bfound = true;
            return i;
        }
    }
    return -1;
}

int iPOLYPHONY_BUFFER_SIZE = 6;
std::deque<int> dqiPolyphonyFIFO;


void handleNoteOn(byte channel, byte pitch, byte velocity) {
    if(dqiPolyphonyFIFO.size() < iPOLYPHONY_BUFFER_SIZE){
        dqiPolyphonyFIFO.push_back(pitch);
    }
    else if(dqiPolyphonyFIFO.size() >= iPOLYPHONY_BUFFER_SIZE){
        Serial.println("Polyphony limit reached");
        dqiPolyphonyFIFO.pop_front();
        dqiPolyphonyFIFO.push_back(pitch);
    }
    Serial.print("Note On  | Channel: ");
    Serial.print(channel);
    Serial.print(" | Pitch: ");
    Serial.print(pitch);
    Serial.print(" | Velocity: ");
    Serial.println(velocity);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
    int iIndexInBuffer = iFindIndexInDeque(pitch, dqiPolyphonyFIFO);
    if(iIndexInBuffer != -1){
        dqiPolyphonyFIFO.erase(dqiPolyphonyFIFO.begin() + iIndexInBuffer);
    }
    else{
        Serial.println("For some reason an attempt was made to turn off a note that was never played. Safely continuing...");
    }
    Serial.print("Note Off | Channel: ");
    Serial.print(channel);
    Serial.print(" | Pitch: ");
    Serial.print(pitch);
    Serial.print(" | Velocity: ");
    Serial.println(velocity);
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("MIDI Receiver Initializing...");

    midiSerial.begin(31250);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);

    Serial.println("MIDI Receiver Ready!");
}

void loop() {
    MIDI.read();
    
    // OUTPUTS:
    // Six active TOG selects
    // Will need a separate function to encode the TOG selects
    // Six divider signals
}
