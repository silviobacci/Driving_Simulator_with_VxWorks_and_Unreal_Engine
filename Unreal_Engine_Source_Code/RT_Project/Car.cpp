#include "RT_Project.h"
#include "Messages.h"
#include "CustomData.h"
#include "Car.h"

#define ANGLE           30.0f
#define RAY_DISTANCE    500.0f
#define CAR_WIDTH       45.0f
#define CAR_LENGTH      95.0f
#define DISTANCE        0

// Sets default values
ACar::ACar() {
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	OurVisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OurVisibleComponent"));
    OurVisibleComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	OurVisibleComponent->SetupAttachment(RootComponent);
	RootComponent = OurVisibleComponent;

	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	OutBoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OutBoardCamera"));
	
	CameraSpringArm->SetRelativeRotation(FRotator(-15.f, 180.f, 0.f));
	CameraSpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
	CameraSpringArm->SetupAttachment(RootComponent);
	CameraSpringArm->TargetArmLength = 600.0f;
	CameraSpringArm->bEnableCameraRotationLag = true;
	CameraSpringArm->CameraRotationLagSpeed = 7.f;
	CameraSpringArm->bInheritPitch = false;
	CameraSpringArm->bInheritRoll = false;
    CameraSpringArm->bInheritYaw = false;

	OutBoardCamera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);
	OutBoardCamera->bUsePawnControlRotation = false;
	OutBoardCamera->FieldOfView = 90.f;
    
	OurCommunicationComponent = CreateDefaultSubobject<UUDP_Communication>(TEXT("CommuncationComponent"));
    
    CreateSensors();
}

void ACar::BeginPlay() {
	Super::BeginPlay();
}

void ACar::CreateSensors(){
    // We compute the car length on the direction of each sensor
    carLengthSensor[0] = CAR_WIDTH / cos(    ANGLE * PI / 180);
    carLengthSensor[1] = CAR_WIDTH / cos(2 * ANGLE * PI / 180);
    carLengthSensor[2] = CAR_LENGTH;
    carLengthSensor[3] = CAR_WIDTH / cos(2 * ANGLE * PI / 180);
    carLengthSensor[4] = CAR_WIDTH / cos(    ANGLE * PI / 180);
    
    FVector distance = FVector(RAY_DISTANCE, 0.0f, 0.0f);
    
    // We create 5 FRotator sensor of lentgh RAY_DISTANCE
    for(int i = 0; i < NUM_SENSOR; i++)
        sensorRotation[i] = distance.Rotation();
}

void ACar::SetSensorRotation(FVector Position, FRotator Rotation, int sensorDirection, int sensorVisible){
    // According to the direction of the car, according to the current orinetation we compure the rotation of the sensors
    if(sensorDirection >= 0){
        sensorRotation[0].Yaw = -2 * ANGLE + Rotation.Yaw;
        if(sensorRotation[0].Yaw  <  (- 180.0f))
            sensorRotation[0].Yaw += 360.0f;
        
        sensorRotation[1].Yaw =     -ANGLE + Rotation.Yaw;
        if(sensorRotation[1].Yaw  <  (- 180.0f))
            sensorRotation[1].Yaw += 360.0f;
        
        sensorRotation[2].Yaw =              Rotation.Yaw;
        
        sensorRotation[3].Yaw =      ANGLE + Rotation.Yaw;
        if(sensorRotation[3].Yaw  >  180.0f)
            sensorRotation[3].Yaw += (- 360.0f);
        
        sensorRotation[4].Yaw =  2 * ANGLE + Rotation.Yaw;
        if(sensorRotation[4].Yaw  >  180.0f)
            sensorRotation[4].Yaw+= (- 360.0f);
    }
    else {
        sensorRotation[0].Yaw =  2 * ANGLE + Rotation.Yaw - 180;
        if(sensorRotation[0].Yaw  <  (- 180.0f))
            sensorRotation[0].Yaw += 360.0f;
        
        sensorRotation[1].Yaw =      ANGLE + Rotation.Yaw - 180;
        if(sensorRotation[1].Yaw  <  (- 180.0f))
            sensorRotation[1].Yaw += 360.0f;
        
        sensorRotation[2].Yaw = Rotation.Yaw + 180;
        
        sensorRotation[3].Yaw =     -ANGLE + Rotation.Yaw + 180;
        if(sensorRotation[3].Yaw  >  180.0f)
            sensorRotation[3].Yaw += (- 360.0f);
        
        sensorRotation[4].Yaw = -2 * ANGLE + Rotation.Yaw + 180;
        if(sensorRotation[4].Yaw  >  180.0f)
            sensorRotation[4].Yaw += (- 360.0f);
    }
    
    // For each sensor we create the real sensor, that is vector of length RAY_DISTANCE starting from the car
    for(int i = 0; i < NUM_SENSOR; i++){
        sensorVector[i] = sensorRotation[i].Vector() * FVector(RAY_DISTANCE + carLengthSensor[i], RAY_DISTANCE + carLengthSensor[i], 0);
        if(sensorVisible == 1)
            DrawDebugLine(GetWorld(), Position, Position + sensorVector[i], FColor(255,255,255), false, -1.0f, 0, 10.0f);
    }
}

void ACar::DetectObstacles(float * distanceFromSensor, int * obstacleDetected, int * collisionDetected, int index, FVector Position, FCollisionQueryParams Params){
    FHitResult Hit;
    ECollisionChannel TraceChannel = ECollisionChannel();
    
    // We check if an object is detected or not by the sensor
    bool obsDet = GetWorld()->LineTraceSingleByChannel(Hit, Position, Position+sensorVector[index], TraceChannel, Params);
    if(obsDet == true){
        // If the obstacle is detected in Hit.Distance we can find the distance to the obstacle
        *distanceFromSensor = Hit.Distance;
        // The distance is taken from the center of the car so we need to subtract the car length
        *distanceFromSensor -= carLengthSensor[index];
        *obstacleDetected = 1;
        // If the distance is less than a certain value there was a collision
        if(*distanceFromSensor <= DISTANCE)
            *collisionDetected = 1;
    }
}

void ACar::AvoidLandscape(FCollisionQueryParams * Params){
    Params->AddIgnoredActor(this);
    
    // We search the landscape among all the components and then we say that we want to avoid it
    UWorld* MyWorld = GetWorld();
    for(TObjectIterator<AActor> Itr; Itr; ++Itr)
        if(MyWorld == Itr->GetWorld() && Itr->GetName() == "Landscape_0")
            Params->AddIgnoredActor(((int32) Itr->GetUniqueID()));
}

void ACar::SetView(int view, int speed, FVector Position){
    switch(view){
        case 0:
            OutBoardCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
            CameraSpringArm->TargetArmLength = 600.f;
            CameraSpringArm->SetRelativeRotation(FRotator(-15.f, 180.f, 0.f));
            CameraSpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
            break;
        case 1:
            OutBoardCamera->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
            CameraSpringArm->TargetArmLength = 0.f;
            CameraSpringArm->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
            CameraSpringArm->TargetOffset = FVector(0.f, 0.f, 2500.f);
            break;
        case 2:
            OutBoardCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
            if(speed >= 0){
                CameraSpringArm->TargetArmLength = 0.f;
                CameraSpringArm->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
                CameraSpringArm->TargetOffset = FVector(0.f, 0.f, Position.Z + 55.f);
            }
            else{
                CameraSpringArm->TargetArmLength = -50.f;
                CameraSpringArm->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
                CameraSpringArm->TargetOffset = FVector(0.f, 0.f, Position.Z + 55.f);
            }
            break;
    }
}

void ACar::Tick(float DeltaTime) {
    int                 sensorVisible;
    int                 sensorDirection;
    int                 view;
	int                 speed;
    FCustomOutputData   SendData;
    FRotator            Rotation = FRotator(0, 0, 0);
	FVector             Position = FVector(0, 0, 0);
	
	Super::Tick(DeltaTime);

	// Get the pose of the object in the 3D space
	GetPose(&Rotation, &Position, &speed, &view, &sensorVisible, &sensorDirection);

	// Set Orientation
	SetActorRelativeRotation(Rotation.Quaternion());
	
    // Set Position
	SetActorLocation(Position);
    
    // Set the current view chosen by the user
    SetView(view, speed, Position);

    // Reset the rotation to original value
    Rotation.Yaw += 180.f;
    
    // Set sensor rotation and eventually show the sensors
    SetSensorRotation(Position, Rotation, sensorDirection, sensorVisible);
    
    // Create paramters to detect obstacles and indicate which obstacle we don't want to detect
    FCollisionQueryParams Params = FCollisionQueryParams(true);
    AvoidLandscape(&Params);
    
    SendData.collisionDetected = 0;
    
    // For each sensor we call the function to detect obstacles
    DetectObstacles(&SendData.distanceFromSensor0, &SendData.obstacleDetected, &SendData.collisionDetected, 0, Position, Params);
    DetectObstacles(&SendData.distanceFromSensor1, &SendData.obstacleDetected, &SendData.collisionDetected, 1, Position, Params);
    DetectObstacles(&SendData.distanceFromSensor2, &SendData.obstacleDetected, &SendData.collisionDetected, 2, Position, Params);
    DetectObstacles(&SendData.distanceFromSensor3, &SendData.obstacleDetected, &SendData.collisionDetected, 3, Position, Params);
    DetectObstacles(&SendData.distanceFromSensor4, &SendData.obstacleDetected, &SendData.collisionDetected, 4, Position, Params);
    
    OurCommunicationComponent->SendData(&SendData);
}

void ACar::SetupPlayerInputComponent(class UInputComponent* InputComponent){
    Super::SetupPlayerInputComponent(InputComponent);
}

void ACar::GetPose(FRotator* Rotation, FVector* Position, int *speed, int *currentView, int *sensorVisible, int *sensorDirection){
	FCustomInputData ReceivedData;

    // Get data from UDP socket
	OurCommunicationComponent->GetData(&ReceivedData);

    // The mesh of the car is in the opposite direction, so we need to turn it of 180 degrees
    Rotation->Yaw = ReceivedData.Yaw + 180.f;
    
    // Our computation are done in m but Unreal Engine needs cm
    Position->X = ReceivedData.X * 100;
    Position->Y = ReceivedData.Y * 100;

    // Set all the other parameters we need
	*speed = ReceivedData.speed;
	*currentView = ReceivedData.currentView;
    *sensorDirection = ReceivedData.sensorDirection;
    *sensorVisible = ReceivedData.sensorVisible;
}

void ACar::PreInitializeComponents(){
	Super::PreInitializeComponents();
	OurCommunicationComponent->StartUDPComm("PawnCommunicationComponent");
}
