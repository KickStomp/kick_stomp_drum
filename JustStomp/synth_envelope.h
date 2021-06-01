// ##################################################################
// Synth Envelopes

// number synth envelopes
#define SYNTH_ENVELOPES 10

// length of synth envelope
#define SYNTH_ENVELOPE_LENGTH 12

const PROGMEM uint16_t kick_synth[]  = {
// wave, f1,  f2,    tSweep, aTime, dTime, sTime, rTime, aLevel, dLevel, sLevel, rLevel
//
   0, 106, 50, 51, 16, 48, 62, 94, 255, 124, 48, 0 ,
   0, 243, 50, 43, 43, 59, 40, 94, 255, 24, 8, 0,
   0, 63, 50, 43, 5, 59, 40, 94, 255, 24, 8, 0,
   1, 177, 50, 16, 5, 29, 37, 94, 255, 128, 64, 0,
   0, 177, 50, 16, 5, 29, 37, 94, 255, 32, 13, 0,
   4, 108, 58, 20, 5, 18, 18, 29, 255, 122, 46, 0,
   4, 108, 58, 20, 5, 46, 48, 62, 255, 103, 40, 0,
   0, 88, 58, 13, 5, 37, 27, 62, 255, 255, 18, 0,
   0, 345, 58, 51, 5, 84, 78, 130, 255, 255, 5, 0,
   0, 835, 58, 36, 21, 86, 86, 78, 255, 111, 32, 0
};   
 

// ##################################################################
// length of noise envelope
#define NOISE_ENVELOPE_LENGTH 8

const PROGMEM uint16_t kick_noise_synth[]  = {
// aTime, dTime, sTime, rTime, aLevel, dLevel, sLevel, rLevel
//
   13, 43, 40, 113, 78, 29, 10, 0,
   16, 54, 46, 113, 56, 18, 8, 0,
   13, 35, 35, 56, 54, 10, 2, 0,
   24, 35, 35, 56, 0, 10, 2, 0,
   24, 35, 35, 56, 0, 10, 2, 0,
   16, 24, 0, 2, 103, 0, 0, 0,
   10, 16, 24, 2, 255, 119, 0, 0,
   0, 13, 24, 2, 255, 255, 0, 0,
   13, 21, 24, 2, 29, 16, 0, 0,
   13, 21, 24, 2, 29, 16, 0, 0
};  