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
// QADriver.cpp : Defines the entry point for the console application.
//
#include "BatchBuild.h"
#include "ClientInterface.h"
#include <iostream>

#define _MESSAGING

#ifdef _MESSAGING
#include "msg.h"
#endif


int main(int argc, char* argv[]) {
ClientInterface clients;

	if(argc<2) {
		cout << "No service, server host and address specified in the command line."<<endl;
		return -1;
	}
	clients.registerService(argv[1]);
	clients.setModelRoot(argv[2]);
	clients.setSharedRoot(argv[3]);
	
	if(argc == 6) {
		init_lp_service(argv[4],argv[5]);
		clients.setLpService(argv[4]);
		clients.setMessageGroup(argv[5]);
	} else {
		if(argc == 5) {
			int pid = atoi(argv[4]);
			printf("QAEngine: PID is %d\n",pid);
			clients.setModelServerHandle(pid);
		}
	}
	clients.requestsLoop();
	return 0;
}

