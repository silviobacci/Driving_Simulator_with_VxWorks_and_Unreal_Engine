import socket

# QUESTO SERVER DEVE GIRARE SUL PC IN CUI SI TROVA VXWORKS

# NON MODIFICARE NESSUNA PORTA

# Da non modificare (e' l'indirizzo dell'interfaccia ethernet vista dalla board)
UDP_IP_RECV = "192.168.1.1"
UDP_PORT_RECV = 8000

# Da modificare con l'indirizzo IP del pc in cui sta girando UE
UDP_IP_SEND = "192.168.1.26"
UDP_PORT_SEND = 8000

sock_recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_recv.bind((UDP_IP_RECV, UDP_PORT_RECV))

while True:
	data, addr = sock_recv.recvfrom(5000)
	print ("received message")
	sock_send.sendto(data, (UDP_IP_SEND, UDP_PORT_SEND))
	
