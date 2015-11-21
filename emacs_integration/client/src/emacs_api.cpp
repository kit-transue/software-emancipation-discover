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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
	#include <windows.h>
	#include <winsock.h>
#else 
   #ifndef hp10
	#include <sysent.h>
   #endif
   #include <signal.h>
   #include <arpa/inet.h>
   #include <netinet/tcp.h>
   #include <netdb.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/wait.h>
   #ifdef sun4
      #include <poll.h>
      #include <time.h>
      #include <sys/time.h>
   #endif
   #ifdef irix6
      #include <sys/poll.h>
   #endif
   #ifdef hp700
      #include <sys/poll.h>
      #include <time.h>
      extern "C" int select(int, int*, int*, int*, const struct timeval*);
   #endif
   #define closesocket(sock) close(sock)
#endif

#include "lib.h"
#include "editor_api.h"
#include "connect_mgr.h"

#include "nameServCalls.h"
#include "CommProto.h"
#include "startproc.h"
#include "debug.h"

extern CConnectionDescriptor Connection;
#define EDITOR_NAME "Editor:Emacs"
#define INTEGRATOR_NAME "EmacsIntegrator"

//  ----------------------------------------------------------------------------------
//  These are the static routines used in io.
//  ----------------------------------------------------------------------------------

static int sendToServer( SOCKET s, char* msg ) {
	_DBG(fprintf(log_fd,"Sending message to the server: %s\n",msg));
	return sendReply( s, msg)*strlen(msg);
}

static char* getSelectionFile(void) {
	char msg[MAXLISPEXPR];
	static char file[MAXLISPEXPR];
	int ret = -1, msz;
	if(Connection.m_IntegratorSocket >= 0 ) {
		sprintf (msg, "(roverserv-get-current-file)");
		if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) { 
			msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
			if( msz ) {
				strncpy( file, msg, MAXLISPEXPR);
				file[MAXLISPEXPR-1] = 0;
			}
		}
	}
	_DBG(fprintf(log_fd,"Returning current selected file %s\n", file); );
    return file;
}

static int getSelectionLine(void) {
	char msg[MAXLISPEXPR];
	int ret = -1, msz;
	if( Connection.m_IntegratorSocket >= 0 ) {
		sprintf (msg, "(roverserv-get-current-line)");
		if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) { 
			msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
			if( msz ) ret = atoi(msg);
		}
	}
	_DBG(fprintf(log_fd,"Get cursor position (line number) ret = %d\n", ret); );
	return ret;
}

static int getSelectionLineOffset(void) {
	char msg[MAXLISPEXPR];
	int ret = -1, msz;
	if( Connection.m_IntegratorSocket >= 0 ) {
		sprintf (msg, "(roverserv-get-current-column)");
		if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) { 
			msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
			if( msz ) ret = atoi(msg);
		}
	}
	_DBG(fprintf(log_fd,"Get cursor position (within line) ret = %d\n", ret); );
	return ret;
}


// This is the rountine to receive data from the emacs "server".
// Format of data from emacs server is (ERROR|OK)/NUMBYTESDATA:DATA
// Data is extracted and returned to calling function.
static int getFromServer(SOCKET s, char *msg, int msize ) {
	_DBG(fprintf(log_fd,"Reading string from the server..."));
	char* cmdServer = getString(s);
	if(cmdServer == NULL) {
		_DBG(fprintf(log_fd,"No reply!!!\n"));
		return 0;
	} else
		strcpy(msg,cmdServer+2);
	_DBG(fprintf(log_fd,"Reply is %s\n",msg));
	return strlen(msg);
}

//  -----------------------------------------------------------------------------
//  All Externally Published Rountines
//  -----------------------------------------------------------------------------
void ConstructServicePrefix(char prefix[1024]) {
	char*	userName;
	char*	dnStart;
	// determine the current host name
	gethostname(prefix,1024);
	// this part of code just removes domain name suffix from the hostname
	if((dnStart = strchr(prefix,'.'))>0)
		prefix[dnStart-prefix] = 0;
	// constructing service prefix
	strcat(prefix,":");
	userName = getenv("USER");
#ifdef _WIN32
	if(userName == NULL) {
		char	userString[1024];
		DWORD	nUserSize = 1024;
		if(GetUserName(userString,&nUserSize)==TRUE)
			userName = userString;
	}
#endif
	if(userName!=NULL) {
		strcat(prefix,userName);
		strcat(prefix,":");
	}
}

char *GetIntegratorName(void) {
static char integratorName[1024];
	ConstructServicePrefix(integratorName);
	strcat(integratorName,INTEGRATOR_NAME);
	return integratorName;
}

char *GetEditorName(void) {
static char	editorName[1024];
	ConstructServicePrefix(editorName);
	strcat(editorName,EDITOR_NAME);
	return editorName;
}

// Launch emacs.  Sources roverserv.el and starts server exec.
// Emacs and this server talk via a pipe.  This is required because emacs 
// Must spawn anything it wants to talk to.  We want the browser to pre-exist emacs
// so this inbetween in necessary.
#ifdef _WIN32
int LaunchIntegrator(void) {
	char intname[1024];
	char msg[1024];
	HKEY someKey;

	#define EMACS_PATH_LEN 1024l

	char some_dir[EMACS_PATH_LEN];
	char some_path[EMACS_PATH_LEN];
	char tmp_path[EMACS_PATH_LEN];
	DWORD some_dir_len = sizeof(some_dir);
	DWORD paramType = REG_EXPAND_SZ;

	_DBG(fprintf(log_fd,"Starting Integrator... \n"));

	// getting emacs path (if it's properly installed)
	// this is directory for emacs
	some_path[0]=0;
	LONG res = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\GNU\\Emacs",0,KEY_READ,&someKey);
	if(res == ERROR_SUCCESS) {
		res = RegQueryValueEx( someKey, "emacs_dir", NULL, &paramType, (BYTE *)some_dir, &some_dir_len);
		if(res == ERROR_SUCCESS) {
			// this is directory for emacs - it must be something like this "%emacs_dir%/bin"
			some_dir_len = sizeof(some_path);
			res = RegQueryValueEx( someKey, "EMACSPATH", NULL, &paramType, (BYTE *)some_path, &some_dir_len);
			if(res!=ERROR_SUCCESS)	{
				some_path[0]=0;
				some_dir[0]=0;
			}
		}
	}
	// constructing emacs path
	if(some_path[0] == '%') {
		for(unsigned i=1;i<strlen(some_path) && some_path[i]!='%';i++)
			tmp_path[i-1]=some_path[i];
		if( i != strlen(some_path) ) {
			tmp_path[i-1] = 0;
			i++;
		}
		if(strcmp(tmp_path,"emacs_dir")==0) {
			strcat(some_dir,some_path+i);
		} else {
			strcpy(some_dir,some_path);
		}
	} else {
		strcat(some_dir,some_path);
	}
	// adding emacs path to the launch parameter
	if(strlen(some_dir)>0) {
		strcpy(tmp_path,"\"");
		strcat(tmp_path,some_dir);
		strcat(tmp_path,"\\emacs.exe\"");
	} else {
		strcpy(tmp_path,"emacs.exe");
	}
	// we convert our path string to short file name because emacs doesn't understand spaces 
	// in file names
	strcpy(intname,tmp_path);
	strcat(intname," -l ");

    // getting DIScover directory
	char* envPath = getenv("PSETHOME");
	if(envPath)
		strcpy(some_dir,envPath);
	else
		strcpy(some_dir,"");

	strcpy(tmp_path,"\"");
	strcat(tmp_path,some_dir);
	strcat(tmp_path,"\\bin\\roverserv.el\"");
	// we convert our path string to short file name because emacs doesn't understand spaces 
	// in file names
	strcat(intname,tmp_path);

	// we convert our path string to short file name because emacs doesn't understand spaces 
	// in file names
	strcpy(some_path,some_dir);
	if(some_path[strlen(some_path)-1]=='\\' || some_path[strlen(some_path)-1]=='/')
		some_path[strlen(some_path)-1]=0;


	for(int i=strlen(intname); i; i--) if(intname[i]=='/') intname[i]='\\';
	for(i=strlen(some_path); i; i--) if(some_path[i]=='/') some_path[i]='\\';

	DWORD retCode;
	if((retCode = start_new_process(intname,START_NO_WAIT,OPEN_WINDOW))==-1) {
		wsprintf (msg,"%s - return code %d\n", intname, retCode);
		_DBG(fprintf(log_fd,"Error while launching integrator server. Command is %s\n",intname));
		return 0;
	}
	_DBG(fprintf(log_fd,"Integrator started. Command is %s\n",intname));
	return 1;
}
#else
int LaunchIntegrator(void) {
	char emacsCmd[1024];
	char msg[1024];

	_DBG(fprintf(log_fd,"Starting Integrator... \n"));

    // getting DIScover directory
	char* envPath = getenv("PSETHOME");
	if(envPath) {
		strcpy(emacsCmd,"\"");
		strcat(emacsCmd,envPath);
		strcat(emacsCmd,"/bin/emacs\" -l ");
		strcat(emacsCmd,"\"");
		strcat(emacsCmd,envPath);
		strcat(emacsCmd,"/bin/");
	} else 
		strcpy(emacsCmd,"emacs -l \"");
	strcat(emacsCmd,"roverserv.el\"");

	DWORD retCode;
	if((retCode = start_new_process(emacsCmd,START_NO_WAIT,OPEN_WINDOW))==-1) {
		sprintf (msg,"%s - return code %d\n",emacsCmd, retCode);
		_DBG(fprintf(log_fd,"Error while launching integrator server. Command is %s\n",emacsCmd));
		return 0;
	}
	_DBG(fprintf(log_fd,"Integrator started. Command is %s\n",emacsCmd));
	return 1;
}
#endif

static char * trim( char * sep ) {
	while ( sep && *sep ) {
		if( *sep == ' ' || *sep == '\n' || *sep == '\r' )
			sep++;
		else
			break;
	}
	return sep;
}

// Used by driver to get incoming data from server.
int getDataFromServer(SOCKET s, char *msg, int msize, char * hdr ) {
	return getFromServer(s, msg, msize );
}

int MakeSelectionOnLine(char* file, int line, int lineoffset,int length) {
	char msg[MAXLISPEXPR];
	int ret = 0, msz;
	if( Connection.m_IntegratorSocket >= 0 ) {
		if( strlen(file) + 50 < MAXLISPEXPR ) {
			sprintf (msg, "(roverserv-select-region \"%s\" %d %d %d)", file, line, lineoffset, length );
			if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) {
				msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
				if( msz >0 && !strncmp(msg, EMACSOK, strlen(EMACSOK) ) ) ret = 1;
			}
		}
	}
   _DBG(fprintf(log_fd,"Make selection: File:%s Line:%d Column:%d Length:%d.\n",file,line,lineoffset,length););
   return ret;
}

int ShowMessage(char* messag) {
	char msg[MAXLISPEXPR];
	int msz, ret = -1;
	if(messag && strlen(messag) && Connection.m_IntegratorSocket >= 0 ) {
		sprintf (msg, "info\t\"%s\"", messag);
		if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) { 
			msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
			if( msz > 0 ) ret = atoi(msg);
		}
	}
	_DBG(fprintf(log_fd,"Print message in status bar %s\n", messag););
	return ret;
}

struct selection {
	int line_offset;
	int column_offset;
	char *file;
} GSel = { -1, -1, NULL };

int FreezeSelection(void) {
  _DBG(fprintf(log_fd,"Freeze selection.\n"));
  GSel.file = getSelectionFile();
  GSel.line_offset = getSelectionLine();
  GSel.column_offset = getSelectionLineOffset();
  return 1;
}

char* GetSelectionFile(void) {
    return GSel.file;
}

int GetSelectionLine(void) {
	return GSel.line_offset;
}

int GetSelectionLineOffset(void) {
	return GSel.column_offset;
}


int OpenFileOnLine(char* file, int line) {
	char msg[MAXLISPEXPR];
	int ret = 1, msz;
	if( Connection.m_IntegratorSocket >= 0 ) {
		// roverserv-edit-files-quickly responses immeadiatedly without waiting
		// for errors, user interaction, etc.
		if( strlen(file) + 50 < MAXLISPEXPR ) {
			sprintf(msg, "open\t\"%s\"\t%d", file, line);
			if(sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) {
				msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
				if( msz > 0 && !strncmp(msg, EMACSOK, strlen(EMACSOK) ) ) ret = 0;
			}
		}
	}
	_DBG(fprintf(log_fd,"Open file %s at line %d. ret = %d\n",file,line, ret); );
	return ret;
}

void decrementRefCount(SOCKET sock) {
	// do nothing - there is no such command
	return;
}

// returns maximum length of command
int getCommandLength(void) {
	return 1024;
}







