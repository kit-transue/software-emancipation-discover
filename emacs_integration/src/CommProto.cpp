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
#ifdef _WIN32
	#include <winsock.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/uio.h>
#endif
#include <string.h>

#include "CommProto.h"
#include "nameServCalls.h"

int sendReply(int socket,char* str) {
char* rpStr;

	int len = strlen(str);
	rpStr = new char[len+3];
	strcpy(rpStr,"1 ");
	strcat(rpStr,str);
	int nRet = sendData(socket,rpStr);
	delete rpStr;
	return nRet;
}

//--------------------------------------------------------------------------------------------
// Event happend in the editor. We need to notify all clients.
//--------------------------------------------------------------------------------------------
int sendEvent(int socket,char* str) {
char* evStr;

	int len = strlen(str);
	evStr = new char[len+3];
	strcpy(evStr,"0 ");
	strcat(evStr,str);
	int nRet = SendString(socket,str);
	delete evStr;
	return nRet;
}

