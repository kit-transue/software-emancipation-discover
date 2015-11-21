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
#include "integrator.h"

static	DWORD		gDDEClientId					= NULL;
static	int			gnClientCount					= 0;
static	HSZ			ghszIntegratorService	= NULL;
static	HSZ			ghszDebugMessageItem	= NULL;
static	HSZ			ghszCommandItem				= NULL;
static	FARPROC	glpIDEClientCallBack	= NULL;	
static	BOOL		gbServerConnected			= FALSE;	

/***************************************************************************
	Constructor & Destructor
****************************************************************************/
DDEIntegratorClient::DDEIntegratorClient (char* szService)
{
	DDEIntegratorClientInit( szService );
}

DDEIntegratorClient::DDEIntegratorClient ()
{
	DDEIntegratorClientInit( DDE_SERVICE_IDEINFO );
}

void DDEIntegratorClient::DDEIntegratorClientInit (char* pszService)
{
	m_hszClientTopic= NULL;
	m_hConv					= NULL;
	if ( gnClientCount == 0 )
		{
			glpIDEClientCallBack = MakeProcInstance( (FARPROC)DDEIntegratorClientCallBack,
																							GetModuleHandle(NULL) );
			if ( DdeInitialize( (LPDWORD)&gDDEClientId, (PFNCALLBACK)glpIDEClientCallBack,
													APPCMD_CLIENTONLY			 | APPCMD_FILTERINITS | 
													CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS, 0L) )
					return;

			ghszIntegratorService = 
				DdeCreateStringHandle( gDDEClientId, (LPSTR)pszService, CP_WINANSI );
			ghszDebugMessageItem = 
				DdeCreateStringHandle( gDDEClientId, (LPSTR)DDE_ITEM_DEBUGMESSAGE, CP_WINANSI );
			ghszCommandItem = 
				DdeCreateStringHandle( gDDEClientId, (LPSTR)DDE_ITEM_COMMAND, CP_WINANSI );
		}

	gnClientCount++;
}

//-----------------
//
DDEIntegratorClient::~DDEIntegratorClient ()
{
	Disconnect();

	if ( m_hszClientTopic )
		DdeFreeStringHandle( gDDEClientId, m_hszClientTopic);
	m_hszClientTopic				= NULL;

	if ( --gnClientCount == 0 )
		{
		if ( ghszIntegratorService )
			DdeFreeStringHandle( gDDEClientId, ghszIntegratorService);
		if ( ghszDebugMessageItem )
			DdeFreeStringHandle( gDDEClientId, ghszDebugMessageItem);
		if ( ghszCommandItem )
			DdeFreeStringHandle( gDDEClientId, ghszCommandItem);
		ghszIntegratorService	= NULL;
		ghszDebugMessageItem	= NULL;
		ghszCommandItem				= NULL;

		if ( gDDEClientId )
			DdeUninitialize(gDDEClientId);
		gDDEClientId = NULL;
		
		FreeProcInstance(glpIDEClientCallBack);
		glpIDEClientCallBack = NULL;
		}

}

/***************************************************************************
	Connect & Disconnect
****************************************************************************/
void	DDEIntegratorClient::SetClientTopic	( char* pszTopicName )
{
	if ( m_hszClientTopic )
		DdeFreeStringHandle( gDDEClientId, m_hszClientTopic);
	m_hszClientTopic =
			DdeCreateStringHandle( gDDEClientId, (LPSTR)pszTopicName, CP_WINANSI );
}

//--------------------
//
BOOL	DDEIntegratorClient::ConnectToServer ()
{
	Disconnect();
	m_hConv = 
		DdeConnect( gDDEClientId, ghszIntegratorService, m_hszClientTopic, (PCONVCONTEXT)NULL );
	
	if ( m_hConv )
		gbServerConnected = TRUE;
	return (BOOL)m_hConv;
}

//--------------------
//
BOOL	DDEIntegratorClient::ConnectToServer ( char* pszClientName )
{
	SetClientTopic( pszClientName );
	return ConnectToServer();
}

//--------------------
//
void	DDEIntegratorClient::Disconnect ()
{
	if (m_hConv)
	{
		DdeDisconnect(m_hConv);
		m_hConv	= NULL;
	}
}

//--------------------
//
BOOL	DDEIntegratorClient::IsConnected()
{
	return ( gbServerConnected && (BOOL)m_hConv );
}

/***************************************************************************
	DDE Exchange
****************************************************************************/
void	DDEIntegratorClient::SendDebugMessage( UINT nErrLevel, char* pszMessageText )
{
	if (m_hConv == NULL)
		return;

	static char	szBuffer[256];
	_itoa( nErrLevel, szBuffer, 10);
	strcat( szBuffer, ":");
	strcat( szBuffer, pszMessageText);

	DdeClientTransaction( (UCHAR*)szBuffer, strlen(szBuffer)+1,
												m_hConv, ghszDebugMessageItem,
												CF_TEXT, XTYP_POKE, DDE_IDECLIENT_TIMEOUT, NULL);
			
}

//--------------------
//
BOOL	DDEIntegratorClient::RecieveDataFromServer( char* pszDataName, char* pszBuffer, int nBufferLen )
{
	if (m_hConv == NULL)
		return FALSE;

	HSZ	hszNameItem = 
		DdeCreateStringHandle( gDDEClientId, (LPSTR)pszDataName, CP_WINANSI );
	if ( hszNameItem == NULL )
		return FALSE;

	m_dwDDEResult = 0;
	HDDEDATA hData = DdeClientTransaction( NULL, 0, m_hConv, hszNameItem, CF_TEXT, 
																				 XTYP_REQUEST, DDE_IDECLIENT_TIMEOUT, &m_dwDDEResult);
	
	if ( hData )
	{
		 DdeGetData( hData, (UCHAR*)pszBuffer, nBufferLen, 0);
		 DdeFreeDataHandle( hData );
	}
	
	DdeFreeStringHandle( gDDEClientId, hszNameItem);
	return (BOOL)hData;
}


//--------------------
//
BOOL	DDEIntegratorClient::SendDataToServer( char* pszDataName, char* pszData )
{
	if (m_hConv == NULL)
		return FALSE;

	HSZ	hszNameItem = 
		DdeCreateStringHandle( gDDEClientId, (LPSTR)pszDataName, CP_WINANSI );
	if ( hszNameItem == NULL )
		return FALSE;

	HDDEDATA	hData = DdeCreateDataHandle( gDDEClientId, 
																				(UCHAR*)pszData, strlen(pszData)+1,
																				0L, hszNameItem, CF_TEXT, 0);
	if ( hData )
	HDDEDATA  hDataT= DdeClientTransaction( (UCHAR*)hData, (ULONG)-1L,
																					m_hConv, hszNameItem, CF_TEXT,
																					XTYP_POKE, DDE_IDECLIENT_TIMEOUT, NULL);
	DdeFreeDataHandle (hData );
	DdeFreeStringHandle( gDDEClientId, hszNameItem);
	return (BOOL)hData;
}


//--------------------
//
BOOL	DDEIntegratorClient::CommandToServer( IDE_COMMAND iCommand, char* pszParams )
{
	char	szCommand[512];
	_itoa( (int)iCommand, szCommand, 10);
	strcat( szCommand, " ");
	if ( pszParams )
		strcat( szCommand, pszParams );

	HDDEDATA	hData = DdeCreateDataHandle( gDDEClientId, 
																				(UCHAR*)szCommand, strlen(szCommand)+1,
																				0L, ghszCommandItem, CF_TEXT, 0);
	if ( hData )
	HDDEDATA  hDataT= DdeClientTransaction( (UCHAR*)hData, (ULONG)-1L,
																					m_hConv, ghszCommandItem, CF_TEXT,
																					XTYP_POKE, DDE_IDECLIENT_TIMEOUT, NULL);
	return (BOOL)hData;
}

/***************************************************************************
	DDE Callback
****************************************************************************/
HDDEDATA CALLBACK DDEIntegratorClientCallBack
	(
	WORD	wType,
	WORD	wFmt,
	HCONV	hConv,
	HSZ		hsz1,
	HSZ		hsz2,
	HDDEDATA	hData,
 	DWORD	dwData
)
{
	switch (wType)
	{
	case XTYP_DISCONNECT:
		gbServerConnected = FALSE;
		return (HDDEDATA)NULL;
	case XTYP_ERROR:
		break;
	case XTYP_XACT_COMPLETE:
		break;

	case XTYP_ADVDATA:
		break;
	case XTYP_ADVREQ:
		break;
	case XTYP_ADVSTART:
		break;
	case XTYP_ADVSTOP:
		break;
	case XTYP_EXECUTE:
		break;
	case XTYP_CONNECT:
		break;
	case XTYP_CONNECT_CONFIRM:
		break;
	case XTYP_POKE:
		break;
	case XTYP_REGISTER:
		break;
	case XTYP_REQUEST:
		break;
	case XTYP_UNREGISTER:
		break;
	case XTYP_WILDCONNECT :
		break;
	}
	return (HDDEDATA)NULL;
}

