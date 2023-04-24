#include <Wire.h>
#include <string.h>

const int ARRSIZE = 256;

const int ledPin = 6;
const int buzPin = 7;
const int button1Pin = 12;
const int button2Pin = 11;
const int button3Pin = 10;

// for communicating with lcd
const char gmStrt = 'X';
const char gmIncr = 'Y';

// for communicating with dsp
char board[8][8];
const char zz = 'L';
const char zo = 'M';
const char oz = 'N';
const char oo = 'O';

// for communicating with jys
char dir;
char preDir;
const char up = 'w';
const char lf = 'a';
const char dn = 's';
const char rt = 'd';
const char clk = 'q';

// serialvars
unsigned long time_start;
int refreshT = 2000;
unsigned long lstSnd;
const byte dspId = 0x08;
const byte lcdId = 0x0C;
const byte jysId = 0x0A;

// coord struct
struct coor {
  int x;
  int y;
};

// snakevars
coor snake[ARRSIZE];
int head;
int tail;
int snakeLen;

bool gameStart = false;
const int easy = 1200;
const int medium = 500;
const int hard = 250;
int turn = 0;
int gameDifficulty;

int firstCurrState = 0, firstPressed = 1; 
int secondCurrState = 0, secondPressed = 1;
int thirdCurrState = 0, thirdPressed = 1;

// miscGamevars
coor nextPos;
coor foodPos;
int paused;

static void sendBrd(byte id);

void setup() {
  // begin i2c as driver
  Wire.begin();
  Serial.begin(9600);

  // initialize global vars
  time_start = millis();
  lstSnd = millis();

  // initialize pins
  pinMode(ledPin, OUTPUT);
  pinMode(buzPin, OUTPUT);

  randomSeed(analogRead(3));

  // begin game
  gmInit();
}

void loop() {
  int t = millis();

  while (gameStart == false) {
    if(turn == 0){
    sendIt(lcdId, gmStrt);
    ++turn;
    }

    firstCurrState = digitalRead(button1Pin);
    secondCurrState = digitalRead(button2Pin);
    thirdCurrState = digitalRead(button3Pin);

    if(firstPressed == 0 && firstCurrState == 1){
      gameDifficulty = easy;
      gameStart = true;
      turn = 0;
      sendIt(lcdId, gmIncr);
    }
    else if (secondPressed == 0 && secondCurrState == 1){
      gameDifficulty = medium;
      gameStart = true;
      turn = 0;
      sendIt(lcdId, gmIncr);
    }
    else if (thirdPressed == 0 && thirdCurrState == 1){
      gameDifficulty = hard;
      gameStart = true;
      turn = 0;
      sendIt(lcdId, gmIncr);
    }

    secondPressed = secondCurrState;
    firstPressed = firstCurrState;
    thirdPressed = thirdCurrState;

  }

  if ((t - lstSnd) > gameDifficulty) {
    char buf;
    Wire.requestFrom((int)jysId, 1);
    if (Wire.available() > 0) {
      buf = Wire.read();
    }


    if (buf == clk) {
      // paused = !paused;
    } else {
      dir = buf;
    }


    if (!paused) {
      updateSnake();
    }

    drawBrd();
    decodeBrd(encodeBrd());
    dumpBrd();
    sendBrd(dspId);
    lstSnd = t;
  }
}

static void ping(byte id) {
  Wire.beginTransmission(id);
  Wire.write(gmStrt);
  Wire.endTransmission();
}

static void sendIt(byte id, char it) {
  Wire.beginTransmission(id);
  Wire.write(it);
  Wire.endTransmission();
}

static String encodeBrd() {
  String toReturn = "";
  int pos = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j += 2) {
      char pos1 = board[i][j];
      char pos2 = board[i][j + 1];
      if (pos1 == '0' && pos2 == '0') {
        toReturn += zz;
      } else if (pos1 == '0' && pos2 == '1') {
        toReturn += zo;
      } else if (pos1 == '1' && pos2 == '0') {
        toReturn += oz;
      } else if (pos1 == '1' && pos2 == '1') {
        toReturn += oo;
      }
    }
  }

  return toReturn;
}


static void sendBrd(byte id) {
  String brd = encodeBrd();
  Wire.beginTransmission(id);
  for (int i = 0; i < 32; i++) {
    Wire.write(brd.charAt(i));
  }
  Wire.endTransmission();
}

//helper to make checkNextPos cleaner.
static bool hasDied() {

  return (nextPos.x > 7 || nextPos.x < 0 || nextPos.y > 7 || nextPos.y < 0 || isInSnake(nextPos));
}


//checks whether snake ate food, collided with wall or itself, or it moved to a valid space.
static int checkNextPos() {
  if (hasDied()) { 
    return -1;
  }
  if (nextPos.x == foodPos.x && nextPos.y == foodPos.y) { return 1; }
  return 0;
}

void changeReverse() {

  if (dir == up && preDir == dn || dir == dn && preDir == up || dir == lf && preDir == rt || dir == rt && preDir == lf) {
    dir = preDir;
  }
}

void updateNextPos() {

  changeReverse();

  //Serial.println(dir);
  if (dir == up) {
    nextPos.x = snake[head].x - 1;
    nextPos.y = snake[head].y;
  } else if (dir == lf) {
    nextPos.x = snake[head].x;
    nextPos.y = snake[head].y - 1;
  } else if (dir == dn) {
    nextPos.x = snake[head].x + 1;
    nextPos.y = snake[head].y;
  } else if (dir == rt) {
    nextPos.x = snake[head].x;
    nextPos.y = snake[head].y + 1;
  } else {
    dir = preDir;
    updateNextPos();
  }

  preDir = dir;
}

// this moves the snake and checks if the move is valid and handles if it either dies, eats food or moves.
static void updateSnake() {

  updateNextPos();

  int result = checkNextPos();

  if (result == 0) {
    //snake moved.
    head++;

    if (head >= ARRSIZE) {
      head == 0;
    }

    snake[head].x = nextPos.x;
    snake[head].y = nextPos.y;

    tail++;

    if (tail >= ARRSIZE) {
      tail = 0;
    }
  } else if (result == 1) {
    //snake ate food
    //only head should move
    snakeLen++;

    head++;

    if (head >= ARRSIZE) {
      head = 0;
    }

    snake[head].x = nextPos.x;
    snake[head].y = nextPos.y;
    tone(buzPin, 880, 1000);
    sendIt(lcdId, gmIncr);

    if (snakeLen < ARRSIZE) {
      placeFood();
    } else {
      gmInit();
    }
  } else {
    gmInit();
  }
}

static void brdWipe() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      board[i][j] = '0';
    }
  }
}

static void drawBrd() {
  brdWipe();
  for (int i = tail; i != (head + 1); i++) {
    if (i >= ARRSIZE) { i = 0; }
    coor curr = snake[i];
    board[curr.x][curr.y] = '1';
  }
  board[foodPos.x][foodPos.y] = '1';
}

static void dumpBrd() {
  Serial.print("\n\n\n\n\n\n\n\n\n\n");
  Serial.println(dir);
  //snake head
  Serial.print(snake[head].x);
  Serial.println(snake[head].y);
  //snake tail
  Serial.print(snake[tail].x);
  Serial.println(snake[tail].y);
  //next pos
  Serial.print(nextPos.x);
  Serial.println(nextPos.y);
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      Serial.print(board[i][j]);
      Serial.print(" ");
    }
    Serial.println("");
  }
}

static int isInSnake(coor coord) {
  for (int i = tail; i != head + 1; i++) {
    if (i >= ARRSIZE) { i = 0; }
    if (coord.x == snake[i].x && coord.y == snake[i].y) { return 1; }
  }
  return 0;
}

static void gmInit() {
  paused = 0;

  snakeLen = 2;
  head = 1;
  snake[head].x = 3;
  snake[head].y = 4;
  tail = 0;
  snake[tail].x = 3;
  snake[tail].y = 3;

  nextPos.x = snake[head].x;
  nextPos.y = snake[head].y + 1;

  dir = rt;
  preDir = rt;

  placeFood();

  //sendIt(lcdId, gmStrt);
  sendIt(jysId, gmStrt);

  digitalWrite(ledPin, HIGH);
  gameStart = false;
  delay(1000);
  digitalWrite(ledPin, LOW);
}

// uses random to find suitable location for next food
static void placeFood() {
  foodPos.x = random(0, 8);
  foodPos.y = random(0, 8);

  while (isInSnake(foodPos)) {
    foodPos.x = random(0, 8);
    foodPos.y = random(0, 8);
  }
}

static void decodeBrd(String brdCode) {
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
