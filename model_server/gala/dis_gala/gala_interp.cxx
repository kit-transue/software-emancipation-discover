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
#include "machdep.h"
#include <stdlib.h>
#include <vport.h>
#include vtimerHEADER
#include vstdlibHEADER

#ifdef printf
#define gala_printf printf
#undef printf
#endif

#include "settings.h"

#ifdef gala_printf
#define printf gala_printf
#endif

#include "tcl.h"
#include "gcontrolObjects.h"
#include "gapl_menu.h"
#include "gString.h"
#include "gviewer.h"
#include "gview.h"
#include "glayer.h"
#include "monitorBatch.h"
#include "Application.h"
#include "pdumem.h"
#include "pdupath.h"
#include "pdudir.h"
#include "shelltextitem.h"
#include "../../DIS_main/interface.h"
#include "vdasservcmds.h"
#include "deaddirs.h"
gString gPDFName;

extern int dis_clipboard (ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_init(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_uninit(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_open_session(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_request(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_set_status_auto(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_close_session(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_make_report(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_vlistitem_has_sel(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_app_package(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_insert_app(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_remove_app(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_partition(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_delete_session(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_package_new_session(ClientData, Tcl_Interp *, int, char *[]);
extern int dis_vdasserv(ClientData, Tcl_Interp *, int, char *[]);

static int dis_build_dead_dirs(ClientData, Tcl_Interp *, int, char *[]);
static int dis_build_pmods_list (ClientData, Tcl_Interp *, int, char *[]);
static int dis_download_file_from_pmod (ClientData, Tcl_Interp *, int, char *[]);
static int dis_shutdown_pmodserver (ClientData, Tcl_Interp *, int, char *[]);
static int dis_calc_file_checksum (ClientData, Tcl_Interp *, int, char *[]);
static int dis_get_pmod_pdf_checksum (ClientData, Tcl_Interp *, int, char *[]);
static int dis_expand_envvars (ClientData, Tcl_Interp *, int, char *[]);
static int dis_init_access_shell (ClientData, Tcl_Interp *, int, char *[]);
static int dis_client_access_eval (ClientData, Tcl_Interp *, int, char *[]);
static int dis_client_access_eval_async (ClientData, Tcl_Interp *, int, char *[]);
static int dis_exit (ClientData, Tcl_Interp *interp, int argc, char *argv[]);
static int dis_setPDFname(ClientData, Tcl_Interp *interp, int argc, char *argv[]);
static int dis_vlistitem_num_items(ClientData, Tcl_Interp *interp, int argc, char *argv[]);


extern int build_pmod_servers_list( Tcl_Interp* interp, const char *reg_exp );
extern int download_file_from_pmod( int type, const char *pszServiceName, const char *pszPmodFile, 
                            const char *pszSaveAs, int &checksum  );
extern int shutdown_pmodserver( const char *servicename );
extern int get_pmod_pdf_checksum( const char *pszServiceName );
extern int calc_file_checksum(const char *pszFile);


void register_gala_specific_commands(Tcl_Interp* interp)
{
  Tcl_CreateCommand (interp, "dis_clipboard", dis_clipboard, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_CreateBatch", dis_CreateBatch, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_CancelBatch", dis_CancelBatch, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_OpenSettings", dis_OpenSettings, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_CloseSettings", dis_CloseSettings, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_ReadSettingsVar", dis_ReadSettingsVar, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_WriteSettingsVar", dis_WriteSettingsVar, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_FlushSettings", dis_FlushSettings, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_build_pmods_list", dis_build_pmods_list, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_download_file_from_pmod", dis_download_file_from_pmod, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_shutdown_pmodserver", dis_shutdown_pmodserver, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_calc_file_checksum", dis_calc_file_checksum, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_get_pmod_pdf_checksum", dis_get_pmod_pdf_checksum, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_expand_envvars", dis_expand_envvars, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_init_access_shell", dis_init_access_shell, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_client_access_eval", dis_client_access_eval, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_client_access_eval_async", dis_client_access_eval_async, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_exit", dis_exit, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_init", dis_package_init, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_uninit", dis_package_uninit, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_open_session", dis_package_open_session, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_request", dis_package_request, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_set_status_auto", dis_package_set_status_auto, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_close_session", dis_package_close_session, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_make_report", dis_package_make_report, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_vlistitem_has_sel", dis_vlistitem_has_sel, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_app_package", dis_package_app_package, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_remove_app", dis_package_remove_app, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_insert_app", dis_package_insert_app, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_partition", dis_package_partition, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_delete_session", dis_package_delete_session, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_package_new_session", dis_package_new_session, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_setPDFname", dis_setPDFname, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_vlistitem_num_items", dis_vlistitem_num_items, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_build_dead_dirs", dis_build_dead_dirs, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_vdasserv", dis_vdasserv, NULL, NULL);
  Tcl_CreateCommand (interp, "dis_remove_directory", dis_remove_directory, NULL, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//	arguments
// 1: script to execute when you finish
// 2: name of text item to display results in 
// 3: log file name
// 4+2n  commands to be executed
// 5+2n: comments to be dumped in the log file
// Optional (used only on nt):
// if the last one (argc-1) is "UseProgressBar" then the second to last one (argc-2)
// is the series of messages to be displayed
//  must be in the form  {message1}{message2}{message3}
// be careful, as the parser for this item is fragile.
//////////////////////////////////////////////////////////////////////////////////////////////////


int dis_CreateBatch(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  int nRet = TCL_ERROR;
  if( argc >= 6 )
  {
	int UseProgressBar= (strcmp(argv[argc-1], "UseProgressBar")==0);
    int nCommands = (argc - 4)/2;
	//if the we are using hte progress bar, than the last two are comments, tag
	if (UseProgressBar)
		nCommands--;

    char **cmds = new char *[nCommands];
    if( cmds )
    {
      char **comments = new char *[nCommands];
      if( comments )
      {
        for( int i=0; i<nCommands; i++ )
        {
          cmds[i] = argv[2*i + 4];
          comments[i] = argv[2*i + 5];
        }
       
		if( UseProgressBar)
		{
			if( monitorBatch::Create( nCommands, cmds, comments, interp, argv[1], argv[2], argv[3], argv[argc-2] ) )
				nRet = TCL_OK;
		}
		else
		{
			if( monitorBatch::Create( nCommands, cmds, comments, interp, argv[1], argv[2], argv[3] ) )
				nRet = TCL_OK;
		}
		delete comments;
      }
      delete cmds;
    }
  }
  else
    TCL_EXECERR ("wrong number of arguments")
  return nRet;
}

int dis_CancelBatch(ClientData, Tcl_Interp* , int , char * [])
{
  int nRet = TCL_OK;
  monitorBatch::Cancel();
  return nRet;
}


Settings *settings_instance = NULL;

//argv[1] - group name
//argv[2] - pref file name
//argv[3] (optional) - persistent flag if closing old settings 
int dis_OpenSettings(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  int nRet = TCL_OK;
  char pszResult[10];
  int nResult = 0;
  if( argc > 2 )
  {
    Settings *new_settings_instance = new Settings( argv[1] );
    if( new_settings_instance )
      if( new_settings_instance->open( argv[2] ) != -1 )
      {
        nResult = 1;
        if( settings_instance )
        {
          int fPersistent = 0;
          if( argc > 3 )
            fPersistent = atoi( argv[3] );
          settings_instance->close( fPersistent );
          delete settings_instance;
        }
        settings_instance = new_settings_instance;
      }
  }
  sprintf( pszResult, "%d", nResult );
  Tcl_SetResult (interp, pszResult, TCL_VOLATILE ); 
  return nRet;
}

int dis_CloseSettings(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  int nRet = TCL_OK;
  char pszResult[10];
  int nResult = 0;
  int fPersistent = 0;
  if( argc > 1 )
    fPersistent = atoi( argv[1] );
  if( settings_instance )
  {
    if( settings_instance->close( fPersistent ) != -1 )
      nResult = 1;
    delete settings_instance;
    settings_instance = NULL;
  }
  sprintf( pszResult, "%d", nResult );
  Tcl_SetResult (interp, pszResult, TCL_VOLATILE ); 
  return nRet;
}

int dis_ReadSettingsVar(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  int nRet = TCL_ERROR;
  const char *pszValue = NULL;
  if( argc > 1 && settings_instance)
  {
    Settings::Item item( (const char *)argv[1] );
    if( settings_instance->read( item ) != -1 )
      if( item.read( &pszValue )!=-1 && pszValue )
      {
        Tcl_SetResult (interp, (char *)pszValue, TCL_VOLATILE );
        nRet = TCL_OK;
      }
  }
  return nRet;
}

int dis_WriteSettingsVar(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  int nRet = TCL_OK;
  char pszResult[10];
  int nResult = 0;
  if( argc > 2 && settings_instance)
  {
    Settings::Item item( argv[1] );
    if( item.write( argv[2] ) != -1 )
      if( settings_instance->write( item )!=-1 )
        nResult = 1;
  }
  sprintf( pszResult, "%d", nResult );
  Tcl_SetResult (interp, pszResult, TCL_VOLATILE ); 
  return nRet;
}

int dis_FlushSettings(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  int nRet = TCL_OK;
  char pszResult[10];
  int nResult = 0;
  if( settings_instance )
  {
    char *pszNewFile = NULL;
    if( argc > 1 )
      pszNewFile = argv[1];
    if( settings_instance->flush( pszNewFile ) != -1 )
      nResult = 1;
  }
  sprintf( pszResult, "%d", nResult );
  Tcl_SetResult (interp, pszResult, TCL_VOLATILE ); 
  return nRet;
}

int dis_build_pmods_list(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
   int nRet = TCL_OK;
   if( argc > 1 )
     build_pmod_servers_list( interp, argv[1] );
   else
    Tcl_SetResult (interp, "", TCL_STATIC );
   return nRet;
}

/************************************************************************
* argv[1] - service name (required)
* argv[2] - save received file as (required)
* argv[3] - type (optional, defaults to 0. 0 - default pdf, 1 - default prefs, 2 - any file)
* argv[4] - filename to get (for type 2), optional
*************************************************************************/

int dis_download_file_from_pmod(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
   char pszCheckSum[ 20 ];
   int nRet = TCL_ERROR;
   int checksum = 0;
   char *pszPmodFile = NULL;
   int type = 0;
   if( argc > 2 )
   {
     char *pszServiceName = argv[1];
     char *pszSaveAs = argv[2];
     if( argc > 3 )
       type = atoi( argv[3] );
     if( argc > 4 )
       pszPmodFile = argv[4];
     else if( type==2 )
       type = 0;
     if( download_file_from_pmod( type, pszServiceName, 
                                  pszPmodFile, pszSaveAs, checksum ) )
       nRet = TCL_OK;
   }
   sprintf( pszCheckSum, "%d", checksum );
   Tcl_SetResult (interp, pszCheckSum, TCL_VOLATILE );
   return nRet;
}

int dis_shutdown_pmodserver( ClientData, Tcl_Interp* interp, int argc, char *argv[] )
{
  char pszResult[10];
  int nResult = 0;
  if( argc > 1 )
    nResult = shutdown_pmodserver( argv[1] );
  sprintf( pszResult, "%d", nResult );
  Tcl_SetResult (interp, pszResult, TCL_VOLATILE );
  return TCL_OK;  
}

int dis_get_pmod_pdf_checksum( ClientData, Tcl_Interp* interp, int argc, char *argv[] )
{
  char pszResult[20];
  int nResult = 0;
  if( argc > 1 )
    nResult = get_pmod_pdf_checksum( argv[1] );
  sprintf( pszResult, "%d", nResult );
  Tcl_SetResult (interp, pszResult, TCL_VOLATILE );
  return TCL_OK;  
}


int dis_calc_file_checksum( ClientData, Tcl_Interp* interp, int argc, char *argv[] )
{
  char pszResult[20];
  int nResult = 0;
  if( argc > 1 )
    nResult = calc_file_checksum( argv[1] );
  sprintf( pszResult, "%d", nResult );
  Tcl_SetResult (interp, pszResult, TCL_VOLATILE );
  return TCL_OK;  
}

int dis_expand_envvars (ClientData, Tcl_Interp *interp, int argc, char **argv)
{
  char *pszResult = "";
  char *pszExpanded = NULL;
  if( argc > 1 )
  {
    pszExpanded = pdexpandenv( argv[1] );    
    if( pszExpanded )
      pszResult = pszExpanded;
    else
      pszResult = argv[1];
  }
  Tcl_SetResult (interp, pszResult, TCL_VOLATILE );
  if( pszExpanded )
    FREE_MEMORY( pszExpanded );
  return TCL_OK;
}

static void AccessReqProcessFunc ( shellTextItem *item, const vchar *text )
{
  item->BlockInput();
  Application *appl = Application::findApplication("DISmain");
  int nId = -1;
  Viewer* vr;
  View *v;
  Viewer::GetExecutingView (vr, v);
  if (vr && v)
    nId = v->GetLayer()->getId();
  if( appl )
    rcall_cli_process_shell_request( appl, (char *)text, nId, (char *)item->GetTag());
}


int dis_init_access_shell (ClientData, Tcl_Interp *interp, 
                               int argc, char *argv[]) 
{
  int nRet = TCL_ERROR;
  if (argc < 2) 
    TCL_EXECERR("wrong number of arguments")
  shellTextItem *item = NULL;
  Viewer* vr;
  View *v;
  Viewer::GetExecutingView (vr, v);
  if (vr && v)
    item = shellTextItem::CastDown(v->FindItem ((vchar*) argv[1]));
  if( item )
  {
    nRet = TCL_OK;
    item->SetProcessFunc( AccessReqProcessFunc );
  }
  return nRet;
}

int dis_client_access_eval (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    int nRet = TCL_ERROR;
    if (argc < 2) 
	TCL_EXECERR("wrong number of arguments")
	    char *pszResult = NULL;
    Application *appl = Application::findApplication("DISmain");
    if( appl && appl->mySession())
	nRet = rcall_dis_access_eval (appl, argv[1], pszResult);
    if( pszResult )
	{
	    Tcl_SetResult (interp, pszResult, TCL_VOLATILE );
	    free( pszResult );
	}
    else
	Tcl_SetResult (interp, "Could not locate Access interpreter", TCL_VOLATILE );
    return nRet;
}

int dis_client_access_eval_async (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    if (argc < 2) 
	TCL_EXECERR("wrong number of arguments")

    Application *appl = Application::findApplication("DISmain");
    if( appl && appl->mySession())
	rcall_dis_access_eval_async (appl, argv[1]);
    return TCL_OK;
}

int dis_exit (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
  exit( EXIT_FAILURE );
  return TCL_OK;
}

int dis_setPDFname (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
  int nRet = TCL_ERROR;
  if (argc < 2) 
    TCL_EXECERR("wrong number of arguments")
  else
  {
    nRet = TCL_OK;
    gPDFName = argv[1];
  }
  return nRet;
}

int dis_vlistitem_num_items(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 1 )
  {
    nRet = TCL_OK;
    char pszRes[12];
    int nItems = 0;
    char *pszAppListTag = argv[1];
    vlistitem *items_list = vlistitem::CastDown((vdialogItem *)dis_findItem(interp, (vchar*)pszAppListTag));
    if( items_list )
      nItems = items_list->GetItemCount();
    sprintf( pszRes, "%d", nItems );
    Tcl_SetResult( interp, pszRes, TCL_VOLATILE );
  }
  else
    TCL_EXECERR("wrong number of arguments")
  return nRet;
}
//builds list of discover directories which ought to be deleted.
//usage is dis_build_dead_dirs [-n]
// returns a TCL list of dead dirs or (-n) the number of dead dirs

int dis_build_dead_dirs(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
	int nRet=TCL_ERROR;
	if (argc <3)
	{
		char * t=NULL;
		int x=0;
		x=build_dead_dirs(t);
		nRet=TCL_OK;
		if (argc==2 && !strcmp(argv[1],"-n"))
		{
		  char  pszResult [4];
		  sprintf(pszResult, "%d%",x);
		  Tcl_SetResult(interp, pszResult, TCL_STATIC);
		}
		else {
		Tcl_SetResult(interp, t, TCL_VOLATILE);
		}
		if (t)
		FREE_MEMORY(t);
	}
	else
		TCL_EXECERR("syntax: dis_build_dead_dirs [-n]")

	return nRet;
}



int dis_vdasserv(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
	int  nState=-1;

	if (argc == 2)
	{
		if (strcmp(argv[1], "isalive")==0)
		{
			nState = vdasserv_isalive();
		}
		else if (strcmp(argv[1], "purge")==0) 
		{
			nState = 0;
			if (vdasserv_isalive()) {
				if (vdasserv_purge() == 0)
					nState = 1;
			}
		}
	}

	int nRet = TCL_ERROR;
	if (nState >= 0)
	{
		char pszResult[10];
		sprintf( pszResult, "%d", nState );		
		Tcl_SetResult(interp, pszResult, TCL_STATIC);	
		nRet = TCL_OK;
	}
	else
	{
		Tcl_SetResult(interp, "Incorrect Arguments--syntax is dis_vdasserv <action>", TCL_STATIC);
	}

	return nRet;
}
 
int dis_remove_directory(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
	int nRet=TCL_ERROR;
	int nState=-1;
	switch (argc)
	{
	case 1: 
		Tcl_SetResult (interp, "dis_remove_directory <dirname> [-r]", TCL_STATIC);
		break;
	case 2:
		nState=pdrmdir(argv[1]);
		break;
	default:
		if (!strcmp(argv[2], "-r"))
			nState=pdrmdirr(argv[1]);
		else
			nState=pdrmdir(argv[1]);
	}
	if (argc != 1)
	{
	  	char pszResult[20];
		sprintf( pszResult, "%d", nState );		
		Tcl_SetResult(interp, pszResult, TCL_STATIC);	
		nRet = TCL_OK;
	}
	return nRet;
}



