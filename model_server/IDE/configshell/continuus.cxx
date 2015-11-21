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

#define	_THIS_FILE_		"Configurator_Continuus"

//--------------------------------
BOOL Configurator_Continuus::FindProjAndLocalDir(char *pszFilePath, char *cm_revfile)
{
  if ( Configurator_MSAPI::FindProjAndLocalDir(pszFilePath, cm_revfile) )
	return TRUE;

  char pszProjName[_MAX_PATH] = "";
  char pszLocalDir[_MAX_PATH] = "";

  if ( pszFilePath && *pszFilePath )
  {
    char szLocalDir[_MAX_PATH] = ".\\";
    char szLine1[2*_MAX_PATH] = "";
    char szLine2[2*_MAX_PATH] = "";
	char szProject[_MAX_PATH] = "";
    char* p;

    LogDebug( "FindProjAndLocalDir", "pszFilePath", pszFilePath );
    char *pszFileName = strrchr( pszFilePath, '\\' );
    if ( pszFileName )
	{
	  *pszFileName =0;
	  strcpy( szLocalDir, pszFilePath );
	  strcat( szLocalDir, "\\" );
	  *pszFileName++ = '\\';
	}
	else
	  pszFileName = pszFilePath;

	char szCmd[2*_MAX_PATH] = "";
	sprintf( szCmd, "ccm finduse %s", pszFilePath );
	LogDebug( "FindProjAndLocalDir", "start", szCmd );
    FILE* pfOut = CM_OpenPipe( szCmd );
 	if ( pfOut )
	{
	  if ( CM_ReadPipe(szLine1,sizeof(szLine1)-1,pfOut) )
	  {
		if ( strstr( szLine1, "Updating database with") )
		  CM_ReadPipe(szLine1,sizeof(szLine2)-1,pfOut);

		if ( strncmp( szLine1, pszFileName, strlen(pszFileName)) == 0 )
	    {
		  while ( CM_ReadPipe(szLine2,sizeof(szLine2)-1,pfOut) )
		  {
	        if ( (p=strrchr(szLine2, '@')) )
			{
			  strcpy( szProject, "\\" );
			  strcat( szProject, p+1 );
			  strcat( szProject, "\\" );
			  if ( strstr( szLocalDir, szProject) )
			  {
		        strcpy( pszProjName, p+1 );
		        strcpy( pszLocalDir, szLocalDir );
		        LogDebug( "FindProjAndLocalDir", "ProjName", pszProjName );
                LogDebug( "FindProjAndLocalDir", "LocalDir", pszLocalDir );
			  }
		    }
		  }
		  if ( *pszLocalDir == 0 )
		     LogWarning( "FindProjAndLocalDir", "project not found from: ccm finduse", "" );
	    }
	    else
	      LogWarning( "FindProjAndLocalDir", "wrong 1 line from: ccm finduse", szLine1 );
	  }
	  else
	  {
	    LogWarning( "FindProjAndLocalDir", "no output from: ccm finduse", pszFilePath );
	  }
	  CM_ClosePipe( pfOut );
	}
	else
	  LogWarning( "FindProjAndLocalDir", "cannot execute ccm finduse", pszFilePath );
  }

  if ( *pszProjName && *pszLocalDir )
  {
	SetEnv( CM_PROJECT,  pszProjName );
	SetEnv( CM_LOCALDIR, pszLocalDir );
	return TRUE;
  }
  else
	return FALSE;
}

/***********************************************************/
/***      Implemantation of Configurator_ClearCase       ***/
/***********************************************************/

Configurator_Continuus::Configurator_Continuus(CMProvider* pCMProvider)
: Configurator_MSAPI(pCMProvider)
{
}

//-----------------------------------
int Configurator_Continuus::cm_setup_src( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int nResult = 0;
  LogDebug( "Configurator_Continuus::cm_setup_src", pszCommand, pszWorkPath );
  SetEnv( CM_REVISION_FILE, GetEnv( CM_WORKFILE ) );
  if ( strcmp(pszCommand, "copy_src") == 0  && 
       strcmp(pszWorkPath, pszSysPath) == 0     )
	  SetEnv( CM_SYSFILE, GetEnv( CM_WORKFILE ) );
  else
  {
    nResult = Configurator_MSAPI::cm_setup_src(pszWorkPath,pszSysPath,pszCommand,pszOpt);
    if ( !m_fScriptInvoked )
      nResult = 0;
  }
  
  return nResult;
}

//-----------------------------------
int Configurator_Continuus::cm_setup_pset( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int nResult = 0;
  SetEnv( CM_REVISION_FILE, GetEnv( CM_WORKFILE ) );
  if ( strcmp(pszCommand, "copy_pset") == 0  && 
       strcmp(pszWorkPath, pszSysPath) == 0     )
	  SetEnv( CM_SYSFILE, GetEnv( CM_WORKFILE ) );
  else
  {
    nResult = Configurator_MSAPI::cm_setup_pset(pszWorkPath,pszSysPath,pszCommand,pszOpt);
    if ( !m_fScriptInvoked )
      nResult = 0;
  }
  
  return nResult;
}

//-----------------------------------
int Configurator_Continuus::cm_stat()
{
  int nResult = Configurator::cm_stat();
  if ( !m_fScriptInvoked )
  {
    nResult = 0;
  }
	else
	{
		ConfigShellResult* pConfigShellResult = ConfigShellLastOutput();
		if ( pConfigShellResult )
		{
			char*	pszRev = pConfigShellResult->GetText();
			if ( pszRev )
				SetEnv( CM_RESULT, pszRev );
		}
	}
  return nResult;
}

//-----------------------------------
int Configurator_Continuus::cm_exec()
{
  char* pszOP = GetEnv( CM_SYSFILE );
  char  szCmd[256] = "ccm ";
  char  sz[1024] = "%%";
  LogDebug( "cm_exec", pszOP, "" );

  if ( strnicmp(pszOP, "SelectDefaultTask", 6) == 0 )
  {
    strcat( szCmd, "task /default /g" );
  }
  else if ( strnicmp(pszOP, "CheckInDefaultTask", 7) == 0 )
  {
    strcat( szCmd, "ci /g /task \"\"" );
  }
  else if ( strnicmp(pszOP, "CreateDefaultTask", 6) == 0 )
  {
    strcat( szCmd, "create_task /g" );
  }
  else if ( strnicmp(pszOP, "Reconfigure", 8) == 0 )
  {
    strcat( szCmd, "reconfigure " );
    strcat( szCmd, GetEnv(CM_WORKFILE) );
    strcat( szCmd, " /g" );
  }
  else if ( strnicmp(pszOP, "Reconcile", 8) == 0 )
  {
    strcat( szCmd, "reconcile2 " );
    strcat( szCmd, GetEnv(CM_WORKFILE) );
    strcat( szCmd, " /g" );
  }

  LogDebug( "cm_exec", "szCmd=", szCmd );
  FILE* pfOut = CM_OpenPipe( szCmd );
 	if ( pfOut )
	{
	  while ( CM_ReadPipe(sz+2,sizeof(sz)-3,pfOut) )
	    ConfigShellOutput( sz );
	  CM_ClosePipe( pfOut );
    LogDebug( "cm_exec", "Finish", "" );
	}
  else
    LogDebug( "cm_exec", "cannot", "execute!!!!" );

  return 0;
}


/*
              ccm delim 
              ccm prop /f "%project" phisfile

              in phisfn /projname~vername
              ccm wa /s /p path 

*/



