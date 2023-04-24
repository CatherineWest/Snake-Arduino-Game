#include <LiquidCrystal.h>
#include <Wire.h>
char receive;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int score = -1;
int buzzerPin = 10;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  Serial.begin(9600);

  Wire.begin(0x0C);

  Wire.onReceive(dataRcv);  // call dataRcv function when we receive something from the console arduino

  pinMode(buzzerPin, OUTPUT);
}

void loop() {
}

void dataRcv() {

  receive = Wire.read();
  Serial.println(receive);

  if (receive == 'X') {
    lcd.clear();
    score = -1;
    tone(buzzerPin, 1000, 1000);  // Set the buzzer to a frequency of 1000Hz
    lcd.setCursor(0, 0);
    lcd.print("Game Starting... ");
    lcd.setCursor(0, 1);
    lcd.print("Press a Button");
  }

  else if (receive == 'Y') {
    ++score;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Score = ");
    lcd.print(score);
  }
}
