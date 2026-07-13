#include <Arduino.h>
#include <MIDI.h>
#include <deque>
#include <utility>
#include <string>

// Replace with hardware serial if possible
// SoftwareSerial midiSerial(4, 3); // RX = pin 4, TX = pin 3
const int MIDI_IN_PIN = 4;
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

struct Voice{
    int pitch;
    int octave;
    bool free = true;
};

const int iPOLYPHONY_BUFFER_SIZE = 6;

Voice voices[iPOLYPHONY_BUFFER_SIZE]; // creates a fixed-length array of length iPOLYPHONY_BUFFER_SIZE
std::deque<int> dqiNotePriority;

std::pair<std::string, int> psiMIDINotetoSPN(const int MIDInote){
    int iScaleIndex = MIDInote % 12;
    int iOctaveIndex = MIDInote / 12; // integer division
    std::string pitchName;
    int octave = iOctaveIndex - 1; // This can be shown to be true (assuming that iOctaveIndex is a quotient. Look at the MIDI table)
    // C has remainder 0
    // Which means C# has remainder 1.... etc
    switch(iScaleIndex){
        case 0:
            pitchName = "C";
            break;
        case 1:
            pitchName = "C#";
            break;
        case 2:
            pitchName = "D";
            break;
        case 3:
            pitchName = "D#";
            break;
        case 4:
            pitchName = "E";
            break;
        case 5:
            pitchName = "F";
            break;
        case 6:
            pitchName = "F#";
            break;
        case 7:
            pitchName = "G";
            break;
        case 8:
            pitchName = "G#";
            break;
        case 9:
            pitchName = "A";
            break;
        case 10:
            pitchName = "A#";
            break;
        case 11:
            pitchName = "B";
            break;
        default:
            pitchName = "Z";
            break;
    }
    return std::make_pair(pitchName, octave);
}


int iFindIndexInDeque(int ipitchNum){
    // Search the note-priority deque for the given MIDI pitch and return its index
    for(int i = 0; i < dqiNotePriority.size(); i++){
        if(dqiNotePriority[i] == ipitchNum){
            return i;
        }
    }
    return -1;
}

int findFreeVoice(){
    for(int i = 0; i < iPOLYPHONY_BUFFER_SIZE; i++){
        if(voices[i].free){
            return i;
        }
    }
    return -1;
}

int findVoicefromPitch(int pitch){
    for(int i = 0; i < iPOLYPHONY_BUFFER_SIZE; i++){
        if(voices[i].pitch == pitch){
            return i;
        }
    }
    return -1;
}

void handleNoteOn(byte channel, byte pitch, byte velocity){
    Serial.print("Note On  | Channel: ");
    Serial.print(channel);
    Serial.print(" | Pitch: ");
    Serial.print(pitch);
    Serial.print(" | Velocity: ");
    Serial.println(velocity);
    Serial.print(" | Assigned voice: ");
    // Look for a free voice
    int iNextFreeVoice = findFreeVoice();
    if(iNextFreeVoice != -1){ // If a free voice is found
        voices[iNextFreeVoice].pitch = pitch;
        voices[iNextFreeVoice].octave = pitch / 12;
        voices[iNextFreeVoice].free = false;
        //voices[iNextFreeVoice].velocity = velocity;
        dqiNotePriority.push_back(pitch);
        Serial.println(iNextFreeVoice);
    }
    else{ // If no free voices are found, remove the oldest voice

        // Get the element at the front of dqiNotePriority
        int iOldestVoice_pitch = dqiNotePriority.front();
        // Find the index of the voice with this pitch
        int iOldestVoice_index = findVoicefromPitch(iOldestVoice_pitch);
        // Remove all data
        // And rewrite with the new note
        if(iOldestVoice_index != -1){
            dqiNotePriority.pop_front();
            voices[iOldestVoice_index].pitch = pitch;
            voices[iOldestVoice_index].octave = pitch / 12;
            voices[iOldestVoice_index].free = false;
            //voices[iOldestVoice_index].velocity = velocity;
            dqiNotePriority.push_back(pitch);
            Serial.println(iOldestVoice_index);
        }
        else{
            Serial.println("Tried to overwrite a note, but its index cannot be found.");
        }

    }
}

void handleNoteOff(byte channel, byte pitch, byte velocity){
    Serial.print("Note Off | Channel: ");
    Serial.print(channel);
    Serial.print(" | Pitch: ");
    Serial.print(pitch);
    Serial.print(" | Velocity: ");
    Serial.println(velocity);
    // Look for switched off note's pitch in the running voices
    int iSwitchedOffVoice = findVoicefromPitch(pitch);
    // Mark note as free in the array and remove from deque if present
    if(iSwitchedOffVoice != -1){
        voices[iSwitchedOffVoice].free = true;
        // Find the DEQUE index of the note to be removed

        int iSwitchedOffVoiceIndex_deque = iFindIndexInDeque(pitch);
        dqiNotePriority.erase(dqiNotePriority.begin() + iSwitchedOffVoiceIndex_deque);
    }
    else{
        Serial.println("Tried to turn off a note that was force-removed by the polyphony limit, or never played");
    }
}

void setup() {
    Serial.begin(115200);
    Serial1.begin(31250, SERIAL_8N1, MIDI_IN_PIN, -1, false);
    while(Serial1.available()){
        Serial1.read();
    }
    delay(500);
    Serial.println("MIDI Receiver Initializing...");

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