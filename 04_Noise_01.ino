/*  Example playing noise,
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

void updateControl(){}


int updateAudio(){
static uint8_t audio_count = 8;  
int16_t new_audio;
int8_t noise;

  noise = Rnd();
  new_audio = ((int16_t)( (int8_t)noise ) << 6); // scale up to 14 bit +-8192 (hifimode)

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
