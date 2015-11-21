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
#ifdef WIN32

#undef _WINDOWS_

#include <windows.h>
#include <io.h>
#include <fcntl.h>

#define getpid _getpid

extern "C" int getpid();

#else

#include <setprocess.h>

#include "/paraset/include/machdep.h"
#include "/paraset/include/cLibraryFunctions.h"

#ifdef sun4
extern "C" {
#include <vfork.h>
}         
#else
#include <unistd.h>
#endif

#endif

#include <vport.h>
#include <vstdio.h>
#include <vmem.h>
#include "gArray.h"

#include "syspipe.h"

gArrayOf(FILEPID) *System::pids;

#ifdef _WIN32

void CreatePIDenv(void)
{
	char buf[20];

	int pid = getpid();
	sprintf(buf, "%d", pid);
	SetEnvironmentVariable("DIS_PPID", buf);
}

extern "C" void kill_running_processes(void)
{
    int pid;

    for(int i = 0; i < System::pids->size(); i++){
	   FILEPID& pid_ptr = (*System::pids)[i];
	   pid = pid_ptr.pid;
	   HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	   if(h != INVALID_HANDLE_VALUE)
	   TerminateProcess(h, 0);
   }
}

#endif

static void save_pid(const int fd, const int pid)
{
	FILEPID data;

	data.fd = fd;
	data.pid = pid;

	System::pids->append (data);
}

System::System ()
{
    pids = new gArrayOf(FILEPID);
}

System::~System()
{
    delete pids;
}

FILE* System::Open (const char* command, const char* type)
{
#ifndef _WIN32
    int no_sh = 0;
    if (*type == '-') {
        type++;
        no_sh = 1;
    }

    if(!type  ||  !(*type == 'r'  ||  *type == 'R'  ||  *type == 'w'))
    {
	fprintf(stderr, "vpopen: invalid stream type\n");
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

	    strcpy (cmd, command);
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
	    execl("/bin/sh", "/bin/sh", "-c", command, (char*)0);

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
#else

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
	    fprintf (stderr, "Cannot create pipe\n");
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
    if (*type == 'r' || *type == 'R') {
        file_handle = _open_osfhandle((long)ReadHandle, _O_RDONLY);
        if(file_handle < 0) {
            fprintf (stderr, "Cannot convert pipe handle\n");
            return NULL;
    }
         
    ret_handle = _fdopen(file_handle, "r");
    if (ret_handle == NULL) {
        fprintf (stderr, "Cannot open output file\n");
        return NULL;
    }
    sinfo.hStdOutput = WriteHandle;
    if(*type == 'R')
        sinfo.hStdError = WriteHandle;
    } else {
        file_handle = _open_osfhandle((long)WriteHandle, _O_WRONLY);
        if (file_handle < 0) {
	        fprintf (stderr, "Cannot convert pipe handle\n");
	        return NULL;
        }
        ret_handle = _fdopen(file_handle, "w");
        if(ret_handle == NULL){
	    fprintf (stderr, "Cannot open output file\n");
	    return NULL;
        }
        sinfo.hStdInput = ReadHandle;
    }

    PROCESS_INFORMATION pinfo;
    char *arg = strchr(command, ' ');
    char *pure_command;
    if(arg) {
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

    CreatePIDenv();

    if (!CreateProcess(pure_command, (char*)command, NULL, NULL, TRUE,
        CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS, 
        GetEnvironmentStrings(), NULL, &sinfo, &pinfo)) {

        fprintf (stderr, "Cannot create process %s\n", pure_command);
        CloseHandle(ReadHandle);
        CloseHandle(WriteHandle);
        delete [] pure_command;
        return NULL;
    }
    CloseHandle(pinfo.hThread);
    CloseHandle(pinfo.hProcess);
    delete [] pure_command;
    if (*type == 'w') {
        save_pid(file_handle, (const int)pinfo.dwProcessId);
        CloseHandle(ReadHandle);
    } else {
        save_pid(file_handle, (const int)pinfo.dwProcessId);
        CloseHandle(WriteHandle);
    }
    return ret_handle;
#endif
}

int System::Open2Way (const char* command, FILE** read_pipe, FILE** write_pipe) 
{
	return -1;
}

int System::Close (FILE*) 
{
	return -1;
}

int System::FindPid (FILE* f)
{
    for (int i=0; i<System::pids->size(); i++) {
		if ((*System::pids)[i].file == f)
			return (*System::pids)[i].pid;
    }

   return 0;
}




#if 0


// VARIABLE DEFINITIONS

static genArr(int) process_ids;

// FUNCTION DEFINITIONS


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

extern "C" FILE* vpopen2way(
    const char* sh_command, FILE **read_pipe_ptr, FILE **write_pipe_ptr)
{
    int fdr[2];		// read pipe from child's stdout to parent
    int fdw[2];		// write pipe from parent to child's stdin
    int p1, p2;
    fdr[0]=fdr[1]=fdw[0]=fdw[1]=-1;
    *read_pipe_ptr = *write_pipe_ptr = NULL;
    p1=OSapi_pipe(fdr);
    p2=OSapi_pipe(fdw);
    if(p1 || p2)
    {
	OSapi_perror("vpopen2way: cannot create pipes");
	if (fdr[0]>=0)  OSapi_close(fdr[0]);
	if (fdr[0]>=0)  OSapi_close(fdr[1]);
	if (fdw[0]>=0)  OSapi_close(fdw[0]);
	if (fdw[0]>=0)  OSapi_close(fdw[1]);
	return NULL;
    }

#ifdef _WIN32
    const int pid = (const int)fork();
#else
    const int pid = (const int)vfork();
#endif
    if(pid == 0)		// CHILD
    {
#if 0
	fprintf(stderr, "handles are r%d %d w%d %d\n",
		fdr[0], fdr[1], fdw[0], fdw[1]);
#endif
	// fdr is for reading stdout from child
	OSapi_close(fdr[0]);
	if(OSapi_dup2(fdr[1], 1) < 0)
	{
	    OSapi_perror("vpopen2way: cannot setup stdout for child");
	    OSapi__exit(-1);
	}

	// fdw is for writing stdin to child
	OSapi_close(fdw[1]);
	if(OSapi_dup2(fdw[0], 0) < 0)
	{
	    OSapi_perror("vpopen2way: cannot setup stdin for child");
	    OSapi__exit(-1);
	}

	// Note:  getdtablesize() defines number of handles possible,
	//    numbered from 0 to max-1
	int max=OS_dependent::getdtablesize();
#if 0
	fprintf(stderr, "There are max %d handles\n",max);
#endif
	// close all the handles not needed by the child
	for(int i = 3; i < max; i++)
	    OSapi_close(i);

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
	    if (fdr[0]>=0)  OSapi_close(fdr[1]);
	    if (fdw[0]>=0)  OSapi_close(fdw[0]);
	    if (fdw[0]>=0)  OSapi_close(fdw[1]);
	}
	else
	{
	    OSapi_close(fdr[1]);
	    save_pid(fdr[0], pid);
	    *read_pipe_ptr = OSapi_fdopen(fdr[0], "r");

	    OSapi_close(fdw[0]);
	    save_pid(fdw[1], pid);
	    *write_pipe_ptr = OSapi_fdopen(fdw[1], "w");
	}
	return *read_pipe_ptr;
    }
    return NULL;
}

#else

extern "C" FILE *vpopen2way(char *command, FILE **read_pipe_ptr, FILE **write_pipe_ptr)
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
		CREATE_DEFAULT_ERROR_MODE | DETACHED_PROCESS  | NORMAL_PRIORITY_CLASS, GetEnvironmentStrings(),
		NULL, &sinfo, &pinfo)){
		cerr << "Cannot create process : " << pure_command << endl;
		CloseHandle(ReadPipeReadHandle);
		CloseHandle(ReadPipeWriteHandle);
		CloseHandle(WritePipeReadHandle);
		CloseHandle(WritePipeWriteHandle);
		*write_pipe_ptr = *read_pipe_ptr = NULL;
		delete [] pure_command;
		return NULL;
	}
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	delete [] pure_command;
	save_pid(read_pipe_file_handle, (const int)pinfo.dwProcessId);
	save_pid(write_pipe_file_handle, (const int)pinfo.dwProcessId);
	CloseHandle(WritePipeReadHandle);
	CloseHandle(ReadPipeWriteHandle);
	return *read_pipe_ptr;
}

#endif


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
	fprintf(stderr, "vpclose: unknown pipe stream\n");
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
		fprintf(stderr, "vpclose: child stopped by signal %d\n",
			(ret_signal = OSapi_WSTOPSIG(status)));
		return -ret_signal;
	    }
	    else if(OSapi_WIFSIGNALED(status))
	    {
		fprintf(stderr, "vpclose: child killed by signal %d\n",
			(ret_signal = OSapi_WTERMSIG(status)) );
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
	fprintf(stderr, "vpclose: unknown pipe stream\n");
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
int vsystem(const char* cmd)
//
// Spawns a child process to make shell call.
// Returns 0 if sucessful, errno value otherwise.
//
{
    Initialize(vsystem);
    const char*  arglist[4];
    arglist[0] = "sh";
    arglist[1] = "-c";
    arglist[2] = cmd;
    arglist[3] = NULL;
    int retval = -1;

    // save CHILD_DIED handler
    SIG_TYP oldsig = (SIG_TYP) OSapi_signal (SIGCHLD, SIG_DFL);

#ifdef _WIN32
    const pid_t pid = fork();
#else
    const pid_t pid = vfork();
#endif
    if(pid == 0) {
	// Child process
        OSapi_closeChildren;
	OSapi_execv("/bin/sh", (char *const *)arglist);

	OSapi_perror("vsystem: exec failed\n");
    } else {
	// Parent process
	int s = 0;
     
	while (1) {
	    if (OSapi_waitpid(pid, &s, 0) == -1) {
		if (errno != EINTR)
		    break;
	    } else {
		retval = s;
		break;
	    }
	}
    }

    OSapi_signal (SIGCHLD, oldsig);
    return retval;
}
#else
int vsystem(const char *cmd)
{
    Initialize(vsystem);
    
    int ret_val = system(cmd);
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

int v_system(char* cmd)
//
// Backwards compatibility for misnamed function.
//
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
    static char* fname = OSapi_tempnam((char *)0, "vpopen_patch");

    if(!type  ||  !(*type == 'r'  ||  *type == 'R')) {
	fprintf(stderr, "vpopen_sync: invalid stream type\n");
    } else {
	genString cmd = sh_command;
	if (fname) {
	    cmd += " 1>";
	    cmd += fname;
	    if (*type == 'R') {
		cmd += " 2>";
		cmd += fname;
	    }	
	    if (vsystem(cmd) == 0) {
		retval = OSapi_fopen (fname, "r");
	    }
	}
    }

    return retval;
}
#endif

#if 0
int main(int argc, char** argv)
{
	System sys;

    FILE* f = sys.Open("\\mksnt\\ls.exe", "r");
	char buffer[456];

	if (f) {
		while (fgets (buffer, 256, f))
		printf ("->%s", buffer);
	}

	return 0;
}
#endif
