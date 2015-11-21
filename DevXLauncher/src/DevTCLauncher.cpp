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
#else
#define strcmpi	strcasecmp
#include "unistd.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "curses.h"
#endif
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

#undef __LOG_FILE

#ifdef __LOG_FILE
	FILE* log_fd = fopen("log.txt","rw+");
#endif


//------------------------------------------------------------------------------
// This function will start the process and exit without waiting for it
// to complete startup.  Returns 0 to indicate failure, 1 to indicate
// that process was started, 2 to indicate that the process started and
// is already registered.
//------------------------------------------------------------------------------
#ifdef _WIN32
int start_new_process(char* cmd) {
STARTUPINFO si;
PROCESS_INFORMATION pi;

  memset(&si,0,sizeof(STARTUPINFO));
  SetEnvironmentVariable("DIS_CONNECTION", "http") ;
  for(int i=0;i<strlen(cmd);i++) {
	  if(cmd[i]=='\\')
		  cmd[i] = '/';
  }
  if(::CreateProcess(NULL,(char *)cmd,NULL,NULL,TRUE,DETACHED_PROCESS,NULL,NULL,&si,&pi)==FALSE) {
      return 0;
    }
  return 1;
}

#else  // for UNIX

int start_new_process(char* cmd) {
	char	val[1024];
const char* arglist[20];
    char*	p = cmd;
    char*	tmpcmd;

	while(*p==' ') 
		p++;
    int i=0;
    int j=0;
    while(*p!=0) {
		if(*p==' ') {
			while(*p==' ') 
				p++;
			val[i]=0;
			arglist[j]=strdup(val);
			i=0;
			j++;
			continue;
		}
		val[i++]=*p;
		p++;
    }
    val[i]=0;
    arglist[j++]=strdup(val);
    arglist[j] = NULL;

	int r = putenv("DIS_CONNECTION=http");
	char* res = getenv("DIS_CONNECTION");
#ifdef __LOG_FILE
	char msg[300];
	sprintf(msg,"%s\n",res);
	fwrite(msg,1,strlen(msg),log_fd);
#endif //__LOG_FILE
	const pid_t pid = fork();
	if(pid==-1) 
		return 0;
	if(pid == 0) {
		setpgid(0,0);
		if(execvp(arglist[0], (char*const*)arglist))
			_exit(1);
	}
    return 1;
}
#endif
//------------------------------------------------------------------------------

static char* read_line(FILE* fd) {
int i;
static char line[1024];
char ch;


   for(i=0;i<1024;i++) {
	   if(fread(&ch,1,sizeof(char),fd)!=1) break;
	   if(ch == '\n') 
		   break;
	   else
		   if(ch == '\r') { 
			   i--;
			   continue;
		   }
	   line[i]=ch;
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
			return value;
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
	return value;
}


#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
 	// TODO: Place code here.
#ifdef __LOG_FILE
	log_fd = fopen("log.txt","rw+");
#endif //__LOG_FILE
	
	char* filename=NULL;
#ifdef _WIN32
	if(strlen(lpCmdLine)!=0)
		filename = strdup(lpCmdLine);
	else {
#ifdef __LOG_FILE
		char* error_msg="No input file\n";
		fwrite(error_msg,1,strlen(error_msg),log_fd);
#endif //__LOG_FILE
		return 1;
	}
#else  //_WIN32
	if(argv[1]!=NULL)
		filename = strdup(argv[1]);
	else {
#ifdef __LOG_FILE
		char* error_msg="No input file\n";
		fwrite(error_msg,1,strlen(error_msg),log_fd);
#endif //__LOG_FILE
		return 1;
	}
#endif //_WIN32

	int j=0;
	for(int i=0;i<strlen(filename);i++) {
		if(filename[i]=='"') continue;
		filename[j++]=filename[i];
	}
	filename[j]=0;

    char install_path[2048];

    char*	path = getenv("PSETHOME");
	if(path == NULL)
		strcpy(install_path, "C:/DeveloperXPress/");
	else
		strcpy(install_path, path);

	if(install_path[strlen(install_path)-1]!='/' &&
	   install_path[strlen(install_path)-1]!='\\') {
	   strcat(install_path,"/");
	}
	strcat(install_path,"bin/");

#ifdef __LOG_FILE
	fwrite(filename,1,strlen(filename),log_fd);
	fwrite("\n",1,sizeof(char),log_fd);
#endif //__LOG_FILE
	FILE* fd = fopen(filename,"r");
	if(fd==NULL) {
		 free(filename);
		 return 1;
    }
	char *line;
	char *host=NULL;
	char *model=NULL;
	char *root=NULL;

	do {
	   line = read_line(fd);
	   char* key = GetKey(line);
	   if(strcmpi(key,"HOST")==0)  
		   host = strdup(GetValue(line));
	   if(strcmpi(key,"MODEL")==0) 
		   model = strdup(GetValue(line));
	   if(strcmpi(key,"SOURCE-ROOT")==0) 
		   root = strdup(GetValue(line));
	} while(strlen(line)>0);

	fclose(fd);

	if(host==NULL || model==NULL || root==NULL) {
#ifdef __LOG_FILE
		char* error_msg="Uncorrect file format\n";
		fwrite(error_msg,1,strlen(error_msg),log_fd);
#endif
	} else {		
	// Will pick up developer studio driver here
		char command[2048];
#ifdef _WIN32
		sprintf(command,"\"%s%s\" -service %s@%s",install_path,"dislite",host,model);
#else
		sprintf(command,"%s%s -service %s@%s",install_path,"dislite",host,model);
#endif

#ifdef __LOG_FILE
		fwrite(command,1,strlen(command),log_fd);
		fwrite("\n",1,sizeof(char),log_fd);
#endif
		start_new_process(command);		
	}

	free(filename);
	if(host) free(host);
	if(model) free(model);
	if(root) free(root);
	return 0;
}



