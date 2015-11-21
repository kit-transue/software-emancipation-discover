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
#include "cLibraryFunctions.h"
#include <stdlib.h>
#include "uiexec.h"
#include "uicmds.h"
#include "uievent.h"
#include "uirqst.h"
#include "uistat.h"
#include "uilink.h"
#include "uicbck.h"
#include "parray.h"
#include "pdumem.h"
#include "pkArgs.h"
#include "pkerror.h"
#include "gxupdate.h"
#include "gview.h"
#include "gviewer.h"
#include "gString.h"
#include "gdialogLayer.h"
#include "packageObjs.h"
#include <vport.h>
#include vcomboHEADER
#include vlistitemHEADER
#include <tcl.h>
#include "machdep.h"
#include "Application.h"
#include "../../DIS_main/interface.h"

extern gString gPDFName;
static const char* WRONG_ARG_NUM = "Wrong number of arguments!";
static galaSessionOpen *sess_open_mon = NULL;
static galaSessionClose *sess_close_mon = NULL; 
static galaSessionRemove *sess_remove_mon = NULL; 

static updateList *old_report_notifier = (updateList *)0;
static updateList *old_appls_notifier = (updateList *)0;

 
static int provide_current_layer_id()
{
  int nRet = -1;
  Viewer* vr;
  View *v;
  Viewer::GetExecutingView (vr, v);
  if (vr && v)
    nRet = v->GetLayer()->getId();
  return nRet;
}

int dis_package_init(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  int nId = 0;
  if( argc > 1 )
    nId = atoi( argv[1] );
  
  int nLayerId = provide_current_layer_id();
  
  rqstStatGala *pStat = (rqstStatGala *)ui_provideStatCtrl(0);
  if( pStat )
    pStat->SetLayerId( nLayerId );
  if( !pkerror_init(1) )
  {
    uiexec *exec = new uiexec( nId );
    if( exec )
    {
      if( exec->open( argc, argv ) == 0 )
        nRet = TCL_OK;
      sess_open_mon = new galaSessionOpen( nLayerId );
      sess_close_mon = new galaSessionClose( nLayerId );
      sess_remove_mon = new galaSessionRemove( nLayerId );
    }
  }
  return nRet;
}

int dis_package_uninit(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nId = 0;
  if( argc > 1 )
    nId = atoi( argv[1] );
  uiexec *exec = uiexec::package(nId);
  if( exec )
  {
    delete exec;
    pkerror_init(0);
    if( sess_open_mon )
    {
      delete sess_open_mon;
      sess_open_mon = NULL;
    }
    if( sess_close_mon )
    {
      delete sess_close_mon;
      sess_close_mon = NULL;
    }
    if( sess_remove_mon )
    {
      delete sess_remove_mon;
      sess_remove_mon = NULL;
    }

    if( old_report_notifier )
    {
      delete old_report_notifier;
      old_report_notifier = NULL;
    }

    if( old_appls_notifier )
    {
      delete old_appls_notifier;
      old_appls_notifier = NULL;
    }
  }
  return TCL_OK;
}


int dis_package_open_session(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  
  if( argc > 1 )
  {
    int nLayerId = provide_current_layer_id();
    
    nRet = TCL_OK;
    
    int nId = 0;
    if( argc > 2 )
      nId = atoi( argv[2] );
    
    char *pszSessionName = argv[1];
    if( old_appls_notifier )
      delete old_appls_notifier;
    old_appls_notifier = new galaMonAppls(PACK_APP_LIST, pszSessionName, nLayerId );
    execCmd_Session(pszSessionName, pkArgVal_OPEN);
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

int dis_package_close_session(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 1 )
  {
    nRet = TCL_OK;
    execCmd_RptClose(argv[1]);
    execCmd_Session(argv[1], pkArgVal_CLOSE);
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

 
int dis_package_request(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 3 )
  {
    nRet = TCL_OK;
    int req_id = atoi( argv[1] );
    int gxrqst_type = -1;
    char *pszTag = argv[2];
    char *pszSession = argv[3];
    updateList * not_obj = (updateList *)0;
    int nId = provide_current_layer_id();
    switch( req_id )
    {
    case 0: //Applications
      gxrqst_type = gxList_APPLICATION;
      not_obj = new galaMonAppls(pszTag, pszSession, nId );
      break;
    case 1: //Call Clusters
      gxrqst_type = gxList_CALL_CLUSTER;
      not_obj = new galaMonCCls(pszTag, pszSession, nId );
      break;
    case 2: //Data Clusters
      gxrqst_type = gxList_DATA_CLUSTER;
      not_obj = new galaMonDCls(pszTag, pszSession, nId );
      break;
    case 3: //Entities
      gxrqst_type = gxList_ELEMENTS;
      not_obj = new galaMonElems(pszTag, pszSession, nId );
      break;
    }
    if( gxrqst_UpdateList( gxrqst_type, pszSession ) != 0 )
    {
      delete not_obj;
      not_obj = (updateList *)0;
    }
    if( not_obj )
    {
      if( old_report_notifier )
        delete old_report_notifier;
      old_report_notifier = not_obj;
    }
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}



int dis_package_make_report (ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 6 )
  {
    nRet = TCL_OK;
    int elem_val = atoi( argv[1] );
    int req_id = atoi( argv[2] );
    int gxrqst_type = -1;
    char *pszSession = argv[3];
    char *rptTypeTag = argv[4];
    char *rptFile = argv[5];
    char *rptListTag = argv[6];
   
    int nId = provide_current_layer_id();

    switch( req_id )
    {
    case 0: //Applications
      gxrqst_type = gxList_APPLICATION;
      break;
    case 1: //Call Clusters
      gxrqst_type = gxList_CALL_CLUSTER;
      break;
    case 2: //Data Clusters
      gxrqst_type = gxList_DATA_CLUSTER;
      break;
    case 3: //Entities
      gxrqst_type = gxList_ELEMENTS;
      break;
    }
    char * elet = (char *)0;
	  int objt = gxlist_execCmdObject(gxrqst_type, &elet);

    pkArgVal_ID report_type = pkArgVal_TEXT_ASCII;
    vcombo *type_combo = vcombo::CastDown( LayerFindItem(nId, rptTypeTag) );
    vstr *type = NULL;
    if( type_combo && (type = type_combo->GetTextAsString()) )
    {
	    if( !strcmp( (const char *)type, "List" ) )
        report_type = pkArgVal_TEXT_LIST;
      else if( !strcmp( (const char *)type, "Group" ) )
        report_type = pkArgVal_TEXT_GROUP;
      vstrDestroy( type );
    }
    vlistitem *rpt_elements = vlistitem::CastDown(LayerFindItem(nId, rptListTag));
    if( rpt_elements )
    {
      parray elems(8);
      parray dests(8);
      vlistitem_GetStrSelection(rpt_elements, &elems);
      dests.insert((void *)rptFile);
      dests.insert((void *)elet);
      dests.insert((void *)report_type);
      if (elem_val)
		    dests.insert((void *)".");
      execCmd_Report((char *)pszSession, objt, &elems, &dests);
    }
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

int dis_package_set_status_auto(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 1 )
  {
    nRet = TCL_OK;
    rqstStatGala *pStat = (rqstStatGala *)ui_provideStatCtrl(0);
    if( pStat )
      pStat->SetAuto( atoi(argv[1]) );
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}


int dis_vlistitem_has_sel(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 1 )
  {
    nRet = TCL_OK;
    char pszSel[2];
    int fSel = 0;
    char *pszAppListTag = argv[1];
    int nLayerId = provide_current_layer_id();
    vlistitem *app_list = vlistitem::CastDown(LayerFindItem(nLayerId, pszAppListTag));
    if( app_list && app_list->GetSelectedItem()!=
        vlistitemNO_CELLS_SELECTED )
      fSel = 1;
    sprintf( pszSel, "%d", fSel );
    Tcl_SetResult( interp, pszSel, TCL_VOLATILE );
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

int dis_package_app_package(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 3 )
  {
    nRet = TCL_OK;
    char *pszAppListTag = argv[3];
    char *pszSession = argv[1];
    char *pszSrcFile = argv[2];
    int nLayerId = provide_current_layer_id();
    vlistitem *app_list = vlistitem::CastDown(LayerFindItem(nLayerId, pszAppListTag));
    if( app_list )
    {
      parray update_arr(8);
      if (vlistitem_GetStrSelection(app_list, &update_arr) > 0) 
        execCmd_PackageUpdate(pszSession, &update_arr, pszSrcFile);
    }
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

int dis_package_insert_app(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 2 )
  {
    nRet = TCL_OK;
    char *pszSession = argv[1];
    char *pszApp = argv[2];
    char * ele_type = (char *)0;
	  int    obj_type = gxlist_execCmdObject(gxList_APPLICATION, &ele_type);
    execCmd_SessionIns(pszSession, obj_type, pszApp, ele_type);
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

int dis_package_remove_app(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 2 )
  {
    nRet = TCL_OK;
    char *pszAppListTag = argv[2];
    char *pszSession = argv[1];
    
    int nLayerId = provide_current_layer_id();
    vlistitem *app_list = vlistitem::CastDown(LayerFindItem(nLayerId, pszAppListTag));
    parray rm_arr(8);
    if (app_list && vlistitem_GetStrSelection(app_list, &rm_arr) > 0) 
    {
      char * elet = (char *)0;
      int    objt = gxlist_execCmdObject(gxList_APPLICATION, &elet);
      execCmd_SessionRmv(pszSession, objt, &rm_arr, elet);
    }
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

int dis_package_partition(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 1 )
  {
    nRet = TCL_OK;
    char *pszSession = argv[1];
    execCmd_Partition(pszSession, 0);
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

int dis_package_delete_session(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 1 )
  {
    nRet = TCL_OK;
    char *pszSession = argv[1];
    execCmd_RptClose(pszSession);
    execCmd_Session((char *)pszSession, pkArgVal_DELETE);
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;
}

int dis_package_new_session(ClientData, Tcl_Interp *interp, int argc, char * argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 4 )
  {
    char buff[20];
    int nFuncs = -1;
    nRet = TCL_OK;
    char *pszNewFile = argv[1];
    int fUseScope = atoi( argv[2] );
    char *pszScope = argv[3];
    int rtl_id = atoi( argv[4] );
    char *pszTmpFile = OSapi_tempnam( ".", NULL );
    if( pszTmpFile )
    {
      nFuncs = 0;
      Application *appl = Application::findApplication("DISmain");
      if( appl && appl->mySession())
        nFuncs = rcall_dis_create_package_input (appl, fUseScope, rtl_id, pszScope, pszTmpFile);
      if( nFuncs > 0 )
      {
        tmpFDEleteCbk *cbk = new tmpFDEleteCbk( pszTmpFile );
        const char *pszPDF = (const char *)0;
        if( gPDFName.not_null() )
          pszPDF = gPDFName;
        else
          pszPDF = "Discover.pdf";
        parray arg_arr(8);
        arg_arr.insert((void *)pszNewFile);
        arg_arr.insert((void *)pszPDF);
        arg_arr.insert((void *)pszScope);
        arg_arr.insert((void *)OSPATH(pszTmpFile));
        arg_arr.insert((void *)"rtl");
        
        int nLayerId = provide_current_layer_id();
        if( old_appls_notifier )
          delete old_appls_notifier;
        old_appls_notifier = new galaMonAppls(PACK_APP_LIST, pszNewFile, nLayerId );

        execCmd_SessionCreate(&arg_arr, cbk);
      }
      else
        OSapi_unlink( pszTmpFile );

      OSapi_free( pszTmpFile );
    }
    sprintf( buff, "%d", nFuncs );
    Tcl_SetResult( interp, buff, TCL_VOLATILE );
  }
  else
    Tcl_SetResult( interp, (char *)WRONG_ARG_NUM, TCL_STATIC );
  return nRet;

}



