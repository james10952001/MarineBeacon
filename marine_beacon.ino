 /*
 Multi-pattern flasher for a marine navigation beacon
 Implements all IALA recommended flash patterns configurable via hex switches
 Note: This is intended for display use and is not intended be utilized in a functional navigation aid

 v0.8 initial release, ugly but tested and fully working (c)2019 James Sweet

 Prototype was built around an Arduino Nano clone, overkill but cheap and already on hand. The only external 
 compnents required are a logic level MOSFET or SSR for driving the lamp and a pair of hex encoded rotary 
 switches or a single 8 position DIP switch can be used. Selection switch pulls configuration pins to ground. 
 Flash patterns and selection codes taken directly from Sealite LED beacon brochure which can be used 
 as a reference.

 Ideas for future extension and improvements, many commercial units incorporate some of these features:
 - Add sun switch (photocell) utilizing analalog input
 - Rewrite flashing routine to be interrupt driven instead of using delays
 - Clean up pattern select routine to avoid big unwieldy switch statement
 - Store flash patterns in EEPROM instead of RAM
 - Implement sleep mode to reduce idle power consumption
 - Lamp current monitoring to detect incandescent lamp failure
 - Control of automatic lampchanger
 - GPS receiver with 1PPS output to synchronize flashing of multiple beacons
 - RF or WiFi module to enable remote configuration or monitoring
 - Battery voltage and solar panel voltage monitoring and battery management
 - PWM dimming to allow multiple brightness levels to be selected
 - Brightness ramp up/down to simulate look of incandescent lamp with LED source
 - Port to lower power hardware for use in self contained solar powered lights
*/



// Nidec-Copal S-1030A hex encoded rotary switches are a good fit
#define Sw_A0 3 // Hex switch A is on digital pins 3-6
#define Sw_A1 4 // Arduino does not have a complete 8 bit wide port available so individual pins are used
#define Sw_A2 5
#define Sw_A3 6
#define Sw_B0 7 // Hex switch B on digital pins 7-10
#define Sw_B1 8
#define Sw_B2 9
#define Sw_B3 10
#define LED 13  // LED already built into Arduino Nano, could be used to indicate a variety of things
#define Lamp 2 // Lamp driver is on digital pin 2



void setup()
{
  // Setup inputs and outputs and enable internal pullups on inputs
  pinMode(Lamp, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(Sw_A0, INPUT);
  pinMode(Sw_A1, INPUT);
  pinMode(Sw_A2, INPUT);
  pinMode(Sw_A3, INPUT);
  pinMode(Sw_B0, INPUT);
  pinMode(Sw_B1, INPUT);
  pinMode(Sw_B2, INPUT);
  pinMode(Sw_B3, INPUT);
  digitalWrite(Sw_A0, HIGH);
  digitalWrite(Sw_A1, HIGH);
  digitalWrite(Sw_A2, HIGH);
  digitalWrite(Sw_A3, HIGH);
  digitalWrite(Sw_B0, HIGH);
  digitalWrite(Sw_B1, HIGH);
  digitalWrite(Sw_B2, HIGH);
  digitalWrite(Sw_B3, HIGH);     
}



// Flash code descriptions:
// FL Flash followed by number Eg. FL 1 S, one flash every second
// F Fixed
// Q Quick flash
// VQ Very quick flash
// OC Occulting; greater period on than off
// ISO Isophase; equal period on and off
// LFL Long flash long
// MO Morse code ( ) contains letter
// For example, VQ (6) + LFL 10 S means 6 very quick flashes followed by a long flash, during a
// 10-second interval.
//
// Recommended Rhythm for Flashing Light - IALA Regions A and B
// Port Hand & Starboard Marks: Any, other than Composite Group Flashing (2+1)
// Preferred Channel Starboard: Composite Group Flashing (2+1)
// Preferred Channel Port: Composite Group Flashing (2+1)
// North Cardinal Mark: Very quick or quick
// East Cardinal Mark: Very quick (3) every 5 seconds or quick (3) every 10 seconds
// South Cardinal Mark: Very quick (6) + long flash every 10 seconds or quick (6) + long flash every 15 seconds
// West Cardinal Mark: Very quick (9) every 10 seconds or quick (9) every 15 seconds
// Isolated Danger Mark: Group flashing (2)
// Safe Water Mark: Isophase, occulting, one long flash every 10 seconds or Morse Code “A”
// Special Marks: Any, other than those described for Cardinal, Isolated Danger or Safe Water Marks

// Arrays to hold the flash patterns, these values are in 0.1s increments on, off
// Delays longer than 25.5 seconds can be accommodated by placing a zero between two delays
// Several unused places are available where you can add your own custom patterns if desired
byte Code01[] = {8, 32}; //FL 4 S
byte Code02[] = {10, 140}; //FL 15 S
byte Code03[] = {30, 20}; //OC 5 S
byte Code04[] = {5, 10}; //FL 1.5 S
byte Code05[] = {5, 11}; //FL 1.5 S
byte Code06[] = {2, 98}; //FL 10 S
byte Code07[] = {5, 10, 5, 30}; //FL (2) 5 S
byte Code08[] = {5, 15, 5, 15, 5, 105}; // FL (3) 15 S
byte Code09[] = {7, 5, 7, 5, 19, 107}; //FL (2 + 1) 15 S / MO (U) 15 S
byte Code0A[] = {5, 10, 5, 20}; //FL (2) 4 S
byte Code0B[] = {3, 10, 3, 10, 3, 61}; //FL (3) 9 S
byte Code0C[] = {3, 2, 3, 2, 3, 37}; //VQ (3) 5 S
byte Code0D[] = {5, 15, 5, 15, 5, 45, 5, 105}; //FL (4) 20 S
byte Code0E[] = {3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 67}; //VQ (9) 15 S
byte Code0F[] = {5, 5, 5, 5, 5, 5, 5, 165}; //Q (4) 20 S
byte Code10[] = {3, 12}; //FL 1.5 S
byte Code11[] = {10, 30}; //FL 4 S
byte Code12[] = {10, 10}; //ISO 2 S
byte Code13[] = {40, 10}; //OC 5 S
byte Code14[] = {15, 85}; //FL 10 S
byte Code15[] = {2, 28}; //FL 3 S
byte Code16[] = {3, 97}; //FL 10 S
byte Code17[] = {10, 10, 10, 20}; //FL (2) 5 S
byte Code18[] = {10, 10, 10, 40, 101}; //FL (2+1) 13.5 S
byte Code19[] = {7, 7, 7, 7, 21, 101}; //FL (2+1) 15 S
byte Code1A[] = {3, 10, 3, 29}; //FL (2) 4.5 S
byte Code1B[] = {5, 15, 5, 15, 5, 75}; //FL (3) 12 S
byte Code1C[] = {1, 5, 1, 5, 1, 137}; //VQ (3) 15 S
byte Code1D[] = {5, 10, 5, 10, 50, 10, 5, 50}; //FL (4) 10 S
byte Code1E[] = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 48}; //VQ (9) 15 S
byte Code1F[] = {2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 68}; //VQ (9) 15 S
byte Code20[] = {2, 18}; //FL 2 S
byte Code21[] = {15, 25}; //FL 4 S
byte Code22[] = {15, 15}; //ISO 3 S
byte Code23[] = {45, 5}; //OC 5 S
byte Code24[] = {40, 60}; //LFL 10 S
byte Code25[] = {4, 26}; //FL 3 S
byte Code26[] = {8, 92}; //FL 10 S
// byte Code27[] = {}; // Not used
byte Code28[] = {8, 12, 8, 32}; //FL (2) 6 S
byte Code29[] = {5, 5, 5, 35}; //Q (2) 5 S
byte Code2A[] = {4, 10, 4, 27}; //FL (2) 4.5 S
byte Code2B[] = {3, 4, 3, 12, 3, 35}; //FL (2+1) 6 S
byte Code2C[] = {2, 12, 2, 34}; //FL (2) 5 S
byte Code2D[] = {8, 12, 8, 12, 8, 12, 8, 32}; //FL (4) 10 S
byte Code2E[] = {3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 20, 70}; //Q (6) + LFL 15 S
byte Code2F[] = {2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 20, 70}; //Q (6) + LFL 15 S
byte Code30[] = {3, 17}; //FL 2 S                                                                                                                                                                                         byte Code30[] = {3, 17}; //FL 2 S
byte Code31[] = {13, 30}; //FL 3.2 S
byte Code32[] = {20, 20}; //ISO 4 S
byte Code33[] = {45, 15}; //OC 6 S
byte Code34[] = {20, 100}; //LFL 12 S
byte Code35[] = {20, 10}; //FL 3 S
byte Code36[] = {25, 15}; //OC 4 S
byte Code37[] = {10, 10, 10, 30}; //FL (2) 6 S
byte Code38[] = {5, 7, 5, 21, 5, 57}; //FL (2+1) 10 S
byte Code39[] = {3, 7, 3, 7, 3, 47}; //FL (2) 6 S
byte Code3A[] = {5, 10, 5, 25}; //FL (2) 4.5 S
byte Code3B[] = {5, 15, 5, 15, 5, 155}; //FL (3) 20 S
byte Code3C[] = {5, 10, 5, 10, 25}; //FL (3) 6 S
byte Code3D[] = {8, 12, 8, 12, 8, 12, 8, 52}; //FL (4) 12 S
byte Code3E[] = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 20, 58}; //Q (6) + LFL 15 S
byte Code3F[] = {15, 15, 15, 15, 15, 15, 15, 95}; //FL (4) 20 S                   
byte Code40[] = {4, 16}; //FL 2 S
byte Code41[] = {3, 47}; //FL 5 S
byte Code42[] = {25, 25}; //ISO 5 S
byte Code43[] = {50, 10}; //OC 6 S
byte Code44[] = {40, 110}; //LFL 15 S
byte Code45[] = {2, 38}; //FL 4 S
byte Code46[] = {25, 20}; //6 OC 3.5 S
byte Code47[] = {5, 10, 5, 60}; //FL (2) 8 S
byte Code48[] = {8, 12, 8, 24, 8, 60}; //FL (2+1) 12 S
byte Code49[] = {6, 4, 6, 84}; //Q (2) 10 S
byte Code4A[] = {4, 6, 4, 36}; //FL (2) 5 S
byte Code4B[] = {5, 30, 5, 30, 125}; //FL (3) 20 S
byte Code4C[] = {30, 20, 10, 20}; //OC (2) 8 S
byte Code4D[] = {5, 15, 5, 15, 5, 15, 5, 85}; //FL (4) 15 S
byte Code4E[] = {2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 58}; //VQ (9) 10 S
byte Code4F[] = {5, 5, 5, 5, 5, 5, 5, 85}; //FL (4) 12 S
byte Code50[] = {5, 15}; //FL 2 S
byte Code51[] = {5, 45}; //FL 5 S
byte Code52[] = {30, 30}; //ISO 6 S
byte Code53[] = {70, 30}; //OC 10 S
byte Code54[] = {20, 10}; //OC 3 S
byte Code55[] = {3, 37}; //FL 4 S
byte Code56[] = {45, 25}; //OC 7 S
byte Code57[] = {10, 10, 10, 50}; //FL (2) 8 S
byte Code58[] = {10, 10, 10, 40, 10, 40}; //FL (2+1) 12 S
byte Code59[] = {2, 3, 2, 3, 2, 38}; //VQ (3) 5 S
byte Code5A[] = {4, 14, 4, 33}; //FL (2) 5.5 S
byte Code5B[] = {8, 12, 8, 12, 8, 152}; //FL (3) 20 S
byte Code5C[] = {50, 10, 10, 10}; //OC (2) 8 S
byte Code5D[] = {5, 15, 5, 15, 5, 15, 5, 135}; //FL (4) 20 S
byte Code5E[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 49}; //VQ (9) 10 S
byte Code5F[] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 155}; //FL (5) 20 S                                                                                                                                                                                                                                                                                                             
byte Code60[] = {2, 13}; //FL 2 S
byte Code61[] = {10, 40}; //FL 5 S
byte Code62[] = {40, 40}; //ISO 8 S
byte Code63[] = {75, 25}; //OC 10 S 
byte Code64[] = {40, 20}; //OC 6 S
byte Code65[] = {6, 34}; //FL 4 S
byte Code66[] = {50, 30}; //OC 8 S
byte Code67[] = {5, 10, 5, 8}; //FL (2) 10 S
byte Code68[] = {10, 20, 10, 50, 10, 50}; //FL (2 + 1) 15 S
byte Code69[] = {5, 20, 5, 70}; //FL (2) 10 S
byte Code6A[] = {3, 10, 3, 44}; //FL (2) 6 S
byte Code6B[] = {10, 10, 10, 10, 10, 150}; //FL (3) 20 S
//byte Code6C[] = []; // Not used
//byte Code6D[] = {}; // Not used
byte Code6E[] = {2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 20, 50}; //VQ (6) + LFL 10 S
byte Code6F[] = {5, 5, 5, 5, 5, 5, 5, 255, 0, 10}; //FL (4) 30 S (hack to get 26.5sec using byte variables)
byte Code70[] = {3, 12}; //FL 2 S
byte Code71[] = {15, 35}; //FL 5 S
byte Code72[] = {50, 50}; //ISO 10 S
byte Code73[] = {2, 8}; //Q 1 S
byte Code74[] = {100, 50}; //OC 15 S
byte Code75[] = {12, 48}; //FL 6 S
byte Code76[] = {60, 30}; //OC 9 S
byte Code77[] = {5, 15, 5, 75}; //FL (2) 10 S
byte Code78[] = {3, 6, 10, 41}; //FL (2) 6 S/MO (A) 6 S
byte Code79[] = {5, 5, 5, 5, 5, 25}; //Q (3) 5 S
byte Code7A[] = {4, 10, 4, 42}; //FL (2) 6 S
byte Code7B[] = {4, 6, 20, 50}; //FL (2) 7 S
byte Code7C[] = {4, 6, 20, 50}; //MO (A) 8 S
byte Code7D[] = {15, 5, 5, 5, 5, 5, 5, 105}; //FL (4) 15 S/MO (B) 15 S
byte Code7E[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 20, 44}; //VQ (6) + LFL 10 S
byte Code7F[] = {5, 10, 5, 10, 5, 10, 5, 10, 50, 10, 5, 70}; //FL (6) 15 S                                                                             
byte Code80[] = {3, 22}; //FL 2.5 S
byte Code81[] = {5, 5}; //FL 6 S
byte Code82[] = {20, 30}; //LFL 5 S
byte Code83[] = {3, 7}; //Q 1 S
byte Code84[] = {8, 2}; //Q 1 S
byte Code85[] = {2, 48}; //FL 5 S
byte Code86[] = {60, 40}; //OC 10 S
byte Code87[] = {8, 12, 8, 72}; //FL (2) 10 S
byte Code88[] = {8, 12, 24, 36}; //FL (2) 8 S/MO (A) 8 S
byte Code89[] = {50, 10, 10, 10, 10, 10}; //OC (3) 10 S/MO (D) 10 S
byte Code8A[] = {4, 10, 4, 62}; //FL (2) 8 S
byte Code8B[] = {2, 8, 2, 138}; //Q (2) 15 S
//byte Code8C[] = {}; // Not used
byte Code8D[] = {4, 6, 4, 6, 4, 6, 4, 26}; //Q (4) 6 S
byte Code8E[] = {10, 10, 10, 10, 10, 10, 80}; //FL (4) 15 S
byte Code8F[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 20, 94}; //VQ (6) + LFL 15 S
byte Code90[] = {5, 20}; //FL 2.5 S
byte Code91[] = {6, 54}; //FL 6 S
byte Code92[] = {20, 40}; //LFL 6 S
byte Code93[] = {4, 6}; //Q 1 S
byte Code94[] = {5, 7}; //Q 1.2 S
byte Code95[] = {9, 41}; //FL 5 S
byte Code96[] = {20, 50}; //FL 7 S 
byte Code97[] = {10, 15, 10, 65}; //FL (2) 10 S
byte Code98[] = {5, 5, 15, 75}; //MO (A) 10 S
byte Code99[] = {5, 10, 4, 40}; //FL (2) 6 S
byte Code9A[] = {4, 16, 4, 76}; //FL (2) 10 S
byte Code9B[] = {3, 7, 3, 37}; //Q (2) 5
//byte Code9C[] = {}; // Not used
byte Code9D[] = {4, 10, 4, 10, 118}; //FL (3) 15 S
byte Code9E[] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 111}; //FL (5) 20 S
byte Code9F[] = {8, 12, 8, 12, 8, 12, 8, 12, 8, 112}; //FL (5) 20 S
byte CodeA0[] = {3, 27}; //FL 3 S
byte CodeA1[] = {10, 50}; //1 FL 6 S
byte CodeA2[] = {20, 60}; //LFL 8 S
byte CodeA3[] = {5, 5}; //Q 1 S
byte CodeA4[] = {10, 60}; //FL 7 S
byte CodeA5[] = {20, 180}; //LFL 20 S
byte CodeA6[] = {10, 20, 10, 110}; //FL (2) 15 S
byte CodeA7[] = {5, 15, 20, 110}; //FL (2) 15 S
byte CodeA8[] = {5, 15, 20, 110}; //FL (2) 15/MO (A) 15 S
byte CodeA9[] = {10, 10, 10, 40}; //Q (2) 6 S
byte CodeAA[] = {3, 9, 3, 45}; //FL (2) 6 S
byte CodeAB[] = {3, 7, 3, 7, 3, 37}; //Q (3) 6 S
//byte CodeAC[] = {}; // Not used
byte CodeAD[] = {3, 7, 3, 7, 3, 7, 3, 87}; //Q (4) 12 S
byte CodeAE[] = {5, 10, 5, 10, 5, 10, 5, 10, 5, 10, 5, 70};
byte CodeAF[] = {3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 97};
byte CodeB0[] = {15, 15}; //FL 3 S
byte CodeB1[] = {15, 45}; //FL 6 S
byte CodeB2[] = {30, 50}; //LFL 8 S
byte CodeB3[] = {3, 9}; //Q 1.2 S
byte CodeB4[] = {10, 70}; //FL 8 S
byte CodeB5[] = {3, 57}; //FL 6 S
byte CodeB6[] = {10, 10, 10, 30, 10, 50}; //FL (3) 12 S
byte CodeB7[] = {8, 12, 8, 12, 8, 42}; //FL (3) 9 S
byte CodeB8[] = {3, 7, 3, 7, 9, 71}; //FL (3) 10 S/MO (U) 10 S
byte CodeB9[] = {10, 10, 10, 70}; //FL (2) 10 S
byte CodeBA[] = {4, 10, 4, 102}; //FL (2) 12 S
byte CodeBB[] = {3, 7, 3, 7, 3, 77}; //Q (3) 10 S
//byte CodeBC[] = {}; // Not used
byte CodeBD[] = {3, 7, 3, 7, 3, 7, 3, 27}; //Q (4) 6 S
byte CodeBE[] = {3, 17, 3, 17, 3, 17, 3, 57}; //FL (4) 12 S
byte CodeBF[] = {3, 3, 3, 3, 3, 3, 3, 23}; //VQ (4) 4 S
byte CodeC0[] = {7, 23}; //FL 3 S
byte CodeC1[] = {8, 67}; //FL 7.5 S
byte CodeC2[] = {20, 80}; //LFL 10 S
byte CodeC3[] = {6, 6}; //Q 1.2 S
byte CodeC4[] = {10, 80}; //FL 9 S
byte CodeC5[] = {4, 56}; //FL 6 S
byte CodeC6[] = {2, 58}; //FL 6 S
byte CodeC7[] = {5, 15, 5, 15, 5, 55}; //FL (3) 10 S
byte CodeC8[] = {4, 6, 4, 6, 12, 68}; //FL (3) 10 S/MO (U) 10 S
byte CodeC9[] = {5, 10, 5, 100}; //FL (2) 12 S
byte CodeCA[] = {10, 30, 10, 150}; //FL (2) 20 S
byte CodeCB[] = {5, 5, 5, 5, 5, 75}; //FL (3) 10 S
//byte CodeCC[] = {}; // Not used
byte CodeCD[] = {3, 30, 3, 30, 3, 30, 3, 98}; //FL (4) 20 S
byte CodeCE[] = {5, 15, 5, 15, 5, 15, 5, 55}; //FL (4) 12 S
//byte CodeCF[] = {}; // Not used
byte CodeD0[] = {10, 20}; //FL 3 S
byte CodeD1[] = {5, 95}; //FL 10 S
byte CodeD2[] = {30, 70}; //LFL 10 S
byte CodeD3[] = {2, 3}; //VQ 0.5 S
byte CodeD4[] = {25, 95}; //FL 12 S
byte CodeD5[] = {5, 70}; //FL 7.5 S
byte CodeD6[] = {10, 15}; //FL 2.5 S
byte CodeD7[] = {10, 10, 10, 10, 10, 50}; //FL (3) 10 S
byte CodeD8[] = {5, 5, 5, 5, 15, 65}; //FL (2+1) 10 S/MO (U) 10 S
byte CodeD9[] = {15, 20, 15, 70}; //FL (2) 12 S
byte CodeDA[] = {10, 10, 10, 220}; //FL (2) 25 S
byte CodeDB[] = {6, 6, 6, 6, 6, 70}; //FL (3) 10 S
//byte CodeDC[] = {}; // Not used
byte CodeDD[] = {3, 7, 3, 7, 3, 7, 3, 7, 3, 27}; //Q (5) 7 S
byte CodeDE[] = {5, 15, 5, 15, 5, 15, 5, 95}; //FL (4) 16 S
//byte CodeDF[] = {}; // Not used
byte CodeE0[] = {4, 36}; //FL 4 S
byte CodeE1[] = {10, 90}; //FL 10 S
byte CodeE2[] = {25, 5}; //OC 3 S
byte CodeE3[] = {2, 4}; //VQ 0.6 S
byte CodeE4[] = {10, 250}; //FL 26 S
byte CodeE5[] = {5, 75}; //FL 8 S
//byte CodeE6[] = {}; // Not used
byte CodeE7[] = {8, 12, 8, 12, 8, 72}; //FL (3) 12 S
byte CodeE8[] = {5, 15, 5, 15, 5, 15, 5, 15, 5, 35}; //FL (5) 12 S
byte CodeE9[] = {3, 3, 3, 3, 3, 35}; //VQ (3) 5 S
byte CodeEA[] = {5, 20, 5, 20, 5, 65}; //FL (3) 12 S
byte CodeEB[] = {2, 10, 2, 26}; //VQ (2) 4 S
//byte CodeEC[] = {}; // Not used
byte CodeED[] = {3, 7, 3, 7, 3, 7, 3, 7, 3, 57}; //Q (5) 10 S
byte CodeEE[] = {5, 5, 5, 5, 5, 5, 5, 245}; //Q (4) 28 S
//byte CodeEF[] = {}; // Not used 
byte CodeF0[] = {5, 35}; //FL 4 S
byte CodeF1[] = {12, 108}; //FL 12 S
byte CodeF2[] = {30, 10}; //OC 4 S
byte CodeF3[] = {3, 3}; //VQ 0.6 S
byte CodeF4[] = {2, 13}; //FL 1.5 S
byte CodeF5[] = {9, 81}; //FL 9 S
//byte CodeF6[] = {}; // Not used
byte CodeF7[] = {3, 17, 3, 17, 3, 107}; //FL (3) 15 S
byte CodeF8[] = {6, 3, 6, 3, 14, 118}; //FL (2+1) 15 S/MO (U) 15 S
byte CodeF9[] = {2, 8, 2, 38}; //FL (2) 5 S
byte CodeFA[] = {5, 10, 5, 10, 5, 45}; //FL (3) 8 S
byte CodeFB[] = {2, 10, 2, 66}; //VQ (2) 8 S
//byte CodeFC[] = {}; // Not used
byte CodeFD[] = {3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 47}; //Q (6) 10 S
byte CodeFE[] = {3, 7, 3, 7, 3, 7, 3, 67}; //Q (4) 10 S
byte CodeFF[] = {6, 3, 14, 3, 6, 3, 6, 10, 14, 3, 14, 3, 14, 10, 6, 3, 14, 3, 6, 3, 6, 70}; //MO (LOL) 20 S // Not a real IALA code



// This takes in an array of timing values and blinks the lamp
// Nothing else needs doing in the meantime so just using delays is fine
void blinkLamp(byte pattern[], byte pattern_length)
{
  boolean lampState = false;
  for (byte i = 0;  i <  pattern_length; i++){
    int millisecs = pattern[i] * 100; // Timing values are stored as byte variables, so multiply by 100
    if (millisecs == 0){ // A bit of a hack to allow stacking intervals by placing a zero interval between
      lampState = !lampState;     
    }
    else
    {
      lampState = !lampState;
      digitalWrite(Lamp, lampState);
      digitalWrite(LED, lampState); // Just mirroring the lamp with the onboard LED
      delay(millisecs); 
    }
  }
}



// Read the configuration switches to get the flash pattern setting
// Set these to LOW if using true code and HIGH if using complementary code hex switches
byte readSwitches()
{
  byte sum = 0;
  if (digitalRead(Sw_A0) == HIGH)
    sum += 1;
  if (digitalRead(Sw_A1) == HIGH)
    sum += 2;
  if (digitalRead(Sw_A2) == HIGH)
    sum += 4;
  if (digitalRead(Sw_A3) == HIGH)
    sum += 8;
  if (digitalRead(Sw_B0) == HIGH)
    sum += 16;
  if (digitalRead(Sw_B1) == HIGH)
    sum += 32;
  if (digitalRead(Sw_B2) == HIGH)
    sum += 64;
  if (digitalRead(Sw_B3) == HIGH)
    sum += 128;  
  return sum;  
}



void loop()
{
  // Select the flash pattern based on the configuration switches
  // Gigantic switch case statement is uuuuuugggly but it does work
  byte setting = readSwitches();
  switch (setting) {
  case 0:
    digitalWrite(Lamp, HIGH); // Setting 0 is steady light, no flash pattern
    digitalWrite(LED, HIGH); // Mirror the main lamp with the onboard LED
    break;
  case 1:
    blinkLamp(Code01, sizeof(Code01));
    break;
  case 2:
    blinkLamp(Code02, sizeof(Code02));
    break;
  case 3:
    blinkLamp(Code03, sizeof(Code03));
    break;
  case 4:
    blinkLamp(Code04, sizeof(Code04));
    break;
  case 5:
    blinkLamp(Code05, sizeof(Code05));
    break;
  case 6:
    blinkLamp(Code06, sizeof(Code06));
    break;
  case 7:
    blinkLamp(Code07, sizeof(Code07));
    break;
  case 8:
    blinkLamp(Code08, sizeof(Code08));
    break;
  case 9:
    blinkLamp(Code09, sizeof(Code09));
    break;
  case 10:
    blinkLamp(Code0A, sizeof(Code0A));
    break;
  case 11:
    blinkLamp(Code0B, sizeof(Code0B));
    break;
  case 12:
    blinkLamp(Code0C, sizeof(Code0C));
    break;
  case 13:
    blinkLamp(Code0D, sizeof(Code0D));
    break;
  case 14:
    blinkLamp(Code0E, sizeof(Code0E));
    break;
  case 15:
    blinkLamp(Code0F, sizeof(Code0F));
    break;
  case 16:
    blinkLamp(Code10, sizeof(Code10));
    break;
  case 17:
    blinkLamp(Code11, sizeof(Code11));
    break;
  case 18:
    blinkLamp(Code12, sizeof(Code12));
    break;
  case 19:
    blinkLamp(Code13, sizeof(Code13));
    break;
  case 20:
    blinkLamp(Code14, sizeof(Code14));
    break;
  case 21:
    blinkLamp(Code15, sizeof(Code15));
    break;
  case 22:
    blinkLamp(Code16, sizeof(Code16));
    break;
  case 23:
    blinkLamp(Code17, sizeof(Code17));
    break;
  case 24:
    blinkLamp(Code18, sizeof(Code18));
    break;
  case 25:
    blinkLamp(Code19, sizeof(Code19));
    break;
  case 26:
    blinkLamp(Code1A, sizeof(Code1A));
    break;
  case 27:
    blinkLamp(Code1B, sizeof(Code1B));
    break;
  case 28:
    blinkLamp(Code1C, sizeof(Code1C));
    break;
  case 29:
    blinkLamp(Code1D, sizeof(Code1D));
    break;
  case 30:
    blinkLamp(Code1E, sizeof(Code1E));
    break;
  case 31:
    blinkLamp(Code1F, sizeof(Code1F));
    break;
  case 32:
    blinkLamp(Code20, sizeof(Code20));
    break;
  case 33:
    blinkLamp(Code21, sizeof(Code21));
    break;
  case 34:
    blinkLamp(Code22, sizeof(Code22));
    break;
  case 35:
    blinkLamp(Code23, sizeof(Code23));
    break;
  case 36:
    blinkLamp(Code24, sizeof(Code24));
    break;
  case 37:
    blinkLamp(Code25, sizeof(Code25));
    break;
  case 38:
    blinkLamp(Code26, sizeof(Code26));
    break;
  case 40:
    blinkLamp(Code28, sizeof(Code28));
    break;
  case 41:
    blinkLamp(Code29, sizeof(Code29));
    break;
  case 42:
    blinkLamp(Code2A, sizeof(Code2A));
    break;
  case 43:
    blinkLamp(Code2B, sizeof(Code2B));
    break;
  case 44:
    blinkLamp(Code2C, sizeof(Code2C));
    break;
  case 45:
    blinkLamp(Code2D, sizeof(Code2D));
    break;
  case 46:
    blinkLamp(Code2E, sizeof(Code2E));
    break;
  case 47:
    blinkLamp(Code2F, sizeof(Code2F));
    break;
  case 48:
    blinkLamp(Code30, sizeof(Code30));
    break;
  case 49:
    blinkLamp(Code31, sizeof(Code31));
    break;
  case 50:
    blinkLamp(Code32, sizeof(Code32));
    break;
  case 51:
    blinkLamp(Code33, sizeof(Code33));
    break;
  case 52:
    blinkLamp(Code34, sizeof(Code34));
    break;
  case 53:
    blinkLamp(Code35, sizeof(Code35));
    break;
  case 54:
    blinkLamp(Code36, sizeof(Code36));
    break;
  case 55:
    blinkLamp(Code37, sizeof(Code37));
    break;
  case 56:
    blinkLamp(Code38, sizeof(Code38));
    break;
  case 57:
    blinkLamp(Code39, sizeof(Code39));
    break;
  case 58:
    blinkLamp(Code3A, sizeof(Code3A));
    break;
  case 59:
    blinkLamp(Code3B, sizeof(Code3B));
    break;
  case 60:
    blinkLamp(Code3C, sizeof(Code3C));
    break;
  case 61:
    blinkLamp(Code3D, sizeof(Code3D));
    break;
  case 62:
    blinkLamp(Code3E, sizeof(Code3E));
    break;
  case 63:
    blinkLamp(Code3F, sizeof(Code3F));
    break;
  case 64:
    blinkLamp(Code40, sizeof(Code40));
    break;
  case 65:
    blinkLamp(Code41, sizeof(Code41));
    break;
  case 66:
    blinkLamp(Code42, sizeof(Code42));
    break;
  case 67:
    blinkLamp(Code43, sizeof(Code43));
    break;
  case 68:
    blinkLamp(Code44, sizeof(Code44));
    break;
  case 69:
    blinkLamp(Code45, sizeof(Code45));
    break;
  case 70:
    blinkLamp(Code46, sizeof(Code46));
    break;
  case 71:
    blinkLamp(Code47, sizeof(Code47));
    break;
  case 72:
    blinkLamp(Code48, sizeof(Code48));
    break;
  case 73:
    blinkLamp(Code49, sizeof(Code49));
    break;
  case 74:
    blinkLamp(Code4A, sizeof(Code4A));
    break;
  case 75:
    blinkLamp(Code4B, sizeof(Code4B));
    break;
  case 76:
    blinkLamp(Code4C, sizeof(Code4C));
    break;
  case 77:
    blinkLamp(Code4D, sizeof(Code4D));
    break;
  case 78:
    blinkLamp(Code4E, sizeof(Code4E));
    break;
  case 79:
    blinkLamp(Code4F, sizeof(Code4F));
    break;
  case 80:
    blinkLamp(Code50, sizeof(Code50));
    break;
  case 81:
    blinkLamp(Code51, sizeof(Code51));
    break;
  case 82:
    blinkLamp(Code52, sizeof(Code52));
    break;
  case 83:
    blinkLamp(Code53, sizeof(Code53));
    break;
  case 84:
    blinkLamp(Code54, sizeof(Code54));
    break;
  case 85:
    blinkLamp(Code55, sizeof(Code55));
    break;
  case 86:
    blinkLamp(Code56, sizeof(Code56));
    break;
  case 87:
    blinkLamp(Code57, sizeof(Code57));
    break;
  case 88:
    blinkLamp(Code58, sizeof(Code58));
    break;
  case 89:
    blinkLamp(Code59, sizeof(Code59));
    break;
  case 90:
    blinkLamp(Code5A, sizeof(Code5A));
    break;
  case 91:
    blinkLamp(Code5B, sizeof(Code5B));
    break;
  case 92:
    blinkLamp(Code5C, sizeof(Code5C));
    break;
  case 93:
    blinkLamp(Code5D, sizeof(Code5D));
    break;
  case 94:
    blinkLamp(Code5E, sizeof(Code5E));
    break;
  case 95:
    blinkLamp(Code5F, sizeof(Code5F));
    break;
  case 96:
    blinkLamp(Code60, sizeof(Code60));
    break;
  case 97:
    blinkLamp(Code61, sizeof(Code61));
    break;
  case 98:
    blinkLamp(Code62, sizeof(Code62));
    break;
  case 99:
    blinkLamp(Code63, sizeof(Code63));
    break;
  case 100:
    blinkLamp(Code64, sizeof(Code64));
    break;
  case 101:
    blinkLamp(Code65, sizeof(Code65));
    break;
  case 102:
    blinkLamp(Code66, sizeof(Code66));
    break;
  case 103:
    blinkLamp(Code67, sizeof(Code67));
    break;
  case 104:
    blinkLamp(Code68, sizeof(Code68));
    break;
  case 105:
    blinkLamp(Code69, sizeof(Code69));
    break;
  case 106:
    blinkLamp(Code6A, sizeof(Code6A));
    break;
  case 107:
    blinkLamp(Code6B, sizeof(Code6B));
    break;
  case 110:
    blinkLamp(Code6E, sizeof(Code6E));
    break;
  case 111:
    blinkLamp(Code6F, sizeof(Code6F));
    break;
  case 112:
    blinkLamp(Code70, sizeof(Code70));
    break;
  case 113:
    blinkLamp(Code71, sizeof(Code71));
    break;
  case 114:
    blinkLamp(Code72, sizeof(Code72));
    break;
  case 115:
    blinkLamp(Code73, sizeof(Code73));
    break;
  case 116:
    blinkLamp(Code74, sizeof(Code74));
    break;
  case 117:
    blinkLamp(Code75, sizeof(Code75));
    break;
  case 118:
    blinkLamp(Code76, sizeof(Code76));
    break;
  case 119:
    blinkLamp(Code77, sizeof(Code77));
    break;
  case 120:
    blinkLamp(Code78, sizeof(Code78));
    break;
  case 121:
    blinkLamp(Code79, sizeof(Code79));
    break;
  case 122:
    blinkLamp(Code7A, sizeof(Code7A));
    break;
  case 123:
    blinkLamp(Code7B, sizeof(Code7B));
    break;
  case 124:
    blinkLamp(Code7C, sizeof(Code7C));
    break;
  case 125:
    blinkLamp(Code7D, sizeof(Code7D));
    break;
  case 126:
    blinkLamp(Code7E, sizeof(Code7E));
    break;
  case 127:
    blinkLamp(Code7F, sizeof(Code7F));
    break;
  case 128:
    blinkLamp(Code80, sizeof(Code80));
    break;
  case 129:
    blinkLamp(Code81, sizeof(Code81));
    break;
  case 130:
    blinkLamp(Code82, sizeof(Code82));
    break;
  case 131:
    blinkLamp(Code83, sizeof(Code83));
    break;
  case 132:
    blinkLamp(Code84, sizeof(Code84));
    break;
  case 133:
    blinkLamp(Code85, sizeof(Code85));
    break;
  case 134:
    blinkLamp(Code86, sizeof(Code86));
    break;
  case 135:
    blinkLamp(Code87, sizeof(Code87));
    break;
  case 136:
    blinkLamp(Code88, sizeof(Code88));
    break;
  case 137:
    blinkLamp(Code89, sizeof(Code89));
    break;
  case 138:
    blinkLamp(Code8A, sizeof(Code8A));
    break;
  case 139:
    blinkLamp(Code8B, sizeof(Code8B));
    break;
  case 141:
    blinkLamp(Code8D, sizeof(Code8D));
    break;
  case 142:
    blinkLamp(Code8E, sizeof(Code8E));
    break;
  case 143:
    blinkLamp(Code8F, sizeof(Code8F));
    break;
  case 144:
    blinkLamp(Code90, sizeof(Code90));
    break;
  case 145:
    blinkLamp(Code91, sizeof(Code91));
    break;
  case 146:
    blinkLamp(Code92, sizeof(Code92));
    break;
  case 147:
    blinkLamp(Code93, sizeof(Code93));
    break;
  case 148:
    blinkLamp(Code94, sizeof(Code94));
    break;
  case 149:
    blinkLamp(Code95, sizeof(Code95));
    break;
  case 150:
    blinkLamp(Code96, sizeof(Code96));
    break;
  case 151:
    blinkLamp(Code97, sizeof(Code97));
    break;
  case 152:
    blinkLamp(Code98, sizeof(Code98));
    break;
  case 153:
    blinkLamp(Code99, sizeof(Code99));
    break;
  case 154:
    blinkLamp(Code9A, sizeof(Code9A));
    break;
  case 155:
    blinkLamp(Code9B, sizeof(Code9B));
    break;
  case 157:
    blinkLamp(Code9D, sizeof(Code9D));
    break;
  case 158:
    blinkLamp(Code9E, sizeof(Code9E));
    break;
  case 159:
    blinkLamp(Code9F, sizeof(Code9F));
    break;
  case 160:
    blinkLamp(CodeA0, sizeof(CodeA0));
    break;
  case 161:
    blinkLamp(CodeA1, sizeof(CodeA1));
    break;
  case 162:
    blinkLamp(CodeA2, sizeof(CodeA2));
    break;
  case 163:
    blinkLamp(CodeA3, sizeof(CodeA3));
    break;
  case 164:
    blinkLamp(CodeA4, sizeof(CodeA4));
    break;
  case 165:
    blinkLamp(CodeA5, sizeof(CodeA5));
    break;
  case 166:
    blinkLamp(CodeA6, sizeof(CodeA6));
    break;
  case 167:
    blinkLamp(CodeA7, sizeof(CodeA7));
    break;
  case 168:
    blinkLamp(CodeA8, sizeof(CodeA8));
    break;
  case 169:
    blinkLamp(CodeA9, sizeof(CodeA9));
    break;
  case 170:
    blinkLamp(CodeAA, sizeof(CodeAA));
    break;
  case 171:
    blinkLamp(CodeAB, sizeof(CodeAB));
    break;
  case 173:
    blinkLamp(CodeAD, sizeof(CodeAD));
    break;
  case 174:
    blinkLamp(CodeAE, sizeof(CodeAE));
    break;
  case 175:
    blinkLamp(CodeAF, sizeof(CodeAF));
    break;
  case 176:
    blinkLamp(CodeB0, sizeof(CodeB0));
    break;
  case 177:
    blinkLamp(CodeB1, sizeof(CodeB1));
    break;
  case 178:
    blinkLamp(CodeB2, sizeof(CodeB2));
    break;
  case 179:
    blinkLamp(CodeB3, sizeof(CodeB3));
    break;
  case 180:
    blinkLamp(CodeB4, sizeof(CodeB4));
    break;
  case 181:
    blinkLamp(CodeB5, sizeof(CodeB5));
    break;
  case 182:
    blinkLamp(CodeB6, sizeof(CodeB6));
    break;
  case 183:
    blinkLamp(CodeB7, sizeof(CodeB7));
    break;
  case 184:
    blinkLamp(CodeB8, sizeof(CodeB8));
    break;
  case 185:
    blinkLamp(CodeB9, sizeof(CodeB9));
    break;
  case 186:
    blinkLamp(CodeBA, sizeof(CodeBA));
    break;
  case 187:
    blinkLamp(CodeBB, sizeof(CodeBB));
    break;
  case 189:
    blinkLamp(CodeBD, sizeof(CodeBD));
    break;
  case 190:
    blinkLamp(CodeBE, sizeof(CodeBE));
    break;
  case 191:
    blinkLamp(CodeBF, sizeof(CodeBF));
    break;
  case 192:
    blinkLamp(CodeC0, sizeof(CodeC0));
    break;
  case 193:
    blinkLamp(CodeC1, sizeof(CodeC1));
    break;
  case 194:
    blinkLamp(CodeC2, sizeof(CodeC2));
    break;
  case 195:
    blinkLamp(CodeC3, sizeof(CodeC3));
    break;
  case 196:
    blinkLamp(CodeC4, sizeof(CodeC4));
    break;
  case 197:
    blinkLamp(CodeC5, sizeof(CodeC5));
    break;
  case 198:
    blinkLamp(CodeC6, sizeof(CodeC6));
    break;
  case 199:
    blinkLamp(CodeC7, sizeof(CodeC7));
    break;
  case 200:
    blinkLamp(CodeC8, sizeof(CodeC8));
    break;
  case 201:
    blinkLamp(CodeC9, sizeof(CodeC9));
    break;
  case 202:
    blinkLamp(CodeCA, sizeof(CodeCA));
    break;
  case 203:
    blinkLamp(CodeCB, sizeof(CodeCB));
    break;
  case 205:
    blinkLamp(CodeCD, sizeof(CodeCD));
    break;
  case 206:
    blinkLamp(CodeCE, sizeof(CodeCE));
    break;
  case 208:
    blinkLamp(CodeD0, sizeof(CodeD0));
    break;
  case 209:
    blinkLamp(CodeD1, sizeof(CodeD1));
    break;
  case 210:
    blinkLamp(CodeD2, sizeof(CodeD2));
    break;
  case 211:
    blinkLamp(CodeD3, sizeof(CodeD3));
    break;
  case 212:
    blinkLamp(CodeD4, sizeof(CodeD4));
    break;
  case 213:
    blinkLamp(CodeD5, sizeof(CodeD5));
    break;
  case 214:
    blinkLamp(CodeD6, sizeof(CodeD6));
    break;
  case 215:
    blinkLamp(CodeD7, sizeof(CodeD7));
    break;
  case 216:
    blinkLamp(CodeD8, sizeof(CodeD8));
    break;
  case 217:
    blinkLamp(CodeD9, sizeof(CodeD9));
    break;
  case 218:
    blinkLamp(CodeDA, sizeof(CodeDA));
    break;
  case 219:
    blinkLamp(CodeDB, sizeof(CodeDB));
    break;
  case 221:
    blinkLamp(CodeDD, sizeof(CodeDD));
    break;
  case 222:
    blinkLamp(CodeDE, sizeof(CodeDE));
    break;
  case 224:
    blinkLamp(CodeE0, sizeof(CodeE0));
    break;
  case 225:
    blinkLamp(CodeE1, sizeof(CodeE1));
    break;
  case 226:
    blinkLamp(CodeE2, sizeof(CodeE2));
    break;
  case 227:
    blinkLamp(CodeE3, sizeof(CodeE3));
    break;
  case 228:
    blinkLamp(CodeE4, sizeof(CodeE4));
    break;
  case 229:
    blinkLamp(CodeE5, sizeof(CodeE5));
    break;
  case 231:
    blinkLamp(CodeE7, sizeof(CodeE7));
    break;
  case 232:
    blinkLamp(CodeE8, sizeof(CodeE8));
    break;
  case 233:
    blinkLamp(CodeE9, sizeof(CodeE9));
    break;
  case 234:
    blinkLamp(CodeEA, sizeof(CodeEA));
    break;
  case 235:
    blinkLamp(CodeEB, sizeof(CodeEB));
    break;
  case 237:
    blinkLamp(CodeED, sizeof(CodeED));
    break;
  case 238:
    blinkLamp(CodeEE, sizeof(CodeEE));
    break;
  case 240:
    blinkLamp(CodeF0, sizeof(CodeF0));
    break;
  case 241:
    blinkLamp(CodeF1, sizeof(CodeF1));
    break;
  case 242:
    blinkLamp(CodeF2, sizeof(CodeF2));
    break;
  case 243:
    blinkLamp(CodeF3, sizeof(CodeF3));
    break;
  case 244:
    blinkLamp(CodeF4, sizeof(CodeF4));
    break;
  case 245:
    blinkLamp(CodeF5, sizeof(CodeF5));
    break;
  case 247:
    blinkLamp(CodeF7, sizeof(CodeF7));
    break;
  case 248:
    blinkLamp(CodeF8, sizeof(CodeF8));
    break;
  case 249:
    blinkLamp(CodeF9, sizeof(CodeF9));
    break;
  case 250:
    blinkLamp(CodeFA, sizeof(CodeFA));
    break;
  case 251:
    blinkLamp(CodeFB, sizeof(CodeFB));
    break;
  case 253:
    blinkLamp(CodeFD, sizeof(CodeFD));
    break;
  case 254:
    blinkLamp(CodeFE, sizeof(CodeFE));
    break;
  default: // End up here if invalid code is selected
    blinkLamp(CodeFF, sizeof(CodeFF));
    break;
  }
}
