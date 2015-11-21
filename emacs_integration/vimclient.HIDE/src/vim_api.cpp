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
#include <windows.h> 
#include <winsock.h>
#include "../../src/lib.h"
#include "editor_api.h"
#include "connect_mgr.h"
#include "../../../../DevXLauncher/debug.h"
#include "../../../../../nameserver/include/nameServCalls.h"
#include "../../src/CommProto.h"

extern CConnectionDescriptor Connection;
#define EDITOR_NAME "Editor:VIM"
#define INTEGRATOR_NAME "VimIntegrator"

//  ----------------------------------------------------------------------------------
//  These are the static routines used in io.
//  ----------------------------------------------------------------------------------
static int sendToServer( SOCKET s, TCHAR *msg ) {
	_DBG(fprintf(log_fd,"Sending message to the server: %s\n",msg));
	return sendReply( s, msg)*strlen(msg);
}

static char* getSelectionFile(void) {
	TCHAR msg[MAXLISPEXPR];
	static char file[MAXLISPEXPR];
	int ret = -1, msz;
	if(Connection.m_IntegratorSocket >= 0 ) {
		wsprintf (msg, "get-current-file");
		if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) { 
			msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
			if( msz ) {
				strncpy( file, msg, MAXLISPEXPR);
				file[MAXLISPEXPR-1] = 0;
			}
		}
	}
	_DBG(fprintf(log_fd,"Returning current selected file %s\n", file););
    return file;
}

static int getSelectionLine(void) {
	TCHAR msg[MAXLISPEXPR];
	int ret = -1, msz;
	if( Connection.m_IntegratorSocket >= 0 ) {
		wsprintf (msg, "get-current-line");
		if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) { 
			msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
			if( msz ) ret = atoi(msg);
		}
	}
	_DBG(fprintf(log_fd,"Get cursor position (line number) ret = %d\n", ret););
	return ret;
}

static int getSelectionLineOffset(void) {
	TCHAR msg[MAXLISPEXPR];
	int ret = -1, msz;
	if( Connection.m_IntegratorSocket >= 0 ) {
		wsprintf (msg, "get-current-column");
		if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) { 
			msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
			if( msz ) ret = atoi(msg);
		}
	}
	_DBG(fprintf(log_fd,"Get cursor position (within line) ret = %d\n", ret););
	return ret;
}


// This is the rountine to receive data from the emacs "server".
// Format of data from emacs server is (ERROR|OK)/NUMBYTESDATA:DATA
// Data is extracted and returned to calling function.
static int getFromServer(SOCKET s, TCHAR *msg, int msize ) {
	char* cmdServer = getString(s);
	_DBG(fprintf(log_fd,"Reading string from the server..."));
	if(cmdServer == NULL) {
		_DBG(fprintf(log_fd,"No reply!!!\n"));
		return 0;
	}
	else
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
int LaunchIntegrator(void) {
STARTUPINFO si;
PROCESS_INFORMATION pi;
char intname[1024];

#define EMACS_PATH_LEN 1024l

char some_dir[EMACS_PATH_LEN];
char vim_path[EMACS_PATH_LEN];
char some_path[EMACS_PATH_LEN];

DWORD some_dir_len = sizeof(some_dir);
DWORD paramType = REG_EXPAND_SZ;

    // getting Code Rover directory
	_DBG(fprintf(log_fd,"Starting Integrator... \n"));
	char* envPath = getenv("CODE_ROVER_HOME");
	if(envPath==NULL)
		envPath = getenv("PSETHOME");
	if(envPath)
		strcpy(some_dir,envPath);
	else
		strcpy(some_dir,"C:\\Program Files\\Code Rover");
	
	// adding emacs path to the launch parameter
	strcat(some_dir,"\\bin\\gvim.exe");

	GetShortPathName(some_dir,intname,sizeof(intname));


	// we convert our path string to short file name because emacs doesn't understand spaces 
	// in file names
	if(envPath)
		strcpy(some_dir,envPath);
	else
		strcpy(some_dir,"C:\\Program Files\\Code Rover");

	GetShortPathName(some_dir,some_path,sizeof(some_path));
	if(some_path[strlen(some_path)-1]=='\\' || some_path[strlen(some_path)-1]=='/')
		some_path[strlen(some_path)-1]=0;

	for(int i=strlen(some_path); i; i--) if(some_path[i]=='\\') some_path[i]='/';

	_DBG(fprintf(log_fd,"CRHOME is %s\n",some_path));
	SetEnvironmentVariable("CRHOME", some_path) ;
	strcpy(vim_path, some_path);
	strcat(vim_path, "/lib/vim" );
	_DBG(fprintf(log_fd,"VIM is %s\n",vim_path));
	SetEnvironmentVariable("VIM", vim_path) ;
	_DBG(fprintf(log_fd,"DIS_VIM_DEVXPRESS is 1\n"));
	SetEnvironmentVariable("DIS_VIM_DEVXPRESS", "1") ;

	memset(&si,0,sizeof(STARTUPINFO));
	si.cb=sizeof(STARTUPINFO);
	if(::CreateProcess(NULL,intname,NULL,NULL,FALSE,DETACHED_PROCESS,NULL,NULL,&si,&pi)==FALSE) {
		_DBG(fprintf(log_fd,"Error while launching integrator server. Command is %s\n",intname));
		return 0;
	}
	_DBG(fprintf(log_fd,"Integrator started. Command is %s\n",intname));
	return 1;
}

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
	int ret, retcode, dsz, rsz;
	char *sep;
	static char buffer[10000];
	static int init_buffer = 0;

	if( !init_buffer ) { init_buffer = 1 ; buffer[0] = 0; }

	retcode = -1;
	ret = -1;
	while(ret < 0) {
		if( !buffer[0] )
		{
			ret = getFromServer(s, msg, msize );
			if( ret > 0 ) strcat(buffer, msg);
			_DBG(fprintf(log_fd,"Buffer empty read from socket: %s\n", buffer););
		}
		else {
			ret = strlen(buffer);
			_DBG(fprintf(log_fd,"Buffer is not empty: %s\n", buffer););
		}

		if( ret > 0 )
		{
			sep = buffer;
			if( !strncmp(sep,hdr,strlen(hdr)) ) {
				if( sep = strchr(sep, '/') ) {
					dsz = atoi( sep + 1 );
					if( sep = strchr(sep, ':') ) {
						sep++;
						strncpy(msg, sep, dsz);
						msg[dsz] = 0;

						sep = sep + dsz;
						sep = trim(sep);
						if( *sep ) {
							rsz = strlen(sep);
							memmove(buffer, sep, rsz );
							buffer[ rsz ] = 0;
							_DBG(fprintf(log_fd,"More than one pkt recieved storing remainder: %s\n", buffer););
						}	
						else 
							buffer[0] = 0;
						retcode = dsz;
					}
				}
			}
			else if ( !strncmp(msg,EditorErrorHdr,strlen(EditorErrorHdr)) )
				ret = 0;  // Exit with error set.
			else {
				ret = -1;  // Discard data not matching hdr.
				if( sep = strchr(sep, '/') ) {
					dsz = atoi( sep + 1 );
					if( sep = strchr(sep, ':') ) {
						sep++;
						strncpy(msg, sep, dsz);
						msg[dsz] = 0;
						_DBG(fprintf(log_fd,"Discarding packet: %s looking for hdr %s\n", msg, hdr););

						sep = sep + dsz;
						sep = trim( sep );
						if( *sep ) {
							rsz = strlen(sep);
							memmove(buffer, sep, rsz );
							buffer[ rsz ] = 0;
							_DBG(fprintf(log_fd,"More than one pkt recieved storing remainder: %s\n", buffer););
						}	
						else
							buffer[0] = 0;
					}
				}

			}
		}
	}
	return retcode;
}

int MakeSelectionOnLine(char* file, int line, int lineoffset,int length) {
	TCHAR msg[MAXLISPEXPR];
	int ret = 0, msz;
	if( Connection.m_IntegratorSocket >= 0 ) {
		if( strlen(file) + 50 < MAXLISPEXPR ) {
			wsprintf (msg, "select-region \"%s\" %d %d %d", file, line, lineoffset, length );
			if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) {
				msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
				if( !strncmp(msg, EMACSOK, strlen(EMACSOK) ) ) ret = 1;
			}
		}
	}
   _DBG(fprintf(log_fd,"Make selection: File:%s Line:%d Column:%d Length:%d.\n",file,line,lineoffset,length););
   return ret;
}

int ShowMessage(char* messag) {
	TCHAR msg[MAXLISPEXPR];
	int msz, ret = -1;
	if(messag && strlen(messag) && Connection.m_IntegratorSocket >= 0 ) {
		wsprintf (msg, "info \"%s\"", messag);
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
	TCHAR msg[MAXLISPEXPR];
	int ret = 1, msz;
	if( Connection.m_IntegratorSocket >= 0 ) {
		// roverserv-edit-files-quickly responses immeadiatedly without waiting
		// for errors, user interaction, etc.
		if( strlen(file) + 50 < MAXLISPEXPR ) {
			wsprintf (msg, "open \"%s\" %d", file, line );
			if( sendToServer(Connection.m_IntegratorSocket, msg) != SOCKET_ERROR ) {
				msz = getDataFromServer(Connection.m_IntegratorSocket, msg, MAXLISPEXPR, EditorDataHdr);
				if( !strncmp(msg, EMACSOK, strlen(EMACSOK) ) ) ret = 0;
			}
		}
	}
	_DBG(fprintf(log_fd,"Open file %s at line %d. ret = %d\n",file,line, ret););
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
