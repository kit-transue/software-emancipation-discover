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
#ifndef	_DISREGISTRYINTEGRATOR_H
#define _DISREGISTRYINTEGRATOR_H

#define _ERR_MESSAGEBOX
#include "DisRegistry.h"

#define	REG_BROWSER_COMMAND				"command"
#define REG_DISCOVER_HOST         "cmdhost"
#define REG_DISCOVER_GROUP        "group"
#define REG_INTEGRATOR_SELECTION	"SelectionEvent"
#define REG_INTEGRATOR_DIR        "Location"
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
#define REG_ENV                   "Environment"
#define REG_INTEGRATOR_IDE				"IDE"

/***************************************************************************
		Registry
****************************************************************************/
BOOL  BuildDisCommand( const char *pszCmdSection, char *pszResult, UINT nBuffLen, BOOL fGetOptions = TRUE );
void	SetCustomEnvironment( const char *pszPrebuilt );

#endif	// #ifndef	_DISREGISTRYINTEGRATOR_H
