// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Networking.h"
#include "Components/ActorComponent.h"
#include "CustomData.h"
#include "Messages.h"
#include "UDP_Communication.generated.h"


UCLASS( ClassGroup=(Communication))
class RT_PROJECT_API UUDP_Communication : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Remote Address")
		FString SourceIP_Address;
	UPROPERTY(EditAnywhere, Category = "Remote Address")
		int32 PortIn;

	UPROPERTY(EditAnywhere, Category = "Remote Address")
		FString DestIP_Address;
	UPROPERTY(EditAnywhere, Category = "Remote Address")
		int32 PortOut;

public:	

	// Sets default values for this component's properties
	UUDP_Communication();

	/** Called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Parsing the Received Data
	void Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	// Start the communication
	bool StartUDPComm(const FString& YourChosenSocketName);

	// Retrieve Data
	void GetData(FCustomInputData* RetData);

	// Send Data
	void SendData(FCustomOutputData* OutData);

private:

	// UDP Socket stuff
	FSocket* ListenSocket;   // Input Socket
	FSocket* SendSocket;	 // Output Socket

	ISocketSubsystem* SocketSubsystem;

	FUdpSocketReceiver* UDPReceiver = nullptr;

	// Address of the remote machine
	TSharedPtr<FInternetAddr>	RemoteAddr;

	// Received Data
	FCustomInputData Data;

	// Output Data
	FCustomOutputData DataOut;
	
};


