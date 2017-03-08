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
	print 'Server is listening on port #: %d' %clientServerPort
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

	#create a socket
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	#convert the data port to an int to open the data port listening socket
	clientServerPort = int(sys.argv[4])

	#bind the socket
	BindSocket(serverSocket, clientServerPort)

	#Start listening on socket	
	ListenSocket(serverSocket)

	#get the name of the client server to send for the data port connection
	clientHost = socket.gethostname()

	#concantenate arguments into message string
	message = command + ':' + dataPort + ':' + clientHost + ':'

	try:
	    sock.sendall(message)
	 	
	    #while amount_received < amount_expected:
	     #   data = sock.recv(16)
	     #   amount_received += len(data)
	    response = sock.recv(500)
	    print 'Message returned from server: %s' %response
	
	finally:
		print >>sys.stderr, 'closing socket'
		sock.close()
		serverSocket.close()
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

'''
#check the type of command entered for the third command line argument
if validListCommand == True:
	print 'listing directories...'
	dataPort = int(sys.argv[4])

	#create a socket
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	#bind the socket
	BindSocket(serverSocket, dataPort)

	#Start listening on socket	
	ListenSocket(serverSocket)

	print 'hostname: %s' %socket.getfqdn()
	host = socket.getfqdn()
	message = arguments + host

	SendListCommand(sock, message)

	#close the data connection
	serverSocket.close()
	#do more in here
elif validFileRequestCommand == True:
	print 'requesting file...'
	fileName = sys.argv[4]
	dataPort = sys.argv[5]

	#create a socket
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	#bind the socket
	BindSocket(serverSocket, dataPort)

	#Start listening on socket	
	ListenSocket(serverSocket)

	SendGetFileCommand(sock, arguments)

	#close the data connection
	serverSocket.close()
else:
	print 'INVALID REQUEST ARGUMENT!'
	exit()	
'''
