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
////////////////////////////////////////////////////////////////////////////////
//
// Session List UI
//
////////////////////////////////////////////////////////////////////////////////

//++GENERAL
#include <msg.h>
#include <general.h>
#include <genError.h>
#include <genString.h>
#include <messages.h>
#include <systemMessages.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
//--GENERAL

//++GT
#include <gt.h>
#include <gtDlgTemplate.h>
#include <gtList.h>
#include <gtStringEd.h>
//--GT

//++UI
#include <QualityCockpitUI.h>
#include <sessionListUI.h>
//--UI

QualityCockpitUI::sessionListUI::sessionListUI(bool allow_edit)
{
  Initialize(QualityCockpitUI::sessionListUI::sessionListUI(bool));

  dialog = gtDialogTemplate::create(NULL, "session_mgr", TXT("Session Manager"));
  
  // create the list
  session_list = gtList::create(dialog, "session_list", TXT("Available Sessions"), gtBrowse, 0, 0);
  session_list->attach(gtTop);
  session_list->attach(gtRight);
  session_list->attach(gtLeft);
  session_list->manage();

  // create the text box
  selection_txt = gtStringEditor::create(dialog, "selected_session", NULL);
  selection_txt->attach(gtLeft);
  selection_txt->attach(gtRight);
  selection_txt->attach(gtTop, session_list, 10);
  selection_txt->manage();
  if ( ! allow_edit )
    selection_txt->read_only();

  // add OK and cancel buttons
  dialog->add_button("ok", TXT("OK"), OK_CB, this);
  dialog->add_button("cancel", TXT("Cancel"), cancel_CB, this);
}


QualityCockpitUI::sessionListUI::~sessionListUI(void)
{
  delete dialog;
}

int QualityCockpitUI::sessionListUI::getSession(genString const *sessions, int num, genString &selection)
{
  int retval = -1;

  // convert genStrings to char **
  char const **strings = 0;
  
  if ( sessions && num > 0 )
    {
      strings = new char const * [num];
      if ( strings )
	for ( int i = 0; i < num; i++ )
	  strings[i] = sessions[i].str();
    }
  
  // setup list
  session_list->delete_all_items();
  
  if ( sessions && num > 0 )
    {
      // add items to list
      session_list->add_items(strings, num, 0);
      session_list->select_pos(1, 0);
      session_list->select_callback(setSessionText_CB, this);
      
      // setup text editor
      selection_txt->text(strings[0]);
    }
  
  // set vars
  selection_made = false;
  cancelled = false;
  
  // set return
  selection = "";
  
  // display and wait
  dialog->popup(1);
  dialog->take_control_top(waitForSelection_CB, this);
  dialog->popdown();
  
  // get the selection (on OK, not on CANCEL)
  if ( selection_made )
    {
      selection = selection_txt->text();
      retval = 1;
    }
  else if ( cancelled )
    {
      retval = 0;
    }
  
  delete [] strings;

  return retval;
}

void QualityCockpitUI::sessionListUI::setSessionText_CB(gtList*, gtEventPtr, void *obj, gtReason)
{
  sessionListUI *ui = (sessionListUI *) obj;
  if ( ui )
    {
      if ( ! ui->selection_made && ! ui->cancelled )
	{
	  int *positions;
	  int num_positions;
	  ui->session_list->get_selected_pos(&positions, &num_positions);
      
	  if ( num_positions > 0 )
	    {
	      // adjust for the difference in Motif/gt posisitioning
	      int position = 0;
	      if ( *positions == 0 )
		position = ui->session_list->num_items() - 1;
	      else
		position = *positions - 1;
	      
	      // get the item
	      ui->selection_txt->text(ui->session_list->item_at_pos(position));
	    }
	}
    }
}

int QualityCockpitUI::sessionListUI::waitForSelection_CB(void *obj)
{
  int retval = 0;

  sessionListUI *ui = (sessionListUI *) obj;
  if ( ui )
    {
      retval = ui->isSelectionSelected();
    }

  return retval;
}

int QualityCockpitUI::sessionListUI::isSelectionSelected(void)
{
  int retval = 0;

  if ( selection_made || cancelled )
    retval = 1;
  
  return retval;
}

void QualityCockpitUI::sessionListUI::OK_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::sessionListUI::OK_CB(gtPushButton*, gtEventPtr, void *obj, gtReason));

  sessionListUI *ui = (sessionListUI *) obj;
  if ( ui )
    {
      if ( ! ui->cancelled )
	{
	  char *selection = ui->selection_txt->text();
	  if ( selection || OSapi_strlen(selection) > 0 )
	    ui->selection_made = true;
	  else
	    {
	      msg("ERROR: Please enter a selection.") << eom;
	    }
	}
    }
}

void QualityCockpitUI::sessionListUI::cancel_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::sessionListUI::cancel_CB(gtPushButton*, gtEventPtr, void *obj, gtReason));

  sessionListUI *ui = (sessionListUI *) obj;
  if ( ui )
    {
      if ( ! ui->selection_made )
	ui->cancelled = true;
    }
}


