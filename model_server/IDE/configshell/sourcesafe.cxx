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

class lstProject;

#define	_THIS_FILE_		"Configurator_SourceSafe"

#define VSS_ENVVAR        "SS_PATH"
#define VSS_DBDIR         "SSDIR"
#define VSS_CMDTOOL_NAME  "ss.exe"
#define VSS_DIR_LINE	  "Dir ("
#define VSS_STATUS_LINE	  "No files found checked out by"
#define VSS_LSCO_LINE	    "You do not have access rights to"

class lstProject
{
public:
  static lstProject* firstProject;

  char*		  m_pszName;
  long		  m_lStatus;
  lstProject* m_pNext;

  lstProject () { m_pszName =0; m_pNext = 0; m_lStatus = SCC_STATUS_NOTCONTROLLED;}
  ~lstProject() 
  { 
	if ( m_pszName ) 
	  delete [] m_pszName;
	m_pszName =0;
	if ( m_pNext )
	  delete m_pNext;
	m_pNext = 0;
  }

  char*	getName() { return m_pszName; }
  lstProject* getNext() { return m_pNext; }
  void setStatus(long l) { m_lStatus = l; }
  long getStatus() { return m_lStatus; }
  void saveName( char* psz )
  {
	*psz = 0;
	if ( m_pszName )
	{
	  if ( strcmp(m_pszName, "$" )== 0 )
		strcpy( psz, "\"$/\"" );
	  else
		sprintf( psz, "\"%s\"", m_pszName );
	}
  }

  static void DeleteAll() 
  {
	if ( firstProject )
	  delete firstProject;
	firstProject =0;
  }

  static void CreateNew(char* pszName) 
  {
	char* psz = new char [ strlen(pszName) +1 ];
	char *p;
	if ( psz )
	{
	  strcpy(psz, pszName);
	  if ( strcmp(psz, "$/") && (p = GetLastSlash(psz)) ) *p =0;
	  lstProject* plstProject = new lstProject;
	  if ( plstProject )
	  {
		plstProject->m_pszName = psz;
		plstProject->m_pNext = firstProject;
		firstProject = plstProject;
	  }
	  else
		delete [] psz;
	}
  }
};

lstProject *  lstProject::firstProject= 0;

class lstWrkDir
{
public:
  static lstWrkDir* firstWrkDir;

  char*		  m_pszProjPath;
  char*		  m_pszDirPath;
  char*		  m_pszMachine;
  lstWrkDir*  m_pNext;

  lstWrkDir () { m_pszProjPath =0; m_pszDirPath =0; m_pNext = 0; }
  ~lstWrkDir() 
  { 
	if ( m_pszProjPath ) 
	  delete [] m_pszProjPath;
	m_pszProjPath =0;
	if ( m_pszDirPath ) 
	  delete [] m_pszDirPath;
	m_pszDirPath =0;
	if ( m_pszMachine ) 
	  delete [] m_pszMachine;
	m_pszMachine =0;
	if ( m_pNext )
	  delete m_pNext;
	m_pNext = 0;
  }

  char*	getProjPath() { return m_pszProjPath; }
  char*	getDirPath()  { return m_pszDirPath; }
  BOOL	isMachine(char* pszMachine)  { return (stricmp(m_pszMachine, pszMachine) == 0); }
  lstWrkDir* getNext() { return m_pNext; }

  static void DeleteAll() 
  {
	if ( firstWrkDir )
	  delete firstWrkDir;
	firstWrkDir =0;
  }

  static void CreateNew(char* pszProjPath, char* pszLine, char** ppszSSiniEnv, int nSSiniEnv) 
  {
	char* pszDirPath = 0;
	char *pMachine, *p;

	pMachine = pszLine + strlen(VSS_DIR_LINE);
	while ( *pMachine == ' ' ) pMachine++;
	
	pszDirPath = strchr(pMachine, ')' );
	if ( pszDirPath  == 0 )
	  return;

	*pszDirPath++ = 0;
	p = pszDirPath-2;
	while ( *p == ' ' && p > pMachine ) *p-- = 0;

	BOOL found = FALSE;
	for ( int n = 0; !found && n < nSSiniEnv; n++ )
	{
	  if ( stricmp( pMachine, ppszSSiniEnv[n] ) == 0 )
		found = TRUE;
	}
	if ( !found )
	  return;
	
	pszDirPath = strchr( pszDirPath, '=' );
	if ( pszDirPath == 0 )
	  return;
	pszDirPath++;
	while ( *pszDirPath == ' ' ) pszDirPath++;
	p = pszDirPath + strlen(pszDirPath) -1;
	while ( *p == ' ' || *p == '\t' || *p == 0x0a || *p == 0x0a ) *p-- = 0;

	char* pszProj = new char [ strlen(pszProjPath) +1 ];
	char* pszDir  = new char [ strlen(pszDirPath) +1 ];
	char* pszMachine  = new char [ strlen(pMachine) +1 ];
	lstWrkDir* plstWrkDir = new lstWrkDir;
	if ( pszProj && pszDir && plstWrkDir && pszMachine )
	{
	  int len;
	  strcpy(pszProj, pszProjPath);
	  strcpy(pszDir, pszDirPath);
	  strcpy(pszMachine, pMachine);

	  while ( (len=strlen(pszProj)) && 
			  (  *(pszProj+len-1) == 0x0d || *(pszProj+len-1) == 0x0a || *(pszProj+len-1) == ']') )
		*(pszProj+len-1) = 0;
	  while ( (len=strlen(pszDir)) && 
			  (  *(pszDir+len-1) == 0x0d || *(pszDir+len-1) == 0x0a)  )
		*(pszDir+len-1) = 0;
	  
	  LogDebug( "lstWrkDir::CreateNew", "for project  :", pszProj );
	  LogDebug( "lstWrkDir::CreateNew", "for machine  :", pszMachine );
	  LogDebug( "lstWrkDir::CreateNew", "new lstWrkDir:", pszDirPath );

	  plstWrkDir->m_pszProjPath = pszProj;
	  plstWrkDir->m_pszDirPath  = pszDir;
	  plstWrkDir->m_pszMachine  = pszMachine;
	  plstWrkDir->m_pNext = firstWrkDir;
	  firstWrkDir = plstWrkDir;
	}
	else
	{
	  if ( pszProj ) delete [] pszProj;
	  if ( pszDir ) delete [] pszDir;
	  if ( pszMachine ) delete [] pszMachine;
	  if ( plstWrkDir ) delete plstWrkDir;
	}
  }
};

lstWrkDir *  lstWrkDir::firstWrkDir= 0;

/***********************************************************/
/***      Implemantation of Configurator_ClearCase       ***/
/***********************************************************/
//  char    m_szSS_DirLine  [_MAX_PATH +64];
//  strcpy( m_szSS_DirLine, "Dir (");
//  strcat( m_szSS_DirLine, ") = ");

Configurator_SourceSafe::~Configurator_SourceSafe()
{
  int n;
  if ( m_ppszSSiniEnv  && m_nSSiniEnv )
  {
	for ( n=0; n < m_nSSiniEnv; n++ )
	{
	  if ( m_ppszSSiniEnv[n] )
		delete m_ppszSSiniEnv[n];
	}
	delete m_ppszSSiniEnv;
  }
  m_ppszSSiniEnv = 0;
  m_nSSiniEnv = 0;
}

//---------------------------------
Configurator_SourceSafe::Configurator_SourceSafe(CMProvider* pCMProvider)
: Configurator_MSAPI(pCMProvider)
{
  lstProject::firstProject =0;
  lstWrkDir::firstWrkDir= 0;
  SetTrapVersion(FALSE);
  m_szToolSS     [0]  = 0;
  m_szUserIniFile[0]  = 0;
  m_szSrcFName   [0]  = 0;
  m_szSrcProject [0]  = 0;
  m_szPsetFName  [0]  = 0;
  m_szPsetProject[0]  = 0;
  m_ppszSSiniEnv = 0;

// Fill array of machine names m_ppszSSiniEnv
  char szOut[2*_MAX_PATH];
  char szBuf[1024] = "";
  char szLine[1024] = "";
  char* p, *pszBuf;
  int   n;
  FILE* pFOut;

  LogDebug( "Configurator_SourceSafe", "Configurator_SourceSafe", "--->Build 11-09-99" );
  if ( !GetEnvironmentVariable( "SSiniENV", szBuf, sizeof(szBuf)) || szBuf[0] == 0 )
  {
	pFOut = CM_OpenPipe( "hostname" );
	if ( pFOut )
	{
	  p= szBuf;
	  CM_ReadPipe(szBuf, sizeof(szBuf), pFOut);
		while ( (n=strlen(p)) && *(p+n-1) == 0x0a || *(p+n-1) == 0x0d ) *(p-1) = 0;
		fclose(pFOut);
	}
	strcat( szBuf, " NT Win PC UI");
  }
  LogDebug( "Configurator_SourceSafe", "SSiniENV=", szBuf );
  p = szBuf + strlen(szBuf) -1;
  while ( *p == ' ' ) *p-- = 0;
  pszBuf = szBuf;
  while ( *pszBuf == ' ' ) pszBuf++;

  p = pszBuf; m_nSSiniEnv = 1;
  while ( p && (p=strchr(p,' ')) )
  {
	m_nSSiniEnv++;
	while ( p && *p == ' ' ) p++;
  }

  m_ppszSSiniEnv = new char* [ m_nSSiniEnv ];
  for ( n=0; n < m_nSSiniEnv; n++ )
  {
	if ( (p = strchr(pszBuf, ' ')) )
	  *p = 0;
	m_ppszSSiniEnv[n] = new char [ strlen(pszBuf)+1 ];
	if ( m_ppszSSiniEnv[n] )
	  strcpy( m_ppszSSiniEnv[n], pszBuf );
	if ( p )
	{
	  pszBuf = p+1;
	  while ( *pszBuf == ' ' ) pszBuf++;
	}
  }

  // We need user name
  szBuf[0]  = 0;
  if( GetEnvironmentVariable( "SSUSER", szBuf, sizeof(szBuf)) )
  {
    m_pCMProvider->SetSccUserName(szBuf);
    LogDebug( "Configurator_SourceSafe", "SSUSER:", GetSccUserName() );
  }
  else
    LogDebug( "Configurator_SourceSafe", "Login UserName:", GetSccUserName() );
  szBuf[0]  = 0;
  if( GetEnvironmentVariable( "SSPWD", szBuf, sizeof(szBuf )) && szBuf[0] )
    LogDebug( "Configurator_SourceSafe", "SSPWD:", szBuf );


  // We also need name of user ss.ini file
  BOOL found = FALSE;
  if( GetEnvironmentVariable( VSS_DBDIR, m_szUserIniFile, sizeof( m_szUserIniFile ) ) &&
      strlen( m_szUserIniFile ) ) 
  {
    if ( *(m_szUserIniFile + strlen(m_szUserIniFile) -1) != '\\' )
	    strcat( m_szUserIniFile, "\\" );
	strcpy( szBuf, m_szUserIniFile );
	strcat( szBuf, "srcsafe.ini" );
	if ( (pFOut = fopen( szBuf, "r" )) )
	{
	  while ( !feof(pFOut) && fgets(szLine, sizeof(szLine)-2, pFOut) )
	  {
		p = szLine; 
		while ( *p == ' ' || *p == '\t' ) p++;
		if ( strnicmp( p, "Users_Txt", strlen("Users_Txt")) == 0 && (p=strchr(p,'=')) )
		{
		  p ++;
		  while ( *p == ' '  || *p == '\t' ) p++;
		  if ( szLine[1] != ':' )
		  {
			strcpy( szBuf, m_szUserIniFile );
			strcat( szBuf, p );
		  }
		  else
			strcpy( szBuf, p );
		  p = szBuf + strlen( szBuf ) -1;
		  while ( *p == ' '  || *p == '\t' || *p == 0x0a || *p == 0x0d ) *p-- = 0;
		  found = TRUE;
		  break;
		}
	  }
	  fclose(pFOut);
	}
	else
	{
	  LogDebug( "Configurator_SourceSafe", "cannot open file", szBuf );
	  sprintf( szOut, "%%%%Cannot open file \"%s\"", szBuf );
	  ConfigShellOutput( szOut );
	}
	if ( !found )
	{
	  LogDebug( "Configurator_SourceSafe", "File", szBuf );
	  LogDebug( "Configurator_SourceSafe", "!!!Has ho information about users.txt file", "" );
	  sprintf( szOut, "%%%%Cannot open file \"%s\"", szBuf );
	  ConfigShellOutput( szOut );
	  strcpy( szBuf, m_szUserIniFile );
	  strcat( szBuf, "users.txt" );
	  LogDebug( "Configurator_SourceSafe", "Trying file", szBuf );
	}
	
	found = FALSE;
	if ( (pFOut = fopen( szBuf, "r" )) )
	{
	  while ( !feof(pFOut) && fgets(szLine, sizeof(szLine)-2, pFOut) )
	  {
		p = szLine; while ( *p == ' ' || *p == '\t' ) p++;
		if ( strnicmp( p, GetSccUserName(), strlen(GetSccUserName())) == 0 && (p=strchr(p,'=')) )
		{
		  p++;
		  while ( *p == ' '  || *p == '\t' ) p++;
		  if ( szLine[1] != ':' )
			strcat( m_szUserIniFile, p );
		  else
			strcpy( m_szUserIniFile, p );
		  p = m_szUserIniFile + strlen( m_szUserIniFile ) -1;
		  while ( *p == ' '  || *p == '\t' || *p == 0x0a || *p == 0x0d ) *p-- = 0;
		  found = TRUE;
		  break;
		}
	  }
	  fclose(pFOut);
	}
	else
	{
	  LogDebug( "Configurator_SourceSafe", "cannot open file", szBuf );
	  sprintf( szOut, "%%%%Cannot open file \"%s\"", szBuf );
	  ConfigShellOutput( szOut );
	}
  }
  else
  {
	LogDebug( "Configurator_SourceSafe", "Does not set environment variable", VSS_DBDIR );
	sprintf( szOut, "%%%%Does not set environment variable \"%s\"", VSS_DBDIR );
	ConfigShellOutput( szOut );
  }

  if ( !found )
  {
	m_szUserIniFile[0] = 0;
	LogDebug( "Configurator_SourceSafe", "Cannot find user ini file", "" );
	sprintf( szOut, "%%%%Cannot find ss.ini file for user %s", GetSccUserName() );
	ConfigShellOutput( szOut );
	sprintf( szOut, "%%%%May be you are not legal SourceSafe user" );
	ConfigShellOutput( szOut );
  }

  // Other initialization
  BOOL fFound = FALSE;
  char *pLastFSlash;

  if( GetEnvironmentVariable( "TEMP", m_szTmpFile, sizeof( m_szTmpFile ) ) && strlen( m_szTmpFile ) ||
	  GetEnvironmentVariable( "TMP", m_szTmpFile, sizeof( m_szTmpFile ) ) && strlen( m_szTmpFile ) ||
	  GetEnvironmentVariable( "TMPDIR", m_szTmpFile, sizeof( m_szTmpFile ) ) && strlen( m_szTmpFile ) )
  {
	strcat( m_szTmpFile, "\\#ss_dis#.txt");
  }
  else
	strcpy( m_szTmpFile, "C:\\#ss_dis#.txt");

  if( GetEnvironmentVariable( VSS_ENVVAR, m_szToolSS, sizeof( m_szToolSS ) ) &&
      strlen( m_szToolSS ) ) 
  {
    n = strlen( m_szToolSS ) - 1;
    if( m_szToolSS[n] != '/' && m_szToolSS[n] != '\\' )
      strcat( m_szToolSS, "/" );  
    strcat( m_szToolSS, VSS_CMDTOOL_NAME ); 
    if( !_access( m_szToolSS, 0 ) )
      fFound = TRUE;
  }
  if( !fFound )
  {
    char *pszFile;
    if( SearchPath( NULL, VSS_CMDTOOL_NAME, NULL, sizeof( m_szToolSS ), 
      m_szToolSS, &pszFile ) )
      fFound = TRUE;
  }
  if( !fFound )
  {
    strcpy( m_szToolSS, m_pCMProvider->GetSccServerPath() );
    pLastFSlash = GetLastSlash( m_szToolSS );
    if ( pLastFSlash )
    {
      *(pLastFSlash+1) = 0;
      strcat( m_szToolSS, VSS_CMDTOOL_NAME );
      fFound = TRUE;
    }
  }

  if ( fFound )
  {
    pLastFSlash = GetLastSlash( m_szToolSS );
    *pLastFSlash = 0;
    LogDebug( "Configurator_SourceSafe", "m_szToolSS=", m_szToolSS );
    SetEnvironmentVariable("cm_tool_dir", m_szToolSS );
    *pLastFSlash = '\\';
  }
}

//--------------------------
static int getProjCommonSegmentLevel( char* path1, char* path2 )
{
  int nLevel = 0;
  if ( path1 && *path1 && path2 && *path2 && strnicmp(path1, path2, strlen(path2)) == 0 )
  {
	if ( *(path1 + strlen(path1) -1) != '/' )
		strcat( path1, "/" );
    char* pSub = path1;
	char* pSlash;
	while ( (pSlash=strchr(pSub,'/')) )
	{
	  *pSlash = 0;
	  int len = strlen(path1);
	  if ( strnicmp(path1, path2, len) == 0 && 
		   ( *(path2+len) == '/' || *(path2+len) == 0 ) )
	  {
		nLevel++;
		*pSlash = '/';
		pSub = pSlash+1;
	  }
	  else
	  {
		*pSlash = '/';
		break;
	  }
	}
	*(path1 + strlen(path1) -1) = 0;
  }
  return nLevel;
}

//--------------------------
static char FindProjectWorkPath( char* pszProject, char* pszMachine, char* pszDir )
{
  *pszDir = 0;

  lstWrkDir* pWrkDir = lstWrkDir::firstWrkDir;
  int nLevel =0;
  int nCurLevel;
  if ( strcmp(pszProject, "$/") == 0 )
  {
	while ( pWrkDir )
	{
	  if ( pWrkDir->isMachine(pszMachine) && strcmp( pWrkDir->getProjPath(), "$/") == 0 )
	  {
		  strcpy( pszDir, pWrkDir->getDirPath() );
		  break;
	  }
	  pWrkDir = pWrkDir->getNext();
	}
	return *pszDir;
  }

  while ( pWrkDir )
  {
	if ( pWrkDir->isMachine(pszMachine) )
	{
	  nCurLevel = getProjCommonSegmentLevel(pszProject, pWrkDir->getProjPath() );
	  if ( nLevel < nCurLevel )
	  {
		nLevel = nCurLevel;
		strcpy( pszDir, pWrkDir->getDirPath() );
	  }
	}
    pWrkDir = pWrkDir->getNext();
  }

  char* p = pszProject;
  for ( nCurLevel = 0; nCurLevel < nLevel; nCurLevel ++ )
  {
	  p= strchr( p+1, '/' );
  }
  if ( p )
	  strcat( pszDir, p );
  while ( (p=strchr(pszDir,'/')) ) *p='\\';
  return *pszDir;
}

//--------
BOOL Configurator_SourceSafe::FindProjAndLocalDir(char *cm_workfile, char *cm_revfile)
{
  LogDebug( "Configurator_SourceSafe", "FindProjAndLocalDir", "--->Build 11-09-99" );
  LogDebug( "Configurator_SourceSafe", "cm_workfile", cm_workfile );
  LogDebug( "Configurator_SourceSafe", "cm_revfile", cm_revfile );

  if ( Configurator_MSAPI::FindProjAndLocalDir(cm_workfile, cm_revfile) )
	return TRUE;

  BOOL bRet = FALSE;
  char pszProject[_MAX_PATH+2] = "";
  char pszLocalDir[_MAX_PATH+2] = "";
 
  lstProject::DeleteAll();

  if( cm_workfile && cm_revfile )
  {
    char *pLastFSlash, *pszRevFileName;
    char *pszExt = strrchr(cm_revfile, '.');

    strcpy( pszLocalDir, cm_workfile );
    pLastFSlash = GetLastSlash( pszLocalDir );
    if( pLastFSlash )
      *pLastFSlash = '\0';

    pszRevFileName = GetLastSlash( cm_revfile );
    LogDebug( "FindProjAndLocalDir", "pszRevFileName=", pszRevFileName );
    if( pszRevFileName )
    {
      LogDebug( "FindProjAndLocalDir", "pszExt=", pszExt );
      if ( pszExt )
        {
          if ( stricmp( pszExt, ".pset" ) == 0 && stricmp( m_szPsetFName, cm_revfile ) == 0 )
          {
            strcpy( pszProject, m_szPsetProject );
            bRet = TRUE;
            LogDebug( "FindProjAndLocalDir", "Old pset file pszProject=", pszProject );
          }
          else if ( stricmp( m_szSrcFName, cm_revfile ) == 0 )
          {
            strcpy( pszProject, m_szSrcProject );
            bRet = TRUE;
            LogDebug( "FindProjAndLocalDir", "Old src file pszProject=", pszProject );
          }

          if ( bRet )
          {
            LogDebug( "FindProjAndLocalDir", "Old file!!", "Try to open old project" );
			SetEnv( CM_PROJECT,    pszProject );
			SetEnv( CM_LOCALDIR,   pszLocalDir );
            return bRet;
          }
        }

// Try command "ss locate <file name>"
    int nLen;
	char szLine[ 2*MAX_PATH ];
    char pszCmd [ _MAX_PATH ];
	char *pszSrcFileName = pLastFSlash+1;

    LogDebug( "Configurator_SourceSafe", "UserName:", GetSccUserName() );
	LogDebug( "Configurator_SourceSafe", "user.ini file:", m_szUserIniFile );

	  sprintf( pszCmd, "%s locate \"%s\"", m_szToolSS, pszSrcFileName );
      LogDebug( "FindProjAndLocalDir", "Try command:", pszCmd );
	  FILE* pFOut = CM_OpenPipe( pszCmd );
	  if ( pFOut )
	  {
        while( !feof(pFOut) && CM_ReadPipe(szLine, sizeof(szLine), pFOut)  )
        {
          LogDebug( "FindProjAndLocalDir", "ss locate output:", szLine );
          nLen = strlen( szLine );
          if( nLen )
          {
            if( *(szLine + nLen - 1) == '\n' )
              *(szLine +nLen - 1) = '\0';
            if( *szLine == '$' && !strstr( szLine, "(Deleted)" ) )
            {
              LogDebug( "FindProjAndLocalDir", "new lstProject:", szLine );
              lstProject::CreateNew( szLine );
            }
            else
              LogDebug( "FindProjAndLocalDir", "not $ or (Deleted)", "skip this line" );
          }
          else
            LogDebug( "FindProjAndLocalDir", "ss locate output has", "ZERO length" );
        }
		CM_ClosePipe(pFOut);
	  }
	  else
		LogDebug( "FindProjAndLocalDir", "pipe is NOT created", "" );


	  if ( lstProject::firstProject == 0 )
	  {
		LogDebug( "FindProjAndLocalDir", "output from ss locate ...is empty", "" );
		return FALSE;
	  }

	  //------------------
	  // Now we are testing all checked out files..
	  BOOL bProjectFound = FALSE;
	  lstProject* pProject = lstProject::firstProject;

	  while ( !bProjectFound && pProject )
	  {
        LogDebug( "FindProjAndLocalDir", "project from ss", pProject->getName() );

		sprintf( pszCmd, "%s status -U%s \"%s/%s\"", m_szToolSS, GetSccUserName(), pProject->getName(), pszSrcFileName );
		LogDebug( "FindProjAndLocalDir", "Try command:", pszCmd );
		pFOut = CM_OpenPipe( pszCmd );
		if ( pFOut && CM_ReadPipe(szLine, sizeof(szLine), pFOut) )
		{
		  CM_ClosePipe(pFOut);
		  if ( strnicmp( szLine, VSS_STATUS_LINE, strlen(VSS_STATUS_LINE)) )
		  {
			LogDebug( "FindProjAndLocalDir", "File checked out -> Try path", "" );
			DeleteFile( m_szTmpFile );
			sprintf( pszCmd, "%s status -U%s -O\"%s\" \"%s/%s\"", m_szToolSS, GetSccUserName(), m_szTmpFile, pProject->getName(), pszSrcFileName );
			if ( (pFOut = CM_OpenPipe( pszCmd )) )
			  CM_ClosePipe(pFOut);
			*szLine =0;
			if ( (pFOut = fopen( m_szTmpFile, "r" )) && fgets(szLine, sizeof(szLine), pFOut) && *szLine )
			{
			  while ( (nLen=strlen(szLine)) && *(szLine+nLen-1) == 0x0a || *(szLine+nLen-1) == 0x0d ) *(szLine+nLen-1) = 0;
			  if ( strnicmp( pszLocalDir, szLine + nLen - strlen(pszLocalDir), strlen(pszLocalDir) ) == 0 )
			  {
				pProject->saveName( pszProject );
				bProjectFound = TRUE;
	  			LogDebug( "FindProjAndLocalDir", "File checked out from project", pszProject );
			  }
			}
			if ( pFOut )
			  fclose(pFOut);
		  }
		}
		pProject = pProject->getNext();
	  }

	  if ( !bProjectFound )
	  {
		//------------------
		// Reding user.ini file
		LogDebug( "FindProjAndLocalDir", "Reding user.ini file:", m_szUserIniFile );
		FILE* fileSS = 0;
		if ( (fileSS= fopen(m_szUserIniFile, "r" )) == 0 )
		{
		  LogDebug( "FindProjAndLocalDir", "cannot open file:", m_szUserIniFile );
		  return FALSE;
		}
		char szSSiniLine[ _MAX_PATH +64 ];
		while ( !feof(fileSS) && fgets(szSSiniLine, sizeof(szSSiniLine)-2, fileSS) )
		{
		  if ( strncmp( szSSiniLine, "[$/", 3) != 0 )
			  continue;
		  while ( fgets(szLine, sizeof(szLine)-2, fileSS) && strnicmp(szLine, VSS_DIR_LINE, strlen(VSS_DIR_LINE)) == 0 )
		  {
			LogDebug( "FindProjAndLocalDir", "Line to lstWrkDir::CreateNew", szLine );
			lstWrkDir::CreateNew( szSSiniLine+1, szLine, m_ppszSSiniEnv, m_nSSiniEnv );
		  }
		}
		fclose(fileSS);

		//------------------
		// Again reading list of projects from ss locate..
		for ( int n = 0; !bProjectFound && n< m_nSSiniEnv; n++ )
		{
	      LogDebug( "Configurator_SourceSafe", "seek for machine", m_ppszSSiniEnv[n] );
		  lstProject* pProject = lstProject::firstProject;
		  LogDebug( "FindProjAndLocalDir", "searching project for LocalDir", pszLocalDir );
		  while ( !bProjectFound && pProject )
		  {
			LogDebug( "FindProjAndLocalDir", "project from ss", pProject->getName() );

			if ( FindProjectWorkPath( pProject->getName(), m_ppszSSiniEnv[n], szSSiniLine ) )
			{
			  LogDebug( "FindProjAndLocalDir", "Directory from ss.ini:", szSSiniLine );
			  if ( stricmp( pszLocalDir, szSSiniLine ) == 0 )
				{
				  bProjectFound = TRUE;
				  pProject->saveName( pszProject );
				  LogDebug( "FindProjAndLocalDir", "New Directory OK==> project:", pszProject );
				}
			   else
				LogDebug( "FindProjAndLocalDir", "!!!reject:", "Directory from ss.ini and LocalDir are differenrt" );
			}
			pProject = pProject->getNext();
		  }
		}
	  }
	  
	  lstProject::DeleteAll();
	  lstWrkDir::DeleteAll();

//---------------------------------------------------
	  if ( bProjectFound )
	  {
		LogDebug( "FindProjAndLocalDir", "Project", pszProject );
		LogDebug( "FindProjAndLocalDir", "LocalDir", pszLocalDir );
		SetEnv( CM_PROJECT,    pszProject );
		SetEnv( CM_LOCALDIR,   pszLocalDir );
		return TRUE;
	  }
	  else
	  {
		LogDebug( "FindProjAndLocalDir", "project not found for", cm_workfile );
//		char szOut[2*_MAX_PATH];
//		sprintf( szOut, "%%%%File \"%s\" is not contained in any SourceSafe working area of this machine", cm_workfile );
//		ConfigShellOutput( szOut );
	  }
//---------------------------------
	}
  }
  else //  if( cm_workfile && cm_revfile )
  {
    LogDebug( "FindProjAndLocalDir", "cm_workfile == 0 || cm_revfile== 0", "" );
  }

  return bRet;
}


//-----------------------------------
int Configurator_SourceSafe::cm_setup_src( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int nResult = 0;
  LogDebug( "cm_setup_src", "CM_WORKFILE", GetEnv( CM_WORKFILE ) );
  SetEnv( CM_REVISION_FILE, GetEnv( CM_WORKFILE ) );
  LogDebug( "cm_setup_src", "CM_REVISION_FILE", GetEnv( CM_REVISION_FILE ) );
  if ( strcmp(pszCommand, "copy_src") == 0  && 
       strcmp(pszWorkPath, pszSysPath) == 0     )
  {
	SetEnv( CM_SYSFILE, GetEnv( CM_WORKFILE ) );
    LogDebug( "cm_setup_src", "copy_src", pszSysPath );
  }
  else
  {
    LogDebug( "cm_setup_src", "", "start Configurator_MSAPI::cm_setup_src" );
    LogDebug( "cm_setup_src", "pszWorkPath", pszWorkPath );
    LogDebug( "cm_setup_src", "pszSysPath", pszSysPath );
    LogDebug( "cm_setup_src", "pszCommand", pszCommand );
    LogDebug( "cm_setup_src", "pszOpt", pszOpt );
    nResult = Configurator_MSAPI::cm_setup_src(pszWorkPath,pszSysPath,pszCommand,pszOpt);
    if ( !m_fScriptInvoked )
      nResult = 0;
  }
  
  return nResult;
}

//-----------------------------------
int Configurator_SourceSafe::cm_setup_pset( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int nResult = 0;
  SetEnv( CM_REVISION_FILE, GetEnv( CM_WORKFILE ) );
  if ( strcmp(pszCommand, "copy_pset") == 0  && 
       strcmp(pszWorkPath, pszSysPath) == 0     )
  {
	  SetEnv( CM_SYSFILE, GetEnv( CM_WORKFILE ) );
    LogDebug( "cm_setup_pset", "copy_pset", pszSysPath );
  }
  else
  {
    LogDebug( "cm_setup_pset", "", "start Configurator_MSAPI::cm_setup_pset" );
    nResult = Configurator_MSAPI::cm_setup_pset(pszWorkPath,pszSysPath,pszCommand,pszOpt);
    if ( !m_fScriptInvoked )
      nResult = 0;
  }
  
  return nResult;
}

//-----------------------------------
#define VSS_STATKEY	"  Version:"

int Configurator_SourceSafe::cm_stat()
{
  int nResult = Configurator::cm_stat();

  if ( !m_fScriptInvoked )
  {
    nResult = 0;
  }
	else
	{
		ConfigShellResult* pConfigShellResult = ConfigShellFindOutput(VSS_STATKEY );
		if ( pConfigShellResult )
		{
			char*	pszRev = strstr(pConfigShellResult->GetText(), VSS_STATKEY) + strlen(VSS_STATKEY);
			if ( pszRev )
			{
				while ( *pszRev == ' ' || *pszRev == '\t' )
					pszRev++;
				SetEnv( CM_RESULT, pszRev );
			}
		}
	}

  return nResult;
}

//---------------------------------------------
int Configurator_SourceSafe::cm_lsco()
{
  int nResult = Configurator::cm_lsco();
  if ( !m_fScriptInvoked )
  {
	int nLenDir =0;
	char szDir[MAX_PATH+1];
	char* p;
  char *cm_workfile = GetEnv( CM_WORKFILE );

  LogDebug( "cm_lsco", "cm_workfile=", cm_workfile );
	if ( cm_workfile && *cm_workfile )
	{
	  strcpy( szDir, cm_workfile );
	  if ( (GetFileAttributes(szDir) & FILE_ATTRIBUTE_DIRECTORY) == 0 )
	  {
		p = strrchr( szDir, '\\' );
		if ( p ) *p = 0;
	  }
	  else if ( *(szDir + strlen(szDir)-1) != '\\' )
		strcat( szDir, "\\" );

	  nLenDir = strlen( szDir );
	}

	//-----------------------------------------
	FILE* pFOut;
	char szCurProj[ 2*MAX_PATH ] = "";
	char pszCmd [ 2*_MAX_PATH ];
	char line   [ 3*_MAX_PATH ];
	int  i, len;
	char ffile[5000];
	char fpath[5000];
	char sch[5000];
	char szOut[5000];

	// Read current project
	strcpy( pszCmd, m_szToolSS );
	strcat( pszCmd, " project" );
  LogDebug( "cm_lsco", "CM_OpenPipe: pszCmd=", pszCmd );
	pFOut = CM_OpenPipe( pszCmd );
	if ( pFOut )
	{
      if ( CM_ReadPipe(line, sizeof(line), pFOut)  )
	  {
		if ( (p=strchr(line, '$')) )
		  strcpy( szCurProj, p );
		else
		  LogDebug( "cm_lsco", "ss project", "cannot define current project" );
	  }
	  else
		LogDebug( "cm_lsco", "ss project", "cannot read pipe" );
	  CM_ClosePipe(pFOut);
	}
	else
	  LogDebug( "cm_lsco", "ss project", "pipe is NOT created" );
	
	// Setting current project to the root
	strcpy( pszCmd, m_szToolSS );
	strcat( pszCmd, " cp $/" );
  LogDebug( "cm_lsco", "CM_OpenPipe: pszCmd=", pszCmd );
	pFOut = CM_OpenPipe( pszCmd );
	if ( pFOut == 0 )
	{
	  LogDebug( "cm_lsco", "ss project", "pipe is NOT created" );
	  return 1;
	}
	CM_ClosePipe(pFOut);

	 // Reading the projects list
	DeleteFile( m_szTmpFile );
	sprintf( pszCmd, "%s status -R -NS -O\"%s\" -U%s", m_szToolSS, m_szTmpFile, GetSccUserName() );
  LogDebug( "cm_lsco", "CM_OpenPipe: pszCmd=", pszCmd );
	if ( (pFOut = CM_OpenPipe( pszCmd )) )
	{
	  CM_ClosePipe(pFOut);
    LogDebug( "cm_lsco", "open file", m_szTmpFile );
	  if ( (pFOut = fopen( m_szTmpFile, "r" )) )
	  {
		  while ( !feof(pFOut) && fgets(line, sizeof(line), pFOut) )
		  {
		    while ( (len=strlen(line)) && ( *(line + len -1) == 0x0d || *(line + len -1) == 0x0a) )
			   *(line + len -1) = 0;

//        LogDebug( "cm_lsco", "fgets(line): ", line );
		    if ( line[0] == 0 || line[0] == '$' || strnicmp(line, VSS_LSCO_LINE, strlen(VSS_LSCO_LINE))==0 )
			    continue;

        LogDebug( "cm_lsco", "fgets(line): ", line );
		    int field=0;
		    int fcnt=0;
		    int pcnt=0;
		    for(i=0;i< (int)strlen(line);i++) 
        {
			    if(line[i]==' ' && field!=4) 
          {
				    while(line[i]== ' ') i++;
            if ( strnicmp(&(line[i]), "Exc ", 4) )
				      field++;
			    }
			    switch(field) 
          {
				    case 0 : ffile[fcnt++] = line[i]; break;
				    case 4 : fpath[pcnt++] = line[i]; break;
			    }
		    }
		    ffile[fcnt]=0;
		    fpath[pcnt]=0;

        LogDebug( "cm_lsco", "fpath=", fpath );
        LogDebug( "cm_lsco", "ffile=", ffile );
		    sprintf(sch, "%s\\%s", fpath, ffile );
		    WIN32_FIND_DATA data;
		    FindFirstFile(sch,&data);

		    sprintf( szOut, "%%%%%s\\%s", fpath, data.cFileName );
 		    if ( nLenDir == 0 || strnicmp( szOut+2, szDir, nLenDir ) == 0 )
        {
          LogDebug( "cm_lsco", "szOut=", szOut );
			    ConfigShellOutput( szOut );
        }
		  }
	  }
	  else
		  LogDebug( "cm_lsco", "cannot open file", m_szTmpFile );
	}
	else
	  LogDebug( "cm_lsco", "cp $/", "pipe is NOT created" );
	DeleteFile( m_szTmpFile );

	// Reset the current project to the prev.
	if ( szCurProj[0] )
	{
	  sprintf( pszCmd, "%s cp %s", m_szToolSS, szCurProj );
    LogDebug( "cm_lsco", "CM_OpenPipe: pszCmd=", pszCmd );
	  pFOut = CM_OpenPipe( pszCmd );
	  if ( pFOut == 0 )
		  LogDebug( "cm_lsco", "reset project", "pipe is NOT created" );
	  else
		  CM_ClosePipe(pFOut);
	}

  
  }
  return 0;
}
