//------------------------------------------------------------------------------
// KEYBOARD: 	Detection of buttons pressed and decoding of the key 
//------------------------------------------------------------------------------

#include 	<assert.h> 
#include 	<stdio.h>
#include 	<tyLib.h>

#include 	"keyboard.h"

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// INT TO KEY:	Returns which button has been pressed and returns NOPE if the
//				button pressed is not expected
//------------------------------------------------------------------------------

static key 	charToKey(char c) {	
			switch (c) {
				case 'A':
					return UP;
					break;
				case 'B':
					return DOWN;
					break;
				case 'C':
					return RIGHT;
					break;
				case 'D':
					return LEFT;
					break;
				case 'm':
				case 'M':
					return MODE;
					break;
				case 'o':
				case 'O':
					return PERIOD_DOWN;
					break;
				case 'p':
				case 'P':
					return PERIOD_UP;
					break;
				case 'r':
				case 'R':
					return RESET;
					break;
				case 's':
				case 'S':
					return SENSORS;
					break;	
				case 'v':
				case 'V':
					return VIEW;
					break;	
				default:
					return NOPE;
			}
}

//------------------------------------------------------------------------------
// KEY PRESSED:	Checks if a button was pressed and returns the associated 
//				integer value
//------------------------------------------------------------------------------

static char keyPressed(void) {
int     	nread;			// Contains the number of pressed buttons
int     	rc;				// Contains the state after check of the IO buffer
char     	oneChar;		// Contains the char of the button pressed
	
			// We check IO buffer and save in nread the number of arrived char
			rc = ioctl(0, FIONREAD, (int) &nread);
			assert(rc != ERROR);
			// If at least one char is arrived we take the first from the buffer
			if (nread > 0) {
				for (; nread; nread--)
					oneChar = getchar();
				return oneChar;
			}
			
			// If no chars are in the buffer we return NULL
			return NULL;
}

//------------------------------------------------------------------------------
// DETECT BUTTON PRESSED:	Checks and returns which button has been pressed by
//							using the previous functions
//------------------------------------------------------------------------------

key 	detectButtonPressed(void) {
char	key;			// Contains the integer values of the button pressed
	
        // We change IO buffer's options to read char before pressing enter
        ioctl(0, FIOSETOPTIONS, OPT_TERMINAL & ~OPT_LINE & ~OPT_ECHO);
        key = keyPressed();
			
        if (key != NULL) {
            // Reset options of the terminal and return the pressed button
            ioctl(0, FIOSETOPTIONS, ioctl(0, FIOGETOPTIONS, 0));
            return charToKey(key);
        }
        
        // If no chars are in the buffer we return NOPE as pressed button
        return NOPE;
}
