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
#include "Integrator.h"

#define MENU_BASE 13

BOOL ChooseMyCustomMenu( HWND hWnd, char *pszCommandPath )
{

	BOOL fResult = FALSE;
	int nSourceControlItems = 0;
	if( hWnd && pszCommandPath )
	{
	
		_strupr( pszCommandPath ); //convert to upper case
		HMENU hMainMenu = GetMenu( hWnd );
		if( hMainMenu )
		{
			int nItemCount = GetMenuItemCount( hMainMenu );
			HMENU hSubMenu = NULL;
			char pszBuffer[ 30 ];
			
			for( int i =0; (i < nItemCount) && !hSubMenu; i++ )
			{
				GetMenuString( hMainMenu, i, pszBuffer, 29, MF_BYPOSITION);
				if( ! strcmp( pszBuffer, "&Tools" ) )
					hSubMenu = GetSubMenu( hMainMenu, i );
			}

			if( hSubMenu )
			{
				nItemCount = GetMenuItemCount( hSubMenu );

				HKEY hKey, hKeySourceControl;
				DWORD nLastError;

				// Open a regestry key for 'Tools' menu
				if ( (nLastError = RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Developer\\Tools", NULL,
								KEY_READ, &hKey )) == ERROR_SUCCESS )
				{
					// Open a regestry key for Source Control
					if ( (nLastError = RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Developer\\Source Control", NULL,
									KEY_READ, &hKeySourceControl )) == ERROR_SUCCESS )
					{
						char pszName[20];
						BYTE pbData[MAX_PATH];
						int nMenuOffset = 0;
						DWORD nNameSize = 20; 
						DWORD nDataSize = MAX_PATH; 
						DWORD nType; 
						DWORD nIndex = 0;
						BOOL fIsFound = FALSE;
						char *pszSlash;


						//Check if IDE is integrated with version control software
						
						while ( RegEnumValue( hKeySourceControl, nIndex, pszName, &nNameSize, 
							NULL, &nType, pbData, &nDataSize) == ERROR_SUCCESS )
						{
							nIndex++;
							if( (nType == REG_SZ) && 
									( !_stricmp( pszName, "UserID") ) )
							{
								nSourceControlItems = 2;
								break; // break from the loop
							}
							nDataSize = MAX_PATH; //restore variable values before next iteration
							nNameSize = 20;
						}
						
						nDataSize = MAX_PATH;
						nNameSize = 20;
						nIndex = 0;
						//Enumerate all registry key values to find one pointing to our executable
						while ( RegEnumValue( hKey, nIndex, pszName, &nNameSize, 
							NULL, &nType, pbData, &nDataSize) == ERROR_SUCCESS )
						{
							nIndex++;
							if( (nType == REG_SZ) && 
									( strstr( pszName, "PathName#") == pszName ) )
							{
								_strupr( (char *) pbData ); //convert to upper case
								if( !strcmp( pszCommandPath, (char *) pbData ) ||  // if value data is a 
																					 // name of the executable	
									( (pszSlash=strrchr( (char *) pbData, '\\' ))  // or if it contains a back slash
																					 // and a name of the executable
																					 // immediately after it
										&&( strstr( (char *) pbData, pszCommandPath ) == pszSlash +1 ) ) )
								{
									char *pszNum = strchr( pszName, '#' );
									if( pszNum )
									{
										nMenuOffset = atoi( pszNum + 1 );// Get the numner of our tool in a list
																										 // of all custom tools
										fIsFound = TRUE;
									}
									break; // break from the loop

								}
							}

							nDataSize = MAX_PATH; //restore variable values before next iteration
							nNameSize = 20;
						}

						if( fIsFound ) //if key value was found for our executable
						{
							//consider all standard items in 'Tools'
							int nItem = MENU_BASE + nMenuOffset + nSourceControlItems; 
							if( nItem < nItemCount )
							{
								UINT nId = GetMenuItemID( hSubMenu, nItem );
								if( nId )
								{
									//Everything is OK, we can send WM_COMMAND message with an ID in its wParam
									PostMessage( hWnd, WM_COMMAND, MAKEWPARAM( nId, 0 ), 0 );
									fResult = TRUE;
								}
							}
						}
						RegCloseKey( hKeySourceControl );
					}
					RegCloseKey( hKey );
				}
			}
		}
	}


	return fResult;

}



/*****************************************************************************\
* BOOL ChooseMyMenu( HWND hWnd, UINT nMenuID, char *pszTopMenuTitle, 
*											char *pszSubMenuTitle )
*
* Use this function to simulate selecting a menu item (except custom menu
* items in 'Tools')
*
* Arguments:
*    HWND hWnd - window handle for the menu owner
*    UINt nMenuID - menu ID, may be 0
*    char *pszTopMenuTitle - title of the menu in a horizontal menu bar. This
*							 parameter is ignored if nMenuID != 0
*	 char *pszSubMenuTitle - title of the menu item in a popup menu. This
*							 parameter is ignored if nMenuID != 0
*
* Menu item is found either by menu ID or by the title of the menu in a 
* horizontal menu bar and title of the menu item in a popup menu
*
* Returns:
*    TRUE - Everything is ok
*    FALSE- Error.
\*****************************************************************************/
BOOL ChooseMyMenu( HWND hWnd, UINT nMenuID, char *pszTopMenuTitle, 
											char *pszSubMenuTitle )
{
	BOOL fResult = FALSE;
	//Ensure that window handle is not NULL and that it is valid
	if( hWnd || IsWindow( hWnd ) )
	{
		if( !nMenuID ) //We have to determine nMenuID
		{
			nMenuID = GetMenuIdByName( hWnd, pszTopMenuTitle, pszSubMenuTitle );
		}
	
		//Everything is OK, we can send WM_COMMAND message with an ID in its wParam
		if( nMenuID )
		{
			PostMessage( hWnd, WM_COMMAND, MAKEWPARAM( nMenuID, 0 ), 0 );
			fResult = TRUE;
		}
	
	}
	
	
	return fResult;
}



/*****************************************************************************\
* int GetMenuIdByName( HWND hWnd, char *pszTopMenuTitle, char *pszSubMenuTitle )
*
* Parameters: Handle of a menu owner window, menu name string (should contain '&'
*	if shortcut letters are used) and a sumenu name string
*
* Return value: menu item ID in case of success, 0 in case of failure
*
\*****************************************************************************/


int GetMenuIdByName( HWND hWnd, char *pszTopMenuTitle, char *pszSubMenuTitle, 
										BOOL fSubstring/*FALSE is a default parameter*/ )
{
 
	int nMenuID = 0;
	if( hWnd && IsWindow( hWnd ) && pszTopMenuTitle && pszSubMenuTitle )
	{
		HMENU hMainMenu = GetMenu( hWnd );
		if( hMainMenu )
		{
			int nItemCount = GetMenuItemCount( hMainMenu );
			HMENU hSubMenu = NULL;
			char pszBuffer[ 100 ];
			for( int i =0; (i < nItemCount) && !hSubMenu; i++ )
			{
				GetMenuString( hMainMenu, i, pszBuffer, 99, MF_BYPOSITION);
				if( ! strcmp( pszBuffer, pszTopMenuTitle ) )
					hSubMenu = GetSubMenu( hMainMenu, i );
			}

			if( hSubMenu )
			{
				nItemCount = GetMenuItemCount( hSubMenu );
				
				for( i =0; (i < nItemCount) && !nMenuID; i++ )
				{
					GetMenuString( hSubMenu, i, pszBuffer, 99, MF_BYPOSITION);
					char *pTab = strchr( pszBuffer, '\t' );
					if( pTab )
						*pTab = '\0';
					if( ! strcmp( pszBuffer, pszSubMenuTitle ) 
						 || ( fSubstring && (strstr( pszBuffer, pszSubMenuTitle )
																== pszBuffer) ) )
						nMenuID = GetMenuItemID( hSubMenu, i );
				}
			}
		}
	}
	return nMenuID;
}

