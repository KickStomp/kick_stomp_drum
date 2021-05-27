#include <MozziGuts.h>

#define CONTROL_RATE 128

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    startMozzi();

}




void updateControl(){
static  uint8_t input1 = 0;
  
         input1 = (mozziAnalogRead(A3) >> 2);
         Serial.println(input1);
}

int updateAudio(){
}

void loop(){
  audioHook(); // required here
}
