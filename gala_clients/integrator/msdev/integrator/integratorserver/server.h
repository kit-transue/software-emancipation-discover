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

#ifndef _SERVER_H
#define _SERVER_H

#include "Integrator.h"
#include "resource.h"
#include "CList.h"

#define BROWSER_START_TIMEOUT     60000

/****************************************************************************
	Classes Query
*****************************************************************************/
CircleList* GetQueueList();

BOOL IsQueryIdCorrect(int n);
void GenericQueryToServer( UINT nIDEId, UINT nQueryId, long, void*, void* );

enum QUERY_STATUS
{
		QUERY_STATUS_SLEEP,
		QUERY_STATUS_WAIT_CONSOLE,
		QUERY_STATUS_WAIT_IDESPY,
		QUERY_STATUS_WAIT_SOCKET,
		QUERY_STATUS_WAIT_DDECLIENT_DISCONNECT
};

struct IDECommand
{
	HCONV					hConvFrom;
	IDE_COMMAND		iIDECommnd;
	char*					pszParams;
	int						nParamsLen;
};

//-------
class IDEInfo;

class DI_QueryGeneric
{
protected:
	HWND					m_hMainIDEWnd;
	IDEInfo*			m_pIDEInfo;
	DI_QUERIES		m_nQueryId;
	QUERY_STATUS	m_nQueryStatus;
	HCONV					m_hConv;

public:
	~DI_QueryGeneric();
	DI_QueryGeneric();
	DI_QueryGeneric(HWND);

	HWND					GetMainIDEWnd()			{ return m_hMainIDEWnd; }
	DI_QUERIES		GetQueryId()				{ return m_nQueryId; }
	HCONV					GetHConv()					{ return m_hConv; }
	void					SetHConv(HCONV h)		{ m_hConv = h; }
	BOOL					IsQueryProcessed()	{ return (QUERY_STATUS_SLEEP != m_nQueryStatus); }
	BOOL					IsQueryDiscover()		{ return (m_nQueryId < D_MAX_QUERYNUM); }

	BOOL					IsQueryCompat(DI_QUERIES);
	void					SetIDEInfo(IDEInfo* pIDE) { m_pIDEInfo=pIDE; }

	virtual BOOL	StartQuery() = 0;	
	virtual char* SignalToQuery(IDECommand*) = 0;
	
};
//------------------
class DQueryDiscoverStarted: public DI_QueryGeneric
{
public:
	DQueryDiscoverStarted(HWND);
	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
	void  CopyData(DI_QueryGeneric*);
};
//------------------
class DQueryOpenFile: public DI_QueryGeneric
{
protected:
	char	m_pszFilePath[_MAX_PATH+14];
  BOOL  m_bNetMapFlag;
public:
	DQueryOpenFile(HWND, char * = NULL, char * = NULL );
	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
  char* GetFilePath() { return m_pszFilePath; }
  GetNetMapFlag()     { return m_bNetMapFlag; }
};

//------------------
class DQueryCompileOptions: public DI_QueryGeneric
{
protected:
	char	m_pszFilePath[_MAX_PATH+1];
public:
	DQueryCompileOptions(HWND, char * = NULL);
	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
};
//------------------
class DQueryStartIDE: public DI_QueryGeneric
{
protected:
	char	m_pszProjectPath[_MAX_PATH+1];
public:
	DQueryStartIDE(HWND, char * = NULL);
	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
};
//------------------
class DQueryDisplayStatus: public DI_QueryGeneric
{
protected:
	char*	m_pszStatusText;
public:
	DQueryDisplayStatus(HWND, char * = NULL);
	~DQueryDisplayStatus();

	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
	BOOL	SetStatusText( char* pszStatusText );
  char* GetStatusText() { return m_pszStatusText; }
};
//------------------
class IQueryQuery: public DI_QueryGeneric
{
protected:
	UINT		m_nQueryQueryType;
public:
	IQueryQuery(HWND);

	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
};
//------------------
class IQueryProjectChanged: public DI_QueryGeneric
{
protected:
	char	m_pszProjectPath[_MAX_PATH+1];
public:
	IQueryProjectChanged(HWND);
	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
};
//------------------
class IQueryIDEFinished: public DI_QueryGeneric
{
public:
	IQueryIDEFinished(HWND);
	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
};
//------------------
class IQueryRefreshProject: public DI_QueryGeneric
{
protected:
	char	m_pszProjectPath[_MAX_PATH+1];
public:
	IQueryRefreshProject(HWND);
	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
};
//------------------
class IQueryStartDiscover: public DI_QueryGeneric
{
public:
	IQueryStartDiscover(HWND);
	BOOL	StartQuery();
	char* SignalToQuery(IDECommand* );
};
//
//------------------ DI_QueryList
//
class DI_QueryList
{
protected:
	DI_QueryList*			m_pNextItem;
	DI_QueryGeneric*	m_pQuery;
public:
	BOOL							InsertQuery( DI_QueryGeneric* );
	void							DeleteQuery();
	DI_QueryGeneric*	GetQuery()	{ return m_pQuery; }	
	DI_QueryList*			GetNext()		{ return m_pNextItem; }
};

/****************************************************************************
	Classes IDEInfo
*****************************************************************************/
class IDEInfo
{
protected:
	IDEInfo*							m_pIDEInfoNext;
	IDE_TYPES							m_nIDEType;
	char									m_szProjectName[_MAX_PATH];
	char									m_szConfigName [_MAX_PATH];
	time_t								m_tProjectTime;
	HWND									m_hMainIDEWnd;
	DDEIntegratorClient*	m_pIDESpyClient;
	HCONV									m_hIDESpyConv;

	DI_QueryGeneric * m_pQueries[DI_MAX_QUERYNUM];

public:
	 IDEInfo( HWND, IDE_TYPES, char* pszProjectName );
	~IDEInfo();

	DQueryDiscoverStarted	* GetDQueryDiscoverStarted	(){ return (DQueryDiscoverStarted*)m_pQueries[D_QUERY_DISCSTARTED]; }
	DQueryOpenFile				* GetDQueryOpenFile					(){ return (DQueryOpenFile			 *)m_pQueries[D_QUERY_OPENFILE]; }
	DQueryCompileOptions	* GetDQueryCompileOptions		(){ return (DQueryCompileOptions *)m_pQueries[D_QUERY_GETOPTIONS]; }
	DQueryStartIDE				* GetDQueryStartIDE					(){ return (DQueryStartIDE			 *)m_pQueries[D_QUERY_STARTIDE]; }
	DQueryDisplayStatus		* GetDQueryDisplayStatus		(){ return (DQueryDisplayStatus	 *)m_pQueries[D_QUERY_DISPLAYSTATUS]; }
	IQueryQuery						* GetIQueryQuery						(){ return (IQueryQuery					 *)m_pQueries[I_QUERY_QUERY]; }
	IQueryProjectChanged	* GetIQueryProjectChanged		(){ return (IQueryProjectChanged *)m_pQueries[I_QUERY_PROJECTCHANGED]; }
	IQueryIDEFinished			* GetIQueryIDEFinished			(){ return (IQueryIDEFinished		 *)m_pQueries[I_QUERY_IDEFINISHED]; }
	IQueryRefreshProject	* GetIQueryRefreshProject		(){ return (IQueryRefreshProject *)m_pQueries[I_QUERY_REFRESHPROJECT]; }
	IQueryStartDiscover		* GetIQueryStartDiscover		(){ return (IQueryStartDiscover	 *)m_pQueries[I_QUERY_STARTDISCOVER]; }
	
	IDEInfo*							GetNextIDE			()	{ return m_pIDEInfoNext; }
	IDE_TYPES							GetIDEType			()	{ return m_nIDEType; }
	HWND									GetMainIDEWnd		()	{ return m_hMainIDEWnd; }
	HCONV									GetIDESpyConv		()	{ return m_hIDESpyConv; }
	char*									GetProjectName	()	{ return m_szProjectName; }
	char*									GetConfigName		()	{ return m_szConfigName; }
	time_t								GetProjectTime	()	{ return m_tProjectTime; }
	DDEIntegratorClient*	GetIDESpyClient	()	{ return m_pIDESpyClient; }
	const char*						GetIDEShortName	();

	void				SetNextIDE	(IDEInfo* pIDE) { m_pIDEInfoNext = pIDE; }
	
	void	CreateIDESpyClient( char* pszHWND );
	void	SetIDESpyConv			( HCONV hConv )	{ m_hIDESpyConv = hConv; }
	void	SetProjectName		( char* );
	void	SetConfigName			( char* );
	BOOL	IsQueryPermited		( DI_QUERIES);
	BOOL	ResetAndStartQuery( Node* );
	char*	SignalToAllQuery	( IDECommand* );

};

//
//------------------ IDEInfoList
//
class IDEInfoList
{
protected:
	IDEInfo*	m_pIDEInfoHead;

public:

	 IDEInfoList();

IDEInfo*	GetFirstIDEInfo						()	{ return m_pIDEInfoHead; }
IDEInfo*	FindIDEInfoByIDESpyConv		( HCONV );
IDEInfo*	FindIDEInfoByMainWnd			( HWND  );
IDEInfo*	CreateIDEInfoObject				( HWND, char* pszCmdLine);
void			DestroyIDEInfoObject			( IDEInfo* pIDEInfo );
HWND			FindMainAppWindow					( char* pszCmdLine );

char*		SignalToAllQuery( IDECommand* );

};
										
IDEInfoList*	GetIDEInfoList				();
IDE_TYPES			GetIDETypeByShortName	( char* pszIDEShortName );
const char*		GetCaptionByIDEType		( IDE_TYPES );

/***************************************************************************
	Win
****************************************************************************/
#define WSA_READ								(WM_USER + 1)
#define WSA_WRITE								(WM_USER + 2)
#define WSA_CONNECT             (WM_USER + 3)
#define WM_LOOKNEWQUERY					(WM_USER + 20)
#define WM_IDECOMMAND						(WM_USER + 31)
#define WM_IDESPYFINISH					(WM_USER + 32)
#define WM_IDESERVERFINISH			(WM_USER + 33)
#define WM_CREATE_NEW_IDEINFO		(WM_USER + 34)

HINSTANCE	GetServerHInstance		();
BOOL			InitApplication				(HINSTANCE);
HWND			GetIntegratorMainWnd	();

LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

BOOL InitIntegratorServer ();
void ExitIntegratorServer ();

void SignalToAllQuery					( IDE_COMMAND, char*, HCONV, HWND);
BOOL MakeDoubleClick					( HWND hWnd, UINT nMilliSecsTimeout, UINT nXpos, UINT nYpos );
void TerminateServer					( char*	szPostText );

void  DebugServerAdapter  ();
void	DebugServerDDE      ();
void  DebugServerIDEInfo  ();
void  DebugServerQuery    ();
void  DebugServerWin      ();

/***************************************************************************
	Socket Procedures
****************************************************************************/

BOOL			InitTclSocket			();
BOOL			ConnectToInpSocket();
BOOL			ConnectToOutSocket();
void			InpSocketRead			(WPARAM wParam, LPARAM lParam);
void			OutSocketRead			(WPARAM wParam, LPARAM lParam);
void			CloseTclSocket		();
SOCKET		GetSocketOut			();
SOCKET  	GetSocketInp      ();
char*			GetComputerName		();

void			sendDis( SOCKET, char*, int Flag=0);

/***************************************************************************
	DDE Procedures
****************************************************************************/

BOOL	DDEServerCheckInstance	();
BOOL	DDEServerStart					();
void	DDEServerFinish					();
void  DDEStopStarting         ();
void  DDEBeginFinishing       ();
BOOL  IsDDEStartingOrFinished ();


DWORD		 GetDDEIDEInfoServiceID						();
HDDEDATA SendIDEConsoleData								(HCONV, HSZ);
HDDEDATA RecieveDebugMessage							(HSZ, HDDEDATA );
HDDEDATA RecieveIDEParam									(HDDEDATA, HCONV, HSZ);
HDDEDATA SendIDESpyData										(HCONV, HSZ);
HDDEDATA RecieveIDESpyData								(HDDEDATA, HCONV, HSZ);

HDDEDATA CALLBACK IDEInfoServiceCallBack(
			WORD		wType,
			WORD		wFmt,
			HCONV		hConv,
			HSZ			hsz1,
			HSZ			hsz2,
			HDDEDATA	hData,
			DWORD		dwData1,
			DWORD		dwData2
			);

void InpSockExecute( char *, UINT );

#endif // #ifndef _SERVER_H


