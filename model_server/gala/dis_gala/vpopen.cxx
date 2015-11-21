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

#include <cLibraryFunctions.h>
#include <machdep.h>

#ifdef _WIN32
#include <Application.h>
#endif

#ifndef _WIN32
#ifdef hp700
#include <sys/errno.h>
#else
#include <sysent.h>
#endif
#else
#include <windows.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#include <stdlib.h>
#endif
#include <gArray.h>
#include <gString.h>
#ifdef sun4
extern "C" {
#include <vfork.h>
}
#elif !defined(_WIN32)
#include <unistd.h>
#endif

#ifdef WIN32
#define DEVNULL "NUL"
#else
#define DEVNULL "/dev/null"
#endif

#if defined(ibm) || defined (__GNUG__)
        typedef void SIG_FUNC_TYP(int);
        typedef SIG_FUNC_TYP *SIG_TYP;
#endif

// VARIABLE DEFINITIONS

static gArray(int) process_ids;

// Info for child death reporters

typedef void notifier_function();

static notifier_function* notifier = NULL;
static int last_pid = -1;
static SIG_TYP old_sig_handler = NULL;
#ifdef _WIN32
int spawn_minimized( const char * );
int set_termination_handler( int, notifier_function * );
HANDLE hProcessToWait = NULL;
#endif

// FUNCTION DEFINITIONS

void signal_handler(int)
{
#ifndef WIN32
    int status;

    int pid  = OSapi_waitpid(-1, &status, 0);

    if (pid > 0 && pid == last_pid) {
	pid = -1;
	if (notifier) notifier();
        signal (SIGCHLD, (SIG_TYP)old_sig_handler);
	
    }
#endif
}

static void save_pid(const int fd, const int pid)
{
    if (process_ids.size() <= fd)
	    process_ids.grow(fd - process_ids.size() + 1);
    process_ids[fd] = pid;
}

static void clear_pid(const int fd)
{
    int pid = 0;
	if (fd >= 0 && fd < process_ids.size())
    process_ids[fd] = 0;
}


extern "C" int vp_find_pid (FILE* f)
{

    int pid = 0;
    if (f) {
        int fd = OSapi_fileno(f);
        if (fd >= 0 && fd < process_ids.size())
            pid = process_ids[fd];
    }
    return pid;
}

#ifdef _WIN32

static void CreatePIDenv(void)
{
	char buf[20];

	int pid = OSapi_getpid();
	sprintf(buf, "%d", pid);
	SetEnvironmentVariable("PSET_PPID", buf);
}

extern "C" void kill_running_processes(void)
{
    int pid;

    for(int i = 0; i < process_ids.size(); i++){
	int* pid_ptr = &process_ids[i];
	if(pid_ptr){
	    pid      = *pid_ptr;
	    //HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	    //if(h != INVALID_HANDLE_VALUE);
      TerminateProcess((HANDLE)pid, 0); //pid we save on NT is really a process handle - mstarets
	}
    }
}
#endif


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

extern "C" FILE* vpopen_tmp(const char* sh_command, const char* type, const char* ofile, notifier_function* handler);

extern "C" FILE *vpopen(const char *command, const char* type)
{
	return vpopen_tmp (command, type, NULL, NULL);
}

FILE* vpopen_tmp(const char* sh_command, const char* type, const char* file, notifier_function* handler)
{

    notifier = handler;

    int use_shell = 1;
    if (*type == '-') {
        type++;
        use_shell = 0;
    }

    FILE* fileptr;

    int fd[3];
    fd[0] = -1;
    fd[1] = -1;
    fd[2] = -1;
    FILE* outfile = NULL;
    FILE* infile = NULL;

    if (*type == '>') {
  char *mode = "w+";
  if( *(type + 1) == 'a' )
    mode = "a+";
	outfile =  OSapi_fopen (file, mode);
	if (!outfile) {
		OSapi_perror ("vpopen: cannot write file");	
		return NULL;
	}

        fileptr = outfile;
        fd[0] = OSapi_open(DEVNULL, O_RDONLY);
        fd[1] = OSapi_fileno(outfile);
	fd[2] = fd[1];

    } else if (*type == '<') {
        infile =  OSapi_fopen (file, "r");
        if (!infile) {
            OSapi_perror ("vpopen: cannot read file");	
            return NULL;
        }

        fileptr = infile;
        fd[0] = dup(OSapi_fileno(infile));
        fd[1] = OSapi_open (DEVNULL, O_WRONLY);
	fd[2] = fd[1];

    } else if (*type == 'r') {
        if (OSapi_pipe(fd)) {
            OSapi_perror("vpopen: cannot create pipe");
            return NULL;
        }

        fileptr = OSapi_fdopen(fd[1], "w+");
	fd[0] = OSapi_open (DEVNULL, O_RDONLY);
	fd[2] = dup (2);

    } else if (*type == 'R') {
        if (OSapi_pipe(fd)) {
            OSapi_perror("vpopen: cannot create pipe");
            return NULL;
        }

        fileptr = OSapi_fdopen(fd[1], "w+");
	fd[0] = OSapi_open (DEVNULL, O_RDONLY);
	fd[2] = fd[1];

    } else if (*type == 'w') {
	if (OSapi_pipe(fd)) {
	    OSapi_perror("vpopen: cannot create pipe");
	    return NULL;
	}

        fileptr = OSapi_fdopen(fd[0], "r");
	fd[1] = OSapi_open (DEVNULL, O_RDONLY);
	fd[2] = dup(fd[1]);

   } else return NULL;

    int oldin = dup(0);
    int oldout = dup(1);
    int olderr = dup(2);

    OSapi_close (0);
    OSapi_close (1);
    OSapi_close (2);

    OSapi_dup2 (fd[0], 0);
    OSapi_dup2 (fd[1], 1);
    OSapi_dup2 (fd[2], 2);
    
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

#ifdef WIN32

    //last_pid = _spawnvp(_P_NOWAIT, argv[0], (const char* const *)argv);
    last_pid = spawn_minimized( sh_command );
    if( (last_pid != -1) && handler )
      set_termination_handler( last_pid, handler ); 
#else

    old_sig_handler = signal (SIGCHLD, (SIG_TYP)signal_handler);

    last_pid = (int)vfork();
 
    if (last_pid == 0) {
	OSapi_close (oldin);
	OSapi_close (oldout);
	OSapi_close (olderr);

        OSapi_execv (argv[0], argv);

	OSapi__exit(-1);
   }
#endif

    OSapi_close(0);
    OSapi_close(1);
    OSapi_close(2);

    OSapi_dup2 (oldin, 0);
    OSapi_dup2 (oldout, 1);
    OSapi_dup2 (olderr, 2);

    OSapi_close(oldin);
    OSapi_close(oldout);
    OSapi_close(olderr);

    if (last_pid < 0) {
        fclose (fileptr);
	fileptr = NULL;
    }

    return fileptr;
}


#if 0
        } else if(*type == 'w' || *type == '<') {
	        OSapi_close(fd[1]);
	        if(OSapi_dup2(fd[0], 0) < 0) {
	            OSapi_perror("vpopen: cannot setup stdin for child");
	            OSapi__exit(-1);
	        }
        }

        if (outfile) OSapi_fclose(outfile);
        if (infile) OSapi_fclose(infile);


        if (use_shell) {
	        execl("/bin/sh", "/bin/sh", "-c", sh_command, (char*)0);
    
	        OSapi_perror("vpopen: exec failed\n");
	        OSapi__exit(1);
        } else {
	}
}
#endif

/*
#if 0
else {
#if 0
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
	    } else {
	        gString cmd = ((vchar*)sh_command);
	        if (ofile) {
		    if (tu[e 
	    	    cmd += " ">&*"
    
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
#endif


#else

FILE* vpopen_tmp(const char *command, const char* type, const char* ofile, void*)
{ 
	gString cmd;
	if (ofile) {
		cmd = (vchar*)"start ";
		cmd += (vchar*)command;
		cmd += (vchar*)" > ";
		cmd += (vchar*)ofile;
		printf ("system = %d\n", system ((char*)(vchar*)cmd));
		return NULL;
	}

	HANDLE StdOut;
	HANDLE StdIn; 
	HANDLE StdErr;

    StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	StdIn  = GetStdHandle(STD_INPUT_HANDLE);
	StdErr = GetStdHandle(STD_ERROR_HANDLE);

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
	CreatePIDenv();
	if(!CreateProcess(pure_command, (char*)command, NULL, NULL, TRUE,
		CREATE_DEFAULT_ERROR_MODE | /*DETACHED_PROCESS  |*/ /*NORMAL_PRIORITY_CLASS, GetEnvironmentStrings(),
		NULL, &sinfo, &pinfo)){
		cerr << "Cannot create process " << pure_command << endl;
		CloseHandle(ReadHandle);
		CloseHandle(WriteHandle);
		delete [] pure_command;
		return NULL;
	}
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	delete [] pure_command;
	if(*type == 'w'){
	    save_pid(file_handle, (const int)pinfo.dwProcessId);
	    CloseHandle(ReadHandle);
	} else {
	    save_pid(file_handle, (const int)pinfo.dwProcessId);
	    CloseHandle(WriteHandle);
	}
	return ret_handle;
}

#endif
*/

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
	OSapi_fprintf(stderr, "handles are r%d %d w%d %d\n",
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
	OSapi_fprintf(stderr, "There are max %d handles\n",max);
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

FILE *vpopen2way(char *command, FILE **read_pipe_ptr, FILE **write_pipe_ptr)
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
		CREATE_DEFAULT_ERROR_MODE | /*DETACHED_PROCESS  |*/ NORMAL_PRIORITY_CLASS, GetEnvironmentStrings(),
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
	OSapi_fprintf(stderr, "vpclose: unknown pipe stream\n");
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
		OSapi_fprintf(stderr, "vpclose: child stopped by signal %d\n",
			(ret_signal = OSapi_WSTOPSIG(status)));
		return -ret_signal;
	    }
	    else if(OSapi_WIFSIGNALED(status))
	    {
		OSapi_fprintf(stderr, "vpclose: child killed by signal %d\n",
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
int vsystem(const char* cmd)
//
// Spawns a child process to make shell call.
// Returns 0 if sucessful, errno value otherwise.
//
{
    //gInit(vsystem);
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
    //gInit(vsystem);
    
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

extern "C" void vpkill (int, int)
{
}


// spawn_redirected will execute a command (without spawning a shell)
// and redirect its output to a file.
// It returns pid of the newly created process

#ifndef _WIN32
int spawn_redirected( const char *pszCommand, const char *pszOutFile )
{
  int nPid = -1;
  FILE *fResult = vpopen_tmp( pszCommand, ">a", pszOutFile, NULL );
  if( fResult )
  {
    nPid = last_pid;
    OSapi_fclose( fResult );
  }
  return nPid;
}

#else
int spawn_redirected( const char *pszCommand, const char *pszOutFile )
{
  int nPid = -1;

  SECURITY_ATTRIBUTES sa;
  ZeroMemory( &sa, sizeof(sa) );
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;
  
  HANDLE hRedirect = CreateFile( pszOutFile, GENERIC_WRITE, 
                FILE_SHARE_READ|FILE_SHARE_WRITE, &sa,
                OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL );
  
  if( hRedirect != INVALID_HANDLE_VALUE )
  {
    SetFilePointer( hRedirect, 0, NULL, FILE_END );
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory( &si, sizeof( si ) );
    ZeroMemory( &pi, sizeof( pi ) );
    si.cb = sizeof( si );
    si.hStdOutput = hRedirect;
    si.hStdError = hRedirect;
    si.hStdInput = INVALID_HANDLE_VALUE;
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    BOOL fResult = CreateProcess( NULL, (char *)pszCommand, NULL, NULL, TRUE, /*inherit handles*/ 
                           0, NULL, NULL, &si, &pi );

    CloseHandle( hRedirect );

    if( fResult )
      nPid = (int)pi.hProcess;
  }
  return nPid;
}
#endif

#ifndef _WIN32
int process_is_active( int nPid )
{
  //returns 1 if process is active, 0 otherwise
  // you shouldn't try to kill a -1 pid...
  int ret_val=-1;
  if (nPid>=0)
    ret_val = OSapi_kill(nPid, 0);
  if (ret_val<0 && errno==EPERM)
    ret_val = 0;	
  return (ret_val==0);
}
#else
int process_is_active( int nPid )
{
  int nResult = 0;
  DWORD nExitStatus = 0;
  if( nPid != -1 && GetExitCodeProcess( (HANDLE)nPid, &nExitStatus )
      && nExitStatus == STILL_ACTIVE )
    nResult = 1;
  return nResult;
}
#endif

#ifndef _WIN32
void process_terminate( int nPid )
{
  OSapi_kill(nPid, SIGKILL);
}
#else
void process_terminate( int nPid )
{
  TerminateProcess( (HANDLE)nPid, 0 );
}
#endif

#ifdef _WIN32
int spawn_minimized( const char *pszCommand )
{
  int nReturn = -1;
  PROCESS_INFORMATION pi;
  char pszFile[ MAX_PATH ];
  STARTUPINFO si;
  ZeroMemory( &si, sizeof( si ) );
  ZeroMemory( &pi, sizeof( pi ) );
  si.cb = sizeof( si );
  BOOL fInherit = FALSE;
  DWORD dCreationFlags = 0;
  HANDLE hRedirect = INVALID_HANDLE_VALUE;
  
  DWORD nRet = GetEnvironmentVariable( "DISCOVER_XTERM", pszFile, sizeof( pszFile ) );
  if( nRet && nRet < sizeof( pszFile ) )
  {
    if( !_stricmp( pszFile, "console" ) )
      dCreationFlags = CREATE_NEW_CONSOLE;
    else
    {
      SECURITY_ATTRIBUTES sa;
      ZeroMemory( &sa, sizeof(sa) );
      sa.nLength = sizeof(sa);
      sa.bInheritHandle = TRUE;
      hRedirect = CreateFile( pszFile, GENERIC_WRITE|GENERIC_READ, 
                    FILE_SHARE_READ|FILE_SHARE_WRITE, &sa,
                    CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL );
      if( hRedirect != INVALID_HANDLE_VALUE )
      {
        si.hStdOutput = hRedirect;
        si.hStdError = hRedirect;
        si.hStdInput = INVALID_HANDLE_VALUE;
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        fInherit = TRUE;
        si.wShowWindow = SW_HIDE;
      }
      else
        dCreationFlags = CREATE_NEW_CONSOLE;
    }
  }
  else
  {
//#ifndef _DEBUG
    //don't show console and don't redirect output to a file
    dCreationFlags = CREATE_NEW_CONSOLE;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
//#endif
  }

  BOOL fResult = CreateProcess( NULL, (char *)pszCommand, NULL, NULL, fInherit, /*inherit handles*/ 
                           dCreationFlags, NULL, NULL, &si, &pi );

  if( hRedirect != INVALID_HANDLE_VALUE )
    CloseHandle( hRedirect );

  if( fResult )
    nReturn = (int)pi.hProcess;

  return nReturn;
}

static void thread_control_func( void *param )
{
  callbackEvent *pEvent = (callbackEvent *) param;
  if ( hProcessToWait && WaitForSingleObject( hProcessToWait, INFINITE ) == WAIT_OBJECT_0 )
  {
    if( pEvent )
      pEvent->PostAtInterrupt();
    //After we posted a message, wake up main thread's event loop
    veventCancelWaitProc cancel_wait_proc = vevent::GetCancelWaitProc();
    if( cancel_wait_proc )
      cancel_wait_proc();
  }
  _endthread();
}

int set_termination_handler( int nProcess, notifier_function *pFunc )
{
  int nResult = 0;
  hProcessToWait = (HANDLE) nProcess;
  callbackEvent *pEvent = new callbackEvent( pFunc );
  nResult = (int)_beginthread( thread_control_func, 0, (void *)pEvent );
  return nResult;
}
#endif
