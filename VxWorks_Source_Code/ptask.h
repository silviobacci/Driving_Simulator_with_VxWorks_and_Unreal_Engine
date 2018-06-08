#ifndef PTASK_H
#define PTASK_H

//------------------------------------------------------------------------------
// GLOBAL CONSTANTS
//------------------------------------------------------------------------------

#define 	TASK_NUMBER				4		// Number of running tasks

//------------------------------------------------------------------------------
// GLOBAL AND EXTERN DATA STRUCTURES
//------------------------------------------------------------------------------

typedef struct taskParam {	
	int priority;			// Task's priority
	int dmiss;				// Number of deadline miss by the task
} taskParam;

// Running tasks
enum tasks {
	receiver, 
	keyboard, 
	engine, 
	sender
};

//------------------------------------------------------------------------------
// GLOBAL AND EXTERN VARIABLES
//------------------------------------------------------------------------------

extern int		priority[TASK_NUMBER];
extern int 		period;
extern int		decPeriod;
extern int 		incPeriod;
extern int 		resPeriod;
extern SEM_ID	semPeriod, semDecPeriod, semIncPeriod, semResPeriod, semArrTime;

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// INIT PARAM:	Initializes the structure that contains tasks' parameters
//------------------------------------------------------------------------------

void initParam(int task, int pr);

//------------------------------------------------------------------------------
// SET PRIORITY:	Initializes tasks' priorities
//------------------------------------------------------------------------------

void setPriority(void);

//------------------------------------------------------------------------------
// SET PERIOD:		Saves the arrival tick number of the task
//------------------------------------------------------------------------------

void setArrivalTime(void);

//------------------------------------------------------------------------------
// WAIT FOR PERIOD:		The task waits the next period to continue
//------------------------------------------------------------------------------

void waitForPeriod(int index);

//------------------------------------------------------------------------------
// DETETCT DEADLINE MISSED:		Checks if a deadline was missed
//------------------------------------------------------------------------------

void detectDeadlineMissed(int index, char * taskName);

//------------------------------------------------------------------------------
// GET INTERVAL: 	Returns the difference between a specified tick's number
//					and the current tick's number
//------------------------------------------------------------------------------

int getInterval(int * previousTickNumber);

#endif
