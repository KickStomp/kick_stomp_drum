#include <MozziGuts.h>
#include <Oscil.h>
#include <EventDelay.h>
#include <ADSR.h>

// used waveforms, length 512 should be sufficient
#include <tables/square_analogue512_int8.h>
#include <tables/sin512_int8.h>
#include <tables/square_no_alias512_int8.h>
#include <tables/saw_analogue512_int8.h>
#include <tables/triangle_analogue512_int8.h>
#include <tables/triangle512_int8.h>


#define CONTROL_RATE 32

// Drum Pattern
// 1 Pattern

#define DRUM_PATTERN 1

const PROGMEM uint16_t kick[]  = {
//   1e+e2e+e3e+e4e+e
   0b1000100010001000,      // Std 8 Bass
};

uint16_t kick_wave, kick_f1, kick_f2, kick_tSweep, kick_aTime, kick_dTime, kick_sTime, kick_rTime, kick_aLevel, kick_dLevel, kick_sLevel, kick_rLevel;
uint16_t kick_noise_aTime, kick_noise_dTime, kick_noise_sTime, kick_noise_rTime, kick_noise_aLevel, kick_noise_dLevel, kick_noise_sLevel, kick_noise_rLevel;

ADSR <AUDIO_RATE, AUDIO_RATE> kick_wave_envelope;
ADSR <AUDIO_RATE, AUDIO_RATE> kick_noise_envelope;

Oscil <512, AUDIO_RATE> kickOscil;

// line for frequency shifts
Line <uint32_t> kickGliss;

// counter of audio steps
uint32_t kick_audio_steps = 0;

int32_t kick_audio_step_counter = 0;

uint32_t kick_gliss_start;


// for triggering the envelope
EventDelay noteDelay;


void setup(){
  Serial.begin(115200);


  noteDelay.set(2000); // 2 second countdown
  startMozzi();
  
  kick_wave = 0;
  kick_f1 = 800;
  kick_f2 = 63;
  kick_tSweep = 30;
  
  kick_aTime = 10;
  kick_dTime = 40;
  kick_sTime = 80;
  kick_rTime = 160;
  
  kick_aLevel = 255;
  kick_dLevel = 200;
  kick_sLevel = 128;
  kick_rLevel = 0; 
  
  kick_noise_aTime = 20;
  kick_noise_dTime = 10;
  kick_noise_sTime = 20;
  kick_noise_rTime = 40;
  
  kick_noise_aLevel = 100;
  kick_noise_dLevel = 50;
  kick_noise_sLevel = 25;
  kick_noise_rLevel = 0;
  
  SetKick();
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


void SetKick(void){
  uint32_t gliss_end;


   if(kick_wave == 0){
     kickOscil.setTable(SIN512_DATA);
   }
   if(kick_wave == 1){
     kickOscil.setTable(SQUARE_ANALOGUE512_DATA);
   }
   if(kick_wave == 2){
     kickOscil.setTable(SQUARE_NO_ALIAS512_DATA);
   }
   if(kick_wave == 3){
     kickOscil.setTable(SAW_ANALOGUE512_DATA);
   }
   if(kick_wave == 4){
     kickOscil.setTable(TRIANGLE512_DATA);
   } 
   if(kick_wave == 5){
     kickOscil.setTable(TRIANGLE_ANALOGUE512_DATA);
   }
      

       kick_gliss_start = kickOscil.phaseIncFromFreq(kick_f1);
       gliss_end   = kickOscil.phaseIncFromFreq(kick_f2);

       kick_audio_steps = (AUDIO_RATE / 1000) *( kick_tSweep )   ; // number of steps at audiorate
       
       // set the audio rate line for frequency sweep
       kickGliss.set(kick_gliss_start, gliss_end, kick_audio_steps);   

       kick_wave_envelope.setTimes(kick_aTime, kick_dTime, kick_sTime, kick_rTime);
       kick_wave_envelope.setLevels(kick_aLevel, kick_dLevel, kick_sLevel, kick_rLevel);
 
       kick_noise_envelope.setTimes(kick_noise_aTime, kick_noise_dTime, kick_noise_sTime, kick_noise_rTime);
       kick_noise_envelope.setLevels(kick_noise_aLevel, kick_noise_dLevel, kick_noise_sLevel, kick_noise_rLevel);

      
}


void updateControl(){
  static uint8_t bar_count = 0;
  
  static uint16_t note_length_16t = 250;
  static uint16_t pattern_select = 0b1000000000000000;  // go through all 16 bits
  
  uint8_t modified = 0;
  uint16_t val1, val2, val3, val4, val5, val6, val7, val8, val9, val10, val11, val12, val13, val14, val15, val16, val17, val18, val19, val20;


// kick_wave, kick_f1, kick_f2, kick_tSweep, 
// kick_aTime, kick_dTime, kick_sTime, kick_rTime, 
// kick_aLevel, kick_dLevel, kick_sLevel, kick_rLevel;
// kick_noise_aTime, kick_noise_dTime, kick_noise_sTime, kick_noise_rTime, 
// kick_noise_aLevel, kick_noise_dLevel, kick_noise_sLevel, kick_noise_rLevel;

if (Serial.available()>0)
  {
    if(Serial.find('a'))  {
    val1=(uint16_t)(long)Serial.parseInt();
    val2=(uint16_t)(long)Serial.parseInt();
    val3=(uint16_t)(long)Serial.parseInt();
    val4=(uint16_t)(long)Serial.parseInt();
    }
    if(Serial.find('b'))  {
    val5=(uint16_t)(long)Serial.parseInt();
    val6=(uint16_t)(long)Serial.parseInt();
    val7=(uint16_t)(long)Serial.parseInt();
    val8=(uint16_t)(long)Serial.parseInt();
    }
    if(Serial.find('c'))  {
    val9=(uint16_t)(long)Serial.parseInt();
    val10=(uint16_t)(long)Serial.parseInt();
    val11=(uint16_t)(long)Serial.parseInt();
    val12=(uint16_t)(long)Serial.parseInt();
    }
    if(Serial.find('d'))  {
    val13=(uint16_t)(long)Serial.parseInt();
    val14=(uint16_t)(long)Serial.parseInt();
    val15=(uint16_t)(long)Serial.parseInt();
    val16=(uint16_t)(long)Serial.parseInt();
    }
    if(Serial.find('e'))  {
    val17=(uint16_t)(long)Serial.parseInt();
    val18=(uint16_t)(long)Serial.parseInt();
    val19=(uint16_t)(long)Serial.parseInt();
    val20=(uint16_t)(long)Serial.parseInt();
    }
  
  
}

  if(val1 != kick_wave){
    kick_wave = val1;
    modified = 1;
  }
  if(val2 != kick_f1){
    kick_f1 = val2;
    modified = 1;
  }
  if(val3 != kick_f2){
    kick_f2 = val3;
    modified = 1;
  }
  if(val4 != kick_tSweep){
    kick_tSweep = val4;
    modified = 1;
  }
  
// kick_aTime, kick_dTime, kick_sTime, kick_rTime, 


  if(val5 != kick_aTime){
    kick_aTime = val5;
    modified = 1;
  }
  if(val6 != kick_dTime){
    kick_dTime = val6;
    modified = 1;
  }
  if(val7 != kick_sTime){
    kick_sTime = val7;
    modified = 1;
  }
  if(val8 != kick_rTime){
    kick_rTime = val8;
    modified = 1;
  }

// kick_aLevel, kick_dLevel, kick_sLevel, kick_rLevel;

  if(val9 != kick_aLevel){
    kick_aLevel = val9;
    modified = 1;
  }
  if(val10 != kick_dLevel){
    kick_dLevel = val10;
    modified = 1;
  }
  if(val11 != kick_sLevel){
    kick_sLevel = val11;
    modified = 1;
  }
  if(val12 != kick_rLevel){
    kick_rLevel = val12;
    modified = 1;
  }

// kick_noise_aTime, kick_noise_dTime, kick_noise_sTime, kick_noise_rTime, 


  if(val13 != kick_noise_aTime){
    kick_noise_aTime = val13;
    modified = 1;
  }
  if(val14 != kick_noise_dTime){
    kick_noise_dTime = val14;
    modified = 1;
  }
  if(val15 != kick_noise_sTime){
    kick_noise_sTime = val15;
    modified = 1;
  }
  if(val16 != kick_noise_rTime){
    kick_noise_rTime = val16;
    modified = 1;
  }

// kick_noise_aLevel, kick_noise_dLevel, kick_noise_sLevel, kick_noise_rLevel;

  if(val17 != kick_noise_aLevel){
    kick_noise_aLevel = val17;
    modified = 1;
  }
  if(val18 != kick_noise_dLevel){
    kick_noise_dLevel = val18;
    modified = 1;
  }
  if(val19 != kick_noise_sLevel){
    kick_noise_sLevel = val19;
    modified = 1;
  }
  if(val20 != kick_noise_rLevel){
    kick_noise_rLevel = val20;
    modified = 1;
  }

  if(modified){
    SetKick();
   /* 
    Serial.print("Wave="); Serial.print(kick_wave);
    Serial.print(" f1="); Serial.print(kick_f1);
    Serial.print(" f2="); Serial.print(kick_f2);
    Serial.print(" tsweep="); Serial.print(kick_tSweep);

    Serial.print(" aT="); Serial.print(kick_aTime);
    Serial.print(" dT="); Serial.print(kick_dTime);
    Serial.print(" sT="); Serial.print(kick_sTime);
    Serial.print(" rT="); Serial.print(kick_rTime);

    Serial.print(" aL="); Serial.print(kick_aLevel);
    Serial.print(" dL="); Serial.print(kick_dLevel);
    Serial.print(" sL="); Serial.print(kick_sLevel);
    Serial.print(" rL="); Serial.print(kick_rLevel);

    Serial.print(" naT="); Serial.print(kick_noise_aTime);
    Serial.print(" ndT="); Serial.print(kick_noise_dTime);
    Serial.print(" nsT="); Serial.print(kick_noise_sTime);
    Serial.print(" nrT="); Serial.print(kick_noise_rTime);

    Serial.print(" naL="); Serial.print(kick_noise_aLevel);
    Serial.print(" ndL="); Serial.print(kick_noise_dLevel);
    Serial.print(" nsL="); Serial.print(kick_noise_sLevel);
    Serial.print(" nrL="); Serial.println(kick_noise_rLevel);
*/

    
  }

  if(noteDelay.ready()){
    // proceed to next 16th note
    noteDelay.start(note_length_16t);

         if( ( pgm_read_word_near(kick) & pattern_select ) ){
           kick_wave_envelope.noteOn();
           kick_noise_envelope.noteOn();
           kickGliss.set(kick_gliss_start);   
           kick_audio_step_counter = kick_audio_steps; // start glitch-line
         }

  
         pattern_select = pattern_select >> 1;

         if(!pattern_select){
           pattern_select = 0b1000000000000000;   // start again
           bar_count++;
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
