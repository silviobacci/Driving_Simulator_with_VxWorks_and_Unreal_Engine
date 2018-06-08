//------------------------------------------------------------------------------
// PTASK: 	Manages all the parameters of the tasks
//------------------------------------------------------------------------------

#include 	<math.h>
#include 	<semLib.h>
#include 	<stdio.h>
#include 	<taskLibCommon.h>
#include 	<tickLib.h>

#include 	"ptask.h"

//------------------------------------------------------------------------------
// GLOBAL CONSTANTS
//------------------------------------------------------------------------------

#define 	TASK_NUMBER				4		// Number of running tasks
#define 	DEFAULT_PERIOD			4		// Default period for all tasks
#define 	MINIMUM_PERIOD			4		// Minimum period for all tasks
#define 	MAXIMUM_PERIOD			20		// Maximum period for all tasks
#define 	STEP_PERIOD				4		// Period to add and subtract

//------------------------------------------------------------------------------
// GLOBAL DATA STRUCTURES
//------------------------------------------------------------------------------

static taskParam 	tp[TASK_NUMBER];		// Parameters of the tasks
static int 			arrivalTickNumber = 0;	// Arrival tick number of all tasks

int 		priority[TASK_NUMBER];			// Priorities of the tasks
int 		period = 	DEFAULT_PERIOD;		// Period of the tasks

int			decPeriod = FALSE;	// Say to the first task to decrement the period
int 		incPeriod = FALSE;	// Say to the first task to increment the period
int 		resPeriod = FALSE;	// Say to the first task to reset the period

// Semaphore to protect shared variables
SEM_ID 		semPeriod, semDecPeriod, semIncPeriod, semResPeriod, semArrTime;	

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DECREMENT PERIOD:	Decrements the period the shows it in the terminal
//------------------------------------------------------------------------------

static void decrementPeriod(void) {
			// To protect the shared variable period we need to use a semaphore
			semTake(semPeriod, WAIT_FOREVER);
				// If period is not greater than a value we don't decrement it
				if (period > MINIMUM_PERIOD)
					period -= STEP_PERIOD;
				printf("PERIOD = %d\n\n", period);
			semGive(semPeriod);
			semFlush(semPeriod);
}

//------------------------------------------------------------------------------
// INCREMENT PERIOD:	Increments the period the shows it in the terminal
//------------------------------------------------------------------------------

static void	incrementPeriod(void) {
			// To protect the shared variable period we need to use a semaphore
			semTake(semPeriod, WAIT_FOREVER);
				 // If the period is not less than a value we don't increment it
				if (period < MAXIMUM_PERIOD)
					period += STEP_PERIOD;
				printf("PERIOD = %d\n\n", period);
			semGive(semPeriod);	
			semFlush(semPeriod);
}

//------------------------------------------------------------------------------
// INIT PARAM:	Initializes the structure that contains tasks' parameters
//------------------------------------------------------------------------------

void 	initParam(int task, int pr) {
		tp[task].priority =	pr;
		tp[task].dmiss =	0;
}

//------------------------------------------------------------------------------
// SET PRIORITY:	Initializes tasks' priorities
//------------------------------------------------------------------------------

void 	setPriority(void){
		// Priorities are assigned in order to obtain a FIFO order
		priority[receiver] = 	101;
		priority[keyboard] = 	102;
		priority[engine] = 		103;
		priority[sender] = 		104;
}

//------------------------------------------------------------------------------
// RESET PERIOD:	Resets the period to the default value
//------------------------------------------------------------------------------

static void	resetPeriod(void){
			// To protect the shared variable period we need a semaphore
			semTake(semPeriod, WAIT_FOREVER);
				period = DEFAULT_PERIOD;
				printf("PERIOD = %d\n\n", period);
			semGive(semPeriod);
			semFlush(semPeriod);
}

//------------------------------------------------------------------------------
// CHECK PERIOD CHANGE:	The first task executes this function to check if
//						it has to modify the period. The global booleans
//						indicate if a change should be performed.
//------------------------------------------------------------------------------

static void	checkPeriodChange() {
// Support variables to save the value of the shared global booleans
int 		tmpDecPeriod;
int			tmpIncPeriod;
int			tmpResPeriod;

			// To protect the shared variable period we need to use a semaphore
			semTake(semDecPeriod, WAIT_FOREVER);
				tmpDecPeriod = decPeriod;
				decPeriod = FALSE;
			semGive(semDecPeriod);
			semFlush(semDecPeriod);
			
			// To protect the shared variable period we need to use a semaphore
			semTake(semIncPeriod, WAIT_FOREVER);
				tmpIncPeriod = incPeriod;
				incPeriod = FALSE;
			semGive(semIncPeriod);
			semFlush(semIncPeriod);
			
			// To protect the shared variable period we need to use a semaphore
			semTake(semResPeriod, WAIT_FOREVER);
				tmpResPeriod = resPeriod;
				resPeriod = FALSE;
			semGive(semResPeriod);
			semFlush(semResPeriod);
			
			if(tmpDecPeriod == TRUE)
				decrementPeriod();
				
			if(tmpIncPeriod == TRUE)
				incrementPeriod();
			
			if(tmpResPeriod == TRUE)
				resetPeriod();
}

//------------------------------------------------------------------------------
// SET PERIOD:		Saves the arrival tick number of the task
//------------------------------------------------------------------------------

void 	setArrivalTime(void){
		// To protect the shared variable period we need to use a semaphore
		semTake(semArrTime, WAIT_FOREVER);
			arrivalTickNumber = tickGet();
		semGive(semArrTime);
		semFlush(semArrTime);
}

//------------------------------------------------------------------------------
// WAIT FOR PERIOD:		The task waits the next period to continue
//------------------------------------------------------------------------------

void 	waitForPeriod(int index){
int     currentTickNumber;				// Current tick's number
int     p;								// Tasks' period support variable
int		arrivalTime;					// Arrival tick's number
	
        // To protect the shared variable period we need to use a semaphore
		semTake(semPeriod, WAIT_FOREVER);
			p = period;
		semGive(semPeriod);
		semFlush(semPeriod);
		
		semTake(semArrTime, WAIT_FOREVER);
			arrivalTime = arrivalTickNumber;
		semGive(semArrTime);
		semFlush(semArrTime);
		
		currentTickNumber = tickGet();
		
        // The current thread sleeps up to the next activation time
		taskDelay(p - (currentTickNumber - arrivalTime));
		
		//----------------------------------------------------------------------
        // When the first thread wakes up, it saves the current tick's number
		// and eventually, change the period according to user's decisions
		//----------------------------------------------------------------------
		if (index == receiver ){
			setArrivalTime();
			checkPeriodChange();
		}
}

//------------------------------------------------------------------------------
// DETETCT DEADLINE MISSED:		Checks if a deadline was missed
//------------------------------------------------------------------------------

void 	detectDeadlineMissed(int index, char * name){
int     currentTickNumber;				// Current tick's number
int     deadline;						// Deadline of the current task
int     p;								// Tasks' period support variable
int		arrivalTime;					// Arrival tick's number
		
        // To protect the shared variable period we need to use a semaphore
		semTake(semPeriod, WAIT_FOREVER);
			p = period;
		semGive(semPeriod);
		semFlush(semPeriod);
		
		semTake(semArrTime, WAIT_FOREVER);
			arrivalTime = arrivalTickNumber;
		semGive(semArrTime);
		semFlush(semArrTime);
		
		currentTickNumber = tickGet();
        // Deadlines are computed automatically from task's period and index
		deadline = ceil((p / TASK_NUMBER) * (index + 1));
		
        // If the deadline is missed we increment the counter and display it
		if ((currentTickNumber - arrivalTime) > deadline) {
			tp[index].dmiss++;
			printf("TASK %s: DEADLINE MISSED n. %d\n", name, tp[index].dmiss);
		}
}

//------------------------------------------------------------------------------
// GET INTERVAL: 	Returns the difference between a specified tick's number
//					and the current tick's number
//------------------------------------------------------------------------------

int 	getInterval(int * previousTickNumber){
int     currentTickNumber;			// Current tick's number
int     ret;						// Return value

		currentTickNumber = tickGet();
		ret = currentTickNumber - *previousTickNumber;
		*previousTickNumber = currentTickNumber;
		return ret;
}
