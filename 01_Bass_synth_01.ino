/*  Example for a bass kickdrum,
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


ADSR <AUDIO_RATE, AUDIO_RATE> kick_wave_envelope; // envelope for kickdrum

Oscil <512, AUDIO_RATE> kickOscil; // oscillator for kickdrum, fixed length of 512 for easy wavetable switching

// for triggering the envelope
EventDelay noteDelay;

void setup(){
  Serial.begin(115200); // for serial debugging, comment out if not needed
  noteDelay.set(2000); // 2 second countdown
  startMozzi();
  SetKick(10,100,255); // 10ms attack, 100ms decay, full level
}

/*
   time1 wave-Attack
   time2 wave total exponential decay time
   level : level of attack 0..255
*/
void SetKick(uint16_t time1, uint16_t time2, uint8_t level){
  uint16_t kickwave_time_A, kickwave_time_D, kickwave_time_S, kickwave_time_R;
  uint16_t kickwave_level_A, kickwave_level_D, kickwave_level_S, kickwave_level_R;

     // set the wavetable
     kickOscil.setTable(SIN512_DATA);

     // set the frequency
     kickOscil.setFreq(50); 

     // set wave envelope timings
     kickwave_time_A = time1;

     // exponential decay
     kickwave_time_D = time2 >> 3; // 1/8
     kickwave_time_S = time2 >> 2; // 1/4
     kickwave_time_R = kickwave_time_D + kickwave_time_S + kickwave_time_S;  // 5/8


     // set wave envelope levels
     kickwave_level_A = level;
     
     // exponential decay
     kickwave_level_D = level >> 1; // 1/2
     kickwave_level_S = level >> 3; // 1/8
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
 
}

/*
   automatic running at controlrate
   for changing parameters
*/
void updateControl(){
  static uint8_t bpm_speed = 80;
  uint16_t note_length_4t = 750;

    // only needed when changing BPM
    // note_length_4t = (uint16_t)(60000) / bpm_speed;  // ( 4* 60 * 1000 / 4 ) / bpm

       
     // periodic start of kick-envelope
     if(noteDelay.ready()){
        // start event
        noteDelay.start(note_length_4t);

        // start the kick envelope
        kick_wave_envelope.noteOn();
        
     }

  }

/*
   automatic running at audiorate
   for updating audio-stream
*/
int updateAudio(){
  int16_t new_audio;
  static uint8_t audio_count = 4;  

  // update kick drum envelope
  kick_wave_envelope.update();

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
     audio_count = 4;
   }
   audio_count--;
*/
  
  return new_audio;   
  
}

// do not touch
void loop(){
  audioHook(); // required here
}
