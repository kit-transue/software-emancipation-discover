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
#include <fcntl.h>
#include "configurator.h"
#include "DisFName.h"

#define	_THIS_FILE_		"Configurator_Custom"

//-----------------------------------
void cm_CreateDirectoryForFile( char* pszFullFilePath )
{
  if ( pszFullFilePath && *pszFullFilePath )
  {
    if( _access( pszFullFilePath, 0/*existance only*/ ) == 0 )
      return;

    char *p, *pszCurDir = strchr( pszFullFilePath, '\\' );
    while ( (p=strchr( pszCurDir+1, '\\')) )
    {
      *p = 0;
      CreateDirectory( pszFullFilePath, 0 );
      *p = '\\';
      pszCurDir = p;
    }

  }
}


/***********************************************************/
/***      Implemantation of Configurator_Custom            ***/
/***********************************************************/

Configurator_Custom::~Configurator_Custom()
{
}

//---------------------------------
Configurator_Custom::Configurator_Custom(CMProvider* pCMProvider)
: Configurator_MSAPI(pCMProvider)
{
  SetTrapVersion(FALSE);
}

//---------------------------------
BOOL Configurator_Custom::FindProjAndLocalDir(char *cm_workfile, char *cm_revfile)
{
  if ( Configurator_MSAPI::FindProjAndLocalDir(cm_workfile, cm_revfile) )
	return TRUE;

  if ( cm_workfile == 0 || *cm_workfile == 0 )
	return FALSE;

  char pszLocalDir[_MAX_PATH+2] = "";
  strcpy( pszLocalDir, cm_workfile );
  char* pLastFSlash = GetLastSlash( pszLocalDir );
  if( pLastFSlash )
    *pLastFSlash = '\0';

  SetEnv( CM_PROJECT,  pszLocalDir );
  SetEnv( CM_LOCALDIR, pszLocalDir );
  return TRUE;
}

//-----------------------------------
int Configurator_Custom::cm_setup_src( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int nResult = 0;
  SetEnv( CM_REVISION_FILE, GetEnv( CM_WORKFILE ) );
  if ( strcmp(pszCommand, "copy_src") == 0  && 
       strcmp(pszWorkPath, pszSysPath) == 0     )
	  SetEnv( CM_SYSFILE, GetEnv( CM_WORKFILE ) );
  else
  {
    FindProjAndLocalDir( GetEnv(CM_WORKFILE), GetEnv(CM_REVISION_FILE) );
    nResult = Configurator::cm_setup_src(pszWorkPath,pszSysPath,pszCommand,pszOpt);
    if ( !m_fScriptInvoked )
      nResult = 0;
  }
  
  return nResult;
}

//-----------------------------------
int Configurator_Custom::cm_setup_pset( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int nResult = 0;
  SetEnv( CM_REVISION_FILE, GetEnv( CM_WORKFILE ) );
  if ( strcmp(pszCommand, "copy_pset") == 0  && 
       strcmp(pszWorkPath, pszSysPath) == 0     )
	  SetEnv( CM_SYSFILE, GetEnv( CM_WORKFILE ) );
  return nResult;
}

//-----------------------------------
void Configurator_Custom::cm_unget_internal()
{
}

void Configurator_Custom::cm_copy_internal()
{
}

//-----------------------------------
void Configurator_Custom::cm_put_internal()
{
}

//--------

int Configurator_Custom::cm_get_src()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  if ( cm_workfile && *cm_workfile )
    cm_CreateDirectoryForFile( cm_workfile );

  int nResult = Configurator::cm_copy_src();
  if ( !m_fScriptInvoked )
    cm_copy_internal();
  return 0;
}

//--------

int Configurator_Custom::cm_copy_src()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  if ( cm_workfile && *cm_workfile )
    cm_CreateDirectoryForFile( cm_workfile );
  int nResult = Configurator::cm_copy_src();

  if ( !m_fScriptInvoked )
    cm_copy_internal();
  return 0;
}

//--------

int Configurator_Custom::cm_copy_pset()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  if ( cm_workfile && *cm_workfile )
    cm_CreateDirectoryForFile( cm_workfile );
  int nResult = Configurator::cm_copy_pset();

  if ( !m_fScriptInvoked )
    cm_copy_internal();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_put_pset()
{
  int nResult = Configurator::cm_put_pset();

  if ( !m_fScriptInvoked )
    cm_put_internal();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_put_src()
{
  int nResult = Configurator::cm_put_src();

  if ( !m_fScriptInvoked )
    cm_put_internal();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_fast_put_src()
{
  int nResult = Configurator::cm_put_src();

  if ( !m_fScriptInvoked )
    cm_put_internal();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_lsco()
{
  char szOldDir[MAX_PATH+1];
  GetCurrentDirectory( sizeof(szOldDir), szOldDir );
  
  int nResult = Configurator::cm_lsco();
  if ( !m_fScriptInvoked )
  {
   // Setting current project to the root
	char pszCmd [ 2*_MAX_PATH ];
	char szLine [ 2*_MAX_PATH ];
	char szOut  [ 2*_MAX_PATH ];
	int  nLen;
	char szDir[MAX_PATH+1];

  char *cm_workfile = GetEnv( CM_WORKFILE );
	if ( cm_workfile && *cm_workfile )
	{
	  strcpy( szDir, cm_workfile );
	  if ( (GetFileAttributes(szDir) & FILE_ATTRIBUTE_DIRECTORY) == 0 )
	  {
		char* p = strrchr( szDir, '\\' );
		if ( p ) *p = 0;
	  }
	  else if ( *(szDir + strlen(szDir)-1) != '\\' )
		strcat( szDir, "\\" );

	  SetCurrentDirectory( szDir );
	}

	strcpy( pszCmd, "cmd.exe /C dir /S/B/A:-D/A:-R " );
	FILE* pFOut = CM_OpenPipe( pszCmd );
	if ( pFOut )
	{
      while( !feof(pFOut) && CM_ReadPipe(szLine, sizeof(szLine), pFOut)  )
      {
        LogDebug( "cm_lsco()", "dir output:", szLine );
		    while ( (nLen=strlen(szLine)) && ( *(szLine + nLen -1) == 0x0a || *(szLine + nLen -1) == 0x0d ) )
		       *(szLine + nLen -1) = 0;
        if ( *szLine )
        {
          szLine[0] = toupper( szLine[0] );
    		  sprintf( szOut, "%%%%%s", szLine );
    		  ConfigShellOutput( szOut );
        }
	  }
	  CM_ClosePipe(pFOut);
	}
	else
	  LogDebug( "cm_lsco", "pipe is NOT created", "" );

  }

  SetCurrentDirectory( szOldDir );
  return nResult;
}

//-----------------------------------
int Configurator_Custom::cm_unget_src()
{
  int nResult = Configurator::cm_unget_src();
  if ( !m_fScriptInvoked )
    cm_unget_internal();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_unget_pset()
{
  int nResult = Configurator::cm_unget_pset();
  if ( !m_fScriptInvoked )
    cm_unget_internal();
  return 0;
}

//-----------------------------------
void Configurator_Custom::cm_delete_internal()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  if ( cm_workfile && *cm_workfile )
  {
    if ( _access( cm_workfile, 0/*existance only*/ ) == 0 ) //if cm_workfile exist
      DeleteFile( cm_workfile );
  }
}

//-----------------------------------
int Configurator_Custom::cm_delete_src()
{
  int nResult = Configurator::cm_delete_src();
  if ( !m_fScriptInvoked )
    cm_delete_internal();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_delete_pset()
{
  int nResult = Configurator::cm_delete_pset();
  if ( !m_fScriptInvoked )
    cm_delete_internal();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_stat()
{
  int nResult = Configurator::cm_stat();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_lock()
{
  int nResult = Configurator::cm_lock();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_unlock()
{
  int nResult = Configurator::cm_unlock();
  return 0;
}

//-----------------------------------
int Configurator_Custom::cm_info()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_sysfile  = GetEnv( CM_SYSFILE );
  int nResult = Configurator::cm_info();

  if ( !m_fScriptInvoked )
  {
    char sz[1024];
    if ( cm_workfile && *cm_workfile )
	  {
	    strcpy(cm_workfile, DisFN_realOSPATH(cm_workfile) );
	    DisFN_convert_forward_slashes(cm_workfile);

	    sprintf( sz, "%%%%%s\n", cm_workfile );
	    ConfigShellOutput( sz );
	    sprintf( sz, "%%%%   User    : %s\n", GetSccUserName() );
	    ConfigShellOutput( sz );
  		sprintf( sz, "%%%%   Project : (none)\n" );
	  	ConfigShellOutput( sz );
      char* pSlash = GetLastSlash(cm_workfile);
      if ( pSlash )
      {
        *pSlash = 0;
    		sprintf( sz, "%%%%   LocalDir: %s\n", cm_workfile );
        *pSlash = '\\';
      }
      else
  		   sprintf( sz, "%%%%   LocalDir: (none)\n" );
	  	ConfigShellOutput( sz );

		sprintf( sz, "%%%%   Status  :" );
    if( _access( cm_workfile, 0/*existance only*/ ) == 0 ) //if cm_workfile exist
    {
			strcat( sz, " CONTROLLED" );
      if( _access( cm_workfile, 6 ) == 0 ) //if cm_workfile RW
			  strcat( sz, " CHECKEDOUT" );
  		strcat( sz, "\n");
	  	ConfigShellOutput( sz );
    }
    else
    {
  		sprintf( sz, "%%%%File is not in work_area or does not controlled by %s", GetSccName() );
	  	ConfigShellOutput(sz);
    }
	}
	else
	{
	  sprintf( sz, "%%%%%s\n", GetSccName() );
	  ConfigShellOutput( sz );
	}
    nResult = 0;
  }
  return nResult;
}

