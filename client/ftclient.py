import socket
import sys
import os.path

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

#this function is used to receive messages from the client
#It takes a connection object as an argument. 
def ReceiveMessage(connectionSocket):
	message = connectionSocket.recv(5000)
	return message

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

'''
This function checks the current directory for the file the
user requested. If the file exists, an error is displayed and 
the program terminates. If it does not exist, execution continues
normally. It takes a fileName, and the open socket as arguments. 
'''
def checkForExistingFile(requestedFile):
	#check for duplicate file
	existingFile = os.path.exists(fileName)

	#if user is requesting an existing file. Display message and quit
	if existingFile == True:
		return True
	return False
		#print 'DUPLICATE FILE WARNING. %s ALREADY EXISTS.' %fileName
		#socket.close()
		#exit()

'''
This function is used to handle a received file
'''
def receiveFile(fileName, message):
	print "%s" %fileName
	print(os.path.exists(fileName))
	if (os.path.exists(fileName) == True):
		print 'DUPLICATE FILE WARNING. "%s" ALREADY EXISTS.' %fileName
	else:
		#get a file ready to receive data being sent back on data connection
		newFile = open(fileName, "w")
		newFile.write(message);
		newFile.close()

def handleControlConnectionMessage(sock, message):
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

	handleControlConnectionMessage(sock, message)

elif len(sys.argv) == 6:
	#get arguments into variables
	hostName = sys.argv[1]
	portNumber = int(sys.argv[2])
	command = sys.argv[3]
	fileName = sys.argv[4]
	dataPort = sys.argv[5]
	#clientHost = sys.argv[6]	

	#check current directory for the file the user is requesting
	#checkForExistingFile(fileName, sock)

	#get the name of the client server to send for the data port connection
	clientHost = socket.gethostname()

	#concantenate arguments into message string
	message = command + ':' + fileName + ':' + dataPort + ':' + clientHost + ':' 

	#get a file ready to receive data being sent back on data connection
	#newFile = open(fileName, "w")

	handleControlConnectionMessage(sock, message)

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
		print 'waiting for client to connect...'
		connection = ConnectToClient(serverSocket)
		
		try:
			while True:
				#receive message from client
				clientMessage = ReceiveMessage(connection)

				if clientMessage:
					if (len(sys.argv) == 6):	
						receiveFile(fileName, clientMessage)					
						break
					else:
						#display response
						print '%s' %clientMessage
						break
				else:
					print >>sys.stderr, 'no more data from client'
					connection.close()
					break
		except:
			connection.close()
			serverSocket.close()
		break		
except:
	connection.close()
	serverSocket.close()
	print '\n'
	print 'Goodbye!\n'

 
serverSocket.close()

'''	try:
		sock.sendall(message)
			 	
		response = sock.recv(500)
		if response != '1':
			print '%s' %response
		else: 
			print 'Valid command recieved'
			
	finally:
		print >>sys.stderr, 'closing socket'
		sock.close()
'''

'''
Replace this code if receiveFile does not work
#check current directory for the file the user is requesting
#checkForExistingFile(fileName, sock)
print "%s" %fileName
						print(os.path.exists(fileName))
						if (os.path.exists(fileName) == True):
							print 'DUPLICATE FILE WARNING. "%s" ALREADY EXISTS.' %fileName
							break
						else:
							#get a file ready to receive data being sent back on data connection
							newFile = open(fileName, "w")
							newFile.write(clientMessage);
							newFile.close()


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

