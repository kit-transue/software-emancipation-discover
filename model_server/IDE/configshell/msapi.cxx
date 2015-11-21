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
#include "DisFName.h"

#define	_THIS_FILE_		"Configurator_MSAPI"


/***********************************************************/
/***      Implemantation of Configurator_MSAPI         ***/
/***********************************************************/

Configurator_MSAPI::Configurator_MSAPI(CMProvider* pCMProvider)
: Configurator(pCMProvider)
{
  ClearOpenedFileName();

  m_szProjectName[0]= '\0';
  m_szLocalDir   [0]= '\0';
  m_szAuxProjPath[0]= '\0';
  SetTrapVersion(FALSE);
}

    
//--------
SCCRTN Configurator_MSAPI::OpenProj( BOOL bPrintError )
{ 
  SCCRTN  sRet = SCC_E_UNKNOWNPROJECT;
  PFNSCC  pf;
  long    lFl;
  char	  pszLocalDir[_MAX_PATH] = "";
  char	  pszProjectName[_MAX_PATH] = "";
  
  if ( GetEnv(CM_PROJECT) )
	strcpy( pszProjectName, GetEnv(CM_PROJECT) );
  if ( GetEnv(CM_LOCALDIR) )
	strcpy( pszLocalDir, GetEnv(CM_LOCALDIR) );
  
  LogDebug( "OpenProj", pszProjectName, pszLocalDir );
  if ( pszLocalDir && *pszLocalDir && pszProjectName && *pszProjectName )
  {
    LogDebug( "OpenProj", "m_szLocalDir", m_szLocalDir );
    LogDebug( "OpenProj", "m_szProjectName", m_szProjectName );
    if ( stricmp(m_szLocalDir, pszLocalDir) || stricmp(m_szProjectName, pszProjectName) )
    {
      if ( m_szProjectName[0] && m_szLocalDir[0] )
      {
        pf = GetSccInterface( SCC_CMD_SccCloseProject );
        (*pf)(GetSccContext());
        m_szProjectName[0]= '\0';
        m_szLocalDir   [0]= '\0';
      }

      lFl = SCC_OP_CREATEIFNEW|SCC_OP_SILENTOPEN;
      pf  = GetSccInterface( SCC_CMD_SccOpenProject );
      void* pContext = GetSccContext();
      char* pszUser = GetSccUserName();

      LogDebug( "OpenProj", "SccUserName", GetSccUserName() );
      LogDebug( "OpenProj", "pszProjectNamer", pszProjectName );
      LogDebug( "OpenProj", "pszLocalDir", pszLocalDir );
  
      sRet= (*pf)(GetSccContext(), 
                  NULL, 
                  GetSccUserName(), 
                  pszProjectName, 
                  pszLocalDir, 
                  m_szAuxProjPath, 
                  NULL, 
                  PrintCallbackFunc, 
                  lFl);

      LogDebugInt( "OpenProj", sRet );

      if ( sRet == SCC_OK )
        {
          strcpy( m_szLocalDir, pszLocalDir);
          strcpy( m_szProjectName, pszProjectName);

          LogDebug( "OpenProj", pszProjectName, "OK!" );
  
        }
      else
        {
          m_szProjectName[0]= '\0';
          m_szLocalDir   [0]= '\0';
	  if ( bPrintError )
	  {
	    fprintf( stderr, "%%%%CM provider could not open a project for %s\n", pszProjectName );
	    fflush( stderr );
	  }
        }
    } //if ( stricmp(m_szLocalDir, pszLocalDir) || stricmp(m_szProjectName, pszProject) )
    else
      sRet = SCC_OK;
  
  } // if ( pszLocalDir && *pszLocalDir && pszProjectName && *pszProjectName )

  char* psz = GetEnv(CM_WORKFILE);
  strcpy(m_szOpenedFileName, psz); 
  m_sOpenedFileRet = sRet;
  if ( sRet == SCC_OK )
	SetEnv(CM_REVISION_FILE, GetEnv(CM_WORKFILE) );

  return sRet;
}

    
//--------
SCCRTN Configurator_MSAPI::CloseProj()
{ 
  if ( m_szProjectName[0] && m_szLocalDir[0] )
  {
    m_szProjectName[0]= '\0';
    m_szLocalDir   [0]= '\0';
    PFNSCC  pf = GetSccInterface( SCC_CMD_SccCloseProject );
    (*pf)(GetSccContext());
  }

	ClearOpenedFileName();
  return SCC_OK;
}

//-----------------------------------
BOOL Configurator_MSAPI::FindProjAndLocalDir(char *cm_workfile, char *cm_revfile)
{
  BOOL bRet = FALSE;
  if ( cm_workfile && *cm_workfile && cm_revfile && *cm_revfile )
  {
	char *psz = GetLastSlash( cm_workfile );
	if( psz )
	  SetEnv( CM_FNAME, psz+1 );
	else
	  SetEnv( CM_FNAME, cm_workfile );

	if ( GetEnv(CM_PROJECT) && GetEnv(CM_LOCALDIR) )
	  bRet = TRUE;
  }
  else
  {
	SetEnv( CM_PROJECT,   NULL );
	SetEnv( CM_LOCALDIR,  NULL );
  }
  return bRet;
}

//-----------------------------------
int Configurator_MSAPI::cm_setup_src( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  LogDebug( "cm_setup_src", "CM_WORKFILE", GetEnv( CM_WORKFILE ) );
  LogDebug( "cm_setup_src", "CM_REVISION_FILE", GetEnv( CM_REVISION_FILE ) );
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);

  int   nResult = Configurator::cm_setup_src(pszWorkPath,pszSysPath,pszCommand,pszOpt);

  if ( !m_fScriptInvoked )
  {
    nResult = 1;

    if( OpenProj() == SCC_OK )
    {
      if ( GetSccFileStatus(cm_workfile) == SCC_STATUS_NOTCONTROLLED )
        nResult = 99;
      else
        nResult = 0;
    }
  }

  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_setup_pset( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_sysfile  = GetEnv( CM_SYSFILE );
  FindProjAndLocalDir(cm_workfile, cm_sysfile);
  int   nResult = Configurator::cm_setup_pset(pszWorkPath,pszSysPath,pszCommand,pszOpt);

  if ( !m_fScriptInvoked )
  {
    nResult = 1;

    if( OpenProj() == SCC_OK )
    {
      if ( GetSccFileStatus(cm_workfile) == SCC_STATUS_NOTCONTROLLED )
      {
        LogDebug( "cm_setup_pset", cm_workfile, "SCC_STATUS_NOTCONTROLLED" );
        nResult = 99;
      }
      else
      {
        LogDebug( "cm_setup_pset", cm_workfile, "OK!!" );
        nResult = 0;
      }
    }
    else
      LogDebug( "cm_setup_pset", cm_workfile, "???OpenProj???" );

  }
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_exec()
{
  return 0;
}

//-----------------------------------
int Configurator_MSAPI::cm_info()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_sysfile  = GetEnv( CM_SYSFILE );
  if ( cm_workfile && *cm_workfile )
	FindProjAndLocalDir(cm_workfile, cm_sysfile);
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
	  if ( OpenProj() == SCC_OK )
	  {
		sprintf( sz, "%%%%   Project : %s\n", m_szProjectName );
		ConfigShellOutput( sz );
		sprintf( sz, "%%%%   LocalDir: %s\n", m_szLocalDir );
		ConfigShellOutput( sz );

		int status = GetSccFileStatus(cm_workfile);
		sprintf( sz, "%%%%   Status  :" );
		if ( status == SCC_STATUS_NOTCONTROLLED )
		{
		  strcat( sz, " NOTCONTROLLED" );
		}
		else
		{
		  if ( status & SCC_STATUS_CONTROLLED )
			strcat( sz, " CONTROLLED" );
		  if ( status & SCC_STATUS_CHECKEDOUT )
			strcat( sz, " CHECKEDOUT" );
		  if ( status & SCC_STATUS_OUTOTHER )
			strcat( sz, " OUTOTHER" );
		  if ( status & SCC_STATUS_OUTEXCLUSIVE )
			strcat( sz, " OUTEXCLUSIVE" );
		  if ( status & SCC_STATUS_OUTMULTIPLE )
			strcat( sz, " OUTMULTIPLE" );
		  if ( status & SCC_STATUS_OUTOFDATE )
			strcat( sz, " OUTOFDATE" );
		  if ( status & SCC_STATUS_DELETED )
			strcat( sz, " DELETED" );
		  if ( status & SCC_STATUS_LOCKED )
			strcat( sz, " LOCKED" );
		  if ( status & SCC_STATUS_MERGED )
			strcat( sz, " MERGED" );
		  if ( status & SCC_STATUS_SHARED )
			strcat( sz, " SHARED" );
		  if ( status & SCC_STATUS_PINNED )
			strcat( sz, " PINNED" );
		  if ( status & SCC_STATUS_MODIFIED )
			strcat( sz, " MODIFIED" );
		  if ( status & SCC_STATUS_OUTBYUSER )
			strcat( sz, " OUTBYUSER" );
		}
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

//-----------------------------------
BOOL cm_lsco_populate ( void* pvCallerData, BOOL fAddRemove, LONG nStatus, LPSTR lpFileName)
{
  Configurator_MSAPI *pConfigurator= (Configurator_MSAPI*)pvCallerData;

  if ( !fAddRemove )
  {
	char sz[_MAX_PATH+8];
	sprintf( sz, "%%%%%s", lpFileName );
	ConfigShellResult* pResult= pConfigurator->ConfigShellFindOutput(sz);
	if ( pResult )
	  pConfigurator->ConfigShellDeleteOutput(pResult);
  }
  return TRUE;
}

//-----------------------------------
int Configurator_MSAPI::cm_lsco()
{
  BOOL bMemOK = TRUE;
  int iFName;
  long*	pStatus =0;
  ConfigShellResult* pResult;
  long	  nFiles;
  LPSTR*  lpFileNames;
  long*	  lpStatus;
  char	  szLine[_MAX_PATH +32] = "cmd.exe /C dir /S/B/A:-D-R ";

  int nResult = Configurator::cm_lsco();
  if ( !m_fScriptInvoked )
  {
	int		nFileNames = 0;
	char**	ppszFileNames = 0;
	strcat( szLine, GetEnv(CM_WORKFILE) );
	FILE* pipeCmdDir = CM_OpenPipe( szLine );
	if ( pipeCmdDir )
	{
	  strcpy( szLine, ":%" );
	  while ( CM_ReadPipe( szLine+2, _MAX_PATH, pipeCmdDir) )
	  {
	    ConfigShellOutput( szLine );
		nFileNames++;
	  }
	  CM_ClosePipe(pipeCmdDir);
	}
	if ( nFileNames > 0 )
	{
	  ppszFileNames = new char* [ nFileNames ];
	  pStatus = new long [ nFileNames ];
	  if ( ppszFileNames == 0 || pStatus == 0 )
		bMemOK = FALSE;
	  else
	  {
		for ( iFName =0; iFName < nFileNames; iFName++ ) 
		{
		  pStatus[iFName] = SCC_STATUS_NOTCONTROLLED;
		  ppszFileNames[iFName] =0;
		}
	  }
	  
	  iFName =0;
	  pResult = 0;
	  while ( (pResult= ConfigShellFindOutput( ":%")) )
	  {
		char* pszFName = pResult->GetText();
		*pszFName = '%';
		pszFName += 2;
		if ( bMemOK )
		{
		  ppszFileNames[iFName] = new char [ strlen(pszFName) +1 ];
		  if ( ppszFileNames[iFName] == 0 )
			bMemOK = FALSE;
		  else
			strcpy( ppszFileNames[iFName], pszFName );
		}
		iFName++;
	  }
	}

	if ( !bMemOK )
	{
      ConfigShellOutput( "%%Not enough memory to create file list\n" );
	  nResult = 1;
	}
	else
	{
	  for ( iFName =0; iFName < nFileNames; iFName++ ) 
	  {
		FindProjAndLocalDir(ppszFileNames[iFName], ppszFileNames[iFName]);
		if( OpenProj() == SCC_OK )
		  break;
	  }
	  if ( iFName < nFileNames )
	  {
		nFiles = nFileNames - iFName;
		lpFileNames = ppszFileNames + iFName;
		lpStatus = pStatus + iFName;

        PFNSCC  pf  = GetSccInterface( SCC_CMD_SccPopulateList );
        SCCRTN  sRet= (*pf)(GetSccContext(), SCC_COMMAND_CHECKIN, 
							nFiles, lpFileNames, cm_lsco_populate, this, lpStatus, NULL );
	  }
	}

	for ( iFName =0; iFName < nFileNames; iFName++ ) 
	{
	  pResult= ConfigShellFindOutput(ppszFileNames[iFName]);
	  if ( pResult )
	  {
		ConfigShellDeleteOutput(pResult);
		if ( pStatus[iFName] != SCC_STATUS_NOTCONTROLLED )
		{
		  strcpy(szLine, "%%" );
		  strcat(szLine, DisFN_realOSPATH(ppszFileNames[iFName]) );
		  ConfigShellOutput( DisFN_convert_forward_slashes(szLine) );
		}
	  }
	}

	if ( pStatus )
	  delete [] pStatus;
	if ( ppszFileNames )
	{
	  for ( iFName =0; iFName < nFileNames; iFName++ ) 
	  {
		if ( ppszFileNames[iFName] )
		  delete [] ppszFileNames[iFName];
	  }
	  delete [] ppszFileNames;
	}
    nResult = 0;
  }
  return nResult;
}

//-----------------------------------
BOOL cm_query_populate ( void* pvCallerData, BOOL fAddRemove, LONG nStatus, LPSTR lpFileName)
{
  TargetFileDef *ptfdFile= (TargetFileDef*)pvCallerData;

  if ( stricmp(lpFileName, ptfdFile->GetSysFilePath() ) == 0 )
  {
    ptfdFile->SetStatus( nStatus );
    ptfdFile->SetFlag( fAddRemove );
    return FALSE;
  }
  else
    return TRUE;
}

//-----------------------------------
int Configurator_MSAPI::cm_query(SCCCOMMAND sccCommand)
{
  char *cm_workfile = m_ptfdFile->GetWorkFile();
  char *cm_sysfile  = m_ptfdFile->GetSysFile();
  FindProjAndLocalDir(cm_workfile, cm_sysfile);
  long  nStatus = 0;

  if( OpenProj() == SCC_OK )
  {
    m_ptfdFile->SetFlag(1);
    SetTrapVersion(TRUE, this);
    PFNSCC  pf  = GetSccInterface( SCC_CMD_SccPopulateList );
    SCCRTN  sRet= (*pf)(GetSccContext(), sccCommand, 1, &cm_sysfile, cm_query_populate, (void*)m_ptfdFile, &nStatus, NULL );
    SetTrapVersion(FALSE);
    m_ptfdFile->SetStatus( nStatus );
  }
  else
  {
    m_ptfdFile->SetFlag( 0 );
    m_ptfdFile->SetStatus( SCC_STATUS_NOTCONTROLLED );
  }
  if ( m_ptfdFile->GetFlag() == 0 )
    return 1;
  else
    return 0;
}
//-----------------------------------
int Configurator_MSAPI::cm_query_put()
{
  int nResult = 0;
  m_ptfdFile->SetFlag(1);
  m_ptfdFile->SetStatus(0);

  char* p = m_ptfdFile->GetWorkFile();
  if ( p && *p && (p=m_ptfdFile->GetSysFile()) && *p )
  {
    SetEnv( CM_WORKFILE, m_ptfdFile->GetWorkFile() );
    SetEnv( CM_SYSFILE,  m_ptfdFile->GetSysFile() );
    SetEnv( CM_REVISION_FILE, NULL );

    nResult = Configurator::cm_query_put();
    if ( !m_fScriptInvoked )
      nResult = cm_query( SCC_COMMAND_CHECKIN );
    if ( nResult != 0 )
      m_ptfdFile->SetFlag(0);
  }
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_query_get()
{
  int nResult = 0;
  m_ptfdFile->SetFlag(1);
  m_ptfdFile->SetStatus(0);

  char* p = m_ptfdFile->GetSysFile();
  if ( p && *p && (p=m_ptfdFile->GetSysFile()) && *p )
  {
    SetEnv( CM_WORKFILE, m_ptfdFile->GetWorkFile() );
    SetEnv( CM_SYSFILE,  m_ptfdFile->GetSysFile() );
    SetEnv( CM_REVISION_FILE, NULL );

    nResult = Configurator::cm_query_get();
    if ( !m_fScriptInvoked )
      nResult = cm_query( SCC_COMMAND_CHECKOUT );
    if ( nResult != 0 )
      m_ptfdFile->SetFlag(0);
  }
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_copy_pset()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_sysfile  = GetEnv( CM_SYSFILE );
  FindProjAndLocalDir(cm_workfile, cm_sysfile);
  int nResult = Configurator::cm_copy_pset();

  if ( !m_fScriptInvoked )
  {
    LogDebug( "cm_copy_pset", cm_workfile, "START" );
    if( OpenProj() == SCC_OK )
    {
      int status = GetSccFileStatus(cm_workfile);
      if ( status == SCC_STATUS_NOTCONTROLLED )
      {
        LogDebug( "cm_copy_pset", cm_workfile, "SCC_STATUS_NOTCONTROLLED" );
        nResult = 0;
      }
      else
      {
//        if( _access( cm_workfile, 0/*existance only*/ ) == 0 ) //if cm_workfile exist
//          _chmod( cm_workfile,_S_IREAD | _S_IWRITE );

		SetTrapVersion(TRUE, this);
        PFNSCC  pf  = GetSccInterface( SCC_CMD_SccCheckout );
        SCCRTN  sRet= (*pf)(GetSccContext(), NULL, 1, &cm_workfile, "", 0, NULL );
		SetTrapVersion(FALSE);
        
        LogDebug( "cm_copy_pset", cm_workfile, (sRet==SCC_OK ? "OK" : "INVALID") );

        if( sRet == SCC_OK )
        {
          _access( cm_workfile, 2/*write permission*/ );
          nResult = cm_unlock();
	      }
        else
          nResult = 1;
      }
    }
    else
      LogDebug( "cm_copy_pset", cm_workfile, "???OpenProj???" );
  }
  return nResult;
}
      
//-----------------------------------
int Configurator_MSAPI::cm_copy_src()
{
  LogDebug( "Configurator::cm_copy_src()", "", "" );
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);

  int nResult = Configurator::cm_copy_src();
  LogDebugInt( "Configurator::cm_copy_src() after script", nResult );

  if ( !m_fScriptInvoked )
  {
    nResult = 1;

	  LogDebug( "Configurator::cm_copy_src()", "before OpenProj for", cm_workfile );
    if( OpenProj() == SCC_OK )
    {
   	  LogDebug( "Configurator::cm_copy_src()", "after OpenProj", "" );

      PFNSCC  pf   = GetSccInterface( SCC_CMD_SccCheckout );
		  SetTrapVersion(TRUE, this);
      SCCRTN  sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, "", 0, NULL );
		  SetTrapVersion(FALSE);
      
   	  LogDebugInt( "Configurator::cm_copy_src() after checkout", sRet );
      if( sRet == SCC_OK )
        nResult = _access( cm_workfile, 2/*write permission*/ );
      else
        nResult = 1;
    }
  }
  return nResult;
}


//-----------------------------------
int Configurator_MSAPI::cm_create_dir()
{
  int nResult = Configurator::cm_create_dir();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    char *pszfile = GetEnv( CM_WORKFILE );
    if( pszfile )
      nResult = (int)!CreateParentDirectory( pszfile );
  }
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_create_file()
{
  int nResult = Configurator::cm_create_file();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    char *pszfile = GetEnv( CM_WORKFILE );
    if( pszfile )
    {
      if( !_access( pszfile, 0/*existance only*/ ) )
        nResult = 0; //file already exists
      else
      {
        // CreateParentDirectory() will try to create a parent directory
        // or will return TRUE if directory already exists
        if( CreateParentDirectory( pszfile ) )
        {
          HANDLE hFile = CreateFile( pszfile, GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL, NULL );
          if( hFile != INVALID_HANDLE_VALUE )
          {
            CloseHandle( hFile );
            nResult = 0;
          }
        }
      }
    }
  }
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_delete_src()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_delete_src();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    BOOL fDidUncheckout = FALSE;

    if( OpenProj(FALSE) == SCC_OK )
    {
      PFNSCC  pf   = GetSccInterface( SCC_CMD_SccUncheckout );
		  SetTrapVersion(TRUE, this);
      SCCRTN  sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, 0, NULL );
		  SetTrapVersion(FALSE);
      if( sRet == SCC_OK )
        fDidUncheckout = TRUE;
    }
    if( !fDidUncheckout && cm_workfile )
    {
      //try just to delete a file
			remove( cm_workfile );
      nResult = 0;
    }
  }
  return nResult;
}


//-----------------------------------
int Configurator_MSAPI::cm_delete_pset()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_delete_pset();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    BOOL fDidUncheckout = FALSE;

    if( OpenProj(FALSE) == SCC_OK )
    {
      PFNSCC  pf   = GetSccInterface( SCC_CMD_SccUncheckout );
		  SetTrapVersion(TRUE, this);
      SCCRTN  sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, 0, NULL );
		  SetTrapVersion(FALSE);

      if( sRet == SCC_OK )
        {
          fDidUncheckout = TRUE;
          nResult = 0;
          remove( cm_workfile ); //try to remove file
        }
    }
    if( !fDidUncheckout && cm_workfile )
    {
      //try just to delete a file
			remove( cm_workfile );
      nResult = 0;
    }
  }
  return nResult;
}   


//-----------------------------------
int Configurator_MSAPI::cm_diff()
{
  int nResult = 1;
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);

  if( OpenProj() == SCC_OK )
  {
    PFNSCC pf = GetSccInterface( SCC_CMD_SccDiff );
    long lOpt =  SCC_DIFF_QD_TIME; //this will force CM not to display diff
                                   //or SCC_DIFF_QD_CHECKSUM

    //check if we have to display diff
    if( GetEnv( CM_DISPDIFF ) && GetEnv( CM_DISPDIFF )[0] == '1' )
      lOpt = 0;
    SCCRTN  sRet = (*pf)(GetSccContext(), NULL, cm_workfile, lOpt, 0 );
    if( sRet == SCC_OK )
      nResult = 0;
    else if ( sRet == SCC_I_FILEDIFFERS && lOpt )
    {
      nResult = Configurator::cm_diff();
    }
    else
      nResult = 99;
  }
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_get_src()
{   
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_get_src();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK )
    {
      PFNSCC pf = GetSccInterface( SCC_CMD_SccCheckout );
      char *pszOpt     = GetEnv( CM_OPTIONS );
      char *pszComment = GetEnv( CM_DESCRIPTION );
      if ( *pszOpt == 0 )
        pszOpt = NULL;

      if( !pszComment )
        pszComment = "";
    
      char  szFile1[_MAX_PATH]= "";
      char  szFile2[_MAX_PATH]= "";
      char* ppszFile[2] = { szFile1, szFile2 };

      void* pContext = GetSccContext();
      strcpy( szFile1, cm_workfile );

		  SetTrapVersion(TRUE, this);
      SCCRTN  sRet = (*pf)( pContext, NULL, 1, ppszFile, pszComment, 0, pszOpt );
		  SetTrapVersion(FALSE);
      if( sRet == SCC_OK )
        nResult = 0; 
    }
  }
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_obsolete_src()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_obsolete_src();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK )
    {
      PFNSCC pf = GetSccInterface( SCC_CMD_SccRemove );

      SCCRTN  sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, "", 0, NULL );
      if( sRet == SCC_OK )
        nResult = 0; 
    }
  }
  return nResult;
}


//-----------------------------------
int Configurator_MSAPI::cm_put_pset()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_sysfile  = GetEnv( CM_SYSFILE );
  FindProjAndLocalDir(cm_workfile, cm_sysfile);
  int nResult = Configurator::cm_put_pset();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK )
    {
      PFNSCC pf = GetSccInterface( SCC_CMD_SccCheckin );

      char *pszOpt = GetEnv( CM_OPTIONS );
      if( !pszOpt )
        pszOpt = "";

      char *pszComment = GetEnv( CM_DESCRIPTION );
      if( !pszComment )
        pszComment = "";

		  SetTrapVersion(TRUE, this);
      SCCRTN  sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, pszComment, 0, NULL /*pszOpt*/ );
		  SetTrapVersion(FALSE);
      if( sRet == SCC_OK )
        nResult = 0; 
    }
  }
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_put_src()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_put_src();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK && (GetSccFileStatus(cm_workfile) & SCC_STATUS_OUTBYUSER)  )
    {
     PFNSCC pf = GetSccInterface( SCC_CMD_SccCheckin );

      char *pszOpt = GetEnv( CM_OPTIONS );
      if( !pszOpt )
        pszOpt = "";

      char *pszComment = GetEnv( CM_DESCRIPTION );
      if( !pszComment )
        pszComment = "";

		  SetTrapVersion(TRUE, this);
      SCCRTN  sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, pszComment, 0, NULL /*pszOpt*/ );
		  SetTrapVersion(FALSE);
      if( sRet == SCC_OK )
        nResult = 0; 
    }
  }
  return nResult;
}


//-----------------------------------
int Configurator_MSAPI::cm_fast_put_src()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_put_src();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK && (GetSccFileStatus(cm_workfile) & SCC_STATUS_OUTBYUSER)  )
    {
      PFNSCC pf = GetSccInterface( SCC_CMD_SccCheckin );

      char *pszOpt = GetEnv( CM_OPTIONS );
      if( !pszOpt )
        pszOpt = "";

      char *pszComment = GetEnv( CM_DESCRIPTION );
      if( !pszComment )
        pszComment = "";

      char  szFile1[_MAX_PATH]= "";
      char  szFile2[_MAX_PATH]= "";
      char* ppszFile[2] = { szFile1, szFile2 };

      void* pContext = GetSccContext();
      strcpy( szFile1, cm_workfile );

		  SetTrapVersion(TRUE, this);
      SCCRTN  sRet = (*pf)( pContext, NULL, 1, ppszFile, pszComment, 0, NULL /*pszOpt*/ );
//      SCCRTN  sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, pszComment, 0, NULL /*pszOpt*/ );
		  SetTrapVersion(FALSE);
      if( sRet == SCC_OK )
        nResult = 0; 
    }
  }
  return nResult;
}


//-----------------------------------
int Configurator_MSAPI::cm_unget_src()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_unget_src();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK )
    {
      SCCRTN  sRet;
      PFNSCC  pf;
      int     status = GetSccFileStatus(cm_workfile);

      if ( status & SCC_STATUS_OUTBYUSER )
      {
        pf = GetSccInterface( SCC_CMD_SccUncheckout );
			  SetTrapVersion(TRUE, this);
        sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, 0, NULL );
			  SetTrapVersion(FALSE);
        if( sRet == SCC_OK )
        {
          status = GetSccFileStatus(cm_workfile);
          if ( (status & SCC_STATUS_OUTBYUSER) == 0 )
	          nResult = 0;
        }
      }
      else // if ( status & SCC_STATUS_OUTBYUSER )
        nResult = 0;

			if ( nResult == 0 && _access( cm_workfile, 2 ) == 0 ) //if cm_workfile RW
        _chmod( cm_workfile,_S_IREAD );
        
    } // if( OpenProj(
  }  // if ( !m_fScriptInvoked )
  
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_unget_pset()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_sysfile  = GetEnv( CM_SYSFILE );
  FindProjAndLocalDir(cm_workfile, cm_sysfile);
  int nResult = Configurator::cm_unget_pset();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK )
    {
      SCCRTN  sRet;
      PFNSCC  pf;
      int     status = GetSccFileStatus(cm_workfile);

      if ( status & SCC_STATUS_OUTBYUSER )
      {
        pf = GetSccInterface( SCC_CMD_SccUncheckout );
			  SetTrapVersion(TRUE, this);
        sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, 0, NULL );
			  SetTrapVersion(FALSE);
        
        LogDebug( "cm_unget_pset", cm_workfile, (sRet==SCC_OK ? "OK" : "INVALID") );

        if( sRet == SCC_OK )
        {
        status = GetSccFileStatus(cm_workfile);
        if ( (status & SCC_STATUS_OUTBYUSER) == 0 )
          {
          if( _access( cm_workfile, 2 ) == 0 ) //if cm_workfile RW
            _chmod( cm_workfile,_S_IREAD );
          nResult = 0;
          }
        }
      }
      else // if ( status & SCC_STATUS_OUTBYUSER )
        nResult = 0;
    } // if( OpenProj(
  }  // if ( !m_fScriptInvoked )
  
  return nResult;
}

//-----------------------------------
int Configurator_MSAPI::cm_version_list()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_version_list();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK )
    {
      PFNSCC pf = GetSccInterface( SCC_CMD_SccHistory );

      SCCRTN  sRet = (*pf)(GetSccContext(), NULL, 1, &cm_workfile, 0, NULL );
      if( sRet == SCC_OK || sRet == SCC_I_RELOADFILE )
      {
        nResult = 0;
      }
    }
  }
  return nResult;
}
  

//-----------------------------------
int Configurator_MSAPI::cm_stat()
{
  int nResult = Configurator::cm_stat();

  if ( !m_fScriptInvoked )
  {
    nResult = 0;
  }

  return nResult;
}


//-----------------------------------
int Configurator_MSAPI::cm_src_version()
{
  int nResult = Configurator::cm_src_version();

  if ( !m_fScriptInvoked )
  {
    nResult = cm_stat();
  }

  return nResult;
}


//-----------------------------------
int Configurator_MSAPI::cm_is_locked()
{
  char *cm_workfile = GetEnv( CM_WORKFILE );
  char *cm_revfile  = GetEnv( CM_REVISION_FILE );
  FindProjAndLocalDir(cm_workfile, cm_revfile);
  int nResult = Configurator::cm_is_locked();

  if ( !m_fScriptInvoked )
  {
    nResult = 1;
    if( OpenProj() == SCC_OK )
    {
	  int nStatus = GetSccFileStatus(cm_workfile);
	  if ( (nStatus & SCC_STATUS_LOCKED ) || 
		 ( (nStatus & SCC_STATUS_OUTEXCLUSIVE ) && !(nStatus & SCC_STATUS_OUTBYUSER )) )
        nResult = 1;
	  else
		nResult = 0;
    }
  }

  return nResult;
}

//--------
int Configurator_MSAPI::cm_lock()  
{
  int nResult = Configurator::cm_lock();
  if ( !m_fScriptInvoked )
  	nResult= 0;
	return nResult;
}

//--------
int Configurator_MSAPI::cm_unlock() 
{
  int nResult = Configurator::cm_unlock();
  if ( !m_fScriptInvoked )
  	nResult= 0;

	return nResult;
}

#define _STAND_ALONE_DISFNAME
#include "\paraset\src\IDE\DisFName\DisFName.cpp"

