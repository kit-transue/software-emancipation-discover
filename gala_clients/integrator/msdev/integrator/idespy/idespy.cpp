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
#include <commctrl.h>
#include <ddeml.h>
#include "integrator.h"
#include "resource.h"
#include "idespy.h"




HWND				ghDeveloperStudioWnd = NULL;   
HWND				ghOutputWnd = NULL;            
HWND				ghOutputParentWnd = NULL;      
HWND				ghDiscoverWindow = NULL;
HWND				ghMDIClientWnd = NULL;
HWND				ghWorkerWindow = NULL;
HWND				ghToolTips = NULL;       
HWND				ghToolbar = NULL;
HWND				ghProjectComboBox = NULL;
HWND				ghProjComboBoxList = NULL;
HINSTANCE		ghInstance = NULL;         
WNDPROC			gpOldOutputWndProc = NULL; 
WNDPROC			gpOldParentWndProc = NULL; 
WNDPROC			gpOldDeveloperStudioProc = NULL;
HANDLE			ghEvent = NULL;
HANDLE			ghMutex = NULL;
BOOL				fClosing = FALSE;
HHOOK				hWaitHook = NULL;
int					gnSaveAllId = 0;
int					gnSaveId = 0;
int					gnCompileId = 0;
int					gnBuildId = 0;
int					gnRebuildAllId = 0;
int					gnBatchBuildId = 0;
char				pszStatus[ MAX_PATH ]; //Status line global string
char				gpszConfiguration[ MAX_PATH ];

HFILE hDebug = NULL;	//temporary


/*****************************************************************************\
* DllMain (hModule,cbHeap,lpchCmdLine)
*
* Called when the libary is loaded
*
* Arguments:
*    PVOID hModule - Module handle for the libary.
*    ULONG ulReason - DLL purpose
*    PCONTEXT pctx - not used
*
* Returns:
*    TRUE - Everything is ok
*    FALSE- Error.
\*****************************************************************************/

extern "C" BOOL
APIENTRY DllMain(
    PVOID hModule,
    ULONG ulReason,
    PCONTEXT pctx
    )
{
	char pszBuffer[ 10 ], pszModuleName[ MAX_PATH ];
	BOOL fResult = TRUE;
	// DLL is instantiated.
  if ( ulReason == DLL_PROCESS_ATTACH )
	{
		GetModuleFileName( NULL, pszModuleName, sizeof( pszModuleName ) );
		if( strstr( _strupr( pszModuleName ), ATTACH_PROCESS_NAME ) ) 
		//if we are attaching to Visual C++ process
		{
			hDebug = _lcreat(	"d:\\debug.txt", 0 ); //temporary
			pszStatus[0] = '\0';

			DWORD nThreadID = GetCurrentThreadId();
			EnumThreadWindows( nThreadID, (WNDENUMPROC) EnumCbck, 0 );
			if( ghDeveloperStudioWnd )
			{
				wsprintf( pszBuffer, "%ld", (DWORD) ghDeveloperStudioWnd );
				ghEvent = OpenEvent( EVENT_MODIFY_STATE, TRUE, pszBuffer );
				if( ghEvent )
				{
					wsprintf( pszBuffer, "M%ld", (DWORD) ghDeveloperStudioWnd );
					ghMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, pszBuffer );
					if( !ghMutex )
					{
						fResult = FALSE;
						CloseHandle( ghEvent );
					}
				}
				else
					fResult = FALSE;
			}
			else
				fResult = FALSE;
		}
		
  }
	else if ( ulReason == DLL_PROCESS_DETACH )
	{
		// If we got here and ghMutex is not NULL, it means that dll is unloading
		// from a process space of Visual C, and we can close a mutex handle
		if( ghMutex )
			CloseHandle( ghMutex );
	}

    return fResult;
}


/*****************************************************************************\
* GetMsgHookProc
*
* The Get Message hook function.
*
\*****************************************************************************/

LRESULT CALLBACK
GetMsgHookProc(
    INT hc,
    WPARAM wParam,
    LPARAM lParam
    )
{
    
	//WaitForSingleObject( ghMutex, INFINITE );
	PMSG pmsg;
	static int i = 1;
	pmsg = (PMSG)lParam;
	LRESULT lResult = 0;
	
	if( i==1 )
	{
		i++;
		if ( GetWindowHandles() ) // Get handles of all windows we are interested in
		{
			if ( InitSubclass() )		// Subclass window procedures
			{
				
				ghWorkerWindow = CreateWorkerWindow( ghDeveloperStudioWnd );
				ghDiscoverWindow = CreateDiscoverWindow( ghOutputWnd );
				if( !(ghDiscoverWindow && ghWorkerWindow) )
				{
					SignalFinish();
				}
				else
				{
					GetConfiguration();
					GetDDEClient()->CommandToServer( IDE_MSG_SPYSTARTED, gpszConfiguration );
          PostMessage( ghDiscoverWindow, WM_COMMAND, (WPARAM)ID_ACTIVBROWSER, 0L );
				}
			}
			else
				SignalFinish();
		}
		else
			SignalFinish();
	}
	else
	{
		switch( pmsg->message )
		{
		case WM_LBUTTONDBLCLK:
			if( pmsg->wParam == (MK_CONTROL|MK_LBUTTON) )
			{
				HWND hParent = GetParent( pmsg->hwnd );
				if( hParent )
					hParent = GetParent( hParent );
				if( hParent )
				{
					char buffer[TITLE_LENGTH];
					if( GetWindowTextLength( hParent ) )
						GetWindowText( hParent, buffer, sizeof( buffer ) - 1 );
					else
						buffer[0] = '\0';
					if(	(GetParent( hParent )==ghMDIClientWnd) &&
							_stricmp( buffer, "InfoViewer Topic" /*don't care about help windows*/ ) 
							&& ghWorkerWindow )
						//Send Double-Click Event
					{
						PostMessage( ghWorkerWindow, WM_USERSHIFTDBLCLK, 0L, 0L );
					}
				}
			}
			break;
		}


	}


  if( !fClosing)
		lResult = CallNextHookEx(NULL, hc, wParam, lParam);
	//ReleaseMutex( ghMutex );
	return lResult;
}


/*****************************************************************************\
* GetWindowHandles()
*
* Parameters: NONE
* Return value: Handle of Developer Studio's output window
*
* Gets handles for all windows in Developer Stdudio used by IDESpy 
*
\*****************************************************************************/

BOOL GetWindowHandles()
{

	HWND hParent = NULL;
	if( ghDeveloperStudioWnd )
	{
		EnumChildWindows( ghDeveloperStudioWnd, (WNDENUMPROC) EnumCbck4, 0 ); 
		EnumChildWindows( ghDeveloperStudioWnd, (WNDENUMPROC) EnumCbck2, (LPARAM) &hParent );
	}
	if( !hParent )
		EnumChildWindows( GetDesktopWindow(), (WNDENUMPROC) EnumCbck2, (LPARAM) &hParent );
	if( hParent )
	{
		ghOutputParentWnd = hParent;
		hParent = GetParent( hParent );
		if( hParent )
			EnumChildWindows( hParent, (WNDENUMPROC) EnumCbck3, 0 );
	}

	return (BOOL) ghOutputWnd;
}


/*****************************************************************************\
* CleanUp( BOOL fRemove)
*
* Parameters: If fRemove is TRUE, original window procedure of a main Developer
*						  Studio window is put back
* Return value: TRUE if success, otherwise FALSE
*
* Puts all window procedures back
*
\*****************************************************************************/
BOOL CleanUp( BOOL fRemove )
{
	if ( gpOldOutputWndProc && ghOutputWnd )
		SetWindowLong( ghOutputWnd, GWL_WNDPROC, (long)gpOldOutputWndProc);

	if ( gpOldParentWndProc && ghOutputParentWnd )
		SetWindowLong( ghOutputParentWnd, GWL_WNDPROC, (long)gpOldParentWndProc);

	if( fRemove )
	{
		if ( gpOldDeveloperStudioProc && ghDeveloperStudioWnd )
			SetWindowLong( ghDeveloperStudioWnd, GWL_WNDPROC, (long)gpOldDeveloperStudioProc);
		if( hWaitHook )
			UnhookWindowsHookEx( hWaitHook );
		if( ghWorkerWindow )
			DestroyWindow( ghWorkerWindow );
		DDEServerFinish();
	}
	return TRUE;
}

/*****************************************************************************\
* SignalFinish()
*
* Parameters: NONE
* Return value: NONE
*
*	Sets an event indicating necessity to unload a dll, to a signaled state
*
\*****************************************************************************/
void SignalFinish()
{
	_lclose( hDebug );  //temporary
	CleanUp();
	if( ghEvent )
	{
		SetEvent( ghEvent );
		CloseHandle( ghEvent );
	}
}

/*****************************************************************************\
* InitSubclass( BOOL fSubclass)
*
* Parameters: If fSubclass is TRUE, original window procedure of a main Developer
*						  Studio window is replaced with a custom window procedure
* Return value: TRUE if success, otherwise FALSE
*
* Replaces window procedures with custom ones 
*
\*****************************************************************************/

BOOL InitSubclass( BOOL fSubclass)
{
	BOOL fResult1 = FALSE, fResult2 = FALSE, fResult3 = FALSE, fResult4 = FALSE;
	ghInstance = (HINSTANCE) GetModuleHandle( DLL_MODULE_NAME );
	gpOldOutputWndProc = (WNDPROC) GetWindowLong( ghOutputWnd, GWL_WNDPROC);
	gpOldParentWndProc = (WNDPROC) GetWindowLong( ghOutputParentWnd, GWL_WNDPROC);
	
	fResult1 = SetWindowLong( ghOutputWnd, GWL_WNDPROC, (long)NewOutputWindowProc);
	fResult2 = SetWindowLong( ghOutputParentWnd, GWL_WNDPROC, (long)NewOutputParentWindowProc);
	
	if( fSubclass )
	{
		// Make sure that Combobox on Project toolbar window exists
		// We send WM_COMMAND message twice to ensure that user's settings are kept
		PostMessage( ghDeveloperStudioWnd, WM_COMMAND, MAKEWPARAM( ID_PROJECTTOOLBAR, 0 ), 0L );
		PostMessage( ghDeveloperStudioWnd, WM_COMMAND, MAKEWPARAM( ID_PROJECTTOOLBAR, 0 ), 0L );
		
		gpOldDeveloperStudioProc = (WNDPROC) GetWindowLong( ghDeveloperStudioWnd, GWL_WNDPROC);
		fResult3 = SetWindowLong( ghDeveloperStudioWnd, GWL_WNDPROC, 
															(long)NewDeveloperStudioProc );
		gpszConfiguration[0] = '\0';
		gnSaveId = GetMenuIdByName( ghDeveloperStudioWnd, "&File", "&Save" );
		gnSaveAllId = GetMenuIdByName( ghDeveloperStudioWnd, "&File", "Save Al&l" );
		gnCompileId = GetMenuIdByName( ghDeveloperStudioWnd, "&Build", "&Compile", TRUE );
		gnBuildId = GetMenuIdByName( ghDeveloperStudioWnd, "&Build", "&Build", TRUE );
		gnRebuildAllId = GetMenuIdByName( ghDeveloperStudioWnd, "&Build", "&Rebuild All" );
		gnBatchBuildId = GetMenuIdByName( ghDeveloperStudioWnd, "&Build", "Batch B&uild..." );
		fResult4 = DDEServerStart(ghDeveloperStudioWnd); 
	}
	else
	{
		fResult3 = TRUE;
		fResult4 = TRUE;
	}
	return fResult1 && fResult2 && fResult3 && fResult4;	
}

/*****************************************************************************\
* CreateDiscoverWindow( HWND )
*
* Parameters: Handle of a parent window
* Return value: Handle of a newly created window
*
* Creates a window with a toolbar 
*
\*****************************************************************************/

HWND CreateDiscoverWindow( HWND hOwner )
{
	RECT rect;
	
	GetClientRect( ghOutputWnd, &rect );
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW ;
  wc.lpfnWndProc   = (WNDPROC)DiscWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = ghInstance;
  wc.hIcon         = NULL;
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName	 = NULL;
	wc.lpszClassName = "MyDiscoverWndClass";

	RegisterClass( &wc );
	HWND hDscWnd = CreateWindow( "MyDiscoverWndClass", "", WS_VISIBLE|WS_CHILD, 0, 0, 
												rect.right, rect.bottom, hOwner, 0, (HINSTANCE) ghInstance, 
												NULL);
	
	
	return hDscWnd;
}


/*****************************************************************************\
* CreateWorkerWindow( HWND )
*
* Parameters: Handle of a parent window
* Return value: Handle of a newly created window
*
* Creates an invisible window for message handling purposes 
*
\*****************************************************************************/

HWND CreateWorkerWindow( HWND hOwner )
{
	
	WNDCLASS wc;
	wc.style         = 0;
  wc.lpfnWndProc   = (WNDPROC)WorkerWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = ghInstance;
  wc.hIcon         = NULL;
  wc.hCursor       = NULL;
  wc.hbrBackground = NULL;
	wc.lpszMenuName	 = NULL;
	wc.lpszClassName = "MyWorkerWndClass";

	RegisterClass( &wc );
	HWND hWorkerWnd = CreateWindow( "MyWorkerWndClass", "", WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, 
												CW_USEDEFAULT, CW_USEDEFAULT, hOwner, 0, (HINSTANCE) ghInstance, 
												NULL);
	return hWorkerWnd;
}

/*****************************************************************************\
* ChangeStatusLine( char *pszLineToDisplay )
*
* Parameters: 0-terminated string to display
* Return value: None
*
* Changes global status string and invalidates status window
*
\*****************************************************************************/

void ChangeStatusLine( char *pszLineToDisplay )
{
	if( ghDiscoverWindow && IsWindow(ghDiscoverWindow) && pszLineToDisplay )
	{
		RECT rect;
		rect.left   = DISC_DRAW_LEFT;
		rect.top    = DISC_DRAW_TOP;
		rect.right  = DISC_DRAW_RIGHT;
		rect.bottom = DISC_DRAW_BOTTOM;
		strcpy( pszStatus, pszLineToDisplay );
		InvalidateRect( ghDiscoverWindow, &rect, TRUE );
		UpdateWindow( ghDiscoverWindow );
	}
}

/*****************************************************************************\
* Enumeration callbacks. Used to find window handles
*
\*****************************************************************************/

BOOL CALLBACK EnumCbck( HWND hWnd, LPARAM lParam)
{

	char lpszWindowTitle[TITLE_LENGTH], lpszWindowClass[CLASSNAME_LENGTH];
	GetClassName( hWnd, lpszWindowClass, CLASSNAME_LENGTH );
	if( GetWindowTextLength( hWnd ) )
		GetWindowText( hWnd, lpszWindowTitle, TITLE_LENGTH);
	if ( strstr( lpszWindowTitle, MAIN_WINDOW_TITLE ) ) //!strcmp( lpszWindowClass, "Afx:400000:8" )	)
	{
		ghDeveloperStudioWnd = hWnd;
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK EnumCbck2( HWND hWnd, LPARAM lParam)
{

	char lpszWindowTitle[TITLE_LENGTH], lpszWindowClass[CLASSNAME_LENGTH];
	GetClassName( hWnd, lpszWindowClass, CLASSNAME_LENGTH );
	if( GetWindowTextLength( hWnd ) )
		GetWindowText( hWnd, lpszWindowTitle, TITLE_LENGTH);
	if ( !strcmp( lpszWindowClass, TABBED_WINDOW_CLASS )	&&
		 !strcmp( lpszWindowTitle, DISCOVER_WINDOW_NAME ))
	{
		*( (HWND *)lParam ) = hWnd;
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK EnumCbck3( HWND hWnd, LPARAM lParam)
{
	char lpszWindowTitle[TITLE_LENGTH], lpszWindowClass[CLASSNAME_LENGTH];
	GetClassName( hWnd, lpszWindowClass, CLASSNAME_LENGTH );
	if( GetWindowTextLength( hWnd ) )
		GetWindowText( hWnd, lpszWindowTitle, TITLE_LENGTH);
	if ( !strcmp( lpszWindowClass, OUTPUT_WINDOW_CLASS )	)
	{
		ghOutputWnd = hWnd;
		return FALSE;
	}
	return TRUE;
}


BOOL CALLBACK EnumCbck4( HWND hWnd, LPARAM lParam)
{
	char lpszWindowClass[CLASSNAME_LENGTH];
	GetClassName( hWnd, lpszWindowClass, CLASSNAME_LENGTH );
	if ( !strcmp( lpszWindowClass, "MDIClient" )	)
	{
		ghMDIClientWnd = hWnd;
		return FALSE;
	}
	return TRUE;
}


/*****************************************************************************\
* FindProjectComboBox()
*
* Parameters: None
* Return value: Handle of Project configutaion selection combobox
*
* Enumerates windows and finds a combobox handle 
*
\*****************************************************************************/

HWND FindProjectComboBox( void )
{
	EnumChildWindows( ghDeveloperStudioWnd, (WNDENUMPROC) EnumComboCbck, 0L );
	if( (!ghProjectComboBox) || (!IsWindow( ghProjectComboBox ) ) )
		EnumChildWindows( GetDesktopWindow(), (WNDENUMPROC) EnumComboCbck, 0L );
	return ghProjectComboBox;
}


/*****************************************************************************\
* BOOL GetConfiguration()
*
* Parameters: None
* Return value: TRUE in case of success, FALSE in case of failure
*
* Stores current project configuration text in gpszConfiguration global array 
*
\*****************************************************************************/
BOOL GetConfiguration()
{
	BOOL fResult = FALSE;
	if( (!ghProjectComboBox) || (!IsWindow( ghProjectComboBox ) ) )
		FindProjectComboBox();
	if( ghProjectComboBox && IsWindow( ghProjectComboBox ) )
	{
		if( GetWindowTextLength( ghProjectComboBox ) )
		{
			GetWindowText( ghProjectComboBox, gpszConfiguration, sizeof( gpszConfiguration ) );
			fResult = TRUE;
		}
	}
	return fResult;
}

BOOL CALLBACK EnumComboCbck( HWND hWnd, LPARAM lParam )
{
	char lpszWindowClass[CLASSNAME_LENGTH];
	long lId = 0;
	GetClassName( hWnd, lpszWindowClass, sizeof( lpszWindowClass ) );
	lId = GetWindowLong( hWnd, GWL_ID );

	if ( (!strcmp( lpszWindowClass, "ComboBox" )) && (lId == ID_PROJECTCOMBOBOX )
		&& ( GetWindowThreadProcessId( hWnd, NULL ) == GetCurrentThreadId() ) )
	{
		ghProjectComboBox = hWnd;
		return FALSE;
	}
	return TRUE;
}

/*****************************************************************************\
* Terminate()
*
* Parameters: None
* Return value: None
*
* Terminates integration (closes all windows and, unsubclasses window procedures
* and initiates unloading a dll
*
\*****************************************************************************/

void Terminate()
{
	fClosing = TRUE;
	if( ghWorkerWindow && IsWindow( ghWorkerWindow ) )
		DestroyWindow( ghWorkerWindow );
	if( ghDiscoverWindow && IsWindow( ghDiscoverWindow ) )
		DestroyWindow( ghDiscoverWindow );
	else
		SignalFinish();
}

/*****************************************************************************\
* BOOL SetConfiguration( char *pszNewConfiguration )
*
* Parameters: pszNewConfiguration - new project configuration
* Return value: TRUE in case of success, FALSE in case of failure
*
* Sets new project configuration 
*
\*****************************************************************************/

BOOL SetConfiguration( char *pszNewConfiguration )
{
	BOOL fResult = FALSE;
	if( pszNewConfiguration && strlen( pszNewConfiguration ) )
	{
		if( !ghProjectComboBox || !IsWindow( ghProjectComboBox ) )
			FindProjectComboBox();
		if( !ghProjComboBoxList || !IsWindow( ghProjComboBoxList ) )
			FindProjComboBoxList();
		if( ghProjectComboBox && IsWindow( ghProjectComboBox ) &&
				ghProjComboBoxList && IsWindow( ghProjComboBoxList ) )
		{
			LRESULT lSelect = SendMessage( ghProjectComboBox, CB_SELECTSTRING, (WPARAM)-1, 
																			(LPARAM) pszNewConfiguration );
			SendMessage( ghProjectComboBox, WM_COMMAND, MAKEWPARAM( ID_PROJECTCOMBOLISTBOX, 
									 CBN_SELCHANGE ), (LPARAM)ghProjComboBoxList );
			if( lSelect != CB_ERR )
				fResult = TRUE;
		}
	}
	return fResult;
}

/*****************************************************************************\
* HWND FindProjComboBoxList()
*
* Parameters: None
* Return value: Handle of a listbox portion of project configutaion selection 
* combobox
*
* Enumerates windows and finds a listbox portion handle 
*
\*****************************************************************************/

HWND FindProjComboBoxList( void )
{
	EnumChildWindows( GetDesktopWindow(), (WNDENUMPROC) EnumComboCbckList, 0L );
	return ghProjComboBoxList;
}


BOOL CALLBACK EnumComboCbckList( HWND hWnd, LPARAM lParam )
{
	char lpszWindowClass[CLASSNAME_LENGTH];
	long lId = 0;
	GetClassName( hWnd, lpszWindowClass, sizeof( lpszWindowClass ) );
	lId = GetWindowLong( hWnd, GWL_ID );

	if ( (!strcmp( lpszWindowClass, "ComboLBox" )) && (lId == ID_PROJECTCOMBOLISTBOX )
		&& ( GetWindowThreadProcessId( hWnd, NULL ) == GetCurrentThreadId() ) )
	{
		ghProjComboBoxList = hWnd;
		return FALSE;
	}
	return TRUE;
}
