// Filename     : Snake
// Version Date : 2019-08-04
// Programmer   : Gabriel Stewart
// Description  : This file contains the source code for the snake game application. It utilizes a 
//                8-bit LED matrix display to play a simple snake like game. A joystick
//                is used for the primpary input. The wiring for the display is identical 
//                to the Arduino Tutorial "LED Matrix".


// Includes
#include <time.h>

// Definitions
#define kWinCount 40

// Global values
int latchPin = 12;          // Pin connected to ST_CP of 74HC595（Pin12）
int clockPin = 13;          // Pin connected to SH_CP of 74HC595（Pin11）
int dataPin = 11;           // Pin connected to DS of 74HC595（Pin14）
int LEDPin[] = { 2, 3, 4, 5, 6, 7, 8, 9 };    // column pin (cathode) of LED Matrix
int xAxisPin = 0;           // define X pin of Joystick
int yAxisPin = 1;           // define Y pin of Joystick
int xVal, yVal;       // define 3 variables to store the values of 3 direction
int segCount = 3;
int lastMove = 0;
bool full = false;

// The snakes segment array
uint8_t board[41][8] = {
  { 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 }
};

// The array of segments that will be displayed
uint8_t boardDisplay[] = {
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// The food segment array
uint8_t food[] = {
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00
};

// The array data for the 'X'
uint8_t X[] = {
  129, 66, 36, 24, 24, 36, 66, 129
};

// The array data for the smile
int smile[] = {
  60, 66, 169, 133, 133, 169, 66, 60
};


// Function   : setup
// Programmer : Gabriel Stewart
// Description: This is the setup function, it prepares the pins and baud rate
// Parameters : None
// Returns    : None
void setup() {
	// set pins to output
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	for (int i = 0; i < 8; i++) {
		pinMode(LEDPin[i], OUTPUT);
	}
	Serial.begin(9600);         // initialize the serial port with baud rate 9600
	Serial.println("UNO is ready!");    // print the string "UNO is ready!"
}


// Function   : loop
// Programmer : Gabriel Stewart
// Description: This is the main game loop. It moves the player and displays the snake as well
//              as the food
// Parameters : None
// Returns    : None
void loop() {

	// Define a one-byte variable (8 bits) which is used to represent the selected state of 8 column.
	int cols;
	int moveCount = 0x00;

	// Display pattern defined in matrices
	for (;;) {

		// Get input direction
		xVal = analogRead(xAxisPin);
		yVal = analogRead(yAxisPin);
		if (abs(xVal - 500) > abs(yVal - 500))
			yVal = 500;
		else
			xVal = 500;

		// Move player ever nth iteration
		if (moveCount > 50) {
			move();
			moveCount = 0x00;
		}

		// Assemble the dots to display by or-ing all the segments
		for (int seg = 0; seg < segCount; seg++) {
			for (int x = 0; x < 8; x++) {
				boardDisplay[x] |= board[seg][x];
			}
		}

		// Display data in board display
		cols = 0x01;
		for (int x = 0; x < 8; x++) {
			matrixColsVal(cols);
			matrixRowsVal(boardDisplay[x]);
			matrixRowsVal(0x00);
			cols <<= 1;
		}

		// Reset the boardDisplay after outputting to matrix display
		memset(boardDisplay, 0x00, 8);

		// Reset cols val so that food can be displayed for a frame
		cols = 0x01;

		// Display the food
		for (int y = 0; y <= 7; y++) {
			matrixColsVal(cols);
			matrixRowsVal(food[y]);
			matrixRowsVal(0x00);
			cols <<= 1;
		}

		// Increment move counter
		moveCount++;
	}
}


// Function   : move
// Programmer : Gabriel Stewart
// Description: This function reads joystick input and moves snake accordingly
// Parameters : None
// Returns    : None
void move() {

	// Loop through all segments
	for (int seg = segCount; seg >= 0; seg--) {

		// Perform movment only on first segment
		if (seg == 0) {

			// Up
			if ((yVal > 600) && (lastMove != 1)) {
				moveUp(seg);
			}
			// Down
			else if ((yVal < 400) && (lastMove != 0)) {
				moveDown(seg);
			}
			// Left
			else if ((xVal < 400) && (lastMove != 3)) {
				moveLeft(seg);
			}
			// Right
			else if ((xVal > 600) && (lastMove != 2)) {
				moveRight(seg);
			}
			// If no input direction was specified, move according to previous input
			else {
				switch (lastMove) {
				case 0:
					moveUp(seg);
					break;
				case 1:
					moveDown(seg);
					break;
				case 2:
					moveLeft(seg);
					break;
				case 3:
					moveRight(seg);
					break;
				}
			}

			// Check if we collided with food, the body, or if we won!
			checkFood();
		}
		// Set each other segment to be equal to the one before it
		else {
			for (int i = 0; i < 8; i++) {
				board[seg][i] = board[seg - 1][i];
			}
		}
	}
}


// Function   : checkFood
// Programmer : Gabriel Stewart
// Description: This function checks the current snake head position against the foods position
//              and should collision be detected, add a new segment and move food
// Parameters : None
// Returns    : None
void checkFood() {

	// Array of possible food positions
	int posArray[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

	// Check if head has collided with food or body of snake
	for (int x = 0; x <= 7; x++) {


		// If head has collided with food
		if ((board[0][x] == food[x]) && board[0][x] != 0x00) {
			memset(food, 0x00, 8);
			srand(time(NULL) + segCount);

			Serial.println(rand());

			food[rand() % 8] = posArray[rand() % 8];

			for (int x = 0; x <= 7; x++) {
				for (int seg = 0; seg < segCount; seg++) {
					if (board[seg][x] == food[x]) {

						memset(food, 0x00, 8);
						food[rand() % 8] = posArray[rand() % 8];
					}
				}
			}

			// Add a new segment
			segCount++;
		}

		// If snake has collided with itself
		for (int seg = 1; seg < segCount; seg++) {
			if ((board[0][x] == board[seg][x]) && (board[0][x] != 0x00))
				// Display losing 'X' and reset segment count
				lose();
		}

		// Ceck if we have enough segments to win
		if (segCount >= kWinCount)
			win();
	}
}


// Function   : lose
// Programmer : Gabriel Stewart
// Description: This function displays the 'X' indicating that the player has lost
// Parameters : None
// Returns    : None
void lose() {
	for (int x = 0; x < 1000; x++) {
		int cols = 0x01;
		for (int y = 0; y <= 7; y++) {
			matrixColsVal(cols);
			matrixRowsVal(X[y]);
			matrixRowsVal(0x00);
			cols <<= 1;
		}
	}
	segCount = 3;
}


// Function   : win
// Programmer : Gabriel Stewart
// Description: This function displays the smile indicating that the player has won
// Parameters : None
// Returns    : None
void win() {
	for (int x = 0; x < 1000; x++) {
		int cols = 0x01;
		for (int y = 0; y <= 7; y++) {
			matrixColsVal(cols);
			matrixRowsVal(smile[y]);
			matrixRowsVal(0x00);
			cols <<= 1;
		}
	}
	segCount = 3;
}


// Function   : matrixRowsVal
// Programmer : Arduino
// Description: This function/matrixColsVal output a binary value to the matrix display
// Parameters : byte value - Binary to be displayed
// Returns    : None
void matrixRowsVal(int value) {
	// make latchPin output low level
	digitalWrite(latchPin, LOW);
	// Send serial data to 74HC595
	shiftOut(dataPin, clockPin, LSBFIRST, value);
	// make latchPin output high level, then 74HC595 will update the data to parallel output
	digitalWrite(latchPin, HIGH);
}


// Function   : matrixColsVal
// Programmer : Arduino
// Description: This function/matrixRowsVal output a binary value to the matrix display
// Parameters : byte value - Binary to be displayed
// Returns    : None
void matrixColsVal(byte value) {
	byte cols = 0x01;
	// Output the column data to the corresponding port.
	for (int i = 0; i < 8; i++) {
		digitalWrite(LEDPin[i], ((value & cols) == cols) ? LOW : HIGH);
		cols <<= 1;
	}
}



// Function   : moveUp
// Programmer : Gabriel Stewart
// Description: This function shifts the board matrix to move a segment up 1 space
// Parameters : int seg - The current segment to move
// Returns    : None
void moveUp(int seg) {

	// Variable Declaration
	int cols;

	// Move segment up by performing a left shift
	lastMove = 0;
	for (int x = 0; x <= 7; x++) {
		if (board[seg][x] == 128)
			board[seg][x] = 1;
		else
			board[seg][x] <<= 1;
	}
}


// Function   : moveDown
// Programmer : Gabriel Stewart
// Description: This function shifts the board matrix to move a segment down 1 space
// Parameters : int seg - The current segment to move
// Returns    : None
void moveDown(int seg) {

	// Variable Declaration
	int cols;

	// Move segment down by performing a right shift
	lastMove = 1;
	for (int x = 0; x <= 7; x++) {
		if (board[seg][x] == 1)
			board[seg][x] = 128;
		else
			board[seg][x] >>= 1;
	}
}

// Function   : moveLeft
// Programmer : Gabriel Stewart
// Description: This function offsets the positions of the values in the board matrix to
//              make the segments move 1 space left
// Parameters : int seg - The current segment to move
// Returns    : None
void moveLeft(int seg) {

	// Variable Declaration
	int cols;

	// Offset all values in board matrix to move segments left
	lastMove = 2;
	int temp = board[seg][0];
	for (int x = 0; x <= 7; x++) {
		board[seg][x] = board[seg][x + 1];
	}
	board[seg][7] = temp;
}


// Function   : moveRight
// Programmer : Gabriel Stewart
// Description: This function offsets the positions of the values in the board matrix to
//              make the segments move 1 space right
// Parameters : int seg - The current segment to move
// Returns    : None
void moveRight(int seg) {

	// Variable Declaration
	int cols;

	// Offset all values in board matrix to move segments right
	lastMove = 3;
	int temp = board[seg][7];
	for (int x = 7; x >= 0; x--) {
		board[seg][x] = board[seg][x - 1];
	}
	board[seg][0] = temp;
}