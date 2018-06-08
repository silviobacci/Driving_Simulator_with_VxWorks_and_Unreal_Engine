//------------------------------------------------------------------------------
// UDP: 	Creation and utilization of the structures for UDP communication
//------------------------------------------------------------------------------

#include 	<hostLib.h> 
#include 	<inetLib.h>
#include 	<sockLib.h>
#include 	<string.h>

#include 	"udp.h" 

//------------------------------------------------------------------------------
// GLOBAL CONSTANTS
//------------------------------------------------------------------------------

#define IP_ADDRESS_UE			"192.168.1.1" 		
#define PORT_TO_SEND     		8000    		  
#define PORT_TO_RECEIVE			3000    

//------------------------------------------------------------------------------
// GLOBAL VARIABLES
//------------------------------------------------------------------------------

static address	unrealEngineAddress, myAddress;	// Informations about peers

static int 		sockRecv, sockSend;				// Sockets to send and receive

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// UDP BIND: Initialization of the structure and execution of the bind operation
//------------------------------------------------------------------------------

static int 	UDPbind(void) {
			// Initialization of the structure with board network's info
			bzero((char *) &myAddress, sizeof(address));
			myAddress.sin_len = (u_char) sizeof(address);
			myAddress.sin_family = AF_INET;
			myAddress.sin_port = htons(PORT_TO_RECEIVE);
			myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
			
			return bind(sockRecv, (sockaddr *) &myAddress, sizeof(address)); 
}

//------------------------------------------------------------------------------
// CREATE UNRAL ENGINE ADDRESS: Creation of the structure of Unreal Engine
//------------------------------------------------------------------------------

static int 	createUnrealEngineAddress(void) {	
			// Initialization of the struct with network's info of Unreal Engine
			bzero((char *) &unrealEngineAddress, sizeof(address));
			unrealEngineAddress.sin_len = (u_char) sizeof(address);
			unrealEngineAddress.sin_family = AF_INET;
			unrealEngineAddress.sin_port = htons(PORT_TO_SEND);
		
			if ((int)(unrealEngineAddress.sin_addr.s_addr = 
											inet_addr(IP_ADDRESS_UE)) == ERROR)
				return -1;
			return 0;
}

//------------------------------------------------------------------------------
// UDP INIT: Creation of the connection
//------------------------------------------------------------------------------

int 		UDPinit(void) { 
const int 	REUSE_ADDR = 1;			// Constant required to create a connection
	
			if ((sockRecv = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
				return -1;
			
			if (setsockopt(sockRecv, SOL_SOCKET, SO_REUSEADDR, &REUSE_ADDR, 
															sizeof(int)) < 0)
				return -1;
			
			if (UDPbind() < 0)
				return -1;
			
			if ((sockSend = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
				return -1;
			
			if (setsockopt(sockSend, SOL_SOCKET, SO_REUSEADDR, &REUSE_ADDR, 
															sizeof(int)) < 0)
				return -1;
			
			if (createUnrealEngineAddress() < 0)
				return -1;
	
		return 0;
}

//------------------------------------------------------------------------------
// UDP SEND: 	Send the car structure to Unreal Engine
//------------------------------------------------------------------------------

void 	UDPsend(car * tmpCar) {
    	sendto(sockSend, tmpCar, sizeof(car), 0, 
    					(sockaddr *) &unrealEngineAddress, sizeof(address));
}

//------------------------------------------------------------------------------
// UDP RECV: 	Receive sensor's data from Unreal Engine and returns the number 
//				of bytes read.
//------------------------------------------------------------------------------

int		UDPrecv(sensorData * tmpSd) {
int     sockAddrSize = sizeof(address);
		return recvfrom(sockRecv, tmpSd, sizeof(sensorData), 
				MSG_DONTWAIT, (sockaddr *) &myAddress, &sockAddrSize);		
}
