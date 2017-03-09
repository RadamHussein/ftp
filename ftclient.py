import socket
import sys

#this function is used for binding the socket to the port number.
#It takes a socket object as an argument. 
def BindSocket(serverSocket, port):
	try:
		serverSocket.bind(('', port))
	except socket.error as msg:
		print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
		sys.exit()

#this function starts the server listening on the specified port
#It takes a socket object as an argument.
def ListenSocket(serverSocket):
	serverSocket.listen(1)
	print 'Data port open on port #: %d' %clientServerPort
	#print 'hostname: %s' %socket.getfqdn()

'''
This functions examines the third command line argument 
to see if it is a "-l" list command. 
'''
def CheckForListCommand(command):
	listCommand = '-l'
	if command == listCommand:
		return True
	else:
	 return False

'''
This functions examines the third command line argument 
to see if it is a "-g" command requesting a file. 
'''
def CheckForFileRequestCommand(command):
	fileCommand = '-g'
	if command == fileCommand:
		return True
	else:
		return False

#function for sending command and port number through 
#control connection to the server
def SendListCommand(sock, message):
	try:
    
	    # Send data
	    #message = 'This is the message.'
	    #print >>sys.stderr, 'sending "%s"' % message
	    sock.sendall(message)

	    # Look for the response
	    #amount_received = 0
	    #amount_expected = len(message)
	 	
	    #while amount_received < amount_expected:
	     #   data = sock.recv(16)
	     #   amount_received += len(data)
		
	finally:
		print >>sys.stderr, 'closing socket'
		sock.close()

'''
This function is for sending a Get File command
through the control connection to the server
'''
def SendGetFileCommand(sock, message):
	try:
    
	    # Send data
	    #message = 'This is the message.'
	    #print >>sys.stderr, 'sending "%s"' % message
	    sock.sendall(message)

	    # Look for the response
	    #amount_received = 0
	    #amount_expected = len(message)
	 	
	    #while amount_received < amount_expected:
	     #   data = sock.recv(16)
	     #   amount_received += len(data)
	
	finally:
		print >>sys.stderr, 'closing socket'
		sock.close()

#this function is used to establish a connection with a client
#It takes a socket object as an argument
def ConnectToClient(socket):
	connectionSocket, addr = socket.accept()
	print 'Connected with ' + addr[0] + ':' + str(addr[1])
	print '\n'
	return connectionSocket

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

#get arguments into variables
hostName = sys.argv[1]
portNumber = int(sys.argv[2])

# Connect the socket to the port where the server is listening
server_address = (hostName, portNumber)
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)

#checking arguments
print 'Number of arguments: ', len(sys.argv)
print 'Argument List:', str(sys.argv)
arguments = str(sys.argv)

#new control logic - no more validating arguments in client
if len(sys.argv) == 5:
	#get arguments into variables
	hostName = sys.argv[1]
	portNumber = sys.argv[2]
	command = sys.argv[3]
	dataPort = sys.argv[4]

	#get the name of the client server to send for the data port connection
	clientHost = socket.gethostname()

	#concantenate arguments into message string
	message = command + ':' + dataPort + ':' + clientHost + ':'

	try:
		sock.sendall(message)
			 	
		response = sock.recv(500)
		if response != '1':
			print '%s' %response
		else: 
			print 'Valid command recieved'
			
	finally:
		print >>sys.stderr, 'closing socket'
		sock.close()
elif len(sys.argv) == 6:
	#get arguments into variables
	hostName = sys.argv[1]
	portNumber = int(sys.argv[2])
	command = sys.argv[3]
	fileName = sys.argv[4]
	dataPort = sys.argv[5]
	clientHost = sys.argv[6]	

	#concantenate arguments into message string
	message = hostName + portNumber + command + fileName + dataPort + clientHost
else:
	print "IVALID ARGUMENTS FORMAT... <SERVER_HOST> <SERVER_PORT> <COMMAND> <[FILENAME]> <DATAPORT>"
	exit()

#DATA PORT CODE
#create a socket for data port
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

#convert the data port to an int to open the data port listening socket
clientServerPort = int(dataPort)

#bind the socket for data port
BindSocket(serverSocket, clientServerPort)

#Start listening on socket for data port
ListenSocket(serverSocket)

try:
	while True:
		#connect to through data port
		connection = ConnectToClient(serverSocket)
		
		try:
			while True:
				#receive message from client
				clientMessage = ReceiveMessage(connection)

				if clientMessage:
					#write response to file or display
					print '%s' %clientMessage
					break
				else:
					print >>sys.stderr, 'no more data from client'
					connection.close()
					break
		except:
			connection.close()
			serverSocket.close()
except:
	connection.close()
	serverSocket.close()
	print '\n'
	print 'Goodbye!\n'

 
serverSocket.close()

'''
	try:
		while True:
			#connect to server through data port
			connection = ConnectToClient(serverSocket)

			#get the name of the client server to send for the data port connection
			clientHost = socket.gethostname()

			#concantenate arguments into message string
			message = command + ':' + dataPort + ':' + clientHost + ':'

			try:
			    sock.sendall(message)
			 	
			    response = sock.recv(500)
			    print 'Message returned from server: %s' %response
			
			finally:
				print >>sys.stderr, 'closing socket'
				sock.close()
	finally:
		connection.close()
		serverSocket.close()
'''

