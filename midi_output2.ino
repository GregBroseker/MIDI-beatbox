

// ************ BrosBeats **************
// ***                                                                ***
// ***********************************
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

const int MUX_1 =    3; // Output of MUX 1
const int MUX_2 =    4; // Output of MUX 2
const int MUX_A =    7; // LSB of MUX address selector
const int MUX_B =    6;
const int MUX_C =    5; // MSB of MUX address selector 

// Wire pot 1 to A0
// Wire pot 2 to A1
// Wire pot 3 to A2      
// Wire pot 4 to A3
     
const int bankSelect = A4;
const int potMax = 1023;

boolean buttons_old[16];
boolean buttons_new[16];
int pots_old[5];
int pots_new[5];

// ********************************************************
void setup() {
  MIDI.begin (); // MIDI START
  // Serial.begin(9600); 
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  pinMode(MUX_1, INPUT_PULLUP);
  pinMode(MUX_2, INPUT_PULLUP);
  
  // initialize button and pot arrays
  for (int i = 0; i < 16; i++) {
    buttons_old[i] = HIGH;
    pots_old[i] = 0;
  }
}

// ******************** Functions *********************

int getKey(int pressed){
  int bank = map(analogRead(bankSelect), 0, potMax, 0, 3);
  return bank * 16 + pressed;
}

void incrementCounter(){  // 3-bit ring counter
  static boolean Q1; static boolean Q2; static boolean Q3;

  Q1 = ! Q1;
  // On falling edge of Q1, toggle Q2
  if (!Q1) Q2 = !Q2;
  // On falling edge of Q2, toggle Q3
  if(!Q1 && ! Q2) Q3 = !Q3;
  
  // Output binary count to MUX address pins
  digitalWrite(MUX_A, Q1); 
  digitalWrite(MUX_B, Q2);
  digitalWrite(MUX_C, Q3);
}
// ******************************************************

void loop() {

  // *** Scan the button MUX for key presses ***
  // Wire so that a button press pulls MUX input low

  for(int i = 0; i < 16; i++){  // for each button
      if (i < 8) {
        buttons_new[i] = digitalRead(MUX_1);
      }
      else buttons_new[i] = digitalRead(MUX_2);
      incrementCounter();
      // send the appropriate note or turn it off
      if (buttons_new[i] != buttons_old[i]){
        if (buttons_new[i] == LOW){             // if button pressed
          MIDI.sendNoteOn(36 + getKey(i), 127, 1);
        }
        else {                                  // if button released  
          MIDI.sendNoteOff(35 + getKey(i), 0, 1);
        }
        buttons_old[i] = buttons_new[i];
      }  
  }
  // *** Scan the 4 continuous pots ***
  for(int i = 0; i < 4; i++){
      pots_new[i] = map(potMax - analogRead(14 + i), 0, potMax, 0, 120); // A0 = 14 so this scans A0-A3
      if (abs(pots_new[i]- pots_old[i]) >= 4){  // if change is above noise floor
         pots_old[i]= pots_new[i];
         // assume pot 1 CC is 54, pot 2 is 55 etc
         //Serial.print(54+i);Serial.print("\t"); Serial.println(pots_new[i]);
         MIDI.sendControlChange(54 + i, pots_new[i], 1);
      }
  }
  delay(20);
}
