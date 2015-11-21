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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <pduio.h>
#include <pdufile.h>
#include <pdumem.h>
#include <pdupath.h>
#include <pduproc.h>
#include <pdustring.h>
#ifdef _WIN32
#include <windows.h>
#endif

char const *get_psethome(char const *disperl)
{
  char const *retval = pdgetenv("PSETHOME");

  if ( ! retval )
    {
      // get value of PSETHOME from path of executable (disperl will be $PSETHOME/bin/disperl)
      char *disperl_path = 0;
      if ( ! pdabspath(disperl) )
	{
	  char path[1025];
	  disperl_path = pdmpack("scs", "%s%c%s", pdgetcwd(path, 1024), PATH_FILE_SEP_CH, disperl);
	  if (!pdisfile(disperl_path)) {
	    FREE_MEMORY(disperl_path);
	    disperl_path = 0;
	  }
	}
      else
	{
	  disperl_path = pdstrdup(disperl);
	}

      if ( disperl_path )
	{
	  char *lookfor = pdmpack("c", "%cdisperl", PATH_FILE_SEP_CH);
	  char *where   = (char *) pdstrstr(disperl_path, lookfor);
	  if ( where )
	    {
	      where[1] = '.';
	      where[2] = '.';
	      where[3] = '\0';
	      retval = disperl_path;
	    }
	  FREE_MEMORY(lookfor);
	}
      if(retval) {
	  // will add this string to environment -- should not be freed
	  char *psethomeVal = pdmpack("s", "PSETHOME=%s", retval);
	  pdputenv(psethomeVal);
      }
    }

  return retval;
}

// Add pathelement to the begining of $PATH if it is not present.
void set_PATH_environment_variable(char const* pathelement)
{
    static char *prev_path_env = 0;

    char const *old_path = pdgetenv("PATH");
    if (pdstrncmp(old_path, pathelement, pdstrlen(pathelement), 0) != 0) {
        // will add this string to environment -- should not be freed
#ifdef _WIN32
        char *path_env = pdmpack("ss", "PATH=%s;%s", pathelement, old_path);
#else
        char *path_env = pdmpack("ss", "PATH=%s:%s", pathelement, old_path);
#endif
	pdputenv((char *)path_env);

        // if we have already allocated memory - free it
	if(prev_path_env!=0) FREE_MEMORY(prev_path_env);
	prev_path_env = path_env;
    }
}

#ifdef _WIN32

HANDLE hChildProcess = NULL;
HANDLE hStdIn = NULL; // Handle to parents std input.
BOOL bRunThread = TRUE;
HANDLE hFile = NULL; // Handle to the file to write output to

   /////////////////////////////////////////////////////////////////////// 
   // DisplayError
   // Displays the error number and corresponding message.
   /////////////////////////////////////////////////////////////////////// 
   void DisplayError(char *pszAPI,HANDLE hOutputFile)
   {
       LPVOID lpvMessageBuffer;
       CHAR szPrintBuffer[512];
       DWORD nCharsWritten;

       FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpvMessageBuffer, 0, NULL);

       wsprintf(szPrintBuffer,
         "ERROR: API    = %s.\n   error code = %d.\n   message    = %s.\n",
                pszAPI, GetLastError(), (char *)lpvMessageBuffer);

       WriteFile(hOutputFile,szPrintBuffer,
                     lstrlen(szPrintBuffer),&nCharsWritten,NULL);

       LocalFree(lpvMessageBuffer);
	   CloseHandle(hOutputFile);
       ExitProcess(GetLastError());
   }

   ///////////////////////////////////////////////////////////////////////
   // PrepAndLaunchRedirectedChild
   // Sets up STARTUPINFO structure, and launches redirected child.
   /////////////////////////////////////////////////////////////////////// 
   void PrepAndLaunchRedirectedChild(char* szCmd,
									 HANDLE hChildStdOut,
                                     HANDLE hChildStdIn,
                                     HANDLE hChildStdErr,
									 HANDLE hOutputFile
									)
   {
      PROCESS_INFORMATION pi;
      STARTUPINFO si;

      // Set up the start up info struct.
      ZeroMemory(&si,sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);
      // Use this if you want to hide the child:
      si.wShowWindow = SW_HIDE;
      // Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
      // use the wShowWindow flags.
      si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
      si.hStdOutput = hChildStdOut;
      si.hStdInput  = hChildStdIn;
      si.hStdError  = hChildStdErr;

      // Launch the process that you want to redirect
      if (!CreateProcess(NULL,szCmd,NULL,NULL,TRUE,
                         0,NULL,NULL,&si,&pi))
         DisplayError("CreateProcess",hOutputFile);


      // Set global child process handle to cause threads to exit.
      hChildProcess = pi.hProcess;


      // Close any unnecessary handles.
      if (!CloseHandle(pi.hThread)) DisplayError("CloseHandle pi.hThread",hOutputFile);
   }

   /////////////////////////////////////////////////////////////////////// 
   // ReadAndHandleOutput
   // Monitors handle for input. Exits when child exits or pipe breaks.
   ///////////////////////////////////////////////////////////////////////
   void ReadAndHandleOutput(HANDLE hPipeRead,HANDLE hOutputFile)
   {
      CHAR lpBuffer[256];
      DWORD nBytesRead;
      DWORD nCharsWritten;

      while(TRUE)
      {
         if (!ReadFile(hPipeRead,lpBuffer,sizeof(lpBuffer),
                                          &nBytesRead,NULL) || !nBytesRead)
         {
            if (GetLastError() == ERROR_BROKEN_PIPE)
               break; // pipe done - normal exit path.
            else
               DisplayError("ReadFile ReadAndHandleOutput",hOutputFile); // Something bad happened.
         }

         // Display the character read on the screen.
         if (!WriteFile(hOutputFile,lpBuffer,
                           nBytesRead,&nCharsWritten,NULL))
            DisplayError("WriteFile ReadAndHandleOutput",hOutputFile);
      }
   }

   /////////////////////////////////////////////////////////////////////// 
   // GetAndSendInputThread
   // Thread procedure that monitors the console for input and sends input
   // to the child process through the input pipe.
   // This thread ends when the child application exits.
   /////////////////////////////////////////////////////////////////////// 
   DWORD WINAPI GetAndSendInputThread(LPVOID lpvThreadParam)
   {
      CHAR read_buff[256];
      DWORD nBytesRead,nBytesWrote;
      HANDLE hPipeWrite = (HANDLE)lpvThreadParam;

      // Get input from our console and send it to child through the pipe.
      while (bRunThread)
      {
         if(!ReadFile(hStdIn,read_buff,1,&nBytesRead,NULL)) ;
            DisplayError("ReadConsole GetAndSendInputThread",hFile);

         read_buff[nBytesRead] = '\0'; // Follow input with a NULL.

         if (!WriteFile(hPipeWrite,read_buff,nBytesRead,&nBytesWrote,NULL))
         {
            if (GetLastError() == ERROR_NO_DATA)
               break; // Pipe was closed (normal exit path).
            else ;
            DisplayError("WriteFile GetAndSendInputThread",hFile);
         }
      }

      return 1;
   }

int RunOutputRedirected(char* szFile, char* cmd) 
{
      HANDLE hOutputReadTmp,hOutputRead,hOutputWrite;
      HANDLE hInputWriteTmp,hInputRead,hInputWrite;
      HANDLE hErrorWrite;
      HANDLE hThread;
      DWORD ThreadId;
      SECURITY_ATTRIBUTES sa;	
	  BOOL bRedirectInput = FALSE;
	  
	  hFile = CreateFile(szFile,           // create MYFILE.TXT 
             GENERIC_WRITE ,               // open for writing 
             FILE_SHARE_READ,              // do not share 
             NULL,                         // no security 
             CREATE_ALWAYS,                // overwrite existing 
             FILE_ATTRIBUTE_NORMAL,        // normal file 
             NULL);                        // no attr. template 

	  if (hFile == INVALID_HANDLE_VALUE) 
	  { 
		return -1;  // process error 
	  } 

      // Set up the security attributes struct.
      sa.nLength= sizeof(SECURITY_ATTRIBUTES);
      sa.lpSecurityDescriptor = NULL;
      sa.bInheritHandle = TRUE;


      // Create the child output pipe.
      if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0))
         DisplayError("CreatePipe hOutputWrite",hFile);


      // Create a duplicate of the output write handle for the std error
      // write handle. This is necessary in case the child application
      // closes one of its std output handles.
      if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite,
                           GetCurrentProcess(),&hErrorWrite,0,
                           TRUE,DUPLICATE_SAME_ACCESS))
         DisplayError("DuplicateHandle hOutputWrite",hFile);


      // Create the child input pipe.
      if (!CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0))
         DisplayError("CreatePipe hInputRead",hFile);


      // Create new output read handle and the input write handles. Set
      // the Properties to FALSE. Otherwise, the child inherits the
      // properties and, as a result, non-closeable handles to the pipes
      // are created.
      if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
                           GetCurrentProcess(),
                           &hOutputRead, // Address of new handle.
                           0,FALSE, // Make it uninheritable.
                           DUPLICATE_SAME_ACCESS))
         DisplayError("DupliateHandle hInputRead",hFile);

      if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
                           GetCurrentProcess(),
                           &hInputWrite, // Address of new handle.
                           0,FALSE, // Make it uninheritable.
                           DUPLICATE_SAME_ACCESS))
      DisplayError("DupliateHandle hInputWrite",hFile);


      // Close inheritable copies of the handles you do not want to be
      // inherited.
      if (!CloseHandle(hOutputReadTmp)) DisplayError("CloseHandle hOutputReadTmp",hFile);
      if (!CloseHandle(hInputWriteTmp)) DisplayError("CloseHandle hInputWriteTmp",hFile);


	  if(bRedirectInput) {
	      // Get std input handle so you can close it and force the ReadFile to
    	  // fail when you want the input thread to exit.
	      if ( (hStdIn = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE )
    	     DisplayError("GetStdHandle STD_INPUT_HANDLE",hFile);
      }

      PrepAndLaunchRedirectedChild(cmd,hOutputWrite,hInputRead,hErrorWrite,hFile);


      // Close pipe handles (do not continue to modify the parent).
      // You need to make sure that no handles to the write end of the
      // output pipe are maintained in this process or else the pipe will
      // not close when the child process exits and the ReadFile will hang.
      if (!CloseHandle(hOutputWrite)) DisplayError("CloseHandle hOutputWrite",hFile);
      if (!CloseHandle(hInputRead)) DisplayError("CloseHandle hInputRead",hFile);
      if (!CloseHandle(hErrorWrite)) DisplayError("CloseHandle hErrorWrite",hFile);


	  if(bRedirectInput) {
	      // Launch the thread that gets the input and sends it to the child.
    	  hThread = CreateThread(NULL,0,GetAndSendInputThread,
        	                      (LPVOID)hInputWrite,0,&ThreadId);
	      if (hThread == NULL) DisplayError("CreateThread GetAndSendInputThread",hFile);
      }

      // Read the child's output.
      ReadAndHandleOutput(hOutputRead,hFile);
      // Redirection is complete


	  if(bRedirectInput) {
	      // Force the read on the input to return by closing the stdin handle.
    	  if (!CloseHandle(hStdIn)) DisplayError("CloseHandle hStdIn",hFile);


	      // Tell the thread to exit and wait for thread to die.
    	  bRunThread = FALSE;

	      if (WaitForSingleObject(hThread,INFINITE) == WAIT_FAILED)
    	     DisplayError("WaitForSingleObject",hFile);
      }

      if (!CloseHandle(hOutputRead)) DisplayError("CloseHandle hOutputRead",hFile);
      if (!CloseHandle(hInputWrite)) DisplayError("CloseHandle hInputWrite",hFile);

	  CloseHandle(hFile);
      return 0;
}

int RunStdOutput(char* cmd)
{
    int retval;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si,0,sizeof(STARTUPINFO));
	si.wShowWindow=SW_HIDE;
	si.cb=sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
	si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
	if(::CreateProcess(NULL,cmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)==FALSE) {
		retval = 1;       		            
	} else {
		DWORD dstatus = 0;
		WaitForSingleObject(pi.hProcess,INFINITE);
		GetExitCodeProcess(pi.hProcess, &dstatus);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		retval = (dstatus != 0);
	}
	return retval;
}
#endif

int main ( int argc, char **argv )
{
  int retval = -1;
  int nArgsStart = 1;
  char* szFile = NULL;

  if(argv[1]) {
      if(argv[1][0]=='-' && argv[1][1]=='o') {
		szFile = argv[2];
		nArgsStart = 3;
      }
  }

  char const *psethome = get_psethome(argv[0]);
  if ( psethome ) {
      if ( pdisdir(psethome) ) {
#ifdef _WIN32
      char const *psethome_bin = pdmpack("s", "%s\\bin", psethome);
#else
      char const *psethome_bin = pdmpack("s", "%s/bin", psethome);
#endif
	  set_PATH_environment_variable(psethome_bin);
	  FREE_MEMORY(psethome_bin);
#ifdef _WIN32
	  char const *perlbin = pdmpack("s", "%s\\perl\\bin", psethome);
#else
	  char const *perlbin = pdmpack("s", "%s/perl/bin", psethome);
#endif
	  set_PATH_environment_variable(perlbin);
	  FREE_MEMORY(perlbin);
	  char const *perl5lib = pdmpack("s", "PERL5LIB=%s/perl/lib", psethome);
	  if ( perl5lib ) {
	      pdputenv((char *)perl5lib);
	      char const *cmd = pdmpack("ss", "\"%s/perl/bin/perl\" -I \"%s/perl/lib\" ", psethome, psethome);
	      if ( cmd ) {
		  // add arguments
		  for ( int i = nArgsStart; i < argc; i++ ) {
		    if ( argv[i] ) {
		      char const *tmp = pdmpack("ss", "%s \"%s\"", cmd, argv[i]);
		      if ( tmp ) {
		          FREE_MEMORY(cmd);
		          cmd = tmp;
		      }
		    }
		  }

#ifndef _WIN32
		  retval = pdsystem(cmd);
#else
		  if(szFile != NULL)
			retval = RunOutputRedirected(szFile,(char* )cmd);
		  else
			retval = RunStdOutput((char* )cmd);
#endif

		}
	      FREE_MEMORY(cmd);
	    }
	}
      else
	{
	  // not directory
	  printf("%s: Error: PSETHOME is not a directory!\n", argv[0]);
	  printf("%s:        PSETHOME=%s\n", argv[0], psethome);
	}
    }
  else
    {
      // cant get psethome
      printf("%s: Error: Cannot determine PSETHOME!\n", argv[0]);
    }

  return retval != 0;
}
  
