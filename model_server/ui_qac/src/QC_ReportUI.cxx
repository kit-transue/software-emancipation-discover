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
// Quality Cockpit Report UI
//
////////////////////////////////////////////////////////////////////////////////

#include <cLibraryFunctions.h>
#include <msg.h>
#include <genError.h>
#include <help.h>
#ifndef ISO_CPP_HEADERS
#include <limits.h>
#else /* ISO_CPP_HEADERS */
#include <limits>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>
#include <messages.h>
#include <systemMessages.h>

#include <top_widgets.h>
#include <gtCascadeB.h>
#include <gtDisplay.h>
#include <gtDlgTemplate.h>
#include <gtFileSel.h>
#include <gtForm.h>
#include <gtFrame.h>
#include <gtHorzBox.h>
#include <gtLabel.h>
#include <gtList.h>
#include <gtMainWindow.h>
#include <gtMenuBar.h>
#include <gtOptionMenu.h>
#include <gtPushButton.h>
#include <gtRadioBox.h>
#include <gtSepar.h>
#include <gtStringEd.h>
#include <gtStringEd.h>
#include <gtTogB.h>
#include <gtTopShell.h>
#include <gtVertBox.h>
#include <gtBaseXm.h>

//+ACCESS
#include <TclList.h>
#include <Interpreter.h>
//--ACCESS

//++UI
#include <QC_ReportUI.h>
//--UI

//++LICENSING
#include <xxinterface.h>
//--LICENSING

////////////////////////////////////////////////////////////////////////////////
//
// Static initialization
//
////////////////////////////////////////////////////////////////////////////////

QC_ReportUI         *QC_ReportUI::instance = NULL;
 
////////////////////////////////////////////////////////////////////////////////
//
// Methods
//
////////////////////////////////////////////////////////////////////////////////

void QC_ReportUI::Invoke(char const *title, gtBase* pParent/*=NULL*/)
{
  Initialize(QC_ReportUI::Invoke(char const *title));
  
  if (instance) 
    {
      if(!instance->invoked)
	{
	  instance->dialog->popup(1);
	  instance->invoked = 1;
	}
      instance->dialog->bring_to_top();
    } 
  else 
    {
	  push_busy_cursor();
	  
	  if (!title) title = TXT("DIScover Caliper Report");
	  instance = new QC_ReportUI(title,pParent);
	  
	  instance->dialog->popup(1);
	  instance->dialog->bring_to_top();
	  instance->invoked = 1;
	  
	  pop_cursor();
    }
}

void QC_ReportUI::Quit(void)
{
  Initialize(QC_ReportUI::Quit(void));
  
  if ( instance ) 
    {
      instance->dialog->popdown();
      instance = NULL;
    }
}

QC_ReportUI::QC_ReportUI(char const *title /*= 0*/,gtBase* pParent/*=NULL*/) : m_pAttachment(NULL)
{
  Initialize(QC_ReportUI::QC_ReportUI(char const *title));

  parent           = pParent;
  cur_report_type  = "";
  report_types     = 0;
  invoked          = 0;
  filebrowser      = 0;
  debug_access     = -1;

  buildInterface(title);
}

QC_ReportUI::~QC_ReportUI()
{
  Initialize(QC_ReportUI::~QC_ReportUI());

  if(m_pAttachment != NULL) delete m_pAttachment;
  delete [] report_types;
  dialog->popdown();
}

////////////////////////////////////////////////////////////////////////////////
//
// INITIALIZATION METHODS
//
////////////////////////////////////////////////////////////////////////////////

void QC_ReportUI::buildInterface(char const *title)
{
  Initialize(QC_ReportUI::buildInterface(char const *));

  // 
  // Create Dialog
  //

  dialog = gtDialogTemplate::create(NULL, "QC_ReportUI", title);
  
  //
  // Report Format Area
  //
  
  gtOptionMenu *report_type_menu = gtOptionMenu::create(dialog, "report_type_menu", TXT("Report Format:"), NULL);
  int num = getReportTypes(report_types);
  for ( int i = 0; i < num; i++ )
    report_type_menu->insert_entries(0, gtMenuStandard, "report_type_option", report_types[i].str(), this, reportType_CB, NULL);
  cur_report_type = report_types[0];
  
  verbosity = gtToggleButton::create(dialog, "verbosity", TXT("Verbose Report"), NULL, NULL);
  verbosity->set(1, 0);
  verbosity->height(report_type_menu->pheight());
  verbosity->attach(gtTop, 0, 5);
  verbosity->attach(gtRight, 0, 5);

  report_type_menu->attach(gtTop, 0, 5);
  report_type_menu->attach(gtLeft, 0, 5);
  report_type_menu->attach(gtRight, verbosity, 10);

  report_type_menu->manage();
  verbosity->manage();
  
  // 
  // Filename Area
  // 
  
  gtLabel        *filename_lbl = gtLabel::create(dialog, "filename_lbl", TXT("Report Filename:"));
  gtPushButton   *browse_btn   = gtPushButton::create(dialog, "browse_btn", TXT("Browse..."), browse_CB, this);
                  filename_txt = gtStringEditor::create(dialog, "filename_txt", NULL);

  filename_lbl->height(filename_txt->pheight());
  filename_lbl->attach(gtTop, report_type_menu, 10);
  filename_lbl->attach(gtLeft, 0, 5);

  browse_btn->height(filename_txt->pheight());
  browse_btn->width(75);
  browse_btn->attach(gtRight, 0, 5);
  browse_btn->attach(gtTop, report_type_menu, 10);

  filename_txt->width(275);
  filename_txt->attach(gtLeft, filename_lbl, 5);
  filename_txt->attach(gtRight, browse_btn, 5);  
  filename_txt->attach(gtTop, report_type_menu, 10);
  filename_txt->text(TXT("CaliperReport"));

  filename_lbl->manage();
  filename_txt->manage();
  browse_btn->manage();

  attachToIM = gtToggleButton::create(dialog, "imattach", TXT("Attach to IM issue..."), NULL, NULL);
  attachToIM->set(0, 0);
  attachToIM->height(verbosity->pheight());
  attachToIM->attach(gtTop, filename_lbl, 5);
  attachToIM->attach(gtLeft, 0, 5);
  attachToIM->manage();
  //
  // Bottom Buttons
  //

  dialog->add_button("run_report_btn", TXT("Run Report"), run_CB, this);
  dialog->add_button("cancel_btn", TXT("Cancel"), cancel_CB, this);
}

////////////////////////////////////////////////////////////////////////////////
//
// CALLBACKS
//
////////////////////////////////////////////////////////////////////////////////
void QC_ReportUI::ReadyToAttach(int nIsOK) {
  static const int nNumFiles = 8;
  static const char* nameSuffix[nNumFiles]={"",
					    ".chart0.gif",
					    ".chart1.gif",
					    ".chart2.gif",
					    ".chart3.gif",
					    ".chart4.gif",
					    ".chart5.gif",
					    ".chart6.gif"};
  genString filename = filename_txt->text();
  genString type     = cur_report_type;

  if(m_pAttachment!=NULL && nIsOK && ProcessReportName(&filename,type)) {
    Widget wParent = NULL;
    if(parent!=NULL) wParent = parent->rep()->widget();
    
    for(int i=0;i<nNumFiles;i++) {
      string currentFileName = filename.str();
      currentFileName += nameSuffix[i];
      
      if(!m_pAttachment->Attach(currentFileName,wParent))
	break;
    }
  }
  Quit();
}

void QC_ReportUI::reportType_CB(gtPushButton *menuoption, gtEventPtr, void *obj, gtReason)
{
  Initialize(QC_ReportUI::reportType_CB(gtPushButton *, gtEventPtr, void *, gtReason));

  QC_ReportUI *ui = (QC_ReportUI *) obj;
  if ( ui && ui->invoked && menuoption )
    {
      ui->cur_report_type = menuoption->title();
    }
}

void QC_ReportUI::browse_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QC_ReportUI::browse_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QC_ReportUI *ui = (QC_ReportUI *) obj;
  if ( ui )
    {
      ui->browseForFilename();
    }
}

void QC_ReportUI::browse_OK_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QC_ReportUI::browse_OK_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QC_ReportUI *ui = (QC_ReportUI *) obj;
  if ( ui )
    {
      ui->handleFilenameSelection(OK);
    }
}

void QC_ReportUI::browse_cancel_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QC_ReportUI::browse_cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QC_ReportUI *ui = (QC_ReportUI *) obj;
  if ( ui )
    {
      ui->handleFilenameSelection(CANCEL);
    }
}

void QC_ReportUI::run_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QC_ReportUI::remove_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  QC_ReportUI *ui = (QC_ReportUI *) obj;
  if ( ui )
    {
      if ( ui->runReport() == 0 )
	if(ui->attachToIM->set()) {
	  if(ui->m_pAttachment!=NULL) delete ui->m_pAttachment;
	  ui->m_pAttachment = new CIMAttachment();
	  ui->m_pAttachment->AddCallback(ui);
	
	  ui->m_pAttachment->AskForParameters(ui->dialog->rep()->widget());
	} else
	  ui->Quit();
    }
}

void QC_ReportUI::cancel_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QC_ReportUI::cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  QC_ReportUI *ui = (QC_ReportUI *) obj;
  if ( ui )
    {
      ui->Quit();
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// UI's API (higher level that Access, lower than Callbacks)
//
////////////////////////////////////////////////////////////////////////////////

int QC_ReportUI::browseForFilename(void)
{
  Initialize(QC_ReportUI::browseForFilename(void));

  int retval = -1;
  
  if ( filebrowser )
    {
      retval = 1;
    }
  else
    {
      filebrowser = gtFileSelector::create(dialog, "selector", browse_OK_CB, this, browse_cancel_CB, this);
      filebrowser->title("Select Report Filename");
      filebrowser->manage();
      retval = 0;
    }

  return retval;
}

int QC_ReportUI::handleFilenameSelection(selection s) 
{
  Initialize(QC_ReportUI::handleFilenameSelection(selection));
  
  int retval = -1;
  int again  = 0;

  if ( filebrowser )
    {
      switch ( s )
	{
	case OK:
	  {
	    char const *name = filebrowser->selected();
	    if ( name )
	      {
		if ( OSapi_strcmp((char *)name, "") == 0 )
		  {
		    msg("ERROR: You have selected an empty filename.\nPlease try again.") << eom;
		    again = 1;
		  }
		else
		  {
		    // get previous
		    char const *text = filename_txt->text();
		    if ( text ) gtFree((void *)text);
		    
		    // set current
		    filename_txt->text(name);
		  }
		retval = 0;
	      }
	    break;
	  }
	  
	case CANCEL:
	  {
	    retval = 1;
	    break;
	  }
	}

      delete filebrowser;
      filebrowser = 0;
    }

  if ( again )
    browseForFilename();
  
  return retval;
}

bool QC_ReportUI::ProcessReportName(genString* filename,genString& type) {
  if(filename->is_null() || (*filename) == "" ) {
      return false;
  } else {
      // tack on .html to filename if needed
      if( type == "HTML" ) {
	  genString extension = OSapi_rindex(*filename, '.');
	  if ( extension.is_null() || (OSapi_strcasecmp(extension, ".html") != 0 && OSapi_strcasecmp(extension, ".htm") != 0) ) {
	      if((*filename)[filename->length()-1] != '/' )
		(*filename) += ".html";
	  }
      }
  }
  return true;
}

int QC_ReportUI::runReport(void)
{
  Initialize(QC_ReportUI::runReport(void));
  
  int retval = -1;

  push_busy_cursor();

  genString filename = filename_txt->text();
  genString type     = cur_report_type;
  bool      verbose  = verbosity->set();

  if(!ProcessReportName(&filename,type))
      msg("ERROR: Please specify a report filename.") << eom;
  else
     retval = runReport(type, filename, verbose);
  
  pop_cursor();

  return retval;
}

////////////////////////////////////////////////////////////////////////////////
//
// Methods to use Access API to Quality Cockpit Report information
//
////////////////////////////////////////////////////////////////////////////////

int QC_ReportUI::getReportTypes(genString *&types)
{
  Initialize(QC_ReportUI::getReportTypes(genString *&));

  int retval = -1;
  
  genString result;
  if ( sendCmd("sev_get_report_formats", result) == TCL_OK )
    {
      TclList l = (char *) result;
      if ( l.Size() > 0 )
	{
	  types = new genString [l.Size()];
	  for ( int i = 0; i < l.Size(); i++ )
	    types[i] = l[i];
	}
      retval = l.Size();
    }

  return retval;
}

int QC_ReportUI::debuggingAccess(void)
{
  Initialize(QC_ReportUI::debuggingAccess(void));
  
  if ( debug_access == -1 )
    if ( OSapi_getenv("DISCOVER_DEBUG_QC_REPORT") )
      debug_access = 1;
    else
      debug_access = 0;
  
  return debug_access;
}

int QC_ReportUI::runReport(genString const &type, genString const &filename, bool verbose)
{
  Initialize(QC_ReportUI::runReport(genString const &, genString const &, bool));

  int retval = -1;

  if ( type.not_null() && filename.not_null() )
    {
      genString command, result;
      command.printf("sev_create_report \"%s\" \"%s\" %d", type.str(), filename.str(), verbose ? 1 : 0);
      if ( sendCmd(command, result) == TCL_OK )
	retval = 0;
    }

  return retval;
}

int QC_ReportUI::sendCmd(genString command, genString &result)
{
  Initialize(QC_ReportUI::sendCmd(genString, genString &));
  
  int retval = TCL_ERROR;
  
  Interpreter *i = GetActiveInterpreter();
  if ( i )
    {
      if ( debuggingAccess() )
	msg("QC_ReportUI: Sending command: $1") << command.str() << eom;

      cli_eval_string(command);  
      retval = i->GetResult(result);  
      
      if ( debuggingAccess() )
	msg("QC_ReportUI: Received result: $1") << result.str() << eom;

      if (retval == TCL_ERROR) 
	notifyTclError();
    }
  
  return retval;
}

void QC_ReportUI::notifyTclError(void)
{
  Initialize(QC_ReportUI::notifyTclError(void));

  Interpreter *i = GetActiveInterpreter();
  if ( i )
    {
      char *err_info = Tcl_GetVar(i->interp, "errorInfo", TCL_GLOBAL_ONLY);
      msg("ERROR: The following error occured:\n\n$1") << err_info << eom;
    }
}




