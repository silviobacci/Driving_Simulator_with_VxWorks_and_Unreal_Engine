#ifndef KEYBOARD_H
#define KEYBOARD_H

//------------------------------------------------------------------------------
// GLOBAL AND EXTERN STRUCTURES
//------------------------------------------------------------------------------

typedef enum key {
	UP, 
	DOWN, 
	LEFT, 
	RIGHT, 
	MODE, 
	RESET, 
	VIEW, 
	PERIOD_UP, 
	PERIOD_DOWN, 
	SENSORS, 
	NOPE
} key;

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DETECT BUTTON PRESSED:	Checks and returns which button has been pressed by
//							using the previous functions
//------------------------------------------------------------------------------

key detectButtonPressed(void);

#endif
