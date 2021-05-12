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

uint8_t noise_color = 0;

void setup(){
  Serial.begin(115200);
  startMozzi(); // uses the default control rate of 64, defined in mozzi_config.h

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
          return (127);
        }
        else{
          return(-127);
        }
}

void updateControl(){
  static uint16_t i=10;

  
    if(i){
      i--;
    }
    else{
      i = (mozziAnalogRead(A4)>>2 ); 
      // scroll through different noise_color values, rate depending on poti setting
      noise_color++;
      Serial.println(noise_color);
    }
    
  }


int updateAudio(){
static uint8_t audio_count = 8;  
int16_t new_audio;
int8_t noise;

  noise = Rnd_color(noise_color);
  
  new_audio = ((int16_t)((int8_t)noise) << 6);  // scale up to 14 bit +-8192 (hifimode)

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
