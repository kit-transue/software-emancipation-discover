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
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../../nameserver/include/nameServCalls.h"
#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <io.h>
#else
   #include <signal.h>
   #include <arpa/inet.h>
   #include <netinet/tcp.h>
   #include <netdb.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/uio.h>
   #include <sys/wait.h>
   #include <errno.h>
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
#endif
#include "startproc.h"
#include "debug.h"


//---------------------------------------------------------------------------
// This static function will launch name server.
// Returns 0 if unable to launch or 1 if OK.
//---------------------------------------------------------------------------
int LaunchNameServer(int stop) {
	char	nameservCmd[1024];
	char*	location;
	int		i;
	
    memset(nameservCmd,0,1024*sizeof(char));

    location = getenv("CODE_ROVER_HOME");
    if(location != NULL) {
        strcpy(nameservCmd, location);
        strcat(nameservCmd, "\\bin\\nameserv.exe");
    } else {
        location = getenv("PSETHOME");
        if(location != NULL) {
            strcpy(nameservCmd, location);
            strcat(nameservCmd, "/bin/nameserv");
        } else {
            strcpy(nameservCmd, "nameserv");
        }
    }

	if(do_debug)
		strcat(nameservCmd," -debug");
	if(stop==STOP_SERVER)
		strcat(nameservCmd," -stop");
	if(start_new_process(nameservCmd,START_NO_WAIT)!=-1) {
		int port,host;
		for(i=0;i<200;i++) {
			if(NameServerGetService("TEST",port,host)<0) 
				break;
		}
		if(i==200) {
			_DBG(fprintf(log_fd,"Can not connect to Nameserv.\n"));
			return 0;
		}
		if(stop) {
			_DBG(fprintf(log_fd,"Nameserv has been stoped.\n"));
		} else {
			_DBG(fprintf(log_fd,"Nameserv has been started.\n"));
		}
		return 1;
	} else {
		_DBG(fprintf(log_fd,"Nameserv has NOT been started.\n"));
		return 0;
	}
}
//---------------------------------------------------------------------------


int parseCommandLine(char* lpCmdLine, char argv[20][1024]) {
	int		indexes[20];
	unsigned int	idx;
	int		inQ=0;
	int		argc = 0; 

        for(idx=0;idx<20;idx++) {
		argv[idx][0]=0;
		indexes[idx]=0;
	}
	for(idx = 0; idx<strlen(lpCmdLine);idx++) {
		if(lpCmdLine[idx]=='"') {
			inQ=!inQ;
			continue;
		}
		if(inQ==0 && lpCmdLine[idx]==' ') {
		   argv[argc][indexes[argc]]=0;
		   argc++;
		   continue;
		}
		argv[argc][indexes[argc]]=lpCmdLine[idx];
		indexes[argc]++;
	}
        argv[argc][indexes[argc]]=0;
        argc++;
	argv[argc][0]=0;
	return argc;
}

#ifdef _WIN32
DWORD start_new_process(char* cmd,char* outputFile) {
	HANDLE	outputHandle;
	HANDLE	readPipe,writePipe;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD	threadReturnCode;
	DWORD startMode = CREATE_NO_WINDOW;

	SECURITY_ATTRIBUTES sAttr;
	memset(&sAttr,sizeof(SECURITY_ATTRIBUTES),0);
	sAttr.bInheritHandle = true;
	sAttr.lpSecurityDescriptor = NULL;
	sAttr.nLength = sizeof(SECURITY_ATTRIBUTES);

	if(CreatePipe(&readPipe,&writePipe,&sAttr,1000)==false) {
			return 1;
	}

	memset(&si,0,sizeof(STARTUPINFO));
	si.cb=sizeof(STARTUPINFO);
	si.hStdInput  = readPipe;
	si.hStdOutput = writePipe;
	si.hStdError  = writePipe;
	si.dwFlags    = STARTF_USESTDHANDLES;

	if(::CreateProcess(NULL,cmd,NULL,NULL,true,startMode,NULL,NULL,&si,&pi)==false) {
		_DBG(fprintf(log_fd,"CreateProcess FAILED... Command is %s\n",cmd));
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
		_DBG(fprintf(log_fd,"Error is %s\n",lpMsgBuf));
		CloseHandle(pi.hProcess);
		return -1;
	}

	if((outputHandle = ::CreateFile(outputFile, GENERIC_READ|GENERIC_WRITE, 
			FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE) {
		return 1;
	}

	bool bEOF = false;
	DWORD	dwFileSize = 0;
	GetExitCodeProcess(pi.hProcess,&threadReturnCode);
	dwFileSize = GetFileSize(readPipe,NULL);

	DWORD	dwRead = 0;
	DWORD	dwWritten = 0;
	char	cBuff[1024];
		
	DWORD	dwReadCount = 1024;

	while(((dwFileSize > 0) || 
		  (threadReturnCode==STILL_ACTIVE)) &&
		  !bEOF){

		do{
			dwReadCount = 1024;
			if(dwReadCount > dwFileSize) {
				dwReadCount = dwFileSize;
			}

			if(dwReadCount > 0 ){
				bEOF = !ReadFile(readPipe,cBuff,dwReadCount,&dwRead,NULL);
				WriteFile(outputHandle,cBuff,dwRead,&dwWritten,NULL);
				dwFileSize -= dwRead;
			}
			if(dwFileSize<=0)
				dwFileSize = GetFileSize(readPipe,NULL);
  		} while(dwFileSize > 0); // read while pipe is not empty

		// if pipe is empty - check if we still running
		GetExitCodeProcess(pi.hProcess,&threadReturnCode);
		if(threadReturnCode==STILL_ACTIVE) // if still running - sleep a little
			Sleep(100);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(outputHandle);
	CloseHandle(readPipe);
	CloseHandle(writePipe);

	return threadReturnCode;
}

DWORD start_new_process(char* cmd,int waitType) {
	return start_new_process(cmd,waitType,NO_WINDOW);
}
DWORD start_new_process(char* cmd,int waitType,int noWindow) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD	threadReturnCode;
	DWORD	startFlags = 0;
	int		nAttempt = 0;
	LPVOID	lpMsgBuf;
	DWORD	waitStat;

	if(noWindow == NO_WINDOW)
		startFlags |= CREATE_NO_WINDOW;

	memset(&si,0,sizeof(STARTUPINFO));
	si.cb=sizeof(STARTUPINFO);
	for(unsigned int i=0;i<strlen(cmd);i++) {
		  if(cmd[i]=='/')
		  cmd[i] = '\\';
	}	
	switch(waitType) {
	case START_NO_WAIT:
		_DBG(fprintf(log_fd,"START_NO_WAIT...\n"));
		if(::CreateProcess(NULL,cmd,NULL,NULL,true,startFlags,NULL,NULL,&si,&pi)==false) {
			_DBG(fprintf(log_fd,"CreateProcess FAILED... Command is %s\n",cmd));
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);
			_DBG(fprintf(log_fd,"Error is %s\n",lpMsgBuf));
			CloseHandle(pi.hProcess);
			return -1;
		}
		GetExitCodeProcess(pi.hProcess,&threadReturnCode);
		CloseHandle(pi.hProcess);
		break;
	case START_AND_WAIT:
		_DBG(fprintf(log_fd,"START_AND_WAIT...\n"));
		if(::CreateProcess(NULL,cmd,NULL,NULL,true, startFlags,NULL,NULL,&si,&pi)==false) {
			_DBG(fprintf(log_fd,"CreateProcess FAILED... Command is %s\n",cmd));
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);
			_DBG(fprintf(log_fd,"Error is %s\n",lpMsgBuf));
			CloseHandle(pi.hProcess);
			return -1;
		}
		_DBG(fprintf(log_fd,"Waiting untill end of process....\n"));
		waitStat = WaitForSingleObject(pi.hProcess,INFINITE);
		_DBG(fprintf(log_fd,"WaitForSingleObject returned %d\n",waitStat));
		_DBG(fprintf(log_fd,"Checking end of process....\n"));
		do{
			if(GetExitCodeProcess(pi.hProcess,&threadReturnCode)==false) {
				_DBG(fprintf(log_fd,"GetExitCodeProcess FAILED. \n"));
				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
					);
				_DBG(fprintf(log_fd,"Error is %s\n",lpMsgBuf));
				CloseHandle(pi.hProcess);
				return -1;
			}
			_DBG(fprintf(log_fd,"Attempt %d...\n",nAttempt++));
			Sleep(1000);
		}while(threadReturnCode == STILL_ACTIVE);
		CloseHandle(pi.hProcess);
		break;
	default:
		_DBG(fprintf(log_fd,"Incorrect start process type.\n"));
		return -1;
	}
	return threadReturnCode;
}

DWORD start_new_process(char* cmd,int waitType, HANDLE input, HANDLE output, HANDLE error) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD	threadReturnCode;
	DWORD startMode = CREATE_NO_WINDOW;

	memset(&si,0,sizeof(STARTUPINFO));
	si.cb=sizeof(STARTUPINFO);
	si.hStdInput  = input;
	si.hStdOutput = output;
	si.hStdError  = error;
	si.dwFlags    = STARTF_USESTDHANDLES;

	switch(waitType) {
	case START_NO_WAIT:
		if(::CreateProcess(NULL,cmd,NULL,NULL,true,startMode,NULL,NULL,&si,&pi)==false) {
			_DBG(fprintf(log_fd,"CreateProcess FAILED... Command is %s\n",cmd));
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);
			_DBG(fprintf(log_fd,"Error is %s\n",lpMsgBuf));
			CloseHandle(pi.hProcess);
			return -1;
		}
		GetExitCodeProcess(pi.hProcess,&threadReturnCode);
		CloseHandle(pi.hProcess);
		break;
	case START_AND_WAIT:
		if(::CreateProcess(NULL,cmd,NULL,NULL,true,startMode,NULL,NULL,&si,&pi)==false) {
			_DBG(fprintf(log_fd,"CreateProcess FAILED... Command is %s\n",cmd));
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);
			_DBG(fprintf(log_fd,"Error is %s\n",lpMsgBuf));
			CloseHandle(pi.hProcess);
			return -1;
		}
		do{
			if(GetExitCodeProcess(pi.hProcess,&threadReturnCode)==false) {
				_DBG(fprintf(log_fd,"GetExitCodeProcess FAILED. \n"));
				LPVOID lpMsgBuf;
				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
					);
				_DBG(fprintf(log_fd,"Error is %s\n",lpMsgBuf));
				return -1;
			}
			Sleep(1000);
		}while(threadReturnCode == STILL_ACTIVE);
		CloseHandle(pi.hProcess);
		break;
	default:
		_DBG(fprintf(log_fd,"Incorrect start process type.\n"));
		return -1;
	}
	return threadReturnCode;
}

#else  // for UNIX
DWORD start_new_process(char* cmd,char* outputFile) {
	return -1;
}

DWORD start_new_process(char* cmd,int waitType,int noWindow) {
	return start_new_process(cmd,waitType);
}

DWORD start_new_process(char* cmd,int waitType) {
  return start_new_process( cmd, waitType, -1, -1, -1);
}

DWORD start_new_process(char* cmd,int waitType, int stdinHandle, int stdoutHandle, int stderrHandle) {
    char	arglist[20][1024];
    char*	path;

    int		numParams;
    int		status;

    for(int ii = 0; ii < strlen(cmd) ; ++ii) {
    	if(cmd[ii]=='\\') {
	    cmd[ii] = '/';
    	}
    }
    numParams = parseCommandLine(cmd,arglist);
    _DBG(fprintf(log_fd,"\n%s new process %s\n",(waitType==START_AND_WAIT?"START_AND_WAIT":"START_NO_WAIT"),cmd));
    _DBG(fprintf(log_fd,"Arguments - %d\n",numParams));

    char **new_args = new char *[numParams+2];
    for (int i = 0; i < numParams; ++i) {
         new_args[i] = (char *)arglist[i];
    }
    //terminate argument array with null pointer
    new_args[numParams] = NULL;
    const pid_t pid = fork();
    if (pid==-1) {
	_DBG(fprintf(log_fd,"Fork failed...\n"));
	delete new_args;
	return -1;
    }
    if(pid == 0) {
	_DBG(fprintf(log_fd,"Child proccess is alive.\n"));
	setpgid(0,0);
	_DBG(fprintf(log_fd,"Executing %s...\n",arglist[0]));
	// redirect streams to specified handles
	if(stdinHandle!=-1) dup2(stdinHandle,0);
	if(stdoutHandle!=-1) dup2(stdoutHandle,1);
	if(stderrHandle!=-1) dup2(stderrHandle,2);
       	if(execvp(arglist[0], new_args)!=0) {
		_DBG(fprintf(log_fd,"Execvp failed!!! Error is "));
		switch(errno) {
			case E2BIG: _DBG(fprintf(log_fd,"E2BIG\n")); break;
			case EACCES:_DBG(fprintf(log_fd,"EACCES\n")); break;
			case EAGAIN:_DBG(fprintf(log_fd,"EAGAIN\n")); break;
			case EFAULT:_DBG(fprintf(log_fd,"EFAULT\n")); break;
			case EINTR:_DBG(fprintf(log_fd,"EINTR\n")); break;
			case ELOOP:_DBG(fprintf(log_fd,"ELOOP\n")); break;
			case EMULTIHOP:_DBG(fprintf(log_fd,"EMULTIHOP\n")); break;
			case ENAMETOOLONG:_DBG(fprintf(log_fd,"ENAMETOOLONG\n")); break;
			case ENOENT:_DBG(fprintf(log_fd,"ENOENT\n")); break;
			case ENOEXEC:_DBG(fprintf(log_fd,"ENOEXEC\n")); break;
			case ENOLINK:_DBG(fprintf(log_fd,"ENOLINK\n")); break;
			case ENOMEM:_DBG(fprintf(log_fd,"ENOMEM\n")); break;
			case ENOTDIR:_DBG(fprintf(log_fd,"ENOTDIR\n")); break;
		}
		exit(1);
	}
	_DBG(fprintf(log_fd,"We shall never be there :)\n"));
    } else {
	int childPID;
	_DBG(fprintf(log_fd,"Continue parent process\n"));
	if(waitType == START_AND_WAIT){
		do{
			_DBG(fprintf(log_fd,"Waiting for child...\n"));
			childPID = wait(&status);
		} while((childPID!=pid) && (childPID>0));
	}
    }
    delete new_args;
    return status;
}
#endif
//--------------------------------------------------------------------------------------------


