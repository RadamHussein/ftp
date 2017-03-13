/****************************************************************
* ftserver.c
* 
* CS372 - Project 2
* Adam Smith
* Winter 2017
*
* This program implements the ftp protocol. This is the server side. 
* It waits for a connection request from a client which will establish
* a control connection. The client will then send a command to the 
* server requesting either a listing of files in the current directory,
* or a specific file. The server will verify the parameters of the 
* request and check for the file in the directory. If there is an error
* with the request, an error message will be sent back to the client. 
* If everything is okay, an "okay" message will be sent back to the client
* though the control connection and a data connection will be initiated by the 
* server. The requested data will be sent through the data connection which 
* will be terminated by the client. The server will run until terminated by 
* the administrator. 
* To compile: gcc ftserver.c -o ftserver
* To run: ./ftserver <port number>
*****************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <dirent.h> 

//signal handler function
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
}

/************************************************
* This functtion extracts a portion of the message 
* recieved from the client up to the first : character.
*************************************************/
char *parseControlConnectionMessage(char *message){
	const char delim[2] = ":";
	char *command;

	command = strtok(message, delim);

	return command;
}

/*******************************************
* This function is used to parse more tokens out of
* a string after a call to strtok.
********************************************/
void parseToken(char **token, char *delim){
	*token = strtok(NULL, delim);
}

/********************************************
* This function uses strtok to parse the arguments
* out of the string sent by the client. It is 
* called when the client has sent a -l command.
*********************************************/
void parseGetListTokens(char *message, int *portNumber, char **host){
	//printf("after parsing command, message contains: %s\n", message);
	char *delim = ":";
	char *token;

	//get the command out of the string
	token = strtok(message, delim);

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
void parseGetFileTokens(char *message, char **file, int *portNumber, char **host){
	char *delim = ":";
	char *token;

	//get the command out of the string
	token = strtok(message, delim);

	//get file name into a variable
	parseToken(&token, delim);
	*file = token;

	//get data port number into a variable
	parseToken(&token, delim);

	//convert port number string to integer
	*portNumber = atoi(token);

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

/*
* This function checks that the entire
* message was sent by comparing the charsWritten
* variable to the message length.
*/
void checkForCompletion(int charsWritten, char *message){
	if (charsWritten < strlen(message)) {
			fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
		}
}

/*
* This function sets up and opens
* the client socket.
*/
void setUpSocket(int *sockFD){
	// Set up the socket
	*sockFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (sockFD < 0) {
		fprintf(stderr, "CLIENT: ERROR opening socket\n");
	}
}

/*
* This function connects the socket to the server
* address struct.
*/
void connectToServer(int *sockFD, struct sockaddr_in serverAddress){
	if (connect(*sockFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to address
		fprintf(stderr, "CLIENT: ERROR connectiong\n");
	}
}

void sendRequestThroughDataConnection(char **dataPortHost, int dataPortNumber, char *dataBuffer){
	//data connection structs
	struct sockaddr_in dataPortAddress;
	struct hostent* serverHostInfo;

	int dataSocketFD;
	int charsWritten;

	// Set up the server address struct
	memset((char*)&dataPortAddress, '\0', sizeof(dataPortAddress)); 	// Clear out the address struct
	dataPortAddress.sin_family = AF_INET; 							// Create a network-capable socket
	dataPortAddress.sin_port = htons(dataPortNumber); 				// Store the port number
	serverHostInfo = gethostbyname(*dataPortHost);						            //ip is localhost - WILL HAVE TO CHANGE!
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
	}
	memcpy((char*)&dataPortAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	//set up data port socket
	setUpSocket(&dataSocketFD);

	// Connect to client through data port
	connectToServer(&dataSocketFD, dataPortAddress);

	//send message to server
	charsWritten = sendMessage(&dataSocketFD, charsWritten, dataBuffer);

	//check that charsWritten == message length
	checkForCompletion(charsWritten, dataBuffer);

	close(dataSocketFD);	//close data connection
}

void handleRequest(char *buffer, int establishedConnectionFD){
	char temp[256];
	char filepath[100];
	char cwd[100];
	char *controlConnectionCommand;
	char *response;
	char *dataPortHost;
	char *filename;
	int dataPortNumber;
	int validCommandRecieved;
	int charsRead;
	int textFileSize;
	int responseLength;
	char *responseLengthString;
	FILE *textFP;

	//set up directory struct
	DIR *d;
	struct dirent *dir;
	d = opendir(".");

	//copy buffer into temp
	strcpy(temp, buffer);

	//check the command sent through the control connection
	controlConnectionCommand = parseControlConnectionMessage(temp);
	validCommandRecieved = isValidCommand(controlConnectionCommand);

		//determine the type of response based on the request
		if (validCommandRecieved == 1){							//valid -l command
			//parse out the other arguments from the message
			parseGetListTokens(buffer, &dataPortNumber, &dataPortHost);
			response = "1\n";

			//send response back to client
			charsRead = sendMessage(&establishedConnectionFD, charsRead, response);

			//check for completion
			if (charsRead < 0) {
				printf("ERROR writing to socket\n");
			}

			char listItem[256];							//will hold the current list item
			char tempBuffer[256];						//will hold the last contents of dataBuffer
			char dataBuffer[256];						//will hold final contents to send to client

			//get contents of directory
			if (d){
				while ((dir = readdir(d)) != NULL){
					//printf("%s\n", dir->d_name);
					memset(listItem, '\0', sizeof(listItem));
					memset(tempBuffer, '\0', sizeof(tempBuffer));
					memcpy(tempBuffer, dataBuffer, strlen(dataBuffer)+1);
					memcpy(listItem, dir->d_name, strlen(dir->d_name)+1);
					sprintf(dataBuffer, "%s\n%s", tempBuffer, listItem);
				}
				closedir(d);
			}
			responseLength = strlen(dataBuffer);

			sendRequestThroughDataConnection(&dataPortHost, dataPortNumber, dataBuffer);
		}
		else if (validCommandRecieved == 2){					//valid -g command
			//parse out the other arguments from the message
			parseGetFileTokens(buffer, &filename, &dataPortNumber, &dataPortHost);

			//get current working directory
			getcwd(cwd, sizeof(cwd));

			//concantenate filename and filepath
			sprintf(filepath, ".%s/%s", cwd, filename);

			//open file for reading
			textFP = fopen(filename, "r");

			if (textFP != NULL){
				response = "1\n";	//okay response
				//send response back to client
				charsRead = sendMessage(&establishedConnectionFD, charsRead, response);

				//check for completion
				if (charsRead < 0) {
					printf("ERROR writing to socket\n");
					//break; MAYBE REPLACE WITH A RETURN STATEMENT
				}

				//get file size
				fseek(textFP, 0, SEEK_END);
				textFileSize = ftell(textFP);
				fseek(textFP, 0, SEEK_SET);

				//create an array large enough to hold the file and fill it with 0's
				char dataBuffer[textFileSize];
				memset(dataBuffer, 0, sizeof(dataBuffer));

				//read the file into the the buffer array
				fread(dataBuffer, sizeof(dataBuffer), 1, textFP);

				fclose(textFP);

				//send requested file to client
				sendRequestThroughDataConnection(&dataPortHost, dataPortNumber, dataBuffer);
			}
			else{
				response = "FILE NOT FOUND\n";
				//send response back to client
				charsRead = sendMessage(&establishedConnectionFD, charsRead, response);

				//check for completion
				if (charsRead < 0) {
					printf("ERROR writing to socket\n");
				}
			}			
		}	
		else{													//invalid command
			response = "INVALID COMMAND\n";
		}

		//send response back to client
		charsRead = sendMessage(&establishedConnectionFD, charsRead, response);

		//check for completion
		if (charsRead < 0) {
			printf("ERROR writing to socket\n");
		}
}

/********************************************************
* This function binds the socket and starts it listening 
* for connection requests. It takes in a pointer to the 
* listenFD integer, the socket struct and the port number
* to listen on. 
********************************************************/
void startup(int *listenSocketFD, struct sockaddr_in serverAddress, int portNumber){
	// Set up the socket
	*listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (*listenSocketFD < 0) {
		printf("ERROR opening socket\n");
	}

	// Enable the socket to begin listening
	if (bind(*listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
		printf("ERROR on binding\n");
		exit(1);
	} 
	else {
		listen(*listenSocketFD, 2); // Flip the socket on - it can now receive up to 2 connections
		printf("Server listening on port %d\n", portNumber);
	}
}

int main(int argc, char *argv[])
{
	int listenSocketFD;									//control connection socket
	int dataSocketFD;									//data connection socket
	int establishedConnectionFD; 						//connection socket
	int portNumber;										//control connection port number
	int charsWritten;									//number of chars written in message
	int textFileSize;									//size of the open text file
	socklen_t sizeOfClientInfo;
	char buffer[256];									//
	struct sockaddr_in serverAddress, clientAddress;	//control connection structs

	//data connection structs
	struct sockaddr_in dataPortAddress;
	struct hostent* serverHostInfo;

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

	//start the server listening
	startup(&listenSocketFD, serverAddress, portNumber);

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

		//do something with the request
		handleRequest(buffer, establishedConnectionFD);
	}

return 0;
}
