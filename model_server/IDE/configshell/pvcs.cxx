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

#define	_THIS_FILE_		"Configurator_PVCS"

#define PVCS_INI_FILE_NAME        "ISLV.ini"
#define PVCS_INI_GUI_SECT         "PVCSGUI"
#define PVCS_INI_PROJ_DIR         "PVCSPROJ"
#define PVCS_PROJ_FILE            "pvcsproj.pub"
#define PVCS_FOLD_FILE            "pvcsfold.pub"
#define PVCS_PROJ_SECTION         "Project"
#define PVCS_DOC_SECTION          "DOCUMENT"
#define PVCS_FOLDER_SECTION       "FOLDER"
#define PVCS_CFGDIR_ITEM          "DIR"
#define PVCS_WRKDIR_ITEM          "WKDIR"
#define PVCS_ARCDIR_ITEM          "ARDIR"

typedef struct tagDIR_LIST 
            { 
              char          szDir[_MAX_PATH]; 
              tagDIR_LIST*  pNext;
            } DIR_LIST;

/***********************************************************/
/***      Implemantation of Configurator_PVCS            ***/
/***********************************************************/

Configurator_PVCS::Configurator_PVCS(CMProvider* pCMProvider) 
: Configurator_MSAPI(pCMProvider)
{
  char  szSection      [_MAX_PATH];
  char  szPVCSFileINI  [_MAX_PATH];
  FILE* filePVCSFileINI;

  char szTool[ _MAX_PATH ];
  strcpy( szTool, m_pCMProvider->GetSccServerPath() );
  char *pLastFSlash = GetLastSlash( szTool );
  if ( pLastFSlash )
  {
    *(pLastFSlash+1) = 0;
    SetEnvironmentVariable("cm_tool_dir", szTool );
  }

  m_szPVCSMainPUB[0] = '\0';
  if ( GetWindowsDirectory( (char*)szPVCSFileINI, sizeof(szPVCSFileINI)-1 ) )
  {
    strcat( szPVCSFileINI, "\\" );
    strcat( szPVCSFileINI, PVCS_INI_FILE_NAME );
    if ( (filePVCSFileINI = fopen( szPVCSFileINI, "r+t" )) )
    {
      GetIniNextSection( filePVCSFileINI, PVCS_INI_GUI_SECT, szSection, sizeof(szSection) );
      GetIniNextKeyString( filePVCSFileINI, PVCS_INI_PROJ_DIR, 
                           m_szAuxProjPath, sizeof(m_szAuxProjPath)-1);
      fclose( filePVCSFileINI );

      strcpy( m_szPVCSMainPUB, m_szAuxProjPath );
      strcat( m_szPVCSMainPUB, "\\" );
      strcat( m_szPVCSMainPUB,PVCS_PROJ_FILE );
      if ( _access( m_szPVCSMainPUB, 4) )
        m_szPVCSMainPUB[0] = '\0';

  
      LogDebug( "::Configurator_PVCS", szPVCSFileINI, m_szAuxProjPath );
  
    } // if ( (filePVCSFileINI = fopen( szPVCSFileINI, "r+t" )) )
  } // if ( GetWindowsDirectory( (char*)szPVCSFileINI, sizeof(szPVCSFileINI)-1 ) )

  SetTrapVersion(FALSE);
}

//--------
BOOL Configurator_PVCS::FindProjAndLocalDir(char *cm_workfile, char *cm_revfile)
{ 
  if ( Configurator_MSAPI::FindProjAndLocalDir(cm_workfile, cm_revfile) )
	return TRUE;

  BOOL bRet = FALSE;
  BOOL  fNotFound = TRUE;
  
  char  szPVCSProjPUB  [_MAX_PATH];
  char  szFileName     [_MAX_PATH];
  char  szCfgDir       [_MAX_PATH];
  char  szLocalDir     [_MAX_PATH];
  char  szProject      [_MAX_PATH];
  
  char  szWrkDir       [_MAX_PATH];
  char  szWrkFile      [_MAX_PATH];
  char  szArcDir       [_MAX_PATH];
  char  szArcFile      [_MAX_PATH];
  
  FILE* filePVCSMainPUB;
  FILE* filePVCSProjPUB;
  
  char  szDocument     [3*_MAX_PATH];
  
  if ( m_szPVCSMainPUB[0] == 0  || (filePVCSMainPUB = fopen( m_szPVCSMainPUB, "r+t" )) == NULL )
  {
    fprintf( stderr, "\nPVCS is probably crashed\n" );
    fflush( stderr );
  }
  
  else if ( cm_workfile && *cm_workfile )
  {
    _fullpath( szFileName, cm_workfile, sizeof(szFileName)-1 );
    LogDebug( "::Configurator_PVCS", "cm_workfile", cm_workfile );
    while ( fNotFound && 
      GetIniNextSection( filePVCSMainPUB, PVCS_PROJ_SECTION, szProject, sizeof(szProject) ) )
    {
      long lOffsetPVCSMainPUB = ftell( filePVCSMainPUB );
      
      m_szArcDir[0] = 0;
      GetIniNextKeyString( filePVCSMainPUB, PVCS_ARCDIR_ITEM, m_szArcDir, sizeof(m_szArcDir)-1);
      fseek(filePVCSMainPUB, lOffsetPVCSMainPUB, SEEK_SET);
      
      szLocalDir[0] = 0;
      GetIniNextKeyString( filePVCSMainPUB, PVCS_WRKDIR_ITEM, szLocalDir, sizeof(szLocalDir)-1);
      fseek(filePVCSMainPUB, lOffsetPVCSMainPUB, SEEK_SET);
      
      szCfgDir[0] = 0;
      GetIniNextKeyString( filePVCSMainPUB, PVCS_CFGDIR_ITEM , szPVCSProjPUB, sizeof(szCfgDir)-1);
      if ( *szPVCSProjPUB )
      {
        if ( IsAbsPath(szPVCSProjPUB) )
          strcpy( szCfgDir, szPVCSProjPUB );
        else
        {
          strcpy( szCfgDir, m_szAuxProjPath );
          strcat( szCfgDir, "\\" );
          strcat( szCfgDir, szPVCSProjPUB);
        }
        strcpy( szPVCSProjPUB, szCfgDir );
        strcat( szPVCSProjPUB, "\\" );
        strcat( szPVCSProjPUB, PVCS_FOLD_FILE );
	
        LogDebug( "::Configurator_PVCS", "szPVCSProjPUB", szPVCSProjPUB );
        if ( (filePVCSProjPUB = fopen( szPVCSProjPUB, "r+t" )) ) 
	{
          LogDebug( "::Configurator_PVCS", "Looking for szPVCSProjPUB", szPVCSProjPUB );
	  DIR_LIST *pDirList = NULL, *pDir;
	  while ( GetIniNextSection( filePVCSProjPUB, PVCS_FOLDER_SECTION, szDocument, sizeof(szDocument) ) )
	  {
	    if ( GetIniNextKeyString(filePVCSProjPUB, PVCS_WRKDIR_ITEM, szDocument, sizeof(szDocument) ) )
	    {
	      if ( pDirList )
	      {
		pDir->pNext = new DIR_LIST;
		pDir = pDir->pNext;
	      }
	      else
	      {
		pDirList = new DIR_LIST;
		pDir = pDirList;
	      }
	      
	      pDir->pNext = NULL;
	      strcpy(pDir->szDir, szDocument);
	      strcat(pDir->szDir, "\\" );
	    }
	  }
	  
	  fseek(filePVCSProjPUB, 0, SEEK_SET);
	  while ( fNotFound &&
	    GetIniNextSection( filePVCSProjPUB, PVCS_DOC_SECTION, szDocument, sizeof(szDocument) ) )
	  {
            LogDebug( "::Configurator_PVCS", "szDocument=", szDocument );
	    ParseDocumentLine( szDocument, szLocalDir, szArcDir, szArcFile, szWrkDir, szWrkFile );
	    
	    if ( CmpFilePath( szWrkDir, szWrkFile, szFileName) == 0)
	      fNotFound = FALSE;
	    else
	      for( pDir=pDirList; pDir; pDir= pDir->pNext ) {
		if ( CmpFilePath( pDir->szDir, szWrkFile, szFileName) == 0)
		{
		  strcpy(szLocalDir, pDir->szDir );
		  szLocalDir[ strlen(szLocalDir) - 1] = 0; 
		  fNotFound = FALSE;
		  break;
		}
	      }
	      
              if ( !fNotFound )
              {
	        LogDebug( "::Configurator_PVCS", "szArcDir", szArcDir );
	        LogDebug( "::Configurator_PVCS", "szArcFile", szArcFile );
		strcpy( m_szArcFile, szArcDir );
		strcat( m_szArcFile, szArcFile );
		_fullpath( szArcFile, m_szArcFile, sizeof(m_szArcFile)-1 );
		SetEnv(CM_REVISION_FILE, szArcFile );
              }
	      
	  } // while ( fNotFound && GetIniNextSection( filePVCSProjPUB, PVCS_DOC_SECTION, szDocument, sizeof(szDocument) ) )
	  
	  while ( pDirList )
	  {
	    pDir = pDirList;
	    pDirList = pDir->pNext;
	    delete pDir;
	  }
	  
	  fclose(filePVCSProjPUB);
        }
      } // if ( (filePVCSProjPUB = fopen( szPVCSProjPUB, "r+t" )) ) 
    } // while ( GetIniNextSection( filePVCSMainPUB, PVCS_PROJ_SECTION, szProject, sizeof(szProject) ) )
    
    //================================= Abs File path not found. Looking for unic file name
    if ( fNotFound )
    {
      LogDebug( "::Configurator_PVCS", "Abs File path not found. Looking for unic file name", "" );
      char  szUnicLocalDir [_MAX_PATH];
      char  szWrkLocalDir  [_MAX_PATH];
      char  szUnicProject  [_MAX_PATH];
      int		nCounts =0;
      
      fseek(filePVCSMainPUB, 0, SEEK_SET);
      while ( nCounts <= 1 &&
	GetIniNextSection( filePVCSMainPUB, PVCS_PROJ_SECTION, szProject, sizeof(szProject) ) )
      {
	long lOffsetPVCSMainPUB = ftell( filePVCSMainPUB );
	
	m_szArcDir[0] = 0;
	GetIniNextKeyString( filePVCSMainPUB, PVCS_ARCDIR_ITEM, m_szArcDir, sizeof(m_szArcDir)-1);
	fseek(filePVCSMainPUB, lOffsetPVCSMainPUB, SEEK_SET);
	
	szLocalDir[0] = 0;
	GetIniNextKeyString( filePVCSMainPUB, PVCS_WRKDIR_ITEM, szLocalDir, sizeof(szLocalDir)-1);
	fseek(filePVCSMainPUB, lOffsetPVCSMainPUB, SEEK_SET);
	
	szCfgDir[0] = 0;
	GetIniNextKeyString( filePVCSMainPUB, PVCS_CFGDIR_ITEM , szPVCSProjPUB, sizeof(szCfgDir)-1);
	if ( *szPVCSProjPUB && *szLocalDir == '.' )
	{
	  if ( IsAbsPath(szPVCSProjPUB) )
	    strcpy( szCfgDir, szPVCSProjPUB );
	  else
	  {
	    strcpy( szCfgDir, m_szAuxProjPath );
	    strcat( szCfgDir, "\\" );
	    strcat( szCfgDir, szPVCSProjPUB);
	  }
	  strcpy( szPVCSProjPUB, szCfgDir );
	  strcat( szPVCSProjPUB, "\\" );
	  strcat( szPVCSProjPUB, PVCS_FOLD_FILE );
	  
	  if ( (filePVCSProjPUB = fopen( szPVCSProjPUB, "r+t" )) ) 
	  {
            LogDebug( "::Configurator_PVCS", "Looking for szPVCSProjPUB", szPVCSProjPUB );
	    DIR_LIST *pDirList = NULL, *pDir;
	    while ( GetIniNextSection( filePVCSProjPUB, PVCS_FOLDER_SECTION, szDocument, sizeof(szDocument) ) )
	    {
	      if ( GetIniNextKeyString(filePVCSProjPUB, PVCS_WRKDIR_ITEM, szDocument, sizeof(szDocument) ) )
	      {
		if ( pDirList )
		{
		  pDir->pNext = new DIR_LIST;
		  pDir = pDir->pNext;
		}
		else
		{
		  pDirList = new DIR_LIST;
		  pDir = pDirList;
		}
		
		pDir->pNext = NULL;
		strcpy(pDir->szDir, szDocument);
		strcat(pDir->szDir, "\\" );
	      }
	    }
	    
	    fseek(filePVCSProjPUB, 0, SEEK_SET);
	    while ( nCounts <= 1 &&
	      GetIniNextSection( filePVCSProjPUB, PVCS_DOC_SECTION, szDocument, sizeof(szDocument) ) )
	    {
              LogDebug( "::Configurator_PVCS", "szDocument =", szDocument );
	      BOOL fInstanceFound = FALSE;
	      ParseDocumentLine( szDocument, szLocalDir, szArcDir, szArcFile, szWrkDir, szWrkFile );
//              LogDebug( "::Configurator_PVCS", "  szWrkDir =", szWrkDir );
	      if ( IsAbsPath(szWrkDir) )
		continue;
              
//              LogDebug( "::Configurator_PVCS", "  szWrkFile=", szWrkFile );
	      if ( CmpFileUnicName( szWrkLocalDir, szWrkDir, szWrkFile, szFileName)== 0 
		&& (++nCounts) == 1 )
		fInstanceFound = TRUE;
	      else
		for( pDir=pDirList; pDir && nCounts <= 1; pDir= pDir->pNext ) 
		{
		  if ( CmpFileUnicName( szWrkLocalDir, pDir->szDir, szWrkFile, szFileName)== 0 
		    && (++nCounts) == 1)
		  {
		    fInstanceFound = TRUE;
		  }
		}
		
		if ( fInstanceFound && nCounts == 1 )
		{
                  LogDebug( "::Configurator_PVCS", "InstanceFound", szArcFile );
		  strcpy( szUnicLocalDir, szWrkLocalDir );
		  strcpy( m_szArcFile, szArcDir );
		  strcpy( m_szArcFile, szArcDir );
		  strcat( m_szArcFile, szArcFile );
		  LogDebug( "::Configurator_PVCS", "szArcDir", szArcDir );
		  LogDebug( "::Configurator_PVCS", "szArcFile", szArcFile );
		  LogDebug( "::Configurator_PVCS", "szUnicLocalDir", szUnicLocalDir );
		  _fullpath( szArcFile, m_szArcFile, sizeof(m_szArcFile)-1 );
		  SetEnv(CM_REVISION_FILE, szArcFile );
		  strcpy( szUnicProject, szProject );
		}
		
	    } // while ( fNotFound && GetIniNextSection( filePVCSProjPUB, PVCS_DOC_SECTION, szDocument, sizeof(szDocument) ) )
	    
	    while ( pDirList )
	    {
	      pDir = pDirList;
	      pDirList = pDir->pNext;
	      delete pDir;
	    }
	    
	    fclose(filePVCSProjPUB);
	  }
	} // if ( (filePVCSProjPUB = fopen( szPVCSProjPUB, "r+t" )) ) 
      } // while ( GetIniNextSection( filePVCSMainPUB, PVCS_PROJ_SECTION, szProject, sizeof(szProject) ) )
      
      if ( nCounts == 1 )
      {
		fNotFound = FALSE;
		strcpy( szLocalDir, szUnicLocalDir );
		strcpy( szProject, szUnicProject );
        LogDebug( "::Configurator_PVCS", "szUnicLocalDir", szUnicLocalDir );
        LogDebug( "::Configurator_PVCS", "szUnicProject", szUnicProject );
      }
      else
      {
		SetEnv(CM_REVISION_FILE, "" );
        LogDebug( "::Configurator_PVCS", "Unic File path not found.", "" );
      }
    }
    //=================================================================
    
    fclose( filePVCSMainPUB );
    
    if ( !fNotFound )
    {
      _fullpath( szWrkDir, szLocalDir, sizeof(szWrkDir)-1 );
	  SetEnv( CM_PROJECT,  szProject );
	  SetEnv( CM_LOCALDIR, szWrkDir );
	  return TRUE;
    }
  }
  
  return FALSE;
}

/***********************************************************/
/***      Internal function                              ***/
/***********************************************************/
int Configurator_PVCS::CmpFilePath( char* szWrkDir, char* szWrkFile, char* szFileName)
{
  char  szWrkFilePath  [_MAX_PATH];

  strcpy( szWrkFilePath, szWrkDir );
  strcat( szWrkFilePath, szWrkFile);
  _fullpath( m_szWrkFile, szWrkFilePath, sizeof(m_szWrkFile)-1 );
  return stricmp( m_szWrkFile, szFileName );
}

//--------
int Configurator_PVCS::CmpFileUnicName( char* szWrkLocalDir, char* szWrkDir, char* szWrkFile, char* szFileName)
{
  char  szWrkFilePath  [_MAX_PATH];
  char* pszWrkFilePath = szWrkFilePath;
  int	nWrkPathLen, nFileNameLen = strlen(szFileName);
  int   nRet =1;

  strcpy( pszWrkFilePath, szWrkDir );
  strcat( pszWrkFilePath, szWrkFile);
  if ( *pszWrkFilePath == '.' )
      pszWrkFilePath++;
  if ( *(pszWrkFilePath+1) == '.' )
      pszWrkFilePath += 2;
  nWrkPathLen = strlen( pszWrkFilePath );

  if ( nFileNameLen > nWrkPathLen )
    nRet = stricmp( pszWrkFilePath, szFileName + nFileNameLen - nWrkPathLen );
  if ( nRet == 0 )
  {
    strcpy( szWrkLocalDir, szFileName );
    *(szWrkLocalDir + nFileNameLen - nWrkPathLen) =0;
    if ( *(szWrkLocalDir + strlen(szWrkLocalDir) -1) == '\\' )
	  *(szWrkLocalDir + strlen(szWrkLocalDir) -1) = 0;
  }

  return nRet;
}

//--------
void Configurator_PVCS::ParseDocumentLine( char* szDocument, char* szLocalDir, 
                                           char* szArcDir, char* szArcFile, char* szWrkDir, char* szWrkFile )
{
  char  *p, *pszDocument;
  pszDocument= szDocument;
  if ( (p=strchr( pszDocument, ';' )) )
  {
    *p++ = 0;
    if ( *pszDocument )
    {
      if ( *m_szArcDir == 0 || IsAbsPath(pszDocument) )
        strcpy( szArcDir, pszDocument );
      else
      {
        strcpy( szArcDir, m_szArcDir );
        strcat( szArcDir, "\\" );
        strcat( szArcDir, pszDocument);
      }
      strcat( szArcDir, "\\" );
    }
    pszDocument = p;
  }

  p= strchr( pszDocument, ';' );
  *p++ = 0;
  strcpy( szArcFile, pszDocument );
  pszDocument = p;
  
  if ( (p=strchr( pszDocument, ';' )) )
  {
    *p++ = 0;
    if ( *pszDocument )
    {
      if ( *szLocalDir == 0 || IsAbsPath(pszDocument) )
        strcpy( szWrkDir, pszDocument );
      else
      {
        strcpy( szWrkDir, szLocalDir );
        strcat( szWrkDir, "\\" );
        strcat( szWrkDir, pszDocument);
      }
    }
    strcat( szWrkDir, "\\" );
    pszDocument = p;
  }
  strcpy( szWrkFile, pszDocument );
}


//--------
char* Configurator_PVCS::GetIniLine(FILE* pFile, char* pszBuffer, int nBufferLen )
{
  if ( fgets(pszBuffer, nBufferLen, pFile) )
  {
    char *p;

    if ( *pszBuffer && (*(p= pszBuffer + strlen(pszBuffer)-1 ) == 0x0d  || *p == 0x0a) )
      *p = 0;
    if ( *pszBuffer && (*(p= pszBuffer + strlen(pszBuffer)-1 ) == 0x0d  || *p == 0x0a) )
      *p = 0;

    if ( *pszBuffer == 0 )
      return NULL;

    p= pszBuffer + strlen(pszBuffer);
    while ( *(--p) == ' ' );
    *(++p) = 0;

    p= pszBuffer;
    while ( *(p++) == ' ' );
    --p;

    return p;
  }
  else
    return NULL;
}


//--------
char* Configurator_PVCS::GetIniNextSection( FILE* pFile, char* pszSect, char* pszResult, int nResultLen )
{
  *pszResult = '\0';
  if (!( pFile && pszSect && *pszSect && pszResult && nResultLen > 0 ))
    return NULL;

  char  szBuffer[1024];
  char  szSect[256] = "[";
  char  *p, *q;

  strcat( szSect, pszSect );
  int   nSectLen = strlen(szSect);

  while ( !feof(pFile) )
  {
    if ( (p=GetIniLine(pFile, szBuffer, sizeof(szBuffer)-1 )) && *p )
    {
      if ( strnicmp(p, szSect, nSectLen) == 0 && (q=strchr(p,']')) && 
           ( *(p+=nSectLen) == '=' || *p == ']' ) )
      {
        *q = 0;
        if ( *p )
          p++;
        if ( *p )
          strcpy( pszResult, p);
        return pszResult;
      }
    }
  }
  return NULL;
}


//--------
char* Configurator_PVCS::GetIniNextKeyString(FILE* pFile, char* pszKey, char* pszResult, int nResultLen )
{
  *pszResult = '\0';
  if (!( pFile && pszKey && *pszKey && pszResult && nResultLen > 0 ))
    return NULL;

  char  szBuffer[1024];
  char  *p;
  int   nKeyLen = strlen(pszKey);

  while ( !feof(pFile) )
  {
    if ( (p=GetIniLine(pFile, szBuffer, sizeof(szBuffer)-1 )) && *p )
    {
      if ( *p=='[' )
      {
        return NULL;
      }
      else
      {
        if ( strnicmp(p, pszKey, nKeyLen) == 0 && (*(p+=nKeyLen) == ' ' || *p == '=') ) 
        {
          while ( *(p++) == ' ' );
          --p;
          if ( *p++ == '=' )
          {
            while ( *(p++) == ' ' );
            --p;
            if ( *p )
            {
              strncpy( pszResult, p, nResultLen ); 
              break;
            }
          } // if ( *p++ == '=' )
        } // if ( stricmp(p, pszKey) == 0
      }
    } // if ( (p=GetIniLine(pFile, szBuffer, sizeof(szBuffer)-1 )) && *p )
  }
  if ( *pszResult )
    return pszResult;
  else
    return NULL;
}


//--------
BOOL Configurator_PVCS::IsAbsPath (char* pszPath)
{
  if ( *pszPath && 
        (*pszPath == '/' || *pszPath == '\\' || ( isalpha(*pszPath) && *(pszPath+1) == ':'))
     )
    return TRUE;
  else 
    return FALSE;
}

//-----------------------------------

#define PVCS_STATKEY	"Last trunk rev:"

int Configurator_PVCS::cm_stat()
{
  int nResult = Configurator::cm_stat();

  if ( !m_fScriptInvoked )
  {
    nResult = 0;
  }
	else
	{
		ConfigShellResult* pConfigShellResult = ConfigShellFindOutput( PVCS_STATKEY );
		if ( pConfigShellResult )
		{
			char*	pszRev = strstr(pConfigShellResult->GetText(), PVCS_STATKEY) + strlen(PVCS_STATKEY);
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

