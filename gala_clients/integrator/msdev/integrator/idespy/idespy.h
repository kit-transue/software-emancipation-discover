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
#define DLL_MODULE_NAME				"idespy"
#define DISCOVER_WINDOW_NAME	"Discover"
#define WM_USERSHIFTDBLCLK		WM_USER + 145
#define	WM_USERSAVETRAPED			WM_USER + 146
#define	WM_USERSAVEALLTRAPED	WM_USER + 147
#define WM_USERTERMINATE			WM_USER + 148
#define ID_PROJECTTOOLBAR			33284
#define ID_PROJECTCOMBOBOX		0x8521
#define	ID_PROJECTCOMBOLISTBOX 0x3e8
#define TITLE_LENGTH					100
#define	CLASSNAME_LENGTH			100
#define	ATTACH_PROCESS_NAME		"MSDEV"
#define	MAIN_WINDOW_TITLE			"Microsoft Developer Studio"
#define	TABBED_WINDOW_CLASS		"Afx:400000:0"
#define	OUTPUT_WINDOW_CLASS		"Afx:400000:8"
#define GENERIC_AFX_CLASS			"AfxWnd40"
#define MAX_TOOLTIP_TEXT			50

#define DISC_DRAW_LEFT		0
#define DISC_DRAW_TOP			35
#define DISC_DRAW_RIGHT		1200
#define DISC_DRAW_BOTTOM	100

extern HWND		ghDeveloperStudioWnd;
extern HWND		ghOutputWnd;
extern HWND		ghOutputParentWnd;
extern HWND		ghDiscoverWindow;
extern HWND		ghToolbar;
extern HWND		ghToolTips;
extern HWND		ghMDIClientWnd;
extern HWND		ghWorkerWindow;
extern HWND		ghProjectComboBox;
extern HINSTANCE	ghInstance;
extern WNDPROC		gpOldOutputWndProc;
extern WNDPROC		gpOldParentWndProc;
extern WNDPROC		gpOldDeveloperStudioProc;
extern DWORD		gnDDEClientId;
extern HANDLE		ghEvent;
extern HANDLE		ghMutex;
extern BOOL			fWasDestroyed;
extern BOOL			fClosing;
extern HHOOK		hWaitHook;
extern BOOL			fDisableScroll;
extern char			pszStatus[];
extern char			gpszConfiguration[];
extern int			gnSaveAllId;
extern int			gnSaveId;
extern int			gnCompileId;
extern int			gnBuildId;
extern int			gnRebuildAllId;
extern int			gnBatchBuildId;




extern "C" __declspec(dllexport) LRESULT CALLBACK GetMsgHookProc( INT, WPARAM, LPARAM );
LRESULT CALLBACK WndMsgHookProc( INT, WPARAM, LPARAM );
LRESULT CALLBACK WorkerWndProc ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK DiscWndProc	 ( HWND, UINT, WPARAM, LPARAM );


BOOL InitSubclass( BOOL fSubclass = TRUE);
HWND CreateDiscoverWindow( HWND );
HWND CreateWorkerWindow( HWND );
HWND CreateTheToolbar( HWND );
BOOL GetWindowHandles();
BOOL CleanUp( BOOL fRemove = TRUE );
BOOL SetConfiguration( char * );
LRESULT CALLBACK NewOutputWindowProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK NewOutputParentWindowProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK NewDeveloperStudioProc( HWND, UINT, WPARAM, LPARAM );
void SignalFinish();
void ChangeStatusLine( char * );
HWND FindProjectComboBox();
HWND FindProjComboBoxList();
BOOL GetConfiguration();
BOOL CALLBACK EnumComboCbck( HWND, LPARAM );
BOOL CALLBACK EnumComboCbckList( HWND, LPARAM );
void Terminate();

/***********************************************************************
	IDESpy Server & Client
************************************************************************/

DDEIntegratorClient*	GetDDEClient();
BOOL									DDEServerStart(HWND hIDEMainWnd);
void									DDEServerFinish();
void									IDESpyDebug();
BOOL									DebugAction();


BOOL CALLBACK EnumCbck( HWND, LPARAM); //callback function for EnumChildWindows
									   //Sets hDeveloperStudio

BOOL CALLBACK EnumCbck2( HWND, LPARAM); //callback function for EnumChildWindows
									   //Sets hParent

BOOL CALLBACK EnumCbck3( HWND, LPARAM); //callback function for EnumChildWindows
									   //Sets hVcOutputWindow
BOOL CALLBACK EnumCbck4( HWND, LPARAM);
