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
#include <objbase.h>
#include <assert.h>
#include "ComFactory.h"
#include "ComIface.h"
#include <stdlib.h>
#include <fcntl.h>

#define	_THIS_FILE_		"ConfigShell"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//		Main process 
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
HWND g_hWndListBox = NULL ;
BOOL g_bCustomCM = FALSE;

BOOL InitWindow(int nCmdShow) ;
extern "C" LONG APIENTRY MainWndProc(HWND hWnd,
                                     UINT message,
                                     UINT wParam,
                                     LONG lParam) ;


//--------------------------------------------------------
// WinMain procedure
//
extern "C" int WINAPI WinMain(HINSTANCE hInstance, 
                              HINSTANCE hPrevInstance,
                              LPSTR lpCmdLine, 
                              int nCmdShow)
{
	// Controls whether UI is shown or not
	BOOL bUI = TRUE ;

	// If TRUE, don't loop.
	BOOL bExit = FALSE ;

#ifdef _DEBUG
	if ( GetDisRegistryInt("Debug", "ConfigShell", 0) )
		_asm int 3;
#endif

	// Initialize the COM Library.
	HRESULT hr = CoInitialize(NULL) ;
	if (FAILED(hr))
	{
		return 0 ;
	}
   
   
	// Get Thread ID.
	CFactory::s_dwThreadID = ::GetCurrentThreadId() ;
	CFactory::s_hModule = hInstance ;

	// Read the command line.
	char szTokens[] = "-/" ;

	char* szToken = strtok(lpCmdLine, szTokens) ; 
	while (szToken != NULL)
	{
		if (_stricmp(szToken, "UnregServer") == 0)
		{
			CFactory::UnregisterAll() ;
			// We are done, so exit.
			bExit = TRUE ;
			bUI = FALSE ;
		}
		else if (_stricmp(szToken, "RegServer") == 0)
		{
			CFactory::RegisterAll() ;
			// We are done, so exit.
			bExit = TRUE ;
			bUI = FALSE ;
		}
		else if (_stricmp(szToken, "Embedding") == 0)
		{
			// Don't display a window if we are embedded.
			bUI = FALSE ;
			break ;
		}
		szToken = strtok(NULL, szTokens) ;
	}

	// If the user started us, then show UI.
	if (bUI)
	{
		if (!InitWindow(nCmdShow))
		{
			// Exit since we can't show UI.
			bExit = TRUE ;
		}
		else
		{
			::InterlockedIncrement(&CFactory::s_cServerLocks) ;
		}
	}

	if (!bExit)
	{
		// Register all of the class factories.
		CFactory::StartFactories() ;

		// Wait for shutdown.
		MSG msg ;
		while (::GetMessage(&msg, 0, 0, 0))
		{
			::DispatchMessage(&msg) ;
		}

		// Unregister the class factories.
		CFactory::StopFactories() ;
	}

	// Uninitialize the COM Library.
	CoUninitialize() ;
	return 0 ;
}


//--------------------------------------------------------
// Initialize window
//
BOOL InitWindow(int nCmdShow) 
{
	// Fill in window class structure with parameters
	// that describe the main window.
	WNDCLASS wcListview ;
	wcListview.style = 0 ;                     
	wcListview.lpfnWndProc = (WNDPROC)MainWndProc ; 
	wcListview.cbClsExtra = 0 ;              
	wcListview.cbWndExtra = 0 ;              
	wcListview.hInstance = CFactory::s_hModule ;
	wcListview.hIcon = ::LoadIcon(CFactory::s_hModule,
	                              MAKEINTRESOURCE(IDC_ICON)) ;
	wcListview.hCursor = ::LoadCursor(NULL, IDC_ARROW) ;
	wcListview.hbrBackground = (struct HBRUSH__ *)::GetStockObject(WHITE_BRUSH) ; 
	wcListview.lpszMenuName = NULL ;  
	wcListview.lpszClassName = "DiscoverServerWinClass" ;

	BOOL bResult = ::RegisterClass(&wcListview) ;
	if (!bResult)
	{
		return bResult ;
	}

	HWND hWndMain ;

	hWndMain = ::CreateWindow(
		"DiscoverServerWinClass",
		"ConfigShell Server", 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,               
		NULL,               
		CFactory::s_hModule,          
		NULL) ;

	// If window could not be created, return "failure".
	if (!hWndMain)
	{
		return FALSE ;
	}

	// Make the window visible; update its client area;
	// and return "success".
	::ShowWindow(hWndMain, nCmdShow) ;
	::UpdateWindow(hWndMain) ;
	return TRUE ;
}

//--------------------------------------------------------
// Main window procedure
//
extern "C" LONG APIENTRY MainWndProc(
	HWND hWnd,                // window handle
	UINT message,             // type of message
	UINT wParam,              // additional information
	LONG lParam)              // additional information
{
	switch (message) 
	{
	case WM_CREATE:
		{
			// Get size of main window
			CREATESTRUCT* pcs = (CREATESTRUCT*) lParam ;

			// Create a listbox for output.
			g_hWndListBox = ::CreateWindow(
				"LISTBOX",
				NULL, 
				WS_CHILD | WS_VISIBLE | LBS_USETABSTOPS
					| WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
				0, 0, pcs->cx, pcs->cy,
				hWnd,               
				NULL,               
				CFactory::s_hModule,          
				NULL) ;
			if (g_hWndListBox  == NULL)
			{
				// Listbox not created.
				::MessageBox(NULL,
				             "Listbox not created!",
				             NULL,
				             MB_OK) ;
				return -1 ;
			}
		}
		break ;

	case WM_SIZE:
		::MoveWindow(g_hWndListBox, 0, 0,
			LOWORD(lParam), HIWORD(lParam), TRUE) ;
		break;

	case WM_DESTROY:          // message: window being destroyed
		if (CFactory::CanUnloadNow() == S_OK)
		{
			// Only post the quit message, if there is
			// no one using the program.
			::PostQuitMessage(0) ;
		}
		break ;

	case WM_CLOSE:
		// Decrement the lock count.
		::InterlockedDecrement(&CFactory::s_cServerLocks) ;

		// The list box is going away.
		g_hWndListBox = NULL ;

		//Fall through 
	default:
		return (DefWindowProc(hWnd, message, wParam, lParam)) ;
	}
	return 0 ;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Interface IConfigurator
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

// Constructor
//
ConfigShell::ConfigShell(IUnknown* pUnknownOuter)
: CUnknown(pUnknownOuter)
{
	m_pConfigurator			= NULL;
	m_pConfigShellResult= NULL;
}


///////////////////////////////////////////////////////////
// Destructor
//
ConfigShell::~ConfigShell()
{
  if ( m_pConfigurator )
    delete m_pConfigurator;
  m_pConfigurator = NULL;
	if ( m_pConfigShellResult )
		delete m_pConfigShellResult;
	m_pConfigShellResult = NULL;
}

///////////////////////////////////////////////////////////
// NondelegatingQueryInterface implementation
//
HRESULT __stdcall ConfigShell::NondelegatingQueryInterface(const IID& iid,
                                                  void** ppv)
{ 	
	if (iid == IID_IConfigShell)
	{
		return FinishQI((ConfigShell*)this, ppv) ;
	}
	else if (iid == IID_IMarshal)
	{
		// We don't implement IMarshal.
		return CUnknown::NondelegatingQueryInterface(iid, ppv) ;
	}
	else
	{
		return CUnknown::NondelegatingQueryInterface(iid, ppv) ;
	}
}


///////////////////////////////////////////////////////////
// FinalRelease - called by Release before it deletes the component
//
void ConfigShell::FinalRelease()
{
	// Call base class to incremement m_cRef to prevent recusion.
	CUnknown::FinalRelease() ;

	// Counter the GetOuterUnknown()->Release in the Init method.
	GetOuterUnknown()->AddRef() ;	

}



///////////////////////////////////////////////////////////
//
// Creation function used by CFactory
//
HRESULT ConfigShell::CreateInstance(IUnknown* pUnknownOuter,
                           CUnknown** ppNewComponent)
{
	if (pUnknownOuter != NULL)
	{
		// Don't allow aggregation. Just for the heck of it.
		return CLASS_E_NOAGGREGATION ;
	}

	*ppNewComponent = new ConfigShell(pUnknownOuter);
	return S_OK ;
}


///////////////////////////////////////////////////////////
HRESULT __stdcall ConfigShell::InitCM(wchar_t** wcsCMName)
{
	HRESULT	hr = S_FALSE;
	m_pConfigurator = Configurator::Create();
	if ( m_pConfigurator )
	{
		char*	pszCMName = m_pConfigurator->GetSccName();
		if ( pszCMName && *pszCMName )
			{
				int iLength = (strlen(pszCMName) +1)*sizeof(wchar_t);
				wchar_t* wcsBuff = static_cast<wchar_t*>(::CoTaskMemAlloc(iLength)) ;
				if ( wcsBuff )
				{
					mbstowcs( wcsBuff, pszCMName, iLength );
					*wcsCMName = wcsBuff;
					hr = S_OK;
				}
				else
					hr = E_OUTOFMEMORY ;
			}
	}
	return hr;
}

///////////////////////////////////////////////////////////
HRESULT __stdcall ConfigShell::SetPSETCFG( wchar_t* wcsPSETCFG )
{
	char*		pszPSETCFG	= NULL;
	int			iLength = (wcslen(wcsPSETCFG)+ 1)*sizeof(char) ;
	if ( iLength )
		pszPSETCFG = new char [ iLength ];
	if ( pszPSETCFG == NULL )
		return E_OUTOFMEMORY ;

	wcstombs( pszPSETCFG, wcsPSETCFG, iLength );

	if ( m_pConfigurator == NULL )
	{
	  if ( stricmp(pszPSETCFG,"custom") == 0 )
	  {
		 g_bCustomCM = TRUE;
		 return S_OK;
	  }
	  else
		 return S_FALSE;
	}

	if ( m_pConfigurator->SetPSETCFG( pszPSETCFG ) )
		return S_OK ;
	else
		return E_OUTOFMEMORY ;
}

///////////////////////////////////////////////////////////
HRESULT __stdcall ConfigShell::Run( wchar_t* wcsCmd )
{
	if ( m_pConfigurator == NULL )
		return S_FALSE;

	HRESULT	hr = S_OK;
	char*		pszCmd	= NULL;
	int			iLength = (wcslen(wcsCmd)+ 1)*sizeof(char) ;
	if ( iLength )
		pszCmd = new char [ iLength ];
	if ( pszCmd == NULL )
		return E_OUTOFMEMORY ;

	wcstombs( pszCmd, wcsCmd, iLength );
	char*		pszAnswer = NULL;
	
	int err = m_pConfigurator->Run( pszCmd, &m_pConfigShellResult );
	
	if ( err )
		hr = S_FALSE;
	return hr;
}

///////////////////////////////////////////////////////////
HRESULT __stdcall ConfigShell::GetResult (wchar_t** wcsResult)
{
	if ( m_pConfigShellResult )
		{
			ConfigShellResult *pConfigShellResult = m_pConfigShellResult;
			m_pConfigShellResult = m_pConfigShellResult->GetNext();
			if ( m_pConfigShellResult )
				m_pConfigShellResult->SetPrev(NULL);
			pConfigShellResult->SetNext(NULL);

			*wcsResult = pConfigShellResult->GetWCText();
			delete pConfigShellResult;
		}
	else
		*wcsResult = NULL;
	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// ConfigShellResult
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

// Constructor
//
ConfigShellResult::ConfigShellResult(ConfigShellResult* pPrev)
{
	m_pNext		= NULL;
	m_pPrev		= pPrev;
	m_pszText = NULL;
	if ( pPrev )
	{
		ConfigShellResult *pNext = pPrev->m_pNext;
		pPrev->m_pNext = this;
		m_pNext = pNext;
	}
}


///////////////////////////////////////////////////////////
// Destructor
//
///////////////////////////////////////////////////////////
ConfigShellResult::~ConfigShellResult()
{
	if ( m_pNext )
		delete m_pNext;
	m_pNext = NULL;
	if ( m_pszText )
		delete [] m_pszText;
	m_pszText = NULL;
}

///////////////////////////////////////////////////////////
void ConfigShellResult::SetText(char* pszText)
{
	if ( m_pszText )
		delete [] m_pszText;
	m_pszText = NULL;
	if ( pszText )
		{
			if ( (m_pszText = new char [ strlen(pszText)+1 ]) )
				strcpy( m_pszText, pszText );
		}
}

///////////////////////////////////////////////////////////
wchar_t*	ConfigShellResult::GetWCText()
{
	wchar_t* wcsBuff = NULL;
	if ( m_pszText )
		{
			int iLength = (strlen(m_pszText) +1)*sizeof(wchar_t);
			wcsBuff = static_cast<wchar_t*>(::CoTaskMemAlloc(iLength)) ;
			if ( wcsBuff )
				mbstowcs( wcsBuff, m_pszText, iLength );
		}
	return wcsBuff;
}

///////////////////////////////////////////////////////////
//
// Interface IUnknown
//
///////////////////////////////////////////////////////////

long CUnknown::s_cActiveComponents = 0 ;


//--------------------------------------------------------
//
// Constructor
//
CUnknown::CUnknown(IUnknown* pUnknownOuter)
: m_cRef(1)
{
	// Set m_pUnknownOuter pointer.
	if (pUnknownOuter == NULL)
	{
		m_pUnknownOuter = reinterpret_cast<IUnknown*>
		                     (static_cast<INondelegatingUnknown*>
		                     (this)) ;  // notice cast
	}
	else
	{
		m_pUnknownOuter = pUnknownOuter ;
	}

	// Increment count of active components.
	::InterlockedIncrement(&s_cActiveComponents) ;
}

//--------------------------------------------------------
// Destructor
//
CUnknown::~CUnknown()
{
	::InterlockedDecrement(&s_cActiveComponents) ;

	// If this is an EXE server, shut it down.
	CFactory::CloseExe() ;
}

//--------------------------------------------------------
// FinalRelease - called by Release before it deletes the component
//
void CUnknown::FinalRelease()
{
	m_cRef = 1 ;
}

//--------------------------------------------------------
// Nondelegating IUnknown
//   - Override to handle custom interfaces.
//
HRESULT __stdcall 
	CUnknown::NondelegatingQueryInterface(const IID& iid, void** ppv)
{
	// CUnknown supports only IUnknown.
	if (iid == IID_IUnknown)
	{
		return FinishQI(reinterpret_cast<IUnknown*>
		                   (static_cast<INondelegatingUnknown*>(this)),
		                ppv) ;
	}	
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE ;
	}
}

//--------------------------------------------------------
// AddRef
//
ULONG __stdcall CUnknown::NondelegatingAddRef()
{
	return InterlockedIncrement(&m_cRef) ;
}

//--------------------------------------------------------
// Release
//
ULONG __stdcall CUnknown::NondelegatingRelease()
{
	InterlockedDecrement(&m_cRef) ;
	if (m_cRef == 0)
	{
		FinalRelease() ;
		delete this ;
		return 0 ;
	}
	return m_cRef ;
}

//--------------------------------------------------------
// FinishQI
//   - Helper function to simplify overriding
//     NondelegatingQueryInterface
//
HRESULT CUnknown::FinishQI(IUnknown* pI, void** ppv) 
{
	*ppv = pI ;
	pI->AddRef() ;
	return S_OK ;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Interface IClassFactory
//
///////////////////////////////////////////////////////////
//
// Static variables
//
CFactoryData g_FactoryDataArray[] =
{
	{&CLSID_ConfigShell, ConfigShell::CreateInstance, 
		"DISCOVER, IntegratorServer, Configurator",		// Friendly Name
		"DISCOVER.IntegratorServer.Configurator.1",   // ProgID
		"DISCOVER.IntegratorServer.Configurator",     // Version-independent ProgID
		NULL, 0},	
} ;
int g_cFactoryDataEntries
	= sizeof(g_FactoryDataArray) / sizeof(CFactoryData) ;


LONG		CFactory::s_cServerLocks	= 0 ;			 // Count of locks
HMODULE CFactory::s_hModule				= NULL ;   // DLL module handle
DWORD		CFactory::s_dwThreadID		= 0 ;



///////////////////////////////////////////////////////////
//
// Initialize & Uninitialize
//
void ComServerUninitialize()
{
	::InterlockedDecrement(&CFactory::s_cServerLocks) ;
	CFactory::StopFactories() ;
	CoUninitialize() ;
}

BOOL ComServerInitialize( HINSTANCE hInstance, char* lpCmdLine )
{
	BOOL bExit = FALSE ;

	HRESULT hr = CoInitialize(NULL) ;
	if (FAILED(hr))
	{
		return 0 ;
	}
   
   
	// Get Thread ID.
	CFactory::s_dwThreadID = ::GetCurrentThreadId() ;
	CFactory::s_hModule = hInstance ;

	// Read the command line.
	char szTokens[] = "-/" ;

	char* szToken = strtok(lpCmdLine, szTokens) ; 
	while (szToken != NULL)
	{
		if (_stricmp(szToken, "UnregServer") == 0)
		{
			CFactory::UnregisterAll() ;
			bExit = TRUE ;
		}
		else if (_stricmp(szToken, "RegServer") == 0)
		{
			CFactory::RegisterAll() ;
			bExit = TRUE ;
		}
		szToken = strtok(NULL, szTokens) ;
	}

	if (!bExit)
	{
		// Register all of the class factories.
		::InterlockedIncrement(&CFactory::s_cServerLocks) ;
		CFactory::StartFactories() ;
	}
	else
		CoUninitialize() ;
	
	return !bExit;
}


///////////////////////////////////////////////////////////
//
// CFactory implementation
//

CFactory::CFactory(const CFactoryData* pFactoryData)
: m_cRef(1)
{
	m_pFactoryData = pFactoryData ;
}

//
// IUnknown implementation
//
HRESULT __stdcall CFactory::QueryInterface(REFIID iid, void** ppv)
{ 	
	IUnknown* pI ;
	if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
	{
		pI = this ; 
	}
	else
	{
	   *ppv = NULL;
		return E_NOINTERFACE;
	}
	pI->AddRef() ;
	*ppv = pI ;
	return S_OK;
}

ULONG __stdcall CFactory::AddRef() 
{ 
	return ::InterlockedIncrement(&m_cRef) ; 
}

ULONG __stdcall CFactory::Release() 
{
	if (::InterlockedDecrement(&m_cRef) == 0) 
	{
		delete this; 
		return 0 ;
	}   
	return m_cRef;
}

//
// IClassFactory implementation
//

HRESULT __stdcall CFactory::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv) 
{

	// Aggregate only if the requested IID is IID_IUnknown.
	if ((pUnknownOuter != NULL) && (iid != IID_IUnknown))
	{
		return CLASS_E_NOAGGREGATION ;
	}

	// Create the component.
	CUnknown* pNewComponent ;
	HRESULT hr = m_pFactoryData->CreateInstance(pUnknownOuter,
	                                            &pNewComponent) ;
	if (FAILED(hr))
	{
		return hr ;
	}

	// Initialize the component.
	hr = pNewComponent->Init();
	if (FAILED(hr))
	{
		// Initialization failed.  Release the component.
		pNewComponent->NondelegatingRelease() ;
		return hr ;
	}
	
	// Get the requested interface.
	hr = pNewComponent->NondelegatingQueryInterface(iid, ppv) ;

	// Release the reference held by the class factory.
	pNewComponent->NondelegatingRelease() ;
	return hr ;   
}

// LockServer
HRESULT __stdcall CFactory::LockServer(BOOL bLock) 
{
	if (bLock) 
	{
		::InterlockedIncrement(&s_cServerLocks) ; 
	}
	else
	{
		::InterlockedDecrement(&s_cServerLocks) ;
	}
	// If this is an out-of-proc server, check to see
	// whether we should shut down.
	CloseExe() ;  //@local

	return S_OK ;
}


///////////////////////////////////////////////////////////
//
// GetClassObject
//   - Create a class factory based on a CLSID.
//
HRESULT CFactory::GetClassObject(const CLSID& clsid, 
                                 const IID& iid, 
                                 void** ppv)
{
	if ((iid != IID_IUnknown) && (iid != IID_IClassFactory))
	{
		return E_NOINTERFACE ;
	}

	// Traverse the array of data looking for this class ID.
	for (int i = 0; i < g_cFactoryDataEntries; i++)
	{
		const CFactoryData* pData = &g_FactoryDataArray[i] ;
		if (pData->IsClassID(clsid))
		{

			// Found the ClassID in the array of components we can
			// create. So create a class factory for this component.
			// Pass the CFactoryData structure to the class factory
			// so that it knows what kind of components to create.
			*ppv = (IUnknown*) new CFactory(pData) ;
			if (*ppv == NULL)
			{
				return E_OUTOFMEMORY ;
			}
			return NOERROR ;
		}
	}
	return CLASS_E_CLASSNOTAVAILABLE ;
}

//
// Determine if the component can be unloaded.
//
HRESULT CFactory::CanUnloadNow()
{
	if (CUnknown::ActiveComponents() || IsLocked())
	{
		return S_FALSE ;
	}
	else
	{
		return S_OK ;
	}
}

//
// Register all components.
//
HRESULT CFactory::RegisterAll()
{
	for(int i = 0 ; i < g_cFactoryDataEntries ; i++)
	{
		RegisterServer(s_hModule,
		               *(g_FactoryDataArray[i].m_pCLSID),
		               g_FactoryDataArray[i].m_RegistryName,
		               g_FactoryDataArray[i].m_szVerIndProgID, 
		               g_FactoryDataArray[i].m_szProgID) ;
	}

  char szCmd[ 1024 ] = "regsvr32 /s ";
	DWORD dwResult =
		::GetModuleFileName(s_hModule, 
		                    szCmd + strlen(szCmd), sizeof(szCmd) - strlen(szCmd) );
	char *p = strrchr(szCmd, '\\' );
	*(p+1) = 0;
	strcat( szCmd, "ConfigShellProxy.dll" );

  HANDLE hScriptStdoutRd, hScriptStdoutWr;
	SECURITY_ATTRIBUTES sa;
  ZeroMemory( &sa, sizeof(sa) );
  sa.bInheritHandle = TRUE;
  if( CreatePipe(&hScriptStdoutRd, &hScriptStdoutWr, &sa, 0) )
  {
    FILE *pfOut = NULL;
    int nOut = _open_osfhandle( (long)hScriptStdoutRd, _O_TEXT );
    if( nOut != -1 )
      pfOut = _fdopen( nOut, "r" );
    if( pfOut )
    {
      PROCESS_INFORMATION pi;
      STARTUPINFO si;
      ZeroMemory( &pi, sizeof( pi ) );
      ZeroMemory( &si, sizeof( si ) );
      si.cb = sizeof( si );
      si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;
      si.hStdInput = NULL;
      si.hStdOutput = hScriptStdoutWr;
      si.hStdError = GetStdHandle( STD_ERROR_HANDLE );
      int nReturn = CreateProcess( NULL, szCmd, NULL, NULL, TRUE/*inherit handles*/, 0,
                           NULL, NULL, &si, &pi);
      CloseHandle( hScriptStdoutWr );
      if( nReturn ) //script was started successfully
         while( fgets(szCmd, 1024, pfOut) );
      fclose( pfOut );
      if( WaitForSingleObject( pi.hProcess, SCRIPT_EXECUTION_TIMEOUT )
        == WAIT_TIMEOUT )
        TerminateProcess( pi.hProcess, 100/*exit code*/ );
      }
      else
        CloseHandle( hScriptStdoutRd );
		}

	return S_OK ;
}   
	
HRESULT CFactory::UnregisterAll()
{
	for(int i = 0 ; i < g_cFactoryDataEntries ; i++)   
	{
		UnregisterServer(*(g_FactoryDataArray[i].m_pCLSID),
		                 g_FactoryDataArray[i].m_szVerIndProgID, 
		                 g_FactoryDataArray[i].m_szProgID) ;
	}
	return S_OK ;
}

//////////////////////////////////////////////////////////
//
// Out of process Server support
//
//
// Start factories
//
BOOL CFactory::StartFactories()
{
	CFactoryData* pStart = &g_FactoryDataArray[0] ;
	const CFactoryData* pEnd =
		&g_FactoryDataArray[g_cFactoryDataEntries - 1] ;

	for(CFactoryData* pData = pStart ; pData <= pEnd ; pData++)
	{
		// Initialize the class factory pointer and cookie.
		pData->m_pIClassFactory = NULL ;
		pData->m_dwRegister = NULL ;

		// Create the class factory for this component.
		IClassFactory* pIFactory = new CFactory(pData) ;

		// Register the class factory.
		DWORD dwRegister ;
		HRESULT hr = ::CoRegisterClassObject(
		                  *pData->m_pCLSID,
		                  static_cast<IUnknown*>(pIFactory),
		                  CLSCTX_LOCAL_SERVER,
		                  REGCLS_MULTIPLEUSE,
		                  // REGCLS_MULTI_SEPARATE, //@Multi
		                  &dwRegister) ;
		if (FAILED(hr))
		{
			pIFactory->Release() ;
			return FALSE ;
		}

		// Set the data.
		pData->m_pIClassFactory = pIFactory ;
		pData->m_dwRegister = dwRegister ;
	}
	return TRUE ;
}

//
// Stop factories
//
void CFactory::StopFactories()
{
	CFactoryData* pStart = &g_FactoryDataArray[0] ;
	const CFactoryData* pEnd =
		&g_FactoryDataArray[g_cFactoryDataEntries - 1] ;

	for (CFactoryData* pData = pStart ; pData <= pEnd ; pData++)
	{
		// Get the magic cookie and stop the factory from running.
		DWORD dwRegister = pData->m_dwRegister ;
		if (dwRegister != 0) 
		{
			::CoRevokeClassObject(dwRegister) ;
		}

		// Release the class factory.
		IClassFactory* pIFactory  = pData->m_pIClassFactory ;
		if (pIFactory != NULL) 
		{
			pIFactory->Release() ;
		}
	}
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
// Register the component in the registry.
//
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// Set the given key and its value.
BOOL setKeyAndValue(const char* pszPath,
                    const char* szSubkey,
                    const char* szValue) ;

// Convert a CLSID into a char string.
void CLSIDtochar(const CLSID& clsid, 
                 char* szCLSID,
                 int length) ;

// Determine if a particular subkey exists.
BOOL SubkeyExists(const char* pszPath,
                  const char* szSubkey) ;

// Delete szKeyChild and all of its descendents.
LONG recursiveDeleteKey(HKEY hKeyParent, const char* szKeyChild) ;

////////////////////////////////////////////////////////
//
// Constants
//

// Size of a CLSID as a string
const int CLSID_STRING_SIZE = 39 ;

/////////////////////////////////////////////////////////
//
// Public function implementation
//

HRESULT RegisterServer(HMODULE hModule,            // DLL module handle
                       const CLSID& clsid,         // Class ID
                       const char* szFriendlyName, // Friendly Name
                       const char* szVerIndProgID, // Programmatic
                       const char* szProgID)       //   IDs
{
	// Get server location.
	char szModule[512] ;
	DWORD dwResult =
		::GetModuleFileName(hModule, 
		                    szModule,
		                    sizeof(szModule)/sizeof(char)) ;
	assert(dwResult != 0) ;

	// Convert the CLSID into a char.
	char szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;

	// Build the key CLSID\\{...}
	char szKey[64] ;
	strcpy(szKey, "CLSID\\") ;
	strcat(szKey, szCLSID) ;
  
	// Add the CLSID to the registry.
	setKeyAndValue(szKey, NULL, szFriendlyName) ;

	// Add the server filename subkey under the CLSID key.
	setKeyAndValue(szKey, "LocalServer32", szModule) ;

	// Add the ProgID subkey under the CLSID key.
	setKeyAndValue(szKey, "ProgID", szProgID) ;

	// Add the version-independent ProgID subkey under CLSID key.
	setKeyAndValue(szKey, "VersionIndependentProgID",
	               szVerIndProgID) ;

	// Add the version-independent ProgID subkey under HKEY_CLASSES_ROOT.
	setKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ; 
	setKeyAndValue(szVerIndProgID, "CLSID", szCLSID) ;
	setKeyAndValue(szVerIndProgID, "CurVer", szProgID) ;

	// Add the versioned ProgID subkey under HKEY_CLASSES_ROOT.
	setKeyAndValue(szProgID, NULL, szFriendlyName) ; 
	setKeyAndValue(szProgID, "CLSID", szCLSID) ;

	return S_OK ;
}

//
// Remove the component from the registry.
//
BOOL CheckDeleteResult(LONG lResult)
{
  BOOL bRet= (lResult == ERROR_SUCCESS) ;
  if ( !bRet )
  {
    char szText[] = "This version of DISCOVER was installed by Administator\n" \
	        "Uninstall will continue, but some not critical registry entries will not be deleted";
    MessageBox(NULL, szText,"Warning",MB_OK | MB_ICONEXCLAMATION );
  }
  return bRet;
}

LONG UnregisterServer(const CLSID& clsid,         // Class ID
                      const char* szVerIndProgID, // Programmatic
                      const char* szProgID)       //   IDs
{
	// Convert the CLSID into a char.
	char szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;

	// Build the key CLSID\\{...}
	char szKey[80] ;
	strcpy(szKey, "CLSID\\") ;
	strcat(szKey, szCLSID) ;

	// Check for a another server for this component.
	if (SubkeyExists(szKey, "InprocServer32"))
	{
	  // Delete only the path for this server.
	  strcat(szKey, "\\LocalServer32") ;
	  LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
          CheckDeleteResult(lResult);
	}
	else
	{
	  // Delete all related keys.
	  // Delete the CLSID Key - CLSID\{...}
	  LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
	  if ( CheckDeleteResult(lResult) && (lResult != ERROR_FILE_NOT_FOUND) )
	  {
	    // Delete the version-independent ProgID Key.
	    lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szVerIndProgID) ;
	    if ( CheckDeleteResult(lResult) && (lResult != ERROR_FILE_NOT_FOUND) )
	    {
	      // Delete the ProgID key.
	      lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szProgID) ;
	      CheckDeleteResult(lResult);
	    }
	  }
	}
	return S_OK ;
}

///////////////////////////////////////////////////////////
//
// Internal helper functions
//

// Convert a CLSID to a char string.
void CLSIDtochar(const CLSID& clsid,
                 char* szCLSID,
                 int length)
{
	assert(length >= CLSID_STRING_SIZE) ;
	// Get CLSID
	LPOLESTR wszCLSID = NULL ;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;
	assert(SUCCEEDED(hr)) ;

	// Covert from wide characters to non-wide.
	wcstombs(szCLSID, wszCLSID, length) ;

	// Free memory.
	CoTaskMemFree(wszCLSID) ;
}

//
// Delete a key and all of its descendents.
//
LONG recursiveDeleteKey(HKEY hKeyParent,           // Parent of key to delete
                        const char* lpszKeyChild)  // Key to delete
{
	// Open the child.
	HKEY hKeyChild ;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
	                         KEY_ALL_ACCESS, &hKeyChild) ;
	if (lRes != ERROR_SUCCESS)
	{
		return lRes ;
	}

	// Enumerate all of the decendents of this child.
	FILETIME time ;
	char szBuffer[256] ;
	DWORD dwSize = 256 ;
	while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
	                    NULL, NULL, &time) == S_OK)
	{
		// Delete the decendents of this child.
		lRes = recursiveDeleteKey(hKeyChild, szBuffer) ;
		if (lRes != ERROR_SUCCESS)
		{
			// Cleanup before exiting.
			RegCloseKey(hKeyChild) ;
			return lRes;
		}
		dwSize = 256 ;
	}

	// Close the child.
	RegCloseKey(hKeyChild) ;

	// Delete this child.
	return RegDeleteKey(hKeyParent, lpszKeyChild) ;
}

//
// Determine if a particular subkey exists.
//
BOOL SubkeyExists(const char* pszPath,    // Path of key to check
                  const char* szSubkey)   // Key to check
{
	HKEY hKey ;
	char szKeyBuf[80] ;

	// Copy keyname into buffer.
	strcpy(szKeyBuf, pszPath) ;

	// Add subkey name to buffer.
	if (szSubkey != NULL)
	{
		strcat(szKeyBuf, "\\") ;
		strcat(szKeyBuf, szSubkey ) ;
	}

	// Determine if key exists by trying to open it.
	LONG lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, 
	                              szKeyBuf,
	                              0,
	                              KEY_ALL_ACCESS,
	                              &hKey) ;
	if (lResult == ERROR_SUCCESS)
	{
		RegCloseKey(hKey) ;
		return TRUE ;
	}
	return FALSE ;
}

//
// Create a key and set its value.
//   - This helper function was borrowed and modifed from
//     Kraig Brockschmidt's book Inside OLE.
//
BOOL setKeyAndValue(const char* szKey,
                    const char* szSubkey,
                    const char* szValue)
{
	HKEY hKey;
	char szKeyBuf[1024] ;

	// Copy keyname into buffer.
	strcpy(szKeyBuf, szKey) ;

	// Add subkey name to buffer.
	if (szSubkey != NULL)
	{
		strcat(szKeyBuf, "\\") ;
		strcat(szKeyBuf, szSubkey ) ;
	}

	// Create and open key and subkey.
	long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
	                              szKeyBuf, 
	                              0, NULL, REG_OPTION_NON_VOLATILE,
	                              KEY_ALL_ACCESS, NULL, 
	                              &hKey, NULL) ;
	if (lResult != ERROR_SUCCESS)
	{
		return FALSE ;
	}

	// Set the Value.
	if (szValue != NULL)
	{
		RegSetValueEx(hKey, NULL, 0, REG_SZ, 
		              (BYTE *)szValue, 
		              strlen(szValue)+1) ;
	}

	RegCloseKey(hKey) ;
	return TRUE ;
}
