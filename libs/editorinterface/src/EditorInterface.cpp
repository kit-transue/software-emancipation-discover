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
// Notes:
//// We should provide some sort of error message mechanism


#if defined HP_UX || hp10 || irix6
#include <iostream.h>
	#ifdef irix6
	// for select()
	#include <unistd.h>
	#endif
#else
#include <iostream>
using std::string; using std::cout;
using std::endl;
#endif

#include "EditorInterface.h"

#ifdef _WIN32
#include <ws2tcpip.h> // for socklen_t
#else
// for socket()
#include <sys/types.h>
#include <sys/socket.h>

// for sockaddr_in
#include <arpa/inet.h>

// for htons()
#include <netinet/in.h>
#endif // _WIN32


#include "nameServCalls.h"

#ifndef WIN32
int const SOCKET_ERROR=-1;
#endif


///////////////////////////////////////////////////////////////////////
// Member functions
///////////////////////////////////////////////////////////////////////
// Constructor
EditorInterface::EditorInterface() {
#ifdef WIN32
		// Initialize networking for Windows
		WSAData WindowsSockets;
		if (WSAStartup (MAKEWORD(1,1), &WindowsSockets) != 0) {
			cout << "WSAStartup failed!" << endl;
			// Do something here - an exception?
		}
#endif
       socketListen=socketListenPort=0;
       socketMUA=socketMUAPort=0;
       socketEditor=socketEditorPort = 0;
       
       connectedMUA = false;
}

int EditorInterface::sendString(int socket, char const * string, int strlen) {
		int returnValue = SendBuffer(socket, string, strlen);
		return(returnValue);
}

int EditorInterface::sendInteger(int socket, int integer) {
		int returnValue = SendInteger(socket, integer);
		return(returnValue);
}

// pass a char * that has enough to accomadate strlen + 1
int EditorInterface::readString(int socket, char * charBuff, int strlen) {
	int returnValue = ReceiveBuffer(socket, charBuff, strlen);
	if(returnValue == 0) {
		*charBuff='\0';
	} else {
		charBuff[strlen] = '\0';
	}
	return(returnValue);
}

int EditorInterface::readInteger(int socket) {
	int value;
	int returnValue = ReceiveInteger(socket, &value);
	if(returnValue > 0) { returnValue = value; }
	return(returnValue);
}

string EditorInterface::readMessage(int socket) {
	int returnValue = 0;
	char * receivedString;
	string returnString;

	int stringSize=readInteger(socket);
	if(stringSize > 0) {
		receivedString = new char[stringSize+1]; // for the ending '\0'
		int result = readString(socket, receivedString, stringSize);
		if(result == 0) {
			returnValue = 0;
		} else {
			returnValue = 1;
			returnString = receivedString;	
		}
	}
	if(returnValue == 0) {
		returnString = "";
	}
	
	return(returnString);
}

int EditorInterface::createListener(void) {
		// let's get a socket up and listening
		int returnValue=1;
		struct sockaddr_in listenAddress;

		socketListen = socket(AF_INET, SOCK_STREAM, 0);  // get socket

		char nullOption=1;
		if(setsockopt(socketListen, SOL_SOCKET, SO_REUSEADDR, &nullOption, sizeof(int)) != 0) {
			returnValue = 0;
		}
		
		listenAddress.sin_family = AF_INET;
		listenAddress.sin_port = htons(0);
		listenAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		memset(&listenAddress.sin_zero, 0, 8);

		if(bind(socketListen, (struct sockaddr *)&listenAddress, sizeof(struct sockaddr)) != 0) {
			returnValue = 0;
		}

		if(returnValue && listen(socketListen, 5) != 0) {
			returnValue = 0;
		}

		// we must use getsockname, because apparently this shit is different on Windows
		// and some other UNIX platforms.  That's right - "SHIT".
		//socketListenPort = listenAddress.sin_port;
		sockaddr_in tempAddress;
		socklen_t tempAddressSize = sizeof(sockaddr_in);

		if(returnValue && getsockname(socketListen, (sockaddr *)&tempAddress, &tempAddressSize) != 0) {
			returnValue = 0;
		}

		socketListenPort = htons(tempAddress.sin_port);

		printf("successfully am listening\n");

		return(returnValue);
}

// Here, we register our interface with nameserv 
int EditorInterface::registerNameservInterface(void) {
	int returnValue;
	string nameservEditorName = getEditorName();
	//char nameCharArray[nameServEditorName.size()] = nameservEditorName.c_str();
	returnValue = NameServerRegisterService(nameservEditorName.c_str(), socketListenPort);
	// -1 on error, 1 on success
	return(returnValue);
}

int EditorInterface::unregisterNameservInterface(void) {
	int returnValue;
	string nameservEditorName = getEditorName();
	returnValue = NameServerUnRegisterService(nameservEditorName.c_str());
	return(returnValue);
}

int EditorInterface::commandLoop(void) {
	int returnValue = 1;
	fd_set socketSet;
	struct sockaddr_in address;

	printf("in commandLoop\n");

	connectedMUA = false; // We're not connected at the beginning of commandLoop

	while(returnValue) {
		if(!connectedMUA) {
			// we wait until a connection has been made

			//cout << "Waiting for a connection to be made" << endl;
			printf("Waiting for a connection to be made\n");

			FD_ZERO(&socketSet);
			FD_SET(socketListen, &socketSet);
			select(0, &socketSet, 0, 0, NULL);
			socklen_t sizeOf = sizeof(address);

			socketMUA = accept(socketListen, (struct sockaddr *)&address, &sizeOf);
			socketMUAPort = ntohs(address.sin_port);

			if(socketMUA < 1) {
				returnValue = 0;
			} else {
				connectedMUA=true;
				returnValue = 1;
			}
		} else {
			returnValue = processCommand();
		}
	}
	return(returnValue);
}


// processCommand is to be used when you know something is coming down socketMUA.
//	select's should be done beforehand
int EditorInterface::processCommand(void) {
	int size;
	// read incoming command
	size = readInteger(socketMUA);			
	if(size <= 0) { // indicating the connection has closed or an error has occurred
		closeDriver(CLOSEDRIVER_ERROR);	// close (hastily)
	} else {
		char * commandLine = new char[size+1]; // have to reserve for "delete"
							// +1 for terminating '\0'
		int returnValue = readString(socketMUA, commandLine, size);

		int argc=1; // We always have at least one argument
		for(int i=0; i < size; i++) {
			if(commandLine[i]=='\t') {
				commandLine[i]='\0';
				argc++;
			}
		}

			// create the argv array of arguments
			char ** argv = new char * [argc];

			// point to the first string
			argv[0] = commandLine;

			int index = 0;
			int args = 1;
			while(index < size) {
				if(commandLine[index] == '\0') {
					if(args < argc) {
						argv[args++] = &commandLine[index+1];
					}
				}
				index++;
			}			

			// let's use "command" for shorthand
			char * command = argv[0];

			if(!strcmp(command, "register")) {
				printf("received register\n");
				if(registerInterface()) {
					// send done
					sendInteger(socketMUA, REPLY_DONE_SIZE);
					sendString(socketMUA, REPLY_DONE, REPLY_DONE_SIZE);
				} else {
					returnValue = 0;			
				}
				//done
			} else if(!strcmp(command, "unregister")) {
				unregisterInterface();
				returnValue = -1;
				sendInteger(socketMUA, REPLY_DONE_SIZE);
				sendString(socketMUA, REPLY_DONE, REPLY_DONE_SIZE);
				// done
			} else if(!strcmp(command, "open")) {
				printf("Received request to OPEN\n");
				// the first argument should be the filename
				string fileName = argv[1];
				// the second argument should be the line number
				int lineNumber = atoi(argv[2]);
				// check to see that those arguments are actually available
				if(open(fileName, lineNumber)) {
					sendInteger(socketMUA, REPLY_DONE_SIZE);
					sendString(socketMUA, REPLY_DONE, REPLY_DONE_SIZE);
				} else {
					returnValue = 0;
				}
				//done
			} else if(!strcmp(command, "mark")) {
				string fileName = argv[1];
				int line = atoi(argv[2]);
				int column = atoi(argv[3]);
				int length = atoi(argv[4]);
				if(argc == 5 && mark(fileName, line, column, length)) {
					sendInteger(socketMUA, REPLY_DONE_SIZE);
					sendString(socketMUA, REPLY_DONE, REPLY_DONE_SIZE);
				} else {
					returnValue = 0;
				}
				//done
			} else if(!strcmp(command, "info")) {
				string message = argv[1];
				if(info(message)) {
					sendInteger(socketMUA, REPLY_DONE_SIZE);
					sendString(socketMUA, REPLY_DONE, REPLY_DONE_SIZE);
				} else {
					returnValue = 0;
				}
				//done
			} else if(!strcmp(command, "freeze")) {
				if(freeze()) {
					sendInteger(socketMUA, REPLY_DONE_SIZE);
					sendString(socketMUA, REPLY_DONE, REPLY_DONE_SIZE);
				} else {
					returnValue = 0;
				}
				//done
			} else if(!strcmp(command, "file")) {
				string response;
				if((response = file()).empty()) {
					returnValue = 0;
				} else {
					sendInteger(socketMUA, response.size());
					sendString(socketMUA, response.c_str(), response.size());
				}
				//done
			} else if(!strcmp(command, "line")) {
				int lineNumber;
				if(lineNumber=line()) {
					sendInteger(socketMUA, lineNumber);
				} else {
					returnValue = 0;
				}
				//done
			} else if(!strcmp(command, "lineoffset")) {
				int offsetNumber;
				if(offsetNumber=lineoffset()) {
					sendInteger(socketMUA, offsetNumber);				
				} else {
					returnValue = 0;
				}
				//done
			} else if(!strcmp(command, "close_driver")) {
				if(!closeDriver(CLOSEDRIVER_PROPER)) {
					returnValue = 0;
				} else {
					sendInteger(socketMUA, REPLY_DRIVERCLOSED_SIZE);
					sendString(socketMUA, REPLY_DRIVERCLOSED, REPLY_DRIVERCLOSED_SIZE);
					returnValue = 1;
				}
				returnValue = 0;
				//done
			} else if(!strcmp(command, "private_line_to_model_line")) {
				int lineNumber;
				string localFile = argv[1];
				string baseFile = argv[2];
				int line = atoi(argv[3]);
				if(argc == 4 && ((lineNumber = privateLine2ModelLine(localFile, baseFile, line)) > 0)) {
					sendInteger(socketMUA, lineNumber);
				} else {
					returnValue = 0;
					sendInteger(socketMUA, -1);
				}
			} else if(!strcmp(command, "model_line_to_private_line")) {
				int lineNumber;
				string localFile = argv[1];
				string baseFile = argv[2];
				int line = atoi(argv[3]);
				if(argc == 4 && ((lineNumber = modelLine2PrivateLine(localFile, baseFile, line)) > 0)) {
					sendInteger(socketMUA, lineNumber);
				} else {
					returnValue = 0;
					sendInteger(socketMUA, -1);
				}
		} else {
			// not supported
			returnValue = 0;
		}

	delete(commandLine);
	return(returnValue);	

	}
}

#ifdef EXAMPLE_PROVIDED
//////////////////////////////////////////////////////////////////
/////// The code below is to be used as a guideline //////////////
/////// in the implementation of a custom editor    //////////////
//////////////////////////////////////////////////////////////////

// A dummy class ... insert your own here
class DerivedClass:public EditorInterface {
	// stub all member functions out for the sake
	// of compilation of this example code
	string DerivedClass::getEditorName(void) { return(string("null")); }
	int DerivedClass::startEditor(void) { return(1); }
	int DerivedClass::unregisterInterface(void) { return(1); }
	int DerivedClass::registerInterface(void) { return(1); }
	int DerivedClass::open(string filename, int line) { return(1); }
	int DerivedClass::mark(string filename, int line, int column, int selectionLength) { return(1); }
	int DerivedClass::info(string message) { return(1); }
	int DerivedClass::freeze(void) { return(1); }
	string DerivedClass::file(void) { return(string("null")); }
	int DerivedClass::line(void) { return(1); }
	int DerivedClass::lineoffset(void) { return(1); }
	int DerivedClass::closeDriver(void) { return(1); }
	int DerivedClass::privateLine2ModelLine(string localFile, string baseFile, int line) { return(1); }
	int DerivedClass::modelLine2PrivateLine(string localFile, string baseFile, int line) { return(1); }
};

// This main was written as an example, but feel free
// to adopt it for your own editor integration
int main(int argc, char * argv[]) {
	int returnValue = 0;
	DerivedClass editorInterface;

	// We tell nameserver we're up and running
	if(editorInterface.registerNameservInterface() > 0) {
		// We set up shop - listen for connection
		if(editorInterface.createListener() > 0) {
			// start the loop
			returnValue = editorInterface.commandLoop();	
		}
	}

	return(returnValue);
}
#endif
