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
// DevXLauncher.cpp : Defines the entry point for the application.
//

#ifdef _WIN32
#include "stdafx.h"
#include "io.h"
#include <Winsock2.h>
#else
#define strcmpi	strcasecmp
#include "sys/types.h"
#include "sys/uio.h"
#include "unistd.h"
#include "sys/wait.h"
#include <string.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "nameServCalls.h"
#include "debug.h"
#include "startproc.h"

int do_debug = 0;
FILE* log_fd;
char	dbgFileName[1024];

static char* read_line(int fd) {
int i;
static char line[1024];
char ch,prev = ' ';


   for(i=0;i<1024;i++) {
	    if(read(fd,&ch,1)!=1) break;
	    if(ch=='\n') break;
		if(ch!='\r')
		    line[i]=ch;
		else 
			i--;
   }
   line[i]=0;
   return line;
}

static char* GetKey(char* str) {
static char key[1024];
int j = 0;

    for(int i = 0; i<1023; i++) {
        if(str[i]==0)   break;
		if(str[i]==':') break;
		key[j++]=str[i];
	}
	while(j>0 && key[j-1]==' ') j--;
	key[j]=0;
	return key;
}

static char* GetValue(char* str) {
static char value[1024];
int j=0;
int in_value = 0;

    for(int i = 0; i<1023; i++) {
		if(str[i]==0)   {
	        while(j>0 && value[j-1]==' ') j--;
			value[j]=0;
			break;
		}
		if(str[i]==':' && in_value==0 ) {
			in_value = 1;
			i++;
	        while(i<1023 && str[i]==' ' && str[i]!=0) i++;
		}
        if(in_value == 1)   value[j++]=str[i];
	}
	while(j>0 && value[j-1]==' ') j--;
	value[j]=0;
	// removing leading... 
	if(value[0]=='"')
		strcpy(value,value+1);
	// ...and trailing quotes if any
	if(value[strlen(value)-1]=='"')
		value[strlen(value)-1] = 0;
	return value;
}

bool getMemorySetings(HANDLE handle,char* memorySet) {
	char	curChar;
	DWORD	dwReaded;
	char	currentLine[1024];
	int		currentLineIDX = 0;
	char*	delim;
	char	parameter[1024];
	char	value[1024];

#ifdef _WIN32
	if( ReadFile(handle,&curChar,1,&dwReaded,NULL)==TRUE )
#else //UNIX
	if( read(handle,&curChar,1)>0 )
#endif
		if( curChar == ';' ) { //  Insure that prefs is saved in text mode.  
							  //  Only true for older version of CR.
#ifdef _WIN32
			while(ReadFile(handle,&curChar,1,&dwReaded,NULL)==TRUE) {
#else //UNIX
			while(read(handle,&curChar,1)>0) {
#endif
				// skeep leading spaces
				if(currentLineIDX==0 && curChar==' ')
					continue;
				if(curChar != '\n' || curChar != '\r' || 
				   curChar != ' ' || curChar != '\t') {
					if (currentLineIDX >= 1024 ) {
						strcpy(memorySet,"-mx64M");
						break;
					}
					currentLine[currentLineIDX++]=curChar;
				}
				else {
					currentLine[currentLineIDX] = 0;
					if((delim = strchr(currentLine,'='))!=NULL) {
						strncpy(parameter,currentLine,delim-currentLine);
						parameter[delim-currentLine]=0;
						while(parameter[strlen(parameter)-1]==' ') 
							parameter[strlen(parameter)-1] = 0;
						// next char - current is '='
						delim++;
						// trim all leading...
						while(*delim==' ')	delim++;
						strcpy(value,delim);
						// ...and trailing spaces
						while(value[strlen(value)-1]==' ') 
							value[strlen(value)-1] = 0;
						// looking for necessary parameter
#ifdef _WIN32
						if(_stricmp(parameter,"Memory")==0) {
#else //UNIX
						if(strcasecmp(parameter,"Memory")==0) {
#endif
							sprintf(memorySet,"-mx%sM",value);
							break;
						}
					}
					currentLineIDX = 0;
					currentLine[currentLineIDX]=0;
				 }
			}
		}
		else {
			strcpy(memorySet,"-mx64M");
			return false;
		}
	return true;
}

extern int startExeMain(void);

static const int CODE_ROVER = 0;
static const int PROJECT_EDITOR = 1;

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow) {
	char	argv[20][1024];
	int	argc=parseCommandLine(GetCommandLine(), argv);
	char	userName[1024] = "USER=";
	DWORD	bufSize = 1024;
	
	// setting user name environment variable...
	if(GetUserName( userName+strlen(userName), &bufSize)==TRUE)
		putenv(userName);

#else
int main(int argc, char* argv[]) {
#endif

 	// TODO: Place code here.
	int	bNameServerStarted = 0;
	int i, dont_start_CodeRover = 0;
	int	noService = 0;
	int	startApp = CODE_ROVER;
	char*	filename=NULL;
	char*	currentUser;

	// the first command line parameter is an executable name, so skip it
	for(i=1;i<argc;i++) {
		if(argv[i][0]!='-') {
			if(filename==NULL)
			    filename = strdup(argv[i]);
			continue;
		}
		if(strcmp(argv[i],"-debug")==0) { 
			if(((i+1)<argc) && strlen(argv[i+1])>0) {
				do_debug=1;
				strcpy(dbgFileName,argv[++i]);
				_DBG_OPEN();
				if(log_fd==NULL)
					do_debug = 0;
			}
		} else {
			if(strcmp(argv[i],"-norover")==0)
				dont_start_CodeRover = 1;
			else {
				if(strcmp(argv[i],"-noservice")==0)
					noService = 1;
				else {
					if(strcmp(argv[i],"-rover")==0)
						startApp = CODE_ROVER;
					else {
						if(strcmp(argv[i],"-project")==0)
							startApp = PROJECT_EDITOR;
					}
				}
			}
		}
	}

	if((currentUser=getenv("USER"))!=NULL) {
		_DBG(fprintf(log_fd,"Current user is %s\n",currentUser));
	} else {
#ifdef _WIN32
		char	userNameBuff[1024];
		strcpy(userNameBuff,"USER=");
		unsigned	userWordLen = strlen(userNameBuff);
		unsigned long	maxLen = 1024-userWordLen;
		if(GetUserName(userNameBuff+userWordLen,&maxLen)==TRUE) {
			putenv(userNameBuff);
			if((currentUser=getenv("USER"))!=NULL) {
				_DBG(fprintf(log_fd,"Current user is %s\n",currentUser));
			} else {
				_DBG(fprintf(log_fd,"Current user is UNDEFINED.\n"));
			}
		}
#else
		_DBG(fprintf(log_fd,"Current user is UNDEFINED.\n"));
#endif
	}
	
	int fd = -1;
	if(filename!=NULL) {
		_DBG(fprintf(log_fd,"File name is %s\n",filename));
		fd = open(filename,O_RDONLY);
	}

	char *line;
	char *host=NULL;
	char *model=NULL;
	char *root=NULL;
	char *connection=NULL;

	if(fd>0) {
		_DBG(fprintf(log_fd,"Parsing file...\n"));
		do {
		   line = read_line(fd);
		   char* key = GetKey(line);
		   if(strcmpi(key,"HOST")==0)  {
			   host = strdup(GetValue(line));
				_DBG(fprintf(log_fd,"Host is \"%s\"\n",host));
		   }
		   if(strcmpi(key,"MODEL")==0)  {
			   model = strdup(GetValue(line));
				_DBG(fprintf(log_fd,"Model is \"%s\"\n",model));
		   }
		   if(strcmpi(key,"SOURCE-ROOT")==0)  {
			   root = strdup(GetValue(line));
				_DBG(fprintf(log_fd,"Source root is \"%s\"\n",root));
		   }
		   if(strcmpi(key,"CONNECTION")==0)  {
			   connection = strdup(GetValue(line));
				_DBG(fprintf(log_fd,"Connection is \"%s\"\n",connection));
		   }
		} while(strlen(line)>0);

		close(fd);
	}

#ifdef _WIN32
	WORD    wVersionRequested;  
	WSADATA wsaData; 
	int     err = 0; 
	
	_DBG(fprintf(log_fd,"Calling WSAStartup... "));
	wVersionRequested = MAKEWORD(1, 1); 
#ifndef VIEWER_ONLY
	err               = WSAStartup(wVersionRequested, &wsaData);
#endif
	if (err != 0) {
		_DBG(fprintf(log_fd,"no sockets available\n"));
		return -1;
	}
	_DBG(fprintf(log_fd,"OK\n"));
	putenv("DIS_NLUNIXSTYLE=1");
#endif 

    char	install_path[2048];
	char*	envir;
    DWORD dwBufLen = 2048;

	_DBG(fprintf(log_fd,"Getting environment variable... "));
	envir = getenv("CODE_ROVER_HOME");
	if(envir!=NULL) {
		_DBG(fprintf(log_fd,"CODE_ROVER_HOME=%s\n",envir));
		strcpy(install_path,envir);
	} else {
		_DBG(fprintf(log_fd,"CODE_ROVER_HOME is not set.\n"));
#ifdef _WIN32
		HKEY hKey;
		_DBG(fprintf(log_fd,"Trying to get environment variable from registry...\n"));
		DWORD dwType = REG_SZ;
		char envVar[2048];
		char	crVar[1024];
		DWORD dwSize = 2048;
		RegOpenKey(HKEY_CURRENT_USER,"Environment",&hKey);
		if(RegQueryValueEx( hKey,"CODE_ROVER_HOME",NULL,&dwType,(unsigned char*)envVar,&dwSize) == ERROR_SUCCESS) {
			_DBG(fprintf(log_fd,"   Get key  - OK \n"));
			strcpy(crVar,"CODE_ROVER_HOME=");
			strcat(crVar,envVar);
			putenv(crVar);
			strcpy(install_path,envVar);
		} else {
			_DBG(fprintf(log_fd,"   Get key  - FAILED\n"));
			envir = getenv("PSETHOME");
			if(envir) {
				strcpy(crVar,"CODE_ROVER_HOME=");
				strcat(crVar,envir);
				putenv(crVar);
				strcpy(install_path,envir);
			} else
				strcpy(install_path,"");
		}
#endif
	}

	char right_install_path[4096];
    strcpy(right_install_path,install_path);
	if(install_path[strlen(install_path)-1]!='/' &&
	   install_path[strlen(install_path)-1]!='\\') {
	   strcat(right_install_path,"\\");
	}
	_DBG(fprintf(log_fd,"%s\n",right_install_path));
	char command[4096];
	int	service_port, tcp_addr;

#ifndef VIEWER_ONLY
	// Start nameserver at first
	_DBG(fprintf(log_fd,"Launching nameserv..."));
	if(NameServerGetService("test", service_port, tcp_addr) < 0) {
		if(LaunchNameServer(START_SERVER)==1) {
			bNameServerStarted = 1;
			_DBG(fprintf(log_fd,"OK\n"));
		} else {
			_DBG(fprintf(log_fd,"FAILED!!!\n"));
		}
	} else {
		_DBG(fprintf(log_fd,"OK\n"));
	}
#endif

#ifndef VIEWER_ONLY
#ifndef NO_DRIVERS
	// Will pick up developer studio driver here
	strcpy(command,"\"");
	strcat(command,right_install_path);
	strcat(command,"bin\\msdriver.exe");
	strcat(command,"\"");
	
	_DBG(fprintf(log_fd,command,strlen(command)));
	_DBG(fprintf(log_fd,"\n",1));
	
	_DBG(fprintf(log_fd,"Launching msdriver..."));
	if(start_new_process(command)!=-1) {
		_DBG(fprintf(log_fd,"OK\n"));
	} else {
		_DBG(fprintf(log_fd,"FAILED!!!\n"));
	}
#endif
#endif
	
#ifndef VIEWER_ONLY
#ifndef NO_DRIVERS
	// Will pick up Emacs driver here
	strcpy(command,"\"");
	strcat(command,right_install_path);
	strcat(command,"bin\\emacsclient.exe");
	strcat(command,"\"");

	_DBG(fprintf(log_fd,command,strlen(command)));
	_DBG(fprintf(log_fd,"\n",1));
	
	_DBG(fprintf(log_fd,"Launching emacsclient..."));
    	if(start_new_process(command)!=-1) {
		_DBG(fprintf(log_fd,"OK\n"));
	} else {
		_DBG(fprintf(log_fd,"FAILED!!!\n"));
	}		
#endif
#endif

#ifndef VIEWER_ONLY
#ifndef NO_DRIVERS
	// Will pick up CM driver here
	strcpy(command,"\"");
	strcat(command,right_install_path);
	strcat(command,"bin\\CMDriver.exe");
	strcat(command,"\"");

	_DBG(fprintf(log_fd,command,strlen(command)));
	_DBG(fprintf(log_fd,"\n",1));
	
	_DBG(fprintf(log_fd,"Launching CMDriver..."));
    	if(start_new_process(command)!=-1) {
		_DBG(fprintf(log_fd,"OK\n"));
	} else {
		_DBG(fprintf(log_fd,"FAILED!!!\n"));
	}				
#endif
#endif
	// Will start new applet which will run  Dislite.DisliteApp class

#ifdef _WIN32
	char	memorySet[15];
	char	prefFileName[1024];
	char*	userProfile = getenv("USERPROFILE");
	if(userProfile==NULL) {
		if(getenv("WINDIR")!=NULL)
			strcpy(prefFileName,getenv("WINDIR"));
	} else
		strcpy(prefFileName,userProfile);
	if(strlen(prefFileName)>0)
		if(prefFileName[strlen(prefFileName)-1]!='/' && 
		   prefFileName[strlen(prefFileName)-1]!='\\')
			strcat(prefFileName,"/");
	strcat(prefFileName,"preferences.dat");
	HANDLE	prefFile = CreateFile(prefFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
	if(prefFile != INVALID_HANDLE_VALUE) {
		if(getMemorySetings(prefFile,memorySet)==false) {
			_DBG(fprintf(log_fd,"Settings reading FAILED. Using default value %s.\n",memorySet));
		} else {
			_DBG(fprintf(log_fd,"Memory settings value is %s.\n",memorySet));
		}
		CloseHandle(prefFile);
	} else {
		strcpy(memorySet,"-mx64M");
		_DBG(fprintf(log_fd,"No settings file. Using default value %s.\n",memorySet));
	}
#else //UNIX
	char	memorySet[15];
	char	prefFileName[1024];
	char*	userProfile = getenv("HOME");
	if(userProfile==NULL) {
		strcpy(prefFileName,"");
	} else
		strcpy(prefFileName,userProfile);
	if(strlen(prefFileName)>0)
		if(prefFileName[strlen(prefFileName)-1]!='/' && 
		   prefFileName[strlen(prefFileName)-1]!='\\')
			strcat(prefFileName,"/");
	strcat(prefFileName,"preferences.dat");
	int	prefFile = open(prefFileName,O_RDONLY);
	if(prefFile != -1) {
		if(getMemorySetings(prefFile,memorySet)==false) {
			_DBG(fprintf(log_fd,"Settings reading FAILED. Using default value %s.\n",memorySet));
		} else {
			_DBG(fprintf(log_fd,"Memory settings value is %s.\n",memorySet));
		}
		close(prefFile);
	} else {
		strcpy(memorySet,"-mx64M");
		_DBG(fprintf(log_fd,"No settings file. Using default value %s.\n",memorySet));
	}
#endif

	if( !dont_start_CodeRover ) {
		switch(startApp) {
		case CODE_ROVER:
			sprintf(command,"\"%sjre\\bin\\java\" %s -jar \"%slib\\CodeRover.jar\"",right_install_path, memorySet, right_install_path);
			strcat(command," ");
 			if(host!=NULL ) 
       			strcat(command,host);
			else
				strcat(command,"\"\"");
    		strcat(command," ");
    		if(model!=NULL)
				strcat(command,model);
    		else
				strcat(command,"\"\"");
			strcat(command," ");
			if(connection != NULL) 
				strcat(command,connection);
			else
				strcat(command,"\"\"");
			strcat(command," ");
			if(root!=NULL) 
				strcat(command,root);
			else
				strcat(command,"\"\"");
			break;
		case PROJECT_EDITOR:
			sprintf(command,"\"%sjre\\bin\\java\" -jar \"%slib\\ProjectEditor.jar\"",right_install_path, right_install_path);
			break;
		}

		_DBG(fprintf(log_fd,command,strlen(command)));
		_DBG(fprintf(log_fd,"\n",1));

		_DBG(fprintf(log_fd,"Launching application..."));
		if(start_new_process(command,START_NO_WAIT)!=-1){
			_DBG(fprintf(log_fd,"OK\n"));
		} else {
			_DBG(fprintf(log_fd,"FAILED!!!\n"));
#ifndef _WIN32
			printf("Failed to start CodeRover. Possibly link to java is incorrect.\n");
#endif
			exit(-1);
		}				
	}

	free(filename);
	if(host) free(host);
	if(model) free(model);
	if(root) free(root);
	if(connection) free(connection);

#ifndef VIEWER_ONLY
#ifndef _WIN32
	const pid_t pid = fork();
    if(pid == 0) {
		int retCode = 0;
		if(!noService)
			retCode = startExeMain();
		if(bNameServerStarted == 1) {
			_DBG(fprintf(log_fd,"Closing nameserv..."));
			if(LaunchNameServer(STOP_SERVER)==1) {
				_DBG(fprintf(log_fd,"OK\n"));
			} else {
				_DBG(fprintf(log_fd,"FAILED!!!\n"));
			}
		}
		_DBG(fprintf(log_fd,"Program closed\n"));
		_DBG_CLOSE();
		return 0;
    } else {
		_DBG(fprintf(log_fd,"Program closed\n"));
		_DBG_CLOSE();
		return 0;
    }
#else
	int retCode = 0;
	if(!noService) 
		retCode = startExeMain();
	if(bNameServerStarted == 1) {
		_DBG(fprintf(log_fd,"Closing nameserv..."));
		if(LaunchNameServer(STOP_SERVER)==1) {
			_DBG(fprintf(log_fd,"OK\n"));
		} else {
			_DBG(fprintf(log_fd,"FAILED!!!\n"));
		}
	}
	_DBG(fprintf(log_fd,"Program closed\n"));
	_DBG_CLOSE();
	return retCode;
#endif
#else
	_DBG_CLOSE();
	return 0;
#endif
}
