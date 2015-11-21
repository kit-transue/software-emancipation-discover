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
// Error Output UI
//
////////////////////////////////////////////////////////////////////////////////

//++C++
#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C++

//++GENERAL
#include <msg.h>
#include <general.h>
#include <genError.h>
#include <genString.h>
#include <messages.h>
#include <systemMessages.h>
//--GENERAL

//++GT
#include <gt.h>
#include <gtDlgTemplate.h>
#include <gtFileSel.h>
#include <gtTextEd.h>
//--GT

//++UI
#include <QualityCockpitUI.h>
#include <errorOutputUI.h>
//--UI

QualityCockpitUI::errorOutputUI::errorOutputUI(genString const &errors)
{
  Initialize(QualityCockpitUI::errorOutputUI::errorOutputUI(genString const &));

  dialog = gtDialogTemplate::create(NULL, "errors", TXT("Errors"));
  
  // create the text box
  error_txt = gtTextEditor::create(dialog, "error_text", errors.str(), 0);
  error_txt->num_rows(20);
  error_txt->num_columns(40);
  error_txt->attach_tblr();
  error_txt->read_only_mode();
  error_txt->manage();

  // add Dismiss, Save buttons
  dialog->add_button("ok", TXT("Dismiss"), Dismiss_CB, this);
  dialog->add_button("cancel", TXT("Save Error Text"), Save_CB, this);

  done = false;
  filebrowser = 0;
}


QualityCockpitUI::errorOutputUI::~errorOutputUI(void)
{
  Initialize(QualityCockpitUI::errorOutputUI::~errorOutputUI(void));
  delete dialog;
  delete filebrowser;
}

void QualityCockpitUI::errorOutputUI::show(void)
{
  if ( dialog )
    {
      dialog->popup(1);
      dialog->take_control(waitUntilDone_CB, this);
      dialog->popdown();
    }
}

int QualityCockpitUI::errorOutputUI::waitUntilDone_CB(void *obj)
{
  int retval = 0;

  errorOutputUI *ui = (errorOutputUI *) obj;
  if ( ui )
    {
      retval = ui->done ? 1 : 0;
    }

  return retval;
}

void QualityCockpitUI::errorOutputUI::Save_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::errorOutputUI::Save_CB(gtPushButton*, gtEventPtr, void *obj, gtReason));

  errorOutputUI *ui = (errorOutputUI *) obj;
  if ( ui )
    {
      if ( ! ui->done )
	{
	  ui->filebrowser = gtFileSelector::create(ui->dialog, "selector", filesel_OK_CB, ui, filesel_cancel_CB, ui);
	  ui->filebrowser->title(TXT("Save Error Text"));
	  ui->filebrowser->manage();
	}
    }
}

void QualityCockpitUI::errorOutputUI::Dismiss_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::errorOutputUI::cancel_CB(gtPushButton*, gtEventPtr, void *obj, gtReason));

  errorOutputUI *ui = (errorOutputUI *) obj;
  if ( ui )
    {
      ui->done = true;
    }
}

void QualityCockpitUI::errorOutputUI::filesel_OK_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::errorOutputUI::filesel_OK_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  errorOutputUI *ui = (errorOutputUI *) obj;
  if ( ui && ui->filebrowser )
    {
      char const *filename = ui->filebrowser->selected();
      if ( filename )
	{
	  ofstream file(filename);
	  if ( file )
	    {
	      file << ui->error_txt->text();
	      ui->done = true;
	    }
	  else
	    {
	      msg("ERROR: File '$1' could not be opened for writing.") << filename << eom;
	    }
	}
    } 
}

void QualityCockpitUI::errorOutputUI::filesel_cancel_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::errorOutputUI::filesel_cancel_CB(gtPushButton*, gtEventPtr, void *, gtReason));

  errorOutputUI *ui = (errorOutputUI *) obj;
  if ( ui )
    {
      if ( ui->filebrowser )
	{
	  delete ui->filebrowser;
	  ui->filebrowser = 0;
	}
      ui->done = true;
    }
}


