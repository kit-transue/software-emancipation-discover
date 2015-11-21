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
/* This file has a set of utility methods which make accessing the Registry on NT
* easier.
* The important things to note:
* Every computer has an HKEY_LOCAL_MACHINE\Software\Software Emancipation Technology
*  \DISCOVER\CurrentVersion
* key.  This key is used as an index into the user's profile (and if a value isn't found
* there, into the HKEY_LOCAL MACHINE .... registry.
*
* Recommended utility methods:
* GetRegistryStr
* GetRegistryInt
* these will behave properly (hopefully)!
* these accept a fully qualified path to the key BELOW $CURRENTVERSION,
* and a Name, and will return the value associated with that name
* (eg to get the value of the command Name of the HKCU/S/SET/DISCOVER/CV/Admin
* key, one would just use  GetRegistryStr(Admin, command...)
* note that one would also use the SAME command to get the
* subkey of HKLM
*/

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "DisRegistry.h"

#define	_THIS_FILE_		"DisRegistry"

BOOL	bRegistryDebug = FALSE;

/***************************************************************************
Internal
****************************************************************************/
void SetRegistryDebug( BOOL b)
{
	bRegistryDebug = b;
}

void RegistryDebug( char* Txt1, char* Txt2, char* Txt3, char* Txt4, char* Txt5)
{
	if ( bRegistryDebug )
	{
		FILE*  fileLog = fopen( "c:\\Temp\\Registry.log", "a+t");
		if ( fileLog )
		{
			fprintf( fileLog, "%s %s %s %s %s\n", Txt1, Txt2, Txt3, Txt4, Txt5 );
			fclose ( fileLog );
		}
	}
}


LONG GetRegistryInt_Internal(HKEY hKey, char* pszValName, int *pnValRet, BOOL bKeepKeyOpen)
{
	BYTE*		pbData	= new BYTE[ MAX_PATH +1];
	DWORD		nDataLen= MAX_PATH+1;
	LONG		lRet = -1;
	DWORD		dwType = 0;
	
	if ( hKey && pnValRet && pbData && pszValName )
	{
		lRet = RegQueryValueEx(hKey, pszValName, NULL,
			&dwType, (BYTE *)pbData, &nDataLen);
		if (lRet == ERROR_SUCCESS)
		{
			if ( dwType == REG_SZ )
			{
				*(pbData+nDataLen) = '\0';
				*pnValRet = atoi((char*)pbData);
			}
			else if ( dwType == REG_DWORD )
				*pnValRet = (DWORD)(*pbData);
		}
	}
	
	if ( !bKeepKeyOpen )
		RegCloseKey(hKey);
	if ( pbData )
		delete [] pbData;
	return lRet;
}
//---------------------------------------------------------------------------
//
LONG GetRegistryStr_Internal(HKEY hKey, char* pszValName, char* pszStrBuffer, UINT nBufferLen, BOOL bKeepKeyOpen)
{
	DWORD		dwType;
	BYTE*		pbData	= new BYTE[ MAX_PATH +1];
	char		*pszKeyValue = NULL;
	LONG		lRet = -1;
	
	if ( hKey && pszValName && pszStrBuffer && nBufferLen > 0 )
	{
		DWORD nDataLen = MAX_PATH;
		lRet = RegQueryValueEx(hKey, pszValName, NULL,
			&dwType, (BYTE *)pbData, &nDataLen);
		if( lRet == ERROR_SUCCESS )
		{
			if( dwType == REG_SZ )
				pszKeyValue = (char*) pbData;
		}
		
		if ( !pszKeyValue || strlen(pszKeyValue) >= nBufferLen )
		{
			*pszStrBuffer = '\0';
			lRet = -1;
		}
		else
			strcpy( pszStrBuffer, pszKeyValue );
	}
	
	if ( !bKeepKeyOpen )
		RegCloseKey(hKey);
	if ( pbData )	
		delete [] pbData;
	return lRet;
}

/***************************************************************************
Registry
****************************************************************************/
LONG GetDisCurrentVersion(char* pszVersionPath )
{
  return GetDisCurrentVersion( pszVersionPath, NULL );
}

static char gszDebugKeyVal[512] = "";
int	 GetDisRegistryDebug(char* pszKeyName)
{
  gszDebugKeyVal[0] = 0;
  if ( !GetDisRegistryStr( "Debug", pszKeyName, gszDebugKeyVal, sizeof(gszDebugKeyVal), "" ) )
    return 0;
  else if ( gszDebugKeyVal[0] == 0 || (gszDebugKeyVal[0] == '0' && gszDebugKeyVal[1] == 0) )
    return 0;
  else
    return (int)(void*)gszDebugKeyVal;
}

//---------------------------------------------------------------------------
//
LONG GetDisCurrentVersion(char* pszVersionPath, HKEY hRoot)
{
	HKEY		hKey;
	DWORD		dwVersionNameLen = MAX_PATH;
	DWORD		nType;
	HKEY		hDisRoot = (hRoot == NULL ? HKEY_LOCAL_MACHINE : hRoot);
	LONG		lRet = -1;
	char*		pszVersionName = new char[ MAX_PATH +1];
	
	if ( pszVersionName && pszVersionPath )
	{
		strcpy( pszVersionPath, REG_DISCOVER );
		lRet = RegOpenKeyEx( hDisRoot, REG_DISCOVER, NULL, KEY_READ, &hKey );
		if ( lRet != ERROR_SUCCESS && hRoot == NULL )
		{
			hDisRoot = HKEY_CURRENT_USER;
			lRet = RegOpenKeyEx( hDisRoot, REG_DISCOVER, NULL, KEY_READ, &hKey );
		}
		
		if ( lRet == ERROR_SUCCESS )
		{
			lRet = RegQueryValueEx( hKey, REG_DISCOVER_VERSION, NULL, &nType, (BYTE *) pszVersionName, &dwVersionNameLen );
			if ( lRet == ERROR_SUCCESS )
			{
				*(pszVersionName+dwVersionNameLen) = '\0';
				strcat( pszVersionPath, "\\");
				strcat( pszVersionPath, pszVersionName);
				strcat( pszVersionPath, "\\");
			}
			RegCloseKey(hKey);
		}
	}
	
	if ( pszVersionName )
		delete [] pszVersionName;
	return lRet;
}

//---------------------------------------------------------------------------
//
int	GetDisRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue )
{
  return GetDisRegistryInt( pszSection, pszKeyName, nKeyDefaultValue, REG_DISCOVER_ROOT );
}

int	GetDisRegistryInt(char* pszSection, char* pszKeyName, 
                      int nKeyDefaultValue, HKEY hRoot)
{
	HKEY		hKey;
	char*		pszVersionPath = new char[ MAX_PATH +1];
	char*		pszSectionPath = new char[ MAX_PATH +1];
	DWORD		dwVersionNameLen = MAX_PATH;
	int			nRetVal = nKeyDefaultValue;
	HKEY		hDisRoot = (hRoot == NULL ? HKEY_CURRENT_USER : hRoot);
	LONG		lRet = -1;
	
	if ( pszSection && pszKeyName && pszVersionPath && pszSectionPath )
	{
		lRet = GetDisCurrentVersion( pszVersionPath, hRoot);
		if ( lRet == ERROR_SUCCESS )
		{
			strcpy( pszSectionPath, pszVersionPath );
			strcat( pszSectionPath, pszSection );
			
			hDisRoot = (hRoot == NULL ? HKEY_CURRENT_USER : hRoot);
			lRet = RegOpenKeyEx( hDisRoot, pszSectionPath, NULL, KEY_READ, &hKey );
			if ( lRet == ERROR_SUCCESS )
				lRet = GetRegistryInt_Internal(hKey, pszKeyName, &nRetVal);
			
			if ( lRet != ERROR_SUCCESS && hRoot == NULL )
			{
				hDisRoot = HKEY_LOCAL_MACHINE;
				lRet = RegOpenKeyEx( hDisRoot, pszSectionPath, NULL, KEY_READ, &hKey );
				if ( lRet == ERROR_SUCCESS )
					lRet = GetRegistryInt_Internal(hKey, pszKeyName, &nRetVal);
			}
			
			if ( lRet != ERROR_SUCCESS )
			{
				strcpy( pszSectionPath, pszVersionPath );
				*(pszSectionPath + strlen(pszSectionPath) -1 ) = 0;
				char* p = strrchr( pszSectionPath, '\\' );
				*(p+1) = 0;
				strcat( pszSectionPath, pszSection );
				
				hDisRoot = (hRoot == NULL ? HKEY_CURRENT_USER : hRoot);
				lRet = RegOpenKeyEx( hDisRoot, pszSectionPath, NULL, KEY_READ, &hKey );
				if ( lRet == ERROR_SUCCESS )
					lRet = GetRegistryInt_Internal(hKey, pszKeyName, &nRetVal);
				
				if ( lRet != ERROR_SUCCESS && hRoot == NULL )
				{
					hDisRoot = HKEY_LOCAL_MACHINE;
					lRet = RegOpenKeyEx( hDisRoot, pszSectionPath, NULL, KEY_READ, &hKey );
					if ( lRet == ERROR_SUCCESS )
						lRet = GetRegistryInt_Internal(hKey, pszKeyName, &nRetVal);
				}
			}
			
		}	// GetDisCurrentVersion( pszVersionPath, hRoot) == ERROR_SUCCESS
	}	// if ( pszSection && pszKeyName )
	
	if ( pszVersionPath )
		delete [] pszVersionPath;
	if ( pszSectionPath )
		delete [] pszSectionPath;
	
	if ( lRet != ERROR_SUCCESS )
		nRetVal = nKeyDefaultValue;
	return nRetVal;
}

//---------------------------------------------------------------------------
//
int	GetRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue )
{
  return GetRegistryInt( pszSection, pszKeyName, nKeyDefaultValue, NULL );
}

int	GetRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue, HKEY hRoot)
{
	int			nKeyValue = nKeyDefaultValue;
	HKEY		hKey;
	HKEY    hDisRoot = (hRoot == NULL ? HKEY_CURRENT_USER : hRoot);
	LONG		lRet;
	
	lRet = RegOpenKeyEx( hDisRoot, pszSection, NULL, KEY_READ, &hKey);
	if ( lRet != ERROR_SUCCESS && hRoot == NULL )
		lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszSection, NULL, KEY_READ, &hKey);
	
	if (lRet == ERROR_SUCCESS)
		lRet = GetRegistryInt_Internal( hKey, pszKeyName, &nKeyValue );
	
	if ( lRet != ERROR_SUCCESS )
		nKeyValue = nKeyDefaultValue;
	return nKeyValue;
}

//-------------------------------------
//
BOOL	GetDisRegistryStr( char* pszSection, char* pszKeyName, 
						  char* pszStrBuffer, UINT nBufferLen, char* pszKeyDefaultVal )
{
  return GetDisRegistryStr( pszSection, pszKeyName, pszStrBuffer, nBufferLen, pszKeyDefaultVal, REG_DISCOVER_ROOT );
}

BOOL	GetDisRegistryStr( char* pszSection, char* pszKeyName, 
						  char* pszStrBuffer, UINT nBufferLen, char* pszKeyDefaultVal, 
						  HKEY hRoot)
{
	HKEY		hKey;
	char*		pszVersionPath = new char[ MAX_PATH +1];
	char*		pszSectionPath = new char[ MAX_PATH +1];
	DWORD		dwVersionNameLen = MAX_PATH;
	HKEY    hDisRoot = (hRoot == NULL ? HKEY_CURRENT_USER : hRoot);
	LONG		lRet = -1;
	
	if ( pszSection && pszKeyName && pszStrBuffer && nBufferLen> 0 && 
		pszVersionPath && pszSectionPath )
	{
		//copy default value to return value
		if ( pszKeyDefaultVal && nBufferLen > strlen(pszKeyDefaultVal) )
			strcpy( pszStrBuffer, pszKeyDefaultVal );
		else
			*pszStrBuffer = 0;
		
		RegistryDebug( "GetDisRegistryStr:", pszSection, pszKeyName, "=(?)" , pszStrBuffer );
		
		lRet = GetDisCurrentVersion( pszVersionPath, hRoot);
		if ( lRet == ERROR_SUCCESS )
		{
			strcpy( pszSectionPath, pszVersionPath );
			strcat( pszSectionPath, pszSection );
			RegistryDebug( "GetDisRegistryStr:", "pszSectionPath=", pszSectionPath, " " , " " );
			
			hDisRoot = (hRoot == NULL ? HKEY_CURRENT_USER : hRoot);
			lRet = RegOpenKeyEx( hDisRoot, pszSectionPath, NULL, KEY_READ, &hKey );
			if ( lRet == ERROR_SUCCESS )
				lRet = GetRegistryStr_Internal(hKey, pszKeyName, pszStrBuffer, nBufferLen);
			
			if ( lRet != ERROR_SUCCESS && hRoot == NULL )
			{
				hDisRoot = HKEY_LOCAL_MACHINE;
				lRet = RegOpenKeyEx( hDisRoot, pszSectionPath, NULL, KEY_READ, &hKey );
				if ( lRet == ERROR_SUCCESS )
					lRet = GetRegistryStr_Internal(hKey, pszKeyName, pszStrBuffer, nBufferLen);
			}
			
			if ( lRet != ERROR_SUCCESS )
			{
				strcpy( pszSectionPath, pszVersionPath );
				*(pszSectionPath + strlen(pszSectionPath) -1 ) = 0;
				char* p = strrchr( pszSectionPath, '\\' );
				*(p+1) = 0;
				strcat( pszSectionPath, pszSection );
				
				RegistryDebug( "GetDisRegistryStr:", "pszSectionPath=", pszSectionPath, " " , " " );
				
				hDisRoot = (hRoot == NULL ? HKEY_CURRENT_USER : hRoot);
				lRet = RegOpenKeyEx( hDisRoot, pszSectionPath, NULL, KEY_READ, &hKey );
				if ( lRet == ERROR_SUCCESS )
					lRet = GetRegistryStr_Internal(hKey, pszKeyName, pszStrBuffer, nBufferLen);
				
				if ( lRet != ERROR_SUCCESS && hRoot == NULL )
				{
					hDisRoot = HKEY_LOCAL_MACHINE;
					lRet = RegOpenKeyEx( hDisRoot, pszSectionPath, NULL, KEY_READ, &hKey );
					if ( lRet == ERROR_SUCCESS )
						lRet = GetRegistryStr_Internal(hKey, pszKeyName, pszStrBuffer, nBufferLen);
				}
			}
			
		}	// GetDisCurrentVersion( pszVersionPath, hRoot) == ERROR_SUCCESS
	}	// if ( pszSection && pszKeyName )
	
	if ( pszVersionPath )
		delete [] pszVersionPath;
	if ( pszSectionPath )
		delete [] pszSectionPath;
	
	if ( lRet != ERROR_SUCCESS )
		strcpy( pszStrBuffer, pszKeyDefaultVal );
	
	RegistryDebug( "GetDisRegistryStr:", pszSection, pszKeyName, "=" , pszStrBuffer );
	return (lRet == ERROR_SUCCESS);
}

//-------------------------------------
//
BOOL	GetRegistryStr(char* pszSection, char* pszKeyName, 
					   char* pszStrBuffer, UINT nBufferLen, char* pszKeyDefaultVal )
{
  return GetRegistryStr( pszSection, pszKeyName, pszStrBuffer, nBufferLen, pszKeyDefaultVal, NULL );
}

BOOL	GetRegistryStr(char* pszSection, char* pszKeyName, 
					   char* pszStrBuffer, UINT nBufferLen, 
					   char* pszKeyDefaultVal, HKEY hRoot)
{
	char		*pszKeyValue = pszKeyDefaultVal;
	HKEY		hKey;
	HKEY    hDisRoot = (hRoot == NULL ? HKEY_CURRENT_USER : hRoot);
	LONG		lRet = -1;
	
	lRet = RegOpenKeyEx( hDisRoot, pszSection,	NULL, KEY_READ, &hKey );
	if ( lRet != ERROR_SUCCESS && hRoot == NULL )
		lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszSection,	NULL, KEY_READ, &hKey );
	
	if ( lRet == ERROR_SUCCESS )
		lRet= GetRegistryStr_Internal(hKey, pszKeyName, pszStrBuffer, nBufferLen);
	
	if ( lRet != ERROR_SUCCESS )
		strcpy( pszStrBuffer, pszKeyDefaultVal );
	return (lRet == ERROR_SUCCESS);
}


/****************************************************************************
DEBUG
****************************************************************************/

#ifdef DISMESSAGE_COMMENT_LEN
#undef DISMESSAGE_COMMENT_LEN
#endif
#ifdef DISMESSAGE_LOGTEXT_LEN
#undef DISMESSAGE_LOGTEXT_LEN
#endif

#define	DISMESSAGE_COMMENT_LEN	1024
#define	DISMESSAGE_LOGTEXT_LEN	2048

void ServerDebugMessageLog( UINT nErrLevel, char* pszWho, char* pszAction, 
						   char* pszComment1, char* pszComment2 )
{
	if ( pszWho == NULL )
		return;
	
	int nLOG_DBG = GetDisRegistryInt(REG_LOG_JOURNAL, REG_INTEGRATOR_LOG_DBG, 1954);
	if ( (int)nErrLevel != nLOG_DBG && (int)nErrLevel > GetDisRegistryInt(REG_LOG_JOURNAL, REG_INTEGRATOR_LOG_MSG, 255)	)
		return;
	
	char	szERR[4] = "   ";
	if ( (int)nErrLevel < GetDisRegistryInt(REG_LOG_JOURNAL, REG_INTEGRATOR_LOG_WNG, 128) )
		strcpy( szERR, "WNG" );
	if ( (int)nErrLevel < GetDisRegistryInt(REG_LOG_JOURNAL, REG_INTEGRATOR_LOG_ERR, 128) )
		strcpy( szERR, "ERR" );
	if ( (int)nErrLevel == nLOG_DBG )
		strcpy( szERR, "DBG" );
	
	char* pszComment      = NULL;
	char* pszLogText      = NULL;
	char*	pszLogFileName  = NULL;
	char* pszTimeBuff     = NULL;
	char* pszDateBuff     = NULL;
	
	pszComment      = new char [DISMESSAGE_COMMENT_LEN];
	pszLogText      = new char [DISMESSAGE_LOGTEXT_LEN];
	pszLogFileName  = new char [MAX_PATH+1];
	pszTimeBuff     = new char [64];
	pszDateBuff     = new char [64];
	if ( pszComment == NULL || pszLogText == NULL || pszLogFileName == NULL || pszTimeBuff == NULL || pszDateBuff == NULL)
	{
		if ( pszComment )
			delete [] pszComment;
		if ( pszLogText )
			delete [] pszLogText;
		if ( pszLogFileName )
			delete [] pszLogFileName;
		if ( pszTimeBuff )
			delete [] pszTimeBuff;
		if ( pszDateBuff )
			delete [] pszDateBuff;
		return;
	}
	
	*pszComment = 0;
	if ( pszComment1 )
		strncat( pszComment, pszComment1, DISMESSAGE_COMMENT_LEN-3 );
	strcat (pszComment, " ");
	if ( pszComment2 )
		strncat( pszComment, pszComment2, DISMESSAGE_COMMENT_LEN- strlen(pszComment) -2 );
	
	sprintf( pszLogText, "%s:%16s :%40s : %s", szERR, pszWho, pszAction, pszComment );
	
	FILE*	fileLog = NULL;
	
	GetDisRegistryStr(REG_LOG_JOURNAL, REG_INTEGRATOR_LOG, pszLogFileName, _MAX_PATH, "");
	if ( pszLogFileName && strlen( pszLogFileName ) )
		fileLog = fopen( pszLogFileName, "a+t");
	
	if ( fileLog )
	{
		if ( MESSAGE_TIME )
		{
			_strtime( pszTimeBuff );
			_strdate( pszDateBuff );
			*(pszDateBuff+5) = '\0';
			fprintf( fileLog, "%s %s %s\n", pszDateBuff, pszTimeBuff, pszLogText );
		}
		else
		{
			fprintf( fileLog, "%s\n", pszLogText );
		}
		fclose( fileLog );
	}
#ifdef _ERR_MESSAGEBOX
	if ( (int)nErrLevel < GetDisRegistryInt(REG_LOG_JOURNAL, REG_INTEGRATOR_LOG_ERR, 5)   )
	{
		char szText[256];
		sprintf( szText, "%s: %s", pszAction, pszComment);
		MessageBox( NULL, szText, pszWho, MB_OK );
	}
#endif

	delete [] pszComment;
	delete [] pszLogText;
	delete [] pszLogFileName;
	delete [] pszTimeBuff;
	delete [] pszDateBuff;
	
}

void ServerDebugMessageLogInt( UINT nErrLevel, char* pszWho, char* pszAction, 
							  char* pszComment1, int nInt )
{
	char  sz[64];
	_itoa( nInt, sz, 10);
	ServerDebugMessageLog( nErrLevel, pszWho, pszAction, pszComment1, sz);
}

void ServerDebugMessageLogIntX( UINT nErrLevel, char* pszWho, char* pszAction, 
							   char* pszComment1, int nInt )
{
	char  sz[64];
	_itoa( nInt, sz, 16);
	ServerDebugMessageLog( nErrLevel, pszWho, pszAction, pszComment1, sz);
}



