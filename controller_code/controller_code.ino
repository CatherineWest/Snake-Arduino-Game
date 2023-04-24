#include "SevSeg.h"
#include <Wire.h>

char i2c_rcv;

SevSeg sevseg; //Initiate a seven segment controller object
 
// this takes input from an analog controller and outputs it using 2 bits

const int X_pin = 0; // analog pin connected to X output
const int Y_pin = 1; // analog pin connected to Y output

const int button = 13;

int x_val;
int y_val;

int num;

char direction;

unsigned long joystickRead;

bool cur,pre,recvData,sendData;

void buttonPress();

void initWire(){
  Serial.begin(9600);
  Wire.begin(0x0a);
  Wire.onReceive(dataRcv);
  Wire.onRequest(dataSend);
}

void initDisplay(){
    byte numDigits = 4;  
    byte digitPins[] =  {3, 4, 2, 12};

      //{3,4,2,12};
     
    byte segmentPins[] = //{6, 7, 8, 9, 10, 11, 12, 13};

      //{10,12,8,6,5,9,11,7};

      {9,11,7,6,5,10,8};
   
    bool resistorsOnSegments = 0;
    // variable above indicates that 4 resistors were placed on the digit pins.
    // set variable to 1 if you want to use 8 resistors on the segment pins.
    sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
    sevseg.setBrightness(90);
    num = 0;
    int time = millis();

    pinMode(13,INPUT);

}

void initButton(){
  pinMode(button,INPUT);

  joystickRead = millis();

  pre = false;

}

void setup() {
  // put your setup code here, to run once:
  joystickRead = millis();

  initDisplay();

  initButton();

  initWire();

  sendData = false;

  recvData = false;
 
  Serial.begin(9600);
}

void readController(){
 
  //if input changes, change bit pattern to reflect direction
  // 00 = SOUTH | 01 = EAST | 10 = NORTH | 11 = WEST

  x_val = analogRead(X_pin);
  y_val = analogRead(Y_pin);
  //cur = digitalRead(button);

  if(x_val > 550){

    direction = 'd';
   
    Serial.println("south");

  }
  else if(x_val < 450){

    direction = 'a';
   
    Serial.println("north");
  }
  else if(y_val > 550){
    //west 11
    direction = 's';
    Serial.println("west");
  }
  else if(y_val < 450){
    //east 01
    direction = 'w';
    Serial.println("east");
  }
//  else if(!cur && pre){
//    direction = 'q';
//    num++;
//    Serial.println(direction);
//     pre = cur;
//  }
 

    else{
    //no need to update
    return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

 
   
  if(joystickRead+10 < millis()){
    readController();
    joystickRead = millis();
  }

  if(sendData){
    Wire.write(direction);
    sendData = false;
  }

  if(recvData){
    while(Wire.available()){
      Wire.read();
      num++;
    }
  }



  sevseg.setNumber(num,0);
  sevseg.refreshDisplay();

 

}

void dataSend(){
  Wire.write(direction);
  //Serial.println(direction);
  //delay(1000);
  //sendData = true;
}

void dataRcv() {
  /*while(Wire.available()){
    Wire.read();
    num++;
  }*/
  recvData = true;
}
