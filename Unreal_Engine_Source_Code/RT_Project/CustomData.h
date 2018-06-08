#pragma once

#include "CustomData.generated.h"
USTRUCT()
struct FCustomInputData
{
	GENERATED_USTRUCT_BODY()

	// Position [m]
	float X = 0.0f;
	float Y = 0.0f;

	// Attitude [deg]
	float Yaw = 0.0f;
	
    // Speed of the car [m/s]
	int speed = 0;
    
    // Current view [0/1/2]
	int currentView = 0;
    
    // Direction in which we put sensors
    int sensorDirection = 1;
    
    // Boolean to show sensors or not
    int sensorVisible = 0;

	FCustomInputData() {}
};



USTRUCT()
struct FCustomOutputData
{
	GENERATED_USTRUCT_BODY()
    
	// Collision
	int collisionDetected = 0;
	
	// Obstacle Detection
	int obstacleDetected = 0;
    
    // Detected distances from sensors
    float distanceFromSensor0 = 0;
    float distanceFromSensor1 = 0;
    float distanceFromSensor2 = 0;
    float distanceFromSensor3 = 0;
    float distanceFromSensor4 = 0;

	FCustomOutputData() {}
};


FORCEINLINE FArchive& operator<<(FArchive &Ar, FCustomInputData& TheStruct)
{
	Ar << TheStruct.X;
	Ar << TheStruct.Y;
	Ar << TheStruct.Yaw;
	Ar << TheStruct.speed;
    Ar << TheStruct.currentView;
    Ar << TheStruct.sensorDirection;
    Ar << TheStruct.sensorVisible;

	return Ar;
}


FORCEINLINE FArchive& operator<<(FArchive &Ar, FCustomOutputData& TheStruct)
{
	Ar << TheStruct.collisionDetected;
	Ar << TheStruct.obstacleDetected;
    Ar << TheStruct.distanceFromSensor0;
    Ar << TheStruct.distanceFromSensor1;
    Ar << TheStruct.distanceFromSensor2;
    Ar << TheStruct.distanceFromSensor3;
    Ar << TheStruct.distanceFromSensor4;

	return Ar;
}
