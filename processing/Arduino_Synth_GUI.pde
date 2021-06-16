import controlP5.*;
import processing.serial.*;

Serial myPort;
ControlP5 cp5;

Slider2D sATT, sDEC, sSUS, sREL, snATT, snDEC, snSUS, snREL;

// Parameters
// kick_wave, kick_f1, kick_f2, kick_tSweep, kick_aTime, kick_dTime, kick_sTime, kick_rTime, kick_aLevel, kick_dLevel, kick_sLevel, kick_rLevel;
// kick_noise_aTime, kick_noise_dTime, kick_noise_sTime, kick_noise_rTime, kick_noise_aLevel, kick_noise_dLevel, kick_noise_sLevel, kick_noise_rLevel;

void setup() {
  size(800,600);
  
  frameRate(4);

  // the portnumber needs to be tested by trial & error..
  String portname = Serial.list()[1];
  myPort = new Serial(this, portname, 115200); 

  delay(300);
  
  cp5 = new ControlP5(this);
  
  // KickWave
  sATT = cp5.addSlider2D(" ATT")
         .setPosition(0,256)
         .setSize(100,100)
         .setMinMax(0,255,255,0)
         .setValue(40,255)
         .setColorBackground(color(222, 222, 222, 255))
         ;
 
  sDEC = cp5.addSlider2D(" DEC")
         .setPosition(100,256)
         .setSize(100,100)
         .setMinMax(0,255,255,0)
         .setValue(50,128)
         .setColorBackground(color(222, 222, 222, 255))
         ;
         
  sSUS = cp5.addSlider2D(" SUS")
         .setPosition(200,256)
         .setSize(100,100)
         .setMinMax(0,255,255,0)
         .setValue(50,64)
         .setColorBackground(color(222, 222, 222, 255))
        ;

  sREL = cp5.addSlider2D(" REL")
         .setPosition(300,256)
         .setSize(100,100)
         .setMinMax(0,255,255,0)
         .setColorBackground(color(222, 222, 222, 255))
         .setValue(50,0)
         ;

// KickNoise
  snATT = cp5.addSlider2D(" nATT")
         .setPosition(400,256)
         .setSize(100,100)
         .setMinMax(0,255,255,0)
         .setValue(40,128)
         .setColorBackground(color(0, 0, 64, 255))
         ;
 
  snDEC = cp5.addSlider2D(" nDEC")
         .setPosition(500,256)
         .setSize(100,100)
         .setMinMax(0,255,255,0)
         .setValue(50,64)
         .setColorBackground(color(0, 0, 64, 255))
         ;
         
  snSUS = cp5.addSlider2D(" nSUS")
         .setPosition(600,256)
         .setSize(100,100)
         .setMinMax(0,255,255,0)
         .setValue(50,32)
         .setColorBackground(color(0, 0, 64, 255))
         ;

  snREL = cp5.addSlider2D(" nREL")
         .setPosition(700,256)
         .setSize(100,100)
         .setMinMax(0,255,255,0)
         .setValue(50,0)
         .setColorBackground(color(0, 0, 64, 255))
        ;

    cp5.addSlider("kick_wave")
     .setPosition(100,420)
     .setWidth(400)
     .setRange(0,5) 
     .setValue(1)
     .setNumberOfTickMarks(6)
     .setSliderMode(Slider.FLEXIBLE)
     ;
     
    cp5.addSlider("kick_f1")
     .setPosition(100,440)
     .setWidth(400)
     .setRange(20,2000) 
     .setValue(600)
     .setSliderMode(Slider.FLEXIBLE)
     ;
  
    cp5.addSlider("kick_f2")
     .setPosition(100,460)
     .setWidth(400)
     .setRange(20,2000) 
     .setValue(50)
     .setSliderMode(Slider.FLEXIBLE)
     ;

    cp5.addSlider("kick_tSweep")
     .setPosition(100,480)
     .setWidth(400)
     .setRange(0,255) 
     .setValue(50)
     .setSliderMode(Slider.FLEXIBLE)
     ;   
     
 cp5.addButton("SendData")
   //Set the position of the button : (X,Y)
   .setPosition(250,500)
   //Set the size of the button : (X,Y)
   .setSize(100,50)
   //Set the pre-defined Value of the button : (int)
   .setValue(0)
   //set the way it is activated : RELEASE the mouseboutton or PRESS it
   .activateBy(ControlP5.PRESS);
   ;
   

                    
  smooth();
}


float kick_wave, kick_f1, kick_f2, kick_tSweep, kick_aTime, kick_dTime, kick_sTime, kick_rTime, kick_aLevel, kick_dLevel, kick_sLevel, kick_rLevel;
float kick_noise_aTime, kick_noise_dTime, kick_noise_sTime, kick_noise_rTime, kick_noise_aLevel, kick_noise_dLevel, kick_noise_sLevel, kick_noise_rLevel;

void draw() {
  background(0);

  noStroke();
  
  // Kick Window
  fill(50);
  rect(0, 0, 400,255);
  strokeWeight(1);
 // line(0,0,200, 0);
  stroke(255);
  
  
  kick_aTime = sATT.getArrayValue()[0];
  kick_aLevel = sATT.getArrayValue()[1];
  kick_dTime = sDEC.getArrayValue()[0];
  kick_dLevel = sDEC.getArrayValue()[1];
  kick_sTime = sSUS.getArrayValue()[0];
  kick_sLevel = sSUS.getArrayValue()[1];
  kick_rTime = sREL.getArrayValue()[0];
  kick_rLevel = sREL.getArrayValue()[1];
  
  line(0, yr(345), kick_aTime / 2.55, yr(kick_aLevel+345));
  line(kick_aTime / 2.55, yr(kick_aLevel+345), (kick_aTime + kick_dTime) / 2.55, yr(kick_dLevel+345));
  line((kick_aTime + kick_dTime) / 2.55, yr(kick_dLevel+345), (kick_aTime + kick_dTime + kick_sTime) / 2.55, yr(kick_sLevel+345));
  line((kick_aTime + kick_dTime + kick_sTime) / 2.55, yr(kick_sLevel+345), (kick_aTime + kick_dTime + kick_sTime + kick_rTime) / 2.55, yr(kick_rLevel+345));
 
  kick_noise_aTime = snATT.getArrayValue()[0];
  kick_noise_aLevel = snATT.getArrayValue()[1];
  kick_noise_dTime = snDEC.getArrayValue()[0];
  kick_noise_dLevel = snDEC.getArrayValue()[1];
  kick_noise_sTime = snSUS.getArrayValue()[0];
  kick_noise_sLevel = snSUS.getArrayValue()[1];
  kick_noise_rTime = snREL.getArrayValue()[0];
  kick_noise_rLevel = snREL.getArrayValue()[1];

  stroke(0,0,255);
  line(0, yr(345),  kick_noise_aTime / 2.55, yr(kick_noise_aLevel+345));
  line(kick_noise_aTime / 2.55, yr(kick_noise_aLevel+345), (kick_noise_aTime + kick_noise_dTime) / 2.55, yr(kick_noise_dLevel+345));
  line((kick_noise_aTime + kick_noise_dTime) / 2.55, yr(kick_noise_dLevel+345), (kick_noise_aTime + kick_noise_dTime + kick_noise_sTime) / 2.55, yr(kick_noise_sLevel+345));
  line((kick_noise_aTime + kick_noise_dTime + kick_noise_sTime) / 2.55, yr(kick_noise_sLevel+345), (kick_noise_aTime + kick_noise_dTime + kick_noise_sTime + kick_noise_rTime) / 2.55, yr(kick_noise_rLevel+345));

/*
  line(400, yr(345), 400 + kick_noise_aTime / 2.55, yr(kick_noise_aLevel+345));
  line(400 + kick_noise_aTime / 2.55, yr(kick_noise_aLevel+345), 400 + (kick_noise_aTime + kick_noise_dTime) / 2.55, yr(kick_noise_dLevel+345));
  line(400 + (kick_noise_aTime + kick_noise_dTime) / 2.55, yr(kick_noise_dLevel+345), 400 + (kick_noise_aTime + kick_noise_dTime + kick_noise_sTime) / 2.55, yr(kick_noise_sLevel+345));
  line(400 + (kick_noise_aTime + kick_noise_dTime + kick_noise_sTime) / 2.55, yr(kick_noise_sLevel+345), 400 + (kick_noise_aTime + kick_noise_dTime + kick_noise_sTime + kick_noise_rTime) / 2.55, yr(kick_noise_rLevel+345));
*/

// kick_wave, kick_f1, kick_f2, kick_tSweep, kick_aTime, kick_dTime, kick_sTime, kick_rTime, kick_aLevel, kick_dLevel, kick_sLevel, kick_rLevel;
// kick_noise_aTime, kick_noise_dTime, kick_noise_sTime, kick_noise_rTime, kick_noise_aLevel, kick_noise_dLevel, kick_noise_sLevel, kick_noise_rLevel;
   
   fill(255);
   if(kick_wave == 0){
     text("Sin", 600, 430); 
   }
   if(kick_wave == 1){
     text("Square-analog", 600, 430); 
   }
   if(kick_wave == 2){
     text("Square", 600, 430); 
   }
   if(kick_wave == 3){
     text("Saw", 600, 430); 
   }
   if(kick_wave == 4){
     text("Triangle", 600, 430); 
   } 
   if(kick_wave == 5){
     text("Triangle-analog", 600, 430); 
   }

  
}

public void SendData(int value){
// This is the place for the code, that is activated by the buttonb
     myPort.write("a"+ (int)kick_wave + "," + (int)kick_f1 +"," + (int)kick_f2 +"," + (int)kick_tSweep + 
     "\n" );
     
     myPort.write("b"+ (int)kick_aTime + "," + (int)kick_dTime + "," + (int)kick_sTime + "," + (int)kick_rTime +
     "\n" );            
                 
     myPort.write("c"+ (int)kick_aLevel + "," + (int)kick_dLevel + "," + (int)kick_sLevel + "," + (int)kick_rLevel +
     "\n" );            
                 
     myPort.write("d"+ (int)kick_noise_aTime + "," + (int)kick_noise_dTime + "," + (int)kick_noise_sTime + "," + (int)kick_noise_rTime +
     "\n" );            
                 
     myPort.write("e"+ (int)kick_noise_aLevel + "," + (int)kick_noise_dLevel + "," + (int)kick_noise_sLevel + "," + (int)kick_noise_rLevel +
     "\n" );
  
       println("kick_wave:");
       println((int)kick_wave + ", " + (int)kick_f1 +", " + (int)kick_f2 +", " + (int)kick_tSweep +", " + 
                 (int)kick_aTime + ", " + (int)kick_dTime + ", " + (int)kick_sTime + ", " + (int)kick_rTime +", " + 
                 (int)kick_aLevel + ", " + (int)kick_dLevel + ", " + (int)kick_sLevel + ", " + (int)kick_rLevel
                 ); 
                 
       println("kick_noise:");
       println((int)kick_noise_aTime + ", " + (int)kick_noise_dTime + ", " + (int)kick_noise_sTime + ", " + (int)kick_noise_rTime +", " + 
                 (int)kick_noise_aLevel + ", " + (int)kick_noise_dLevel + ", " + (int)kick_noise_sLevel + ", " + (int)kick_noise_rLevel
                 ); 
       println(" ");
}


float yr(float y_norm) {
  return (height - y_norm);  // transform y
}
