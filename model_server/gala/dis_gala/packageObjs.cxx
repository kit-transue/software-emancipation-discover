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
#include <stdlib.h>
#include <string.h>
#include <vport.h>
#include vstrHEADER
#include vlistitemHEADER
#include vapplicationHEADER
#include vwindowHEADER
#include vnotebookHEADER
#include <tcl.h>
#include "gbar.h"
#include "uiexec.h"
#include "uicmds.h"
#include "uievent.h"
#include "uirqst.h"
#include "uistat.h"
#include "uicbck.h"
#include "gxupdate.h"
#include "gview.h"
#include "gviewer.h"
#include "gdialogLayer.h"
#include "parray.h"
#include "pdumem.h"
#include "pdustring.h"
#include "pkArgs.h"
#include "pdutype.h"
#include "packageObjs.h"
#include "gcontrolObjects.h"
#include "machdep.h"

extern vdialogItem* find_item_recurs(vdialogItemList* itemList, const vname* tagName);


const char *rqstStatGala::STATUS_MSG_TAG = "statStatusVal";
const char *rqstStatGala::OUTPUT_MSG_TAG = "statOutputVal";
const char *rqstStatGala::RQST_NAME_TAG  = "statRqstVal";
const char *rqstStatGala::COMPL_PERCENT_TAG = "statComplVal";
const char *rqstStatGala::STATUS_PROGRESS_TAG = "tagStatProgress";
const char *rqstStatGala::COMPL_TIME_TAG = "statEstTimeVal";

const char *PACK_CURRENT_SESSION_TAG = "tagCurrSession";

extern "C" int ui_enterEventLoop(int proc_id)
{
  return 0;
}

extern "C" int
ui_initializeClient(int argc, char ** argv)
{
  return 0;
}


extern "C" rqstStat *
ui_provideStatCtrl(int proc_id)
{
    static rqstStatGala stat;
    return &stat;
}

int vlistitem_GetStrSelection(vlistitem * list, parray * str_arr)
{
  int ret_val = -1;
  if (list!=0 && str_arr!=0) 
  {
    ret_val = 0;
    vlistSelection * sel_items = list->GetSelection ();
    if (sel_items) 
    {
      vlistIterator iter;
      iter.StartWithSelection(sel_items);
      while (iter.Next()) 
      {
        int ndx = iter.GetRow();
        if (ndx >= 0) 
        {
          if (str_arr->insert((void *)(list->GetItemValue(ndx))) >= 0)
          ret_val++;
        }
      }
      iter.Finish ();
    }
  }
  return ret_val;
}


static int vtextitem_AppendText(vtextitem * txt_item, char * new_data)
{
    int ret_val = -1;
    if (txt_item!=0 && new_data!=0) 
    {
      vbool is_modif = txt_item->IsModifyable();
      txt_item->SetModifyable( vTRUE );
      vtextitemText *text = vtextitemText::CastDown( txt_item->GetTextData() );
      if( text )
      {
        vtextSelection *sel = text->CreateSelection();
        if( sel )
        {
          text->SelectRange( sel, text->GetLength(), text->GetLength() );
          text->InsertText( sel, (const vchar *)new_data, strlen(new_data) );
          txt_item->Scroll( text->GetLineFromPosition( text->GetLength() ), 0 );
          text->DestroySelection( sel );
        }
      }
      txt_item->SetModifyable( is_modif );
    }
    return ret_val;
}


vdialogItem *LayerFindItem( int nLayerId, const char *tag )
{
  vdialogItem *ret = NULL;
  Pane *pane = NULL;
  Layer *lr = Layer::find_layer( nLayerId );
  if( lr )
    pane = lr->getPane();
  if( pane )
  {
    const vname * name = vnameInternGlobal( (const vchar*) tag );
    if( name )
    {
      ret = vdialogItem::CastDown(pane->FindItem (name));
      //if item is not a direct child of the pane, traverse all children
      if( !ret )
        ret = vdialogItem::CastDown(find_item_recurs(pane->GetItemList(), name));
    }
  }
  return ret;
}


static void reset_items( int nLayerId )
{
  vlistitem *list_item = vlistitem::CastDown(LayerFindItem( nLayerId, 
                                PACK_APP_LIST ));
  if( list_item )
    list_item->SetItemCount(0);
  
  list_item = vlistitem::CastDown(LayerFindItem( nLayerId, 
                                PACK_REP_LIST ));
  if( list_item )
    list_item->SetItemCount(0);

#if 0
  rqstStatGala *stat = (rqstStatGala *)ui_provideStatCtrl(0);
  if( stat )
  {
    int is_auto = stat->IsAuto();
    stat->statEnterRqst("");
    stat->statExitRqst(0);
    stat->SetAuto( is_auto );
  }
#endif
}

char * rqstStatGala::statRdSession()
{
  char *ret = NULL;
  gdLabel *item = gdLabel::CastDown(LayerFindItem( m_nLayerId, 
                              PACK_CURRENT_SESSION_TAG ));
  if( item )
    ret = (char *)item->GetTitle();
  return ret;
}

int rqstStatGala::statEnterRqst(char *rqst)
{
  int ret_val = -1;
  statWrRqstName("None");
	statWrCompletion(0.0);
	statWrEstTime(0.0);
  vtextitem *status_msgs = vtextitem::CastDown( LayerFindItem(m_nLayerId, STATUS_MSG_TAG) );
  if( status_msgs )
    status_msgs->SetText((const vchar *)"");
  vtextitem *output_msgs = vtextitem::CastDown( LayerFindItem(m_nLayerId, OUTPUT_MSG_TAG) );
  if (output_msgs)
	  output_msgs->SetText((const vchar *)"");
  ProgressItem *compl_slider = ProgressItem::CastDown( LayerFindItem(m_nLayerId, STATUS_PROGRESS_TAG) );
  if( compl_slider )
  {
    compl_slider->SetNumberSteps((unsigned)100);
    compl_slider->StartOver();
  }
  ChangeStatusState(1);
  return ret_val;
}

int rqstStatGala::statEnterUpdate(char *rqst)
{
  int ret_val = -1;
  if (rqst) 
  {
    updating_rqst = 1;
    ChangeStatusState(2);
  }
  return ret_val;
}

int rqstStatGala::statWrRqstName(char *rqst_name)
{
  int ret_val = -1;

  if (!updating_rqst && rqst_name!=0) 
  {
    vtextitem *request_name = vtextitem::CastDown( LayerFindItem(m_nLayerId, RQST_NAME_TAG) );
    if (request_name) 
    {
      request_name->SetText((vchar *)rqst_name);
      ret_val = 0;
    }
  }
  return ret_val;
}

int rqstStatGala::statWrCompletion(double compl_val)
{
  int ret_val = -1;

  if (!(updating_rqst || compl_val<0.0 || compl_val>100.0)) 
  {
    char val_str[DBL2CHAR];
    sprintf(val_str, "%.2lf", compl_val);
    vtextitem *compl_percent = vtextitem::CastDown( LayerFindItem(m_nLayerId, COMPL_PERCENT_TAG) );
    if (compl_percent)
      compl_percent->SetText((const vchar *)val_str);
    ProgressItem *compl_slider = ProgressItem::CastDown( LayerFindItem(m_nLayerId, STATUS_PROGRESS_TAG) );
    if (compl_slider)
      compl_slider->MoveTo((unsigned)compl_val);
    if (compl_percent || compl_slider)
    ret_val = 0;
  }
  return ret_val;
}

int rqstStatGala::statWrEstTime(double est_val)
{
  int ret_val = -1;

  if (!updating_rqst && est_val>=0.0) 
  {
    char val_str[DBL2CHAR];
    sprintf(val_str, "%.3lf", est_val);
    vtextitem *compl_time = vtextitem::CastDown( LayerFindItem(m_nLayerId, COMPL_TIME_TAG) );
    if (compl_time) 
    {
      compl_time->SetText((const vchar *)val_str);
      ret_val = 0;
    }
  }
  return ret_val;
}

int rqstStatGala::statWrStatus(char * new_text)
{
  int ret_val = -1;
  vtextitem *status_msgs = vtextitem::CastDown( LayerFindItem(m_nLayerId, STATUS_MSG_TAG) );
  if( status_msgs )
    ret_val = vtextitem_AppendText(status_msgs, new_text);
  return ret_val;
}

int rqstStatGala::statWrOutput(char * new_text)
{
  int ret_val = -1;  
  vtextitem *output_msgs = vtextitem::CastDown( LayerFindItem(m_nLayerId, OUTPUT_MSG_TAG) );
  if (output_msgs)
    ret_val = vtextitem_AppendText(output_msgs, new_text);
  return ret_val;
}

int rqstStatGala::statExitRqst(int)
{
  statWrCompletion(100.0);
  ChangeStatusState(0);
  updating_rqst = 0;
  return 0;
}


void rqstStatGala::ChangeStatusState(int val)
{

  if (val == 1) //set busy cursor
  {
    vapplication *app = vapplication::GetCurrent();
    if( app )
    {
      vapplicationWindowIterator iterator;               
      iterator.Start(app);
      while (iterator.Next())
      {
        vwindow *aWindow = iterator.GetWindow();
        if( !aWindow->IsBlocked() )
          aWindow->Block();
      }
      iterator.Finish();
    }
  }
  else //reset busy cursor
  {
    vapplication *app = vapplication::GetCurrent();
    if( app )
    {
      vapplicationWindowIterator iterator;               
      iterator.Start(app);
      while (iterator.Next())
      {
        vwindow *aWindow = iterator.GetWindow();
        if( aWindow->IsBlocked() )
          aWindow->Unblock();
      }
      iterator.Finish();
    }
  }
	if( m_fAutoDiaplay )
  {
    vnotebookPage *thePage = vnotebookPage::CastDown( LayerFindItem(m_nLayerId, "tagPkgPage3") );
    if( thePage )
      thePage->SetCurrent();
  }
}


int vlistitem_ReplaceList(vlistitem * list_item, parray * str_arr)
{
    int ret_val = -1;

    if (list_item) {
	int mode = list_item->GetUpdateMode();

	(void) list_item->SetUpdateMode(vlistviewDO_NOTHING);
	(void) list_item->SetItemCount(0);
	if (str_arr) {
	    ret_val = 0;

	    int size = str_arr->size();
	    if (size > 0) {
		(void) list_item->SetItemCount(size);
		for (int i=0; i<size; i++) {
		    const unsigned char * ch_data = (const unsigned char *)((*str_arr)[i]);
		    if (ch_data) {
			vscribe * ins_data = vcharScribe(ch_data);
			if (ins_data) {
			    (void) list_item->SetItemValueScribed(ins_data, ret_val);
			    ret_val++;
			}
		    }
                }
		if (ret_val != size)
		    (void) list_item->SetItemCount(ret_val);
            }
        }
	(void) list_item->SetUpdateMode(mode);
	(void) list_item->InvalView();
    }

    return ret_val;
}


galaMonAppls::galaMonAppls(const char *tag, char *session, int id)
:updateApplList(NULL, session)
{
  m_nLayerId = id;
  if( tag )
    m_Tag = vstrClone( (const vchar *)tag );
  else
    m_Tag = NULL;
}

galaMonAppls::~galaMonAppls()
{
  if( m_Tag )
    vstrDestroy( m_Tag );
}

int galaMonAppls::update(parray * elems)
{
  int id = GetLayerId();
  int nRet = -1;
  if( m_Tag )
  {
    vlistitem* item = vlistitem::CastDown(LayerFindItem(id, (const char *)m_Tag));
    if( item )
      nRet = vlistitem_ReplaceList(item, elems ); 
  }
  return nRet;
}

galaMonCCls::galaMonCCls(const char *tag, char *session, int id)
:updateCClList(NULL, session)
{
  m_nLayerId = id;
  if( tag )
    m_Tag = vstrClone( (const vchar *)tag );
  else
    m_Tag = NULL;
}

galaMonCCls::~galaMonCCls()
{
  if( m_Tag )
    vstrDestroy( m_Tag );
}

int galaMonCCls::update(parray * elems)
{
  int id = GetLayerId();
  int nRet = -1;
  if( m_Tag )
  {
    vlistitem* item = vlistitem::CastDown(LayerFindItem(id, (const char *)m_Tag));
    if( item )
      nRet = vlistitem_ReplaceList(item, elems ); 
  }
  return nRet;
}

galaMonDCls::galaMonDCls(const char *tag, char *session, int id)
:updateDClList(NULL, session)
{
  m_nLayerId = id;
  if( tag )
    m_Tag = vstrClone( (const vchar *)tag );
  else
    m_Tag = NULL;
}

galaMonDCls::~galaMonDCls()
{
  if( m_Tag )
    vstrDestroy( m_Tag );
}

int galaMonDCls::update(parray * elems)
{
  int id = GetLayerId();
  int nRet = -1;
  if( m_Tag )
  {
    vlistitem* item = vlistitem::CastDown(LayerFindItem(id, (const char *)m_Tag));
    if( item )
      nRet = vlistitem_ReplaceList(item, elems ); 
  }
  return nRet;
}


galaMonElems::galaMonElems(const char *tag, char *session, int id)
:updateElemsList(NULL, session)
{
  m_nLayerId = id;
  if( tag )
    m_Tag = vstrClone( (const vchar *)tag );
  else
    m_Tag = NULL;
}

galaMonElems::~galaMonElems()
{
  if( m_Tag )
    vstrDestroy( m_Tag );
}

int galaMonElems::update(parray * elems)
{
  int id = GetLayerId();
  int nRet = -1;
  if( m_Tag )
  {
    vlistitem* item = vlistitem::CastDown(LayerFindItem(id, (const char *)m_Tag));
    if( item )
      nRet = vlistitem_ReplaceList(item, elems ); 
  }
  return nRet;
}


int gxrqst_UpdateList(int type, char * session_name)
{
    int ret_val = -1;

    if (session_name) {
	rqstUpdate * urqst = (rqstUpdate *)0;

	switch (type) {
	  case gxList_APPLICATION:
	    urqst = new rqstUpdateApplList(session_name);
	    break;
	  case gxList_CALL_CLUSTER:
	    urqst = new rqstUpdateCClList(session_name);
	    break;
	  case gxList_DATA_CLUSTER:
	    urqst = new rqstUpdateDClList(session_name);
	    break;
	  case gxList_ELEMENTS:
	    urqst = new rqstUpdateElemsList(session_name);
	    break;
	  default:
	    break;
	}
	if (urqst) {
	    if ((ret_val=uiexec::package(0)->update(urqst)) < 0)
		delete urqst;
	}
    }

    return ret_val;
}

int gxlist_execCmdObject(int gx_type, char ** elem_type)
{
  int ret_val = -1;
  switch (gx_type) 
  {
    case gxList_APPLICATION:
      ret_val = pkRqstArg_APPLICATION;
    break;
    case gxList_DATA_CLUSTER:
      if (elem_type)
        *elem_type = "D";
    case gxList_CALL_CLUSTER:
      ret_val = pkRqstArg_CLUSTER;
    break;
    default:
    break;
  }
  return ret_val;
}

int galaSessionOpen::update( char *name )
{
  int ret_val = -1;
  gdLabel *item = gdLabel::CastDown(LayerFindItem( m_nLayerId, 
                                PACK_CURRENT_SESSION_TAG ));
  if( item )
  {
    ret_val = 0;
    item->SetTitle( (const vchar *)name );
    item->SetInterpVariable( (vchar *)PACK_CURRENT_SESSION_TAG, (const vchar*)name );
  }
  return ret_val;
}

int galaSessionClose::update( char *name )
{
  int ret_val = -1;
  gdLabel *item = gdLabel::CastDown(LayerFindItem( m_nLayerId, 
                                PACK_CURRENT_SESSION_TAG ));
  if( item )
  {
    ret_val = 0;
    if( !strcmp( (const char*)item->GetTitle(), name ) )
    {
      item->SetTitle( (const vchar *)"" );
      item->SetInterpVariable( (vchar *)PACK_CURRENT_SESSION_TAG, (const vchar*)"" );
    }
  }
  reset_items( m_nLayerId );
  return ret_val;
}

int galaSessionRemove::update( char *name )
{
  int ret_val = -1;
  gdLabel *item = gdLabel::CastDown(LayerFindItem( m_nLayerId, 
                                PACK_CURRENT_SESSION_TAG ));
  if( item )
  {
    ret_val = 0;
    if( !strcmp( (const char*)item->GetTitle(), name ) )
    {
      item->SetTitle( (const vchar *)"" );
      item->SetInterpVariable( (vchar *)PACK_CURRENT_SESSION_TAG, (const vchar*)"" );
    }

  }
  reset_items( m_nLayerId );
  return ret_val;
}

tmpFDEleteCbk::tmpFDEleteCbk( const char *name )
{
  m_pszFileName = pdstrdup( name );
}

tmpFDEleteCbk::~tmpFDEleteCbk()
{
  FREE_MEMORY( m_pszFileName );
}

long tmpFDEleteCbk::observe(int)
{
  long lRet = -1;
  if( m_pszFileName )
    lRet = OSapi_unlink( m_pszFileName );
  delete this;
  return lRet;
}
