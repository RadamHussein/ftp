'''
ftclient.py

CS372 - Project 2
Adam Smith
Winter 2017

This program implements the ftp protocol. This is the client side
which initiates contact with the server via a control connection. 
It then also acts as a server when receiving data through the data
connection. This program allows the user to either request a listing
of the files in the directory the server is running on, or request a 
specific file. This program accepts either 4 or 5 arguments. 
To request a directory list run with:
"python ftclient.py <server hostname> <port number> -l <data port>".
To request a file, run with:
"python ftclient.py <server hostname> <port number> -g <filename> <data port>"
'''

import socket
import sys
import os.path

'''
This function is used for binding the socket to the port number.
It takes a socket object as an argument. 
'''
def BindSocket(serverSocket, port):
	try:
		serverSocket.bind(('', port))
	except socket.error as msg:
		print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
		sys.exit()

'''
This function starts the server listening on the specified port
It takes a socket object as an argument.
'''
def ListenSocket(serverSocket):
	serverSocket.listen(1)
	#print 'Data port open on port #: %d' %clientServerPort

'''
This function is used to receive messages from the client
It takes a connection object as an argument. 
'''
def ReceiveMessage(connectionSocket):
	message = connectionSocket.recv(80000)
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

'''
This function is for sending a command and port number 
through the control connection to the server.
'''
def SendListCommand(sock, message):
	try:
	    # Send data
	    sock.sendall(message)
	finally:
		sock.close()

'''
This function is for sending a Get File command
through the control connection to the server
'''
def SendGetFileCommand(sock, message):
	try:
    	# Send data
	    sock.sendall(message)	
	finally:
		sock.close()

'''
this function is used to establish a connection with a client
It takes a socket object as an argument
'''
def ConnectToClient(socket):
	connectionSocket, addr = socket.accept()
	#print 'Connected with ' + addr[0] + ':' + str(addr[1])
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

'''
This function is used to handle a received file. 
It checks to see if the file requested already
exists. If so it prints a warning and does not
write the requested data to a file. If the file
does not exist, it writes the requested data to
a file.
'''
def receiveFile(fileName, message):
	#print(os.path.exists(fileName))
	if (os.path.exists(fileName) == True):
		print 'DUPLICATE FILE WARNING. "%s" ALREADY EXISTS.' %fileName
	else:
		#get a file ready to receive data being sent back on data connection
		newFile = open(fileName, "w")
		newFile.write(message);
		newFile.close()
		print 'TRANSFER COMPLETE'

'''
This function handles communication with the server
through the control connection. It sends the initial
message and receives the response. It then checks the
response for a '1', meaning that the server verified
the input (commands, porst or requested files). Else
the error message sent back from the server is printed
and the program exits. 
'''
def makeRequest(sock, message):
	try:
		sock.sendall(message)
			 	
		response = sock.recv(500)
		if response != '1\n':
			print '%s' %response
			sock.close()
			exit()			
	finally:
		sock.close()

'''
This function sets up the client socket and 
initiates contact with the server.
'''
def initiateContact(hostName, portNumber):
	# Create a TCP/IP socket
	clientsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	# Connect the socket to the port where the server is listening
	server_address = (hostName, portNumber)
	print >>sys.stderr, 'connecting to %s port %s' % server_address
	print '\n'
	clientsock.connect(server_address)

	return clientsock

'''
This function starts up the server socket for the
data port connection. It starts the socket listening
on the port specified by the user.
'''
def openDataConnection(dataPort):
	#create a socket for data port
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	#bind the socket for data port
	BindSocket(serverSocket, clientServerPort)

	#Start listening on socket for data port
	ListenSocket(serverSocket)

	return serverSocket

'''
This function handles the communication between 
the server and client over the data connections.
It takes in the serverSocket. It calls the 
ConnectToClient, receiveMessage and receiveFile 
functions.
'''
def handleDataConnection(serverSocket):
	try:
		while True:
			#connect to through data port
			connection = ConnectToClient(serverSocket)
			
			try:
				while True:
					#receive message from client
					clientMessage = ReceiveMessage(connection)

					#if a message is received
					if clientMessage:
						if (len(sys.argv) == 6):	
							#if a file was requested, receive the file
							receiveFile(fileName, clientMessage)					
							break
						else:
							#else, list was requested. Display the directory contents
							print '%s' %clientMessage
							break
					else:
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

 

#get arguments into variables
hostName = sys.argv[1]
portNumber = int(sys.argv[2])

#make contact with the server
sock = initiateContact(hostName, portNumber)

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

	makeRequest(sock, message)

elif len(sys.argv) == 6:
	#get arguments into variables
	hostName = sys.argv[1]
	portNumber = int(sys.argv[2])
	command = sys.argv[3]
	fileName = sys.argv[4]
	dataPort = sys.argv[5]

	#get the name of the client server to send for the data port connection
	clientHost = socket.gethostname()

	#concantenate arguments into message string
	message = command + ':' + fileName + ':' + dataPort + ':' + clientHost + ':' 

	#send request to the server
	makeRequest(sock, message)

else:
	print "IVALID ARGUMENTS FORMAT... <SERVER_HOST> <SERVER_PORT> <COMMAND> <[FILENAME]> <DATAPORT>"
	sock.close()
	exit()

#DATA PORT CODE

#convert the data port to an int to open the data port listening socket
clientServerPort = int(dataPort)

serverSocket = openDataConnection(clientServerPort)
handleDataConnection(serverSocket)
serverSocket.close()

