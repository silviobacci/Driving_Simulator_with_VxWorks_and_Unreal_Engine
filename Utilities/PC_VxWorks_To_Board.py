import socket

# QUESTO SERVER DEVE GIRARE SUL PC IN CUI SI TROVA VXWORKS

# NON MODIFICARE NESSUNA PORTA

# Da modificare con l'inidirizzo su cui sta girando VxWorks
UDP_IP_RECV = "192.168.1.4"
UDP_PORT_RECV = 3000

# Da non modificare (e' l'indirizzo della board)
UDP_IP_SEND = "192.168.1.10"
UDP_PORT_SEND = 3000

sock_recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_recv.bind((UDP_IP_RECV, UDP_PORT_RECV))

while True:
	data, addr = sock_recv.recvfrom(5000)
	print ("received message")
	sock_send.sendto(data, (UDP_IP_SEND, UDP_PORT_SEND))
	
