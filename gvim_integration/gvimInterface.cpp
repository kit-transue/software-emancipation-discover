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
//// 1. We should provide an interface to the editor via sockets and pipes.
//// 2. We should provide some sort of error message mechanism
#include "EditorInterface.h"
#include <lmcons.h>
#include <stdio.h>

#ifdef WIN32
DWORD const USERNAME_SIZE=UNLEN+1;
#else
int const USERNAME_SIZE=L_cuserid;
#endif

int const HOSTNAME_SIZE=1024;

// Particular to our implementation of the class
char const * const EDITOR_NAME = "gvim";

using std::string; using std::cout;
using std::endl;

class gvimInterface:public EditorInterface {

public:
	string getEditorName(void) {
		// we generate the editor name each time so that
		//	if we have to generate names per editor it is done
		//	right each time
		string editorName;
		char * charPointer;
		// get hostname prefix
		// get user
		// Editor:
		// gvim
		// the prefix
		char hostname[HOSTNAME_SIZE]; // LIMITING FACTOR! - help?
		gethostname(hostname, HOSTNAME_SIZE);
		charPointer = strchr(hostname, '.');
		if(charPointer) { // sometimes FQDN isn't given by hostname
			*charPointer = '\0';
		}	
		editorName = hostname;
		editorName += ':';
		// user
		char userName[USERNAME_SIZE];
#ifdef WIN32
		DWORD usernameSize = USERNAME_SIZE;
		GetUserName(userName, &usernameSize);
#else
		cuserid(userName);
#endif
		editorName += userName;
		editorName += ':';
		// Editor:
		editorName += "Editor:";
		// the prefix
		editorName += EDITOR_NAME;
		return(editorName);
	}

	int startEditor(void) { return(1); }
	int unregisterInterface(void) { return(1); }
	int registerInterface(void) {
		// Nothing to be done, really
		// This is an indication that the MUA wishes to use gvim as
		//	an editor.  To unregister, the MUA wishes to shut down gvim
		//	and this associated driver
		return(1);
	}
	int open(string filename, int line) { return(1); }
	int mark(string filename, int line, int column, int selectionLength) { return(1); }
	int info(string message) { return(1); }
	int freeze(void) { return(1); }
	string file(void) {
		return(freezeFile);
	}
	int line(void) {
		return(freezeLine);
	}
	int lineoffset(void) {
		return(freezeLineOffset);
	}
	int closeDriver(void) { return(1); }
	int privateLine2ModelLine(string localFile, string baseFile, int line) { return(1); }
	int modelLine2PrivateLine(string localFile, string baseFile, int line) { return(1); }

private:
	string freezeFile;
	int freezeLine;
	int freezeLineOffset;
};

int main(int argc, char * argv[]) {
	int returnValue = 0;
	gvimInterface gvimDriver;

	// We tell nameserver we're up and running

	printf("The editor name is %s\n", gvimDriver.getEditorName().c_str());

	if(gvimDriver.registerNameservInterface() > 0) {
		// We set up shop - listen for connection
		if(gvimDriver.createListener() > 0) {
			// start the loop
			returnValue = gvimDriver.commandLoop();	
		}
	}

	return(returnValue);
}
