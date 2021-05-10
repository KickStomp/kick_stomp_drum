/*  Example for a bass kickdrum, adding potentiometers for parameter editing
    using Mozzi sonification library.

    Demonstrates the use of Oscil to play a wavetable.

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

// rate (repetitions / second) of function updateControl()
#define CONTROL_RATE 64

// digitalReadFast
// Standard Arduino Pins
#define digitalPinToPINReg(P) \
(((P) >= 0 && (P) <= 7) ? &PIND : (((P) >= 8 && (P) <= 13) ? &PINB : &PINC))
#define digitalPinToBit(P) \
(((P) >= 0 && (P) <= 7) ? (P) : (((P) >= 8 && (P) <= 13) ? (P) - 8 : (P) - 14))

#define digitalReadFast(P) bitRead(*digitalPinToPINReg(P), digitalPinToBit(P))
 
// definition of used analog ports, adjust according to your setup
uint8_t analog_input_port[]  = {A4, A5, A6, A7, A3};
// BPM, Pattern, Randomness / Trigger-Level, Instrument, Piezo

// digital-ports (switches)
uint8_t digital_input_port[]  = {4, 2, 3};
// Metronome, Shuffle, Fill

ADSR <AUDIO_RATE, AUDIO_RATE> kick_wave_envelope; // envelope for kickdrum

Oscil <512, AUDIO_RATE> kickOscil; // oscillator for kickdrum, fixed length of 512 for easy wavetable switching

// for triggering the envelope
EventDelay noteDelay;

// line for frequency shifts
Line <uint32_t> kickGliss;

// counter of audio steps, needed as global variable
uint32_t kick_audio_steps = 0;

uint32_t kick_audio_step_counter = 0;

// start value for frequency shift, needed as global variable
uint32_t kick_gliss_start;

void setup(){
  Serial.begin(115200); // for serial debugging, comment out if not needed
  noteDelay.set(2000); // 2 second countdown
  startMozzi();

  
  // setup digital ports with internal pullup
  pinMode(digital_input_port[0],INPUT_PULLUP);
  pinMode(digital_input_port[1],INPUT_PULLUP);
  pinMode(digital_input_port[2],INPUT_PULLUP);

  SetKick(10,100,255, 600, 50, 25); // 10ms attack, 100ms decay, full level, 600Hz, 50Hz, 25ms
}

/*
   time1 wave-Attack ms
   time2 wave total exponential decay time ms
   level : level of attack 0..255
   f1 : start frequency
   f2 : end frequency
   time_gliss : duration of frequency glide ms
*/
void SetKick(uint16_t time1, uint16_t time2, uint8_t level, uint16_t f1, uint16_t f2, uint16_t time_gliss){
  uint16_t kickwave_time_A, kickwave_time_D, kickwave_time_S, kickwave_time_R;
  uint16_t kickwave_level_A, kickwave_level_D, kickwave_level_S, kickwave_level_R;
  uint32_t gliss_end;

     // set the wavetable
     kickOscil.setTable(SIN512_DATA);

     // set wave envelope timings
     kickwave_time_A = time1;

     // exponential decay
     kickwave_time_D = time2 >> 3; // 1/8
     kickwave_time_S = time2 >> 2; // 1/4
     kickwave_time_R = kickwave_time_D + kickwave_time_S + kickwave_time_S;  // 5/8


     // set wave envelope levels
     kickwave_level_A = level;
     
     // exponential decay
     kickwave_level_D = level >> 1;
     kickwave_level_S = level >> 3;
     kickwave_level_R = 0;

    // set the envelopes : timing & levels
      
    kick_wave_envelope.setTimes(kickwave_time_A,
                                kickwave_time_D,
                                kickwave_time_S,
                                kickwave_time_R );
                                 
    kick_wave_envelope.setLevels(kickwave_level_A,
                                 kickwave_level_D,
                                 kickwave_level_S,
                                 kickwave_level_R );

     kick_gliss_start = kickOscil.phaseIncFromFreq(f1);
     gliss_end        = kickOscil.phaseIncFromFreq(f2);

     // number of steps at audiorate
     kick_audio_steps = (AUDIO_RATE / 1000) *(time_gliss)   ; 
     
     // set the audio rate line for frequency sweep
     kickGliss.set(kick_gliss_start, gliss_end, kick_audio_steps);   

}

// returning absolute difference between 2 values
uint16_t delta_value(uint16_t p1, uint16_t p2){
  if(p1>p2){
    return(p1-p2);
  }
  else{
    return(p2-p1);
  }
}

/*
   automatic running at controlrate
   for changing parameters
*/
void updateControl(){
  uint16_t analog_input;

  static uint16_t time1 = 10, time2 = 100, level = 255, f1 = 600, f2 = 50, time_gliss = 25;
  static uint8_t bpm_speed = 80;
  
  uint16_t note_length_4t = 750;
  
  uint8_t digital_input;
  static uint8_t switch1_mode = 0;

  // flag, will be 1 for parameter changes
  uint8_t para_change = 0;

    // read digital port of the 1st switch
    digital_input = digitalReadFast(digital_input_port[0]);

    if(switch1_mode != digital_input){
       switch1_mode = digital_input;
    }

    if(switch1_mode){
      
      // read analog ports, switch turned on
      // Attack time map to 0..64, (AnalogRead : values 0..1024, divide by 2^4=16)
      
      analog_input = (mozziAnalogRead(analog_input_port[1]) >> 4); 
      // settings have changed
      if( analog_input != time1){
        
        // only use new setting if close to old setting (otherwise abruppt parameter changes for using the switch) 
        if(delta_value(analog_input,time1) < 2){
          
          time1 = analog_input;
          para_change = 1;

          Serial.print("Attack = "); Serial.println(time1);
        }
      }
     
     // Decay time map to 0..512, (AnalogRead : values 0..1024, divide by 2^1=2)
      analog_input = (mozziAnalogRead(analog_input_port[2]) >> 1); 
      if( analog_input != time2){
        if(delta_value(analog_input,time2) < 8){
               
          time2 = analog_input;
          para_change = 1;

          Serial.print("Decay = "); Serial.println(time2);
        }
      }

     // Level map to 0..255, (AnalogRead : values 0..1024, divide by 2^2=4)
      analog_input = (mozziAnalogRead(analog_input_port[3]) >> 2); 
      if( analog_input != level){
        if(delta_value(analog_input,level) < 4){
          
          level = analog_input;
          para_change = 1;

          Serial.print("level = "); Serial.println(level);        
        }
      }

    }
    else{
      // read analog ports, switch turned off

      // Start frequency, (AnalogRead : values 0..1024)
      analog_input = (mozziAnalogRead(analog_input_port[1])); 
      if( analog_input != f1){
        if(delta_value(analog_input,f1) < 8){

          f1 = analog_input;
          para_change = 1;

          Serial.print("f1 = "); Serial.println(f1);
        }
      }
     
     // End frequency to 0..512, (AnalogRead : values 0..1024, divide by 2^1=2)
      analog_input = (mozziAnalogRead(analog_input_port[2]) >> 1); 
      if( analog_input != f2){
        if(delta_value(analog_input,f2) < 4){
        
          f2 = analog_input;
          para_change = 1;

          Serial.print("f2 = "); Serial.println(f2);
        }
      }

     // Gliss time map to 0..255, (AnalogRead : values 0..1024, divide by 2^2=4)
      analog_input = (mozziAnalogRead(analog_input_port[3]) >> 2); 
      if( analog_input != time_gliss){
        if(delta_value(analog_input,time_gliss) < 4){

          time_gliss = analog_input;
          para_change = 1;

          Serial.print("time_gliss = "); Serial.println(time_gliss);   
        }     
      }
      
    }

    // only update envelope & oscillator for parameter changes
    if(para_change){
      SetKick(time1, time2, uint8_t(level), f1, f2, time_gliss); 
    }

// (uint16_t time1, uint16_t time2, uint8_t level, uint16_t f1, uint16_t f2, uint16_t time_gliss)
// BPM, Pattern, Randomness / Trigger-Level, Instrument, Piezo
// Metronome, Shuffle, Fill


    // only needed when changing BPM
    // note_length_4t = (uint16_t)(60000) / bpm_speed;  // ( 4* 60 * 1000 / 4 ) / bpm

       
     // periodic start of kick-envelope
     if(noteDelay.ready()){
        // start event
        noteDelay.start(note_length_4t);

        // start the kick envelope
        kick_wave_envelope.noteOn();

        // start glitch-line   
        kickGliss.set(kick_gliss_start);   
        kick_audio_step_counter = kick_audio_steps;     
     }

  }

/*
   automatic running at audiorate
   for updating audio-stream
*/
int updateAudio(){
  int16_t new_audio;
  static uint8_t audio_count = 8;  

  // update kick drum envelope
  kick_wave_envelope.update();

  // change the frequency of the kick oscillator
  if( kick_audio_step_counter  ){
    kickOscil.setPhaseInc(kickGliss.next());
    kick_audio_step_counter--;
  } 

  // mixing all oscillators & envelopes
  new_audio = (int16_t) (
    ((
       (int32_t)( (int16_t)(uint8_t)kick_wave_envelope.next() * (int16_t)(int8_t)kickOscil.next()  ) 
                                                             // 255 * 128 = 32640 == > / 4
    ) >> 2 )                                                 // scale down to signed 14 bit +-8192 (hifimode)
  );

/*
   // for debugging
   if(!audio_count) // if = 0
   {
     Serial.println(new_audio);
     audio_count = 8;
   }
   audio_count--;
*/
  
  return new_audio;   
  
}

// do not touch
void loop(){
  audioHook(); // required here
}
