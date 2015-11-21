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
#include "DisRegistry.h"

#ifdef DLL
	#define CFUNCTION	extern "C" __declspec(dllexport)
#else
	#define CFUNCTION	extern "C" __declspec(dllimport)
#endif


#define	INTEGRATOR_SERVERNAME			"IntegratorServer"
#define	DISCOVER_WINDOW_NAME			"Discover"

#define	EXE_INTEGRATOR						"IntegratorServer.exe"
#define	EXE_IDECONSOLE						"IDEConsole.exe"
#define	EXE_PARAMRECIEVER					"IDEParamReciever.exe"
#define	EXE_IDESPYLOADER					"IDESpyLoader.exe"

#define	DDE_SERVICE_IDEINFO				"IDE-Discover IntegratorServer"
#define	DDE_SERVICE_IDESPY				"IDESpyServer"
#define	DDE_TOPIC_CONTROL					"Control"
#define	DDE_TOPIC_CONSOLE					"IDEConsole"
#define	DDE_TOPIC_PARAMRECIEVER		"IDEParamReciever"
#define	DDE_TOPIC_IDESPY					"IDESpy"
#define	DDE_TOPIC_START 					"ServerStart"
#define	DDE_TOPIC_FINISH 					"ServerFinish"
#define DDE_ITEM_DEBUGMESSAGE			"DebugMessage"
#define DDE_ITEM_COMMAND					"IDECommand"
#define DDE_ITEM_OUTPUTHWND				"OutputHWND"
#define DDE_IDECLIENT_TIMEOUT			(DWORD)1000

#define	IDE_CAPTION_MSVC40				"Microsoft Developer Studio"
#define	IDE_CAPTION_BORLANDC45		"B4.5"

#define IDE_SHORTNAME_MSVC40			"M4"
#define IDE_SHORTNAME_BORLANDC45	"B4"

/******************************************************************************
  TCL commands
******************************************************************************/
#define	TCL_IDE_GETSTATUS				"dis_get_token_info"
#define	TCL_IDE_HOMEPROJECT			"dis_set_home_project"
#define TCL_IDE_DISCONNECT  		"dis_disconnect_ide"
#define TCL_IDE_DISC_ACTIVATE   "dis_viewer_pop_active"
#define TCL_IDE_IMPACT          "dis_impact_analalysis"

#define TCL_DIS_OPENFILE        "dis_open_file"
#define TCL_DIS_DISPSTATUS      "dis_ide_display_message"

/******************************************************************************
  Internal commands
******************************************************************************/
enum IDE_TYPES
{
	IDE_MIN_TYPE_NUM,
	IDE_TYPE_MSVC40,
	IDE_TYPE_BORLANDC45,
	IDE_MAX_TYPE_NUM
};


enum IDE_COMMAND
{
	IDE_MSG_SPYSTARTED,
	IDE_MSG_CONSOLE_STARTED,
	IDE_MSG_DDECLIENT_DISCONNECTED,
	IDE_MSG_DBLCKICKED,
	IDE_MSG_PARAMS_RECIEVED,
	IDE_MSG_WHERE_DEFINED,
	IDE_MSG_WHERE_REFERENCED,
	
	IDE_CMD_CHOOSE_CUSTOM_MENU,
	IDE_CMD_DDECLIENT_DISCONNECT,
  IDE_CMD_ACTIVATEBROWSER,
  IDE_CMD_IMPACT_ANALYSIS,
	IDE_CMD_DSPSTATUS,
	IDE_CMD_DEBUG,
	IDE_CMD_SHOW_INTEGRATOR
};

//-------------------------------------------------------------------

enum DI_QUERIES	
{
	D_QUERY_DISCSTARTED,		// Queries from Discover
	D_QUERY_OPENFILE,
	D_QUERY_GETOPTIONS,
	D_QUERY_STARTIDE,
	D_QUERY_DISPLAYSTATUS,
	D_MAX_QUERYNUM,

	I_QUERY_QUERY,				// Queries to Discover
	I_QUERY_PROJECTCHANGED,
	I_QUERY_IDEFINISHED,
	I_QUERY_REFRESHPROJECT,
	I_QUERY_STARTDISCOVER,

	DI_MAX_QUERYNUM
};

#define SOCKET_PORT_INP		3334 // 31557
#define SOCKET_PORT_OUT		3333 // 31555


/****************************************************************************
	Classes DDEIntegratorClient
*****************************************************************************/

HDDEDATA CALLBACK DDEIntegratorClientCallBack(WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD);

class DDEIntegratorClient
{
protected:
	HDDEDATA			m_hData;
	HSZ						m_hszClientTopic;
	HCONV					m_hConv;
	DWORD					m_dwDDEResult;

	void					DDEIntegratorClientInit (char* pszService);

public:
	 DDEIntegratorClient();
	 DDEIntegratorClient(char* pszService);
	~DDEIntegratorClient();

	 BOOL		ConnectToServer	();
	 BOOL		ConnectToServer	(char* pszTopicName);
	 void		Disconnect			();
	 BOOL		IsConnected			();
	 HCONV	GetConv					()	{ return m_hConv; }
	 HSZ		GetClientTopic	()	{ return m_hszClientTopic; }

	 void		SetClientTopic				( char* pszTopicName );
	 void		SendDebugMessage			( UINT nErrLevel, char* pszMessageText );
	 BOOL		SendDataToServer			( char* pszDataName, char* pszData );
	 BOOL		RecieveDataFromServer	( char* pszDataName, char* pszBuffer, int nBufferLen );
	 BOOL		CommandToServer				( IDE_COMMAND iCommand, char* pszParams );
};


/***************************************************************************
		MenuChooser
****************************************************************************/

int   GetMenuIdByName   ( HWND, char *, char *, BOOL = FALSE );
BOOL  ChooseMyMenu      ( HWND, UINT, char *, char * );
BOOL  ChooseMyCustomMenu( HWND, char * );


#endif // #ifndef INTEGRATOR_H
