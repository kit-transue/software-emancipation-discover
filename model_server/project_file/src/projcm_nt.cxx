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
#include <stdio.h>
#include <objbase.h>
#include <comiface.h>
#include <machdep.h>

void	ProjCM_InitConfigErr();
char*	ProjCM_GetInstallRoot();
char*	ProjCM_GetCfgCmd( const char* cmd,  const char *tag,  const char* arg1, const char * arg2,
                        const char* arg3, const char* arg4, const char* arg5);
int	ProjCM_StoreResult(char* result);

//------------------------------
class CConfigShell
{
private:
  IConfigShell*	    m_pIConfigShell;
  BOOL		    m_bCreated;

public:

  CConfigShell();
 ~CConfigShell();

  BOOL		Init();
  void		Destroy();
  BOOL		Run( char* );
  char*		GetResult();
};

//------------------------------

static CConfigShell g_ConfigShell;

//------------------------------
static long g_ConfigShellCount;
static CRITICAL_SECTION g_ConfigShellCriticalSection;
static BOOL g_bInitCriticalSectionInit = FALSE;

void InitializeConfigCriticalSection()
{
  if ( !g_bInitCriticalSectionInit )
  {
   InitializeCriticalSection( &g_ConfigShellCriticalSection );
   g_bInitCriticalSectionInit = TRUE;
  }
}

//------------------------------
CConfigShell::CConfigShell()
{
  InitializeConfigCriticalSection();
  ::InterlockedIncrement(&g_ConfigShellCount) ;
  m_pIConfigShell = NULL;
  m_bCreated	  = FALSE;
}

//------------------------------
CConfigShell::~CConfigShell()
{
  Destroy();
  InitializeConfigCriticalSection();
  ::InterlockedDecrement(&g_ConfigShellCount) ;
  if ( g_ConfigShellCount == 0 )
    DeleteCriticalSection( &g_ConfigShellCriticalSection );
}

//------------------------------
void	CConfigShell::Destroy()
{
  InitializeConfigCriticalSection();
  EnterCriticalSection(&g_ConfigShellCriticalSection);
  if ( m_pIConfigShell )
  {
    IConfigShell* pIConfigShell = m_pIConfigShell;
    m_pIConfigShell = NULL;
    if ( m_bCreated )
      pIConfigShell->Release();
    CoUninitialize();
  }
  m_bCreated = FALSE;
  LeaveCriticalSection(&g_ConfigShellCriticalSection);
}

//------------------------------
extern int customize_no_cm();

BOOL	CConfigShell::Init()
{
  BOOL bRet = FALSE;
  if ( customize_no_cm() )
     return bRet;

  InitializeConfigCriticalSection();
  EnterCriticalSection(&g_ConfigShellCriticalSection);
  HRESULT hr;
  if ( m_pIConfigShell )
    bRet = TRUE;
  
  else	//if ( m_pIConfigShell == NULL ) // initialize COM interface
  {
    wchar_t*	pwcsCMName = NULL ;
    wchar_t*	pwcsScriptDir;
    char	pszCMName;
    char	szScriptDir[_MAX_PATH];;
    char*	p;
    int		nLength;

#ifdef _DEBUG
	if ( GetDiscoverRegistryDebug("ConfigShellCreate") )
		_asm int 3;
#endif
    CoInitialize(NULL);
    hr = CoCreateInstance(CLSID_ConfigShell,
 		    NULL, CLSCTX_LOCAL_SERVER, 
		    IID_IConfigShell, (void**)&m_pIConfigShell);
    if ( FAILED(hr) )
    {
      ProjCM_InitConfigErr();
      m_bCreated = FALSE;
    }
    else
    {
      m_bCreated = TRUE;
      hr = m_pIConfigShell->InitCM(&pwcsCMName);
      if ( SUCCEEDED(hr) )
      {
	strcpy (szScriptDir, ProjCM_GetInstallRoot() );
	if (szScriptDir[0] && (p= szScriptDir+ strlen(szScriptDir)-1 ) &&
	    (*p == '/' || *p == '\\' )
	   )
	   *p='\0';	
	strcat(szScriptDir,"\\bin\\");
	p = szScriptDir+ strlen(szScriptDir);
	nLength = (wcslen(pwcsCMName) +1)*sizeof(wchar_t);
	wcstombs( p, pwcsCMName, nLength ); 
	::CoTaskMemFree(pwcsCMName) ;

	nLength = strlen( szScriptDir ) +1;
	pwcsScriptDir = new wchar_t [ nLength ]; 
	mbstowcs( pwcsScriptDir, szScriptDir, nLength ); 
	hr = m_pIConfigShell->SetPSETCFG(pwcsScriptDir);
	delete [] pwcsScriptDir;
	bRet = TRUE;
      }
    }
  }

  if ( !bRet )
  {
    ProjCM_InitConfigErr();
    Destroy();
  }
  LeaveCriticalSection(&g_ConfigShellCriticalSection);
  return bRet;
}

//------------------------------
BOOL	CConfigShell::Run( char* pszCmd)
{
  if ( m_pIConfigShell && m_bCreated )
  {
    HRESULT hr;
    int	    nLengthIn = strlen( pszCmd ) +1;
    wchar_t* pwcsCmd = new wchar_t [ nLengthIn ];
    if ( pwcsCmd )
    {
      mbstowcs( pwcsCmd, pszCmd, nLengthIn ); 
      if (pszCmd) delete[] pszCmd;

      hr = m_pIConfigShell->Run(pwcsCmd) ;
      delete [] pwcsCmd;
      return (SUCCEEDED(hr));
    }
  }
  
  return FALSE;
}

//------------------------------
char*	CConfigShell::GetResult()
{
  if ( m_pIConfigShell && m_bCreated )
  {
    HRESULT	hr;
    wchar_t*	pwcsOut = NULL ;
    char*	p;
    char*	pszOut;
    char*	pszLine;
    int		nLengthOut;
    hr =m_pIConfigShell->GetResult(&pwcsOut);
    if ( !SUCCEEDED(hr) || pwcsOut == NULL )
      return NULL;

    nLengthOut = (wcslen(pwcsOut) +1)*sizeof(wchar_t) ;
    pszOut = new char [ nLengthOut ];
    if ( pszOut )
      wcstombs( pszOut, pwcsOut, nLengthOut ); 

    ::CoTaskMemFree(pwcsOut);
    return pszOut;
  }
  else
    return NULL;
}								 
///////////////////////////////////////////////////////////////////////
// The interface to process ConfigShell on NT
//
// cmd: is the config-command
// tag: defined options
// arg1-5: data needed for config-command

int shell_configurator_send_internal ( const char* cmd,  const char *tag,  const char* arg1, const char * arg2,
                         const char* arg3, const char* arg4, const char* arg5)
{
  // We get the 'exit' command from aset_driver_cleanup()
  if (!strcmp(cmd,"exit") || !g_ConfigShell.Init() )
  {
    g_ConfigShell.Destroy();
    return 0;
  }
    
  char*	cfg_cmd = ProjCM_GetCfgCmd( cmd, tag, arg1, arg2, arg3, arg4, arg5);

  if ( !cfg_cmd || !g_ConfigShell.Run(cfg_cmd) )
  {
    ProjCM_StoreResult("%% CM configurator was aborted");
    g_ConfigShell.Destroy();
    return 0;
  }

  return 1;
}

int CM_ShowResult(int err);

int shell_configurator_retrieve_internal ()
{
  char*	pszOut;
  int	return_code=1;

  while ( (pszOut =g_ConfigShell.GetResult() ) )
  {
    return_code = ProjCM_StoreResult(pszOut);
    delete [] pszOut;
  }
  return CM_ShowResult( return_code );
}

//------------------------------------------
int shell_configurator_init()
{
  return (int)g_ConfigShell.Init();
}

void shell_configurator_destroy()
{
  g_ConfigShell.Destroy();
}
