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
#include "Server.h"
#include <stdio.h>

/***************************************************************************
	Global Variables
****************************************************************************/
static char			gszProjectIdName[256];

static DWORD		gIDEInfoServerId					= NULL;
static FARPROC	glpIDEInfoServiceCallBack	= NULL;
static HSZ			ghszIDEInfoService				= NULL;
static HSZ			ghszControlTopic					= NULL;
static HSZ			ghszConsoleTopic					= NULL;
static HSZ			ghszParamRecieverTopic		= NULL;
static HSZ			ghszIDESpyTopic						= NULL;
static HSZ			ghszDebugMessageItem			= NULL;
static HSZ			ghszCommandItem						= NULL;
static HCONV		ghConvIDEInfo							= NULL;

DWORD	GetDDEIDEInfoServiceID() { return gIDEInfoServerId; }

void  DebugServerDDE  ()
{
  _asm int 3;
}

/****************************************************************************
	Check previous copy of IntegratorServer 
****************************************************************************/

BOOL DDEServerCheckInstance()
{
	BOOL	bFirstServerInstance = TRUE;
	DDEIntegratorClient* pDDEClient = new DDEIntegratorClient;
	if ( pDDEClient == NULL )
		return FALSE;

	if ( pDDEClient->ConnectToServer(DDE_TOPIC_CONTROL) )
		{
		bFirstServerInstance = FALSE;
		pDDEClient->SendDebugMessage( 4, "Attempt to start secondary IntegratorServer" );
		}

	pDDEClient->Disconnect();
	delete pDDEClient;
	return bFirstServerInstance;
}

/****************************************************************************
	IntegratorServer DDE Start
****************************************************************************/

BOOL DDEServerStart()
{
	HINSTANCE hInstance= GetServerHInstance();
	gIDEInfoServerId = 0L;

	glpIDEInfoServiceCallBack = MakeProcInstance( (FARPROC)IDEInfoServiceCallBack, hInstance);
	if ( DdeInitialize( (LPDWORD)&gIDEInfoServerId,
						(PFNCALLBACK)glpIDEInfoServiceCallBack,
						APPCLASS_STANDARD | CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS, 0L)
	   )
	{
		return FALSE;
	}
	else
	{
		ghszIDEInfoService = 
			DdeCreateStringHandle( gIDEInfoServerId, (LPSTR)DDE_SERVICE_IDEINFO, CP_WINANSI );

		ghszControlTopic =
			DdeCreateStringHandle( gIDEInfoServerId, (LPSTR)DDE_TOPIC_CONTROL, CP_WINANSI );

		ghszDebugMessageItem = 
			DdeCreateStringHandle( gIDEInfoServerId, (LPSTR)DDE_ITEM_DEBUGMESSAGE, CP_WINANSI );

		ghszCommandItem	 = 
			DdeCreateStringHandle( gIDEInfoServerId, (LPSTR)DDE_ITEM_COMMAND, CP_WINANSI );

		ghszConsoleTopic =
			DdeCreateStringHandle( gIDEInfoServerId, (LPSTR)DDE_TOPIC_CONSOLE, CP_WINANSI );

		ghszParamRecieverTopic =
			DdeCreateStringHandle( gIDEInfoServerId, (LPSTR)DDE_TOPIC_PARAMRECIEVER, CP_WINANSI );

		ghszIDESpyTopic = 
			DdeCreateStringHandle( gIDEInfoServerId, (LPSTR)DDE_TOPIC_IDESPY, CP_WINANSI );

		ghConvIDEInfo = NULL;
		DdeNameService( gIDEInfoServerId, ghszIDEInfoService, (HSZ)NULL, DNS_REGISTER );

		return TRUE;
	}
}

/****************************************************************************
	IntegratorServer DDE Finish
****************************************************************************/

void DDEServerFinish	()
{
	if (ghConvIDEInfo)
	{
		DdeDisconnect(ghConvIDEInfo);
		ghConvIDEInfo = NULL;
	}

	DdeNameService( gIDEInfoServerId, ghszIDEInfoService, (HSZ)NULL, DNS_UNREGISTER );

	DdeFreeStringHandle( gIDEInfoServerId, ghszIDEInfoService);
	DdeFreeStringHandle( gIDEInfoServerId, ghszControlTopic);
	DdeFreeStringHandle( gIDEInfoServerId, ghszConsoleTopic);
	DdeFreeStringHandle( gIDEInfoServerId, ghszParamRecieverTopic);
	DdeFreeStringHandle( gIDEInfoServerId, ghszIDESpyTopic);
	DdeFreeStringHandle( gIDEInfoServerId, ghszDebugMessageItem);
	DdeFreeStringHandle( gIDEInfoServerId, ghszCommandItem);

	FreeProcInstance(glpIDEInfoServiceCallBack);
	DdeUninitialize(gIDEInfoServerId);
}

/****************************************************************************
	IDEInfo CallBack Service
****************************************************************************/

HDDEDATA CALLBACK IDEInfoServiceCallBack
(
	WORD			wType,
	WORD			wFmt,
	HCONV			hConv,
	HSZ				hsz1,
	HSZ				hsz2,
	HDDEDATA	hData,
	DWORD			dwData1,
	DWORD			dwData2
)
{
	IDEInfo*	pIDEInfo = NULL;

	switch (wType)
	{
	case XTYP_CONNECT:						// Create Conversation
		if ( hsz2 != ghszIDEInfoService )
			return (HDDEDATA)FALSE;
		if ( hsz1 == ghszControlTopic )
			return (HDDEDATA)TRUE;
		if ( hsz1 == ghszParamRecieverTopic || hsz1 == ghszConsoleTopic	)
			return (HDDEDATA)TRUE;
		if ( hsz1 == ghszIDESpyTopic )
			return (HDDEDATA)TRUE;
		else 
			return (HDDEDATA)FALSE;
		
	case XTYP_CONNECT_CONFIRM:				// Confirm conversation creation
		ghConvIDEInfo = hConv;
		break;
		
	case XTYP_DISCONNECT:
		pIDEInfo = GetIDEInfoList()->FindIDEInfoByIDESpyConv( hConv );
		if ( pIDEInfo )
			GetIDEInfoList()->DestroyIDEInfoObject( pIDEInfo );
		else
			SignalToAllQuery( IDE_MSG_DDECLIENT_DISCONNECTED, NULL, hConv, NULL);
		ghConvIDEInfo = NULL;
		break;

	case XTYP_ERROR:
		break;

	case XTYP_REQUEST:						// Data request
		if ( hsz1 == ghszConsoleTopic )
			return SendIDEConsoleData( hConv, hsz2 );
		if ( hsz1 == ghszIDESpyTopic )
			return SendIDESpyData( hConv, hsz2 );
		else
			return NULL;
		
	case XTYP_EXECUTE:						// Execution request
		break;

	case XTYP_POKE:							// Send data to server
		if ( hsz2 == ghszDebugMessageItem )
			return RecieveDebugMessage( hsz1, hData );
		if ( hsz1 == ghszParamRecieverTopic )
			return RecieveIDEParam( hData, hConv, hsz2 );
		if ( hsz1 == ghszIDESpyTopic )
			return RecieveIDESpyData( hData, hConv, hsz2 );
		else
			return (HDDEDATA)FALSE;
		
	}

	return (HDDEDATA)NULL;
}

//--------------------
//
IDEInfo*	FindCurrentIDEInfoByProjectItem ( HSZ hszItem )
{
	HWND				hWnd;

	DdeQueryString( gIDEInfoServerId, hszItem, gszProjectIdName, sizeof(gszProjectIdName), CP_WINANSI );
	hWnd = GetIDEInfoList()->FindMainAppWindow( gszProjectIdName);
	if ( hWnd == NULL )
		return NULL;

	return GetIDEInfoList()->FindIDEInfoByMainWnd( hWnd );
}

/****************************************************************************
	Debug Messager
****************************************************************************/

HDDEDATA RecieveDebugMessage( HSZ hszTopic, HDDEDATA	hData )
{
	char				szMessageBuffer[256];
	char				szTopic[32];
	char				*pc, *pszMessageText =(char*)szMessageBuffer, *pszTopic = (char*)szTopic; 
	UINT				nErrLevel = 0;
		
	DdeGetData(hData, (UCHAR*)pszMessageText, sizeof(szMessageBuffer), 0L);
	if ( hData != NULL)
		{
		pc = strchr( pszMessageText, ':');
		if ( pc )
			{
			*pc = '\0';
			nErrLevel = atoi((const char*)pszMessageText);
			if ( nErrLevel )
				pszMessageText = pc+1;
			else
				pszMessageText = (char*)szMessageBuffer;
			}

		if ( !DdeQueryString( gIDEInfoServerId, hszTopic, pszTopic, sizeof(szTopic), CP_WINANSI ) )
			pszTopic = NULL;
		ServerDebugMessageLog( nErrLevel, pszTopic, "DDE", pszMessageText );
		}
	
	return (HDDEDATA)DDE_FACK;
}

/****************************************************************************
	IDEParamReciever
****************************************************************************/

HDDEDATA RecieveIDEParam( HDDEDATA	hData, HCONV hConv, HSZ hszItem )
{
	IDEInfo			*pIDEInfo;
	char				szParamBuffer[256];

	DdeGetData(hData, (UCHAR*)szParamBuffer, sizeof(szParamBuffer), 0L);
	pIDEInfo = FindCurrentIDEInfoByProjectItem( hszItem );
	ServerDebugMessageLog( MSG_DBG, "PrmReciever", "DDE", gszProjectIdName, szParamBuffer );

	if ( pIDEInfo == NULL )
		{
		MessageBox( NULL, "Please connect to Discover", "IntegratorServer", MB_OK);
		return (HDDEDATA)NULL;
		}
	
	SignalToAllQuery( IDE_MSG_PARAMS_RECIEVED, szParamBuffer, hConv, pIDEInfo->GetMainIDEWnd());
	return (HDDEDATA)DDE_FACK;
}

/****************************************************************************
	IDEConsole
****************************************************************************/

HDDEDATA SendIDEConsoleData( HCONV hConv, HSZ hszItem )
{
	IDECommand*	pIDECmd;
  IDEInfo			*pIDEInfo;
	HDDEDATA		hData;
	char				*pszData, szStartng[]=EXTMSG_IDE_STARTING, szEmpty[] = "  ";

	pIDEInfo = FindCurrentIDEInfoByProjectItem( hszItem );

	if ( pIDEInfo == NULL )	// If new IDE ..... 
		{
     HWND    hWnd = NULL;
	   if ( gszProjectIdName )
	      hWnd = GetIDEInfoList()->FindMainAppWindow(gszProjectIdName);

     if ( hWnd )
     {
  		hData = DdeCreateDataHandle(gIDEInfoServerId, (UCHAR*)szStartng, strlen(szStartng)+1,
													0L, hszItem, CF_TEXT, 0);
      PostMessage( GetIntegratorMainWnd(), WM_CREATE_NEW_IDEINFO, 
                         (WPARAM)hWnd, (LPARAM)gszProjectIdName );
     }
     else
     {
		  hData = DdeCreateDataHandle(gIDEInfoServerId, (UCHAR*)szEmpty, strlen(szEmpty)+1,
													0L, hszItem, CF_TEXT, 0);
     }

    return hData;
		}

	//-------------
  pIDECmd = new IDECommand;
	if ( pIDECmd == NULL )
			return (HDDEDATA)NULL;

	pIDECmd->hConvFrom	= hConv;
	pIDECmd->iIDECommnd	= IDE_MSG_CONSOLE_STARTED;
	pIDECmd->pszParams	= NULL;
	pIDECmd->nParamsLen	= 0;
	pszData = pIDEInfo->SignalToAllQuery( pIDECmd );
	delete pIDECmd;

	if ( pszData == NULL )
		pszData = (char*)szEmpty;

	hData = DdeCreateDataHandle(gIDEInfoServerId, (UCHAR*)pszData, strlen(pszData)+1,
															0L, hszItem, CF_TEXT, 0);
	return hData;
}

/****************************************************************************
	IDESpy
****************************************************************************/

HDDEDATA SendIDESpyData( HCONV hConv, HSZ hszItem )
{
	IDEInfo			*pIDEInfo;
	HDDEDATA		hData;

	pIDEInfo = GetIDEInfoList()->FindIDEInfoByIDESpyConv( hConv );
	if ( pIDEInfo == NULL )
		{
		ServerDebugMessageLog( 0, "IDESpy", "DDE-Send", "Wrong hConv/pIDEInfo" );
		return (HDDEDATA)NULL;
		}

	hData = NULL;
//	char				*pszData;
//	hData = DdeCreateDataHandle(gIDEInfoServerId, (UCHAR*)pszData, strlen(pszData)+1,
//															0L, hszItem, CF_TEXT, 0);
	return hData;
}

//-------------------
//
HDDEDATA RecieveIDESpyData( HDDEDATA	hData, HCONV hConv, HSZ hszItem )
{
	IDEInfo			*pIDEInfo;
	char				szBuffer[256];

	DdeGetData(hData, (UCHAR*)szBuffer, sizeof(szBuffer), 0L);
	if ( hszItem == ghszControlTopic )
		{
		pIDEInfo = GetIDEInfoList()->FindIDEInfoByMainWnd( (HWND)(atoi(szBuffer)) );
		if ( pIDEInfo == NULL )
			{
			ServerDebugMessageLog( 4, "IDESpy", "DDE-Recieve", "Wrong hWnd at connect:", szBuffer );
			return (HDDEDATA)NULL;
			}

		pIDEInfo->SetIDESpyConv( hConv );
		pIDEInfo->CreateIDESpyClient( (char*)szBuffer );

		return (HDDEDATA)DDE_FACK;
		}

	else if ( hszItem == ghszCommandItem )
		{
		pIDEInfo = GetIDEInfoList()->FindIDEInfoByIDESpyConv( hConv );
		if ( pIDEInfo == NULL )
				{
				ServerDebugMessageLog( 4, "IDESpy", "DDE", "Wrong hConv/pIDEInfo" );
				return (HDDEDATA)NULL;
				}
		ServerDebugMessageLog( MSG_DBG, "IDESpy", "Command", szBuffer );

		char* pszParams = strchr( szBuffer, ' ');
		if ( pszParams )
			{
				IDE_COMMAND IDECmd = (IDE_COMMAND)atoi(szBuffer);
				if ( IDECmd == IDE_CMD_SHOW_INTEGRATOR )
					ShowWindow(GetIntegratorMainWnd(), SW_SHOW);
				else
					SignalToAllQuery( IDECmd, ++pszParams, hConv, pIDEInfo->GetMainIDEWnd());
			}
		return (HDDEDATA)DDE_FACK;
		}
	else
		return (HDDEDATA)NULL;
}

