#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

void sigint_handler(int sig)
{
	printf("\n");
    printf("GOODBYE!\n");
    exit(0);
}

/**********************************************
* This function takes the socket file descriptor and 
* the message buffer. It recieves the message sent from
* the client and stores it in the buffer variable. 
***********************************************/
void recieveMessage(int sockFD, char *buffer){
	int charsRead;
	charsRead = recv(sockFD, buffer, 255, 0); // Read the client's message from the socket
		
	if (charsRead < 0) {
		printf("ERROR reading from socket\n");
	}
	printf("SERVER: I received this from the client: \"%s\"\n", buffer);
}

/************************************************
* This functtion extracts a portion of the message 
* recieved from the client up to the first : character.
*************************************************/
char *parseControlConnectionMessage(char *message){
	const char delim[2] = ":";
	char *command;

	command = strtok(message, delim);
	printf("Command received: %s\n", command);

	return command;
}

/*******************************************
* This function is used to parse more tokens out of
* a string after a call to strtok.
********************************************/
void parseToken(char **token, char *delim){
	*token = strtok(NULL, delim);
	printf("next token: %s\n", *token);
}

/********************************************
* This function uses strtok to parse the arguments
* out of the string sent by the client. It is 
* called when the client has sent a -l command.
*********************************************/
void parseGetListTokens(char *message, int *portNumber, char **host){
	printf("after parsing command, message contains: %s\n", message);
	char *delim = ":";
	char *token;

	//get the command out of the string
	token = strtok(message, delim);
	printf("first token: %s\n", token);

	parseToken(&token, delim);

	//convert port number string to integer
	*portNumber = atoi(token);

	//get hostname into a variable
	parseToken(&token, delim);
	*host = token;
}

/********************************************
* This function uses strtok to parse the arguments
* out of the string sent by the client. It is 
* called when the client has sent a -g command.
*********************************************/
void parseGetFileTokens(char *message, int *portNumber, char **file, char **host){
	printf("after parsing command, message contains: %s\n", message);
	char *delim = ":";
	char *token;

	//get the command out of the string
	token = strtok(message, delim);
	printf("first token: %s\n", token);

	//get data port number into a variable
	parseToken(&token, delim);

	//convert port number string to integer
	*portNumber = atoi(token);

	//get file name into a variable
	parseToken(&token, delim);
	*file = token;

	//get hostname into a variable
	parseToken(&token, delim);
	*host = token;
}

/***********************************************
* This function checks the command string against
* accepted strings. It returns 1 if a match is found 
* and 0 if no match is found.
************************************************/
int isValidCommand(char *command){
	int isEqual;

	isEqual = strcmp(command, "-l");
	if (isEqual == 0){
		return 1;
	}
	else{
		isEqual = strcmp(command, "-g");
	}
	
	if (isEqual == 0){
		return 2;
	}
	else
		return 0;
}


//DELETE THIS FUNCTION LATER
/***********************************************
* This function takes in an integer and a pointer to 
* a response string. It determines the type of message
* that will be assigned to the repsonse string, then 
* returns the length of the response. 
***********************************************/
void determineResponseType(int commandType, char **response){

	//determine the type of message to respond with 
	if (commandType == 1){
		*response = "Valid command recieved\n";
	}
	else{
		*response = "INVALID COMMAND\n";
	}
}


/*
* This function handles sending messages to the server
* from the client. It returns the number of chars
* written into the connection to check for completion.
*/
int sendMessage(int *sockFD, int charsWritten, char *message){
	charsWritten = send(*sockFD, message, strlen(message), 0); // Write to the server
	if (charsWritten < 0) {
		fprintf(stderr, "CLIENT ERROR writing to socket\n");
	}

	return charsWritten;
}

int main(int argc, char *argv[])
{
	int listenSocketFD;
	int establishedConnectionFD; 
	int portNumber;
	int charsRead;
	int validCommandRecieved;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	char temp[256];										//string to hold copy of buffer
	char *response;
	char *controlConnectionCommand;
	char *filename;										//string holds the name of a requested file
	char *dataPortHost;									//string to hold the name of the client's server data port
	int dataPortNumber;									//holds the port number of the client's server data port
	struct sockaddr_in serverAddress, clientAddress;
	void sigint_handler(int sig); /* prototype */
	struct sigaction sa;

	sa.sa_handler = sigint_handler;
    sa.sa_flags = 0; // or SA_RESTART
    sigemptyset(&sa.sa_mask);


	if (argc < 2) { 
		fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1);
	} // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) {
		printf("ERROR opening socket\n");
	}

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
		printf("ERROR on binding\n");
		exit(1);
	} 
	else {
		listen(listenSocketFD, 2); // Flip the socket on - it can now receive up to 2 connections
		printf("Server listening on port %d\n", portNumber);
	}

	//listen for signals
	if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        //close(listenSocketFD);
        exit(1);
    }
	
	while(1){
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {
			printf("ERROR on accept\n");
		}

		// Get the message from the client
		recieveMessage(establishedConnectionFD, buffer);
		
		//copy buffer into temp
		strcpy(temp, buffer);

		//check the command sent through the control connection
		controlConnectionCommand = parseControlConnectionMessage(temp);
		validCommandRecieved = isValidCommand(controlConnectionCommand);

		//determine the type of response based on the request
		if (validCommandRecieved == 1){
			//parse out the other arguments from the message
			parseGetListTokens(buffer, &dataPortNumber, &dataPortHost);
			response = "Valid command recieved\n";
		}
		else if (validCommandRecieved == 2){
			//parse out the other arguments from the message
			parseGetFileTokens(buffer, &dataPortNumber, &filename, &dataPortHost);
			response = "Valid command recieved\n";
		}
		else{
			response = "INVALID COMMAND\n";
		}

		//send response back to client
		charsRead = sendMessage(&establishedConnectionFD, charsRead, response);

		//check for completion
		if (charsRead < 0) {
			printf("ERROR writing to socket\n");
		}
		
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	}

	close(listenSocketFD); // Close the listening socket
	

return 0;
}
