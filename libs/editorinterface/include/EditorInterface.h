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
#if defined HP_UX || hp10 || irix6
#include <iostream.h>
#include <string.h>
#include <string>
	#ifdef irix6
	using std::string;
	#endif
#else
#include <iostream>
using std::string; using std::cout;
using std::endl;
#endif

#include "nameServCalls.h"

// Constants used in communication
int const NAMESERVPORT=NS_PORT; // 

char const * const REPLY_DONE = "1 done\n";
int const REPLY_DONE_SIZE=7;
char const * const REPLY_DRIVERCLOSED = "1 driver_closed";
int const REPLY_DRIVERCLOSED_SIZE=15;

int const CLOSEDRIVER_ERROR=1;
int const CLOSEDRIVER_PROPER=0;


class EditorInterface {

public:
    virtual ~EditorInterface() {}
    int socketListen; // the socket we listen for connections with
    int socketListenPort;
    int socketMUA; // the socket we connect to the Main User Application with
    int socketMUAPort;
    int socketEditor;
    int socketEditorPort; // the socket connected to the editor
    bool connectedMUA; // the state in reference to MUA

    int sendString(int socket, char const * string, int strlen);
    int sendInteger(int socket, int integer);
    int readString(int socket, char * string, int strlen);
    //int readMessage(int socket, char ** string);
    string readMessage(int socket);
    int readInteger(int socket);

    EditorInterface();
    int createListener(void);
    int registerNameservInterface(void);
    int unregisterNameservInterface(void);
    int commandLoop(void);
    int processCommand(void);

    // Functions to be implemented by user
    virtual string getEditorName(void)=0;
    virtual int startEditor(void)=0;
    virtual int unregisterInterface(void)=0;
    virtual int registerInterface(void)=0;
    virtual int open(string filename, int line)=0;
    virtual int mark(string filename, int line, int column, int selectionLength)=0;
    virtual int info(string message)=0;
    virtual int freeze(void)=0;
    virtual string file(void)=0;
    virtual int line(void)=0;
    virtual int lineoffset(void)=0;
    virtual int closeDriver(int)=0;
    virtual int privateLine2ModelLine(string localFile, string baseFile, int line)=0;
    virtual int modelLine2PrivateLine(string localFile, string baseFile, int line)=0;
};
