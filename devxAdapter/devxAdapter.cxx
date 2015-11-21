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
// for string
#if defined HP_UX || hp10 || irix6
#include <iostream.h>
#include <string>
	#ifdef irix6
	using std::string;
	#endif
#else
#include <iostream>
using std::string;
#endif

#include <stdio.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>

// for bzero()
#include <strings.h>

// for socket()
#include <sys/types.h>
#include <sys/socket.h>

// for isspace()
#include <ctype.h>

// for stat()
#include <sys/types.h>
#include <sys/stat.h>

// for open()
#include <fcntl.h>


// for sockaddr and htons
#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>
extern int errno;

// the real nameserv library
#include "nameServCalls.h"

int const BUFFER_SIZE=1024;
int const MESSAGE_SIZE=(BUFFER_SIZE*2);

char const * const REGISTER_STRING = "register";
char const * const DONE_MESSAGE = "1 done\n";

int editorSocket=-1;
int	do_debug=0;
FILE* 	log_fd=stderr;


bool isConnected = false; // This determines whether an editor is currently connected
bool isShutdown = false; // This determines whether dislite should be stopped

// the location of the editor file and its base service name
char * editorExecutable;
char * editorServiceName;
char ** editorArguments;

// the address of the current editor
struct sockaddr_in editorAddress;

char const * const constructEditorName() {
	static char nameservEditorName[BUFFER_SIZE];
	static char hostname[BUFFER_SIZE];
	char * username;
	gethostname(hostname, BUFFER_SIZE);
	username = getenv("USER");
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
	int timeout = 0;

	// first we execute the editor
	if(fork()== 0) { // the child
		// turn off the stdout - we don't want to interfere with our parent's parent, which
		//	happens to be listening on stdin
		close(fileno(stdout));
	 	int devNull = open("/dev/null", O_WRONLY);		
		int newStdout = dup(devNull);
		if(execv(editorExecutable, editorArguments) == -1) {
			exit(1);
		}
	} else {
		// second we look up the service in nameserv
		char const * const nameservEditorName = constructEditorName();

		int tcp; int port;

		// ideally, we have no timeout?
		while(returnValue != 1 && timeout < 60) {
			//result = findServer(nameservEditorName, &port, &tcp);
			result = NameServerGetService(nameservEditorName, port, tcp);
			if(result == 1) {
				returnValue = 1;
			} else {
				sleep(1); // we have to wait until we connect
				timeout++;
				returnValue = 0;
			}
		}


		// third we connect
		if(returnValue == 1) {
			// get a socket
			editorSocket = socket(AF_INET, SOCK_STREAM, 0);
			bzero(&editorAddress, sizeof(editorAddress));
			editorAddress.sin_family = AF_INET;
			editorAddress.sin_addr.s_addr = htonl(tcp);
			editorAddress.sin_port = htons(port); // the port is stored in host byte order
			result = connect(editorSocket, (struct sockaddr *)&editorAddress, sizeof(editorAddress));
			returnValue = ( result == 0 ? 1 : 0 );
		}

		// now we register
		if(returnValue == 1) {
			// let's register
			if(!SendString(editorSocket, REGISTER_STRING)) {
				returnValue = 0;
			}	
		}

		// if we've succesfully connected, returnValue will be 1, else 0
		if(returnValue == 0) {
			close(editorSocket);
			editorSocket = -1;
		}
		return returnValue;
	}
}

void readDislite() {
        static int result;
	static char messageToEditor[MESSAGE_SIZE];

	if(fgets(messageToEditor, MESSAGE_SIZE, stdin) > 0) {
	        if(!strcmp(messageToEditor,"close_driver\n")) {
		        isShutdown = true;
		        fputs(DONE_MESSAGE, stdout);
			fflush(stdout);		        
	        } else {
		        if(!isConnected) {
			        result = connectToEditor();
				if(result)   isConnected = true;
			}
		}
		if(isConnected) {
                    SendString(editorSocket, messageToEditor);
		}
	} else {
		exit(0);
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

int main(int argc, char * argv[]) {
	struct stat statBuffer;
	int statResult;

	if(argc >= 3) {
		editorServiceName = argv[1];
		editorExecutable = argv[2];
		editorArguments = &argv[2];

		statResult = stat(editorExecutable, &statBuffer);
		if(statResult != 0) {
			// find a way to inform dislite
		}

		if(editorExecutable == 0 || editorServiceName == 0) {
			// find a way to inform dislite
		}
	} else {
		// find a way to inform dislite
	}

	// our variables for select()
	fd_set readFDSet;		// our read file descriptor set
	int maxFDs;			// our max file descriptors
	int stdinFD = fileno(stdin);	// standard in's file descriptor
	// our socket for connection to the editor
	editorSocket = -1; // we clearly haven't connected yet

	//int foobar = 1;
	//while(foobar) { sleep(1); }

	while(!isShutdown) {
		maxFDs = (stdinFD > editorSocket) ? stdinFD : editorSocket;
		maxFDs++; // always plus one

		// we presume that passing an unconnected socket to select will not cause it to return
		//  the socket as marked for being ready to be read
		FD_ZERO(&readFDSet);		// zero the set
		FD_SET(stdinFD, &readFDSet);
		if(editorSocket >= 0) {
			FD_SET(editorSocket, &readFDSet);
		}

		if(select(maxFDs, &readFDSet, NULL, NULL, NULL) != -1) {
			// this block needs to be first, because readDislite can
			//	change the state of editorSocket
			if(editorSocket >= 0 && FD_ISSET(editorSocket, &readFDSet)) {
				readEditor();
			}
			if(FD_ISSET(stdinFD, &readFDSet)) {
				// we need to pass the socket in case we need to connect
				//  to the editor
				readDislite();
			}
		}
	}
}
