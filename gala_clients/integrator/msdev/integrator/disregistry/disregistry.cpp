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

#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "DisRegistry.h"


/***************************************************************************
		Registry
****************************************************************************/

int	GetDisRegistryInt(char* pszKeyName, int nKeyDefaultValue)
{
  return GetRegistryInt(REG_DISCOVER, pszKeyName, nKeyDefaultValue);
}

int	GetRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue)
{
	int			nKeyValue = nKeyDefaultValue;
	HKEY		hKey;
	DWORD		nIndex = 0;
	char		pszName[ MAX_PATH ];
	DWORD		nType;
	BYTE		pbData[ MAX_PATH ];
	if ( RegOpenKeyEx( HKEY_CURRENT_USER, pszSection, 
									NULL, KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		DWORD nNameSize = sizeof( pszName );
		DWORD nDataSize = sizeof( pbData );
		while ( RegEnumValue( hKey, nIndex, pszName, &nNameSize, 
								NULL, &nType, pbData, &nDataSize) == ERROR_SUCCESS )
		{
			nIndex++;
			if( !stricmp( pszName, pszKeyName) )
			{
				if ( nType == REG_SZ )
				{
					*(pbData+nDataSize) = '\0';
          nKeyValue = atoi((char*)pbData);
				}
        else if ( nType == REG_DWORD )
          nKeyValue = (DWORD)(*pbData);
				break;
			}
			nDataSize = sizeof( pszName ); //restore variable values before next iteration
			nNameSize = sizeof( pbData );
		}
	}
	return nKeyValue;
}

//-------------------------------------------
void	SetDisRegistryInt(char* pszKeyName, int nKeyValue)
{
  SetRegistryInt(REG_DISCOVER, pszKeyName, nKeyValue);
}

void	SetRegistryInt(char* pszSection, char* pszKeyName, int nKeyValue)
{
	HKEY		hKey;
	DWORD		nIndex = 0;
	char		pszName[ MAX_PATH ];
	DWORD		nType;
	BYTE		pbData[ MAX_PATH ];
  DWORD   dwValue;
  BOOL    bFind = FALSE;

	if ( RegOpenKeyEx( HKEY_CURRENT_USER, pszSection, 
									NULL, KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		DWORD nNameSize = sizeof( pszName );
		DWORD nDataSize = sizeof( pbData );
		while ( RegEnumValue( hKey, nIndex, pszName, &nNameSize, 
								NULL, &nType, pbData, &nDataSize) == ERROR_SUCCESS )
		{
			nIndex++;
			if( !stricmp( pszName, pszKeyName) )
			{
				if ( nType == REG_SZ )
        {
          bFind = TRUE;
          char szBuffer[16];
          itoa( (int)nKeyValue, szBuffer, 10);
          RegSetValueEx( hKey, pszKeyName, (DWORD)0, REG_SZ,
                                (UCHAR*)szBuffer, strlen(szBuffer)+1);
        }
        else if ( nType == REG_DWORD )
        {
          bFind = TRUE;
          dwValue = nKeyValue;
          RegSetValueEx( hKey, pszKeyName, (DWORD)0, REG_DWORD,
                         (UCHAR*)dwValue, sizeof(dwValue) );
        }
				break;
			}
			nDataSize = sizeof( pszName ); //restore variable values before next iteration
			nNameSize = sizeof( pbData );
		}
	}
	
  if ( !bFind )
    dwValue = nKeyValue;
    RegSetValueEx( hKey, pszKeyName, (DWORD)0, REG_DWORD,
                   (UCHAR*)dwValue, sizeof(dwValue) );

}

//-------------------------------------
//
BOOL	GetDisRegistryStr( char* pszKeyName, char* pszStrBuffer, UINT nBufferLen, char* pszKeyDefaultVal)
{
  return	GetRegistryStr(REG_DISCOVER, pszKeyName, 
                          pszStrBuffer, nBufferLen, pszKeyDefaultVal);
}

BOOL	GetRegistryStr(char* pszSection, char* pszKeyName, 
                     char* pszStrBuffer, UINT nBufferLen, char* pszKeyDefaultVal)
{
	char		*pszKeyValue = pszKeyDefaultVal;
	HKEY		hKey;
	DWORD		nIndex = 0;
	char		pszName[ MAX_PATH ];
	DWORD		nType;
	BYTE		pbData[ MAX_PATH ];
	if ( RegOpenKeyEx( HKEY_CURRENT_USER, pszSection, 
									NULL, KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		DWORD nNameSize = sizeof( pszName );
		DWORD nDataSize = sizeof( pbData );
		while ( RegEnumValue( hKey, nIndex, pszName, &nNameSize, 
								NULL, &nType, pbData, &nDataSize) == ERROR_SUCCESS )
		{
			nIndex++;
			if( (nType == REG_SZ) && 
					( !stricmp( pszName, pszKeyName) ) )
			{
				*(pbData+nDataSize) = '\0';
				pszKeyValue = (char*) pbData;
				break;
			}
			nDataSize = sizeof( pszName ); //restore variable values before next iteration
			nNameSize = sizeof( pbData );
		}
	}
	
	if ( !pszKeyValue || strlen(pszKeyValue) >= nBufferLen )
		{
		*pszStrBuffer = '\0';
		return FALSE;
		}
	else
		{
		strcpy( pszStrBuffer, pszKeyValue );
		return TRUE;
		}
}


/****************************************************************************
		 DEBUG
****************************************************************************/

void ServerDebugMessageLog( UINT nErrLevel, char* pszWho, char* pszAction, 
													  char* pszComment1, char* pszComment2 )
{
	if ( pszWho == NULL )
		return;

	if ( (int)nErrLevel > GetDisRegistryInt(REG_DISCOVER_LOG_MSG, 255)	)
		return;

	char	szComment[256];
	strcpy( szComment, "");
	if ( pszComment1 )
		strcat( szComment, pszComment1 );
	strcat (szComment, " ");
	if ( pszComment2 )
		strcat( szComment, pszComment2 );

	char	szLogFileName[_MAX_PATH];
	FILE*	fileLog;

	GetDisRegistryStr(REG_DISCOVER_LOG, szLogFileName, sizeof(szLogFileName), "c:\\Discover.log");
	fileLog = fopen( szLogFileName, "a+t");

	if ( fileLog )
		{
		char		szTimeBuff[40], szDateBuff[40];
 
    _strtime( szTimeBuff );
    _strdate( szDateBuff );
    if ( (int)nErrLevel < GetDisRegistryInt(REG_DISCOVER_LOG_WNG, 128) )
		  fprintf( fileLog, "%s %s %3d:%20s :%21s : %s\n", szDateBuff, szTimeBuff, nErrLevel,
																							 pszWho, pszAction, szComment );
    else
		  fprintf( fileLog, "%s %s    :%20s :%21s : %s\n", szDateBuff, szTimeBuff, 
																							 pszWho, pszAction, szComment );
		fclose( fileLog );
		}

	if ( (int)nErrLevel < GetDisRegistryInt(REG_DISCOVER_LOG_ERR, 5)   )
		{
		char szText[256];
		sprintf( szText, "%s: %s", pszAction, szComment);
		MessageBox( NULL, szText, pszWho, MB_OK );
		}
}

