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
#include "..\Include\integrator.h"
#include "idespy.h"
#include <stdio.h>

/***************************************************************************
	Global Variables
****************************************************************************/
static DDEIntegratorClient* pDDEClient = NULL;

static DWORD		gIDESpyServerId = 0L;
static FARPROC	glpIDESpyServiceCallBack;
static HSZ			ghszIDESpyService;
static HSZ			ghszIDESpyTopic;
static HSZ			ghszIDESpyCommandItem;
static HSZ			ghszIDESpyOutputHWNDItem;
static HCONV		ghConvIDESpy;
static char			gszItemBuffer[256];
static char			gszDataBuffer[256];

DDEIntegratorClient*	GetDDEClient()	{ return pDDEClient; }

HDDEDATA CALLBACK IDESpyServiceCallBack(WORD,WORD,HCONV,HSZ,HSZ,HDDEDATA,DWORD,DWORD);
HDDEDATA SendDataToIntegratorClient(HSZ);
HDDEDATA RecieveCommandFromIntegratorClient(HDDEDATA);

/****************************************************************************
	IDESpy Server DDE Start
****************************************************************************/

BOOL DDEServerStart(HWND hIDEMainWnd)
{
	BOOL	bConnect, bSend;
	glpIDESpyServiceCallBack = MakeProcInstance( (FARPROC)IDESpyServiceCallBack,
																								GetModuleHandle(NULL));
	if ( DdeInitialize( (LPDWORD)&gIDESpyServerId,
						(PFNCALLBACK)glpIDESpyServiceCallBack,
						APPCLASS_STANDARD | CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS, 0L)
	   )
	{
		return FALSE;
	}
	else
	{
		char	szHWND[10];
		_itoa( (int)hIDEMainWnd, szHWND, 10 );

		ghszIDESpyService = 
			DdeCreateStringHandle( gIDESpyServerId, (LPSTR)DDE_SERVICE_IDESPY, CP_WINANSI );
		ghszIDESpyTopic = 
			DdeCreateStringHandle( gIDESpyServerId, (LPSTR)szHWND, CP_WINANSI );
		ghszIDESpyCommandItem =
			DdeCreateStringHandle( gIDESpyServerId, (LPSTR)DDE_ITEM_COMMAND, CP_WINANSI );
		ghszIDESpyOutputHWNDItem =
			DdeCreateStringHandle( gIDESpyServerId, (LPSTR)DDE_ITEM_OUTPUTHWND, CP_WINANSI );
		
		ghConvIDESpy = NULL;
		DdeNameService( gIDESpyServerId, ghszIDESpyService, (HSZ)NULL, DNS_REGISTER );

		pDDEClient = new DDEIntegratorClient;
		if( pDDEClient )
		{
			bConnect = pDDEClient->ConnectToServer( DDE_TOPIC_IDESPY);
			bSend		 = pDDEClient->SendDataToServer( DDE_TOPIC_CONTROL, szHWND );
		}

		return ((BOOL)pDDEClient) && bConnect && bSend;
	}
}

/****************************************************************************
	IntegratorServer DDE Finish
****************************************************************************/

void DDEServerFinish	()
{
//	_asm int 3;
	if ( pDDEClient )
		{
		pDDEClient->Disconnect();
		delete pDDEClient;
		pDDEClient = NULL;
		}

	if (ghConvIDESpy)
	{
		DdeDisconnect(ghConvIDESpy);
		ghConvIDESpy = NULL;
	}

	if ( gIDESpyServerId )
	{
		DdeNameService( gIDESpyServerId, ghszIDESpyService, (HSZ)NULL, DNS_UNREGISTER );

		DdeFreeStringHandle( gIDESpyServerId, ghszIDESpyService);
		DdeFreeStringHandle( gIDESpyServerId, ghszIDESpyTopic);
		DdeFreeStringHandle( gIDESpyServerId, ghszIDESpyCommandItem);
		DdeFreeStringHandle( gIDESpyServerId, ghszIDESpyOutputHWNDItem);

		FreeProcInstance(glpIDESpyServiceCallBack);
		DdeUninitialize(gIDESpyServerId);
		gIDESpyServerId = 0L;
	}

}

/****************************************************************************
	IDESpy CallBack Service
****************************************************************************/

HDDEDATA CALLBACK IDESpyServiceCallBack
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
	switch (wType)
	{
	case XTYP_CONNECT:						// Create Conversation
		if ( hsz2 != ghszIDESpyService )
			return (HDDEDATA)FALSE;
		if ( hsz1 == ghszIDESpyTopic )
			return (HDDEDATA)TRUE;
		else 
			return (HDDEDATA)FALSE;
		
	case XTYP_CONNECT_CONFIRM:				// Confirm conversation creation
		ghConvIDESpy = hConv;
		break;
		
	case XTYP_DISCONNECT:
		ghConvIDESpy = NULL;
		
		//Cause IDESpy to unload
		fClosing = TRUE;
		if( ghDiscoverWindow )
			DestroyWindow( ghDiscoverWindow );
		else 
			SignalFinish();
		break;

	case XTYP_ERROR:
		break;

	case XTYP_REQUEST:						// Data request
		if ( ghConvIDESpy == hConv && hsz1 == ghszIDESpyTopic )
			return SendDataToIntegratorClient( hsz2 );
		else
			return NULL;
		
	case XTYP_EXECUTE:						// Execution request
		break;

	case XTYP_POKE:							// Send data to server
		if ( ghConvIDESpy == hConv && hsz1 == ghszIDESpyTopic && hsz2 == ghszIDESpyCommandItem )
			return RecieveCommandFromIntegratorClient( hData );
		else
			return (HDDEDATA)FALSE;
		
	}

	return (HDDEDATA)NULL;
}

/*************************************************************************************
	DDE Exchange
*************************************************************************************/
HDDEDATA SendDataToIntegratorClient(HSZ hszItem)
{
	HDDEDATA hData = NULL;

	if ( hszItem == ghszIDESpyOutputHWNDItem )
		{
			_itoa( (int)ghOutputWnd, gszDataBuffer, 10 );
			hData = DdeCreateDataHandle(gIDESpyServerId, (UCHAR*)gszDataBuffer,
																			 strlen(gszDataBuffer)+1,	0L, hszItem, CF_TEXT, 0);
		}
	return hData;
}

//---------------------
//
HDDEDATA RecieveCommandFromIntegratorClient(HDDEDATA hData)
{
DdeGetData(hData, (UCHAR*)gszDataBuffer, sizeof(gszDataBuffer), 0L);

	pDDEClient->SendDebugMessage( MSG_DBG, gszDataBuffer );

	char*				pszParams = strchr( gszDataBuffer, ' ');
	IDE_COMMAND	iIDECmd		= (IDE_COMMAND)atoi(gszDataBuffer);
	switch (iIDECmd)
		{
		case 	IDE_CMD_DSPSTATUS:
			ChangeStatusLine(++pszParams);
			break;

		case IDE_CMD_CHOOSE_CUSTOM_MENU:
			ChooseMyCustomMenu( ghDeveloperStudioWnd, ++pszParams );
			break;

		case IDE_CMD_DDECLIENT_DISCONNECT:
			if( ghWorkerWindow && IsWindow( ghWorkerWindow ) )
				PostMessage( ghWorkerWindow, WM_USERTERMINATE, 0L, 0L );
			break;

		case IDE_CMD_DEBUG:
 			IDESpyDebug();
			break;
		}
		
	return (HDDEDATA)DDE_FACK;
}

