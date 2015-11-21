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
#ifndef _INTEGRATOR_H
#define _INTEGRATOR_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <ddeml.h>
#include "ErrMessages.h"
#include "DisRegistryIntegrator.h"
#include "DisFName.h"

#ifdef DLL
	#define CFUNCTION	extern "C" __declspec(dllexport)
#else
	#define CFUNCTION	extern "C" __declspec(dllimport)
#endif

#define	INTEGRATOR_SERVERNAME			"IntegratorServer"

#define	EXE_INTEGRATOR		  "IntegratorServer.exe"
#define	EXE_IDECONSOLE		  "IDEConsole.exe"
#define	EXE_PARAMRECEIVER	  "IDEParamReceiver.exe"
#define	EXE_IDESPYLOADER	  "IDESpyLoader.exe"

#define DDE_FAKE_SERVICE_IDEINFO  "IDE_Fake_SERVICE_INFO"
#define	DDE_FAKE_SERVICE_IDESPY	  "IDESpy_Fake_Server"
#define	DDE_SERVICE_IDEINFO	  "IDE-Discover IntegratorServer"
#define	DDE_SERVICE_IDESPY	  "IDESpyServer"
#define	DDE_SERVICE_SCC		  "SccDiscover"
#define	DDE_SERVICE_PARAMRECEIVER "IDEParamReceiver"
#define	DDE_SERVICE_CONSOLE	  "IDEConsole"
#define DDE_SERVICE_ADDIN	  "AddInDiscover"
#define	DDE_TOPIC_CONTROL	  "Control"
#define	DDE_TOPIC_IDESPY	  "IDESpy"
#define	DDE_TOPIC_SCC		  "SccDiscover"
#define	DDE_ITEM_SCC		  "SccDiscover"
#define	DDE_ITEM_SCC_EVENT     	  "SccEvent"
#define	DDE_ITEM_SCC_COMMENT  	  "SccEventComment"
#define	DDE_ITEM_SCC_NUMFILES 	  "SccNumFiles"
#define	DDE_ITEM_SCC_FILENAME  	  "SccFileName"
#define DDE_ITEM_DEBUGMESSAGE	  "DebugMessage"
#define DDE_ITEM_COMMAND	  "IDECommand"
#define DDE_ITEM_OUTPUTHWND	  "OutputHWND"
#define DDE_IDECLIENT_TIMEOUT	  (DWORD)1000

#define TITLE_LENGTH		  256
#define	CLASSNAME_LENGTH	  256

#define STARTUP_EVENT		   "DIS_INTEGRATOR_START_EVENT_00"

#define	IDE_CAPTION_MSVC20	  "Microsoft Visual C++"
#define	IDE_CAPTION_MSVC40	  "Microsoft Developer Studio"
#define	IDE_CAPTION_MSVC50	  "Microsoft Developer Studio"
#define	IDE_CAPTION_MSVC60	  "Microsoft Visual C++"
#define	IDE_CAPTION_BORLANDC45	  "B4.5"

#define IDE_SHORTNAME_MSVC20	  "M2"
#define IDE_SHORTNAME_MSVC40	  "M4"
#define IDE_SHORTNAME_MSVC50	  "M5"
#define IDE_SHORTNAME_MSVC60	  "M6"
#define IDE_SHORTNAME_BORLANDC45  "B4"

/******************************************************************************
  SourceControl commands
******************************************************************************/
#define	SCC_CMD_INIT		" init "
#define	SCC_CMD_OPROJ		" oprj "
#define	SCC_CMD_SETOP		" seto "
#define	SCC_CMD_GET		" copy "
#define	SCC_CMD_CHECKIN		" put "
#define	SCC_CMD_CHECKOUT	" get "
#define	SCC_CMD_UNCHECKOUT	" ung "
#define	SCC_CMD_ADD		" add "
#define	SCC_CMD_REMOVE		" del "
#define	SCC_CMD_ERROR		" errf "
#define	SCC_CMD_QINFO		" info "
#define	SCC_CMD_COMMENT		" cmnt "
#define	SCC_CMD_EXIT		"exit"

/******************************************************************************
  TCL commands
******************************************************************************/
#define	TCL_IDE_GETSTATUS	"dis_get_token_info"
#define	TCL_IDE_CURRENTSEL	"dis_IDE_current_selection"
#define	TCL_IDE_HOMEPROJECT	"dis_set_home_project"
#define TCL_IDE_DISCONNECT  	"dis_disconnect_ide"
#define TCL_IDE_DISC_ACTIVATE   "dis_viewer_pop_active"
#define TCL_IDE_IMPACT          "dis_impact_analalysis"
#define TCL_IDE_PASTE_VW_SELECT "dis_PasteViewerSelection"
#define TCL_IDE_OPEN_DEFINITION "dis_ide_open_definition"
#define	TCL_IDE_SCC_EVENT	"dis_cm_event"
#define	TCL_IDE_COMMAND_EVENT	"dis_IDE_command_event"
#define	TCL_IDE_SERVER_CMD	"dis_server_cmd"

#define TCL_DIS_OPENFILE        "dis_open_file"
#define TCL_DIS_DISPSTATUS      "dis_ide_display_message"
#define TCL_DIS_CMGET		"dis_cm_get"

#define TCL_VIEW_CREATE		"view_create"
#define TCL_VIEW_CREATED	"view_created"
#define TCL_VIEW_EXPAND		"view_expand"
#define TCL_VIEW_EXPANDED	"view_expanded"

/******************************************************************************
  Internal commands
******************************************************************************/
enum IDE_TYPES
{
	IDE_MIN_TYPE_NUM,
	IDE_TYPE_MSVC20,
	IDE_TYPE_MSVC40,
	IDE_TYPE_BORLANDC45,
	IDE_TYPE_MSVC50,
	IDE_TYPE_MSVC60,

	IDE_MAX_TYPE_NUM
};


enum IDE_COMMAND
{
  IDE_MSG_SPYSTARTED,									/*  0 */
  IDE_MSG_CONSOLE_STARTED,	/*  1 */
  IDE_MSG_DDECLIENT_DISCONNECTED,/*  2 */
  IDE_MSG_CONSOLE_DISCONNECTED, /*  3 */
  IDE_MSG_DBLCKICKED,		/*  4 */
  IDE_MSG_PARAMS_RECEIVED,	/*  5 */
  IDE_MSG_WHERE_DEFINED,	/*  6 */
  IDE_MSG_WHERE_REFERENCED,	/*  7 */
  IDE_MSG_SCC_EVENT,									/*  8 */
  IDE_MSG_OPENING_FILE,		/*  8 */
  IDE_MSG_OPENING_COMMENT,
  IDE_MSG_DISQUERY,             /*  8 */
	
  IDE_CMD_SHOW_MESSAGE,		/*  9 */
  IDE_CMD_START_IDECONSOLE,  	/* 10 */
  IDE_CMD_START_PARAMRECEIVER,	/* 11 */
  IDE_CMD_DDECLIENT_DISCONNECT,	/* 12 */
  IDE_CMD_ENABLE_IDESPY,	/* 13 */
  IDE_CMD_ACTIVATEBROWSER,	/* 14 */
  IDE_CMD_IMPACT_ANALYSIS,	/* 15 */
  IDE_CMD_DSPSTATUS,		/* 16 */
  IDE_CMD_DEBUG,		/* 17 */
  IDE_CMD_SHOW_INTEGRATOR,      /* 18 */
  IDE_CMD_SHOWWINDOW,		/* 19 */
  IDE_CMD_HIDEWINDOW,		/* 20 */
  IDE_CMD_SCC_COMMAND,		/* 21 */
  IDE_CMD_QUERY,                /* 22 */
  IDE_CMD_OPEN_DEFINITION,      /* 23 */
  IDE_CMD_ACTIVATEDEVSTUDIO,    /* 24 */
  IDE_CMD_BACK_QUERY,           /* 25 */
  IDE_CMD_BACK_DEFINITION,      /* 26 */
  IDE_CMD_VIEW,                 /* 27 */
  IDE_CMD_VIEWANSWER,           /* 28 */

  IDE_CMD_FILEOPENED,           /* 29 */
  IDE_CMD_TOKENINFO,            /* 30 */
  IDE_CMD_BACK_SELECTION,       /* 31 */

  IDE_CMD_MAX                   /* 31 */
};

//-------------------------------------------------------------------

enum DI_QUERIES	
{
  D_QUERY_PREPROC,

  D_QUERY_VIEWCMDANSWER,// Queries from Discover
  D_QUERY_OPENFILE,
  D_QUERY_GETOPTIONS,
  D_QUERY_STARTIDE,
  D_QUERY_SPYCOMMAND,
  D_MAX_QUERYNUM,

  I_QUERY_VIEWCMD,	// Queries to Discover
  I_QUERY_QUERY,
  I_QUERY_PROJECTCHANGED,
  I_QUERY_IDEFINISHED,
  I_QUERY_REFRESHPROJECT,
  I_QUERY_STARTDISCOVER,
  I_QUERY_SCC,

  I_QUERY_DEFAULT,
  DI_MAX_QUERYNUM
};

#define SOCKET_PORT_INP		3334 // 31557
#define SOCKET_PORT_OUT		3333 // 31555


/****************************************************************************
	Classes DDEIntegratorClient
*****************************************************************************/
char* DDEGetLastErrorText( DWORD IdInst );
char* GetIDECommandText( enum IDE_COMMAND );

HDDEDATA CALLBACK DDEIntegratorClientCallBack(WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD);

class DDEIntegratorClient
{
protected:
  HDDEDATA	m_hData;
  HSZ		m_hszClientTopic;
  HCONV		m_hConv;
  DWORD		m_dwDDEResult;

  void		DDEIntegratorClientInit (char* pszService);

public:
   DDEIntegratorClient();
   DDEIntegratorClient(char* pszService);
  ~DDEIntegratorClient();

   BOOL		ConnectToServer	();
   BOOL		ConnectToServer	(char* pszTopicName);
   void		Disconnect	();
   BOOL		IsConnected	();
   HCONV	GetConv		()	{ return m_hConv; }
   HSZ		GetClientTopic	()	{ return m_hszClientTopic; }

   void		SetConv		(HCONV hConv) { m_hConv = hConv; }
   void		SetClientTopic	( char* pszTopicName );
   void		SendDebugMessage( UINT nErrLevel, char* pszMessageText );
   BOOL		SendDataToServer( char* pszDataName, char* pszData );
   BOOL		ReceiveDataFromServer	( char* pszDataName, char* pszBuffer, int nBufferLen );
   BOOL		CommandToServer	( IDE_COMMAND iCommand, char* pszParams );
   BOOL		CommandToServer	( IDE_COMMAND iCommand, long nLong, char* pszParams );
};


/***************************************************************************
		MenuChooser
****************************************************************************/

int   GetMenuIdByName	( HWND, char *, char *, BOOL = FALSE );
BOOL  ChooseMyMenu	( HWND, UINT, char *, char * );
BOOL  ChooseMyCustomMenu( HWND, char *, BOOL = FALSE );
BOOL  ChooseSourceControl( HWND hWnd );

#endif // #ifndef INTEGRATOR_H
