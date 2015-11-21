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
#include <windows.h>
#include <lm.h>
#include "mapnet.h"

static char gpszNetPath[ MAX_PATH ];

static BOOL GetLocalDirectoryByShare( char *, char * );


/*****************************************************************************\
* UnixNetName2LocalMapped( HINSTANCE hInst, HWND hParent, char *pszName, 
*													 char *pszResult, int nResultLength )
*
* Parameters: hInst - handle of application instance (needed for getting a
*							dialog resource IDD_DIALOG2 for a network password dialog;
*							hParent - handle of a parent window for dialog boxes;
*							pszName - UNIX network file name (like /net/host/share/directory/file);
*							pszResult - buffer for a result string. This buffer will contain
*							local NT file path (like [drive]:\directory\file) if function succeds,
*							and an empty string if function fails;
*							nResultLength - length in bytes of the buffer for a result string
* Return value: 0 in case of success, and a negative value described in mapnet.h
*								in a case of failure
*
* This function enumerates a local network to see if there is an existing network
*	connection. If connection is not found, it will try to connect to a network
*	resource.
*
\*****************************************************************************/


int UnixNetName2LocalMapped( HINSTANCE hInst, HWND hParent, char *pszName, char *pszResult, int nResultLength )
{
	
	int nResult = NAME_SUCCESS;
	BOOL fAlreadyMapped = FALSE;
	BOOL fNetHostIsLocal = FALSE;
	char pszSubPath[ MAX_PATH ];
	char pszNameConverted[ MAX_PATH ];
	char pszNameConvertedUNC[ MAX_PATH ];
	char pszNameIntermediate[ MAX_PATH ];
	char pszDriveMapped[ _MAX_DRIVE ];
	char pszResultString[ MAX_PATH ];
	char HostName[ MAX_COMPUTERNAME_LENGTH + 1 ];

	pszNameConverted[0] = '\0';
	strcpy( pszNameConvertedUNC, "\\\\" );
	if( pszName )
	{
		if( strstr( pszName, "/net/" ) == pszName )
		{
			strcpy( pszNameIntermediate, pszName + 5 );
			char *pFirstSlash = strchr( pszNameIntermediate, '/' );
			if( pFirstSlash )
			{
				int nToCopy = (int)( pFirstSlash - pszNameIntermediate );
				strncpy( pszNameConverted, pszNameIntermediate, nToCopy );
				strncpy( pszNameConvertedUNC + 2, pszNameIntermediate, nToCopy );
				
				// Check here if host name is actually a local machine
				DWORD nLength = MAX_COMPUTERNAME_LENGTH + 1;
				if( !GetComputerName( HostName, &nLength ) )
					HostName[0] = '\0';
				pszNameConverted[	nToCopy ] = '\0';
				if( !stricmp( pszNameConverted, HostName )	)
				{
					fNetHostIsLocal = TRUE; //set local machine flag
					strcpy( pszNameConverted, pFirstSlash + 1 );
					char *pFirstSlash = strchr( pszNameConverted, '/' );
					if( pFirstSlash )
						*pFirstSlash = '\0';
					// get a local path by share name
					if( GetLocalDirectoryByShare( pszNameConverted, pszSubPath ) )
					{
						fAlreadyMapped = TRUE;
						// add remaining path to the local directory
						strcat( pszSubPath, "\\" );
						strcat( pszSubPath, pFirstSlash + 1 );
						ChangeSlash( pszSubPath );
					}
				}
				else
				{
					// prepare UNIX-style and UNC-style network path strings
					pszNameConverted[	nToCopy ] = ':';
					pszNameConverted[	nToCopy + 1 ] = '\0';
					pszNameConvertedUNC[	nToCopy + 2 ] = '\0';
					strcat( pszNameConverted, pFirstSlash );
					strcat( pszNameConvertedUNC, pFirstSlash );
					ChangeSlash( pszNameConvertedUNC );
				}
			}
		}
		
		HANDLE hEnum;
		if( !fNetHostIsLocal )
		{
			// Enumerate all existing network connections
			if( WNetOpenEnum( RESOURCE_CONNECTED, RESOURCETYPE_DISK, 
												0, NULL, &hEnum ) == NO_ERROR )
			{
				NETRESOURCE *pNetResource;
				DWORD nCount = 0xFFFFFFFF;
				DWORD nBuffLen = sizeof( NETRESOURCE ) * 50;
				pNetResource = new NETRESOURCE[ nBuffLen ];
				if( pNetResource )
				{
					while( (WNetEnumResource( hEnum, &nCount, pNetResource, 
								&nBuffLen ) == NO_ERROR) && !fAlreadyMapped )
					{
						for( UINT i = 0; (i < nCount) && !fAlreadyMapped ; i++ )
						{
							// Here we are looking for a connected network resource
							// taking into account that network name may be given
							// as host:/share/directory or \\host\share\directory (UNC)
							
							if( strstr( pszNameConverted, pNetResource[i].lpRemoteName ) ==  pszNameConverted)
							{
								fAlreadyMapped = TRUE;
								if( pNetResource[i].lpLocalName )
									strcpy( pszDriveMapped, pNetResource[i].lpLocalName );
								else
									pszDriveMapped[0] = '\0';
								strcpy( pszSubPath, pszNameConverted + strlen( pNetResource[i].lpRemoteName ) );
								ChangeSlash( pszSubPath );
							}
							else if( strstr( pszNameConvertedUNC, pNetResource[i].lpRemoteName ) 
																==  pszNameConvertedUNC)
							{
								fAlreadyMapped = TRUE;
								if( pNetResource[i].lpLocalName )
									strcpy( pszDriveMapped, pNetResource[i].lpLocalName );
								else
									pszDriveMapped[0] = '\0';
								strcpy( pszSubPath, pszNameConvertedUNC + strlen( pNetResource[i].lpRemoteName ) );
								ChangeSlash( pszSubPath );
							}
	 					}
						nCount = 0xFFFFFFFF;
					}
					delete pNetResource;
					WNetCloseEnum( hEnum );
					if( !fAlreadyMapped ) // We have to mount it ourselves
					{
						strcpy( gpszNetPath, pszNameConverted );
						// find a last slash
						char *pLastSlash = strrchr( gpszNetPath, '/' );
						if( pLastSlash )
						{
							strcpy( pszSubPath, pLastSlash );
							ChangeSlash( pszSubPath );
							*pLastSlash = '\0'; // put a zero-terminator before a file name
						}
						else
							pszSubPath[0] = '\0';

						DWORD nAllDrives = GetLogicalDrives();
						int nDrive = -1;
						
						// Looking for an available drive letter (starting from D:)
						nAllDrives = nAllDrives >> 2;
						for( int i = 2; i < 26; i++ )
						{
							if( !(nAllDrives & 1) ) // if-statement will be true
																			// if less-significant bit of nAllDrives is 0
							{
								nDrive = i;
								break;
							}
							
							nAllDrives = nAllDrives >> 1; 
						}
						if( nDrive >= 0 )
						{
							wsprintf( pszDriveMapped, "%c:", 'a'+ (char)nDrive );
							NETRESOURCE NewNetRes;
							NewNetRes.dwType = RESOURCETYPE_DISK;
							NewNetRes.lpLocalName = pszDriveMapped;
							NewNetRes.lpRemoteName = gpszNetPath;
							NewNetRes.lpProvider = NULL;
							// Try to mount a network path
							DWORD nMapped = WNetAddConnection3( hParent, &NewNetRes, NULL, NULL, 0 );

							if( nMapped == NO_ERROR )
								fAlreadyMapped = TRUE;
							// If password is required, ask password and try again
							else if( nMapped == ERROR_INVALID_PASSWORD ) 
							{
								char	*pszPassword = GetPassword(hInst, hParent, gpszNetPath);
								if ( pszPassword )
								{
									nMapped = WNetAddConnection3( hParent, &NewNetRes, pszPassword, NULL, 0 );
									if( nMapped == NO_ERROR )
										fAlreadyMapped = TRUE;
								}
							}
							if( !fAlreadyMapped )
							{
								// If network path is not mapped for some reason,
								// display an error message
								char pszMsgBuff[ 50 ];
								wsprintf( pszMsgBuff, "Win32 Error is %d", nMapped );
								MessageBox( hParent, pszMsgBuff, 
																"Couldn't connect to a network drive", MB_ICONSTOP );
							}
						}

					}
					
				}
				else
				{
					nResult = NAME_ERROR_ALLOCATE;
					WNetCloseEnum( hEnum );
				}
			}
			else
				nResult = NAME_ERROR_CREATE_ENUM;
		}

		if( fAlreadyMapped )
		{
			if( !fNetHostIsLocal )
				strcpy( pszResultString, pszDriveMapped );
			else
				pszResultString[0] = '\0';
			strcat( pszResultString, pszSubPath );
			int nLen = min( (int)strlen( pszResultString ) + 1, nResultLength );
			// ensure that we don't write beyond a boundary of user-supplied buffer
			strncpy( pszResult, pszResultString, nLen - 1 );
			pszResult[ nLen - 1 ] = '\0';
		}
		else
		{
			if( nResultLength > 0 )
				pszResult[ 0 ] = '\0';
			nResult = NAME_ERROR_NOT_MAPPED;
		}

	}
	else
		nResult = NAME_INVALID_BUFFER;
	
	return nResult;
}


/*****************************************************************************\
* GetLocalDirectoryByShare( char *pszShareName, char *pszResult )
*
* Parameters: pszShareName - network share name (non-UNICODE);
*							pszResult - result buffer for a non-UNICODE string
*
* Return value: TRUE in case of success, FALSE in case of failure 
*
* This function gets a local path by a share name on a local machine.
*
\*****************************************************************************/


BOOL GetLocalDirectoryByShare( char *pszShareName, char *pszResult )
{
	BOOL fFound = FALSE;
	SHARE_INFO_2 *pInfo;
	wchar_t ShareName[ MAX_PATH ];
	pszResult[0] = '\0';
	if( MultiByteToWideChar( CP_ACP, 0, pszShareName, -1, ShareName, MAX_PATH ) )
	{
		if( NetShareGetInfo( NULL/*local*/, (LPTSTR)ShareName, 2, (LPBYTE *) &pInfo ) 
				== NERR_Success )
		{
			if( (pInfo->shi2_path) && WideCharToMultiByte( CP_ACP, 0, (wchar_t *)pInfo->shi2_path, -1, pszResult,
													 MAX_PATH, NULL, NULL ) )
				fFound = TRUE;

		}

	}
	return fFound;
}

/*****************************************************************************\
* ChangeSlash
*
* Parameters: pszString - string to be converted
*
* Return value: NONE
*
* This function converts all forward slashes in a string into backslashes
*
\*****************************************************************************/

void ChangeSlash( char *pszString )
{

	char *pSlash;
	while( (pSlash=strchr( pszString, '/' )) != NULL )
		*pSlash = '\\';

}

static BOOL GetSharedPath( wchar_t *, wchar_t *, char *, char * );


/*****************************************************************************\
* LocalMapped2UnixNetName( char *pszName, char *pszResult, int nResultLength )
*
* Parameters: pszName - local path (like [drive]:\share(directory)\directory\file);
*							pszResult - buffer for a result string. This buffer will contain
*							UNIX-style network path (like /net/host/share/directory/file) if 
*							function succeds;
*							and an empty string if function fails;
*							nResultLength - length in bytes of the buffer for a result string
* Return value: 0 in case of success, and a negative value described in mapnet.h
*								in a case of failure
*
* This function checks if local path is actually a mounted network path or is
*	shared by a local machine itself. It will write a UNIX-style network name
* to a result string if possible or just a 0-terminator in case of some failure
*
\*****************************************************************************/

int LocalMapped2UnixNetName( char *pszName, char *pszResult, int nResultLength )
{
	
	int nResult = NAME_ERROR_NOT_MAPPED;
	BOOL fFound = FALSE;
	char pszSubPath[ _MAX_DIR ];
	char pszNetPath[ MAX_PATH ];
	char pszIntermediate[ MAX_PATH ];
	char pszDriveMapped[ _MAX_DRIVE ];
	char pszFile[ _MAX_FNAME ];
	char pszExt[ _MAX_EXT ];
	char pszResultString[ MAX_PATH ];

	pszResult[0] = '\0';

	if( pszName )
	{

		_splitpath( pszName, pszDriveMapped, pszSubPath, pszFile, pszExt );
		DWORD nLen = MAX_PATH;
		DWORD nGetConnResult = WNetGetConnection( pszDriveMapped, pszNetPath, &nLen );
		if( nGetConnResult == NO_ERROR )
		{

			pszIntermediate[0] = '\0';
			char *pFirstColon = strchr( pszNetPath, ':' );
			if( pFirstColon ) //assuming "host:/share/directory" style
			{
				int nToCopy = (int)( pFirstColon - pszNetPath );
				strncpy( pszIntermediate, pszNetPath, nToCopy );
				pszIntermediate[ nToCopy ] = '\0';
				strcat( pszIntermediate, pFirstColon + 1 );
			}
			else if( strstr( pszNetPath, "\\\\") == pszNetPath ) //assuming UNC style
				strcpy( pszIntermediate, pszNetPath + 2 );
				
			ChangeSlashForward( pszIntermediate );
			ChangeSlashForward( pszSubPath );
			wsprintf( pszResultString, "/net/%s%s%s%s", pszIntermediate, pszSubPath, pszFile,	pszExt );
			fFound = TRUE;
		}
		else if( nGetConnResult == ERROR_NOT_CONNECTED || nGetConnResult == ERROR_BAD_DEVICE)
		{
		 // Assuming that pszName is a local path

		 // Now we have to enumerate all shares and find out
		 //	if pszName is shared
			SHARE_INFO_2 *pInfo;
			DWORD nReturned = 0; 
			DWORD nTotal = 0;
			DWORD nNextEnum = 0;
			UINT i;
			long lErr;
			DWORD nBytes = sizeof(SHARE_INFO_2)*512;

			while( (lErr = NetShareEnum(NULL, 2/*we request answer in a form of SHARE_INFO_2*/, 
													(LPBYTE *) &pInfo, nBytes, &nReturned,	&nTotal, &nNextEnum) )
					== ERROR_MORE_DATA && !fFound )

			{
				for ( i=0; i < nReturned; i++ ) 
				{
					if (pInfo[i].shi2_type == STYPE_DISKTREE)
						// found a shared directory
						if( GetSharedPath( (wchar_t *)pInfo[i].shi2_path, (wchar_t *)pInfo[i].shi2_netname,
															 pszName, pszResultString ) )	
						{
							fFound = TRUE;
							break;
						}
				}
			}
			// enumeration is complete
			if( lErr==NERR_Success && !fFound )
				for ( i=0; i < nReturned; i++ ) 
				{
					if (pInfo[i].shi2_type == STYPE_DISKTREE)
							// found a shared directory

						if( GetSharedPath( (wchar_t *)pInfo[i].shi2_path, (wchar_t *)pInfo[i].shi2_netname,
															 pszName, pszResultString ) )	
						{
							fFound = TRUE;
							break;
						}
				}
		}
		if( fFound )
		{
			// ensure that we don't write beyond a boundary of user-supplied buffer
			int nLength = min( (int)strlen( pszResultString ) + 1, nResultLength );
			strncpy( pszResult, pszResultString, nLength - 1 );
			pszResult[ nLength - 1 ] = '\0';
			nResult = NAME_SUCCESS;
		}
	}

	return nResult;
}

/*****************************************************************************\
* GetSharedPath( wchar_t *pSharedPath, wchar_t *pSharedName, char *pszNeeded, 
*									  char *pszBuffer )
*
* Parameters: pSharedPath - local directory being shared (UNICODE string);
*							pSharedName - name of the share (UNICODE string);
*							pszNeeded - path we are trying to make visible from UNIX (non-UNICODE)
*							pszBuffer - buffer for a result string (non-UNICODE)
*							
* Return value: TRUE in case of success, FALSE in case of failure 
*
* This function checks if a given share contains a path we are trying to make
*	visible from UNIX and writes UNIX-style net name into a pszBuffer if posiible.
*
\*****************************************************************************/

BOOL GetSharedPath( wchar_t *pSharedPath, wchar_t *pSharedName, char *pszNeeded, 
									  char *pszBuffer )
{
	char SharedDir[ MAX_PATH ];
	char NeededPath[ MAX_PATH ];
	char ShareName[ _MAX_FNAME ];
	char HostName[ MAX_COMPUTERNAME_LENGTH + 1 ];

	BOOL fResult = FALSE;
	// convert strings from UNICODE to one-byte-character strings
	if( WideCharToMultiByte( CP_ACP, 0, pSharedPath, -1, SharedDir,
													 MAX_PATH, NULL, NULL )  && 
			WideCharToMultiByte( CP_ACP, 0, pSharedName, -1, ShareName,
													 _MAX_FNAME, NULL, NULL ) )
	{
		// Convert needed path and shared path to uppercase since FAT and NTFS are
		// not case sensitive
		strupr( SharedDir );
		strcpy( NeededPath, pszNeeded );
		strupr( NeededPath );
		if( strstr( NeededPath, SharedDir ) == NeededPath ) // shared directory contains
																											 // needed path
		{
			fResult = TRUE;
			DWORD nLength = MAX_COMPUTERNAME_LENGTH + 1;
			if( !GetComputerName( HostName, &nLength ) )
				HostName[0] = '\0';
			wsprintf( pszBuffer, "/net/%s/%s%s", HostName, ShareName, 
								pszNeeded + strlen( SharedDir ) );
			ChangeSlashForward( pszBuffer );

		}
	}


	return fResult;
}

/*****************************************************************************\
* ChangeSlashForward( char *pszString )
*
* Parameters: pszString - string to be converted
*
* Return value: NONE
*
* This function converts all backslashes in a string to forward slashes
*
\*****************************************************************************/

void ChangeSlashForward( char *pszString )
{

	char *pSlash;
	while( (pSlash=strchr( pszString, '\\' )) != NULL )
		*pSlash = '/';

}

