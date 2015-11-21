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

#ifndef _SCCContext_H
#define _SCCContext_H

#include	<integrator.h>
#include	"Scc.h"

#define	STR_SCC_DISCOVER_REG_LOCATION			STR_SCC_PROVIDER_REG_LOCATION"\\InstalledSCCProviders"
#define STR_SCC_DISCOVER_REG_NAME					"Discover CM Support"
#define STR_SCC_DISCOVER_SUBSTITUTION			"SccServerSubstitution"
#define	STR_SCC_DISCOVER_LOG_PERMIT				"SccLogPermit"

typedef SCCRTN (* PFNSCC)(...);

class CSccContext
{
private:
	LPVOID								m_pSccContext;
	HINSTANCE							m_ghInstSrvr; 							// Loaded instance of the provider
	PFNSCC								m_gpFuncs[SCC_CMD_COUNT+1]; 	// Array of call-backs for dispatch

public:
	BOOL									m_bLogPermit;
	BOOL									m_bInitialize;
	HWND									m_IDEhWnd;
	char*									m_pszSccName;
	char*									m_pszAuxPathLabel;
	char*	 								m_pszProjName;
	char*									m_pszLocalProjPath;
	char*									m_pszAuxProjPath;

	POPLISTFUNC						m_pfnIDEPopulate;
	LPVOID								m_pvIDECallerData;

	SCCRTN								m_sccCmdOpt[ SCC_COMMAND_OPTIONS +1 ];

public:
	CSccContext();
 ~CSccContext();

	BOOL			LoadServer				();
	void			ClearFileList			();

	HINSTANCE	GetInstServer			()					{ return m_ghInstSrvr; }
	PFNSCC		SccFunc						(int nDex ) { return m_gpFuncs[nDex]; }
	void			SetSccFunc				(int nDex, PFNSCC pFunc ) { m_gpFuncs[nDex] = pFunc; }
	LPVOID		GetSccContext			()					{ return m_pSccContext; }
	void			SetSccContext			(LPVOID pv)	{ m_pSccContext=pv; }

	void			ResetCmdOptions		();
	SCCRTN		GetCmdOptions			(enum  SCCCOMMAND n)	{ return m_sccCmdOpt[(int)n]; }
};

LPCSTR	SccFuncName	(int nDex );

#endif  // #ifndef _SCCCSccContext_H
