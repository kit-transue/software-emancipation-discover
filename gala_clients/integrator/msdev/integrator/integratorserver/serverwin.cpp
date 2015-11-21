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
#include "server.h"
#include <stdio.h>

/***************************************************************************
	Global Variables
****************************************************************************/

static	HWND				ghMainWnd;
static	HINSTANCE		ghInstance;
static	const char	gszTitle[]	= INTEGRATOR_SERVERNAME;
static	char*				gpszCmdLine = NULL;
static	HWND				ghIDEWnd		= NULL;
static	BOOL				gbFinish		= FALSE;

HINSTANCE	GetServerHInstance() { return ghInstance; }
HWND			GetIntegratorMainWnd()	{	return ghMainWnd; }

static UINT gnDebugErrLevel = 
#ifdef _DEBUG
	3;
#else
	0;
#endif

/***************************************************************************
	Windows Procedures
****************************************************************************/


extern "C" int CALLBACK WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{

	MSG		msg;

	if ( lpCmdLine )					// CmdLine = "IDEConsole <IDE> <ProjectName>"
	{
		gpszCmdLine = strchr( lpCmdLine, ' ');
		if ( gpszCmdLine )			// gpszCmdLine = "<IDE> <ProjectName>"
				ghIDEWnd =	GetIDEInfoList()->FindMainAppWindow(++gpszCmdLine);
	}

	if (!InitApplication(hInstance)) { // Initialize shared things
		return (FALSE);     // Exits if unable to initialize
        }

	ServerDebugMessageLog( MSG_DBG, INTEGRATOR_SERVERNAME,  "----Started----", lpCmdLine );
	if ( !InitTclSocket() )
		return FALSE;

	ghInstance= hInstance;
	ghMainWnd = CreateWindow( 
				gszTitle , gszTitle,
				WS_CAPTION  | WS_OVERLAPPED,
				0, 0, 200, 50,
				0, 0, ghInstance, NULL);
	if (!ghMainWnd)
		return FALSE;

	if ( !DDEServerStart() )
		{
			ServerDebugMessageLog( 0, INTEGRATOR_SERVERNAME,  "DDE Start", "Error!" );
			return FALSE;
		}

	if ( GetDisRegistryInt(REG_DISCOVER_WND_HIDE, 1) )
		ShowWindow(ghMainWnd, SW_HIDE);
	else
		ShowWindow(ghMainWnd, SW_SHOW);
	while (GetMessage(&msg, 0, 0, 0))
	{
	 TranslateMessage(&msg);
	 DispatchMessage (&msg);
	}

	ServerDebugMessageLog( MSG_DBG, INTEGRATOR_SERVERNAME,  "----Finished---" );
	return msg.wParam;
}


/****************************************************************************

        FUNCTION: InitApplication(HINSTANCE)

        PURPOSE: Initializes window data and registers window class

        COMMENTS:

                This function is called at initialization time only if no other
                instances of the application are running.  This function performs
                initialization tasks that can be done once for any number of running
                instances.

                In this case, we initialize a window class by filling out a data
                structure of type WNDCLASS and calling the Windows RegisterClass()
                function.  Since all instances of this application use the same window
                class, we only need to do this when the first instance is initialized.


****************************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
		if ( !DDEServerCheckInstance() )
			return FALSE;

    WNDCLASS  wc;

        // Fill in window class structure with parameters that describe the
        // main window.

        wc.style         = CS_HREDRAW | CS_VREDRAW;// Class style(s).
        wc.lpfnWndProc   = (WNDPROC)WndProc;       // Window Procedure
        wc.cbClsExtra    = 0;                      // No per-class extra data.
        wc.cbWndExtra    = 0;                      // No per-window extra data.
        wc.hInstance     = hInstance;              // Owner of this class
        wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_SERVER)); // Icon name from .RC
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);	// Cursor
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);	// Default color
        wc.lpszMenuName  = MAKEINTRESOURCE(IDM_SERVER); // Menu from .RC
        wc.lpszClassName = gszTitle;					// Name to register as

        // Register the window class and return success/failure code.
        return (RegisterClass(&wc));
}


/****************************************************************************
	WndProc
****************************************************************************/

LRESULT CALLBACK
WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		{
			ghMainWnd = hwnd;
			ConnectToOutSocket();
      if ( ghIDEWnd )
        GetIDEInfoList()->CreateIDEInfoObject( ghIDEWnd, gpszCmdLine);
			return 0;

		}
	case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDM_EXIT:
				{
					TerminateServer( EXTMSG_DIS_FINISHED );
					return 0;
				}

				case IDM_DEBUG_VIEWMESSAGES:
				{
					gnDebugErrLevel = 10;
					return 0;
				}

        case ID_DEBUG_SERVER_ADAPTER:
				{
					DebugServerAdapter();
					return 0;
				}

        case ID_DEBUG_SERVER_DDE:
				{
					DebugServerDDE();
					return 0;
				}

				case ID_DEBUG_SERVER_IDEINFO:
				{
					DebugServerIDEInfo();
					return 0;
				}

				case ID_DEBUG_SERVER_QUERY:
				{
					DebugServerQuery();
					return 0;
				}

				case IDM_DEBUG_SERVER_WIN:
				{
					DebugServerWin();
					return 0;
				}

				case IDM_DEBUG_IDESPY:
				{
					IDEInfo*	pIDEInfo = GetIDEInfoList()->GetFirstIDEInfo();
					if ( pIDEInfo )
					{
						DDEIntegratorClient* pClient= pIDEInfo->GetIDESpyClient();
						if ( pClient && pClient->IsConnected() )
							pClient->CommandToServer(IDE_CMD_DEBUG, "Menu" );
					}
					return 0;
				}

				case IDM_LOG_VIEW:
				{
        	char szBuffer[_MAX_PATH+12];
          int  nCommandLen;
          strcpy( szBuffer, "notepad.exe ");
          nCommandLen = strlen(szBuffer);
          GetDisRegistryStr(REG_DISCOVER_LOG, &(szBuffer[nCommandLen]),
                          sizeof(szBuffer)-nCommandLen, " ");
          WinExec( szBuffer, SW_MAXIMIZE );
					return 0;
				}

				case IDM_LOG_CLEAR:
				{
        	char	szLogFileName[_MAX_PATH];
        	GetDisRegistryStr(REG_DISCOVER_LOG, szLogFileName, sizeof(szLogFileName), "c:\\Discover.log");
        	FILE*	fileLog = fopen( szLogFileName, "w");
	        if ( fileLog )
		        {
		        char		szTimeBuff[40], szDateBuff[40];

//            _tzset();
            _strtime( szTimeBuff );
            _strdate( szDateBuff );

		        fprintf( fileLog, "%s %s -----------------Log cleared\n", szDateBuff, szTimeBuff );
		        fclose( fileLog );
		        }

					return 0;
				}

				default:
					return 0;
		}
	}
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

  case WM_CREATE_NEW_IDEINFO:
    {
      char* pszProjName = (char*)lParam;
      if ( wParam && pszProjName && *pszProjName )
        GetIDEInfoList()->CreateIDEInfoObject( (HWND)wParam, pszProjName);
			return 0;
    }

	case WM_IDESPYFINISH:
		{
			DDEIntegratorClient*	pClient = (DDEIntegratorClient*)lParam;
			if ( pClient && pClient->IsConnected() )
					pClient->CommandToServer(IDE_CMD_DDECLIENT_DISCONNECT, NULL);
			return 0;
		}

	case WM_IDESERVERFINISH:
		{
			if ( gbFinish && GetIDEInfoList()->GetFirstIDEInfo() == NULL )
			{
				DDEServerFinish();
				DestroyWindow(hwnd);
			}
			return 0;
		}

	case WM_IDECOMMAND:
		{
			IDECommand*	pIDECmd = (IDECommand*)lParam;
			if ( wParam )
      {
      	IDEInfo* pIDEInfo = GetIDEInfoList()->FindIDEInfoByMainWnd( (HWND)wParam );
        if ( pIDEInfo )
				  pIDEInfo->SignalToAllQuery(pIDECmd);
      }
			else
				GetIDEInfoList()	->SignalToAllQuery(pIDECmd);
			
			if ( pIDECmd->pszParams && pIDECmd->nParamsLen > 0 )
					delete [] (pIDECmd->pszParams);
			delete pIDECmd;
			return 0;
		}

	case WM_LOOKNEWQUERY:
		{
			Node* pNodeFirst = GetQueueList()->GetEntryPoint();
			if ( pNodeFirst == NULL )
				return 0;
			
			Node* pNode = pNodeFirst;
			do {
				DI_QueryGeneric*	pQuery		= (DI_QueryGeneric*)(pNode->GetData());
				IDEInfo*					pIDEInfo	= GetIDEInfoList()->FindIDEInfoByMainWnd( pQuery->GetMainIDEWnd() );
				if ( pIDEInfo == NULL || !IsQueryIdCorrect(pQuery->GetQueryId()) )
				{
          ServerDebugMessageLog( 4, "ServerWin", "WM_LOOKNEWQUERY", "Clear Error Query" );
					GetQueueList()->RemoveNode( pNode );
					delete pQuery;
					if ( (pNode= pNodeFirst= GetQueueList()->GetEntryPoint()) == NULL )
						return 0;
					else
						continue;
				}
				if ( pIDEInfo->ResetAndStartQuery(pNode) )
					return 0;

        pNode= pNode->GetNextClockWise();
			}
			while ( pNode != pNodeFirst ); 
			return 0;
	}

  case WSA_CONNECT:
    {
      ConnectToInpSocket();
      return 0;
    }

  case WSA_READ: 
	 	{
			InpSocketRead(wParam, lParam);
			return 0;
   	}

	case WSA_WRITE: 
	 	{
			OutSocketRead(wParam, lParam);
			return 0;
   	}

	default:
		break;
	}
return DefWindowProc(hwnd, msg, wParam, lParam);
}


/****************************************************************************
		 Terminate
****************************************************************************/

void	TerminateServer( char*	szPostText )
{

	gbFinish = TRUE;
	CloseTclSocket();

	Node* pNodeFirst = GetQueueList()->GetEntryPoint();
	if ( pNodeFirst )
		{
		ServerDebugMessageLog( 4, "ServerWin", "Terminate", "Clear All Query" );
		Node* pNode = pNodeFirst;
		do {
				DI_QueryGeneric*	pQuery		= (DI_QueryGeneric*)(pNode->GetData());
				GetQueueList()->RemoveNode( pNode );
				delete pQuery;
				if ( (pNode= pNodeFirst= GetQueueList()->GetEntryPoint()) == NULL )
					break;
			}
		while ( pNode != pNodeFirst );
	 }

	DDEIntegratorClient*	pClient;
	IDEInfo*	pIDEInfo = GetIDEInfoList()->GetFirstIDEInfo();
	IDEInfo*	pIDEInfoNext;
	HWND			hWnd = GetIntegratorMainWnd();

	if ( pIDEInfo == NULL )
		 PostMessage( hWnd, WM_IDESERVERFINISH, NULL, NULL);
	else
		while ( pIDEInfo )
		{
      pIDEInfoNext = pIDEInfo->GetNextIDE();
			pClient= pIDEInfo->GetIDESpyClient();
			if ( pClient && pClient->IsConnected() )
				PostMessage( hWnd, WM_IDESPYFINISH, NULL, (LPARAM)pClient );
			else
				GetIDEInfoList()->DestroyIDEInfoObject(pIDEInfo);

			pIDEInfo = pIDEInfoNext;
		}

	if ( !GetIDEInfoList() )
		 PostMessage( hWnd, WM_IDESERVERFINISH, NULL, NULL);
}



BOOL MakeDoubleClick( HWND hWnd, UINT nMilliSecsTimeout, UINT nXpos, UINT nYpos )
{
	if( !hWnd || !IsWindow( hWnd ) )
		return FALSE;

	Sleep( nMilliSecsTimeout );
	LPARAM lParam = MAKELPARAM (nXpos, nYpos);
	SendMessage( hWnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
	SendMessage( hWnd, WM_LBUTTONUP, MK_LBUTTON, lParam);
	SendMessage( hWnd, WM_LBUTTONDBLCLK, MK_LBUTTON, lParam);
	return TRUE;
}


void  DebugServerWin  ()
{
  _asm int 3;
}

