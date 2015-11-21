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
#include <stdlib.h>
#include <stdio.h>

#include <../ConfigShell/ComIface.h>
#include <../include/DisRegistry.h>

#define	_THIS_FILE_		"Configtool"

#define INDX_CMD_CHECKOUT_UNRES 1
#define INDX_CMD_CHECKOUT_RES   2
#define INDX_CMD_CHECKIN        3
#define INDX_CMD_UNCHECK        4
#define INDX_CMD_LSCO           5
#define INDX_CMD_CM_INFO        6
#define INDX_CMD_EXEC           7

#define TEXT_CMD_CHECKOUT_UNRES  "copy_src"
#define TEXT_CMD_CHECKOUT_RES    "get_src"
#define TEXT_CMD_CHECKIN         "put_src"
#define TEXT_CMD_UNCHECK         "unget_src"
#define TEXT_CMD_LSCO            "lsco"
#define TEXT_CMD_CM_INFO         "cm_info"
#define TEXT_CMD_EXEC            "cm_exec"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//		Globals
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

int		g_nErr =0;
char*		g_pszCmd = NULL;
int		g_nCmdIndx =0;
char*		g_pszCmdText = NULL;
char**		g_ppszFilePath = NULL;
char*		g_pszComment = NULL;
char*		g_pszCommand = NULL;
char*		g_pszOptions = NULL;
char*		g_pszPSETHOME = NULL;
char*		g_pszSysFileName = NULL;
int		g_nActions = 0;
BOOL		g_bCustomCM = FALSE;

IConfigShell*	 g_pIConfigShell = NULL;

BOOL	InitShell();
void	DestroyShell();
BOOL	RunShell();
char*	GetResultShell();

int	ProcessCommandOption(char*, char*);
void	ProcessFileName(char*);
int	ProcessCommandLine(int argc, char* argv[]);

BOOL	IsCommand(char*);
int	CreateCommand(int iFName);
void	PrintResult();

void	DestroyAll();
void	DestroyShell();
void	DestroyFilePath();
void	DestroyCmd();

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//		Interface to ConfigShell
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void DestroyShell()
{
  if ( g_pIConfigShell )
  {
    IConfigShell* pIConfigShell = g_pIConfigShell;
    g_pIConfigShell = NULL;
    pIConfigShell->Release();
    CoUninitialize();
  }
}

//------------------------------
BOOL InitShell()
{
  BOOL bRet = FALSE;
  HRESULT hr;
  wchar_t*	pwcsCMName = NULL ;
  wchar_t*	pwcsScriptDir;
  wchar_t*	pwcsCustom;
  char	szScriptDir[_MAX_PATH] = "";
  char*	p;
  int		nLength;

  if ( getenv("PSETHOME") )
  {
	strcpy (szScriptDir, getenv("PSETHOME") );
	if (szScriptDir[0] && (p= szScriptDir+ strlen(szScriptDir)-1 ) &&
		(*p == '/' || *p == '\\' ) )
	  *p='\0';	
	strcat(szScriptDir,"\\bin\\");
  }
  else
  {
    LogDebug( "InitShell", "Environment variable 'PSETHOME' is not defined", "?!?" );
	  printf("Environment variable 'PSETHOME' is not defined\n");
	  return FALSE;
  }
  
  CoInitialize(NULL);
  hr = CoCreateInstance(CLSID_ConfigShell,
 		  NULL, CLSCTX_LOCAL_SERVER, 
		  IID_IConfigShell, (void**)&g_pIConfigShell);
  if ( FAILED(hr) )
  {
    LogDebug( "InitShell", "Unable to initialize ConfigShell", "?!?" );
    printf("Unable to initialize ConfigShell\n");
  }
  else
  {
	if ( g_bCustomCM )
	{
	  nLength = strlen( "Custom" ) +1;
	  pwcsCustom = new wchar_t [ nLength ]; 
	  mbstowcs( pwcsCustom, "Custom", nLength ); 
	  hr = g_pIConfigShell->SetPSETCFG(pwcsCustom);
	  delete [] pwcsCustom;
	  if ( !SUCCEEDED(hr) )
        LogDebug( "InitShell", "SetPSETCFG(\"Custom\")", "is not succeeded ?!?" );
	}

    hr = g_pIConfigShell->InitCM(&pwcsCMName);
    if ( SUCCEEDED(hr) )
    {
	  p = szScriptDir+ strlen(szScriptDir);
	  nLength = (wcslen(pwcsCMName) +1)*sizeof(wchar_t);
	  wcstombs( p, pwcsCMName, nLength ); 
  ::	CoTaskMemFree(pwcsCMName) ;

	  nLength = strlen( szScriptDir ) +1;
	  pwcsScriptDir = new wchar_t [ nLength ]; 
	  mbstowcs( pwcsScriptDir, szScriptDir, nLength ); 
	  hr = g_pIConfigShell->SetPSETCFG(pwcsScriptDir);
	  delete [] pwcsScriptDir;
	  bRet = TRUE;
    }
    else
      LogDebug( "InitShell", "InitCM", "is not succeeded ?!?" );
  }

  if ( !bRet )
    DestroyShell();
  return bRet;
}

//------------------------------
BOOL RunShell()
{
  if ( g_pIConfigShell == NULL && !InitShell() )
  {
    LogDebug( "main", "InitShell()", "!?! FALSE !?!" );
	return FALSE;
  }

  if ( g_pIConfigShell )
  {
    HRESULT hr;
    int	    nLengthIn = strlen( g_pszCmd ) +1;
    wchar_t* pwcsCmd = new wchar_t [ nLengthIn ];
    if ( pwcsCmd )
    {
      LogDebug( "RunShell", "g_pszCmd=", g_pszCmd );
      mbstowcs( pwcsCmd, g_pszCmd, nLengthIn );
	    DestroyCmd();

      hr = g_pIConfigShell->Run(pwcsCmd) ;
      delete [] pwcsCmd;
      return (SUCCEEDED(hr));
    }
    else
      LogDebug( "RunShell", "not enough memory", "?!?" );
  }
  else
    LogDebug( "RunShell", "g_pIConfigShell==NULL", "?!?" );
  
  return FALSE;
}

//------------------------------
char*	GetResultShell()
{
  if ( g_pIConfigShell )
  {
    HRESULT	hr;
    wchar_t*	pwcsOut = NULL ;
    char*	pszOut;
    int		nLengthOut;
    hr =g_pIConfigShell->GetResult(&pwcsOut);
    if ( !SUCCEEDED(hr) || pwcsOut == NULL )
      return NULL;

    nLengthOut = (wcslen(pwcsOut) +1)*sizeof(wchar_t) ;
    pszOut = new char [ nLengthOut ];
    if ( pszOut )
      wcstombs( pszOut, pwcsOut, nLengthOut ); 

    ::CoTaskMemFree(pwcsOut);
    return pszOut;
  }
  else
    return NULL;
}		
						 

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//		Process the command
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void DestroyFilePath()
{
  if ( g_ppszFilePath )
  {
	for ( int i=0; i< g_nActions; i++)
	  delete [] (*(g_ppszFilePath+ i));
    delete [] g_ppszFilePath;
  }
  g_ppszFilePath = NULL;
}

void DestroyCmd()
{
  if ( g_pszCmd )
	delete g_pszCmd;
  g_pszCmd = NULL;
}

void DestroyAll()
{
  DestroyShell();
  DestroyFilePath();
  DestroyCmd();
  if ( g_pszCmdText ) delete [] g_pszCmdText;
  if ( g_pszComment ) delete [] g_pszComment;
  if ( g_pszCommand ) delete [] g_pszCommand;
  if ( g_pszOptions ) delete [] g_pszOptions;
  if ( g_pszPSETHOME ) delete [] g_pszPSETHOME;
  if ( g_pszSysFileName ) delete [] g_pszSysFileName;
  g_pszCmdText = NULL;
  g_pszComment = NULL;
  g_pszCommand = NULL;
  g_pszOptions = NULL;
  g_pszPSETHOME = NULL;
  g_pszSysFileName = NULL;

  g_nCmdIndx =0;
  g_nErr =0;
}


BOOL IsCommand(char* pszCmd)
{
  if ( _stricmp( pszCmd, "co" )== 0 || _stricmp( pszCmd, "checkout" )== 0 )
  {
	  g_nCmdIndx  = INDX_CMD_CHECKOUT_UNRES;
	  g_pszCmdText= TEXT_CMD_CHECKOUT_UNRES;
  }
  else if ( _stricmp( pszCmd, "ci" ) == 0 || _stricmp( pszCmd, "checkin" ) == 0 )
  {
	  g_nCmdIndx  = INDX_CMD_CHECKIN;
	  g_pszCmdText= TEXT_CMD_CHECKIN;
  }
  else if ( _stricmp( pszCmd, "unco" ) == 0 || _stricmp( pszCmd, "uncheckout" ) == 0 )
  {
	  g_nCmdIndx  = INDX_CMD_UNCHECK;
	  g_pszCmdText= TEXT_CMD_UNCHECK;
  }
  else if ( _stricmp( pszCmd, "lsco" ) == 0 )
  {
	  g_nCmdIndx  = INDX_CMD_LSCO;
	  g_pszCmdText= TEXT_CMD_LSCO;
  }
  else if ( _stricmp( pszCmd, "cm_info" ) == 0 || _stricmp( pszCmd, "info" ) == 0 )
  {
	  g_nCmdIndx  = INDX_CMD_CM_INFO;
	  g_pszCmdText= TEXT_CMD_CM_INFO;
  }
  else if ( _stricmp( pszCmd, "exec" ) == 0 )
  {
	  g_nCmdIndx  = INDX_CMD_EXEC;
	  g_pszCmdText= TEXT_CMD_EXEC;
  }
  return (BOOL)g_nCmdIndx;
}

//------------------------------------------------------

void ProcessFileName( char* pszFileName )
{
  int nErr = 1;
  char	szFPath[_MAX_PATH+1];
  char	szFName[_MAX_PATH+1];
  strcpy(szFName, pszFileName );

  char* path;
  while ( (path=strchr(szFName,'/')) ) *path = '\\';
  path = _fullpath(szFPath, szFName, _MAX_PATH );
  if ( path == 0 )
  {
    printf("Wrong file name: %s\n", pszFileName);
    g_nErr = 1;
  	return;
  }

  char** ppszFilePath = g_ppszFilePath;
  while ( *ppszFilePath ) ppszFilePath++;

  *ppszFilePath = new char [ _MAX_PATH+1 ];
  if ( *ppszFilePath )
		strcpy( *ppszFilePath, szFPath);
  else
  {
    printf("Not enough memory to store file path\n");
		g_nErr = 1;
		return;
  }
}

int ProcessCommandOption( char* pszOption, char* psz )
{
  int nRet = 0;
  int nErr = 0;
  char* pszToken =0;
  if ( _stricmp(pszOption, "e" ) == 0 && psz )
  {
	nRet = 1;
	if ( g_pszCommand ) delete [] g_pszCommand;
	g_pszCommand = new char [ strlen(psz) +1 ];
	if ( g_pszCommand )
	  strcpy( g_pszCommand, psz );
	else
	{
	  printf("Not enough memory to store Command\n");
	  nErr = 1;
	}
  }
  else if ( _stricmp(pszOption, "c" ) == 0 && psz )
  {
	nRet = 1;
	if ( g_pszComment ) delete [] g_pszComment;
	g_pszComment = new char [ strlen(psz) +1 ];
	if ( g_pszComment )
	  strcpy( g_pszComment, psz );
	else
	{
	  printf("Not enough memory to store comment\n");
	  nErr = 1;
	}
  }
  else if ( _strnicmp(pszOption, "cf", 2 ) == 0 && psz )
  {
	nRet = 1;
	if ( g_pszComment ) delete [] g_pszComment;
	g_pszComment = new char [ strlen(psz) +1 ];
	if ( g_pszComment )
	  strcpy( g_pszComment, psz );
	else
	{
	  printf("Not enough memory to store comment\n");
	  nErr = 1;
	}
  }
  else if ( _strnicmp(pszOption, "c\"", 2 ) == 0 )
  {
	nRet = 0;
	if ( g_pszComment ) delete [] g_pszComment;
	g_pszComment = new char [ strlen(pszOption) +1 ];
	if ( g_pszComment )
	  strcpy( g_pszComment, pszOption+1 );
	else
	{
	  printf("Not enough memory to store comment\n");
	  nErr = 1;
	}
  }
  else if ( _stricmp(pszOption, "custom" ) == 0 )
  {
	nRet = 0;
	g_bCustomCM = TRUE;
  }
  else if ( _stricmp(pszOption, "nc" ) == 0 )
  {
	nRet = 0;
	if ( g_pszComment ) delete [] g_pszComment;
	g_pszComment = 0;
  }
  else if ( _strnicmp(pszOption, "reserve", 3 ) == 0 )
  {
	if ( g_nCmdIndx == INDX_CMD_CHECKOUT_UNRES )
	{
	  g_pszCmdText= TEXT_CMD_CHECKOUT_RES;
	  g_nCmdIndx = INDX_CMD_CHECKOUT_RES;
	}
	nRet = 0;
  }
  else if ( _strnicmp(pszOption, "unreserve", 5 ) == 0 )
  {
	if ( g_nCmdIndx == INDX_CMD_CHECKOUT_RES )
	{
	  g_pszCmdText= TEXT_CMD_CHECKOUT_UNRES;
	  g_nCmdIndx = INDX_CMD_CHECKOUT_UNRES;
	}
	nRet = 0;
  }
  else if ( _strnicmp(pszOption, "sysfile", 4 ) == 0 && psz && *psz )
  {
	nRet = 1;
	char	szFPath[_MAX_PATH+1];
	char	szFName[_MAX_PATH+1];
	strcpy(szFName, psz );

	char* path;
	while ( (path=strchr(szFName,'/')) ) *path = '\\';
	path = _fullpath(szFPath, szFName, _MAX_PATH );
	if ( path == 0 )
	{
	  printf("Wrong file name: %s\n", psz);
	  nErr = 1;
	}
	else
	{
	  if ( g_pszSysFileName ) delete [] g_pszSysFileName;
	  g_pszSysFileName = new char [ strlen(szFPath) +1 ];
	  if ( g_pszSysFileName )
	    strcpy( g_pszSysFileName, szFPath );
	  else
	  {
	    printf("Not enough memory to store SysFileName\n");
	    nErr = 1;
	  }
	}
  }
  else
  {
	  nRet = 1;
	  if ( g_pszOptions ) delete [] g_pszOptions;
	    g_pszOptions = new char [ strlen(pszOption) +1 ];
	  if ( g_pszOptions )
	    strcpy( g_pszOptions, pszOption );
	  else
	  {
	    printf("Not enough memory to store options\n");
	    nErr = 1;
	  }
  }

  if ( nErr )
	  g_nErr = nErr;
  return nRet;
}



//------------------------------------------------------
int ProcessCommandLine(int argc, char* argv[])
{
  if ( argc < 2 )
  {
	  printf("Too few arguments\n");
	  return 1;
  }
  if ( !IsCommand(argv[1]) )
  {
	  printf("Unknown subcommand: %s\n", argv[1] );
	  return 1;
  }

  for ( int iArg=2; iArg < argc ; iArg++ )
  {
	  if ( *argv[iArg] == '-' || *argv[iArg] == '/' )
	    iArg += ProcessCommandOption( argv[iArg]+1, argv[iArg+1] );
	  else
	  {
	    if ( g_ppszFilePath == NULL )
	    {
		    g_nActions = argc - iArg;
		    g_ppszFilePath = new char* [g_nActions];
		    for ( int i=0; i< g_nActions; i++)
			    *(g_ppszFilePath+ i) = NULL;
	    }
	    ProcessFileName( argv[iArg] );
	  }
  }

  if ( g_ppszFilePath == NULL && g_nCmdIndx != INDX_CMD_EXEC && g_nCmdIndx != INDX_CMD_CM_INFO && g_nCmdIndx != INDX_CMD_LSCO )
  {
	  printf("Element pathname required.\n");
	  g_nErr =1;
  }

  return g_nErr;
}



//------------------------------------------------------
int CreateCommand(int iFName)
{
	char*	pszFilePath = NULL;
	if ( g_ppszFilePath && iFName >= 0 )
		pszFilePath = *(g_ppszFilePath+ iFName);

	int nLenComment = ( g_pszComment ? strlen(g_pszComment) : 0);
	int nLenCommand = ( g_pszCommand ? strlen(g_pszCommand) : 0);
	int nLenOptions = ( g_pszOptions ? strlen(g_pszOptions) : 0);
	int nLenFPath   = ( pszFilePath ? strlen(pszFilePath) : 0);
	int nLen = 2*nLenFPath + nLenCommand + nLenComment + nLenOptions +128;
	g_pszCmd = new char [ nLen ];
	if ( g_pszCmd )
	{
	  sprintf( g_pszCmd, "exec ConfigShell %s", g_pszCmdText );
	  if ( g_pszOptions )
	  {
		  strcat( g_pszCmd, " \"" ); 
		  strcat( g_pszCmd, g_pszOptions ); 
		  strcat( g_pszCmd, "\"" ); 
	  }
	  else
		 strcat( g_pszCmd, " default" ); 

    if ( g_nCmdIndx == INDX_CMD_EXEC )
	  {
      strcat( g_pszCmd, " " );
      if ( nLenCommand )
        strcat( g_pszCmd, g_pszCommand );
		  else
			  strcat( g_pszCmd, "unknown" );

		  if ( pszFilePath )
		  {
		    strcat( g_pszCmd, " \"" ); 
		    strcat( g_pszCmd, pszFilePath );
		    strcat( g_pszCmd, "\"" );
		  }
		  else
		    strcat( g_pszCmd, " \"\"" ); 
	  }
    if ( g_nCmdIndx == INDX_CMD_CM_INFO )
	  {
		  if ( pszFilePath )
		  {
		    strcat( g_pszCmd, " \"" ); 
		    strcat( g_pszCmd, pszFilePath );
		    strcat( g_pszCmd, "\"" );
		  }
		  else
		    strcat( g_pszCmd, " \"\"" ); 
	  }
	  else if ( g_nCmdIndx == INDX_CMD_LSCO && pszFilePath )
	  {
		strcat( g_pszCmd, " \"" ); 
		strcat( g_pszCmd, pszFilePath );
		strcat( g_pszCmd, "\"" );
	  }
	  else if ( g_nCmdIndx == INDX_CMD_CHECKIN )
	  {
		strcat( g_pszCmd, " \"" );
		if ( nLenComment )
			strcat( g_pszCmd, g_pszComment );
		else
			strcat( g_pszCmd, "\".\"" );
		strcat( g_pszCmd, "\" \"" );
		strcat( g_pszCmd, pszFilePath );
		strcat( g_pszCmd, "\" \"" );
		if ( g_pszSysFileName )
			strcat( g_pszCmd, g_pszSysFileName );
		else
	  		strcat( g_pszCmd, pszFilePath );
		strcat( g_pszCmd, "\" \"\" \"SRC\" ");
	  }
	  else if ( g_nCmdIndx == INDX_CMD_CHECKOUT_UNRES || g_nCmdIndx == INDX_CMD_CHECKOUT_RES )
	  {
		strcat( g_pszCmd, " \"" );
		if ( g_pszSysFileName )
			strcat( g_pszCmd, g_pszSysFileName );
		else
	  		strcat( g_pszCmd, pszFilePath );
		strcat( g_pszCmd, "\" \"" );
		strcat( g_pszCmd, pszFilePath );
		strcat( g_pszCmd, "\" \"\" \"\" \"SRC\" ");
	  }
	  else if ( g_nCmdIndx == INDX_CMD_UNCHECK )
	  {
		strcat( g_pszCmd, " \"" );
		if ( g_pszSysFileName )
			strcat( g_pszCmd, g_pszSysFileName );
		else
	  		strcat( g_pszCmd, pszFilePath );
		strcat( g_pszCmd, "\" \"" );
		strcat( g_pszCmd, pszFilePath );
		strcat( g_pszCmd, "\" \"\" \"\" \"SRC\" ");
	  }
	}
	else
	{
	  printf("Not enough memory.\n");
	  return 1;
	}
  return 0;
}

//------------------------------------------------------

void PrintResult()
{
  char*	pszResult;
  char*	pszText;

  while ( (pszResult = GetResultShell() ) )
  {
	pszText = pszResult;
	if ( strncmp(pszText,"%%stat;", 7) == 0 )
	  pszText = 0;
	else if ( *pszText == '%' && *(pszText+1) == '%' )
	  pszText += 2;
	else if ( strncmp(pszText,"$error", 6) == 0 )
	{
	  g_nErr = 1;
	  pszText = 0;
	}
        else if ( strncmp(pszText,"$ok", 3) ==0  )
	  pszText = 0;

	if ( pszText && *pszText )
	  printf("%s\n", pszText);
    delete [] pszResult;
  }
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//		Main
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

extern "C" int _execv( const char *cmdname, const char *const *argv  );

int main( int argc, char* argv[] )
{

#ifdef _DEBUG
  if ( GetDisRegistryInt("Debug", "ConfigTool", 0) )
	_asm int 3;
#endif

#ifdef _DEBUG
  char szDbg[1024] = "";
  int iDbg;
  for ( iDbg = 0; iDbg < argc; iDbg++)
  {
    strcat( szDbg, argv[iDbg] );
    strcat( szDbg, " " );
  }
  LogDebug( "main", "(Build 11-09-99)", szDbg );
#endif

  if ( argc < 2 )
  {
    printf("Usage: configtool cm_info [<File Path>] [<File Path>] ...\n");
    printf("       configtool lsco [<Directory>]\n");
    printf("       configtool co [-c <Comment>] [-res[erve] | -unres[erve] ] <File Path> [<File Path>] ...\n");
    printf("       configtool ci [-c <Comment> | -f <CommentFileName>] <File Path> [<File Path>] ...\n");
    printf("       configtool unco <File Path> [<File Path>] ...\n");
	return 0;
  }

  if ( ProcessCommandLine(argc, argv) == 0 )
  {

	if ( g_nCmdIndx == INDX_CMD_CM_INFO || (g_nCmdIndx == INDX_CMD_LSCO && g_ppszFilePath == NULL ) 
        || (g_nCmdIndx == INDX_CMD_EXEC && g_ppszFilePath == NULL ) )
	{
	  if ( CreateCommand(-1) == 0 )
      	  {
  	    if ( !RunShell() )
	      g_nErr = 1;
	    PrintResult();
      	  }
	}

	for ( int i=0; i< g_nActions; i++ )
	{
	  if ( CreateCommand(i) == 0 )
      		{
  	  	if ( !RunShell() )
	  	    g_nErr = 1;
		PrintResult();
      		}
	}
  }

  DestroyAll();
  return g_nErr;
}
