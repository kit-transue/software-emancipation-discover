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

#ifndef _SCCDISCOVERCONTEXT_H
#define _SCCDISCOVERCONTEXT_H

#include	"Scc.h"
#include	<Integrator.h>

#define	STR_SCC_DISCOVER_REG_LOCATION			STR_SCC_PROVIDER_REG_LOCATION"\\InstalledSCCProviders"
#define STR_SCC_DISCOVER_REG_NAME					"Discover CM Support"
#define STR_SCC_DISCOVER_SUBSTITUTION			"SccServerSubstitution"
#define STR_SCC_DISCOVER_ACCEL_RW					"AccelratorRW"
#define STR_SCC_DISCOVER_ACCEL_QINFO			"AccelratorQueryInfo"
#define STR_SCC_DISCOVER_ACCEL_POPULATE		"AccelratorPopulateList"
#define	STR_SCC_DISCOVER_LOG_PERMIT				"SccLogPermit"

typedef SCCRTN (* PFNSCC)(...);

class	FileDef
{
private:
	FileDef*		m_pPrevFileDef;
	FileDef*		m_pNextFileDef;
	FileDef**		m_ppFirstFileDef;
	long				m_nStatus;
	char*				m_pszFilePath;

public:
	FileDef(FileDef**, char* pszFileName, long nStatus=SCC_STATUS_NOTCONTROLLED );
 ~FileDef();

	FileDef*		GetPrev			()					{ return m_pPrevFileDef; }
	FileDef*		GetNext			()					{ return m_pNextFileDef; }
	char*				GetFilePath	()					{ return m_pszFilePath;	 }
	long				GetStatus		()					{ return m_nStatus;			 }
	void				SetStatus		(long n)		{ m_nStatus = n;				 }
	int					NameCmp			(char* psz);
	
	void				Rename			(char* psz);
	void				RiseUp			();
};


class DiscoverContext
{
private:
	LPVOID								m_pSccContext;
	HINSTANCE							m_ghInstSrvr; 							// Loaded instance of the provider
	PFNSCC								m_gpFuncs[SCC_CMD_COUNT+1]; 	// Array of call-backs for dispatch
	FileDef*							m_pFirstFileDef;
	DDEIntegratorClient*	m_pDDEClient;

public:
	BOOL									m_bAccelRW;
	BOOL									m_bAccelQInfo;
	BOOL									m_bAccelPopulate;
	BOOL									m_bLogPermit;
	BOOL									m_bRiseUp;
	BOOL									m_bInitialize;
	HWND									m_IDEhWnd;
	char*									m_pszSccName;
	char*									m_pszAuxPathLabel;
	char*	 								m_pszProjName;
	char*									m_pszLocalProjPath;
	char*									m_pszAuxProjPath;

	char									m_szEventFileName[_MAX_PATH];
	LONG									m_nEventStatus;
	LONG									m_nEventsRemaining;

	POPLISTFUNC						m_pfnIDEPopulate;
	LPVOID								m_pvIDECallerData;

	SCCRTN								m_sccCmdOpt[ SCC_COMMAND_OPTIONS +1 ];

public:
	DiscoverContext();
 ~DiscoverContext();

	void			ClearFileList			();

	HINSTANCE	GetInstServer			()					{ return m_ghInstSrvr; }
	PFNSCC		SccFunc						(int nDex ) { return m_gpFuncs[nDex]; }
	void			SetSccFunc				(int nDex, PFNSCC pFunc ) { m_gpFuncs[nDex] = pFunc; }
	LPVOID		GetSccContext			()					{ return m_pSccContext; }
	void			SetSccContext			(LPVOID pv)	{ m_pSccContext=pv; }
	BOOL			ConnectToServer		();
	void			SendEventToServer	(char* pszEvent, char* pszFileName);
	void			SendEventToServer	(char* pszEvent, int nFiles, char** ppszFileNames);
	
	void			CheckEvents				();
	void			CheckFileList			(LONG, LPCSTR*, LPLONG lpExtStatus=NULL);
	void			GetFileStatus			(LONG, LPCSTR*, LPLONG);
	void			RemoveFiles				(LONG, LPCSTR*);
	void			RenameFile				(LPCSTR pszOldName, LPSTR pszNewName);

	void			ResetCmdOptions		();
	SCCRTN		GetCmdOptions			(enum  SCCCOMMAND n)	{ return m_sccCmdOpt[(int)n]; }

	SCCRTN		DisPopulate	(enum SCCCOMMAND	nCommand, 
													LONG						nFiles, 
													LPCSTR* 				lpFileNames, 
													LPLONG					lpStatus
												);
};

#endif  // #ifndef _SCCDISCOVERCONTEXT_H
