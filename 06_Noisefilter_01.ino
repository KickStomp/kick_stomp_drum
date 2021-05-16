/*  Example playing noise, colored noise tuneable
    using Mozzi sonification library.

    This sketch using HIFI mode on AVR (i.e. the classic Arduino borads, not Teensy 3.x and friends).

    IMPORTANT: this sketch requires Mozzi/mozzi_config.h to be
    be changed from STANDARD mode to HIFI.
    In Mozz/mozzi_config.h, change
    #define AUDIO_MODE STANDARD
    //#define AUDIO_MODE HIFI
    to
    //#define AUDIO_MODE STANDARD
    #define AUDIO_MODE HIFI

    Circuit: Audio output on digital pin 9 and 10 (on a Uno or similar),
    Check the Mozzi core module documentation for others and more detail

                     3.9k
     pin 9  ---WWWW-----|-----output
                    499k           |
     pin 10 ---WWWW---- |
                                       |
                             4.7n  ==
                                       |
                                   ground

    Resistors are ±0.5%  Measure and choose the most precise
    from a batch of whatever you can get.  Use two 1M resistors
    in parallel if you can't find 499k.
    Alternatively using 39 ohm, 4.99k and 470nF components will
    work directly with headphones.

    Mozzi documentation/API
		https://sensorium.github.io/Mozzi/doc/html/index.html

		Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users

    Tim Barrass 2012-13, CC by-nc-sa.
*/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <EventDelay.h> // for scheduling events
#include <ADSR.h> // ADSR envelope

#include <tables/sin512_int8.h> // sine wavetable for oscillator, length 512 is normally sufficient
// some more wavetables
#include <tables/square_analogue512_int8.h>
#include <tables/square_no_alias512_int8.h>
#include <tables/saw_analogue512_int8.h>
#include <tables/triangle_analogue512_int8.h>
#include <tables/triangle512_int8.h>

// rate (repetitions / second) of function updateControl()
#define CONTROL_RATE 64

// digitalReadFast
// Standard Arduino Pins
#define digitalPinToPINReg(P) \
(((P) >= 0 && (P) <= 7) ? &PIND : (((P) >= 8 && (P) <= 13) ? &PINB : &PINC))
#define digitalPinToBit(P) \
(((P) >= 0 && (P) <= 7) ? (P) : (((P) >= 8 && (P) <= 13) ? (P) - 8 : (P) - 14))

#define digitalReadFast(P) bitRead(*digitalPinToPINReg(P), digitalPinToBit(P))

Oscil <512, AUDIO_RATE> kickOscil; // oscillator for kickdrum, fixed length of 512 for easy wavetable switching
ADSR <AUDIO_RATE, AUDIO_RATE> kick_wave_envelope; // envelope for kickdrum

// for triggering the envelope
EventDelay noteDelay;
 
// definition of used analog ports, adjust according to your setup
uint8_t analog_input_port[]  = {A4, A5, A6, A7, A3};
// BPM, Pattern, Randomness / Trigger-Level, Instrument, Piezo

// digital-ports (switches)
uint8_t digital_input_port[]  = {4, 2, 3};
// Metronome, Shuffle, Fill

uint8_t noise_color = 0;
uint8_t switch1_state = 0;
uint8_t switch2_state = 0;
uint8_t switch3_state = 0;
uint8_t fs1 = 0;
uint8_t fs2 = 0;

int16_t LP=0, HP=0, BP=0;

void setup(){
  Serial.begin(115200);
  startMozzi(); // uses the default control rate of 64, defined in mozzi_config.h
    noteDelay.set(2000); // 2 second countdown

  kickOscil.setFreq(52); // set the frequency

  // setup digital ports with internal pullup
  pinMode(digital_input_port[0],INPUT_PULLUP);
  pinMode(digital_input_port[1],INPUT_PULLUP);
  pinMode(digital_input_port[2],INPUT_PULLUP);

      // set the envelopes : timing & levels
      
    kick_wave_envelope.setTimes(10, 10, 20, 40);
                                 
    kick_wave_envelope.setLevels(255, 128, 32, 0);


}

// from https://www.avrfreaks.net/forum/tiny-fast-prng
/*
s= one of 15 18 63 101 129 156 172 208 and
a=96..138
you'll get a period of 55552. These are contiguous ranges of 43 seeds with good period. So you could seed with something like a=96+x%43.
*/
uint8_t Rnd(void) {
static uint8_t s_random=15,a_random=96;

        s_random^=s_random<<3;
        s_random^=s_random>>5;
        s_random^=a_random++>>2;
        return s_random;
}

/*
https://tudl1086.home.xs4all.nl/synthcirc/noisesoftw/index-noisesoftw.htm
https://www.dsprelated.com/showarticle/908.php
https://www.firstpr.com.au/dsp/pink-noise/
http://www.ridgerat-tech.us/pink/pinkalg.htm
http://www.ridgerat-tech.us/pink/newpink.htm
http://www.ridgerat-tech.us/pink/hwpink.htm
*/

// based on http://www.ridgerat-tech.us/pink/hwpink.htm , adjusted to 8bit
uint8_t Rnd_pink(void) {
static uint8_t s_random=15,a_random=96;

const uint8_t pSum[] = { 1, 4,16, 60, 234 };
uint8_t mask = 0x10;      // Start processing from last generator = 0B00010000
    
static uint8_t  gsidx = 0;
const int8_t  gensum_fixed[] = { -127, -66, -79, -19, -85, -24, -38, 23, -78, -17, -30, 31, -36, 25, 12, 73,
                                  -73, -12, -25, 36, -31, 30, 17, 78, -23, 38, 24, 85, 19, 79, 66, 127 };

        s_random^=s_random<<3;
        s_random^=s_random>>5;
        s_random^=a_random++>>2;

    do  {
      // There is a 50-50 chance that updating a 2-valued stage won't
      // change anything. Take advantage of this special case first!
      if  ( s_random & 1 ) break;
      // There is a small probability that no generators change anyway.
      if  ( s_random > pSum[4] ) break;
      
      // If that didn't happen, find the right generator and toggle it.
      if(s_random > pSum[3]) {  gsidx ^= mask; break; } else{ mask >>= 1;}
      if(s_random > pSum[2]) {  gsidx ^= mask; break; } else{ mask >>= 1;}
      if(s_random > pSum[1]) {  gsidx ^= mask; break; } else{ mask >>= 1;}
      if(s_random > pSum[0]) {  gsidx ^= mask; break; } else{ mask >>= 1;}
      gsidx ^= mask;
    } while (0);  // Never loops back!
        
        return gensum_fixed[gsidx];
}

// delivers -127 or 127
// n_color : 0..255 ("bright" to "dark" noise)
int8_t Rnd_color(uint8_t n_color) {
static uint8_t s_random=15,a_random=96;
static uint8_t noise = 0;
static uint8_t scount = 8;

        s_random^=s_random<<3;
        s_random^=s_random>>5;
        s_random^=a_random++>>2;

        if(scount){
          scount--;
        }
        else{
          // flipping bit 1
          noise = noise ^ 0b00000001;
          // set new counter value to random number limited to be < n_color
          scount =  (s_random & n_color);
        }
       
        if(noise){
          return (50);
        }
        else{
          return(-50);
        }
}


// some fast filters as shift filters
// based on https://www.edn.com/a-simple-software-lowpass-filter-suits-embedded-system-applications/
// filter_shift 1..7 sets frequency
int8_t Lowpass(int8_t filter_input, uint8_t filter_shift) {
static int16_t filter_reg = 0; // Delay element – 16 bits
  
  // Update filter with current sample.
  filter_reg = filter_reg - (filter_reg >> filter_shift) + filter_input ;  // sum = sum - filter_out + filter_in;
  
  // Scale output for unity gain.
  return (int8_t)(filter_reg >> filter_shift);
}

// filter_shift 1..7 sets frequency
int8_t Highpass(int8_t filter_input, uint8_t filter_shift) {
static int16_t filter_reg = 0; // Delay element – 16 bits

  // Update filter with current sample.
  filter_reg = filter_reg - (filter_reg >> filter_shift) + filter_input;
  
  // Scale output for unity gain. (Highpass = input-Lowpass)
  return (int8_t)(filter_input - (int8_t)(filter_reg >> filter_shift));
}

// filter_shift1 0..8 increasing cornerfrequency
// filter_shift2 0..8 increasing feedback
int16_t LowpassReso(int8_t filter_input, uint8_t filter_shift1, uint8_t filter_shift2) {
static int16_t filter1 = 0; // Delay element – 16 bits
static int16_t filter2 = 0; // Delay element – 16 bits
static int16_t filterout = 0; // output

  // Update filter with current sample.
  filter1 = filter1 - filterout + filter_input ;
  filter2 = filter2 + ((filter1-filter2) >> filter_shift2);
  
  // Scale output
  filterout = (filter2 >> filter_shift1);
  
  return (filterout);
}

int16_t HighpassReso(int8_t filter_input, uint8_t filter_shift1, uint8_t filter_shift2) {
static int16_t filter1 = 0; // Delay element – 16 bits
static int16_t filter2 = 0; // Delay element – 16 bits
static int16_t filterout = 0; // output

  // Update filter with current sample.
  filter1 = filter1 - filterout + filter_input ;
  filter2 = filter2 + ((filter1-filter2) >> filter_shift2);

  // Scale output 
  filterout = (filter2 >> filter_shift1);
  
  // (Highpass = input-Lowpass)
  return (filter_input -filterout);
}

// https://tudl1086.home.xs4all.nl/synthcirc/filtsoftw/index-filtsoftw.htm
int8_t SVF(int8_t filter_input, uint8_t filter_f, uint8_t filter_q) {
  LP = LP + (BP >> filter_f);
  HP = (int16_t)((int8_t)filter_input) - LP - (BP >> filter_q);
  BP = BP + (HP >> filter_f);
  
  return ((int8_t)(int16_t)LP);
}

void updateControl(){
  uint16_t analog_input;

  uint8_t digital_input;
  static uint8_t switch1_mode = 0;
  static uint8_t switch2_mode = 0;
  static uint8_t switch3_mode = 0;
  static uint8_t wave_table = 0;


    // read digital port of the 1st switch (noise algorithm) switch1_state : 0 = rnd / 1 = rnd_1bit / 2 = pink / 3 = colored / 4 = wav
    digital_input = digitalReadFast(digital_input_port[0]);

    if(switch1_mode != digital_input){
       switch1_mode = digital_input;
       switch1_state++;
       if(switch1_state > 4){
         switch1_state = 0;
       }
       Serial.print("S1 "); Serial.println(switch1_state);
    }

    // read digital port of the 2nd switch (filter algorithm) switch2_state : 0 = no / 1 = LP / 2 = HP / 3 = LPreso / 4 = HPreso / 5 = SVF
    digital_input = digitalReadFast(digital_input_port[1]);

    if(switch2_mode != digital_input){
       switch2_mode = digital_input;
       switch2_state++;
       if(switch2_state > 5){
         switch2_state = 0;
       }
       Serial.print("  S2 "); Serial.println(switch2_state);
    }

    // read digital port of the 3rd switch (filter algorithm) switch3_state : 0 = no / 1 = envelope 
    digital_input = digitalReadFast(digital_input_port[2]);

    if(switch3_mode != digital_input){
       switch3_mode = digital_input;
       switch3_state++;
       if(switch3_state > 1){
         switch3_state = 0;
       }
       Serial.print("  S3 "); Serial.println(switch3_state);
    }
    
    // noise_color 0..255
    analog_input = (mozziAnalogRead(analog_input_port[0]) >> 2); 
      // settings have changed
      if( analog_input != noise_color){
        noise_color = analog_input;
        Serial.print("    ncolor "); Serial.println(noise_color);
      }

    // fs1 0..8
    analog_input = (mozziAnalogRead(analog_input_port[1]) >> 7); 
      // settings have changed
      if( analog_input != fs1){
        fs1 = analog_input;
        Serial.print("      fs1 "); Serial.println(fs1);
      }

    // fs2 0..8
    analog_input = (mozziAnalogRead(analog_input_port[2]) >> 7); 
      // settings have changed
      if( analog_input != fs2){
        fs2 = analog_input;
        Serial.print("        fs2 "); Serial.println(fs2);
      }

    // wave_table 0..5
    analog_input = (mozziAnalogRead(analog_input_port[3]) >> 7); 
     // set the wavetables
    if( analog_input != wave_table){
        wave_table = analog_input;
        Serial.print("        wav "); Serial.println(wave_table);

     
         switch (wave_table) {
           case 1:
             kickOscil.setTable(SQUARE_ANALOGUE512_DATA);
             break;
           case 2:
             kickOscil.setTable(SQUARE_NO_ALIAS512_DATA);
             break;
           case 3:
             kickOscil.setTable(SAW_ANALOGUE512_DATA);
             break;
           case 4:
             kickOscil.setTable(TRIANGLE_ANALOGUE512_DATA);
             break;
           case 5:
             kickOscil.setTable(TRIANGLE512_DATA);
             break;
           default:
             kickOscil.setTable(SIN512_DATA);
             break;
          }
    }
    if(switch3_state == 0){
         // periodic start of kick-envelope
     if(noteDelay.ready()){
        // start event
        noteDelay.start(750);

        // start the kick envelope
        kick_wave_envelope.noteOn();
     }
    }
  }


int updateAudio(){
static uint8_t audio_count = 8;  
int16_t new_audio;
int8_t noise;

  // update kick drum envelope
  kick_wave_envelope.update();

// switch1_state : 0 = rnd / 1 = rnd_1bit / 2 = pink / 3 = colored / 4 = wav
switch (switch1_state) {
  case 0:
    noise = Rnd()>>1;
    break;
  case 1:
    noise = Rnd(); 
    if(noise & 0b00000001) {noise = 50;} else{noise = -50;}
    break;
  case 2:
    noise = Rnd_pink()>>1;
    break;
  case 3:
    noise = Rnd_color(noise_color);
    break;
  default:
    noise = (kickOscil.next()>>1);

}

// switch2_state : 0 = no / 1 = LP / 2 = HP / 3 = LPreso / 4 = HPreso
switch (switch2_state) {
  case 1:
    noise = Lowpass(noise, fs1);
    break;
  case 2:
    noise = Highpass(noise, fs1);
    break;
  case 3:
    noise = LowpassReso(noise, fs1, fs2);
    break;
  case 4:
    noise = HighpassReso(noise, fs1, fs2);
    break;
  case 5:
    noise = SVF(noise, fs1, fs2);
    break;
  default:
    break;
}

// read digital port of the 3rd switch (filter algorithm) switch3_state : 0 = no / 1 = envelope 
switch (switch3_state) {
  case 1:
    // mixing all oscillators & envelopes
    new_audio = (int16_t)   ( ((int16_t)(int8_t)(noise))    << 6      ) ;                                              // scale upt to signed 14 bit +-8192 (hifimode)
 
      break;
  default:
   new_audio = (int16_t) (
    ((
       ( (int16_t)(uint8_t)kick_wave_envelope.next() * (int16_t)(int8_t)noise  ) 
                                                             // 255 * 128 = 32640 == > / 4
      ) >> 2 )                                                 // scale down to signed 14 bit +-8192 (hifimode)
     );
    break;
}


/*
 if(!audio_count)
 {

   Serial.println(new_audio);
   audio_count = 8;
 }
 audio_count--;
*/

    return new_audio;
}


void loop(){
  audioHook(); // required here
}
