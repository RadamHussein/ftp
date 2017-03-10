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
void parseGetFileTokens(char *message, char **file, int *portNumber, char **host){
	printf("after parsing command, message contains: %s\n", message);
	char *delim = ":";
	char *token;

	//get the command out of the string
	token = strtok(message, delim);
	printf("first token: %s\n", token);

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

int main(int argc, char *argv[])
{
	int listenSocketFD;
	int dataSocketFD;
	int establishedConnectionFD; 
	int portNumber;
	int charsRead;
	int charsWritten;
	int validCommandRecieved;
	int textFileSize;						//size of the open text file
	FILE *textFP;							//pointer to text file
	socklen_t sizeOfClientInfo;
	char buffer[256];
	char temp[256];										//string to hold copy of buffer
	char dataBuffer[256];						//string to hold response sent through the data connection
	char cwd[100];										//holds the current working directory
	char filepath[100];
	char *response;
	char *controlConnectionCommand;
	char *filename;										//string holds the name of a requested file
	char *dataPortHost;									//string to hold the name of the client's server data port
	int dataPortNumber;									//holds the port number of the client's server data port
	struct sockaddr_in serverAddress, clientAddress;	//control connection structs

	//data connection structs
	struct sockaddr_in dataPortAddress;
	struct hostent* serverHostInfo;

	//set up directory struct
	DIR *d;
	struct dirent *dir;
	d = opendir(".");

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
		if (validCommandRecieved == 1){							//valid -l command
			//parse out the other arguments from the message
			parseGetListTokens(buffer, &dataPortNumber, &dataPortHost);
			printf("data port is: %d\n", dataPortNumber);
			response = "1\n";

			char listItem[256];							//will hold the current list item
			char tempBuffer[256];						//will hold the last contents of dataBuffer
			//get contents of directory
			if (d){
				while ((dir = readdir(d)) != NULL){
					printf("%s\n", dir->d_name);
					memset(listItem, '\0', sizeof(listItem));
					memset(tempBuffer, '\0', sizeof(tempBuffer));
					memcpy(tempBuffer, dataBuffer, strlen(dataBuffer)+1);
					memcpy(listItem, dir->d_name, strlen(dir->d_name)+1);
					sprintf(dataBuffer, "%s\n%s", tempBuffer, listItem);
				}
				closedir(d);
			}
		}
		else if (validCommandRecieved == 2){					//valid -g command
			//parse out the other arguments from the message
			parseGetFileTokens(buffer, &filename, &dataPortNumber, &dataPortHost);
			response = "1\n";				//okay response

			getcwd(cwd, sizeof(cwd));
			printf("current working directory is: %s\n", cwd);

			sprintf(filepath, ".%s/%s", cwd, filename);

			printf("full file path: %s\n", filepath);

			//open file for reading
			textFP = fopen(filename, "r");

			//check that the file opened or exists
			if (textFP == NULL){
				response = "FILE NOT FOUND\n";
			}
			else{
				//get file size
				//fseek(textFP, 0, SEEK_END);
				//textFileSize = ftell(textFP);
				//fseek(textFP, 0, SEEK_SET);

				//create an array large enough to hold the file and fill it with 0's
				//char dataBuffer[textFileSize];
				memset(dataBuffer, 0, sizeof(dataBuffer));

				//read the file into the the buffer array
				fread(dataBuffer, sizeof(dataBuffer), 1, textFP);

				fclose(textFP);
				response = "1\n";	//okay response
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
		else{
			break;
		}
	}

	close(establishedConnectionFD); // Close the existing socket which is connected to the client

	//wait a moment for the client to begin listening on the data port
	//sleep(10);

	// Set up the server address struct
	memset((char*)&dataPortAddress, '\0', sizeof(dataPortAddress)); 	// Clear out the address struct
	dataPortAddress.sin_family = AF_INET; 							// Create a network-capable socket
	dataPortAddress.sin_port = htons(dataPortNumber); 				// Store the port number
	serverHostInfo = gethostbyname(dataPortHost);						            //ip is localhost - WILL HAVE TO CHANGE!
	//serverHostInfo = gethostbyname("localhost");						            //ip is localhost - WILL HAVE TO CHANGE!
	if (serverHostInfo == NULL) { 
		//DO SOMETHING HERE IF CONNECTION FAILS?
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
	}
	memcpy((char*)&dataPortAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	//set up data port socket
	setUpSocket(&dataSocketFD);

	printf("connecting to data port...\n");

	// Connect to client through data port
	connectToServer(&dataSocketFD, dataPortAddress);

	//send message to server
	charsWritten = sendMessage(&dataSocketFD, charsWritten, dataBuffer);

	//check that charsWritten == message length
	checkForCompletion(charsWritten, response);

	close(dataSocketFD);	//close data connection

	//close(listenSocketFD); // Close the listening socket
	

return 0;
}
