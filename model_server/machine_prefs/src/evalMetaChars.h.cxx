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
// evalMetaChars.h.C
//------------------------------------------
// synopsis:
// accepts a path specification and converts all metacharacters therein appropriately.
//
// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#ifdef _WIN32
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <climits>
#include <cerrno>
#include <ctime>

#include <direct.h>
#include <io.h>

#include <winsock.h>
#include <fcntl.h>


#include <windows.h>
#include <winbase.h>

#endif

#undef MAX
#include <genError.h>
#include <messages.h>
#include <genString.h>
#include <evalMetaChars.h>



//this is stuff which deals with the lock files
#ifdef _WIN32
#include "raFile.h"
#include "pdustring.h"
#endif

#include "xxinterface.h"



// EXTERNAL VARIABLES

// EXTERNAL FUNCTIONS

extern "C" const char* customize_getStrPref_AsIs(const char * key);
//extern const char* main_argv_0;

// PRE-PROCESSOR DEFINITIONS

#ifdef _WIN32
#define gettext( param ) (param)
#endif
// VARIABLE DEFINITIONS

static const char backslash = '\\';

#ifdef _WIN32
static char glob_env_var_chars[32] = "$%";
#else
static char glob_env_var_chars[32] = "$";
#endif

// FUNCTION DEFINITIONS
void init_env_var_chars()
{
    Initialize(init_env_var_chars);
    char* p = (char*)customize_getStrPref_AsIs("EnvVarChars");
    if ( p && *p )
       strcpy( glob_env_var_chars, p );
}

const char* get_env_var_chars()
{
    return (const char*)glob_env_var_chars;
}

bool eval_shell_metachars(const char* input, genString& value)
{
    Initialize(eval_shell_metachars);

    bool status = true;
    genString newbuf;

    if(!input)  return status;

    // Skip leading whitespace
    const char *ptr;
    for(ptr = input; *ptr; ++ptr)
	if(!isspace(*ptr))
	    break;

    // Substitute for initial tilde
    if(*ptr == '~') {
       ptr++;
       if (*ptr == '/' || *ptr == '\\') {
	  const char* home = OSapi_getenv("HOME");
	  if(home) {
	      newbuf += home;
	  } else {
              status = false;
              msg("ERROR: The environment variable '$1' must be defined.") << "HOME" << eom;
	      ReturnValue(status);
	  }
       } else {
#ifndef _WIN32
	  // only UNIX has notion of other users' home directories:
	  genString username = ptr;
	  char* endslash = strchr((char *)username, '/');
	  if (endslash) {
	     *endslash = 0;
	  }
	  dis_passwd  pw(username);
	  if (!(pw==0)) {
	     newbuf += pw.pw_dir;
	     ptr += strlen(username);
	  }
	  else {
              status = false;
              msg("ERROR: There is no user named '$1'.") << (const char*) username << eom;
	      ReturnValue(status);
	  }
#else
	  --ptr; 
#endif
       }
    }

   const char* env_var_chars = get_env_var_chars();
	while(*ptr)
    {
		if ( strchr(env_var_chars, *ptr) )
		{
	      if (ptr[0] == ptr[1]) {  // repeated char is escape sequence
	         ++ptr;  // consume one of the chars
		     newbuf += *ptr++;
	      } 
		  else 
		  {     // variable substitution
  			 const int percent = (ptr[0] == '%');
  			 const int brace = (ptr[1] == '{');
			 char const*const start = ptr + brace + 1;

			 char const* p = start;
	         while (*p && (isalnum(*p) || *p == '_'))
			    ++p;

			 // Must point to char following current token.
			 ptr = p + brace + percent;

			 if (brace && *p != '}')  
			 {
				 status = false;
				 msg("ERROR: Environment variable is missing a closing brace '}'.") << eom;
				 break;
			 } 
			 else if (percent && *(p+brace) != '%')
			 {
				 status = false;
				 msg("ERROR: Environment variable is missing a closing percent.") << eom;
				 break;
			 }

			 const int len = p - start;
			 char variable[128];		// environment variable

			 if (len == 0)  
			 {
				 status = false;
				 msg("ERROR: An illegal format for an environment variable was found.") << eom;
				 break;
			 } 
			 else if (len >= sizeof(variable))  
			 {
				 status = false;
				 msg("ERROR: An environment variable name longer than $1 characters was found.") << 128 << eom;
				 break;
			 } 
			 else
			 {
			   strncpy(variable, start, len);
			   variable[len] = '\0';
			   const char* env = OSapi_getenv(variable);
			   if(env)
			   {
				 newbuf += env;
			   } 
			   else
			   {
				 status = false;
				 msg("ERROR: The environment variable '$1' must be defined.") << variable << eom;
				 break;
			   }
			 }
	      }
		}
		else
		{
            newbuf += *ptr++;
		}
	}

    if(status)
        value = newbuf;

    return (status);
}
//-----------------------------------------
// Create temp directory fo NT
//----------------------------------------
#ifdef _WIN32

static raFile * pfDiscoverLockFile;  //points to the lock file
#endif

void  create_tmp_dir()
{
#ifdef _WIN32
  char* pszDIS_WinDir = new char [sizeof("DIS_WINDIR=") + _MAX_PATH+1];
  strcpy( pszDIS_WinDir, "DIS_WINDIR=" );
  GetWindowsDirectory( pszDIS_WinDir +strlen("DIS_WINDIR="), _MAX_PATH );
  OSapi_putenv( pszDIS_WinDir );
  delete [] pszDIS_WinDir;

  // store user defined TMP
  char* pszDIS_TmpDir = new char [_MAX_PATH+1];
  strcpy( pszDIS_TmpDir, "DIS_TMP_BEFORE_DIS=" );
  char* pszTmpDir = ::getenv("TMP");
  if ( pszTmpDir && *pszTmpDir )
		strcat( pszDIS_TmpDir, pszTmpDir );
  else if (  (pszTmpDir = ::getenv("TEMP")) && *pszTmpDir )
		strcat( pszDIS_TmpDir, pszTmpDir );
  else
    strcat( pszDIS_TmpDir, "C:\\TEMP" );
  OSapi_putenv( pszDIS_TmpDir );

  pszTmpDir = ::getenv("TMPDIR_DIS");
  if ( pszTmpDir && *pszTmpDir )
		strcpy( pszDIS_TmpDir, pszTmpDir );
	else if ( (pszTmpDir = ::getenv("TMPDIR")) && *pszTmpDir )
		strcpy( pszDIS_TmpDir, pszTmpDir );
	else if ( (pszTmpDir = ::getenv("TEMPDIR")) && *pszTmpDir )
		strcpy( pszDIS_TmpDir, pszTmpDir );
  else if (  (pszTmpDir = ::getenv("TEMP")) && *pszTmpDir )
		strcpy( pszDIS_TmpDir, pszTmpDir );
  else if (  (pszTmpDir = ::getenv("TMP")) && *pszTmpDir )
		strcpy( pszDIS_TmpDir, pszTmpDir );
  else
    strcpy( pszDIS_TmpDir, "C:\\TEMP" );
	int n = strlen(pszDIS_TmpDir)-1;
  if ( n > 0  &&  ( *(pszDIS_TmpDir+n) == '/' || *(pszDIS_TmpDir+n) == '\\') )
    *(pszDIS_TmpDir+n) = '\0';
  convert_back_slashes(pszDIS_TmpDir);

// Check the disk temp space
  DWORD dwSectorsPerCluster;
  DWORD dwBytesPerSector;
  DWORD dwNumberOfFreeClusters;
  DWORD dwTotalNumberOfClusters;

  char c=*(pszDIS_TmpDir +3);
  *(pszDIS_TmpDir +3) = '\0';

  DWORD nErr=GetDiskFreeSpace( pszDIS_TmpDir,
                    &dwSectorsPerCluster, &dwBytesPerSector, 
                    &dwNumberOfFreeClusters, &dwTotalNumberOfClusters );
  unsigned __int64 dwFreeSpace = (unsigned __int64)dwSectorsPerCluster * dwBytesPerSector * dwNumberOfFreeClusters / 1024;	
 
  

  //////////////////////////////////////////////////////////////////////////////////////////////////
  //char dfmsg[1024];
  //sprintf(dfmsg,"Sectors per cluster %d\nBytes per cluster %d\nNumber of Free clusters %d\nTotal of of clusters %d\nfree space %d\ndrive %s",
  //	  dwSectorsPerCluster,dwBytesPerSector,dwNumberOfFreeClusters,dwTotalNumberOfClusters,dwFreeSpace, pszDIS_TmpDir);
  //	  int res=::MessageBox(NULL, dfmsg ,"Disk Space Warning:",MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION);
  //////////////////////////////////////////////////////////////////////////////////////////////////
  

#ifdef _WIN32
  if (nErr==0)
  {	
	char dfmsg[384];
	sprintf(dfmsg, "The directory %s is invalid--please check your\nTMP, TEMP, and TMPDIR\nenvironment variables.", pszDIS_TmpDir);
	::MessageBox(NULL, dfmsg, "Disk Space Warning", MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION);
  }
  else if (dwFreeSpace < 20000) {
	char dfmsg[1024];
	sprintf(dfmsg, "Free disk space is less than 20 MB on drive <%c>, too low for DISCOVER to run. Exiting.", *pszDIS_TmpDir);
	::MessageBox(NULL, dfmsg, "Disk Space Error", MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION);
	exit(1);
  } 
  else if(dwFreeSpace < 70000) {
	  char dfmsg[384];
	  sprintf(dfmsg, "You have less than 70 MB of free space on drive <%c>.\nThis is less than the minimum required.", *pszDIS_TmpDir);
	  int res=::MessageBox(NULL, dfmsg ,"Disk Space Warning:",MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION);
 
  }
#endif

  if ( dwFreeSpace < 70000 ) {
    printf("\n\n*WARNING*\nTemporary space available %dK on %s", dwFreeSpace, pszTmpDir);
    printf("\nis less than the minimum recomended (70 MB) for running DISCOVER\n\n");
  }

// Setup temp directory
  *(pszDIS_TmpDir +3) = c;
  strcat( pszDIS_TmpDir, "/DISCOVER-" );
  _strdate( pszDIS_TmpDir + strlen(pszDIS_TmpDir) );
  int nTmpDirLen = strlen(pszDIS_TmpDir);

  *(pszDIS_TmpDir + nTmpDirLen -6) = '-';
	*(pszDIS_TmpDir + nTmpDirLen -3) = '.';
	*(pszDIS_TmpDir + nTmpDirLen -2) = '\0';
  nTmpDirLen = strlen(pszDIS_TmpDir);

  for ( int num=1; ; num++ )
  {
    sprintf(pszDIS_TmpDir + nTmpDirLen, "%d", num);
    if ( _access( pszDIS_TmpDir, 0) )
      break;
    *(pszDIS_TmpDir + nTmpDirLen) = '\0';
  }

  CreateDirectory( pszDIS_TmpDir, NULL );
  char* pszPutEnvTmpDir = new char [sizeof("TMPDIR_DIS=") +_MAX_PATH+1];
  strcpy( pszPutEnvTmpDir, "TMP=");
  strcat( pszPutEnvTmpDir, pszDIS_TmpDir );
  OSapi_putenv( pszPutEnvTmpDir );
  strcpy( pszPutEnvTmpDir, "TEMP=");
  strcat( pszPutEnvTmpDir, pszDIS_TmpDir );
  OSapi_putenv( pszPutEnvTmpDir );
  strcpy( pszPutEnvTmpDir, "TMPDIR=");
  strcat( pszPutEnvTmpDir, pszDIS_TmpDir );
  OSapi_putenv( pszPutEnvTmpDir );
  strcpy( pszPutEnvTmpDir, "TEMPDIR=");
  strcat( pszPutEnvTmpDir, pszDIS_TmpDir );
  OSapi_putenv( pszPutEnvTmpDir );
  strcpy( pszPutEnvTmpDir, "TMPDIR_DIS=");
  strcat( pszPutEnvTmpDir, pszDIS_TmpDir );
  OSapi_putenv( pszPutEnvTmpDir );

  
  ///////////////////////////////////////////////////////
  // stuff to deal with the lock file
  //////////////////////////////////////////////////////
  const char * pName="Discover.lock";
  const char * pContents="This file is being used by DISCOVER\n";

  char * pszDIS_LockFile=new char[strlen(pName)+strlen(pszDIS_TmpDir)+2];
  pdstrcpy(pszDIS_LockFile, pszDIS_TmpDir);
  pdstrcat(pszDIS_LockFile, "/");  
  pdstrcat(pszDIS_LockFile, pName);
  
  
  pfDiscoverLockFile=new raFile(pszDIS_LockFile, 0);
  int nResult=pfDiscoverLockFile->open(raFile_ACCESS_READ|raFile_ACCESS_WRITE, raFile_OPEN_CREATE);
  if (nResult!=-1)
  {nResult=pfDiscoverLockFile->write((char *)pContents, pdstrlen(pContents) );
  }
  if (nResult!=-1)
  {
	pfDiscoverLockFile->lock(0, 1);  //observe convention--we only lock the first byte
  }
  if (nResult==-1)
  {
	  delete pfDiscoverLockFile;
	  pfDiscoverLockFile=NULL;
  }
  ////////////////////////////////////////////////////////////
  //end of stuff to deal with lock files
  ///////////////////////////////////////////////////////////////
  
  delete [] pszDIS_LockFile;
  delete [] pszPutEnvTmpDir;
  delete [] pszDIS_TmpDir;

#endif
}
 


//------------------------------
// remove the temp directory on NT
//------------------------------

void remove_tmp_dir()
{
#ifdef _WIN32
	
	////////////////////////////////////////////
	// deal with the lock file
	///////////////////////////////////////////
	if (pfDiscoverLockFile)
	{
		int nResult=0;
		nResult=pfDiscoverLockFile->unlock(0, 1);
		const char * pContents2="This file is no longer in use by DISCOVER";
		nResult=pfDiscoverLockFile->write((char *)pContents2, pdstrlen(pContents2));
		nResult=pfDiscoverLockFile->close(0);
		delete pfDiscoverLockFile;
	}
	
	const char* pszTmpDir = OSapi_getenv("TMPDIR");
	const char  szPattDis[] = "DISCOVER-";
	pszTmpDir = OSPATH(pszTmpDir);
	if ( strstr( pszTmpDir, convert_to_uniform_case(szPattDis)) )
	{
		char* cmd = new char [ sizeof("rmdir /s /q ") + strlen(pszTmpDir) + 3 ];
		if ( cmd )
		{
			strcpy( cmd, "rmdir /s /q \"" );
			strcat( cmd, convert_forward_slashes( pszTmpDir ) );
                        strcat( cmd, "\"");
			system( cmd );
		}
	}
	
	
#endif
}


