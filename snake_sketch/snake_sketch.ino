#include "LedControl.h"

// TYPE DEFINITIONS
enum direction { LEFT, RIGHT, UP, DOWN };
struct Coordinate {
  int row;
  int col;
};

// CONSTANTS
const int joyX = A0;
const int joyY = A1;
const int DIN = 13;
const int CS = 12;
const int CLK = 11;

// TODO: Update with better sensitivity
const int JOYSTICK_MIN = 0;
const int JOYSTICK_MAX = 1023;

const uint64_t DIGITS[] = {
  0x0000fc8484fc0000,
  0x0002027e22120000,
  0x000044a4948c4400,
  0x0000dca4a48c0000,
  0x00007e1010700000,
  0x00004e5252527400,
  0x00005e5252527e00,
  0x00007e4040406000,
  0x00007e52527e0000,
  0x00007e5252720000,

};

// VARIABLE DEFINITION

// Define LED grid
LedControl lc = LedControl(DIN, CLK, CS, 1);

// use this to store where our snake is. When our snake touches a new tile, set 
// that to the snake's length. Decrement everything. When a entry reaches 0, it 
// should be turned off.
int grid[8][8] = {};

Coordinate food;

// Define snake
Coordinate head;
direction currDirection;

int speed; // lower is faster
int length;

boolean alive;

void setup() {
  Serial.begin(9600);
  initialize();
}

void loop() {
  while(alive) {
    readJoystick();
    move();
  }
  initialize();
}

void initialize() {
  memset(grid, 0, sizeof(grid));

  food = {3, 3};
  head = {0, 3};

  currDirection = RIGHT;

  speed = 500;
  length = 1;
  alive = true;

  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);

  // show starting position
  lc.setLed(0, head.row, head.col, 1);
  grid[head.row][head.col] = 1;
}

void readJoystick() {
  long timestamp = millis();
  
  // Read for long enough
  while(millis() < timestamp + speed && alive) {
    int xValue = analogRead(joyX);
    int yValue = analogRead(joyY);  

    // Ignore current direction and 180 degrees change
    if(currDirection != LEFT && currDirection != RIGHT 
        && xValue == JOYSTICK_MIN){
      currDirection = LEFT;
    } else if (currDirection != RIGHT  && currDirection != LEFT 
        && xValue == JOYSTICK_MAX) {
      currDirection = RIGHT;
    } else if (currDirection != UP && currDirection != DOWN 
        && yValue == JOYSTICK_MIN) {
      currDirection = UP;
    } else if (currDirection != DOWN && currDirection != UP 
        && yValue == JOYSTICK_MAX) {
      currDirection = DOWN;
    }
    lc.setLed(0, food.row, food.col, millis() % 1000 < 600 ? 1 : 0);
  }
}

void move() {
  switch (currDirection) {
		case LEFT:
			head.row--;
			lc.setLed(0, head.row, head.col, 1);
			break;

		case UP:
			head.col++;
			lc.setLed(0, head.row, head.col, 1);
			break;

		case RIGHT:
			head.row++;
			lc.setLed(0, head.row, head.col, 1);
			break;

		case DOWN:
			head.col--;
			lc.setLed(0, head.row, head.col, 1);
			break;

		default:
			return;
	}

  if(isDead()) {
    gameOver();
  }  

  detectAndEatFood();
  updateSnake();
  
  // advance head
  grid[head.row][head.col] = length;
}

void detectAndEatFood() {
  if(food.row == head.row && food.col == head.col) {
    length++;
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if(grid[i][j] > 0) {
          grid[i][j]++;
        }
      }
    }

    do {
			food.col = random(8);
			food.row = random(8);
		} while (grid[food.row][food.col] > 0 && alive);

    speed = max(100, speed*0.9); // its really hard below 100
  }
}

void updateSnake() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if(grid[i][j] > 0) {
        grid[i][j]--;
        if(grid[i][j] == 0) {
          lc.setLed(0, i, j, 0);
        }
      }
    }
  }
}

void gameOver() {
  lc.clearDisplay(0);
  alive = false;

  if (length / 10 != 0) {
    displayImage(DIGITS[length / 10]);
    delay(500);
  }
  displayImage(DIGITS[length % 10]);
  delay(2000);
  lc.clearDisplay(0);
}

boolean isDead() {
  return grid[head.row][head.col] > 0 || head.row > 7 || head.row < 0 
    || head.col > 7 || head.col < 0;
}

void displayImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}