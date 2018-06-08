// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "UDP_Communication.h"
#include "Car.generated.h"

#define NUM_SENSOR      5

UCLASS()
class RT_PROJECT_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Get the Pose from the Data received and return it in the correspondend vectors
	void GetPose(FRotator* Rotation, FVector* Position, int * Speed, int * currentView, int * sensorVisible, int *sensorDirection);
    
    // Create sensors as FRotator elements
    void CreateSensors();
    
    // Compute sensor rotation, change FRotatoe in FVector and show them, if needed, on the screen
    void SetSensorRotation(FVector Position, FRotator Rotation, int sensorDirection, int sensorVisible);
    
    // Detect obstacles during the path of a sensor and save all the parameters we need
    void DetectObstacles(float * distanceFromSensor, int * obstacleDetected, int * collisionDetected, int index, FVector Position, FCollisionQueryParams Params);
    
    // Indicate that the landscape is the only object we want to avoid
    void AvoidLandscape(FCollisionQueryParams * Params);
    
    // Set the view accoridng to user's decision
    void SetView(int view, int speed, FVector Position);

	// Initialize the communication stuff
	virtual void PreInitializeComponents() override;

private:
	// Camera Component
	UPROPERTY(EditAnywhere, Category = "Camera")
	UCameraComponent* OutBoardCamera;

	// Arm of the camera Component
    UPROPERTY(EditAnywhere, Category = "Spring")
	USpringArmComponent* CameraSpringArm;

    // Car Component on which we apply the mesh we desire
	UPROPERTY(EditAnywhere, Category = "Geometry")
	USceneComponent* OurVisibleComponent;

	// Communication Component
	UPROPERTY(EditAnywhere, Category = "Remote Address")
	class UUDP_Communication* OurCommunicationComponent;
    
    // Array of car length in each directions of the sensors
    float carLengthSensor[NUM_SENSOR];
    
    // Array with sensors as FRotator (useful to turn them according to car's direction)
    FRotator sensorRotation[NUM_SENSOR];
    
    // Array with sensors as FVector that are used like real sensors
    FVector sensorVector[NUM_SENSOR];
};
