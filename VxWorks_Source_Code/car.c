//------------------------------------------------------------------------------
// CAR:	Contains car's behavior functions. We have function both for 
// 		manual and automatic mode.
//------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysLib.h> 

#include "car.h"
#include "keyboard.h"
#include "udp.h"

//------------------------------------------------------------------------------
// GLOBAL CONSTANTS
//------------------------------------------------------------------------------

#define MAX_SPEED_F			15		// Max speed in forward movement (m/s)
#define MAX_SPEED_B			-5		// Max speed in back movement (m/s)
#define STEP_SPEED			1		// Step speed to add and subtract (m/s)
#define AUTO_SPEED 			5		// Default speed in automatic mode (m/s)
#define AUTO_ESCAPE_SPEED	3		// Default speed while escaping (m/s)

#define YAW_STEP_F			1		// Step angle to add and subtract (deg)

#define SAFETY_DIST			10		// Safety distance to escape from collisions
#define FL_MARGIN			1.5		// Margin to arrive at finishing line

#define M_PI				3.14159265358979323846
#define M_PI_2				M_PI/2
#define TORAD				M_PI/180	// Converts degrees in radians
#define TODEG				180/M_PI	// Converts radians in degrees

#define MANUAL				0			// Manual mode
#define AUTOMATIC			1			// Automatic mode
#define NUM_MODES			2			// Numbers of modes

#define NUM_VIEW			3			// Number of views

#define NUM_SENSORS			5			// Number of sensors

//------------------------------------------------------------------------------
// GLOBAL DATA STRUCTURES
//------------------------------------------------------------------------------

static car				*myCar;			// Struct that contains car's info

static sensorData		*sd;			// Struct that contains sensors' info

static point			collisionPoint, trajectoryDestinationPoint;

// Boolean that is TRUE if a collision is detected in automatic mode
static int				automaticCollision = FALSE;				

// Boolean that is TRUE if a collision is detected in manual mode
static int				manualCollision = FALSE;				

// Integer > 0 if the collision has been detected in front, otherwise in back
static int				directionBeforeCollision = 0;			

// Array of sensors' distances
static int				distanceFromSensor[NUM_SENSORS];	

point					destinationPoint;

int 					mode = MANUAL;	// Current mode variable

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// YAW TO ANGLE: Convert yaw's integer value into an angle between 0 ° and 360 °
//------------------------------------------------------------------------------

static int	yawToAngle(int yaw) {
			if (yaw >= 0) {
				if (yaw < 90)
					return 90 - yaw;
				else
					return 450 - yaw;
			}
			else
				return 90 - yaw;
}

//------------------------------------------------------------------------------
// CHOOSE TURNING DIRECTION: 	Return turning direction according to the X
// 								Cartesian component of the destination, the 
//								current destination (according to car's 
//								direction) and the specified semiquadrant.
//------------------------------------------------------------------------------

static int 	chooseTurningDirection(int semiquadrant) {
int     	switching;		// Useful to choose in which direction we turn

			// According to the specified semiquadrant we choose the direction
			switching = semiquadrant * 2;
			
			//-----------------------------------------------------------------
			// If the destination is shifted on the top and if we are in the 
			// left semiquadrant we should turn on the right.
			// If we are in the right semiquadrant we need to turn left.
			//-----------------------------------------------------------------
			if (destinationPoint.X > trajectoryDestinationPoint.X)  
				return LEFTDIR + switching ;
			
			//-----------------------------------------------------------------
			// If the destination is shifted on the bottom and if we are in the 
			// left semiquadrant we should turn on the left.
			// If we are in the right semiquadrant we need to turn right.
			//-----------------------------------------------------------------
			else if(destinationPoint.X < trajectoryDestinationPoint.X)
				return RIGHTDIR - switching;
			
			// In all the other cases we go on
			return CENTER;
}

//------------------------------------------------------------------------------
// TURNING DIRECTION DEFAULT CASE:	Returns the turning direction when car's 
//									movement is not parallel to any axis. We
//									need to choose the direction according to
//									car's trajectory and destination point.
//------------------------------------------------------------------------------

static int 	turningDirectionDefaultCase(int currentDirection) {
double  m;
			// Angular coefficient according to current direction
			m = tan((double)(currentDirection * TORAD));		
			
			// Compute current trajectory
			trajectoryDestinationPoint.X = myCar->X + m * (destinationPoint.Y - 
																	myCar->Y);		
			trajectoryDestinationPoint.Y = destinationPoint.Y;
			
			//-----------------------------------------------------------------
			// We decide in which quadrant is directed our car and we compare 
			// our trajectory with the destination point
			//-----------------------------------------------------------------
			if (currentDirection > 90 && currentDirection < 270)
				return chooseTurningDirection(1);
			else
				return chooseTurningDirection(0);
	
			// In all the other cases we go on
			return CENTER;
}

//------------------------------------------------------------------------------
// TURNING DIRECTION VERTICAL AXIS: Returns the turning direction when car's 
//									movement is parallel to a vertical axis
//------------------------------------------------------------------------------

static int 	turningDirectionVerticalAxis(int semiquadrant) {
int     	switching;      // Useful to choose in which direction we turn
	
			switching = semiquadrant * 2;
			
			//------------------------------------------------------------------
			// If the destination is shifted on the left and if we are moving to 
			// the top we need to turn left.
			// If the car is moving to the bottom we need to turn right.
			//------------------------------------------------------------------
			if (myCar->Y > destinationPoint.Y)
				return LEFTDIR + switching;
			
			//------------------------------------------------------------------
			// If the destination is shifted on the right and if we are moving 
			// to the top we need to turn right.
			// If the car is moving to the bottom we need to turn left.
			//------------------------------------------------------------------
			if (myCar->Y < destinationPoint.Y)
				return RIGHTDIR - switching;
			
			//-----------------------------------------------------------------
			// If the destination is exactly on the axis but in the opposite 
			// direction we need to turn to the right.
			//-----------------------------------------------------------------
			if (myCar->X > destinationPoint.X)
				return RIGHTDIR;
			
			// In all the other cases we go on
			return CENTER;
}

//------------------------------------------------------------------------------
// TURNING DIRECTION HORIZONTAL AXIS: 	Returns the turning direction when car's 
//										movement is parallel to horizontal axis
//------------------------------------------------------------------------------

static int 	turningDirectionHorizontalAxis(int semiquadrant) {
int     	switching;      // Useful to choose in which direction we turn
	
			switching = semiquadrant * 2;
		
			//------------------------------------------------------------------
			// If the destination is shifted on the bottom and if we are moving 
			// to the left we need to turn left.
			// If the car is moving to the right we need to turn right.
			//------------------------------------------------------------------
			if (myCar->X > destinationPoint.X)
				return LEFTDIR + switching;
			
			//------------------------------------------------------------------
			// If the destination is shifted on the top and if we are moving to 
			// the left we need to turn right.
			// If the car is moving to the right we need to turn left.
			//------------------------------------------------------------------
			if (myCar->X < destinationPoint.X)
				return RIGHTDIR - switching;
			
			//------------------------------------------------------------------
			// If the destination is exactly on the axis but in the opposite 
			// direction we need to turn to the right.
			//------------------------------------------------------------------
			if (myCar->Y < destinationPoint.Y)
				return RIGHTDIR;
			
			// In all the other cases we go on
			return CENTER;
}

//------------------------------------------------------------------------------
// DIRECTION TO FINISHING LINE: 	Returns turning direction according to: 
//									car's position, car's trajectory and 
//									destination point
//------------------------------------------------------------------------------

static direction 	directionToFinishingLine(void) {
					// Get current direction angle between 0 and 360 degrees
int         		currentDirection = yawToAngle(myCar->yaw);
		
					// We don't care about 0°, we consider always 360°
					if (currentDirection == 0) 									
						currentDirection = 360;
				
					// Select car's current direction
					switch (currentDirection) {
						case 90:	// Parallel movement to axis Y going up
							return turningDirectionVerticalAxis(0);
							break;	
						case 180:	// Parallel movement to axis X going left
							return turningDirectionHorizontalAxis(0);			
							break;
						case 270:	// Parallel movement to axis Y going down
							return turningDirectionVerticalAxis(1);				
							break;	
						case 360:	// Parallel movement to axis X going right
							return turningDirectionHorizontalAxis(1);			
							break;
						default:	// Standard, arbitrary angle									
							return turningDirectionDefaultCase(currentDirection);
							break;
					}
					
					// In all the other cases we go on
					return CENTER;
}

//------------------------------------------------------------------------------
// YAW RIGHT:	Shifts car's current yaw to the right by a predefined step angle
//------------------------------------------------------------------------------

static void yawRight(void) {
			if (myCar->yaw + YAW_STEP_F > 180)
				myCar->yaw += -360; 
			else
				myCar->yaw += YAW_STEP_F;		
}

//------------------------------------------------------------------------------
// YAW LEFT:	Shifts car's current yaw to the left by a predefined step angle
//------------------------------------------------------------------------------

static void yawLeft(void){
			if (myCar->yaw - YAW_STEP_F < -180)
				myCar->yaw += 360;
			else
				myCar->yaw -= YAW_STEP_F;
}

//------------------------------------------------------------------------------
// UPDATE POSITION:	Update car's position according to his yaw, his speed and
//					the interval from the last update
//------------------------------------------------------------------------------

static void updatePosition(int interval) {
float 		clockRate = 		(float) sysClkRateGet();
float 		movementProduced =	(float) (myCar->speed * interval) / clockRate;
    
			myCar->X += movementProduced * cos(myCar->yaw * TORAD);
			myCar->Y += movementProduced * sin(myCar->yaw * TORAD);
}

//------------------------------------------------------------------------------
// ADJUST DIRECTION:	Modifies car's yaw according to the specified 
//						directions. If we are driving looking back we need to 
//						change the turning direction.
//------------------------------------------------------------------------------

static void adjustDirection(direction dir) {
			// If the car is moving forward we turn according to its direction
			if(myCar->speed > 0){
				if (dir == RIGHTDIR)
					yawRight();
				if (dir ==  LEFTDIR)
					yawLeft();
			}
			else{
				// If the car is moving back we turn in the opposite direction
				if (dir == RIGHTDIR)
					yawLeft();
				if (dir ==  LEFTDIR)
					yawRight();
			}
}

//------------------------------------------------------------------------------
// POINT DISTANCE:	Calculates point-to-point distance   
//------------------------------------------------------------------------------

static float	pointDistance(point p1, point p2) {
				return sqrt(pow((double) (p1.Y - p2.Y), 2) + 
							pow((double) (p1.X - p2.X), 2)); 
}

//------------------------------------------------------------------------------
// IS ARRIVED:	Checks if car position is near to destination point according to 
//				a given margin 
//------------------------------------------------------------------------------

static int 	isArrived() {
point   	currentPosition = {myCar->X, myCar->Y};

			//------------------------------------------------------------------
			// Compute distance between car's position and destination point and
			// if the distance is less than a margin we are arrived
			//------------------------------------------------------------------
			if (pointDistance(currentPosition, destinationPoint) <= FL_MARGIN)
				return TRUE;
			return FALSE;
}

//------------------------------------------------------------------------------
// RESET CAR INFO:	Resets car's parameters and sets manual mode
//------------------------------------------------------------------------------

void 	resetCarInfo(car *tmpCar) {
		tmpCar->X = 0;
		tmpCar->Y = 0;
		tmpCar->yaw = 0;
		tmpCar->speed = 0;
		tmpCar->view = NORMAL;
		tmpCar->sensorDirection = tmpCar->speed;
		automaticCollision = FALSE;
		manualCollision = FALSE;
		mode = MANUAL;
}

//------------------------------------------------------------------------------
// CHANGE VIEW:	Changes the current view scrolling one by one each view
//------------------------------------------------------------------------------

void 	changeView(car *tmpCar) {
		tmpCar->view = (tmpCar->view + 1) % NUM_VIEW;
}

//------------------------------------------------------------------------------
// CHANGE SENSOR VISIBILITY:	If the sensors are visibility we hide them,
//								otherwise we display them
//------------------------------------------------------------------------------

void	changeSensorVisibility(car *tmpCar){
		if (tmpCar->sensorVisible == TRUE)
			tmpCar->sensorVisible = FALSE;
		else
			tmpCar->sensorVisible = TRUE;
}

//------------------------------------------------------------------------------
// SENSOR TO ARRAY:	Converts sensors' data into a support array
//------------------------------------------------------------------------------

static void	sensorToArray(void) {
			distanceFromSensor[0] = sd->distanceFromSensor0;
			distanceFromSensor[1] = sd->distanceFromSensor1;
			distanceFromSensor[2] = sd->distanceFromSensor2;
			distanceFromSensor[3] = sd->distanceFromSensor3;
			distanceFromSensor[4] = sd->distanceFromSensor4;
}

//------------------------------------------------------------------------------
// MINIMUM DISTANCE DETETCTED: 	Returns which sensor detected the nearest
//								obstacle. If we want to avoid a sensor's data
//								we need to specify it in the parameter.
//------------------------------------------------------------------------------

static int 	minimumDistanceDetected(int toAvoid) {
float 		minDistance;		// Minimum distance detected different from 0
int 		i;
	
			minDistance = distanceFromSensor[0];
			
			for (i = 1; i < NUM_SENSORS; i++) {
				// If we want to avoid a sensor we need to specify it
				if (i != toAvoid) {
					//----------------------------------------------------------
					// If at least one of the measures is 0 we need to choose 
					// the greater one (we don't care about distance detected 
					// that are equal to 0). Otherwise we choose the minimum.
					//----------------------------------------------------------
					if (minDistance != 0 && distanceFromSensor[i] != 0)
						minDistance = min(minDistance, distanceFromSensor[i]);
					else
						minDistance = max(minDistance, distanceFromSensor[i]);
				}
			}
			
			for (i = 0; i < NUM_SENSORS; i++)
				if (minDistance == distanceFromSensor[i] && i != toAvoid)
					return i; 
		
			return -1; 
}

//------------------------------------------------------------------------------
// DIRECTION TO AVOID OBSTACLE:	Returns the direction to follow to avoid the
//								obstacle. The chosen is taken by knowing which
//								sensor saw the nearest obstacle.
//------------------------------------------------------------------------------
 
static direction 	directionToAvoidObstacle(void) {
int         		sensor;
	
					sensor = minimumDistanceDetected(-1);
					//----------------------------------------------------------
					// If the sensor is the central one we need to compute 
					// the minimum among the others
					//----------------------------------------------------------
					if (sensor == 2)
						sensor = minimumDistanceDetected(2);
					
					// If the minimum is on the left we need to turn right
					if (sensor == 0 || sensor == 1)
						return RIGHTDIR;
					
					// If the minimum is on the right we need to turn left
					if (sensor == 3 || sensor == 4)
						return LEFTDIR;
					
					// If no sensor has detected something and we continue
						return CENTER;
}

//------------------------------------------------------------------------------
// AUTO SPEED:	Adjust car's speed step by step up to a given predefined speed
//------------------------------------------------------------------------------

static void autoSpeed(void) {
			if (myCar->speed > AUTO_SPEED)
				myCar->speed--;
			if (myCar->speed < AUTO_SPEED)
				myCar->speed++;
			// We put the sensors according to current direction
			myCar->sensorDirection = myCar->speed;
}

//------------------------------------------------------------------------------
// ESCAPE FROM COLLISION:	Drifts car away from the collided obstacle in 
//							automatic drive mode when the collision was
//							detected
//------------------------------------------------------------------------------

static void escapeFromCollision(int interval, int sense) {
point   	currentPosition = {myCar->X, myCar->Y};
	
			if (sense > 0)
				myCar->speed = -AUTO_ESCAPE_SPEED ;
			else
				myCar->speed = AUTO_ESCAPE_SPEED ;
			
			// Turn sensors according to the new direction
			myCar->sensorDirection = myCar->speed;		
			
			//------------------------------------------------------------------
			// If we are not arrived at the safety distance we need to go back 
			// again by avoiding back obstacles
			//------------------------------------------------------------------
			if (pointDistance(collisionPoint, currentPosition) < SAFETY_DIST) {
				if (sd->obstacleDetected == TRUE)
					adjustDirection(directionToAvoidObstacle());
				updatePosition(interval);
			}
			else
				// Otherwise we reset state variable to continue our movement
				automaticCollision = FALSE;
			manualCollision = FALSE;
}

//------------------------------------------------------------------------------
// SET COLLISION POINT: Set collision point's coordinates
//------------------------------------------------------------------------------

static void setCollisionPoint(int x , int y) {
			collisionPoint.X = x;	
			collisionPoint.Y = y;
}

//------------------------------------------------------------------------------
// SAVE COLLISION PARAMETER: Set the collision point and store the direction 
//							 before collision
//------------------------------------------------------------------------------

static void saveCollisionParameters(void) {
			if (myCar->speed != 0)
				directionBeforeCollision = myCar->speed;			
			setCollisionPoint(myCar->X, myCar->Y);	
}

//------------------------------------------------------------------------------
// STOP AND REORIENT: 	Stop the car and reorient sensor's direction opposite to
// 						the direction before collision
//------------------------------------------------------------------------------

static void stopAndReorient(void) {
			myCar->speed = 0;								
			myCar->sensorDirection = -directionBeforeCollision;
}

//------------------------------------------------------------------------------
// AUTOMATIC MODE:	Automatic mode main function
//------------------------------------------------------------------------------

void 	automaticMode(car *tmpCar, sensorData *tmpSd, int interval) {	
		// We save in the global variables the parameters of the function
		myCar = tmpCar;
		sd = tmpSd;
		
		// If we are not arrived we need to go to the destination point
		if (isArrived() == FALSE){
			// We prefer to use an array instead of 5 different float values
			sensorToArray();		
			
			// If a collision has been detected we modify the normal behavior
			if (sd->collisionDetected == TRUE || automaticCollision == TRUE || 
													manualCollision == TRUE) {
				
				//--------------------------------------------------------------
				// If it is the first time we enter here 
				// automaticCollision == FALSE and we need to save everything 
				// about the collision
				//--------------------------------------------------------------
				if (automaticCollision == FALSE) {
					saveCollisionParameters();
					automaticCollision = TRUE;	// Automatic collision detected
					stopAndReorient();
				}
				else {
					//----------------------------------------------------------
					// If a collision is detected while we are escaping we need 
					// to start again the escape mode in the opposite direction.
					//----------------------------------------------------------
					if (sd->collisionDetected == TRUE){
						saveCollisionParameters();
						stopAndReorient();
					}
					else
						//------------------------------------------------------
						// According to the kind of collision we choose which 
						// function we have to call to escape
						//------------------------------------------------------
						escapeFromCollision(interval, directionBeforeCollision);
				}
			}
			else {
				autoSpeed();
				// If an obstacle was detected we need to avoid it
				if (sd->obstacleDetected == TRUE)
					adjustDirection(directionToAvoidObstacle());
				else
					// Otherwise we reach the destination
					adjustDirection(directionToFinishingLine());
				updatePosition(interval);
			}
		}
		else{
			// If we are arrived, we stop the car and reset the mode to manual
			myCar->speed = 0;
			mode = MANUAL;
		}
}

//------------------------------------------------------------------------------
// SPEED UP:	Increases car's speed in manual mode
//------------------------------------------------------------------------------

static void speedUp(void) {
			if (myCar->speed != MAX_SPEED_F)
				myCar->speed += STEP_SPEED;
}

//------------------------------------------------------------------------------
// SPEED DOWN:	Decreases car's speed in manual mode
//------------------------------------------------------------------------------

static void speedDown(void) {
			if (myCar->speed != MAX_SPEED_B)
				myCar->speed -= STEP_SPEED;
}

//------------------------------------------------------------------------------
// WAIT FOR REVERSE:	Reacts only to reverse movement button after a collision 
//------------------------------------------------------------------------------

static void waitForReverse(int interval, key tmpKey, key keyToPress) {
			// If the user pressed that button we can go in that direction
			if (tmpKey == keyToPress) {
				if (keyToPress == DOWN)
					speedDown();
				else
					speedUp();
				updatePosition(interval);
				manualCollision = FALSE;
				automaticCollision = FALSE;
			}
}

//------------------------------------------------------------------------------
// MANUAL MODE:	Manual mode main function
//------------------------------------------------------------------------------

void 	manualMode(car *tmpCar, sensorData *tmpSd, key tmpKey, int interval){
		// We save in the global variables the parameters of the function
		myCar = tmpCar;
		sd = tmpSd;
		
		// If a collision has been detected we modify the normal behavior
		if (sd->collisionDetected == TRUE  || manualCollision == TRUE) {	
			//------------------------------------------------------------------
			// If it is the first time we enter here manualCollision == FALSE 
			// and we need to save everything about the collision.
			//------------------------------------------------------------------
			if (manualCollision == FALSE)
				saveCollisionParameters();
			manualCollision = TRUE;
			stopAndReorient();
			
			//------------------------------------------------------------------
			// According to the direction before the collision we choose in 
			// which direction we have to move the car.		
			//------------------------------------------------------------------
			if (directionBeforeCollision > 0)
				waitForReverse(interval, tmpKey, DOWN);
			else
				waitForReverse(interval, tmpKey, UP);
		}
		else{
			// According to the user's choices we execute different functions
			switch (tmpKey) {
				case UP:
					speedUp();
					updatePosition(interval);
					break;
				case DOWN:
					speedDown();
					updatePosition(interval);
					break;			
				case LEFT:
					// We turn the car only if the car is moving
					if (myCar->speed != 0)		
						yawLeft();
					updatePosition(interval);
					break;	
				case RIGHT:
					// We turn the car only if the car is moving
					if (myCar->speed != 0)
						yawRight();
					updatePosition(interval);
					break;
				default:
					updatePosition(interval);
					break;
			}
			myCar->sensorDirection = myCar->speed;
		}
}
