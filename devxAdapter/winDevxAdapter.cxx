/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
#include <io.h>
#include <iostream.h>
#include "Winsock2.h"
#include <string>
#define sleep Sleep
using std::string;
#include "nameServCalls.h"

// for stat()
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

// for GetUserName()
#include <lmcons.h>


int const BUFFER_SIZE=1024;
int const MESSAGE_SIZE=(BUFFER_SIZE*2);

char devxMessage[MESSAGE_SIZE];

char const * const REGISTER_STRING = "register";
char const * const DONE_MESSAGE = "1 done\n";

int editorSocket=-1;
int devxError=0;

bool isConnected = false; // This determines whether an editor is currently connected
bool isShutdown = false; // This determines whether dislite should be stopped

// the location of the editor file and its base service name
string editorExecutable;
char * editorServiceName;
string argumentList;

// the address of the current editor
struct sockaddr_in editorAddress;


FILE * debugFile;
#ifdef XXX_getArgument
// getArgument is unused as of now
/* getArgument searches a char buffer for the next space.  argumentPtr tells
	us where to begin our search, and is placed at the beginning of the
	next string in the string */
char * getArgument(char ** argumentPtr) {
	char * placeHolder = *argumentPtr;
	while(!isspace(**argumentPtr)) { *argumentPtr++; }
	**argumentPtr ='\0';
	*argumentPtr++;
	return(placeHolder);
}
#endif
char const * const constructEditorName() {
	static char nameservEditorName[BUFFER_SIZE];
	char hostname[BUFFER_SIZE];
	char username[UNLEN + 1];
	unsigned long usernameLen = UNLEN + 1;

	// hostname
	gethostname(hostname, BUFFER_SIZE);
	// To remove fully qualified part ...
	int index=0;
	int hostnameLength = strlen(hostname);
	for(index=0; index < hostnameLength && hostname[index] != '.'; index++) {}
	if(index != hostnameLength) {
		hostname[index] = '\0';
	}

	// username
	GetUserName(username, &usernameLen);

	string editorName = hostname; 
	editorName += ":";
	editorName += username;
	editorName += ":";
	editorName += "Editor";
	editorName += ":";
	editorName += editorServiceName;
	if(editorName.size() > BUFFER_SIZE-1) {
		nameservEditorName[0] = '\0';
	} else {
		strcpy(nameservEditorName, editorName.c_str());
	}
	return(nameservEditorName);
}

int connectToEditor() {
	int returnValue = 0, result = 0;
	// ideally, we have no timeout?
	const int nTimeoutSec = 20;
	int timeout = 0;
	int tcp; int port;

	// we look up the service in nameserv
	char const * const nameservEditorName = constructEditorName();
	fprintf(debugFile, "the Editor's constructed name is %s\n", nameservEditorName);
	fflush(debugFile);
	if(NameServerGetService(nameservEditorName, port, tcp)!=1) {
		fprintf(debugFile, "Copying argumentList ...\n");
		fflush(debugFile);
		string commandLine = editorExecutable + " " + argumentList;
		char * tempArgumentList = new char[commandLine.size() + 1];
		strcpy(tempArgumentList, commandLine.c_str());
		fprintf(debugFile, "Done copying argumentList ...\n");
		fflush(debugFile);

		// First, we start up editor
		fprintf(debugFile, "Starting process ....\n");
		fprintf(debugFile, "Starting up %s, with arguments %s\n", editorExecutable.c_str(), tempArgumentList);
		fflush(debugFile);
		PROCESS_INFORMATION procInfo;
		STARTUPINFO startupInfo = { sizeof(STARTUPINFO), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL};
		returnValue = CreateProcess(NULL /*editorExecutable.c_str()*/, tempArgumentList, NULL, NULL, FALSE, CREATE_NO_WINDOW /* | DETACHED_PROCESS*/, NULL, NULL, &startupInfo, &procInfo);
		// What we create, we must destroy
		delete(tempArgumentList);
		if(returnValue != 0) {
	
			// Secondly, we look up the editor
			// Ideally, we will have no timeout
			// Until then, we wait 10 seconds
			returnValue = 0;
			while(returnValue != 1 && timeout < nTimeoutSec) {
				//result = findServer(nameservEditorName, &port, &tcp);
				result = NameServerGetService(nameservEditorName, port, tcp);
				if(result == 1) {
					returnValue = 1;
				} else {
					fprintf(debugFile, "Couldn't find %s ... retrying 10 seconds\n", nameservEditorName);
					fflush(debugFile);
					sleep(1000); // we have to wait until we connect
					timeout++;
					returnValue = 0;
				}
			}
	
			if(returnValue == 0) {
				fprintf(debugFile, "Couldn't connect\n");
				fflush(debugFile);
			}
		} else {
			fprintf(debugFile, "CreateProcess failed with error %d...\n", GetLastError());
			fflush(debugFile);
		}
  	} else {
		returnValue = 1;
    }

	// Third we connect to the editor
	if(returnValue == 1) {
		// get a socket
		editorSocket = socket(AF_INET, SOCK_STREAM, 0);
		memset(&editorAddress, 0, sizeof(editorAddress));
		//bzero(&editorAddress, sizeof(editorAddress));
		editorAddress.sin_family = AF_INET;
		editorAddress.sin_addr.s_addr = htonl(tcp);
		fprintf(debugFile, "trying to connect to %s\n", inet_ntoa(editorAddress.sin_addr));
		fflush(debugFile);
		editorAddress.sin_port = htons(port); // the port is stored in host byte order
		fprintf(debugFile, "Trying to connect ...\n");
		result = connect(editorSocket, (struct sockaddr *)&editorAddress, sizeof(editorAddress));
		if(result == 0) {
			fprintf(debugFile, "Connect to editor SUCCEEDED! ...\n");
			fflush(debugFile);
		} else {
			fprintf(debugFile, "Connect to editor FAILED...\n");
			fflush(debugFile);
		}
		returnValue = ( result == 0 ? 1 : 0 );
	}

	// now we register
	if(returnValue == 1) {
		// let's register
		fprintf(debugFile, "We try to register ...\n");
		fflush(debugFile);

		if(result = SendInteger(editorSocket, strlen(REGISTER_STRING)) > 0) {
			result = SendBuffer(editorSocket, REGISTER_STRING, strlen(REGISTER_STRING));
			if(result < 1) {
				returnValue = 0;
			}
		} else {
			returnValue = 0;
		}	
	}
	// if we've succesfully connected, returnValue will be 1, else 0
	if(returnValue == 0) {
		close(editorSocket);
		editorSocket = -1;
	}
	return(returnValue);
}

void readDislite() {
	static int result;
	//static char messageToEditor[MESSAGE_SIZE];
	fprintf(debugFile, "In readDislite\nString is %s\n", devxMessage);
	fflush(debugFile);
	//if(fgets(messageToEditor, MESSAGE_SIZE, stdin) > 0) {
	if(!strcmp(devxMessage, "close_driver\n")) {
		fprintf(debugFile, "Close driver was sent\n");
		fflush(debugFile);
		isShutdown = true;
		fputs(DONE_MESSAGE, stdout);
		fflush(stdout);
	} else {
		if(!isConnected) {
			fprintf(debugFile, "Going to connectToEditor()\n");
			fflush(debugFile);
			result = connectToEditor();
			if(result) {
				isConnected = true;
			}
		}
	}
	if(isConnected) {
		if(SendInteger(editorSocket, strlen(devxMessage)) > 0) {
			SendBuffer(editorSocket, devxMessage, strlen(devxMessage));
		}
	}
}

void readEditor() {
	static char messageToDevx[MESSAGE_SIZE];
	messageToDevx[0]='\0';

	int messageSize;
	if(ReceiveInteger(editorSocket, &messageSize) > 0) {
		if(messageSize+1 <= MESSAGE_SIZE) { // the +1 for the terminating 0
			ReceiveBuffer(editorSocket, messageToDevx, messageSize);
		}
		messageToDevx[messageSize] = '\0';
		if(messageToDevx[messageSize-1]!='\n') {
		   messageToDevx[messageSize]='\n';
		   messageToDevx[messageSize+1]=0;
        }
		fputs(messageToDevx, stdout);
		fflush(stdout);
	} else {
		isConnected = false;
		close(editorSocket);
		editorSocket = -1;
	}
}

void main(int argc, char * argv[]) {

	/* Let's get this *BULLSHIT* Windows network initialization
		crazy shit out of the way. */

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 
	err = WSAStartup( wVersionRequested, &wsaData );

	// End network initialization

	struct stat statBuffer;
	int statResult=0;

	debugFile = fopen("windevxadapter.log", "w");

	fprintf(debugFile, "I succesfully started\n");

	if(argc >= 3) {
		editorServiceName = argv[1];

		// We can't pass a path with forward slashes '/'
		//	or quotes "'s around it to CreateProcess
		char * tempEditorExecutable = new char[strlen(argv[2])+1];
		strcpy(tempEditorExecutable, argv[2]);
		for(int i=0; i<strlen(tempEditorExecutable); i++) {
			if(tempEditorExecutable[i] == '/') {
				tempEditorExecutable[i] = '\\';
			}
		}
		int bHasSpaces = strchr(tempEditorExecutable,' ')!=NULL;
		if(bHasSpaces) editorExecutable += "\"";
		editorExecutable += tempEditorExecutable;
		if(bHasSpaces) editorExecutable += "\"";
		// And a string for those functions that don't allow those quotes ...
		string noquotesEditorExecutable = tempEditorExecutable;
		delete(tempEditorExecutable);

		int index=3;
		while(index < argc) {
			bHasSpaces = strchr(argv[index],' ')!=NULL;
			if(bHasSpaces) argumentList += "\"";
			argumentList += argv[index];
			if(bHasSpaces) argumentList += "\"";
			argumentList += " "; // a space between args, right?
			index++;
		}
		//editorArguments = argv[3];

		statResult = stat(noquotesEditorExecutable.c_str(), &statBuffer);
		fprintf(debugFile, "Before stat, editorExecutable is %s\n", editorExecutable.c_str());
		fflush(debugFile);
		if(statResult != 0) {
			// find a way to inform dislite
			fprintf(debugFile, "Executable \"%s\"not found.\n", editorExecutable.c_str());
		}

		if(editorServiceName == 0) {
			// find a way to inform dislite
			fprintf(debugFile, "No servicename provided.\n");
		}
	} else {
		fprintf(debugFile, "Wrong arguments specified.\n Argc %d",argc);
		for(int i=0;i<argc;i++)
			fprintf(debugFile, "%s",argv[i]);
		fprintf(debugFile, "\n");
	}

	int stdinFD = fileno(stdin);	// standard in's file descriptor
	struct timeval const timeout = { 0, 100000 };
	// our socket for connection to the editor
	editorSocket = -1; // we clearly haven't connected yet

	//int foobar = 1;
	//while(foobar) { sleep(1); }
	int nResult=0;
	int DEBUG=0;
	while(!isShutdown && !devxError) {
		//fgets(devxMessage, MESSAGE_SIZE, stdin);
		if(filelength(stdinFD)>0) {
			// We have something to read
			fprintf(debugFile, "We have something to read\n");
			fflush(debugFile);
			if(fgets(devxMessage, MESSAGE_SIZE, stdin) != NULL) {
				// We have a messsage
				readDislite();
			}
		} else {
			/*
			fprintf(debugFile, "Nothing to read ...\n");
			fflush(debugFile);
			if(ferror(stdin)) {
				// Error or file stream has closed
				devxError = 1;
			}
			*/
		}

		if(editorSocket && !devxError) {
			// our variables for select()
			fd_set readFDSet;		// our read file descriptor set
			FD_ZERO(&readFDSet);
			FD_SET(editorSocket,&readFDSet);
		    if(select(NULL, &readFDSet, NULL, NULL, &timeout) > 0) // editor communication
				readEditor();
		}

		// Now we read from editor
		//if(editorSocket >=0 && something To Say) {
		//}
		sleep(100);
		/*
		DEBUG++;
		if(DEBUG > 500) {
			break;
		}
		*/
		if(devxError) {
			fprintf(debugFile, "devxError is %d\n", devxError);
			fflush(debugFile);
		}
	}
	fprintf(debugFile, "devxError is %d\n", devxError);
	fflush(debugFile);
	fclose(debugFile);
}
