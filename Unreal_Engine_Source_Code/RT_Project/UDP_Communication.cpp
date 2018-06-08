#include "RT_Project.h"
#include "UDP_Communication.h"

UUDP_Communication::UUDP_Communication(): SourceIP_Address(FString("0.0.0.0")), PortIn(8000), DestIP_Address(FString("192.168.1.10")), PortOut(3000)
{
	bWantsBeginPlay = false;
	bAutoActivate = false;
	PrimaryComponentTick.bCanEverTick = false;

	ListenSocket = NULL;
	SendSocket = NULL;
}

bool UUDP_Communication::StartUDPComm(const FString& YourChosenSocketName) {
	FIPv4Address SourceAddr, DestAddr;

	FIPv4Address::Parse(SourceIP_Address, SourceAddr);
	FIPv4Address::Parse(DestIP_Address, DestAddr);

	FIPv4Endpoint InputEndpoint(SourceAddr, PortIn);
	FIPv4Endpoint OutputEndpoint(DestAddr, PortOut);

	bool valid = false;
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	RemoteAddr->SetIp(*DestIP_Address, valid);
	RemoteAddr->SetPort(PortOut);

	if (!valid)
		return false;

	int32 BufferSize = 2 * 1024 * 1024;

	ListenSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(InputEndpoint)
		.WithReceiveBufferSize(BufferSize);

	SendSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsReusable()
		.WithSendBufferSize(BufferSize)
		.WithBroadcast();

	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(4);
	UDPReceiver = new FUdpSocketReceiver(ListenSocket, ThreadWaitTime, TEXT("UDP RECEIVER"));
	UDPReceiver->OnDataReceived().BindUObject(this, &UUDP_Communication::Recv);
	UDPReceiver->Start();

	return true;
}

void UUDP_Communication::Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt) {
	*ArrayReaderPtr << Data;
}

void UUDP_Communication::GetData(FCustomInputData* RetData) {
	*RetData = Data;
}


void UUDP_Communication::SendData(FCustomOutputData* OutData) {
	FArrayWriter OutputRaw;
	int32 byteSent = 0;
	OutputRaw << *OutData;
	SendSocket->SendTo(OutputRaw.GetData(),OutputRaw.Num(),byteSent,*RemoteAddr);
}


void UUDP_Communication::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	if (UDPReceiver != nullptr){
		UDPReceiver->Stop();
		delete UDPReceiver;
		UDPReceiver = nullptr;
	}
    
	if (ListenSocket){
		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	}

	if (SendSocket){
		SendSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SendSocket);
	}
	
}
