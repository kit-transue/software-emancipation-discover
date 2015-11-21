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


static BOOL	fDisableScroll = TRUE; 




extern HFILE hDebug; //temporary

/*****************************************************************************\
* NewOutputWindowProc
*
* Custom window procedure for a Developer Studio's output window
*
\*****************************************************************************/

LRESULT CALLBACK NewOutputWindowProc( HWND hWnd, UINT nMsgID, WPARAM wParam, LPARAM lParam)
{
	WaitForSingleObject( ghMutex, INFINITE );
	RECT rect;
	LRESULT lResult = 0;

	if( hWnd == ghOutputWnd )
	{
		switch( nMsgID )
		{
		case WM_SIZE: //size our window
			if( ghDiscoverWindow && IsWindow( ghDiscoverWindow ) )
			{
				GetClientRect( ghOutputWnd, &rect );
				MoveWindow( ghDiscoverWindow, 0, 0, rect.right, rect.bottom, TRUE );
			}
			break;
		case WM_SETFOCUS:
			if( fDisableScroll && ghDiscoverWindow && IsWindow( ghDiscoverWindow ) )
			{
				SetFocus( ghDiscoverWindow );
				ReleaseMutex( ghMutex );
				return 0;	//don't let default window procedure process this message
			}
			break;
		
		}
	}

	if( gpOldOutputWndProc )
		lResult = CallWindowProc( (FARPROC)gpOldOutputWndProc, ghOutputWnd, nMsgID, wParam, lParam );
	
	ReleaseMutex( ghMutex );
	return lResult;
}


/*****************************************************************************\
* NewOutputParentWindowProc
*
* Custom window procedure for a window which owns a horisontal tab control
* an a pseudo-tabcontrol
*
\*****************************************************************************/
LRESULT CALLBACK NewOutputParentWindowProc( HWND hWnd, UINT nMsgID, WPARAM wParam, LPARAM lParam)
{
	WaitForSingleObject( ghMutex, INFINITE );
	LRESULT lResult = 0;
	
	if( hWnd == ghOutputParentWnd )
	{
		switch( nMsgID )
		{
		case WM_HSCROLL: case WM_VSCROLL:
			if( fDisableScroll )
			{
				ReleaseMutex( ghMutex );
				return 0;	//don't let default window procedure process this message
			}
			break;
		// The following message is used to trap a selection of a different tab
		// in output window
		case WM_SETTEXT:
			if( ghDiscoverWindow && IsWindow( ghDiscoverWindow ) )
			{
				if( !strcmp( (LPSTR)lParam, DISCOVER_WINDOW_NAME ) )
				{
					ShowWindow( ghDiscoverWindow, SW_SHOWNORMAL );
					SetFocus( ghDiscoverWindow );
					fDisableScroll = TRUE; // our window is active, we have to disable scrolling
				}
				else
				{
					ShowWindow( ghDiscoverWindow, SW_HIDE );
					SetFocus( ghOutputWnd );
					fDisableScroll = FALSE;
				}
			}
		}
	}
	if( gpOldParentWndProc )
		lResult = CallWindowProc( (FARPROC)gpOldParentWndProc, ghOutputParentWnd, nMsgID, wParam, lParam );
	ReleaseMutex( ghMutex );
	return lResult;
}


/*****************************************************************************\
* NewDeveloperStudioProc
*
* Custom window procedure for a Developer Studio's main window
*
\*****************************************************************************/

LRESULT CALLBACK NewDeveloperStudioProc( HWND hWnd, UINT nMessage, 
																				 WPARAM wParam, LPARAM lParam)
{
	
	WaitForSingleObject( ghMutex, INFINITE );
	LRESULT lResult = 0;	

	if( hWnd == ghDeveloperStudioWnd )
	{
		switch( nMessage )
		{
		case WM_CLOSE: case WM_DESTROY:
			// Developer Studio is closing
			fClosing = TRUE;
			// If there is no our window covering an output window,
			// we have to send unload signal from here
			
			if( (!ghDiscoverWindow) || (!IsWindow( ghDiscoverWindow))  )
				SignalFinish();
			break;
		case WM_COMMAND:
			{
				int nId = LOWORD( wParam );
				if( (nId == gnSaveId) || (nId == gnCompileId)  )
				{
					if( ghWorkerWindow )
						PostMessage( ghWorkerWindow, WM_USERSAVETRAPED, 0L, 0L );
				}
				else if ( (nId == gnSaveAllId) || (nId == gnBuildId) || 
					      (nId == gnRebuildAllId) || (nId == gnBatchBuildId) )
				{
					if( ghWorkerWindow )
						PostMessage( ghWorkerWindow, WM_USERSAVEALLTRAPED, 0L, 0L );
				}
			}
		}
	}
	
	if( gpOldDeveloperStudioProc )
		lResult = CallWindowProc( (FARPROC)gpOldDeveloperStudioProc, ghDeveloperStudioWnd, 
													nMessage, wParam, lParam );
	ReleaseMutex( ghMutex );
	return lResult;

}



TBBUTTON tbButtons[] = { 
    { 0, ID_QUIT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0L, 0},
		{ 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0L, 0},
		{ 1, ID_CONFCHANGE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0L, 0},
    { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0L, 0},
    { 2, ID_ACTIVBROWSER, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 
      0L, 0},
    { 3, ID_IMPACTANALYSIS, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0L, 0} };


/*****************************************************************************\
* DiscWndProc
*
* Window procedure for our window that covers an output window
*
\*****************************************************************************/

LRESULT CALLBACK DiscWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	LPTOOLTIPTEXT  lpttt;
	RECT rect;

	switch (message) 
	{
    
		case WM_DESTROY:
			if( fClosing )
			{
				SignalFinish(); // Signal unload only if Developer Studio is closing
												// or user has selected to terminate integration
			}

			else
			{
				CleanUp( FALSE );	// restore all but main window's window procedure
				ghOutputWnd = NULL;      
				ghOutputParentWnd = NULL;
				ghDiscoverWindow = NULL;
				// set a hook to trap a possible recreation of output window
				hWaitHook = SetWindowsHookEx( WH_CALLWNDPROC, 
														(HOOKPROC)WndMsgHookProc, ghInstance, GetCurrentThreadId() );
			}
			break;
		case WM_CREATE:
			ghToolbar = CreateTheToolbar( hWnd );
			break;
		case WM_MOVE:
				SetWindowPos( hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOSIZE );
			break;
		case WM_SIZE:
				GetWindowRect( hWnd, &rect );
				// Size a toolbar
				SetWindowPos( ghToolbar, NULL, 0, 0,
								rect.right - rect.left, 20, SWP_NOZORDER | SWP_NOMOVE);
			break;
		case WM_WINDOWPOSCHANGED:
			if( !ghToolTips || !IsWindow( ghToolTips ) )
			{
				// Destroy and recreate toolbar window if tooltips were destroyed
				// while docking
				DestroyWindow( ghToolbar );
				ghToolbar = CreateTheToolbar( hWnd );
			}
			break;
		case WM_PAINT:
			// Make sure that caret is not visible
			{
				SetCaretPos( 5000, 5000 );
				PAINTSTRUCT paintstruct;
				HDC dc = BeginPaint( hWnd, &paintstruct );
				if( dc )
				{
					TextOut( dc, 10, 40, pszStatus, strlen( pszStatus ) );
					EndPaint( hWnd, &paintstruct );
				}
				if( ghToolbar && IsWindow( ghToolbar ) ) // we have to redraw tolbar window manually
																								 // because it doesn't paint itself correctly
																								 // after a screen saver
				{
					RedrawWindow( ghToolbar, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE );
				}
			}
			break;
		case WM_COMMAND:
			{
				UINT nID = LOWORD( wParam );
				switch ( nID )
				{
				case ID_QUIT:
					{
						if ( DebugAction() )
							break;

					fClosing = TRUE;
					DestroyWindow( hWnd );
					break;
					}
				case ID_CONFCHANGE:
					{
						int nId = GetMenuIdByName( ghDeveloperStudioWnd, "&Build", 
																			"Set Default Con&figuration..." );
						if( nId )
							SendMessage( ghDeveloperStudioWnd, WM_COMMAND, MAKEWPARAM( nId, 0 ), 0 );
						GetConfiguration();
					}
					break;
				case ID_ACTIVBROWSER:
          // Send a command to a Browser asking it to activate itself
          GetDDEClient()->CommandToServer( IDE_CMD_ACTIVATEBROWSER, "<-IDESpy" );
					break;

        case ID_IMPACTANALYSIS:
          GetDDEClient()->CommandToServer( IDE_CMD_IMPACT_ANALYSIS, "<-IDESpy" );
          break;
				default:
					break;
				}
			}
			break;
		case WM_RBUTTONDOWN:
			{
				POINT ClickPoint;
				HMENU hTrackMenu;

				ClickPoint.x = LOWORD( lParam );
				ClickPoint.y = HIWORD( lParam );
				ClientToScreen( hWnd, &ClickPoint );
				HMENU hTopMenu = LoadMenu( ghInstance, 
									MAKEINTRESOURCE( IDR_TRACKMENU ) );
				if( hTopMenu )
				{
					hTrackMenu = GetSubMenu( hTopMenu, 0 );
					if( hTrackMenu )
					{
						TrackPopupMenu( hTrackMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_LEFTBUTTON,
								ClickPoint.x, ClickPoint.y, 0, hWnd, NULL);
						DestroyMenu( hTrackMenu );
					}
					DestroyMenu( hTopMenu );
				}
			}
			break;
		case WM_MOUSEACTIVATE:
			return 0;
			break;
		case WM_NOTIFY:
      {
				lpttt = (LPTOOLTIPTEXT)lParam;
      
				switch (lpttt->hdr.code)
         {
         case TTN_NEEDTEXT:
            {
							SetWindowPos( lpttt->hdr.hwndFrom, HWND_TOP, 0, 0, 0, 0, 
									 SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE );
							// Set tooltip text
							char buffer[MAX_TOOLTIP_TEXT];
							if( LoadString( ghInstance, lpttt->hdr.idFrom, buffer, MAX_TOOLTIP_TEXT ) )
								lstrcpy( lpttt->szText, buffer );
            }
            break;
         }
      }
      break;
		default:
			break;
	}

	LRESULT lResult = DefWindowProc(hWnd, message, wParam, lParam);
	return lResult;
}


/*****************************************************************************\
* WorkerWndProc
*
* Window procedure for our window that covers an output window
*
\*****************************************************************************/

LRESULT CALLBACK WorkerWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) 
	{
		case WM_USERSHIFTDBLCLK:
			{
				ChooseMyMenu( ghDeveloperStudioWnd, 0, "&File", "Save Al&l" );
				DDEIntegratorClient *pClient = GetDDEClient();
				if( pClient )
				{
					GetConfiguration();
					pClient->CommandToServer( IDE_MSG_DBLCKICKED, gpszConfiguration );
				}
			}
			break;
		case WM_USERSAVETRAPED:
			//MessageBox( NULL, "Save was chosen", "", MB_OK );
			if( ghMDIClientWnd && IsWindow( ghMDIClientWnd ) )
			{
				HWND hActive = (HWND) SendMessage( ghMDIClientWnd, WM_MDIGETACTIVE, 0L, 0L );
				if( hActive && IsWindow( hActive ) && GetWindowTextLength( hActive ) )
				{
					char pszBuff[ MAX_PATH ];
					char drive[_MAX_DRIVE];
					char dir[_MAX_DIR];
					char fname[_MAX_FNAME];
					char ext[_MAX_EXT];

					GetWindowText( hActive, pszBuff, MAX_PATH );
					_splitpath( pszBuff, drive, dir, fname, ext );
					wsprintf( pszBuff, "%s%s", fname, ext );
					MessageBox( NULL, pszBuff, "Active file:", MB_OK );
				}
			}
			break;
		case WM_USERSAVEALLTRAPED:
			//MessageBox( NULL, "Save All was chosen", "", MB_OK );
			break;
		case WM_USERTERMINATE:
				Terminate();
			break;

		}

		LRESULT lResult = DefWindowProc(hWnd, message, wParam, lParam);
		return lResult;
}


/*****************************************************************************\
* WndMsgHookProc
*
* WH_CALLWNDPROC hook procedure
*	If WM_CREATE message is received, this procedure checks if output window
* was recreated and sets ghOutputWnd and ghOutputParentWnd global handles in
* case of success
*
\*****************************************************************************/

LRESULT CALLBACK WndMsgHookProc( INT nCode, WPARAM wParam, LPARAM lParam )
{
	//WaitForSingleObject( ghMutex, INFINITE );
	char buffer1[100], buffer2[100];
	static HWND hOutputParentCandidate = NULL;
	static HWND hCommonParent = NULL;
	CWPSTRUCT *pMsg = (CWPSTRUCT *)lParam;

	if( pMsg->message == WM_CREATE )
	{
		GetClassName( pMsg->hwnd, buffer1, sizeof(buffer1) -1 );
		HWND hParent = GetParent( pMsg->hwnd );
		if( hParent )
			GetClassName( hParent, buffer2, sizeof(buffer2) -1 );
		else
			buffer2[0] = '\0';

		if( (!_stricmp(buffer1, TABBED_WINDOW_CLASS)) && (!_stricmp(buffer2, GENERIC_AFX_CLASS)) )
		{
			hOutputParentCandidate = pMsg->hwnd;
			hCommonParent = hParent;
		}
		else if( hOutputParentCandidate )
		{
			if( (!_stricmp(buffer1, OUTPUT_WINDOW_CLASS)) && (!_stricmp(buffer2, GENERIC_AFX_CLASS)) 
				   && (hParent == hCommonParent) )
			{
				ghOutputWnd = pMsg->hwnd;      
				ghOutputParentWnd = hOutputParentCandidate;
				UnhookWindowsHookEx( hWaitHook );
				hWaitHook = NULL;
				if ( InitSubclass( FALSE ) )
				{
					ghDiscoverWindow = CreateDiscoverWindow( ghOutputWnd );
					if( !ghDiscoverWindow )
					{
						SignalFinish();
					}
					else
						SetFocus( ghDiscoverWindow ); 
				}
				else
					SignalFinish();
			}
	
		}
	}

	CallNextHookEx( hWaitHook, nCode, wParam, lParam );
	//ReleaseMutex( ghMutex );
	return 0;
}


/*****************************************************************************\
* CreateTheToolbar( HWND )
*
* Parameters: Handle of a parent window
*	Return value: Handle of newly created toolbar
*
\*****************************************************************************/

HWND CreateTheToolbar( HWND hParent )
{
	
	// Create a toolbar with TBSTYLE_TOOLTIPS style which causes tooltip
	// control to be created
	
	HWND hBar = NULL;
	if( hParent && IsWindow( hParent ) )
	{
		hBar = CreateToolbarEx(hParent,
        WS_CHILD | WS_BORDER | WS_VISIBLE 
        |TBSTYLE_TOOLTIPS, 200, 4, (HINSTANCE) GetModuleHandle(DLL_MODULE_NAME), 
        IDR_TOOLBAR1, (LPCTBBUTTON)&tbButtons,
        6, 20, 20, 20, 20, sizeof(TBBUTTON));
		if( hBar ) //get a handle of Tooltip Control
			ghToolTips = (HWND) SendMessage( hBar, TB_GETTOOLTIPS, 0L, 0L );
	}
	return hBar;
}


//-------------------------------------------
#include <stdio.h>
void IDESpyDebug()
{
	_asm int 3;
}

BOOL DebugAction()
{
	SHORT fKeyLC = GetKeyState(VK_LCONTROL),
				fKeyRC = GetKeyState(VK_RCONTROL),
				fKeyS  = GetKeyState(VK_SHIFT),
				fKeyA  = GetKeyState(VK_MENU);
	if ( fKeyS < 0 )
		{
		GetDDEClient()->CommandToServer( IDE_CMD_SHOW_INTEGRATOR, "<-IDESpy" );
		return TRUE;
		}
	if ( fKeyLC < 0 )
		{
    char szBuffer[_MAX_PATH+12];
    int  nCommandLen;
    strcpy( szBuffer, "notepad.exe ");
    nCommandLen = strlen(szBuffer);
    GetDisRegistryStr(REG_DISCOVER_LOG, &(szBuffer[nCommandLen]),
                    sizeof(szBuffer)-nCommandLen, " ");
    WinExec( szBuffer, SW_MAXIMIZE );
		return TRUE;
		}
	if ( fKeyRC < 0 )
		{
    char	szLogFileName[_MAX_PATH];
    GetDisRegistryStr(REG_DISCOVER_LOG, szLogFileName, sizeof(szLogFileName), "c:\\Discover.log");
    FILE*	fileLog = fopen( szLogFileName, "w");
	  if ( fileLog )
		  {
		  char		szTimeBuff[40], szDateBuff[40];

      _tzset();
      _strtime( szTimeBuff );
      _strdate( szDateBuff );

		  fprintf( fileLog, "%s %s -----------------Log cleared\n", szDateBuff, szTimeBuff );
		  fclose( fileLog );
		  }
		return TRUE;
		}
	if ( fKeyA < 0 )
		{
		_asm int 3;
		return TRUE;
		}
	return FALSE;
}
