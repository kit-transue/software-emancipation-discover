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
#include <winsock.h>
#include <stdio.h>
#include "tcl.h"
#include "readsock.h"

/***************************************************************************
	TCL Interpreter Functions and Externs
****************************************************************************/

static  SOCKET   			gSockInp, gSockOut;			//global socket descriptors
static  ReadSocket    gOutSockRead( gSockOut, NULL ); //global ReadSocket objects
static  ReadSocket    gInpSockRead( gSockInp, InpSockExecute ); 
static	Tcl_Interp		*gTclInterp	= NULL;			//global pointer to interpreter
static  char          ghszComputerName[_MAX_PATH];

enum POST_ERROR_TCL_COMMAND
{
	NO_POST_COMMAND,
	ERR_CREATE_QUERY,
	ERR_ADD_NODE,
	ERR_POST_COMMAND
};

static struct PostTclCommand
{
	POST_ERROR_TCL_COMMAND   nCmdID;
  char		                 szDisplCmd[sizeof(TCL_DIS_DISPSTATUS)];
	char		                 szText[_MAX_PATH];
}
 gPostTclCommand;


static	CircleList		QueueList;							//global Queue List
CircleList* GetQueueList()	{ return &QueueList; }

//-------------------------------------------------------------------
SOCKET	GetSocketOut()					{ return gSockOut; }
SOCKET	GetSocketInp()					{ return gSockInp; }
char*		GetComputerName()				{ return ghszComputerName; }
BOOL		CreateTclInterpreter();

int		DiscoverStarted		(ClientData, Tcl_Interp*, int, char**);
int		OpenFileInIDE			(ClientData, Tcl_Interp*, int, char**);
int		GetCompileOptions (ClientData, Tcl_Interp*, int, char**);
int		StartIDE					(ClientData, Tcl_Interp*, int, char**);
int		DisplayStatusLine (ClientData, Tcl_Interp*, int, char**);


void  DebugServerAdapter  ()
{
  _asm int 3;
}
//-----------------------------------------------------
//

void	sendDis( SOCKET socket, char* pszBuf, int Flag)
{
	unsigned long lLength = (unsigned long) strlen( pszBuf ) + 1;
  unsigned long lNetLen = ntohl( lLength );
  send( socket, TRANSMIT_START_SEQ, TRANSMIT_START_SEQ_LEN, 0 );
  send( socket, (char *)(&lNetLen), sizeof( long ), 0 );
	send( socket, pszBuf, lLength, 0 );
  ServerDebugMessageLog( MSG_DBG, "OutSocket", "Write", pszBuf);
}

//-----------------------------------------------------
//
BOOL InitTclSocket()
{
	WSADATA WSAData;
	if ( WSAStartup (MAKEWORD(1,1), &WSAData) != 0 )
		return FALSE;

	if ( !CreateTclInterpreter() )
		return FALSE;

	return TRUE;
}

//-----------------------------------------------------
//
BOOL ConnectToOutSocket()					//connect to a socket
{
	char					szHost [MAX_PATH], szTemp[200];
	PHOSTENT			phe_Inp, phe_Out;
  int           nPortInp, nPortOut;
	SOCKADDR_IN		dest_sin_Inp;	  /* DESTination Socket INternet */
	SOCKADDR_IN		dest_sin_Out;	  /* DESTination Socket INternet */
  BOOL          fReconnected = FALSE;

	gethostname(ghszComputerName, sizeof(ghszComputerName));
	GetDisRegistryStr(REG_DISCOVER_HOST, szHost, sizeof(szHost), ghszComputerName);

  phe_Inp = gethostbyname(ghszComputerName);

	if ( phe_Inp == NULL ) 
	{
	 	sprintf(szTemp, "%d is the error. Make sure '%s' is listed in the hosts file.", WSAGetLastError(), szHost);
		ServerDebugMessageLog( 5, "Socket", "gethostbyname()", szTemp );
		return FALSE;
	}
	memcpy((char FAR *)&(dest_sin_Inp.sin_addr), phe_Inp->h_addr, phe_Inp->h_length);

	nPortInp = GetDisRegistryInt(REG_INP_SOCKET, SOCKET_PORT_INP);
  dest_sin_Inp.sin_family = AF_INET;
	dest_sin_Inp.sin_port = htons( nPortInp );

	gSockInp = socket( AF_INET, SOCK_STREAM, 0);
  gInpSockRead.ChangeSocket( gSockInp );
	if (gSockInp == INVALID_SOCKET ) 
	{
		ServerDebugMessageLog( 5, "Socket", "socket()", "failed!" );
		return FALSE;
	}

  if( bind( gSockInp, (struct sockaddr FAR *) &dest_sin_Inp, sizeof(dest_sin_Inp)) == SOCKET_ERROR )
  {
		ServerDebugMessageLog( 5, "Socket", "Bind", "failed!" );
  }

  if( listen( gSockInp, 5 ) < 0 )
  {
		ServerDebugMessageLog( 5, "Socket", "Listen", "failed!" );
  }
  
  int statusInp = WSAAsyncSelect( gSockInp, GetIntegratorMainWnd(), WSA_CONNECT,  FD_ACCEPT  );

  //-------------------------------------
  
	phe_Out = gethostbyname(szHost);
	if (phe_Out == NULL ) 
	{
	 	sprintf(szTemp, "%d is the error. Make sure '%s' is listed in the hosts file.", WSAGetLastError(), szHost);
		ServerDebugMessageLog( 5, "Socket", "gethostbyname()", szTemp );
		return FALSE;
	}
  
	memcpy((char FAR *)&(dest_sin_Out.sin_addr), phe_Out->h_addr, phe_Out->h_length);
	nPortOut = GetDisRegistryInt(REG_OUT_SOCKET, SOCKET_PORT_OUT);
	dest_sin_Out.sin_family = AF_INET;
	dest_sin_Out.sin_port = htons( nPortOut );
	gSockOut = socket( AF_INET, SOCK_STREAM, 0);
  gOutSockRead.ChangeSocket( gSockOut ); // We have to call ReadSocket::ChangeSocket()
                                         // each time we assign a new value to a global
                                         // socket handle
	if (gSockInp == INVALID_SOCKET || gSockOut == INVALID_SOCKET) 
	{
		ServerDebugMessageLog( 5, "Socket", "socket()", "failed!" );
    return FALSE;
	}
  
	if ( connect( gSockOut, (PSOCKADDR) &dest_sin_Out, sizeof( dest_sin_Out)) < 0  )
	{ // First connect failed, we will try to start Browser and reconnect
		char pszBrowserCommand[ MAX_PATH ];
    GetDisRegistryStr(REG_BROWSER, pszBrowserCommand, MAX_PATH, "");
    if( strlen( pszBrowserCommand ) ) //path is found in a registry
    {
      PROCESS_INFORMATION pi;
      STARTUPINFO si;
      si.cb = sizeof(STARTUPINFO);
      si.lpReserved = NULL;
      si.lpReserved2 = NULL;
      si.cbReserved2 = 0;
      si.lpDesktop = NULL;
      si.dwFlags = 0;

    
			if( CreateProcess( NULL, pszBrowserCommand, NULL, NULL, FALSE, 0, NULL, NULL,
                         &si, &pi ) )
      {
				WaitForInputIdle( pi.hProcess, BROWSER_START_TIMEOUT );
        if ( connect( gSockOut, (PSOCKADDR) &dest_sin_Out, sizeof( dest_sin_Out)) == 0  )
				{
          fReconnected = TRUE;
				}
        else
        { //try to connect every 0.5 sec 400 times
					int nMaxReconnect = GetDisRegistryInt(REG_BROWSER_WAITING, 400);
          for( int i=0; (i < nMaxReconnect) && !fReconnected; i++)
          {
            Sleep( 500 );
            if ( connect( gSockOut, (PSOCKADDR) &dest_sin_Out, sizeof( dest_sin_Out)) == 0  )
						{
              fReconnected = TRUE;
						}
          }
        }

			}
    
    }
    if( !fReconnected )
    {
      closesocket( gSockInp );
		  closesocket( gSockOut );
		  ServerDebugMessageLog( 5, "Socket", "connect()", "failed!" );
		  return FALSE;
    }
	}
	ServerDebugMessageLog( MSG_DBG, "Socket", "connect", "worked!" );

 /*
	*   Send main window a WSA_READ when either data is pending on
	*   the socket (FD_READ) or the connection is closed (FD_CLOSE)
	*/
	int statusOut = WSAAsyncSelect( gSockOut, GetIntegratorMainWnd(), WSA_WRITE, FD_READ | FD_CLOSE );


	if (statusInp <0	|| statusOut <0 )
	{
		int nErr = WSAGetLastError();
		ServerDebugMessageLog( 5, "Socket", "connect", "Error on WSAAsyncSelect()" );
    closesocket( gSockInp );
    closesocket( gSockOut );
		return FALSE;
	}

	return TRUE;
}

//-----------------------------------------------------
//
BOOL ConnectToInpSocket()					//connect to a socket
{

  BOOL fResult = TRUE;
  SOCKET sock = accept( gSockInp, NULL, NULL );
  gSockInp = sock;
  gInpSockRead.ChangeSocket( gSockInp ); // We have to call ReadSocket::ChangeSocket()
                                         // each time we assign a new value to a global
                                         // socket handle
  int statusInp = WSAAsyncSelect( gSockInp, GetIntegratorMainWnd(), WSA_READ, FD_READ | FD_CLOSE );
  if ( statusInp <0 )
    fResult = FALSE;  
    //MessageBox( NULL, "Connect error", "SocketInp", MB_OK); 

	return fResult;
}




void OutSocketRead( WPARAM wParam, LPARAM lParam)
{
  

  if (WSAGETSELECTEVENT(lParam) == FD_READ) 
  {
    gOutSockRead.ProcessData();

  }     // if (WSAGETSELECTEVENT(lParam) == FD_READ)    
  else 
  {    /* FD_CLOSE -- connection dropped */
		ServerDebugMessageLog( 5, "Socket", "WSA_WRITE", "Connection lost" );
		TerminateServer( EXTMSG_ERR_SOCKET );
  }
	      
}



void InpSocketRead( WPARAM wParam, LPARAM lParam)
{

  if (WSAGETSELECTEVENT(lParam) == FD_READ) 
  {
    gInpSockRead.ProcessData();
  }
  else 
  {    /* FD_CLOSE -- connection dropped */
		ServerDebugMessageLog( 5, "Socket", "WSA_READ", "Connection lost" );
		TerminateServer( EXTMSG_ERR_SOCKET );
  }

}


//-----------------------------------------------------
//
void CloseTclSocket()
{
	WSACleanup();
  closesocket( gSockInp );
  closesocket( gSockOut );

	if (gTclInterp)
		Tcl_DeleteInterp( gTclInterp );
	gTclInterp = 0;
}

/***************************************************************************
		TclInterpreter()
****************************************************************************/

BOOL CreateTclInterpreter()
{
	strcpy( gPostTclCommand.szDisplCmd, TCL_DIS_DISPSTATUS );
  strcat( gPostTclCommand.szDisplCmd, " ");
  gPostTclCommand.nCmdID = NO_POST_COMMAND;

  gTclInterp = NULL; //global pointer to interpreter
	gTclInterp = Tcl_CreateInterp();
	if( !gTclInterp )
		return FALSE;
	
	if( !( Tcl_CreateCommand( gTclInterp, "dsuccess", DiscoverStarted,
										(ClientData) NULL, (Tcl_CmdDeleteProc *)NULL) &&
				 Tcl_CreateCommand( gTclInterp, TCL_DIS_OPENFILE, OpenFileInIDE,
										(ClientData) NULL, (Tcl_CmdDeleteProc *)NULL) &&
				 Tcl_CreateCommand( gTclInterp, "getopt", GetCompileOptions,
										(ClientData) NULL, (Tcl_CmdDeleteProc *)NULL) &&
				 Tcl_CreateCommand( gTclInterp, "startide", StartIDE,
										(ClientData) NULL, (Tcl_CmdDeleteProc *)NULL) &&
				 Tcl_CreateCommand( gTclInterp, TCL_DIS_DISPSTATUS, DisplayStatusLine,
										(ClientData) NULL, (Tcl_CmdDeleteProc *)NULL) ) )

		return FALSE;

	return TRUE;
}


int DiscoverStarted		(ClientData clientdata, Tcl_Interp *pInterp, 
											 int argc, char *argv[])
{
	//MessageBox( NULL, argv[0], "Function Called", MB_OK );
	if( argc < 2 )
		return TCL_ERROR;
	
	DQueryDiscoverStarted * pQuery = new DQueryDiscoverStarted( (HWND)atol(argv[1]) );
	if( !pQuery )
  {
    gPostTclCommand.nCmdID = ERR_CREATE_QUERY;
		return TCL_ERROR;
  }
	if( !GetQueueList()->AddNode( (void *) pQuery ) )
	{
		delete pQuery;
    gPostTclCommand.nCmdID = ERR_ADD_NODE;
		return TCL_ERROR;
	}
	return TCL_OK;
}


int OpenFileInIDE			(ClientData clientdata, Tcl_Interp *pInterp, 
											 int argc, char *argv[])
{
 	char szBuffer[_MAX_PATH+14];
  sprintf(szBuffer,"%s %s %s", argv[1], argv[2], argv[3]);
  ServerDebugMessageLog( MSG_DBG, "TCL", "OpenFileInIDE", szBuffer);

	if( argc < 4 )
		return TCL_ERROR;
	
	DQueryOpenFile * pQuery = new DQueryOpenFile( (HWND)atol(argv[1]),
																								argv[2],  argv[3] );
	if( !pQuery )
  {
    gPostTclCommand.nCmdID = ERR_CREATE_QUERY;
		return TCL_ERROR;
  }
  
  if ( !pQuery->GetNetMapFlag() )
	{
		delete pQuery;
    sprintf( gPostTclCommand.szText, "%s \"%s\"", argv[1], EXTMSG_ERR_NETMAP );
    gPostTclCommand.nCmdID = ERR_POST_COMMAND;
		return TCL_ERROR;
	}

  if ( *(pQuery->GetFilePath()) == '\0' )
	{
		delete pQuery;
    strcpy( gPostTclCommand.szText, "\"Empty FileName\"" );
    gPostTclCommand.nCmdID = ERR_POST_COMMAND;
		return TCL_ERROR;
	}

	if( !GetQueueList()->AddNode( (void *) pQuery ) )
	{
		delete pQuery;
    gPostTclCommand.nCmdID = ERR_ADD_NODE;
		return TCL_ERROR;
	}
	return TCL_OK;
}


int GetCompileOptions (ClientData clientdata, Tcl_Interp *pInterp, 
											 int argc, char *argv[])
{
	//MessageBox( NULL, argv[0], "Function Called", MB_OK );
	if( argc < 3 )
		return TCL_ERROR;
	return TCL_OK;

	DQueryCompileOptions * pQuery = new DQueryCompileOptions( (HWND)atol(argv[1]), argv[2] );
	if( !pQuery )
  {
    gPostTclCommand.nCmdID = ERR_CREATE_QUERY;
		return TCL_ERROR;
  }
	if( !GetQueueList()->AddNode( (void *) pQuery ) )
	{
		delete pQuery;
    gPostTclCommand.nCmdID = ERR_ADD_NODE;
		return TCL_ERROR;
	}
	return TCL_OK;

}


int StartIDE					(ClientData clientdata, Tcl_Interp *pInterp, 
											 int argc, char *argv[])
{
	//MessageBox( NULL, argv[0], "Function Called", MB_OK );
	if( argc < 3 )
		return TCL_ERROR;
	return TCL_OK;

	DQueryStartIDE * pQuery = new DQueryStartIDE( (HWND)atol(argv[1]), argv[2] );

	if( !pQuery )
  {
    gPostTclCommand.nCmdID = ERR_CREATE_QUERY;
		return TCL_ERROR;
  }
	if( !GetQueueList()->AddNode( (void *) pQuery ) )
	{
		delete pQuery;
    gPostTclCommand.nCmdID = ERR_ADD_NODE;
		return TCL_ERROR;
	}
	return TCL_OK;
}


int DisplayStatusLine (ClientData clientdata, Tcl_Interp *pInterp, 
											 int argc, char *argv[])
{
 	ServerDebugMessageLog( MSG_DBG, "TCL", "DisplayStatusLine", argv[1], argv[2]);
	PostMessage( GetIntegratorMainWnd(), WSA_WRITE, (WPARAM)((long)gSockOut), (LPARAM)FD_READ );

	if( argc < 3 )
		return TCL_ERROR;

  HWND  hWnd = (HWND)atol(argv[1]);		
  Node* pNodeFirst = GetQueueList()->GetEntryPoint();
	if ( pNodeFirst  )
    {			
			Node* pNode = pNodeFirst;
			do {
				DI_QueryGeneric*	pQuery		= (DI_QueryGeneric*)(pNode->GetData());
				IDEInfo*					pIDEInfo	= GetIDEInfoList()->FindIDEInfoByMainWnd( pQuery->GetMainIDEWnd() );
				if ( hWnd == pIDEInfo->GetMainIDEWnd() && pQuery->GetQueryId() == D_QUERY_DISPLAYSTATUS )
				{
          ((DQueryDisplayStatus*)pQuery)->SetStatusText( argv[2] );
          return TCL_OK;
				}
				pNode= pNode->GetNextClockWise();
			}
			while ( pNode != pNodeFirst ); 
		}
  
  DQueryDisplayStatus * pQuery = new DQueryDisplayStatus( hWnd, argv[2] );
	if( !pQuery )
  {
    gPostTclCommand.nCmdID = ERR_CREATE_QUERY;
		return TCL_ERROR;
  }
	if( !GetQueueList()->AddNode( (void *) pQuery ) )
	{
		delete pQuery;
    gPostTclCommand.nCmdID = ERR_ADD_NODE;
		return TCL_ERROR;
	}
	return TCL_OK;
}




void InpSockExecute( char *pszTclCommand, UINT nLength )
{
  if( gTclInterp )
  {
		if ( Tcl_Eval( gTclInterp, pszTclCommand ) != TCL_OK )
    {
	    ServerDebugMessageLog( 5, "Tcl", "interp", "Invalid Tcl Command" );
      switch ( gPostTclCommand.nCmdID )
      {
      case NO_POST_COMMAND:
  			ServerDebugMessageLog( 5, "Tcl", "Tcl_Eval", "Invalid Tcl Command" );
        break;

      case ERR_CREATE_QUERY:
  			ServerDebugMessageLog( 5, "Tcl", "new Query", "Error Query creation" );
        break;

      case ERR_ADD_NODE:
  			ServerDebugMessageLog( 5, "Tcl", "AddNode", "Error AddNode creation" );
        break;

      case ERR_POST_COMMAND:
        Tcl_Eval( gTclInterp, gPostTclCommand.szDisplCmd );
        break;

      default:
  			ServerDebugMessageLog( 4, "Tcl", "Error", "Unknown Error!" );
        break;
      }

      gPostTclCommand.nCmdID = NO_POST_COMMAND;
    }
    else
			ServerDebugMessageLog( MSG_DBG, "Tcl", "interp=OK", pszTclCommand );
  }
}
