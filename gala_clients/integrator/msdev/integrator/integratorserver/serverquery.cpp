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
#include <stdio.h>
#include "server.h"
#include "mapnet.h"

/***************************************************************************
	Global Variables
****************************************************************************/

static const BOOL	gfQueryCompat[DI_MAX_QUERYNUM][DI_MAX_QUERYNUM] = 
{
{FALSE,FALSE,FALSE,FALSE,FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, }, //D_QUERY_DISCSTARTED,	
{FALSE,FALSE,FALSE,FALSE,FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, }, //D_QUERY_OPENFILE,
{FALSE,FALSE,FALSE,FALSE,FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, }, //D_QUERY_GETOPTIONS,
{FALSE,FALSE,FALSE,FALSE,FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, }, //D_QUERY_STARTIDE,
{FALSE,FALSE,FALSE,FALSE,FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, }, //D_QUERY_DISPLAYSTATUS,

{TRUE, TRUE, TRUE, TRUE, TRUE,  TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, }, //D_MAX_QUERYNUM,
			
{TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE,FALSE,FALSE,FALSE, }, //I_QUERY_QUERY,
{TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE,FALSE,FALSE,FALSE, }, //I_QUERY_PROJECTCHANGED,
{TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE,FALSE,FALSE,FALSE, }, //I_QUERY_IDEFINISHED,
{TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE,FALSE,FALSE,FALSE, }, //I_QUERY_REFRESHPROJECT,
{TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE,FALSE,FALSE,FALSE, }, //I_QUERY_STARTDISCOVER,
};

BOOL IsQueryIdCorrect(int n)
{
	return (0<=(n)&&(n)<DI_MAX_QUERYNUM&&(n)!=D_MAX_QUERYNUM); 
}

void  DebugServerQuery  ()
{
  _asm int 3;
}

/***************************************************************************
	DI_QueryGeneric
****************************************************************************/
//
DI_QueryGeneric::DI_QueryGeneric()
{
	m_hMainIDEWnd		= NULL;
	m_pIDEInfo			= NULL;
	m_nQueryId			= D_MAX_QUERYNUM;
	m_nQueryStatus	= QUERY_STATUS_SLEEP;
	m_hConv					= NULL;
}

DI_QueryGeneric::~DI_QueryGeneric()
{
  m_nQueryId	= D_MAX_QUERYNUM;
}

DI_QueryGeneric::DI_QueryGeneric(HWND hWnd) 
{
	m_hMainIDEWnd		= hWnd;
	m_pIDEInfo			= NULL;
	m_nQueryId			= D_MAX_QUERYNUM;
	m_nQueryStatus	= QUERY_STATUS_SLEEP;
	m_hConv					= NULL;
}

BOOL	DI_QueryGeneric::IsQueryCompat(DI_QUERIES nQueryId)
{
	if ( m_nQueryStatus	== QUERY_STATUS_SLEEP )
		return TRUE;
	else if ( IsQueryIdCorrect(nQueryId) )
		return gfQueryCompat[m_nQueryId][nQueryId];
	else
		return FALSE;
}

/***************************************************************************
	DQueryDiscoverStarted
****************************************************************************/
//
DQueryDiscoverStarted::DQueryDiscoverStarted(HWND hWnd) : DI_QueryGeneric(hWnd)
{
	m_nQueryId	= D_QUERY_DISCSTARTED;
}

//---------------------------
//
BOOL DQueryDiscoverStarted::StartQuery()
{
	if ( m_pIDEInfo == NULL )
		return FALSE;

	DDEIntegratorClient* pClient= m_pIDEInfo->GetIDESpyClient();
	if ( pClient && pClient->IsConnected() )
		pClient->CommandToServer(IDE_CMD_DSPSTATUS, EXTMSG_DIS_STARTED );

	PostMessage( GetIntegratorMainWnd(), WM_LOOKNEWQUERY, NULL, NULL );

	return TRUE;
}


//---------------------------
//
char* DQueryDiscoverStarted::SignalToQuery(IDECommand* pIDECmd)
{
	return NULL;
}

/***************************************************************************
	DQueryOpenFile
****************************************************************************/
//
DQueryOpenFile::DQueryOpenFile(HWND hWnd, char *pszFilePath, char* szLineNumber) 
: DI_QueryGeneric(hWnd)
{
  ServerDebugMessageLog( MSG_DBG, "DQueryOpenFile", "Create", pszFilePath, szLineNumber);
	m_nQueryId	= D_QUERY_OPENFILE;
  m_bNetMapFlag = TRUE;
	
  int   nNameTransResult;
  m_pszFilePath[0]='\0';
  if( pszFilePath )
  {
    nNameTransResult = UnixNetName2LocalMapped( GetServerHInstance(), GetIntegratorMainWnd(),
                                pszFilePath, m_pszFilePath, sizeof(m_pszFilePath) );
   if ( nNameTransResult != NAME_SUCCESS )
   {
     m_bNetMapFlag = FALSE;
    ServerDebugMessageLog( MSG_DBG, "DQueryOpenFile", "FNameTrans", EXTMSG_ERR_NETMAP );
   }
   else
   {
		sprintf( &(m_pszFilePath[strlen(m_pszFilePath)]), "(%s): ", szLineNumber );
    ServerDebugMessageLog( MSG_DBG, "DQueryOpenFile", "FNameTrans", m_pszFilePath );
   }
  }
}

//---------------------
//
BOOL DQueryOpenFile::StartQuery()
{
	if ( m_pIDEInfo == NULL || m_pszFilePath[0] == '\0' )
		return FALSE;

	m_nQueryStatus = QUERY_STATUS_WAIT_IDESPY;
	DDEIntegratorClient* pClient= m_pIDEInfo->GetIDESpyClient();
	if ( pClient && pClient->IsConnected() )
			pClient->CommandToServer(IDE_CMD_CHOOSE_CUSTOM_MENU, EXE_IDECONSOLE);
	m_nQueryStatus = QUERY_STATUS_SLEEP;
	return TRUE;
}


//---------------------
//
char* DQueryOpenFile::SignalToQuery(IDECommand* pIDECmd)
{
	char	szBuffer[256];

	switch ( pIDECmd->iIDECommnd )
		{
		case IDE_MSG_CONSOLE_STARTED:
			if ( m_hConv == NULL && m_nQueryStatus == QUERY_STATUS_SLEEP  && m_pIDEInfo )
			{
				m_nQueryStatus = QUERY_STATUS_WAIT_DDECLIENT_DISCONNECT;
        ServerDebugMessageLog( MSG_DBG, "DQueryOpenFile", "CONSOLE_STARTED", m_pszFilePath);
				if ( m_pszFilePath[0] == '\0' )
					strcpy( m_pszFilePath, " ");

				m_hConv = pIDECmd->hConvFrom;
				return m_pszFilePath;
			}
			break;

		case IDE_MSG_DDECLIENT_DISCONNECTED:
			if ( m_hConv == pIDECmd->hConvFrom && 
					 m_nQueryStatus == QUERY_STATUS_WAIT_DDECLIENT_DISCONNECT  && m_pIDEInfo )
			{
        ServerDebugMessageLog( MSG_DBG, "DQueryOpenFile", "CONSOLE_FINISHED", m_pszFilePath);
				m_hConv = NULL;
				m_pszFilePath[0]='\0';

				DDEIntegratorClient* pClient= m_pIDEInfo->GetIDESpyClient();
				if ( pClient && pClient->IsConnected() &&
						 pClient->RecieveDataFromServer	( DDE_ITEM_OUTPUTHWND, szBuffer, sizeof(szBuffer) )
					 )
        {
          // Activate Visual C++ window and restore its size if necessary
          HWND hToActivate = m_pIDEInfo->GetMainIDEWnd();
          if( hToActivate && IsWindow( hToActivate ) )
          {
            SetForegroundWindow( hToActivate );
            if( IsIconic( hToActivate ) ) // We use WM_SYSCOMMAND instead of ShowWindow()
                                          // because IDE handles this event and brings all
                                          // its popups to top
              SendMessage( hToActivate, WM_SYSCOMMAND, (WPARAM) SC_RESTORE, 0L);
          }
          MakeDoubleClick ( (HWND)atoi(szBuffer), 
												GetDisRegistryInt(REG_DBLCLICK_TIMEOUT, 500), 3, 3 );
        }
				else
				  ServerDebugMessageLog( 0, "DQueryOpenFile", "CONSOLE_FINISHED", "IDESpy unconnected");
			
				PostMessage( GetIntegratorMainWnd(), WM_LOOKNEWQUERY, NULL, NULL );
 				m_nQueryStatus = QUERY_STATUS_SLEEP;
				return (char*)1;
			}
			break;

		default:
			break;
	}

	return NULL;
}

/***************************************************************************
	DQueryCompileOptions
****************************************************************************/
//
DQueryCompileOptions::DQueryCompileOptions(HWND hWnd, char *pszFilePath) 
: DI_QueryGeneric(hWnd)
{
	if( pszFilePath )
		strcpy( m_pszFilePath, pszFilePath );
	else
		m_pszFilePath[0]='\0';
	m_nQueryId	= D_QUERY_GETOPTIONS;
}

//---------------------
//
BOOL DQueryCompileOptions::StartQuery()
{
	return FALSE;
}

char* DQueryCompileOptions::SignalToQuery(IDECommand* pIDECmd )
{
	return NULL;
}

/***************************************************************************
	DQueryStartIDE
****************************************************************************/
//
DQueryStartIDE::DQueryStartIDE(HWND hWnd, char * pszProjectPath) 
: DI_QueryGeneric(hWnd)
{
	m_nQueryId	= D_QUERY_STARTIDE;
	if( pszProjectPath )
		strcpy( m_pszProjectPath, pszProjectPath );
	else
		m_pszProjectPath[0]='\0';
}

//---------------------
//
BOOL DQueryStartIDE::StartQuery()
{
	return FALSE;
}

char* DQueryStartIDE::SignalToQuery(IDECommand* pIDECmd)
{
	return NULL;
}

/***************************************************************************
	DQueryDisplayStatus
****************************************************************************/
//
DQueryDisplayStatus::DQueryDisplayStatus(HWND hWnd, char *pszStatusText) 
: DI_QueryGeneric(hWnd)
{
  ServerDebugMessageLog( MSG_DBG, "DQueryDsplStatus", "Create", pszStatusText);
	m_nQueryId	= D_QUERY_DISPLAYSTATUS;
	if( pszStatusText )
	{	
		m_pszStatusText = new char [ strlen( pszStatusText ) + 1 ]; 
		strcpy( m_pszStatusText, pszStatusText );		
	}
	else
		m_pszStatusText = NULL;
}

//---------------------
//
DQueryDisplayStatus::~DQueryDisplayStatus()
{
	if ( m_pszStatusText )
	{
		delete [] m_pszStatusText;
		m_pszStatusText = NULL;
	}
  m_nQueryId	= D_MAX_QUERYNUM;
}

BOOL DQueryDisplayStatus::SetStatusText( char* pszStatusText )
{
	int nStatusTextLen = strlen( pszStatusText );
	if ( nStatusTextLen == 0 )
		return FALSE;

	if ( m_pszStatusText )
	{
		delete [] m_pszStatusText;
		m_pszStatusText = NULL;
	}
	m_pszStatusText = new char [ nStatusTextLen+1 ];
	if ( m_pszStatusText == NULL )
		return FALSE;

	strcpy ( m_pszStatusText, pszStatusText );
	return TRUE;
}

BOOL DQueryDisplayStatus::StartQuery()
{
  ServerDebugMessageLog( MSG_DBG, "DQueryDsplStatus", "StartQuery", m_pszStatusText);
	if ( m_pIDEInfo == NULL )
		return FALSE;

	if ( m_pszStatusText )
		{
		m_nQueryStatus = QUERY_STATUS_WAIT_IDESPY;
		DDEIntegratorClient* pClient= m_pIDEInfo->GetIDESpyClient();
		if ( pClient && pClient->IsConnected() )
			pClient->CommandToServer(IDE_CMD_DSPSTATUS, m_pszStatusText);
		else
		  ServerDebugMessageLog( 0, "DQueryDsplStatus", "StartQuery", "IDESpy unconnected");

		delete [] m_pszStatusText;
		m_pszStatusText = NULL;
		PostMessage( GetIntegratorMainWnd(), WM_LOOKNEWQUERY, NULL, NULL );
		m_nQueryStatus = QUERY_STATUS_SLEEP;
		}
	return TRUE;
}

char* DQueryDisplayStatus::SignalToQuery(IDECommand* pIDECmd)
{
	return NULL;
}

/***************************************************************************
	IQueryQuery
****************************************************************************/
//
IQueryQuery::IQueryQuery(HWND hWnd) : DI_QueryGeneric(hWnd)
{
	m_nQueryId		= I_QUERY_QUERY;
}


//---------------------
//
BOOL IQueryQuery::StartQuery()
{
	return FALSE;
}

char* IQueryQuery::SignalToQuery(IDECommand* pIDECmd)
{
  char	szBuffer[2*_MAX_PATH+48];

	switch ( pIDECmd->iIDECommnd )
	{
    case IDE_CMD_IMPACT_ANALYSIS:		
    {
			if ( m_pIDEInfo )
			{
        ServerDebugMessageLog( MSG_DBG, "IQueryQuery", "IMPACT_ANALYSIS");
	      sprintf(szBuffer, "%s %d", 
											      TCL_IDE_IMPACT,
											      m_pIDEInfo->GetMainIDEWnd());
	      sendDis( GetSocketOut(), szBuffer, 0);
      }
			break;
    }

    case IDE_MSG_DBLCKICKED:
    {
			if ( m_pIDEInfo && pIDECmd->nParamsLen >0 && pIDECmd->pszParams )
			{
  	  char* pSpace = strchr( pIDECmd->pszParams, ' ');
	    if ( pSpace )
		    *pSpace = '\0';

			if ( strcmp(m_pIDEInfo->GetConfigName(), pIDECmd->pszParams) != 0)
				{
					m_pIDEInfo->SetConfigName(pIDECmd->pszParams);
					m_pIDEInfo->GetIQueryRefreshProject()->StartQuery();
				}
			}

			m_nQueryStatus = QUERY_STATUS_WAIT_IDESPY;
			DDEIntegratorClient* pClient= m_pIDEInfo->GetIDESpyClient();
			if ( pClient && pClient->IsConnected() )
					pClient->CommandToServer(IDE_CMD_CHOOSE_CUSTOM_MENU, EXE_PARAMRECIEVER);
			else
				  ServerDebugMessageLog( 0, "IQueryQuery", "DBLCKICKED", "IDESpy unconnected");

			m_nQueryStatus = QUERY_STATUS_SLEEP;
			break;
    }

		case IDE_MSG_PARAMS_RECIEVED:
    {
			m_nQueryStatus = QUERY_STATUS_WAIT_IDESPY;
			if ( m_pIDEInfo && pIDECmd->nParamsLen >0 && pIDECmd->pszParams )
			{
				char  *pParam = strchr(pIDECmd->pszParams, ' ');
        int   nNameTransResult, iEndPos;

        ServerDebugMessageLog( MSG_DBG, "IQueryQuery", "PARAMS_RECIEVED", pIDECmd->pszParams);
				sprintf(szBuffer, "%s %d ",	TCL_IDE_GETSTATUS, m_pIDEInfo->GetMainIDEWnd());
        iEndPos = strlen( szBuffer );

        if ( pParam )
        {
          *(pParam++) = '\0';
          nNameTransResult = LocalMapped2UnixNetName( pIDECmd->pszParams, 
                                        &(szBuffer[iEndPos]), sizeof(szBuffer)-iEndPos );
          if ( nNameTransResult == NAME_SUCCESS )
            {
            ServerDebugMessageLog( MSG_DBG, "IQueryQuery", "FNameTrans", szBuffer );
				    strcat( szBuffer, " ");
            strcat( szBuffer, pParam );
				    sendDis( GetSocketOut(), szBuffer, 0);
				    PostMessage( GetIntegratorMainWnd(), WM_LOOKNEWQUERY, NULL, NULL );
            }
          else
            {
             ServerDebugMessageLog( 4, "IQueryQuery", "FNameTrans", "Error LocalMapped2UnixNetName" );
            }
        }       // if ( pParam )
			}         // if ( m_pIDEInfo && pIDECmd->nParamsLen >0

 			m_nQueryStatus = QUERY_STATUS_SLEEP;
			break;
    }           // case IDE_MSG_PARAMS_RECIEVED:

		default:
			break;
  }

	return NULL;
}


/***************************************************************************
	IQueryProjectChanged
****************************************************************************/
//
IQueryProjectChanged::IQueryProjectChanged(HWND hWnd) : DI_QueryGeneric(hWnd)
{
	m_nQueryId	= I_QUERY_PROJECTCHANGED;
}


//---------------------
//
BOOL IQueryProjectChanged::StartQuery()
{
	return FALSE;
}

char* IQueryProjectChanged::SignalToQuery(IDECommand* pIDECmd)
{
	return NULL;
}


/***************************************************************************
	IQueryIDEFinished
****************************************************************************/
//
IQueryIDEFinished::IQueryIDEFinished(HWND hWnd) : DI_QueryGeneric(hWnd)
{
	m_nQueryId	= I_QUERY_IDEFINISHED;
}


//---------------------
//
BOOL IQueryIDEFinished::StartQuery()
{
	m_nQueryStatus = QUERY_STATUS_WAIT_SOCKET;

  char	szBuffer[_MAX_PATH];
	sprintf(szBuffer, "%s %d", 
											TCL_IDE_DISCONNECT,
											m_pIDEInfo->GetMainIDEWnd());
	sendDis( GetSocketOut(), szBuffer, 0);
	ServerDebugMessageLog( MSG_DBG, "IQueryIDEFinished", "StartQuery", szBuffer);

  m_nQueryStatus = QUERY_STATUS_SLEEP;
	return TRUE;
}

char* IQueryIDEFinished::SignalToQuery(IDECommand* pIDECmd)
{
	return NULL;
}


/***************************************************************************
	IQueryRefreshProject
****************************************************************************/
//
IQueryRefreshProject::IQueryRefreshProject(HWND hWnd) : DI_QueryGeneric(hWnd)
{
	m_nQueryId	= I_QUERY_REFRESHPROJECT;
}


//---------------------
//
BOOL IQueryRefreshProject::StartQuery()
{
  if ( *(m_pIDEInfo->GetConfigName()) )
  {
    m_nQueryStatus = QUERY_STATUS_WAIT_SOCKET;

    char	szBuffer[_MAX_PATH+12];
	  sprintf(szBuffer, "%s %d %s", 
											  TCL_IDE_HOMEPROJECT,
											  m_pIDEInfo->GetMainIDEWnd(),
											  m_pIDEInfo->GetConfigName() );
	  sendDis( GetSocketOut(), szBuffer, 0);
	  ServerDebugMessageLog( MSG_DBG, "IQueryRefreshProj", "StartQuery", szBuffer);

    m_nQueryStatus = QUERY_STATUS_SLEEP;
  }
	return TRUE;
}

char* IQueryRefreshProject::SignalToQuery(IDECommand* pIDECmd)
{
	switch ( pIDECmd->iIDECommnd )
		{
		case IDE_MSG_SPYSTARTED:
			{
			m_nQueryStatus = QUERY_STATUS_WAIT_IDESPY;
			if ( pIDECmd->nParamsLen >0 && pIDECmd->pszParams )
				m_pIDEInfo->SetConfigName(pIDECmd->pszParams);
			else
				m_pIDEInfo->SetConfigName(NULL);

 			StartQuery();
			m_nQueryStatus = QUERY_STATUS_WAIT_IDESPY;
			DDEIntegratorClient* pClient= m_pIDEInfo->GetIDESpyClient();
			if ( pClient && pClient->IsConnected() )
				pClient->CommandToServer(IDE_CMD_DSPSTATUS, "Discover connected" );
			else
			  ServerDebugMessageLog( 0, "IQueryRefreshProject", "IDE_MSG_SPYSTARTED", "IDESpy unconnected");
    
      m_nQueryStatus = QUERY_STATUS_SLEEP;
			break;
			}

		default:
			break;
		}

	return NULL;
}


/***************************************************************************
	IQueryStartDiscover
****************************************************************************/
//
IQueryStartDiscover::IQueryStartDiscover(HWND hWnd) : DI_QueryGeneric(hWnd)
{
	m_nQueryId	= I_QUERY_STARTDISCOVER;
}

//---------------------
//
BOOL IQueryStartDiscover::StartQuery()
{
	char	szBuffer[_MAX_PATH];
	sprintf(szBuffer, "%s %d",
											TCL_IDE_DISCONNECT,
											m_pIDEInfo->GetMainIDEWnd());
//	sendDis( GetSocketOut(), szBuffer, 0);
	ServerDebugMessageLog( MSG_DBG, "IQueryStart", "StartQuery", szBuffer);
	return TRUE;
}

char* IQueryStartDiscover::SignalToQuery(IDECommand* pIDECmd)
{
	switch ( pIDECmd->iIDECommnd )
		{
		case IDE_CMD_ACTIVATEBROWSER:
			{
			sendDis( GetSocketOut(), TCL_IDE_DISC_ACTIVATE, 0);
			PostMessage( GetIntegratorMainWnd(), WM_LOOKNEWQUERY, NULL, NULL );
			return (char*)1;
			}

		default:
			break;
		}
  
  return NULL;
}

/*****************************************************************************\
* PswdDlgProc
*
* Dialog procedure for network password dialog box
*
\*****************************************************************************/
static	char	gpszPassword[64];
static	char*	gpszNetPath;

BOOL CALLBACK PswdDlgProc( HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam )
{
	BOOL fResult = TRUE;
	
	switch( nMessage )
	{
	
	case WM_INITDIALOG:
		{
			HWND hName = GetDlgItem( hDlg, IDC_STATIC2 );
			if( hName )
				SetWindowText( hName, gpszNetPath );
		}
		break;
	case WM_COMMAND:
		{
			UINT uId = LOWORD( wParam );
			switch( uId )
			{
			case IDOK:
				{
					HWND hPassword = GetDlgItem( hDlg, IDC_PASSWORD );
					if( hPassword && GetWindowTextLength( hPassword ) )
						GetWindowText( hPassword, gpszPassword, MAX_PATH );
					else
						gpszPassword[0] = '\0';
					EndDialog( hDlg, uId );
				}
				break;
			case IDCANCEL:
				EndDialog( hDlg, uId );
				break;
			default:
				fResult = FALSE;
			}
		}
		break;
	
	default:
		fResult = FALSE;
	}
	return fResult;
}

//================================================================================

char*		GetPassword(HINSTANCE hInst, HWND hParent, char* pszNetPath )
{
	gpszPassword[0] = '\0';
	gpszNetPath			= pszNetPath;

	int nDialogRet = DialogBox( hInst, MAKEINTRESOURCE( IDD_DIALOG2 ),
						 hParent, (DLGPROC)PswdDlgProc );

	return gpszPassword;
}
