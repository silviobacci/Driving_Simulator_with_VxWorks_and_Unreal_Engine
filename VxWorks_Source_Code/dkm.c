//------------------------------------------------------------------------------
// DOWNLOADABLE KERNEL MODULE:	Creates tasks and executes them periodically.
//------------------------------------------------------------------------------

#include <semLib.h>
#include <selectLib.h>
#include <stdio.h>
#include <string.h>
#include <sysLib.h>
#include <tickLib.h>

#include "car.h"
#include "keyboard.h"
#include "ptask.h"
#include "udp.h"

//------------------------------------------------------------------------------
// GLOBAL CONSTANTS
//------------------------------------------------------------------------------

#define NAME_LENGTH				15			// Maximum length of tasks' name			
#define TASKS_STACK_SIZE		5000		// Size of tasks' stack

#define MANUAL					0			// Manual mode
#define AUTOMATIC				1			// Automatic mode
#define NUM_MODES				2			// Numbers of modes

//------------------------------------------------------------------------------
// GLOBAL DATA STRUCTURES
//------------------------------------------------------------------------------

typedef char    string[NAME_LENGTH];		// String of char

typedef STATUS  (*taskFunction) (void);		// Pointers to tasks' functions

//------------------------------------------------------------------------------
// GLOBAL COSTANTS
//------------------------------------------------------------------------------

// Array of tasks' ID that identifies each task
static TASK_ID         task[TASK_NUMBER];		

// Array of pointers to tasks' functions
static taskFunction    function[TASK_NUMBER];

// Array of tasks' name
static string          tasksName[TASK_NUMBER];

// Semaphores to protect shared structures
static SEM_ID          semCar, semSensorData, semButtonPressed;	

// Structure that contains car's informations
static car				myCar = {0.0f, 0.0f, 0.0f, 0, 0, 0};		

// Structure that contains sensors' informations
static sensorData		sd = {0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

// Variable that contains the button pressed by the user
static key				buttonPressed = NOPE;

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SHOW AVAILABLE COMMANDS: prints on the terminal the available commands
//------------------------------------------------------------------------------

static void 	showAvailableCommands(void) {	
				printf(	"WELCOME IN THIS NEW ADVENTURE!\nIn this game you can "
						"drive a car in a virtual world, fun with it!");
				printf("\nThe following commands are available:\n");
				printf("UP ARROW 	-> increment car's speed\n");
				printf("DOWN ARROW 	-> decrement car's speed\n");
				printf("LEFT ARROW 	-> turn the car on the left\n");
				printf("RIGHT ARROW	-> turn the car on the right\n");
				printf("M/m 		-> change the mode (manual/automatic)\n");
				printf("O/o 		-> decrement the period of the tasks\n");
				printf("P/p 		-> increment the period of the tasks\n");
				printf("V/v 		-> change view (normal/over/on board)\n");
				printf("S/s 		-> show and hide sensor's lines\n");
				printf("R/r 		-> reset the game\n\n");
}

//------------------------------------------------------------------------------
// CHOOSE REACTION TO BUTTON PRESSED: 	According to the choice of the user we 
//										perform the operations needed
//------------------------------------------------------------------------------

static void 	chooseReactionToButtonPressed(key bPressed, car * tmpCar){
				switch (bPressed) {
					case RESET:
						resetCarInfo(tmpCar);
						semTake(semResPeriod, WAIT_FOREVER);
							resPeriod = TRUE;
						semGive(semResPeriod);
						semFlush(semResPeriod);
						break;
					case MODE:
						mode = (mode + 1) % NUM_MODES;
						break;
					case PERIOD_UP:
						semTake(semIncPeriod, WAIT_FOREVER);
							incPeriod = TRUE;
						semGive(semIncPeriod);
						semFlush(semIncPeriod);
						break;
					case PERIOD_DOWN:
						semTake(semDecPeriod, WAIT_FOREVER);
							decPeriod = TRUE;
						semGive(semDecPeriod);
						semFlush(semDecPeriod);
						break;
					case VIEW:
						changeView(tmpCar);
						break;
					case SENSORS:
						changeSensorVisibility(tmpCar);
						break;
				}
}

//------------------------------------------------------------------------------
// TASK RECEVIER: 	task that receives data from Unreal Engine. It receives
//					sensor's data useful to detect collision and obstacle's 
//					distance.
//------------------------------------------------------------------------------

static STATUS 	taskReceiver(void) {
sensorData		tmpSd;
sensorData		newSd;
int				byteReceived;
int 			i;    
				
				setArrivalTime();
				
				while (TRUE) {
					semTake(semSensorData, WAIT_FOREVER);
                        newSd = sd;
					semGive (semSensorData);
					semFlush(semSensorData);
					
					//----------------------------------------------------------
					// We execute always more than one receive operation to be
					// sure that we have the most current information.
					//----------------------------------------------------------
					for (i = 0; i < (period / TASK_NUMBER) + 1; i++) {
						byteReceived = UDPrecv(&tmpSd);
						
						// Only if we read correctly from the buffer we save it
						if(byteReceived == sizeof(sensorData) &&
                                                        byteReceived != ERROR)
                            newSd = tmpSd;
					}
					
					// To protect the shared structure we use a semaphore
					semTake(semSensorData, WAIT_FOREVER);
						//------------------------------------------------------
						// Note that newSd contains the most current information
						// that is also correct and complete
						//------------------------------------------------------
                        sd = newSd;
					semGive (semSensorData);
					semFlush(semSensorData);
				
					detectDeadlineMissed(receiver, tasksName[receiver]);
					
					waitForPeriod(receiver);
				}
}

//------------------------------------------------------------------------------
// TASK KEYBOARD: Task that manage the keyboard input
//------------------------------------------------------------------------------

static STATUS 	taskKeyboard(void) {
key				tmpButtonPressed;

				while (TRUE) {
					tmpButtonPressed = detectButtonPressed();
					
					// To protect the shared structure we use a semaphore
					semTake(semButtonPressed, WAIT_FOREVER);
						buttonPressed = tmpButtonPressed;
					semGive(semButtonPressed);
					semFlush(semButtonPressed);
					
					detectDeadlineMissed(keyboard, tasksName[keyboard]);
					
					waitForPeriod(keyboard);
				}
}

//------------------------------------------------------------------------------
// TASK CAR:	Task that implement car's behavior logic
//------------------------------------------------------------------------------

static STATUS 	taskCar(void) {
int     		interval;	// Time's diff used to move the car how much we need
int     		previousTickNumber;	

//------------------------------------------------------------------------------
// Support variables to save shared global variables. This is useful to obtain 
// short critical section in which we only save the shared global variable.
// In this way we can use the local variable to perform all the operations.
//------------------------------------------------------------------------------
car				tmpCar;
sensorData		tmpSd;
key				tmpButtonPressed;
    
				// Set a predefined destination already known in Unreal Engine
				destinationPoint.X = 100.0f;
				destinationPoint.Y = 0.0f;
				
				previousTickNumber = tickGet();	
				
				while (TRUE) {
					// To protect the shared structures we use a semaphores
					semTake(semSensorData, WAIT_FOREVER);
						tmpSd = sd;
					semGive(semSensorData);
					semFlush(semSensorData);
					
					// To protect the shared structures we use a semaphores
					semTake(semButtonPressed, WAIT_FOREVER);
						tmpButtonPressed = buttonPressed;
						buttonPressed = NOPE;
					semGive(semButtonPressed);
					semFlush(semButtonPressed);
					
					// To protect the shared structures we use a semaphores
					semTake(semCar, WAIT_FOREVER);
						tmpCar = myCar;
					semGive(semCar);
					semFlush(semCar);

					// Get interval to move the car how much we need
					interval = getInterval(&previousTickNumber);
			  
					chooseReactionToButtonPressed(tmpButtonPressed, &tmpCar);
				   
					if (mode == MANUAL)
						manualMode(&tmpCar, &tmpSd, tmpButtonPressed, interval);
					else
						automaticMode(&tmpCar, &tmpSd, interval);
					
					// Finally we save in the global variable the final result
					semTake(semCar, WAIT_FOREVER);
						myCar = tmpCar;
					semGive(semCar);
					semFlush(semCar);
								
					detectDeadlineMissed(engine, tasksName[engine]);
					
					waitForPeriod(engine);
				}
}

//------------------------------------------------------------------------------
// TASK SENDER:	Task that sends data to Unreal Engine. It sends the	current
//				informations about position, speed and view of the car.
//------------------------------------------------------------------------------

static STATUS 	taskSender(void) {
car				tmpCar;

				while (TRUE) {
					// To protect the shared structure we use a semaphore
					semTake(semCar, WAIT_FOREVER);
						tmpCar = myCar;
					semGive(semCar);
					semFlush(semCar);
					
					UDPsend(&tmpCar);
					
					detectDeadlineMissed(sender, tasksName[sender]);
					
					waitForPeriod(sender);
				}
}

//------------------------------------------------------------------------------
// SET TASK NAME: Inserts into the specified array the name of the tasks
//------------------------------------------------------------------------------

static void 	setTaskName(void) {
				strcpy(tasksName[receiver], "taskReceiver");
				strcpy(tasksName[keyboard], "taskKeyboard");
				strcpy(tasksName[engine], "taskCar");
				strcpy(tasksName[sender], "taskSender");
}

//------------------------------------------------------------------------------
// SET TASK NAME: Inserts into the specified array the functions of the tasks
//------------------------------------------------------------------------------

static void 	setTaskFunction(void) {
				function[receiver] =    taskReceiver;
				function[keyboard] =    taskKeyboard;
				function[engine] =      taskCar;
				function[sender] =      taskSender;
}

//------------------------------------------------------------------------------
// START: Entry point function that creates all the tasks and starts them
//------------------------------------------------------------------------------

void 	start(void) {	
int     		i;
	
		// Create semaphores that uses inversion priority protocol
		semCar = 			semMCreate(SEM_INVERSION_SAFE);
		semSensorData = 	semMCreate(SEM_INVERSION_SAFE);
		semButtonPressed = 	semMCreate(SEM_INVERSION_SAFE);
		semPeriod = 		semMCreate(SEM_INVERSION_SAFE);
		semIncPeriod = 		semMCreate(SEM_INVERSION_SAFE);
		semDecPeriod = 		semMCreate(SEM_INVERSION_SAFE);
		semResPeriod = 		semMCreate(SEM_INVERSION_SAFE);
		semArrTime =		semMCreate(SEM_INVERSION_SAFE);
			
		// Create UDP connection between board and Unreal Engine
		if (UDPinit() < 0)
			printf("NETWORK ERROR");
		
		// Initialization of all task's parameters
		for (i = 0; i < TASK_NUMBER; i++)
			initParam(i, priority[i]);
		
		sysClkRateSet(300);			// Set the clock's rate to 300 Hz	
		
		showAvailableCommands();
		
		setPriority();
		
		setTaskFunction();
		
		setTaskName();
		
		// Create and execute tasks
		for (i = 0; i < TASK_NUMBER; i++)
			task[i] = taskCreate(tasksName[i], priority[i], 0, 
										TASKS_STACK_SIZE, function[i], 
										0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		
		for (i = 0; i < TASK_NUMBER; i++)
			taskActivate(task[i]);  
}
