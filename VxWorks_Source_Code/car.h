#ifndef CAR_H
#define CAR_H

#include "keyboard.h"

//------------------------------------------------------------------------------
// GLOBAL AND EXTERN DATA STRUCTURES
//------------------------------------------------------------------------------

// This structure is sent to Unreal Engine to describe car's behavior
typedef struct Car {
	float X;					// Car's position on X axis in meters
	float Y;					// Car's position on Y axis in meters

	float yaw;					// Car's direction in degrees
	
	int speed;					// Car's speed in m/s
	int view;					// Current car's view
	int sensorDirection;		// Direction in which sensors are attached
	int sensorVisible;          // With 0 hide sensors, with 1 show sensors
} car;

// This structure is received from Unreal Engine to describe sensors' detections
typedef struct SensorData {
	// Collision
	int collisionDetected;		// Returns 1 if a collision was detected
	
	// Obstacle Detection
	int obstacleDetected;		// Returns 1 if an obstacle was detected
	
	// Each sensor is separated from the next one by an angle of 30 degrees
	float distanceFromSensor0;	// Distance from the sensor to the left
	float distanceFromSensor1;
	float distanceFromSensor2;	// Distance from the central sensor
	float distanceFromSensor3;	
	float distanceFromSensor4;	// Distance from the sensor to the right	
} sensorData;

// Possible views
typedef enum view {
	NORMAL, 
	OVER, 
	ONBOARD
} view;					

// Directions followed by the car
typedef enum dir {
	LEFTDIR, 
	CENTER, 
	RIGHTDIR
} direction;

// Cartesian point
typedef struct Point {												
	float X;
	float Y;
} point;

//------------------------------------------------------------------------------
// GLOBAL AND EXTERN VARIABLES
//------------------------------------------------------------------------------

extern point 		destinationPoint;

extern int          mode;

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// RESET CAR INFO:	Resets car's parameters and sets manual mode
//------------------------------------------------------------------------------

void resetCarInfo(car *tmpCar);

//------------------------------------------------------------------------------
// CHANGE VIEW:	Changes the current view scrolling one by one each view
//------------------------------------------------------------------------------

void changeView(car *tmpCar);

//------------------------------------------------------------------------------
// CHANGE SENSOR VISIBILITY:	If the sensors are visibility we hide them,
//								otherwise we display them
//------------------------------------------------------------------------------

void changeSensorVisibility(car *tmpCar);

//------------------------------------------------------------------------------
// AUTOMATIC MODE:	Automatic mode main function
//------------------------------------------------------------------------------

void automaticMode(car *tmpCar, sensorData *tmpSd, int interval);

//------------------------------------------------------------------------------
// MANUAL MODE:	Manual mode main function
//------------------------------------------------------------------------------

void manualMode(car *tmpCar, sensorData *tmpSd, key tmpKey, int interval);

#endif
