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

#define	_THIS_FILE_		"Configurator_cm"


#undef	SCC_PROC
#define SCC_PROC(val) #val

static LPCSTR gpFuncNames[] = {
    SCC_PROC_NAMES
    };

static LPCSTR	SccFuncName	(int nDex ) { return gpFuncNames	[nDex]; }

//----------------------------------------
CMProvider::CMProvider()
{
  m_pSccContext = NULL;             
  m_ghInstSrvr  = NULL; 						

  for ( int i=0; i< SCC_CMD_COUNT; i++ )
		m_gpFuncs[i]= NULL;

  m_szProviderName[0] = '\0';
  DWORD nLen = SCC_USER_LEN + 1;
  if (!GetUserName( m_szUserName, &nLen))
    m_szUserName[0] = '\0';

  m_SccCaps       = 0;       
  m_lCoCommentLen = 0; 
  m_lCommentLen   = 0;
  m_fIsLoaded     = FALSE;
}

//----------------------------------------
CMProvider::~CMProvider()
{
  if( m_ghInstSrvr )
	  FreeLibrary(m_ghInstSrvr);
}

//----------------------------------------
void CMProvider::SetSccUserName(char* pszUserName)
{
  if ( pszUserName && *pszUserName )
    strcpy( m_szUserName, pszUserName );
}

//----------------------------------------
BOOL CMProvider::LoadServer()
{
  // Set TMP and TEMP environment variables to user defined values (before starting DISCOVER)
  char  szDisTMP  [ _MAX_PATH +1 ]="";
  char* pszTMP;
  char  szSetTMP  [sizeof("TEMP=") + _MAX_PATH +1];

  if ( (pszTMP= getenv("DIS_TMP_BEFORE_DIS")) )
  {
    strcpy( szDisTMP, getenv("TMP") );
    strcpy( szSetTMP, "TEMP=" );
    strcat( szSetTMP, pszTMP );
    putenv( szSetTMP );
    szSetTMP[1] = 'T';
    putenv( szSetTMP+1 );
  }

  char    szLocationRegPath	[_MAX_PATH+1];
  DWORD	  dwSccServerPathLen = _MAX_PATH;
  DWORD   dwType     = 0;
  HKEY    thisKey;
  LONG    lRet;
  BOOL	  fResult = FALSE;

  GetDisRegistryStr	(REG_SCC, REG_SCC_SUBSTITUTION, szLocationRegPath, sizeof(szLocationRegPath), "", HKEY_LOCAL_MACHINE);
	if( szLocationRegPath[0] != '\0' )
  {
    //---------
    // find the SCC dll's location
    // ... open the key
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szLocationRegPath, NULL, KEY_READ, &thisKey);
    if (lRet == ERROR_SUCCESS)
    {
      // ... read the value
      lRet = RegQueryValueEx(thisKey, STR_SCCPROVIDERPATH, NULL,
											       &dwType, (BYTE *) m_szSccServerPath, &dwSccServerPathLen);
      RegCloseKey(thisKey);
      if( lRet == ERROR_SUCCESS && dwType == REG_SZ && dwSccServerPathLen > 1 )
      {
        m_ghInstSrvr = LoadLibrary(m_szSccServerPath);
        if( m_ghInstSrvr )
        {
          // Get the address of the call-backs
          //
          PFNSCC	pSccFunc;
          fResult = TRUE;
          for(int nDex=0;  nDex<SCC_CMD_COUNT;  nDex++)
				  {
            pSccFunc = (PFNSCC)GetProcAddress(m_ghInstSrvr, SccFuncName(nDex) );
            if( pSccFunc )
					    m_gpFuncs[nDex] = pSccFunc;
				    else
              fResult = FALSE; //if any of the entry points wasn't found, function fails
          }
        }
      }  
    }
  }

  // Set TMP and TEMP environment variables to DISCOVER defined values
  if ( szDisTMP[0] )
  {
    strcpy( szSetTMP, "TEMP=" );
    strcat( szSetTMP, szDisTMP );
    putenv( szSetTMP );
    szSetTMP[1] = 'T';
    putenv( szSetTMP+1 );
  }

  return fResult;
}


BOOL	PopulateFunc	(LPVOID, BOOL, LONG, LPCSTR);

//----------------------------------------
BOOL CMProvider::Init()
{
  long  nVer;
  BOOL  fResult = TRUE;
  if ( LoadServer() )
  {
    PFNSCC pf = GetInterface( SCC_CMD_SccGetVersion );
    if( pf )
      nVer = (*pf)();
    pf = GetInterface( SCC_CMD_SccInitialize );
    if( pf )
    {

      char pszAuxPathLabel[ SCC_AUXLABEL_LEN ];
      //Initialize CM provider here
      SCCRTN sRet = (*pf)( &m_pSccContext, 
                           NULL, 
                           CALLER_NAME, 
                           m_szProviderName,
                           &m_SccCaps, 
                           pszAuxPathLabel, 
                           &m_lCoCommentLen,
                           &m_lCoCommentLen );
      if( sRet == SCC_OK )
        m_fIsLoaded = TRUE;
    }

  }
  return fResult;
}


BOOL	PopulateFunc(LPVOID pvCallerData, BOOL fAddRemove, LONG nStatus, LPCSTR lpFileName)
{
  BOOL bRC = TRUE;
  return bRC;
}

//----------------------------------------
void CMProvider::Uninit()
{
  if( IsLoaded() )
  {
    PFNSCC pf;

    pf = m_gpFuncs[ SCC_CMD_SccCloseProject ];
    if( pf )
      (*pf)(m_pSccContext);

    pf = GetInterface( SCC_CMD_SccUninitialize );
    if( pf )
      (*pf)(m_pSccContext);
    
    if ( m_ghInstSrvr )
      FreeLibrary(m_ghInstSrvr);
    m_ghInstSrvr = NULL;

    for(int nDex=0;  nDex<SCC_CMD_COUNT;  nDex++)
			m_gpFuncs[nDex] = NULL;

  }
}

//----------------------------------------
int CMProvider::GetFileStatus(char *pszFilePath )
{
  int     status = SCC_STATUS_INVALID;
  SCCRTN  sRet;
  PFNSCC  pfQuery = m_gpFuncs[ SCC_CMD_SccQueryInfo ];
  char*   lpszFilePath = pszFilePath;
  if (pfQuery)
  {
    char* p = strrchr(pszFilePath, '\\');
    if ( p == NULL )
	    p = strrchr(pszFilePath, '/');
    if ( p )
    {
      *p = 0;
      SetCurrentDirectory( pszFilePath );
      *p = '\\';
    }
    sRet = (*pfQuery)(GetContext(), 1, &lpszFilePath, &status );
    if ( sRet != SCC_OK )
       status = SCC_STATUS_INVALID;
    LogDebug( "GetFileStatus", pszFilePath, (sRet==SCC_OK ? "OK" : "INVALID") );
  }

  return status;
}

//----------------------------------------
static BOOL   fTrapVersion = FALSE;
static char   szVersion[MAX_PATH];
static Configurator* g_pConfigurator = NULL;

void  SetTrapVersion( BOOL f, Configurator* pConfigurator )
{
  fTrapVersion = f;
  szVersion[0] = '\0';
  g_pConfigurator = pConfigurator;
}

//----------------------------------------
char* GetVersionFromCM()
{
  return szVersion;
}

//----------------------------------------
long PrintCallbackFunc( char *pszMsg, DWORD nType )
{
  if ( g_pConfigurator )
  {
    char* psz = new char [ strlen(pszMsg) +32 ];
    if ( psz )
    {
      strcpy( psz, "%%" );
      if ( strcmp(g_pConfigurator->GetSccName(),PVCS_NAME) == 0 )
      {
	if ( strncmp(pszMsg, "Unlocking", strlen("Unlocking")) == 0 )
	{
	  strcat( psz, "Undoing checkout for" );
	  strcat( psz, pszMsg + strlen("Unlocking") );
	}
	else if ( strncmp(pszMsg, "Unlock", strlen("Unlock")) == 0 )
	{
	  strcat( psz, "Undo checkout for" );
	  strcat( psz, pszMsg + strlen("Unlock") );
	}
	else
	  strcat( psz, pszMsg );
      }
      else
	      strcat( psz, pszMsg );
      g_pConfigurator->ConfigShellOutput( psz );
      delete [] psz;
    }
  }
  switch( nType )
  {
  case SCC_MSG_INFO: case SCC_MSG_STATUS:
    if( fTrapVersion ) //get file version from checkout output
    {
      char *pSig = strstr( pszMsg, CLEARCASE_CO_VER_SIGNATURE );
      if( pSig )
			{
        //get rid of last double quote
        char *pQuote = strrchr( pSig, '"' );
        if( pQuote )
          *pQuote = '\0';
        strcpy( szVersion, "@@" );
        strcat( szVersion, pSig + strlen(CLEARCASE_CO_VER_SIGNATURE) );
      }
    }
    break;
  case SCC_MSG_ERROR:case SCC_MSG_WARNING:
    break;
  }

  return SCC_MSG_RTN_OK;
}



//----------------------------------------
char *GetLastSlash( char * pszString)
{
  char *pLastFSlash = strrchr( pszString, '/' );
  char *pLastBSslash = strrchr( pszString, '\\' );
  return max( pLastFSlash, pLastBSslash );
}
