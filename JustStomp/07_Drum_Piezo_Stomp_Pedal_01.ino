
/*  Example applying an ADSR envelope to an audio signal
    with Mozzi sonification library.  This shows
    how to use an ADSR which updates at AUDIO_RATE, in updateAudio(),
    and output using next() at AUDIO_RATE in updateAudio().

    Another example in this folder shows an ADSR updating at CONTROL_RATE,
    which is more efficient, but AUDIO_RATE updates shown in this example
    enable faster envelope transitions.

    Demonstrates a simple ADSR object being controlled with
    noteOn() and noteOff() instructions.

    Mozzi documentation/API
    https://sensorium.github.io/Mozzi/doc/html/index.html

    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users

    Tim Barrass 2013, CC by-nc-sa.
*/

#include <MozziGuts.h>
#include <Oscil.h>
#include <ADSR.h>

// used waveforms, length 512 should be sufficient
#include <tables/square_analogue512_int8.h>
#include <tables/sin512_int8.h>
#include <tables/square_no_alias512_int8.h>
#include <tables/saw_analogue512_int8.h>
#include <tables/triangle_analogue512_int8.h>
#include <tables/triangle512_int8.h>

// synth envelopes, outside for easy exchange
#include "synth_envelope.h"

#define CONTROL_RATE 128

// piezo trigger limit, adjust to your hardware
#define TRIGGER_LIMIT 50

// definition of used analog ports
// Instrument, Piezo
uint8_t analog_input_port[]  = {A7, A3};

// current port
uint8_t analog_port_index = 0;

ADSR <AUDIO_RATE, AUDIO_RATE> kick_wave_envelope;
ADSR <AUDIO_RATE, AUDIO_RATE> kick_noise_envelope;

// kick drum oscillator
Oscil <512, AUDIO_RATE> kickOscil;

// line for frequency shifts
Line <uint32_t> kickGliss;

// counter of audio steps
uint32_t kick_audio_steps = 0;
int32_t kick_audio_step_counter = 0;
uint32_t kick_gliss_start;

// current synth index, for changin to next setting
uint8_t synth_index = 0;

// index of synth-envelope with offset
uint8_t baseindex_synth, baseindex_noise;


void setup(){
  Serial.begin(115200);
  startMozzi();

  // selection synth envelope via synth_index
  baseindex_synth = (uint8_t)(synth_index * SYNTH_ENVELOPE_LENGTH); 
  baseindex_noise = (uint8_t)(synth_index * NOISE_ENVELOPE_LENGTH); 
  SetKick();

}


/*
https://tudl1086.home.xs4all.nl/synthcirc/noisesoftw/index-noisesoftw.htm
https://www.dsprelated.com/showarticle/908.php
https://www.firstpr.com.au/dsp/pink-noise/
http://www.ridgerat-tech.us/pink/pinkalg.htm
http://www.ridgerat-tech.us/pink/newpink.htm
http://www.ridgerat-tech.us/pink/hwpink.htm
*/

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

void SetKick(void){
  uint32_t gliss_end;
  uint8_t new_oscil;

   new_oscil = pgm_read_word_near(kick_synth + baseindex_synth);
   if(new_oscil > 5){
    new_oscil = 5;
   }

   // change wavetable
   if(new_oscil == 0){
     kickOscil.setTable(SIN512_DATA);
   }
   if(new_oscil == 1){
     kickOscil.setTable(SQUARE_ANALOGUE512_DATA);
   }
   if(new_oscil == 2){
     kickOscil.setTable(SQUARE_NO_ALIAS512_DATA);
   }
   if(new_oscil == 3){
     kickOscil.setTable(SAW_ANALOGUE512_DATA);
   }  
   if(new_oscil == 4){
     kickOscil.setTable(TRIANGLE512_DATA);
   }
   if(new_oscil == 5){
     kickOscil.setTable(TRIANGLE_ANALOGUE512_DATA);
   }
      

       kick_gliss_start = kickOscil.phaseIncFromFreq(pgm_read_word_near(kick_synth + baseindex_synth + 1));
       gliss_end   = kickOscil.phaseIncFromFreq(pgm_read_word_near(kick_synth + baseindex_synth + 2));

       kick_audio_steps = (AUDIO_RATE / 1000) *( pgm_read_word_near(kick_synth + baseindex_synth + 3) )   ; // number of steps at audiorate
       
       // set the audio rate line for frequency sweep
       kickGliss.set(kick_gliss_start, gliss_end, kick_audio_steps);   

       // set the envelopes
       kick_wave_envelope.setTimes(pgm_read_word_near(kick_synth + baseindex_synth + 4),
                                 pgm_read_word_near(kick_synth + baseindex_synth + 5),
                                 pgm_read_word_near(kick_synth + baseindex_synth + 6),
                                 pgm_read_word_near(kick_synth + baseindex_synth + 7));
   
       kick_wave_envelope.setLevels( (uint8_t)pgm_read_word_near(kick_synth + baseindex_synth + 8 ),
                                     (uint8_t)pgm_read_word_near(kick_synth + baseindex_synth + 9 ),
                                     (uint8_t)pgm_read_word_near(kick_synth + baseindex_synth + 10),
                                     (uint8_t)pgm_read_word_near(kick_synth + baseindex_synth + 11));
 
       kick_noise_envelope.setTimes(pgm_read_word_near(kick_noise_synth + baseindex_noise    ),
                                  pgm_read_word_near(kick_noise_synth + baseindex_noise + 1),
                                  pgm_read_word_near(kick_noise_synth + baseindex_noise + 2),
                                  pgm_read_word_near(kick_noise_synth + baseindex_noise + 3));

       kick_noise_envelope.setLevels( (uint8_t)pgm_read_word_near(kick_noise_synth + baseindex_noise + 4),
                                    (uint8_t)pgm_read_word_near(kick_noise_synth + baseindex_noise + 5),
                                    (uint8_t)pgm_read_word_near(kick_noise_synth + baseindex_noise + 6),
                                    (uint8_t)pgm_read_word_near(kick_noise_synth + baseindex_noise + 7));
}


void updateControl(){

  uint8_t piezo;
  static uint8_t piezo_status = 0;
  uint8_t analog_input;

  // read piezo sensor signal 0..255
  piezo = (uint8_t)(mozziAnalogRead(analog_input_port[1]) >> 2);

  // read analog port
  // values 0..15
  analog_input = (uint8_t)(mozziAnalogRead(analog_input_port[0]) >> 6); 


// analog ports:
// set Instrument

          // currently max. synth
          if(analog_input >= SYNTH_ENVELOPES){
            analog_input = SYNTH_ENVELOPES - 1;
          }

          if(analog_input != synth_index){
            synth_index = analog_input;

            baseindex_synth = (uint8_t)(synth_index * SYNTH_ENVELOPE_LENGTH); 
            baseindex_noise = (uint8_t)(synth_index * NOISE_ENVELOPE_LENGTH);

            SetKick();
          }
 
 
  // Foot Trigger Mode
  // Serial.println(piezo);

  // check if piezo signal is locked
  if(piezo_status){
     if(piezo < TRIGGER_LIMIT){
      // unlock piezo status when it is below trigger-level again
      piezo_status = 0;
     }
  }
  else{
    if(piezo > TRIGGER_LIMIT){
      // lock piezo status until it is below trigger-level again
      piezo_status = 1;

      // start kick synth
      kick_wave_envelope.noteOn();
      kick_noise_envelope.noteOn();
      kickGliss.set(kick_gliss_start);   
      kick_audio_step_counter = kick_audio_steps; // start glitch-line          
    }
  }

}


int updateAudio(){
static uint8_t audio_count = 8;  
int16_t new_audio;
int16_t noise1;
uint8_t noise;


  kick_wave_envelope.update();
  kick_noise_envelope.update();
    
  // do frequency sweep
  if( kick_audio_step_counter  ){
    kickOscil.setPhaseInc(kickGliss.next());
    kick_audio_step_counter--;
  } 

  // get some noise
  noise = Rnd();

  // use different bits to create new noise streams
  if(noise & 0x001) {noise1 = 127;} else{noise1 = -127;}

  // mix oscillators and envelopes
  new_audio = (int16_t) (
    ((
      (int32_t)(( (int16_t)(uint8_t)kick_wave_envelope.next() * (int16_t)(int8_t)kickOscil.next()  ) ) 
      + 
      (int16_t)(( (int16_t)(uint8_t)kick_noise_envelope.next() * noise1  ) )
      ) >> 2 )                                                 // scale down to signed 14 bit +-8192 (hifimode)
  );


  // poor man's compressor (hard clipping)
  if(new_audio >8191){
    new_audio = 8191;
  }
  else{
    if(new_audio < -8191){
      new_audio = -8191;
    }
  }

/* 
// for debugging
 if(!audio_count)
 {

   Serial.println(new_audio);
   audio_count = 16;
 }
 audio_count--;
*/


  return new_audio;
}


void loop(){
  audioHook(); // required here
}
