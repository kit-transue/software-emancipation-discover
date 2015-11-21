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
#include "configurator.h"

#define	_THIS_FILE_		"Configurator_ClearCase"


/***********************************************************/
/***      Implemantation of Configurator_ClearCase       ***/
/***********************************************************/

Configurator_ClearCase::Configurator_ClearCase(CMProvider* pCMProvider)
: Configurator_MSAPI(pCMProvider)
{
  SetTrapVersion(FALSE);
}

//--------
BOOL Configurator_ClearCase::FindProjAndLocalDir(char *cm_workfile, char *cm_revfile)
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

//--------
void Configurator_ClearCase::SetOptEnv(int argc, char** argv)
{
    SetEnv( CM_ISCLEARCASE, "1" );
    Configurator_MSAPI::SetOptEnv( argc, argv);
}

//-----------------------------------
int Configurator_ClearCase::cm_setup_src( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int nResult = 0;
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
int Configurator_ClearCase::cm_setup_pset( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int nResult = 0;
  SetEnv( CM_REVISION_FILE, GetEnv( CM_WORKFILE ) );
  if ( strcmp(pszCommand, "copy_pset") == 0  && 
       strcmp(pszWorkPath, pszSysPath) == 0     )
	  SetEnv( CM_SYSFILE, GetEnv( CM_WORKFILE ) );
/*  else
  {
    nResult = Configurator_MSAPI::cm_setup_pset(pszWorkPath,pszSysPath,pszCommand,pszOpt);
    if ( !m_fScriptInvoked )
      nResult = 0;
  }
*/  
  return nResult;
}

//--------
void Configurator_ClearCase::SetEnvRevisionFile()
{
  if( strlen( GetVersionFromCM() ) ) // pszVersion is a global array
  {                                  // which is filled in a routine controlling
                                    // CM provider's output (ClearCase only) 
    char pszFile[ _MAX_PATH ];
    pszFile[0] = '\0';
    if( GetEnv( CM_SYSFILE ) )
      strcpy( pszFile, GetEnv( CM_SYSFILE ) );
    strcat( pszFile, GetVersionFromCM()  );
    SetEnv( CM_REVISION_FILE, pszFile );
  }
}


//--------

int Configurator_ClearCase::cm_copy_pset()
{
  int nReturn = Configurator_MSAPI::cm_copy_pset();
  if ( nReturn ==0 )
    SetEnvRevisionFile();
  return nReturn;
}

//--------

int Configurator_ClearCase::cm_copy_src()
{
  int nReturn = Configurator_MSAPI::cm_copy_src();
  if ( nReturn ==0 )
		{
    SetEnvRevisionFile();
		nReturn = cm_unlock();
		}
  return nReturn;
}

//-----------------------------------
int Configurator_ClearCase::cm_stat()
{
  int nResult = Configurator::cm_stat();

  if ( !m_fScriptInvoked )
  {
    nResult = 0;
  }
	else
	{
		char*	cm_workfile = GetEnv( CM_WORKFILE );
		ConfigShellResult* pConfigShellResult = ConfigShellFindOutput( cm_workfile );
		if ( pConfigShellResult )
		{
			char*	pszText = pConfigShellResult->GetText();
			if ( strcmp(pszText + strlen(pszText) - strlen("CHECKEDOUT"), "CHECKEDOUT") == 0 )
			{
				ConfigShellDeleteOutput( pConfigShellResult );
				pConfigShellResult = ConfigShellFindOutput( cm_workfile );
			}

			if ( pConfigShellResult )
			{
				SetEnv( CM_RESULT, pConfigShellResult->GetText() + strlen(cm_workfile) +2 );
				SetEnv( CM_REVISION_FILE, pConfigShellResult->GetText() );
			}

			while ( (pConfigShellResult = ConfigShellFindOutput(cm_workfile)) )
				ConfigShellDeleteOutput( pConfigShellResult );
		}
	}

  return nResult;
}

//---------------------------------------------
int Configurator_ClearCase::cm_lsco()
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
	int  nLen, nLenDir;
	char szDir[MAX_PATH+1];
	char szDriver[]="C:";

	nLenDir =0;
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
	  szDriver[0] = toupper(szDir[0]);
	  strcpy( szDir, szDir+2 );
	  nLenDir = strlen( szDir );
	}

	strcpy( pszCmd, "cleartool lsprivate -short -co" );
	FILE* pFOut = CM_OpenPipe( pszCmd );
	if ( pFOut )
	{
      while( !feof(pFOut) && CM_ReadPipe(szLine, sizeof(szLine), pFOut)  )
      {
        LogDebug( "cm_lsco()", "ct lspriv output:", szLine );
		while ( (nLen=strlen(szLine)) && ( *(szLine + nLen -1) == 0x0a || *(szLine + nLen -1) == 0x0d ) )
		  *(szLine + nLen -1) = 0;
        if ( *szLine == 0 )
		  continue;
		if ( nLenDir == 0 || strnicmp( szLine, szDir, nLenDir ) == 0 )
		{
		  sprintf( szOut, "%%%%%s%s", szDriver, szLine );
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
