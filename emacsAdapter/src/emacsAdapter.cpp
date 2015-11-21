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
#include <windows.h>
#include <winmain.h>
#include <stdio.h>
#include <fcntl.h>
#include <ws2tcpip.h> // for socklen_t
#include <io.h>

// for stat()
#include <time.h>

// for GetUserName()
#include <lmcons.h>

#include <string>

#else // not _WIN32
#include <stdio.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

// for sockaddr_in
#include <arpa/inet.h>
// for socket()
#include <sys/socket.h>

// for htons()
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <iostream>
using namespace std;

#endif // _WIN32

#include <sys/types.h>
#include <sys/stat.h>

#include "emacsAdapter.h"
#include "debug.h"


CEmacsAdapter::CEmacsAdapter(string name) {
	m_szName = name;
	m_bExit = false;
	m_nConnections = 0;

	memset(m_szEmacsVersion,0,sizeof(m_szEmacsVersion));
       	if(sendToEditor("version\t1")) { // version type 1 - just a number 
	  while(!isEditorMessageAvailable()) {
#ifndef _WIN32
	    sleep(1);
#else
	    Sleep(100);
#endif //_WIN32
	  }
	  getFromEditor(m_szEmacsVersion);
        }

	createListener();
       	registerNameservInterface();
}

CEmacsAdapter::~CEmacsAdapter() {
	unregisterNameservInterface();
	sendToEditor("Emacs_Exit\t1");
}

int CEmacsAdapter::unregisterInterface() {
	m_nConnections--;
	if(m_nConnections==0) { 
		m_bExit=true;
	}
	return 1; 
}

int CEmacsAdapter::registerInterface() {
	m_nConnections++;
	return 1;
}
  
int CEmacsAdapter::closeDriver(int){
	m_bExit = true;
	return 1;
}

bool CEmacsAdapter::isEditorMessageAvailable() {
#ifndef _WIN32
	return checkSocket(fileno(stdin));
#else // _WIN32
	return filelength(fileno(stdin))>0;
#endif // _WIN32
}

int CEmacsAdapter::getFromEditor(char* message){
	int msz = 0;

	if(isEditorMessageAvailable()) {
		msz = read(fileno(stdin), message,MAXCOMMANDLENGTH);
        if(msz > 0) message[msz] = 0;
	}
	return msz;
}

bool CEmacsAdapter::sendToEditor(const char* message) {
        _DBG(fprintf(log_fd,"Sending editor message: %s\n",message)); 
	char newline[2]="\n";
	fwrite(message,sizeof(char),strlen(message),stdout);
	fwrite(newline,sizeof(char),strlen(newline),stdout);
	fflush(stdout);
	return true;
}

int CEmacsAdapter::info(string message) {
	char msg[MAXCOMMANDLENGTH];
	int msgLen = message.length();
	if(message[msgLen-1]=='\n') message.resize(msgLen-1);
	sprintf(msg, "info\t\"%s\"", message.c_str());
	return (sendToEditor(msg)?1:0);
}

int CEmacsAdapter::open(string filename, int line){
	char msg[MAXCOMMANDLENGTH];
        string new_filename;
#ifndef _WIN32
	char szMajor[3];
	szMajor[0] = m_szEmacsVersion[0];
	szMajor[1] = m_szEmacsVersion[1];
	szMajor[2] = 0;
	int nMajor = atoi(szMajor);
	if(nMajor < 20) {
	  int filename_len = filename.length();
	  for (int i = 0; i < filename_len; i++) {
            if (isspace(filename[i])) {
	      new_filename += "\\";
            }
            new_filename += filename[i];
	  }
	} else {
	  new_filename = filename;
	}
#else
        new_filename = filename;
#endif //_WIN32

	sprintf(msg, "open\t%s\t%d", new_filename.c_str(), line);
	return (sendToEditor(msg)?1:0);
}

bool CEmacsAdapter::processEditorCommand(){
	char msg[MAXCOMMANDLENGTH];
	int msgLen = 0;
	if((msgLen = getFromEditor(msg)) > 0) {
	        _DBG(fprintf(log_fd,"Processing editor command %s\n",msg)); 
		sendInteger(socketMUA,msgLen);
		sendString(socketMUA,msg,msgLen);
		
		char* pDelim = strchr(msg,' ');
		if(pDelim==NULL) pDelim = msg + msgLen;
		string cmd(msg,pDelim-msg);
		if(cmd=="right_click") {
			string menuCmd("Emacs_Menu");
			sendToEditor(menuCmd.c_str());
		}
	}
	return true;
}

bool CEmacsAdapter::checkSocket(int socket) {
	timeval timeout;
	
	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;
	fd_set sock_set;
	int    nfsd = 0;
#ifndef _WIN32
	nfsd = FD_SETSIZE;
#endif

	FD_ZERO(&sock_set);
	FD_SET(socket, &sock_set);
#ifdef hp700
	if(select(nfsd,(int *)&sock_set,NULL,NULL,&timeout)>0) {
#else
	if(select(nfsd,&sock_set,NULL,NULL,&timeout)>0) {
#endif
		return true;
     }
     return false;
}

int CEmacsAdapter::WaitForEvent(long usec,long seconds) {
	timeval timeout;
	timeval* pTimeout = &timeout;
	fd_set sock_set;
	int    nfsd = 0;
	
	timeout.tv_usec = usec;	
	timeout.tv_sec  = seconds;

#ifndef _WIN32
	nfsd = FD_SETSIZE;

    FD_ZERO(&sock_set);
    FD_SET(fileno(stdin), &sock_set);
    FD_SET(socketListen, &sock_set);
    if(socketMUA>0) FD_SET(socketMUA, &sock_set); 
#ifdef hp700
    if(select(nfsd,(int *)&sock_set,NULL,NULL,pTimeout)>0) {
#else
    if(select(nfsd,&sock_set,NULL,NULL,pTimeout)>0) {
#endif // hp700
		return 1;
	}
	return 0;
#else // _WIN32
    FD_ZERO(&sock_set);
    FD_SET(socketListen, &sock_set);
	if(socketMUA>0)	FD_SET(socketMUA, &sock_set); 
	int nResult = select(nfsd,&sock_set,NULL,NULL,pTimeout);
    if(nResult) {
		return 1;
	}
	return isEditorMessageAvailable();
#endif //_WIN32
}

bool CEmacsAdapter::acceptClientConnection() {
	int returnValue = 1;
	fd_set socketSet;
	struct sockaddr_in address;
	
	if(checkSocket(socketListen)) {
		socklen_t sizeOf = sizeof(address);
		int acceptSocket = accept(socketListen, (struct sockaddr *)&address, &sizeOf);
		int tmp_switch = 1;
		setsockopt(acceptSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
		socketMUAPort = ntohs(address.sin_port);
		if(acceptSocket > 0) {
		  string command = readMessage(acceptSocket);
		  if(command=="register") {
		    connectedMUA=true;
		    socketMUA = acceptSocket;
		    _DBG(fprintf(log_fd,"ACCEPTED.\n")); 
		  }
		}
	}
	return connectedMUA;
}


int	do_debug=0;
FILE* 	log_fd;
char	dbgFileName[1024];
bool 	bExit;

int main(int argc, char* argv[]) {
	string serviceName;
	CEmacsAdapter* pAdapter;

#ifdef _WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 
	WSAStartup( wVersionRequested, &wsaData );
#endif // _WIN32

#if 0 // to force debugging 
        do_debug=1;
        strcpy(dbgFileName,"/tmp/emacsAdapter.debug");
        _DBG_OPEN();
        if(log_fd==NULL)
            do_debug = 0;
#endif
	for(int i=0;i<argc;i++) {
		if(strcmp(argv[i],"-debug")==0) {
			do_debug=1;
			strcpy(dbgFileName,argv[++i]);
			_DBG_OPEN();
			if(log_fd==NULL)
				do_debug = 0;	
		} else {
			if(strcmp(argv[i],"-service")==0) {
				char  host[1024];
				char* dnStart = 0;
				memset(host,0,1024);
				if(!gethostname(host,sizeof(host))) {
#if 0
					if((dnStart = strchr(host,'.'))>0)
						host[dnStart-host]=0;
#endif
					serviceName += host;
					serviceName+=":";
				} 
				char* currentUser = getenv("USER");
				if(currentUser!=NULL) {
					serviceName += currentUser;
					serviceName+=":";
				}
				serviceName += "Editor:";
				serviceName += argv[++i];
			}
		}
	}
	if(!serviceName.empty()) {
		pAdapter = new CEmacsAdapter(serviceName);
		while(!pAdapter->terminate()) {
			if(pAdapter->WaitForEvent(100000,0)) {
				if(pAdapter->acceptClientConnection()) {
					if(pAdapter->checkSocket(pAdapter->socketMUA))
						pAdapter->processCommand();
				}
				pAdapter->processEditorCommand();
			}
		}
		delete pAdapter;
	}
	_DBG_CLOSE();
	return 0;
}
