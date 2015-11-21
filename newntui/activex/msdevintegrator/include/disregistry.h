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
#ifndef	_DISREGITRY_H
#define _DISREGISTRY_H

#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
#endif

#define REG_MSVC_4X								"Software\\Microsoft\\Developer"
#define REG_MSVC_5X								"Software\\Microsoft\\DevStudio"
#define	REG_DEVELOPER_KEY					"DeveloperKey"
#define	REG_ADDIN_STARTUP					"AddInStartUp"

//#define REG_DISCOVER_ROOT         HKEY_LOCAL_MACHINE
#define REG_DISCOVER_ROOT         NULL

#define REG_DISCOVER              "Software\\Software Emancipation Technology\\Discover"
#define REG_DISCOVER_VERSION      "CurrentVersion"
#define REG_DIS_COMMAND           "command"
#define	REG_INTEGRATOR						"Integrator"
#define	REG_BROWSER								"Browser"
#define	REG_LOG_JOURNAL						"Log"
#define	REG_SCC										"Scc"

#define	REG_BROWSER_COMMAND				"command"
#define REG_DISCOVER_HOST         "cmdhost"
#define REG_DISCOVER_GROUP        "group"
#define REG_INTEGRATOR_SELECTION	"SelectionEvent"
#define REG_INTEGRATOR_DIR        "Location"
#define REG_INTEGRATOR_LOG        "LogPath"
#define REG_INTEGRATOR_LOG_ERR    "LogErrLevel"
#define REG_INTEGRATOR_LOG_WNG    "LogWngLevel"
#define REG_INTEGRATOR_LOG_MSG    "LogMsgLevel"
#define REG_INTEGRATOR_LOG_DBG    "LogDbgLevel"
#define	REG_INTEGRATOR_WND_HIDE		"DiscoverStartHide"
#define	REG_INTEGRATOR_WND_HPOS		"WndHPos"
#define	REG_INTEGRATOR_WND_VPOS		"WndVPos"
#define	REG_INTEGRATOR_WND_HSIZE	"WndHSize"
#define	REG_INTEGRATOR_WND_VSIZE	"WndVSize"
#define REG_INP_SOCKET            "cmdport"
#define REG_OUT_SOCKET            "evalport"
#define REG_BROWSER_START_TIMEOUT "BrowserStartTimeOut"
#define REG_BROWSER_START_WAITING	"BrowserStartWaiting"
#define	REG_DBLCLICK_TIMEOUT			"DblClickTimeout"
#define REG_DEBUG_PRMRECEIVER     "DebugParamReceiver"
#define REG_DEBUG_CONSOLE			    "DebugConsole"
#define REG_FILENAME_TRANS        "FileNameTrans"
#define	REG_FONT_NAME							"FontName"
#define REG_FONT_HEIGHT						"FontHeight"
#define REG_FONT_BOLD							"FontBold"
#define	REG_SCC_SUBSTITUTION			"SccServerSubstitution"
#define REG_ENV                   "Environment"
#define REG_INTEGRATOR_IDE				"IDE"


/***************************************************************************
		Registry
****************************************************************************/
LONG  GetDisCurrentVersion(char* pszVersionPath);
LONG  GetDisCurrentVersion(char* pszVersionPath, HKEY );

int		GetRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue=-1 );
int		GetRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue, HKEY );

BOOL	GetRegistryStr(char* pszSection, char* pszKeyName, char* pszStrBuffer, UINT nBufferLen, 
						    		 char* pszKeyDefaultVal=NULL );
BOOL	GetRegistryStr(char* pszSection, char* pszKeyName, char* pszStrBuffer, UINT nBufferLen, 
						    		 char* pszKeyDefaultVal, HKEY );

int		GetDisRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue=-1);
int		GetDisRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue, HKEY );

BOOL	GetDisRegistryStr(char* pszSection, char* pszKeyName, char* pszStrBuffer, UINT nBufferLen, 
						    				char* pszKeyDefaultVal=NULL );
BOOL	GetDisRegistryStr(char* pszSection, char* pszKeyName, char* pszStrBuffer, UINT nBufferLen, 
						    				char* pszKeyDefaultVal, HKEY );

BOOL  BuildDisCommand( const char *pszCmdSection, char *pszResult, UINT nBuffLen, BOOL fGetOptions = TRUE );

void	SetCustomEnvironment( const char *pszPrebuilt );

char const *realOSPATH	(char const *path);


/***************************************************************************
		Debug
****************************************************************************/

void ServerDebugMessageLog( UINT nErrLevel, char* pszWho, char* pszAction, 
													  char* pszComment1 =NULL, char* pszComment2 =NULL );
void ServerDebugMessageLogInt( UINT nErrLevel, char* pszWho, char* pszAction, 
													  char* pszComment1, int nInt );
void ServerDebugMessageLogIntX( UINT nErrLevel, char* pszWho, char* pszAction, 
													  char* pszComment1, int nInt );

/*void ServerDebugMessageWnd( HWND, long );*/

#define	MESSAGE_TIME		FALSE

#if 1 //#ifdef _DEBUG

#define LogError(M1,M2,M3)		  ServerDebugMessageLog(ERR_DBG,_THIS_FILE_,(M1),(M2),(M3))
#define LogErrorInt(M1,M2)		  ServerDebugMessageLogInt(ERR_DBG,_THIS_FILE_,(M1),#M2 "=",(int)(M2))
#define LogErrorIntX(M1,M2)		  ServerDebugMessageLogIntX(ERR_DBG,_THIS_FILE_,(M1),#M2 "=0x",(int)(M2))

#define LogWarning(M1,M2,M3)	  ServerDebugMessageLog(WNG_DBG,_THIS_FILE_,(M1),(M2),(M3))
#define LogWarningInt(M1,M2)	  ServerDebugMessageLogInt(WNG_DBG,_THIS_FILE_,(M1),#M2 "=",(int)(M2))
#define LogWarningIntX(M1,M2)	  ServerDebugMessageLogIntX(WNG_DBG,_THIS_FILE_,(M1),#M2 "=0x",(int)(M2))

#define LogMessage(M1,M2,M3)	  ServerDebugMessageLog(MSG_DBG,_THIS_FILE_,(M1),(M2),(M3))
#define LogMessageInt(M1,M2)	  ServerDebugMessageLogInt(MSG_DBG,_THIS_FILE_,(M1),#M2 "=",(int)(M2))
#define LogMessageIntX(M1,M2)	  ServerDebugMessageLogIntX(MSG_DBG,_THIS_FILE_,(M1),#M2 "=0x",(int)(M2))

#define LogDebug(M1,M2,M3)  	ServerDebugMessageLog(GetDisRegistryInt(REG_LOG_JOURNAL,REG_INTEGRATOR_LOG_DBG,1954),_THIS_FILE_,(M1),(M2),(M3))
#define LogDebugInt(M1,M2)  	ServerDebugMessageLogInt(GetDisRegistryInt(REG_LOG_JOURNAL,REG_INTEGRATOR_LOG_DBG,1954),_THIS_FILE_,(M1),#M2 "=",(int)(M2))
#define LogDebugIntX(M1,M2)  	ServerDebugMessageLogIntX(GetDisRegistryInt(REG_LOG_JOURNAL,REG_INTEGRATOR_LOG_DBG,1954),_THIS_FILE_,(M1),#M2 "=0x",(int)(M2))

#else		//#ifdef _DEBUG

#define LogError(M1,M2,M3)
#define LogErrorInt(M1,M2)
#define LogErrorIntX(M1,M2)
#define LogWarning(M1,M2,M3)
#define LogWarningInt(M1,M2)
#define LogWarningIntX(M1,M2)
#define LogMessage(M1,M2,M3)
#define LogMessageInt(M1,M2)
#define LogMessageIntX(M1,M2)
#define LogDebug(M1,M2,M3)
#define LogDebugInt(M1,M2)
#define LogDebugIntX(M1,M2)

#endif	//#ifdef _DEBUG

#endif	// #ifndef	_DISREGITRY_H
