#include "LedControl.h" // LedControl library is used for controlling a LED matrix. Find it using Library Manager or download zip here: https://github.com/wayoda/LedControl
#include <Wire.h>
#include <String.h>

// 1)light up the led when the info is 1
// 2)at least three consecutive lights to represent the length of the snake
// 3) increase the light from the tail as the snake eats
// more pellets
// 4)somehow show a separate light to represent food pellet


unsigned long t;



char i2c_rcv;

char board[8][8];

const int buttonPin = 6;  // the number of the pushbutton pin 2
int buttonState = 0;  // variable for reading the pushbutton status
int oldButtonState = 0;

bool button = false;



// there are defined all the pins
struct Pin {

  static const short CLK = 10;   // clock for LED matrix
  static const short CS  = 11;  // chip-select for LED matrix
  static const short DIN = 12; // data-in for LED matrix
};


const short intensity = 6;
int potPin = A0; // potentiometer for snake speed control

void setup()
{

  Serial.begin(9600);
  Wire.begin(0x08); // your address in this communication
  Wire.onReceive(dataRcv);
  initialize();         // initialize pins & LED matrix
  wipeBrd(); // initializes all the values in the board to zero
  t = millis();

}


void loop()
{ 

  int ct = millis();

  if ((ct - t) > 200) {

    drawBoard();
    t = ct;
  }

  
}

void dataRcv(int numBytes) {
  String arr = "";
  while (Wire.available()) {
    arr += (char) (Wire.read());
  }
//  Serial.println(arr);
  decodeBoard(arr);
  printBoardtoSerial();


}


LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, 1);


// artificial logarithmity (steepness) of the potentiometer (-1 = linear, 1 = natural, bigger = steeper (recommended 0...1))
const float logarithmity = 0.4;


void initialize() {

  matrix.shutdown(0, false);
  matrix.setIntensity(0, intensity);
  matrix.clearDisplay(0);


}


static void decodeBoard(String brdCode) {

   int codePos = 0;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j += 2) {
      char curr = brdCode.charAt(codePos);
      if (curr == 'L') {
        board[i][j] = '0';
        board[i][j + 1] = '0'; 
      } else if (curr == 'M') {
        board[i][j] = '0';
        board[i][j + 1] = '1';
      } else if (curr == 'N') {
        board[i][j] = '1';
        board[i][j + 1] = '0';
      } else if (curr == 'O') {
        board[i][j] = '1';
        board[i][j + 1] = '1';
      }
      codePos++;
    }
  }

} 

// initializes the board to zero
void wipeBrd() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      board[i][j] = '0';
    }
  }
}


void drawBoard() {
  matrix.clearDisplay(0);
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
    buttonState = digitalRead(buttonPin);
      bool state;
             // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
 /*
   if (buttonState == HIGH && oldButtonState == LOW) {
           if (button == false) {
            Serial.println("Turn off");

             matrix.setLed(0, i, j, false);
             button = true;
           } else if (button == true) {
            Serial.println("Turn on");

              matrix.setLed(0, i, j, true);
             button = false;
           }
   }    
    oldButtonState = buttonState;

*/

      if (board[i][j] == '0') {
        state = false;
        matrix.setLed(0, i, j, false);

      } else {
   
        state = true;

  }

    // use this buttton to turn the led on the matrix board on and then off 
    // and then back on again
   if (buttonState == HIGH && oldButtonState == LOW) {
           if (button == false) {
            Serial.println("Turn off");
          //  matrix.setIntensity(0, 0);
            matrix.shutdown(0, true);
             button = true;
           } else if (button == true) {
            Serial.println("Turn on");

            //  matrix.setIntensity(0, 8);
              matrix.shutdown(0, false);
             button = false;
           }
   }    
    oldButtonState = buttonState;

      matrix.setLed(0, i, j, state);
    }
  }

}

void printBoardtoSerial() {
for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
     Serial.print(board[i][j]);
     Serial.print(" ");
    }
    Serial.println();
  }  

}
