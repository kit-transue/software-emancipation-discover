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
static IDEInfoList	GlobalIDEInfoList;

static const struct
	{
		IDE_TYPES	nIDEType;
		char		szIDEShortName[3];
		char		szIDECaption[256];
	} IDETypesDef[] = 
			{
				{ IDE_MIN_TYPE_NUM,		"", ""},
				{ IDE_TYPE_MSVC40,		IDE_SHORTNAME_MSVC40,			IDE_CAPTION_MSVC40 },
				{ IDE_TYPE_BORLANDC45,IDE_SHORTNAME_BORLANDC45,	IDE_CAPTION_BORLANDC45 },
				{ IDE_MAX_TYPE_NUM,		"", ""}
			};

void  DebugServerIDEInfo ()
{
  _asm int 3;
}

/****************************************************************************
	----
*****************************************************************************/
IDE_TYPES GetIDETypeByShortName( char* szShortName )
{
	for ( int iIDE= IDE_MIN_TYPE_NUM+1; iIDE < IDE_MAX_TYPE_NUM; iIDE++ )
	{
		if ( strcmp( szShortName, IDETypesDef[iIDE].szIDEShortName ) == 0)
			return IDETypesDef[iIDE].nIDEType;
	}
	return IDE_MIN_TYPE_NUM;
}

const char*	GetCaptionByIDEType	( IDE_TYPES iIDE )
{
	return (const char*)IDETypesDef[iIDE].szIDECaption;
}

IDEInfoList*	GetIDEInfoList()
{
	return &GlobalIDEInfoList;
}

void SignalToAllQuery	( IDE_COMMAND iIDECmd, char* pszParams, HCONV hConv, HWND hIDEMainWnd)
{
	IDEInfo* pIDEInfo = GetIDEInfoList()->FindIDEInfoByMainWnd( hIDEMainWnd );
  if ( pIDEInfo )
  {
    IDECommand*	pIDECmd = new IDECommand;
	  if ( pIDECmd )
		  {
		  pIDECmd->hConvFrom	= hConv;
		  pIDECmd->iIDECommnd	= iIDECmd;
		  pIDECmd->pszParams	= pszParams;
		  pIDECmd->nParamsLen	= 0;
		  if ( pIDECmd->pszParams )
			  {
			  pIDECmd->nParamsLen	= strlen( pszParams );
			  if ( pIDECmd->nParamsLen >0 )
				  {
				  pIDECmd->pszParams = new char [pIDECmd->nParamsLen+1];
				  strcpy( pIDECmd->pszParams, pszParams );
				  }
			  }
		  PostMessage( GetIntegratorMainWnd(), WM_IDECOMMAND, 
																				  (WPARAM)hIDEMainWnd, (LPARAM)pIDECmd );
		  } // if ( pIDECmd  )
  }     // if ( pIDEInfo )
}

/****************************************************************************
	IDEInfoList
*****************************************************************************/
IDEInfoList::IDEInfoList()
{
	m_pIDEInfoHead = NULL;
}


//---------------
//
void IDEInfoList::DestroyIDEInfoObject( IDEInfo* pIDE )
{
	if ( pIDE == NULL )
		return;
	if ( m_pIDEInfoHead == NULL )
		delete pIDE;

	if ( m_pIDEInfoHead == pIDE )
	{
		m_pIDEInfoHead = pIDE->GetNextIDE();
		delete pIDE;
	}
	else
	{
		IDEInfo* pIDEPrev = m_pIDEInfoHead;
		while ( pIDEPrev->GetNextIDE() )
		{
			if ( pIDEPrev->GetNextIDE() == pIDE)
			{
				pIDEPrev->SetNextIDE( pIDE->GetNextIDE() );
				delete pIDE;
				break;
			}
		} // while
	} // else

	if ( m_pIDEInfoHead == NULL )
			TerminateServer( EXTMSG_DIS_FINISHED );
}

//------------------------------------------
//				CreateIDEInfoObject
//
IDEInfo* IDEInfoList::CreateIDEInfoObject( HWND hIDEWnd, char* pszCmdLine )
{
	char*	pszProjectName = strchr( pszCmdLine, ' ');
	if ( pszProjectName == NULL )
		return NULL;

	*(pszProjectName++) = '\0';

  IDE_TYPES nIDEType = GetIDETypeByShortName(pszCmdLine);
  if ( nIDEType == IDE_MIN_TYPE_NUM )
    return NULL;
	
	IDEInfo* pIDE = new IDEInfo ( hIDEWnd, nIDEType, pszProjectName );
	if ( pIDE )
		{
		pIDE->SetNextIDE( m_pIDEInfoHead );
		m_pIDEInfoHead	= pIDE;
		}
	
	return pIDE;
}

/****************************************************************************
		 FindMainAppWindow
****************************************************************************/

static HWND hMainAppWindow;
BOOL CALLBACK EnumMainWndProc( HWND  hWnd, LPARAM lParam );

HWND	IDEInfoList::FindMainAppWindow( char* pszCmdLine  )
{
	char*	pszProjectName = strchr( pszCmdLine, ' ');
	if ( pszProjectName == NULL )
		return NULL;

	*pszProjectName = '\0';

  IDE_TYPES nIDEType = GetIDETypeByShortName(pszCmdLine);
  if ( nIDEType == IDE_MIN_TYPE_NUM )
    return NULL;
	
	hMainAppWindow = NULL;
	EnumWindows( (WNDENUMPROC) EnumMainWndProc, (LPARAM)(IDETypesDef[nIDEType].szIDECaption) );

	*pszProjectName = ' ';
	return hMainAppWindow;
}


BOOL CALLBACK EnumMainWndProc( HWND  hWnd, LPARAM lParam )
{

	char lpszWindowTitle[100];
	if( GetWindowTextLength( hWnd ) )
		GetWindowText( hWnd, lpszWindowTitle, 99);
	if ( strstr( lpszWindowTitle, (char *) lParam ) )
	{
		
		hMainAppWindow = hWnd;
		return FALSE;
	}

	return TRUE;
}

/****************************************************************************
		 FindIDEInfo
****************************************************************************/

IDEInfo* IDEInfoList::FindIDEInfoByIDESpyConv	( HCONV hConv )
{
	IDEInfo* pIDE = m_pIDEInfoHead;
	while ( pIDE )
	{
		if ( pIDE->GetIDESpyConv() == hConv )
			break;
		pIDE = pIDE->GetNextIDE();
	}
	
	return pIDE;
}

IDEInfo* IDEInfoList::FindIDEInfoByMainWnd( HWND hWnd )
{
	IDEInfo* pIDE = m_pIDEInfoHead;
	if ( hWnd )
			while ( pIDE )
			{
				if ( pIDE->GetMainIDEWnd() == hWnd )
			 		break;
				pIDE = pIDE->GetNextIDE();
			}
	else
			if ( pIDE && pIDE->GetNextIDE() )
				pIDE == NULL;
	
	return pIDE;
}

char* IDEInfoList::SignalToAllQuery( IDECommand* pIDECmd )
{
	if ( pIDECmd == NULL )
		return NULL;

	IDEInfo*	pIDE = m_pIDEInfoHead;
	char*			pAnswer;
	while ( pIDE )
	{
		if ( (pAnswer = pIDE->SignalToAllQuery(pIDECmd) ) )
			return pAnswer;
		pIDE = pIDE->GetNextIDE();
	}
	return NULL;
}
/****************************************************************************
	Class IDEInfo
*****************************************************************************/
IDEInfo::IDEInfo( HWND hMainIDEWnd, IDE_TYPES nIDEType, char* pszProjectName )
{
	int		iQuery;
	
	m_pIDEInfoNext					= NULL;
	m_nIDEType							= nIDEType;
	m_tProjectTime					= 0L;
	m_hMainIDEWnd						= (HWND)0;
	m_hIDESpyConv						= (HCONV)NULL;
	m_pIDESpyClient					= NULL;
	m_szConfigName[0]				= '\0';

	strcpy( m_szProjectName, pszProjectName );
	m_hMainIDEWnd	= hMainIDEWnd;
	
	for ( iQuery= D_QUERY_DISCSTARTED; iQuery < DI_MAX_QUERYNUM; iQuery++ )
		m_pQueries[iQuery] = NULL;
 
	m_pQueries[D_QUERY_DISCSTARTED]		= new DQueryDiscoverStarted	(m_hMainIDEWnd);
	m_pQueries[D_QUERY_OPENFILE]			= new DQueryOpenFile				(m_hMainIDEWnd);
	m_pQueries[D_QUERY_GETOPTIONS]		= new DQueryCompileOptions	(m_hMainIDEWnd);
	m_pQueries[D_QUERY_STARTIDE]			= new DQueryStartIDE				(m_hMainIDEWnd);
	m_pQueries[D_QUERY_DISPLAYSTATUS]	= new DQueryDisplayStatus		(m_hMainIDEWnd);
	
	m_pQueries[I_QUERY_QUERY]					= new IQueryQuery						(m_hMainIDEWnd);
	m_pQueries[I_QUERY_PROJECTCHANGED]= new IQueryProjectChanged	(m_hMainIDEWnd);
	m_pQueries[I_QUERY_IDEFINISHED]		= new IQueryIDEFinished			(m_hMainIDEWnd);
	m_pQueries[I_QUERY_REFRESHPROJECT]= new IQueryRefreshProject	(m_hMainIDEWnd);
	m_pQueries[I_QUERY_STARTDISCOVER]	= new IQueryStartDiscover		(m_hMainIDEWnd);

	for ( iQuery= D_QUERY_DISCSTARTED; iQuery < DI_MAX_QUERYNUM; iQuery++ )
		if ( m_pQueries[iQuery] )
				 m_pQueries[iQuery]->SetIDEInfo( this );

	char	szBuffer[_MAX_PATH+14];
	GetDisRegistryStr(REG_DISCOVER_DIR, szBuffer, sizeof(szBuffer), "c:\\Integrator\\Bin");
	strcat( szBuffer, EXE_IDESPYLOADER );
	sprintf( &(szBuffer[strlen(szBuffer)]), " %d", m_hMainIDEWnd );
	
	ServerDebugMessageLog( MSG_DBG, "IDEInfo", "Created",
												(char*)(IDETypesDef[m_nIDEType].szIDEShortName),
												(char*)m_szProjectName ); 

	WinExec( szBuffer, SW_HIDE );
}

//------------------------------------------
//
IDEInfo::~IDEInfo()
{
	m_pQueries[I_QUERY_IDEFINISHED]->StartQuery();	
	if ( m_pIDESpyClient )
  {
    m_pIDESpyClient->Disconnect();
	  delete m_pIDESpyClient;
	  m_pIDESpyClient = NULL;
  }

	for ( int iQuery= D_QUERY_DISCSTARTED; iQuery < DI_MAX_QUERYNUM; iQuery++ )
		if ( m_pQueries[iQuery] )
			delete m_pQueries[iQuery]; 

	ServerDebugMessageLog( MSG_DBG, "IDEInfo", "Deleted", 
												(char*)(IDETypesDef[m_nIDEType].szIDEShortName),
												(char*)m_szProjectName ); 

	PostMessage( GetIntegratorMainWnd(), WM_IDESERVERFINISH, NULL, NULL );
}

//------------------------------------------
//
BOOL	IDEInfo::ResetAndStartQuery( Node* pNode ) 
{
	DI_QueryGeneric*	pQuery		= (DI_QueryGeneric*)(pNode->GetData());
	DI_QUERIES	nQueryID = pQuery->GetQueryId();

	for ( int iQuery= D_QUERY_DISCSTARTED; iQuery < DI_MAX_QUERYNUM; iQuery++ )
	{
		if ( iQuery == D_MAX_QUERYNUM )
			continue;
		if ( !m_pQueries[iQuery]->IsQueryCompat(nQueryID) )
			return FALSE;
	}

	delete m_pQueries[nQueryID];
  m_pQueries[nQueryID] = pQuery;
  pQuery->SetIDEInfo( this );

  BOOL b=GetQueueList()->RemoveNode( pNode );
	
  BOOL  bRet = pQuery->StartQuery();
	return TRUE;
}

//------------------------------------------
//
void	IDEInfo::CreateIDESpyClient( char* pszHWND )
{
	m_pIDESpyClient = new DDEIntegratorClient ( DDE_SERVICE_IDESPY );
	if ( m_pIDESpyClient )
		m_pIDESpyClient->ConnectToServer( pszHWND );
}

//------------------------------------------
//
void	IDEInfo::SetProjectName( char* szProjectName )
{
	strcpy( m_szProjectName, szProjectName );
}

//------------------------------------------
//
void	IDEInfo::SetConfigName( char* pszConfigName )
{
  if ( pszConfigName )
  {
    char* pSpace = strchr( pszConfigName, ' ');
	  if ( pSpace )
		  *pSpace = '\0';
	  strcpy( m_szConfigName, pszConfigName );
  }
  else
    m_szConfigName[0] = '\0';
}

//------------------------------------------
//
const char*	IDEInfo::GetIDEShortName()
{
	return (const char*)IDETypesDef[m_nIDEType].szIDEShortName;
}

//------------------------------------------
//
BOOL	IDEInfo::IsQueryPermited(DI_QUERIES nQueryId)
{
	if ( !IsQueryIdCorrect(nQueryId) )
		return FALSE;

	for ( int iQuery= D_QUERY_DISCSTARTED; iQuery < DI_MAX_QUERYNUM; iQuery++ )
	{
		if ( iQuery == D_MAX_QUERYNUM )
			continue;
		if ( m_pQueries[iQuery] && !m_pQueries[iQuery]->IsQueryCompat(nQueryId) )
			return FALSE;
	}
	return TRUE;
}



//------------------------------------------
//
char* IDEInfo::SignalToAllQuery( IDECommand* pIDECmd )
{
	if ( pIDECmd == NULL )
		return NULL;

	char*			pAnswer;
	for ( int iQuery= D_QUERY_DISCSTARTED; iQuery < DI_MAX_QUERYNUM; iQuery++ )
	{
		if ( iQuery == D_MAX_QUERYNUM )
			continue;
		if ( ( pAnswer = m_pQueries[iQuery]->SignalToQuery(pIDECmd) ) )
			return pAnswer;
	}
	return NULL;
}

