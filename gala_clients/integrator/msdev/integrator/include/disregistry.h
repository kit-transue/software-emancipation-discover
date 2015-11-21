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

#define REG_DISCOVER              "Software\\Discover"
#define REG_DISCOVER_DIR          "DiscoverDir"
#define REG_DISCOVER_HOST         "DiscoverHost"
#define REG_DISCOVER_LOG          "DiscoverLog"
#define REG_DISCOVER_LOG_ERR      "DiscoverLogErrLevel"
#define REG_DISCOVER_LOG_WNG      "DiscoverLogWngLevel"
#define REG_DISCOVER_LOG_MSG      "DiscoverLogMsgLevel"
#define	REG_DISCOVER_WND_HIDE			"DiscoverStartHide"
#define REG_INP_SOCKET            "InpSocket"
#define REG_OUT_SOCKET            "OutSocket"
#define REG_BROWSER               "BrCommand"
#define REG_BROWSER_WAITING	      "BrowserStartWaiting"
#define	REG_DBLCLICK_TIMEOUT			"DblClickTimeout"
#define REG_SHOW_WINDOW           "ShowIntegratorWindow"
#define REG_DEBUG_PRMRECIEVER     "DebugParamReciever"
#define REG_USE_CLIPBOARD         "UseClipboard"
#define REG_FILENAME_TRANS        "FileNameTrans"


/***************************************************************************
		Registry
****************************************************************************/

int		GetRegistryInt(char* pszSection, char* pszKeyName, int nKeyDefaultValue=-1);
BOOL	GetRegistryStr(char* pszSection, char* pszKeyName, char* pszStrBuffer, UINT nBufferLen, 
						    		 char* pszKeyDefaultVal=NULL);

int		GetDisRegistryInt(char* pszKeyName, int nKeyDefaultValue=-1);
BOOL	GetDisRegistryStr(char* pszKeyName, char* pszStrBuffer, UINT nBufferLen, 
						    				char* pszKeyDefaultVal=NULL);

void	SetRegistryInt   (char* pszSection, char* pszKeyName, int nKeyValue);
void	SetDisRegistryInt(char* pszKeyName, int nKeyValue);


void ServerDebugMessageLog( UINT nErrLevel, char* pszWho, char* pszAction, 
													  char* pszComment1 =NULL, char* pszComment2 =NULL );
#endif	// #ifndef	_DISREGITRY_H
