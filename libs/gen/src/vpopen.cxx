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
// vpopen.C
//------------------------------------------
// synopsis:
//
// vpopen() is a work-alike for popen(), except that it calls vfork()
// rather than fork().  vpopen() also supports an enhancement to standard
// popen() for tying stdout and stderr together.  This is done by passing
// mode "R" (upper case) instead of "r" (lower case).
//
// vpopen2way() is similar, but it opens two pipes, for both stdin and
// stdout.  Consequently, it has a different calling sequence.
//
// vsystem() similarly is like system(), except that it calls vfork().
//
// v_system() is an obsolete alias for vsystem().
//
//------------------------------------------

// INCLUDE FILES

#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>
#endif /* _WIN32 */
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <genError.h>
#include <genArr.h>
#include <genString.h>
#include <setprocess.h>

#if defined(ibm) || defined (__GNUG__)
        typedef void SIG_FUNC_TYP(int);
        typedef SIG_FUNC_TYP *SIG_TYP;
#endif

// VARIABLE DEFINITIONS
genArr(int);
static intArr * process_ids_ptr = new intArr;
static intArr& process_ids = *process_ids_ptr;
// FUNCTION DEFINITIONS
int vpopen2w(int not_std, const char *sh_command, int *rfd, int *wfd);

static void save_pid(const int fd, const int pid)
{
    Initialize(save_pid);

    if(process_ids.size() <= fd)
	process_ids.grow(fd - process_ids.size() + 1);

    *process_ids[fd] = pid;
}

static void clear_pid(const int fd)
{
    Initialize(clear_pid);

    int pid = 0;
    int* pid_ptr = process_ids[fd];
    if(pid_ptr)
	*pid_ptr = 0;
}

extern "C" int vp_find_pid (FILE* f)
{
    Initialize(vp_find_pid);

    int pid = 0;
    if(f)
    {
	int* pid_ptr = process_ids[OSapi_fileno(f)];
	if(pid_ptr)
	    pid = *pid_ptr;
    }
    return pid;
}

#ifdef _WIN32

void CreatePIDenv(void)
{
	char buf[20];

	int pid = OSapi_getpid();
	sprintf(buf, "%d", pid);
	SetEnvironmentVariable("PSET_PPID", buf);
}

#endif

extern "C" void kill_running_processes(void)
{
    int pid;

    for(int i = 0; i < process_ids.size(); i++){
	int* pid_ptr = process_ids[i];
	if(pid_ptr){
	    pid      = *pid_ptr;
	    OSapi_kill(pid, SIGKILL);
	}
    }
}

extern "C" void vpclose_pid(int pid)
{
  if(pid <= 1)
    return;
  for(int i = 0; i < process_ids.size(); i++){
    int* pid_ptr = process_ids[i];
    if(pid_ptr){
      int cur_pid      = *pid_ptr;
      if(cur_pid == pid){
	OSapi_close(i);
	*pid_ptr = 0;
      }
    }
  }
  
  OSapi_kill(-pid, SIGTERM); //group
}

/**************************************************************************
vpopen:	fork a child task, keeping either a write pipe to its stdin,
	or a read pipe from its stdout.

input:	sh_command	string containing a Bourne shell (/bin/sh) command

	type	"r" for a read pipe (from stdout of child),
		"R" for a combined read pipe (from stdout & stderr of child),
		"w" for a write pipe (to stdin of child)
		Use -R -r -w to run suprocess without /bin/sh

return: stream pointer to the pipe

side effects:  pid of child is saved in process_ids[], a static array
	this pid can be retrieved later from the stream pointer, by using
	vp_find_pid()

***************************************************************************/

#ifndef _WIN32
extern "C" FILE* vpopen(const char* sh_command, const char* type)
{
    int no_sh = 0;
    if (*type == '-') {
        type++;
        no_sh = 1;
    }

    if(!type  ||  !(*type == 'r'  ||  *type == 'R'  ||  *type == 'w'))
    {
	msg("vpopen: invalid stream type") << eom;
	return NULL;
    }

    int fd[2];
    if(OSapi_pipe(fd))
    {
	OSapi_perror("vpopen: cannot create pipe");
	return NULL;
    }

    const int pid = (const int)vfork();
    if(pid == 0)		// CHILD
    {
	if(*type == 'r')	// Reading stdout from child
	{
	    OSapi_close(fd[0]);
	    if(OSapi_dup2(fd[1], 1) < 0)
	    {
		OSapi_perror("vpopen: cannot setup stdout for child");
		OSapi__exit(-1);
	    }
	    OSapi_close(0);
	    OSapi_open("/dev/null", O_RDONLY);
	}
	else if(*type == 'R')	// Reading stdout and stderr from child
	{
	    OSapi_close(fd[0]);
	    if(OSapi_dup2(fd[1], 1) < 0)
	    {
		OSapi_perror("vpopen: cannot setup stdout for child");
		OSapi__exit(-1);
	    }
	    if(OSapi_dup2(fd[1], 2) < 0)
	    {
		OSapi_perror("vpopen: cannot setup stderr for child");
		OSapi__exit(-1);
	    }
	    OSapi_close(0);
	    OSapi_open("/dev/null", O_RDONLY);
	}
	else if(*type == 'w')	// Writing stdin to child
	{
	    OSapi_close(fd[1]);
	    if(OSapi_dup2(fd[0], 0) < 0)
	    {
		OSapi_perror("vpopen: cannot setup stdin for child");
		OSapi__exit(-1);
	    }
	}

        if (no_sh) {
            char cmd[2000];
	    char* cmd_ptr;
	    char quoted;
	    char*  argv[100];
	    int i = 0;

	    strcpy (cmd, sh_command);
	    cmd_ptr = cmd;
	    
 	    while (*cmd_ptr) {
	        while (*cmd_ptr && isspace(*cmd_ptr))
		    cmd_ptr++;
		if (*cmd_ptr == '"') {
		    quoted = *cmd_ptr;
		    cmd_ptr++;
		} else if (*cmd_ptr == '\'') {
		    quoted = *cmd_ptr;
		    cmd_ptr++;
		} else quoted = 0;
		
		argv[i++] = cmd_ptr;
		if (i == 100) break;

		while (*cmd_ptr && (quoted || !isspace(*cmd_ptr))) {
		    if (quoted && quoted == *cmd_ptr) {
			*cmd_ptr = 0;
			cmd_ptr++;
			break;
		    }
		    cmd_ptr++;
		}

		if (*cmd_ptr) {
		    *cmd_ptr = 0;
		    cmd_ptr++;
		}
	    }

	    argv[i] = 0;
	    OSapi_execv(argv[0], argv);

	} else
	    execl("/bin/sh", "/bin/sh", "-c", sh_command, (char*)0);

	OSapi_perror("vpopen: exec failed\n");
	OSapi__exit(1);
    }
    else			// PARENT
    {
	FILE* stream = NULL;

	if(pid < 0)
	{
	    OSapi_perror("vpopen: cannot vfork");
	    OSapi_close(fd[0]);
	    OSapi_close(fd[1]);
	}
	else
	{
	    int which = (*type == 'w');
	    save_pid(fd[which], pid);
	    stream = OSapi_fdopen(fd[which], which ? "w" : "r");
	    OSapi_close(fd[!which]);
	}
	return stream;
    }
    return NULL;
}

#else

extern "C" FILE *vpopen(char *cmd, char *type)
{
	HANDLE StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE StdIn  = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE StdErr = GetStdHandle(STD_ERROR_HANDLE);

	HANDLE ReadHandle;
	HANDLE WriteHandle;

	SECURITY_ATTRIBUTES sattr;
	memset(&sattr, 0, sizeof(sattr));
	sattr.nLength        = sizeof(sattr);
	sattr.bInheritHandle = TRUE;

	if(!CreatePipe(&ReadHandle, &WriteHandle, &sattr, 0)){
		cerr << "Cannot create pipe" << endl;
		return NULL;
	}

	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb         = sizeof(sinfo);
	sinfo.hStdInput  = StdIn;
	sinfo.hStdOutput = StdOut;
	sinfo.hStdError  = StdErr;
	sinfo.dwFlags    = STARTF_USESTDHANDLES;
	FILE *ret_handle = NULL;
	int file_handle  = -1;
	if(*type == 'r' || *type == 'R'){
	  HANDLE hDup;
    if( DuplicateHandle(GetCurrentProcess(), ReadHandle, 
    GetCurrentProcess(), &hDup, 0, 
    FALSE,                  // not inherited 
    DUPLICATE_SAME_ACCESS) )
    {
      CloseHandle(ReadHandle);
      ReadHandle = hDup;
    }
    file_handle = _open_osfhandle((long)ReadHandle, _O_RDONLY);
		if(file_handle < 0){
			cerr << "Cannot convert pipe handle" << endl;
			return NULL;
		}
		ret_handle = _fdopen(file_handle, "r");
		if(ret_handle == NULL){
			cerr << "Cannot open output file" << endl;
			return NULL;
		}
		sinfo.hStdOutput = WriteHandle;
		if(*type == 'R')
			sinfo.hStdError = WriteHandle;
	} else {
    HANDLE hDup;
    if( DuplicateHandle(GetCurrentProcess(), WriteHandle, 
    GetCurrentProcess(), &hDup, 0, 
    FALSE,                  // not inherited 
    DUPLICATE_SAME_ACCESS) )
    {
      CloseHandle(WriteHandle);
      WriteHandle = hDup;
    }
		file_handle = _open_osfhandle((long)WriteHandle, _O_WRONLY);
		if(file_handle < 0){
			cerr << "Cannot convert pipe handle" << endl;
			return NULL;
		}
		ret_handle = _fdopen(file_handle, "w");
		if(ret_handle == NULL){
			cerr << "Cannot open output file" << endl;
			return NULL;
		}
		sinfo.hStdInput = ReadHandle;
	}

	PROCESS_INFORMATION pinfo;
    
    const char* szCOMSPEC = getenv("COMSPEC");
    if ( !szCOMSPEC || !*szCOMSPEC )
      szCOMSPEC = ( _osver & 0x8000 ) ? "command.com" : "cmd.exe";
  
    char *pure_command = new char[ strlen(szCOMSPEC) +1];
    strcpy( pure_command, szCOMSPEC );
	char *command = new char [ strlen(cmd) + sizeof("/C \"") +3 ];
    strcpy( command, "/C \"" );
	strcat( command, cmd );
    strcat( command, "\"");
	cnv_argstr_2OS( command );

	CreatePIDenv();
	if(!CreateProcess(pure_command, command, NULL, NULL, TRUE,
		CREATE_DEFAULT_ERROR_MODE | /*DETACHED_PROCESS  |*/ NORMAL_PRIORITY_CLASS, GetEnvironmentStrings(),
		NULL, &sinfo, &pinfo)){
		cerr << "Cannot create process " << pure_command << endl;
		CloseHandle(ReadHandle);
		CloseHandle(WriteHandle);
		delete [] pure_command;
		delete [] command;
		return NULL;
	}
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	delete [] pure_command;
	delete [] command;
	if(*type == 'w' || *type == 'W'){
	    save_pid(file_handle, (const int)pinfo.dwProcessId);
	    CloseHandle(ReadHandle);
	} else {
	    save_pid(file_handle, (const int)pinfo.dwProcessId);
	    CloseHandle(WriteHandle);
	}
	return ret_handle;
}

#endif


/**************************************************************************
vpopen2way:  fork a child task, keeping both a write pipe to its stdin,
             and a read pipe from its stdout.

 input:  sh_command - string containing the command for the shell
         read_pipe  - ptr to ptr to a read pipe (from stdout of child)
         write_pipe - ptr to ptr to a write pipe (to stdin of child)
 return: stream pointer to the read pipe

 side effects:  pid of child is saved in process_ids[], a static array
       this pid can be retrieved later from the stream pointer, by using
       vp_find_pid()

**************************************************************************/

#ifndef _WIN32

int vpopen2w3f(int not_std, const char* sh_command,int* read_fd, int* write_fd, int*aux_fd)

{
    int fdr[2];		// read pipe from child's stdout to parent
    int fdw[2];		// write pipe from parent to child's stdin
    int fda[2];         // auxiliary (optional) pipe
    int p1, p2, p3;

    int new_in, new_out, new_aux, new_free;
    if(not_std){
      new_in = 3;
      new_out = 4;
      if (aux_fd) {
	  new_aux = 5;
	  new_free = 6;
      } else {
	  new_aux = -1;
	  new_free = 5;
      }
    } else {
      new_in = 0;
      new_out = 1;
      if (aux_fd) {
	  new_aux = 3;
	  new_free = 4;
      } else {
	  new_aux = -1;
	  new_free = 3;
      }
    }
    fdr[0]=fdr[1]=fdw[0]=fdw[1]=-1;
    if (aux_fd) fda[0]=fda[1]=-1;
    *read_fd = *write_fd = -1;
    if (aux_fd) *aux_fd = -1;
    p1=OSapi_pipe(fdr);
    p2=OSapi_pipe(fdw);
    if (aux_fd) p3=OSapi_pipe(fda);
    if(p1 || p2 || (aux_fd && p3))
    {
	OSapi_perror("vpopen2way: cannot create pipes");
	if (fdr[0]>=0)  OSapi_close(fdr[0]);
	if (fdr[1]>=0)  OSapi_close(fdr[1]);
	if (fdw[0]>=0)  OSapi_close(fdw[0]);
	if (fdw[1]>=0)  OSapi_close(fdw[1]);
	if (aux_fd) {
	    if (fda[0]>=0)  OSapi_close(fda[0]);
	    if (fda[1]>=0)  OSapi_close(fda[1]);
	}
	return NULL;
    }

    const int pid = (const int)vfork();

    if(pid == 0) 		// CHILD
    {
#if 0
	OSapi_fprintf(stderr, "handles are r%d %d w%d %d a%d %d\n",
		fdr[0], fdr[1], fdw[0], fdw[1], fda[0], fda[1]);
#endif
	// fdr is for reading stdout from child
	OSapi_close(fdr[0]);
	if(OSapi_dup2(fdr[1], new_out) < 0)
	{
	    OSapi_perror("vpopen2way: cannot setup stdout for child");
	    OSapi__exit(-1);
	}

	// fdw is for writing stdin to child
	OSapi_close(fdw[1]);
	if(OSapi_dup2(fdw[0], new_in) < 0)
	{
	    OSapi_perror("vpopen2way: cannot setup stdin for child");
	    OSapi__exit(-1);
	}

	// fda is an auxiliary pipe
	if (aux_fd) {
	    OSapi_close(fda[1]);
	    if (OSapi_dup2(fda[0], new_aux) < 0)
	    {
		OSapi_perror("vpopen2way: cannot setup auxiliary filedes for child");
		OSapi__exit(-1);
	    }
	}

	// Note:  getdtablesize() defines number of handles possible,
	//    numbered from 0 to max-1
	int max=OS_dependent::getdtablesize();
#if 0
	OSapi_fprintf(stderr, "There are max %d handles\n",max);
#endif
	// close all the handles not needed by the child
	for(int i = new_free; i < max; i++)
	    OSapi_close(i);
        if(not_std){
	  int pid = getpid();
	  setpgid(pid, pid);
	}

	execl("/bin/sh", "/bin/sh", "-c", sh_command, (char*)0);

	OSapi_perror("vpopen2way: exec failed\n");
	OSapi__exit(1);
    }
    else			// PARENT
    {
	if(pid < 0)
	{
	    OSapi_perror("vpopen2way: cannot vfork");
	    if (fdr[0]>=0)  OSapi_close(fdr[0]);
	    if (fdr[1]>=0)  OSapi_close(fdr[1]);
	    if (fdw[0]>=0)  OSapi_close(fdw[0]);
	    if (fdw[1]>=0)  OSapi_close(fdw[1]);
	    if (aux_fd) {
		if (fda[0]>=0)  OSapi_close(fda[0]);
		if (fda[1]>=0)  OSapi_close(fda[1]);
	    }
	}
	else
	{
	    OSapi_close(fdr[1]);
	    save_pid(fdr[0], pid);
	    *read_fd = fdr[0];

	    OSapi_close(fdw[0]);
	    save_pid(fdw[1], pid);
	    *write_fd = fdw[1];

	    if (aux_fd) {
		OSapi_close(fda[0]);
		save_pid(fda[1], pid);
                *aux_fd = fda[1];
	    }
	}
	return pid;
    }
    return NULL;
}

extern "C" FILE* vpopen2way(
    const char* sh_command, FILE **read_pipe_ptr, FILE **write_pipe_ptr)
{
  int rfd=-1, wfd=-1;

  int pid = vpopen2w(0, sh_command, &rfd, &wfd);
  if(pid > 0){
    *read_pipe_ptr = OSapi_fdopen(rfd, "r");
    *write_pipe_ptr = OSapi_fdopen(wfd, "w");
  } else {
    *read_pipe_ptr = NULL;
    *write_pipe_ptr = NULL;
  }
  return *read_pipe_ptr;
}

#else

int vpopen2w3f(int, const char *cmd, int *read_fd, int *write_fd, int *)
{
	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);

	HANDLE ReadPipeReadHandle;
	HANDLE ReadPipeWriteHandle;
	HANDLE WritePipeReadHandle;
	HANDLE WritePipeWriteHandle;

	SECURITY_ATTRIBUTES sattr;
	memset(&sattr, 0, sizeof(sattr));
	sattr.nLength        = sizeof(sattr);
	sattr.bInheritHandle = TRUE;

	if(!CreatePipe(&ReadPipeReadHandle, &ReadPipeWriteHandle, &sattr, 0)){
		return 0;
	}
	if(!CreatePipe(&WritePipeReadHandle, &WritePipeWriteHandle, &sattr, 0)){
		return 0;
	}
	HANDLE hDup;
	if( DuplicateHandle(GetCurrentProcess(), ReadPipeReadHandle, 
		GetCurrentProcess(), &hDup, 0, 
		FALSE,                  // not inherited 
		DUPLICATE_SAME_ACCESS) )
	{
		CloseHandle(ReadPipeReadHandle);
		ReadPipeReadHandle = hDup;
	}
	int read_pipe_file_handle = _open_osfhandle((long)ReadPipeReadHandle, _O_RDONLY);
	if(read_pipe_file_handle < 0){
		return 0;
	}
	*read_fd = read_pipe_file_handle;
	if( DuplicateHandle(GetCurrentProcess(), WritePipeWriteHandle, 
		GetCurrentProcess(), &hDup, 0, 
		FALSE,                  // not inherited 
		DUPLICATE_SAME_ACCESS) )
	{
		CloseHandle(WritePipeWriteHandle);
		WritePipeWriteHandle = hDup;
	}
	int write_pipe_file_handle = _open_osfhandle((long)WritePipeWriteHandle, _O_WRONLY);
	if(write_pipe_file_handle < 0){
		return 0;
	}
	*write_fd = write_pipe_file_handle;

	PROCESS_INFORMATION pinfo;
	char *command = new char [ strlen(cmd) +1 ];
	strcpy( command, cmd );
	cnv_argstr_2OS( command );
	
	/* Create environment variable with parent pid */
	CreatePIDenv();

	char pset_home[4096];
	GetEnvironmentVariable("PSETHOME", pset_home, sizeof(pset_home));
	genString stub_command;
	char* quote_char="\"";
	if(!strchr(pset_home,' ')) quote_char="";
	stub_command.printf("%s%s\\bin\\ntexestub.exe%s %d %d %s", quote_char, pset_home, quote_char, WritePipeReadHandle,
					ReadPipeWriteHandle, command);

	if(!CreateProcess(NULL, (char *)stub_command, NULL, NULL, TRUE,
			  CREATE_DEFAULT_ERROR_MODE | DETACHED_PROCESS | NORMAL_PRIORITY_CLASS /*| CREATE_NEW_CONSOLE*/,
			  GetEnvironmentStrings(),
		NULL, &sinfo, &pinfo)){
#ifdef SHOW_MSG 
	        // just debugging staff to show the error message if the start process fails 
		LPVOID lpMsgBuf;
		if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				 FORMAT_MESSAGE_FROM_SYSTEM |
				 FORMAT_MESSAGE_IGNORE_INSERTS,
				 NULL,
				 GetLastError(),
				 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				 (LPTSTR) &lpMsgBuf,
				 0,
				 NULL )) {
		      // Display the string.
		      MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
		      // Free the buffer.
		      LocalFree( lpMsgBuf );
		}
#endif
		CloseHandle(ReadPipeReadHandle);
		CloseHandle(ReadPipeWriteHandle);
		CloseHandle(WritePipeReadHandle);
		CloseHandle(WritePipeWriteHandle);
		delete [] command;
		return 0;
	}
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	delete [] command;
//save_pid(read_pipe_file_handle, (const int)pinfo.dwProcessId);
//save_pid(write_pipe_file_handle, (const int)pinfo.dwProcessId);
	CloseHandle(WritePipeReadHandle);
	CloseHandle(ReadPipeWriteHandle);
	return 1; //pinfo.hProcess;
}

extern "C" FILE *vpopen2way(char *cmd, FILE **read_pipe_ptr, FILE **write_pipe_ptr)
{
	HANDLE StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE StdIn  = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE StdErr = GetStdHandle(STD_ERROR_HANDLE);

	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb         = sizeof(sinfo);
	sinfo.hStdInput  = StdIn;
	sinfo.hStdOutput = StdOut;
	sinfo.hStdError  = StdErr;
	sinfo.dwFlags    = STARTF_USESTDHANDLES;

	HANDLE ReadPipeReadHandle;
	HANDLE ReadPipeWriteHandle;
	HANDLE WritePipeReadHandle;
	HANDLE WritePipeWriteHandle;

	*read_pipe_ptr = *write_pipe_ptr = NULL;
	SECURITY_ATTRIBUTES sattr;
	memset(&sattr, 0, sizeof(sattr));
	sattr.nLength        = sizeof(sattr);
	sattr.bInheritHandle = TRUE;

	if(!CreatePipe(&ReadPipeReadHandle, &ReadPipeWriteHandle, &sattr, 0)){
		cerr << "Cannot create pipe" << endl;
		return NULL;
	}
	if(!CreatePipe(&WritePipeReadHandle, &WritePipeWriteHandle, &sattr, 0)){
		cerr << "Cannot create pipe" << endl;
		return NULL;
	}
  HANDLE hDup;
  if( DuplicateHandle(GetCurrentProcess(), ReadPipeReadHandle, 
  GetCurrentProcess(), &hDup, 0, 
  FALSE,                  // not inherited 
  DUPLICATE_SAME_ACCESS) )
  {
    CloseHandle(ReadPipeReadHandle);
    ReadPipeReadHandle = hDup;
  }
  int read_pipe_file_handle = _open_osfhandle((long)ReadPipeReadHandle, _O_RDONLY);
	if(read_pipe_file_handle < 0){
		cerr << "Cannot convert pipe handle" << endl;
		return NULL;
	}
	*read_pipe_ptr = _fdopen(read_pipe_file_handle, "r");
	if(*read_pipe_ptr == NULL){
		cerr << "Cannot open output file" << endl;
		return NULL;
	}
	sinfo.hStdOutput = ReadPipeWriteHandle;
  if( DuplicateHandle(GetCurrentProcess(), WritePipeWriteHandle, 
  GetCurrentProcess(), &hDup, 0, 
  FALSE,                  // not inherited 
  DUPLICATE_SAME_ACCESS) )
  {
    CloseHandle(WritePipeWriteHandle);
    WritePipeWriteHandle = hDup;
  }
	int write_pipe_file_handle = _open_osfhandle((long)WritePipeWriteHandle, _O_WRONLY);
	if(write_pipe_file_handle < 0){
		cerr << "Cannot convert pipe handle" << endl;
		return NULL;
	}
	*write_pipe_ptr = _fdopen(write_pipe_file_handle, "w");	
	if(*write_pipe_ptr == NULL){
		cerr << "Cannot open output file" << endl;
		return NULL;
	}

	sinfo.hStdInput = WritePipeReadHandle;
	PROCESS_INFORMATION pinfo;
	char *command = new char [ strlen(cmd) +1 ];
	strcpy( command, cmd );
	cnv_argstr_2OS( command );

	char *arg = strchr(command, ' ');
	char *pure_command;
	if(arg){
		pure_command = new char[arg - command + 1];
		strncpy(pure_command, command, arg - command);
		pure_command[arg - command] = 0;
		while(*arg == ' ' && *arg != 0)
			arg++;
		if(*arg == 0)
			arg = NULL;
	} else {
		pure_command = new char[strlen(command) + 1];
		strcpy(pure_command, command);
	}
	/* Create environment variable with parent pid */
	CreatePIDenv();
	if(!CreateProcess(pure_command, command, NULL, NULL, TRUE,
		CREATE_DEFAULT_ERROR_MODE | /*DETACHED_PROCESS  |*/ NORMAL_PRIORITY_CLASS, GetEnvironmentStrings(),
		NULL, &sinfo, &pinfo)){
		cerr << "Cannot create process : " << pure_command << endl;
		CloseHandle(ReadPipeReadHandle);
		CloseHandle(ReadPipeWriteHandle);
		CloseHandle(WritePipeReadHandle);
		CloseHandle(WritePipeWriteHandle);
		*write_pipe_ptr = *read_pipe_ptr = NULL;
		delete [] command;
		delete [] pure_command;
		return NULL;
	}
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	delete [] command;
	delete [] pure_command;
	save_pid(read_pipe_file_handle, (const int)pinfo.dwProcessId);
	save_pid(write_pipe_file_handle, (const int)pinfo.dwProcessId);
	CloseHandle(WritePipeReadHandle);
	CloseHandle(ReadPipeWriteHandle);
	return *read_pipe_ptr;
}

#endif

int vpopen2w(int not_std, const char* sh_command, int*rfd, int* wfd)
{
  return vpopen2w3f(not_std, sh_command, rfd, wfd, NULL);
}

/*_______________________________________________________________________
!
! function:	vpclose
!
! Arguments:	FILE* stream		stream created by vpopen
!
! Returns:	int
!
! Synopsis:	finds process spawned by vpopen, and waits for it to
!		stop
!		Returns signal number if successful,
!		     -1 if no such pipe stream
!
_______________________________________________________________________*/

#ifndef _WIN32

extern "C" int vpclose(FILE* stream)
{
    const int pid = vp_find_pid(stream);

    if(pid <= 0)
    {
	msg("vpclose: unknown pipe stream") << eom;
	return -1;
    }
    else
    {
	clear_pid (OSapi_fileno(stream));
	OSapi_fclose(stream);

	int status = 0;
	if(OSapi_waitpid(pid, &status, 0) < 0)
	{
	    int ret_signal;

	    if (OSapi_WIFSTOPPED(status))
	    {
		msg("vpclose: child stopped by signal $1") << (ret_signal = OSapi_WSTOPSIG(status)) << eom;
		return -ret_signal;
	    }
	    else if(OSapi_WIFSIGNALED(status))
	    {
		msg("vpclose: child killed by signal $1") << (ret_signal = OSapi_WTERMSIG(status)) << eom;
		return -ret_signal;
	    }
	    else		// Normal exit detected.
	    {
		return OSapi_WEXITSTATUS(status);
	    }
	}
    }
    return NULL;
}

#else

extern "C" int vpclose(FILE* stream)
{
    int pid               = vp_find_pid(stream);
    HANDLE process_handle = (HANDLE)pid;

    if(pid <= 0)
    {
	OSapi_fprintf(stderr, "vpclose: unknown pipe stream\n");
	return -1;
    }
    else
    {
	clear_pid (OSapi_fileno(stream));
	OSapi_fclose(stream);

	int status = 0;
	
    }
    return NULL;
}

#endif


#ifndef _WIN32

int vfork_execv(const char* path, char*const* arglist)
{
    // save CHILD_DIED handler
    SIG_TYP oldsig = (SIG_TYP) OSapi_signal (SIGCHLD, SIG_DFL);
    int retval = -1;
    const pid_t pid = vfork();
    if(pid == 0) {
	// Child process
        OSapi_closeChildren();
	OSapi_execv(path, arglist);

	OSapi_perror("vsystem: exec failed\n");
    } else if (pid > 0) {
	// Parent process
	int s = 0;
     
	while (1) {
	    if (OSapi_waitpid(pid, &s, 0) == -1) {
		if (errno != EINTR)
		    break;
	    } else {
	        retval = (s >> 8) & 0xff;
		break;
	    }
	}
    } else {
      OSapi_perror("vfork_execv: cannot vfork");
    }

    OSapi_signal (SIGCHLD, oldsig);
    return retval;
}

int vsystem(const char* cmd)
//
// Spawns a child process to make shell call.
// Returns 0 if sucessful, errno value otherwise.
//
{
    Initialize(vsystem);
    const char* arglist[4];
    arglist[0] = "sh";
    arglist[1] = "-c";
    arglist[2] = cmd;
    arglist[3] = NULL;
    int retval = vfork_execv("/bin/sh", (char*const*) arglist);
    return retval;
}

int vfork_execl(const char*path, const char* arg0, ...)  // NULL terminated
{
    const char* argv[1024]; // num_args < 1024
    const char*arg = arg0;
    argv[0] = path;
    int ind = 1;
    va_list ap;
    va_start(ap, arg0);
    while(arg){
      argv[ind] = arg;
      arg = va_arg(ap, const char*);
      ++ind;
    }
    va_end(ap);
    argv[ind] = NULL;

    int retval = vfork_execv(path, (char*const*)argv);
    return retval;
}
#else
int vsystem(const char *cmd)
{
  Initialize(vsystem);
    
	char *command = new char [ strlen(cmd) + 1 ];
	strcpy( command, cmd );
	cnv_argstr_2OS( command );

	_flushall();
  int ret_val = system(command);

	delete [] command;
  return ret_val;
}
#endif

// Used by scripts only!
// Domant code analysis, should find and kill this function
// please do not let it.  PMG.
extern "C" int vsystem_from_script(const char* cmd)
{
	return vsystem(cmd);
}


//
// Backwards compatibility for misnamed function.
//
int v_system(char* cmd)
{
	return vsystem(cmd);
}



extern "C" FILE* vpopen_sync(const char* sh_command, const char* type)

/**************************************************************************
vpopen_sync: execute vsystem and place its out to temporary file

input:	sh_command	string containing a Bourne shell (/bin/sh) command

	type	"r" for a read pipe (from stdout of child),
		"R" for a combined read pipe (from stdout & stderr of child),

return: stream pointer to the open for read temporary file

intended to be used instead of vpopen to be sure that reading output by caller
will not be interrupted (typically by SIGCHLD)

call to this function suppose to have matched call to fclose() 
***************************************************************************/

{
    FILE* retval = NULL;
    static char const *fname = OSPATH( OSapi_tempnam((char *)0, "vpopen_patch") );

    if(!type  ||  !(*type == 'r'  ||  *type == 'R')) {
	msg("vpopen_sync: invalid stream type") << eom;
    } else {

	genString cmd = sh_command;
	if (fname) {
	    cmd += " 1> \"";
	    cmd += fname;
            cmd += "\"";
	    if (*type == 'R') {
		cmd += " 2> \"";
		cmd += fname;
            	cmd += "\"";
	    }	
	    if (vsystem(cmd) == 0)
			{
			retval = OSapi_fopen (fname, "r");
	    }
	}
    }

    return retval;
}

/*
-----------------------------------------
$Log: vpopen.cxx  $
Revision 1.43 2003/07/08 23:03:14EDT Dmitry Ryachovsky (dmitry) 

Revision 1.42 2000/09/21 09:01:30EDT sudha 
add quotes to temporary file name to handle spaces, bug19655 use $ADMINDIR/tmp as temp area 
Revision 1.2.1.20  1994/07/27  21:47:54  aharlap
added vpopen_sync()

Revision 1.2.1.19  1994/05/09  13:49:08  boris
Bug track: 7194
Fixed ParaSET "hang" while starting Frame without a license

Revision 1.2.1.18  1994/05/05  13:49:14  kws
Port

Revision 1.2.1.17  1994/03/14  16:07:00  azaparov
Bug track: N/A
Fixed parser reported bugs

Revision 1.2.1.16  1993/11/30  15:00:26  kws
Fix end of arg list problem

Revision 1.2.1.15  1993/11/30  00:50:42  kws
Bug Track 4162
Allow commands to be invoked without /bin/sh

Revision 1.2.1.14  1993/09/24  21:17:55  kws
Port fix

Revision 1.2.1.13  1993/09/23  13:29:37  kws
Bug track: Port
OS_dependent changes for port

Revision 1.2.1.12  1993/04/22  21:21:29  davea
bug 3461 - added returns at end of 3 functions

Revision 1.2.1.11  1993/04/07  00:06:52  glenn
Do not reject "R" in initial validity test in vpopen.
Subsitute "r" for "R" when calling fdopen in vpopen.

Revision 1.2.1.10  1993/04/06  18:19:01  builder
Add missing "else" in option parsing in vpopen.

Revision 1.2.1.9  1993/04/06  15:21:35  glenn
Add "R" option to vpopen.  Rename v_system to vsystem and provide
an alias for old name.
This is part of a fix for bug #2934.

Revision 1.2.1.8  1993/02/16  19:40:09  davea
In vpopen2way, close all the unneeded handles

Revision 1.2.1.7  1993/02/05  16:42:23  davea
added comments to vpopen(), vpclose(), added new function vpopen2way()

Revision 1.2.1.6  1993/02/02  22:31:17  davea
fixed return if child's exec failed; now it exits

Revision 1.2.1.5  1993/02/02  22:24:24  davea
Commented the synopsis

Revision 1.2.1.4  1992/11/20  22:58:41  oak
Initialized a variable to 0. Fixes
bug #2038.

Revision 1.2.1.3  1992/11/11  14:49:59  oak
Moved the function smt_system to vpopen.c and changed
the function's name to v_system.

Revision 1.2.1.2  1992/10/09  18:18:33  kol
moved &Log

Revision 1.3  92/08/01  14:10:00  smit
added find_pid interface.

Revision 1.2  92/08/01  10:55:50  smit
Initialize uninitialized variable.
Use macros for compatibility reasons.

Revision 1.1  92/06/28  15:44:30  glenn
Initial revision

-----------------------------------------
*/
