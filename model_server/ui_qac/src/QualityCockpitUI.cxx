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
// Quality Cockpit UI
//
////////////////////////////////////////////////////////////////////////////////
#include <msg.h>

//++UI
#include <QualityCockpitUI.h>
#include <QC_ReportUI.h>
#include <sessionListUI.h>
#include <errorOutputUI.h>
#include <browserShell.h>
//--UI

//++GENERAL
#include <general.h>
#include <genArr.h>
#include <genString.h>
#include <messages.h>
#include <systemMessages.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--GENERAL

//++HELP
#include <help.h>
//--HELP

//++GT
#include <gt.h>
#include <gtCascadeB.h>
#include <gtDisplay.h>
#include <gtForm.h>
#include <gtList.h>
#include <gtMainWindow.h>
#include <gtMenuBar.h>
#include <gtOptionMenu.h>
#include <gtPushButton.h>
#include <gtRadioBox.h>
#include <gtSepar.h>
#include <gtStringEd.h>
#include <gtTopShell.h>
#include <gtQuestion.h>
#include <NewPrompt.h>
#include <top_widgets.h>
//--GT

//++LDR (for icon definitions)
#include <ldrList.h>
//--LDR

//++ACCESS
#include <TclList.h>
#include <Interpreter.h>
//--ACCESS

//++LICENSING
#include <xxinterface.h>
//--LICENSING

////////////////////////////////////////////////////////////////////////////////
//
// Static initialization
//
////////////////////////////////////////////////////////////////////////////////

QualityCockpitUI   *QualityCockpitUI::instance        = NULL;
int const           QualityCockpitUI::MIN_HEIGHT      = 525;
int const           QualityCockpitUI::MIN_WIDTH       = 715;
char const * const  QualityCockpitUI::ROOT_FOLDER     = "/";
char const          QualityCockpitUI::FOLDER_SEP      = '/';
char const          QualityCockpitUI::ENABLED_FOLDER  = PIX_FOLDER;
char const          QualityCockpitUI::DISABLED_FOLDER = PIX_FOLDER + 1;
char const          QualityCockpitUI::ENABLED_QUERY   = PIX_DOC_STD + 1;
char const          QualityCockpitUI::DISABLED_QUERY  = PIX_DOC_STD;

////////////////////////////////////////////////////////////////////////////////
//
// Methods
//
////////////////////////////////////////////////////////////////////////////////

void QualityCockpitUI::Invoke(char const *title)
{
  Initialize(QualityCockpitUI::Invoke(char const *title));
  
  if (instance) 
    {
      if(!instance->invoked)
	{
	  instance->toplev->popup();
	  instance->invoked = 1;
	}
      instance->toplev->bring_to_top();
    } 
  else 
    {
      if ( _lo(LIC_QAC) == LIC_FAIL )
	{
	  _lm(LIC_QAC);
	}
      else
	{
	  push_busy_cursor();
	  
	  if (!title) title = TXT("Caliper");
	  instance = new QualityCockpitUI(title);
	  
	  // center before popping up new window
	  int root_height = gtDisplay::height();
	  int root_width  = gtDisplay::width();
	  int win_height  = MIN_HEIGHT;
	  int win_width   = MIN_WIDTH;
	  int x_pos       = (root_width - win_width) / 2;
	  int y_pos       = (root_height - win_height) / 2;
	  if ( x_pos < 0 ) x_pos = 0;
	  if ( y_pos < 0 ) y_pos = 0;
	  
	  instance->toplev->pos(x_pos, y_pos);
	  instance->toplev->popup();
	  instance->toplev->bring_to_top();
	  instance->invoked = 1;
	  
	  pop_cursor();
	}
    }
}



int QualityCockpitUI::Quit(void)
{
  Initialize(QualityCockpitUI::Quit(void));
  
  int quit = 1;

  if ( instance ) 
    {
      if ( instance->session_modified )
	{
	  gtQuestion dialog(TXT("Session Modified"),
			    TXT("The current session is modified\n"
				"but not saved. Save the current\n"
				"session before closing?"),
			    TXT("Yes"),
			    TXT("No"),
			    TXT("Cancel"));
	  
	  switch ( dialog.ask() )
	    {
	    case -1: // cancel
	      quit = 0;
	      break;
	      
	    case 1:  // yes
	      if ( instance->saveSession() != 0 )
		quit = 0;
	      break;

	    case 2:  // no
	      break;
	    }
	}
	
      if ( quit )
	{
	  instance->toplev->popdown();
	  instance = NULL;
	  _li(LIC_QAC);
	}
    }
  
  return quit;
}

QualityCockpitUI::QualityCockpitUI(char const *title)
{
  Initialize(QualityCockpitUI::QualityCockpitUI(char const *title));

  base_title       = 0;
  full_title       = 0;
  cur_input_type   = "";
  input_types      = 0;
  invoked          = 0;
  session_modified = false;
  debug_access     = -1;

  report_license_exists = (_lf(LIC_QFS_REPORT) == LIC_SUCCESS) ? true : false;
  run_license_exists    = (_lf(LIC_QFS_RUN) == LIC_SUCCESS) ? true : false;

  initAPI();
  buildInterface(title);
  loadSession("");
  retitle();
  initData();
}

QualityCockpitUI::~QualityCockpitUI()
{
  Initialize(QualityCockpitUI::~QualityCockpitUI());

  if ( base_title ) OSapi_free((void *)base_title);
  if ( full_title ) OSapi_free((void *)full_title);
  delete [] input_types;
  toplev->popdown();
  delete toplev;
}

////////////////////////////////////////////////////////////////////////////////
//
// INITIALIZATION METHODS
//
////////////////////////////////////////////////////////////////////////////////

void QualityCockpitUI::initData(void)
{
  Initialize(QualityCockpitUI::initData(void));

  buildCategoryList(ROOT_FOLDER);
  updateFields();
}  

void QualityCockpitUI::buildInterface(char const *title)
{
  Initialize(QualityCockpitUI::buildInterface(char const *));

  // 
  // Create Top Level Shell
  //

  gtShell *gtsh = NULL;
  genArrOf(browserShellPtr) *instance_array = browserShell::get_browsers_list();
  if (instance_array && instance_array->size()) 
    {
      browserShell *bsh = *((*instance_array)[0]);
      if (bsh)
	gtsh = bsh->top_level();
    }
  toplev = gtTopLevelShell::create(gtsh, title);
  toplev->min_height(MIN_HEIGHT);
  toplev->min_width(MIN_WIDTH);
  toplev->override_WM_destroy(QualityCockpitUI::destroy_CB);

  //
  // Add session name to title
  //

  // save the title
  base_title = OSapi_strdup(title);
  toplev->title(base_title);
  
  //
  // Create Main Window
  // 

  main_window = gtMainWindow::create(toplev, "main_window", 0);
  main_window->manage();
  
  // Menu bar

  gtMenuBar *menu_bar = main_window->menu_bar();

  // "QA Session" menu:

  gtCascadeButton *cascade_btn = gtCascadeButton::create(menu_bar, "qa_session_menu", TXT("Session"), NULL, NULL);
  cascade_btn->pulldown_menu("qa_session_menu",
			     gtMenuStandard,  "new",     TXT("New"),        this, QualityCockpitUI::newSession_CB,
			     gtMenuStandard,  "open",    TXT("Open..."),    this, QualityCockpitUI::openSession_CB,
			     gtMenuStandard,  "close",   TXT("Close"),      this, QualityCockpitUI::closeSession_CB,
			     gtMenuSeparator, "sep",
			     gtMenuStandard,  "save",    TXT("Save"),       this, QualityCockpitUI::save_CB,
			     gtMenuStandard,  "save_as", TXT("Save As..."), this, QualityCockpitUI::saveAs_CB,
			     gtMenuStandard,  "delete",  TXT("Delete..."),  this, QualityCockpitUI::remove_CB,
			     gtMenuSeparator, "sep",
			     gtMenuStandard,  "exit",    TXT("Exit"),       this, QualityCockpitUI::exit_CB,
			     NULL);
  cascade_btn->manage();

  // "Help" menu:

  cascade_btn = gtCascadeButton::create(menu_bar, "help_menu", TXT("Help"), NULL, NULL);
  menu_bar->set_help(cascade_btn);
  cascade_btn->pulldown_menu("help_menu", gtMenuStandard, "help_qa_cockpit", TXT("Contents"), this, gtBase::help_button_callback, NULL);
  REG_HELP(cascade_btn->button("help_qa_cockpit"), "Browser.QACockpit.Help");
  cascade_btn->manage();

  // Create the form:

  gtForm *form = gtForm::create(main_window, "form");
  form->attach_tblr();
  form->manage();

  //
  // CATEGORY AREA
  //

  gtLabel        *category_lbl = gtLabel::create(form, "category_lbl", TXT("Category:"));
  up           = gtPushButton::create(form, "up", TXT("Up"), QualityCockpitUI::categoryUp_CB, this);
  category_txt = gtStringEditor::create(form, "category_txt", NULL);

  category_lbl->height(category_txt->pheight());
  category_lbl->attach(gtTop, 0, 5);
  category_lbl->attach(gtLeft, 0, 5);

  up->height(category_txt->pheight());
  up->width(75);
  up->attach(gtRight, 0, 5);
  up->attach(gtTop, 0, 5);

  category_txt->attach(gtLeft, category_lbl, 5);
  category_txt->attach(gtRight, up, 5);  
  category_txt->attach(gtTop, 0, 5);
  category_txt->read_only();
  category_txt->show_cursor(0);

  category_lbl->manage();
  category_txt->manage();
  up->manage();
  
  // 
  // BOTTOM BUTTON ROW
  // 

  run_all_btn          = gtPushButton::create(form, "run_all_btn", TXT("Run All Queries"), QualityCockpitUI::runAll_CB, this);
  run_selected_btn     = gtPushButton::create(form, "run_selected_btn", TXT("Run Selected Queries"), QualityCockpitUI::runSelected_CB, this);
  reset_btn            = gtPushButton::create(form, "reset_btn", TXT("Reset Session"), QualityCockpitUI::reset_CB, this);
  report_btn           = gtPushButton::create(form, "report_btn", TXT("Generate Report"), QualityCockpitUI::report_CB, this);
  close_btn            = gtPushButton::create(form, "exit_btn", TXT("Exit"), QualityCockpitUI::exit_CB, this);
  gtSeparator *btn_sep = gtSeparator::create(form, "btn_sep", gtHORZ);


  run_all_btn->width(155);
  run_all_btn->attach_pos(gtDirLeft, 0);
  run_all_btn->attach(gtBottom, 0, 5);

  run_selected_btn->width(155);
  run_selected_btn->attach_pos(gtDirLeft, 20);
  run_selected_btn->attach(gtBottom, 0, 5);

  reset_btn->width(155);
  reset_btn->attach_pos(gtDirLeft, 40);
  reset_btn->attach(gtBottom, 0, 5);

  report_btn->width(155);
  report_btn->attach_pos(gtDirLeft, 60);
  report_btn->attach(gtBottom, 0, 5);

  close_btn->width(155);
  close_btn->attach_pos(gtDirLeft, 80);
  close_btn->attach(gtBottom, 0, 5);

  btn_sep->attach(gtLeft);
  btn_sep->attach(gtRight);
  btn_sep->attach(gtBottom, run_selected_btn, 5);

  run_all_btn->manage();
  run_selected_btn->manage();
  report_btn->manage();
  reset_btn->manage();
  close_btn->manage();
  btn_sep->manage();

  if ( ! run_license_exists )
    {
      run_all_btn->set_sensitive(0);
      run_selected_btn->set_sensitive(0);
    }
  if ( ! report_license_exists )
    {
      report_btn->set_sensitive(0);
    }

  //
  // DATA FIELDS
  //

  // Radio buttons
  
  status_select = gtRadioBox::create(form, "status_select", NULL,
				     TXT("Enabled"),  "enabled",  QualityCockpitUI::enableQuery_CB,  this,
				     TXT("Disabled"), "disabled", QualityCockpitUI::disableQuery_CB, this,
				     NULL);
  status_select->num_columns(1);
  status_select->attach(gtLeft, 0, 5);
  status_select->attach(gtBottom, btn_sep, 15);
  status_select->item_set (0, 1, 0);
  status_select->manage();
  
  // Seperate the list from the fields

  gtSeparator *list_sep = gtSeparator::create(form, "list_sep", gtHORZ);
  list_sep->attach(gtLeft);
  list_sep->attach(gtRight);
  list_sep->attach(gtBottom, status_select, 15);
  list_sep->manage();

  //
  // OVERALL SCORE FIELD
  //

  gtLabel *overall_lbl = gtLabel::create(form, "overall_lbl", TXT("Overall Score:"));
  overall_txt = gtStringEditor::create(form, "overall_txt", NULL);

  overall_lbl->height(overall_txt->pheight());
  overall_lbl->attach(gtLeft, status_select, 10);
  overall_lbl->attach(gtTop, list_sep, 7);

  overall_txt->width(66);
  overall_txt->attach(gtTop, list_sep, 5);
  overall_txt->attach(gtLeft, overall_lbl, 5);
  overall_txt->read_only();
  overall_txt->show_cursor(0);

  overall_lbl->manage();
  overall_txt->manage();

  //
  // QUERY SCORE FIELD
  // 

  gtLabel *score_lbl = gtLabel::create(form, "score_lbl", TXT("Query Score:"));
  score_txt = gtStringEditor::create(form, "score_txt", NULL);

  score_lbl->height(score_txt->pheight());
  score_lbl->attach(gtLeft, overall_txt, 10);
  score_lbl->attach(gtTop, list_sep, 7);

  score_txt->width(66);
  score_txt->attach(gtLeft, score_lbl, 5);
  score_txt->attach(gtTop, list_sep, 5);
  score_txt->read_only();
  score_txt->show_cursor(0);

  score_lbl->manage();
  score_txt->manage();

  //
  // WEIGHT SCORE FIELD
  //

  gtLabel *weight_lbl = gtLabel::create(form, "weight_lbl", TXT("Weight:"));
  weight_txt = gtStringEditor::create(form, "weight_txt", NULL);

  weight_lbl->height(weight_txt->pheight());
  weight_lbl->attach(gtLeft, score_txt, 10);
  weight_lbl->attach(gtTop, list_sep, 7);

  weight_txt->width(66);
  weight_txt->attach(gtLeft, weight_lbl, 5);
  weight_txt->attach(gtTop, list_sep, 5);
  weight_txt->activate_callback(weightChanged_CB, this);

  weight_lbl->manage();
  weight_txt->manage();

  //
  // HITS AREA
  //

  gtLabel      *hits_lbl     = gtLabel::create(form, "hits_lbl", TXT("Instances:"));
  hits_txt     = gtStringEditor::create(form, "hits_txt", NULL);

  hits_lbl->height(hits_txt->pheight());
  hits_lbl->attach(gtLeft, weight_txt, 10);
  hits_lbl->attach(gtTop, list_sep, 7);

  hits_txt->width(66);
  hits_txt->attach(gtLeft, hits_lbl, 5);
  hits_txt->attach(gtTop, list_sep, 5);
  hits_txt->read_only();
  hits_txt->show_cursor(0);

  hits_lbl->manage();
  hits_txt->manage();

  //
  // INPUT MENU (we get the list of inputs from SEV (API must be initialized by this point))
  //

  input_menu = gtOptionMenu::create(form, "input_menu", "Input:", NULL);

  int num = getInputTypes(input_types);
  for ( int i = 0; i < num; i++ )
    input_menu->insert_entries(0, gtMenuStandard, "option", input_types[i].str(), this, QualityCockpitUI::typeInput_CB, NULL);
  cur_input_type = input_types[0];

  input_menu->attach(gtBottom, btn_sep, 5);
  input_menu->attach(gtLeft, status_select, 5);
  input_menu->manage();
  
  //
  // BROWSING HITS
  //

  browse_hits  = gtPushButton::create(form, "browse_hits", TXT("Browse Instances"), QualityCockpitUI::browseHits_CB, this);
  browse_hits->attach_opp_left(hits_lbl);
  browse_hits->attach_opp_right(hits_txt);
  browse_hits->attach(gtBottom, btn_sep, 5);
  browse_hits->manage();

  //
  // QUERY LIST
  //

  query_list = gtList::create(form, "query_list", TXT(""), gtBrowse, NULL, 0);
  query_list->height(350);
  query_list->width(700);
  query_list->attach(gtTop, category_txt, 10);
  query_list->attach(gtLeft, 0, 5);
  query_list->attach(gtRight, 0, 5);
  query_list->attach(gtBottom, list_sep, 10);
  query_list->select_callback(QualityCockpitUI::query_select_CB, this);
  query_list->action_callback(QualityCockpitUI::query_dbl_click_CB, this);
  query_list->manage();
}

////////////////////////////////////////////////////////////////////////////////
//
// CALLBACKS
//
////////////////////////////////////////////////////////////////////////////////

int QualityCockpitUI::destroy_CB(void *)
{
  Initialize(QualityCockpitUI::destroy_CB(void *));
  
  return Quit();
}

void QualityCockpitUI::exit_CB(gtPushButton*, gtEventPtr, void *, gtReason)
{
  Initialize(QualityCockpitUI::exit_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  Quit();
}


void QualityCockpitUI::newSession_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::newSession_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui )
    {
      push_busy_cursor();
      ui->newSession();
      pop_cursor();
    }
}

void QualityCockpitUI::openSession_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::openSession_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui )
    {
      push_busy_cursor();
      ui->loadSession();
      pop_cursor();
    }
}

void QualityCockpitUI::closeSession_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::closeSession_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui )
    {
      push_busy_cursor();
      ui->closeSession();
      pop_cursor();
    }
}

void QualityCockpitUI::save_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::save_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui )
    {
      push_busy_cursor();
      ui->saveSession();
      pop_cursor();
    }
}

void QualityCockpitUI::saveAs_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::saveAs_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui )
    {
      push_busy_cursor();
      ui->saveAsSession();
      pop_cursor();
    }
}

void QualityCockpitUI::remove_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::remove_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui )
    {
      push_busy_cursor();
      ui->deleteSession();
      pop_cursor();
    }
}

void QualityCockpitUI::categoryUp_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::categoryUp_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui )
    {
      push_busy_cursor();
      ui->upCategory();
      pop_cursor();
    }
}

void QualityCockpitUI::runAll_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::runAll_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui )
    {
      push_busy_cursor();
      ui->runQueries(ALL);
      pop_cursor();
    }
}

void QualityCockpitUI::runSelected_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::runSelected_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      push_busy_cursor();
      ui->runQueries(SELECTED);
      pop_cursor();
    }
}

void QualityCockpitUI::report_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::report_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      push_busy_cursor();
      ui->runReport();
      pop_cursor();
    }
}

void QualityCockpitUI::reset_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::reset_CB(gtPushButton*, gtEventPtr, void*, gtReason));
  
  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      push_busy_cursor();
      ui->resetWeights();
      pop_cursor();
    }
}

void QualityCockpitUI::browseHits_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::browseHits_CB(gtPushButton*, gtEventPtr, void*, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      push_busy_cursor();
      ui->browseHits();
      pop_cursor();
    }
}

void QualityCockpitUI::query_select_CB(gtList*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::query_select_CB(gtList*, gtEventPtr, void *, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      push_busy_cursor();
      ui->updateFields();
      pop_cursor();
    }
}

void QualityCockpitUI::query_dbl_click_CB(gtList*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::query_dbl_click_CB(gtList*, gtEventPtr, void *, gtReason));
  
  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      push_busy_cursor();
      // get the name of the item selected
      genString query = ui->getSelectedQuery();
      if ( query.not_null() )
	{	  
	  switch ( ui->getType(query) )
	    {
	    case FOLDER:
	      if ( ui->enabled(query) )
		ui->buildCategoryList(query);
	      else
		msg("ERROR: A disabled folder cannot be opened.") << eom;
	      break;
	      
	    case THRESH_QUERY:
	      if ( ui->enabled(query) )
		ui->getNewThreshold(query);
	      else
		msg("ERROR: A disabled query\'s threshhold\nvalue cannot be modified.") << eom;
	      break;
	      
	    default:
	      break;
	    }
	}

      ui->updateFields();
      pop_cursor();
    }
}

void QualityCockpitUI::disableQuery_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::disableQuery_CB(gtPushButton*, gtEventPtr, void *, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      push_busy_cursor();
      genString query = ui->getSelectedQuery();
      if ( query.not_null() )
	{
	  if ( ui->enabled(query) )
	    {
	      ui->disable(query);
	      ui->updateFields();
	      ui->updateIcon();
	    }
	}
      pop_cursor();
    }
}

void QualityCockpitUI::enableQuery_CB(gtPushButton*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::enableQuery_CB(gtPushButton*, gtEventPtr, void *, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      push_busy_cursor();

      genString query = ui->getSelectedQuery();
      if ( query.not_null() )
	{
	  if ( ! ui->enabled(query) )
	    {
	      ui->enable(query);
	      ui->updateFields();
	      ui->updateIcon();
	    }
	}
      pop_cursor();
    }
}

void QualityCockpitUI::typeInput_CB(gtPushButton *button, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::typeInput_CB(gtPushButton *, gtEventPtr, void *, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked && button )
    {
      ui->cur_input_type = button->title();
    }
}

void QualityCockpitUI::weightChanged_CB(gtStringEditor*, gtEventPtr, void *obj, gtReason)
{
  Initialize(QualityCockpitUI::weightChanged_CB(gtStringEditor*, gtEventPtr, void*, gtReason));

  QualityCockpitUI *ui = (QualityCockpitUI *) obj;
  if ( ui && ui->invoked )
    {
      ui->weightChanged();
    }
}  

////////////////////////////////////////////////////////////////////////////////
//
// DATA DISPLAY ROUTINES
//
////////////////////////////////////////////////////////////////////////////////

int QualityCockpitUI::buildCategoryList(genString const &folder)
{
  Initialize(QualityCockpitUI::buildCategoryList(genString));

  int retval = -1;

  if ( query_list )
    {
      genString *children;
      if ( (retval = getChildren(folder, children)) > 0 )
	{
	  // remove previous contents
	  query_list->delete_all_items();

	  // allocate space for new strings
	  char const **items   = new char const * [retval];
	  int  num_items = 0;

	  for ( int i = 0; i < retval; i++ )
	    {
	      // '.' as the first character of a query/folder means it is hidden
	      if ( children[i][0] != '.' )  
		{
		  // add icon for category
		  genString item;
		  addIcon(folder, children[i], item);

		  // save to list
		  items[num_items++] = OSapi_strdup(item);
		}
	    }
	  
	  // add all items to list box
	  query_list->add_items(items, num_items, 0);
	  
	  // select the first item
	  if ( num_items > 0 )
	    query_list->select_pos(1, 0);

	  // free unneeded memory
	  delete [] children;
	  for ( int j = 0; j < num_items; j++ )
	    OSapi_free((void *)(items[j]));
	  delete [] items;
	}
      
      // at any rate, put the folder name in the category list
      category_txt->text(folder);

      // adjust the "up" button as appropriate
      if ( folder == ROOT_FOLDER )
	up->set_sensitive(0);
      else
	up->set_sensitive(1);
    }

  return retval;
}

int QualityCockpitUI::updateFields(void)
{
  Initialize(QualityCockpitUI::updateFields(void));

  int retval = -1;

  if ( query_list )
    {
      genString query = getSelectedQuery();
      if ( query.not_null() && query != "" )
	{	  
	  score_txt->text(getQueryScore(query));
	  weight_txt->text(getQueryWeight(query));
	  if ( getType(query) != FOLDER )
	    hits_txt->text(getQueryHits(query));
	  else
	    hits_txt->text(TXT("N/A"));
	  
	  if ( enabled(query) )
	    {
	      status_select->item_set(0, 1, 0);
	      status_select->item_set(1, 0, 0);
	      score_txt->set_sensitive(1);
	      weight_txt->set_sensitive(1);
	      hits_txt->set_sensitive(1);
              if ( run_license_exists )
                {
	          run_selected_btn->set_sensitive(1);
                }
	      if ( getType(query) != FOLDER )
                {
                  genString hits = getQueryHits(query);
		  browse_hits->set_sensitive(atoi(hits.str()) > 0 ? 1 : 0);
                }
	      else
		browse_hits->set_sensitive(0);
	    }
	  else
	    {
	      status_select->item_set(0, 0, 0);
	      status_select->item_set(1, 1, 0);
	      score_txt->set_sensitive(0);
	      weight_txt->set_sensitive(0);
	      hits_txt->set_sensitive(0);
	      run_selected_btn->set_sensitive(0);
	      browse_hits->set_sensitive(0);
	    }
	  status_select->set_sensitive(1);

	  retval = 1;
	}
      else
	{
	  // none selected: grey out appropriate fields

	  status_select->item_set(0, 0, 0);
	  status_select->item_set(1, 0, 0);
	  status_select->set_sensitive(0);
	  
	  score_txt->text("");
	  score_txt->set_sensitive(0);

	  weight_txt->text("");
	  weight_txt->set_sensitive(0);
	  
	  hits_txt->text("");
	  hits_txt->set_sensitive(0);

	  run_selected_btn->set_sensitive(0);
	  browse_hits->set_sensitive(0);

	  retval = 0;
	}
      
      // either way populate the Overall Score
      overall_txt->text(getOverallScore());
    }

  return retval;
}

int QualityCockpitUI::updateIcon(void)
{
  Initialize(QualityCockpitUI::updateIcon(void));

  int retval = -1;

  // get the current selection
  int *positions;
  int num_positions;
  query_list->get_selected_pos(&positions, &num_positions);
  
  if ( num_positions > 0 )
    {
      // adjust for the difference in Motif/gt posisitioning
      int position = 0;
      if ( *positions == 0 )
	position = query_list->num_items() - 1;
      else
	position = *positions - 1;
      
      // get the item
      genString selection = query_list->item_at_pos(position);
      genString newvalue  = "";

      // switch the icon
      if ( isicon(selection[0]) )
	{
	  retval = 0;

	  if ( selection[0] == ENABLED_FOLDER )
	    newvalue.printf("%c%s", DISABLED_FOLDER, selection.str()+1);
	  else if ( selection[0] == DISABLED_FOLDER )
	    newvalue.printf("%c%s", ENABLED_FOLDER, selection.str()+1);
	  else if ( selection[0] == ENABLED_QUERY )
	    newvalue.printf("%c%s", DISABLED_QUERY, selection.str()+1);
	  else if ( selection[0] == DISABLED_QUERY )
	    newvalue.printf("%c%s", ENABLED_QUERY, selection.str()+1);
	  else
	    retval = -1;

	  if ( retval == 0 )
	    {
	      char const *olditem = selection.str();
	      char const *newitem = newvalue.str();
	      query_list->replace_items(&olditem, &newitem, 1);
	      query_list->select_item(newitem, 1);
	    }
	}
    }
  
  return retval;
}

int QualityCockpitUI::addIcon(genString const &folder, genString const &name, genString &item)
{
  Initialize(QualityCockpitUI::addIcon(genString const &, genString const &, genString &));

  int retval = -1;

  if ( name.not_null() )
    {
      // get full name of item
      genString fullname = folder;
      if ( folder == ROOT_FOLDER )
	fullname += name;
      else
	{
	  fullname += FOLDER_SEP;
	  fullname += name;
	}

      switch ( getType(fullname) )
	{
	case FOLDER:
	  if ( enabled(fullname) )
	    item.printf("%c%s", ENABLED_FOLDER, name.str());
	  else
	    item.printf("%c%s", DISABLED_FOLDER, name.str());
	  break;

	default:
	  if ( enabled(fullname) )
	    item.printf("%c%s", ENABLED_QUERY, name.str());
	  else
	    item.printf("%c%s", DISABLED_QUERY, name.str());
	  break;
	}
    }
  
  return retval;
}

void QualityCockpitUI::retitle(void)
{
  Initialize(QualityCockpitUI::retitle(void));

  genString title;
  genString session = getSessionName();

  if ( session.not_null() )
    {
      if ( session == "" )
	session = "untitled";

      if ( base_title )
	title.printf("%s: Session %s", base_title, session.str());
      else
	title.printf("Session %s", session.str());
    }
      
  full_title = OSapi_strdup(title.str());
  toplev->title(full_title);
}

genString QualityCockpitUI::selectSession(bool allow_edit)
{
  Initialize(QualityCockpitUI::selectSession(bool));

  genString       retval;
  sessionListUI   list(allow_edit);
  genString      *sessions = 0;
  int             num = 0;
  
  if ( (num = getAllSessions(sessions)) >= 0 )
    list.getSession(sessions, num, retval);
  
  delete [] sessions;
  
  return retval;
}

int QualityCockpitUI::displayErrors(genString const &errors)
{
  Initialize(QualityCockpitUI::displayErrors(void));

  errorOutputUI dialog(errors);
  dialog.show();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// UI's API (higher level that Access, lower than Callbacks)
//
////////////////////////////////////////////////////////////////////////////////

int QualityCockpitUI::runQueries(run_scope scope)
{
  Initialize(QualityCockpitUI::runSelected(void));

  int retval = -1;

  genString query;
  switch ( scope ) {
  case ALL:
    query = ROOT_FOLDER;
    break;

  case SELECTED:
    query = getSelectedQuery();
    break;
  }

  if ( query.not_null() && query != "" )
    if ( enabled(query) )
      if ( execQueryTree(query) == 0 ) {
	updateFields();
	retval = 0;
      }
  return retval;
}

int QualityCockpitUI::runReport(void) 
{
  Initialize(QualityCockpitUI::runReport(void));
  
  int retval = 0;
  
  genString session = getSessionName();
  if ( (session.is_null() || session == "") && ! session_modified )
    {
      msg("ERROR: A report cannot be run on an\nunmodified untitled session.") << eom;
    }
  else
    {
      QC_ReportUI::Invoke(NULL,toplev);
    }

  return retval;
}


int QualityCockpitUI::browseHits(void)
{
  Initialize(QualityCockpitUI::browseHits(void));
  
  int retval = 0;

  genString query = getSelectedQuery();
  if ( query.not_null() )
    retval = browseHits(query);

  return retval;
}

int QualityCockpitUI::resetWeights(void)
{
  Initialize(QualityCockpitUI::resetWeights(void));

  int retval = -1;

  if ( msg_question(TXT("This will reset all weights\n"
			    "and scores. Are you sure?")) == 1 ) {
    if ( resetTree() == 0 ) {
      genString current_folder = category_txt->text();
      buildCategoryList(current_folder);
      updateFields();
      retval = 0;
    }
  } else
    retval = 1;
      
  return retval;
}

int QualityCockpitUI::newSession(void)
{
  Initialize(QualityCockpitUI::newSession(void));

  int retval = -1;
  int cancel = 0;

  // save the current session if necessary
  if ( session_modified )
    {
      gtQuestion dialog(TXT("Session Modified"),
			TXT("The current session is modified\n"
			    "but not saved. Save the current\n"
			    "session before opening new one?"),
			TXT("Yes"),
			TXT("No"),
			TXT("Cancel"));
      
      switch ( dialog.ask() )
	{
	case -1: // cancel
	  cancel = 1;
	  break;
	  
	case 1:  // yes
	  if ( saveSession() != 0 )
	    cancel = 1;
	  break;
	  
	case 2:  // no
	  break;
	}
    }

  
  if ( ! cancel )
    {
      // load an empty session
      if ( loadSession("") == 0 )
	{
	  retitle();
	  buildCategoryList(ROOT_FOLDER);
	  updateFields();
	  retval = 0;
	}
    }

  return retval;
}
  
int QualityCockpitUI::closeSession(void)
{
  Initialize(QualityCockpitUI::closeSession(void));

  int retval = -1;
  int cancel = 0;

  // save the current session if necessary
  if ( session_modified )
    {
      gtQuestion dialog(TXT("Session Modified"),
			TXT("The current session is modified\n"
			    "but not saved. Save the current\n"
			    "session before closing it?"),
			TXT("Yes"),
			TXT("No"),
			TXT("Cancel"));
      
      switch ( dialog.ask() )
	{
	case -1: // cancel
	  cancel = 1;
	  break;
	  
	case 1:  // yes
	  if ( saveSession() != 0 )
	    cancel = 1;
	  break;
	  
	case 2:  // no
	  break;
	}
    }

  
  if ( ! cancel )
    {
      // load an empty session
      if ( loadSession("") == 0 )
	{
	  retitle();
	  buildCategoryList(ROOT_FOLDER);
	  updateFields();
	  retval = 0;
	}
    }
  
  return retval;
}  

int QualityCockpitUI::loadSession(void)
{
  Initialize(QualityCockpitUI::loadSession(void));

  int retval = -1;
  int cancel = 0;

  // save the current session if necessary
  if ( session_modified )
    {
      gtQuestion dialog(TXT("Session Modified"),
			TXT("The current session is modified\n"
			    "but not saved. Save the current\n"
			    "session before closing it?"),
			TXT("Yes"),
			TXT("No"),
			TXT("Cancel"));
      
      switch ( dialog.ask() )
	{
	case -1: // cancel
	  cancel = 1;
	  break;
	  
	case 1:  // yes
	  if ( saveSession() != 0 )
	    cancel = 1;
	  break;
	  
	case 2:  // no
	  break;
	}
    }

  
  if ( ! cancel )
    {
      // load a selected session
      genString session = selectSession(false);
      if ( session.not_null() && session != "" )
	if ( loadSession(session) == 0 )
	  {
	    retitle();
	    buildCategoryList(ROOT_FOLDER);
	    updateFields();
	    retval = 0;
	  }
    }

  return retval;
}

int QualityCockpitUI::saveSession(void)
{
  Initialize(QualityCockpitUI::saveSession(void));

  int retval = -1;

  genString session = getSessionName();
  if ( session.is_null() || session == "" )
    {
      retval = saveAsSession();
    }
  else
    {
      if ( saveSession(session) == 0 )
	{
	  retval = 0;
	}
    }

  return retval;
}

int QualityCockpitUI::saveAsSession(void)
{
  Initialize(QualityCockpitUI::saveAsSession(void));

 int retval = -1;

  genString session = selectSession(true);
  if ( session.not_null() && session != "" )
    {
      if ( saveSession(session) == 0 )
	{
	  retitle();
	  retval = 0;
	}
    }

  return retval;
}

int QualityCockpitUI::deleteSession(void)
{
  Initialize(QualityCockpitUI::deleteSession(void));

  int retval = -1;

  genString session = selectSession(false);
  if ( session.not_null() && session != "" )
    {
      genString current_session = getSessionName();
      if ( session == current_session )
	{
	  if ( msg_question(TXT("The current session has been\n"
				"selected for deletion.\n\n"
				"Select OK to confirm.")) == 1 )
	    {
	      if ( deleteSession(session) == 0 )
		{
		  loadSession("");
		  retitle();
		  buildCategoryList(ROOT_FOLDER);
		  updateFields();
		  retval = 0;
		}
	    }
	}
      else
	{
	  if ( deleteSession(session) == 0 )
	    retval = 0;
	}
    }

  return retval;
}

int QualityCockpitUI::upCategory(void)
{
  Initialize(QualityCockpitUI::upCategory(void));

  int retval = -1;

  genString current_folder = category_txt->text();
  if ( current_folder != ROOT_FOLDER )
    {
      genString parent = getParent(current_folder);
      buildCategoryList(parent);
      updateFields();
      retval = 0;
    }

  return retval;
}

int QualityCockpitUI::weightChanged(void)
{
  Initialize(QualityCockpitUI::weightChanged(void));

  int retval = -1;

  // This is called as the weight text box loses focus, or when activted
  // so the first time the user attempts to change a weight, we check
  // to see if we can get a license, and if not, then we don't allow it
  // and make the widget read-only so that no further changes are possible

  genString query  = getSelectedQuery();
  if ( query.not_null() && query != "" )
    {
      genString newval = weight_txt->text();
      genString oldval = getQueryWeight(query);

      if ( newval != oldval )
	{
	      if ( validateInteger(newval) == 1 )
		{
		  changeWeight(query, newval);
		  recalculateTree(ROOT_FOLDER);
		  updateFields();
		  retval = 0;
		}
	}
    }

  return retval;
}

////////////////////////////////////////////////////////////////////////////////
//
// UI "Helper" methods
//
////////////////////////////////////////////////////////////////////////////////
  
int QualityCockpitUI::getNewThreshold(genString const &item)
{
  Initialize(QualityCockpitUI::getNewThreshold(genString const &));

  int retval = -1;

  if ( item.not_null() )
    {
      Prompt dialog(toplev,
		    TXT("Change Threshold"),
		    TXT("Enter new threshold value:"),
		    TXT("Please enter a threshold value."));

      genString newval = getQueryThreshold(item);
      
      if ( dialog.oneshot(newval) == 1 )
	{
	  validateInteger(newval);
	  if ( changeThreshold(item, newval) == 0 )
	    retval = 0;
	}
    }

  return retval;
}

genString QualityCockpitUI::getSelectedQuery(void)
{
  Initialize(QualityCockpitUI::getSelectedQuery(void));

  genString retval;

  if ( query_list )
    {
      int *positions;
      int num_positions;
      query_list->get_selected_pos(&positions, &num_positions);
      
      if ( num_positions > 0 )
	{
	  // adjust for the difference in Motif/gt posisitioning
	  int position = 0;
	  if ( *positions == 0 )
	    position = query_list->num_items() - 1;
	  else
	    position = *positions - 1;

	  // get the item
	  genString selection = query_list->item_at_pos(position);

	  // remove the icon if necessary
	  if ( isicon(selection[0]) )
	    {
	      genString temp = selection;
	      selection = ((char *) temp) + 1;
	    }
	  
	  // create the full name
	  if ( OSapi_strcmp(category_txt->text(), (char *)ROOT_FOLDER) == 0 )
	    retval.printf("%c%s", FOLDER_SEP, selection.str());
	  else
	    retval.printf("%s%c%s", category_txt->text(), FOLDER_SEP, selection.str());
	}
    }
  
  return retval;
}

int QualityCockpitUI::validateInteger(genString &val)
{
  Initialize(QualityCockpitUI::validateInteger(genString &));
  
  if ( val.not_null() && val != "" )
    {
      genString strval;
      strval.printf("%d", OSapi_atoi(val));
      if ( strval != val )
	{
	  msg("ERROR: The enetered value is not a proper\ninteger. It will be replaced with\nits nearest possible approximation.") << eom;
	  val = strval;
	}
    }
  else
    {
      msg("ERROR: The entered value is an empty string.\nIt will be reset to zero.") << eom;
      val = "0";
    }

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Methods to use Access API to Quality Cockpit information
//
////////////////////////////////////////////////////////////////////////////////

void QualityCockpitUI::initAPI(void)
{
  Initialize(QualityCockpitUI::initAPI(void));
  
  genString result;
  sendCmd("source_dis sev.dis", result);
  sendCmd("sev_init", result);
}

int QualityCockpitUI::debuggingAccess(void)
{
  Initialize(QualityCockpitUI::debuggingAccess(void));
  
  if ( debug_access == -1 )
    if ( OSapi_getenv("DISCOVER_DEBUG_QCP") )
      debug_access = 1;
    else
      debug_access = 0;
  
  return debug_access;
}

int QualityCockpitUI::getChildren(genString const &folder, genString *&children)
{
  Initialize(QualityCockpitUI::getChildren(genString const &, genString *&));
  
  int retval = -1;

  if ( folder.not_null() )
    {
      genString command, result;
      command.printf("sev_get {%s} children", folder.str());
      if ( sendCmd(command, result) == TCL_OK )
	{
	  TclList childlist;
	  childlist = (char *) result;
	  if ( (retval = childlist.Size()) > 0 )
	    {
	      children = new genString [childlist.Size()];
	      for ( int i = 0; i < childlist.Size(); i++ )
		children[i] = childlist[i]; 
	    }
	}
    }
  
  return retval;
}

genString QualityCockpitUI::getParent(genString const &folder)
{
  Initialize(QualityCockpitUI::getParent(genString const &));

  genString retval = ROOT_FOLDER;

  if ( folder.not_null() && folder != "" && folder != ROOT_FOLDER )
    {
      int len = OSapi_rindex(folder, FOLDER_SEP) - folder.str();
      if ( len != 0 )
	{
	  retval.put_value(folder, len);
	}
    }
      
  return retval;
}


QualityCockpitUI::categoryType QualityCockpitUI::getType(genString const &name)
{
  Initialize(QualityCockpitUI::getType(genString const &));

  categoryType retval = UNKNOWN;

  if ( name.not_null() )
    {
      genString command, result;
      command.printf("lindex [sev_get_item \"%s\"] 0", name.str());
      if ( sendCmd(command, result) == TCL_OK )
	{
	  if ( result == "folder" )
	    retval = FOLDER;
	  else if ( result == "query_0" )
	    retval = QUERY;
	  else if ( result == "query_1" )
	    retval = THRESH_QUERY;
	  else if ( result == "query_2" )
	    retval = STAT_QUERY;
	}
    }

  return retval;
}

int QualityCockpitUI::enabled(genString const &item)
{
  Initialize(QualityCockpitUI::enabled(genString const &));

  int retval = 0;

  if ( item.not_null() )
    {
      genString command, result;
      command.printf("sev_get \"%s\" on", item.str());
      if ( sendCmd(command, result) == TCL_OK )
	retval = result[0] - '0';
    }
  
  return retval;
}

int QualityCockpitUI::enable(genString const &item)
{
  Initialize(QualityCockpitUI::enable(genString const &));

  int retval = -1;

  if ( item.not_null() )
    {
      genString command, result;
      command.printf("sev_set \"%s\" on 1", item.str());
      if ( sendCmd(command, result) == TCL_OK )
	{
	  session_modified = true;
	  retval = 0;
	}
    }
  
  return retval;
}

int QualityCockpitUI::disable(genString const &item)
{
  Initialize(QualityCockpitUI::disable(genString const &));

  int retval = -1;

  if ( item.not_null() )
    {
      genString command, result;
      command.printf("sev_set \"%s\" on 0", item.str());
      if ( sendCmd(command, result) == TCL_OK )
	{
	  session_modified = true;
	  retval = 0;
	}
    }
  
  return retval;
}

int QualityCockpitUI::execQueryTree(genString const &tree)
{
  Initialize(QualityCockpitUI::execQueryTree(genString const &));

  int retval = -1;

  if ( tree.not_null() )
    {
      genString command, result;
      command.printf("sev_run \"%s\" \"%s\"", tree.str(), cur_input_type.str());
      if ( sendCmd(command, result) == TCL_OK )
	{
	  session_modified = true;
	  retval = 0;
	}
      if ( result.not_null() && result != "" )
	{
	  // this means we have error output
	  displayErrors(result);
	}
    }
  
  return retval;
}

int QualityCockpitUI::getInputTypes(genString *&types)
{
  Initialize(QualityCockpitUI::getInputTypes(genString *&));

  int retval = -1;
  
  genString result;
  if ( sendCmd("sev_get_input_types", result) == TCL_OK )
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

genString QualityCockpitUI::getQueryScore(genString const &item)
{
  Initialize(QualityCockpitUI::getQueryScore(genString const &));

  genString retval("?");
  
  if ( item.not_null() )
    {
      genString command;
      command.printf("sev_get \"%s\" score", item.str());
      sendCmd(command, retval);
    }
  
  return retval;
}


genString QualityCockpitUI::getQueryWeight(genString const &item)
{
  Initialize(QualityCockpitUI::getQueryWeight(genString const &));

  genString retval("?");
  
  if ( item.not_null() )
    {
      genString command;
      command.printf("sev_get \"%s\" weight", item.str());
      sendCmd(command, retval);
    }
  
  return retval;
}

genString QualityCockpitUI::getQueryHits(genString const &item)
{
  Initialize(QualityCockpitUI::getQueryHits(genString const &));

  genString retval("?");
  
  if ( item.not_null() )
    {
      genString command;
      command.printf("sev_get \"%s\" hitno", item.str());
      sendCmd(command, retval);
    }
  
  return retval;
}

genString QualityCockpitUI::getQueryThreshold(genString const &item)
{
  Initialize(QualityCockpitUI::getQueryThreshold(genString const &));

  genString retval("?");
  
  if ( item.not_null() )
    {
      genString command;
      command.printf("sev_get \"%s\" threshold", item.str());
      sendCmd(command, retval);
    }
  
  return retval;
}

genString QualityCockpitUI::getOverallScore(void)
{
  Initialize(QualityCockpitUI::getOverallScore(void));

  return getQueryScore(ROOT_FOLDER);
}

int QualityCockpitUI::resetTree(void)
{
  Initialize(QualityCockpitUI::resetTree(void));

  int retval = -1;

  genString result;
  if ( sendCmd("sev_reset", result) == TCL_OK )
    retval = 0;

  return retval;
}

genString QualityCockpitUI::getSessionName(void)
{
  Initialize(QualityCockpitUI::getSessionName(void));

  genString retval;
  sendCmd("sev_current_session", retval);
  return retval;
}
    

int QualityCockpitUI::browseHits(genString const &path)
{
  Initialize(QualityCockpitUI::browseHits(genString const &));

  int retval = -1;

  genString command, result;
  command.printf("sev_browse_results \"%s\"", path.str());
  if ( sendCmd(command, result) == TCL_OK )
    retval = 0;

  return retval;
}

int QualityCockpitUI::loadSession(genString const &session)
{
  Initialize(QualityCockpitUI::loadSession(genString const &));

  int retval = -1;

  if ( session.not_null() )
    {
      genString command, result;

      if ( session != "" )
	command.printf("sev_load \"%s\"", session.str());
      else
	command = "sev_init";

      if ( sendCmd(command, result) == TCL_OK )
	{
	  session_modified = false;
	  retval = 0;
	}
    }
  
  return retval;
}

int QualityCockpitUI::saveSession(genString const &session)
{
  Initialize(QualityCockpitUI::saveSession(genString const &));

  int retval = -1;

  if ( session.not_null() && session != "" )
    {
      genString command, result;

      command.printf("sev_save \"%s\"", session.str());
      if ( sendCmd(command, result) == TCL_OK )
	{
	  session_modified = false;
	  retval = 0;
	}
    }

  return retval;
}

int QualityCockpitUI::deleteSession(genString const &session)
{
  Initialize(QualityCockpitUI::deleteSession(genString const &));

  int retval = -1;

  if ( session.not_null() && session != "" )
    {
      genString command, result;

      command.printf("sev_delete \"%s\"", session.str());
      if ( sendCmd(command, result) == TCL_OK )
	retval = 0;
    }

  return retval;
}

int QualityCockpitUI::getAllSessions(genString *&sessions)
{
  Initialize(QualityCockpitUI::getAllSessions(genString *&));

  int retval = -1;
  
  genString result;
  if ( sendCmd("sev_list", result) == TCL_OK )
    {
      TclList l = (char *) result;
      if ( l.Size() > 0 )
	{
	  sessions = new genString [l.Size()];
	  for ( int i = 0; i < l.Size(); i++ )
	    sessions[i] = l[i];
	}
      retval = l.Size();
    }
  
  return retval;
}

int QualityCockpitUI::changeWeight(genString const &query, genString const &value)
{
  Initialize(QualityCockpitUI::changeWeight(genString const &, genString const &));

  int retval = -1;

  if ( query.not_null() && query != "" && value.not_null() && value != "" )
    {
      genString command, result;

      command.printf("sev_set \"%s\" weight \"%s\"", query.str(), value.str());
      if ( sendCmd(command, result) == TCL_OK )
	{
	  session_modified = true;
	  retval = 0;
	}
    }

  return retval;
}

int QualityCockpitUI::changeThreshold(genString const &query, genString const &value)
{
  Initialize(QualityCockpitUI::changeThreshold(genString const &, genString const &));

  int retval = -1;

  if ( query.not_null() && query != "" && value.not_null() && value != "" )
    {
      genString command, result;

      command.printf("sev_set \"%s\" threshold \"%s\"", query.str(), value.str());
      if ( sendCmd(command, result) == TCL_OK )
	{
	  session_modified = true;
	  retval = 0;
	}
    }

  return retval;
}

int QualityCockpitUI::recalculateTree(genString const &tree)
{
  Initialize(QualityCockpitUI::recalculateTree(genString const &));

  int retval = -1;

  if ( tree.not_null() && tree != "" )
    {
      genString command, result;
      
      command.printf("sev_calc_score \"%s\"", tree.str());
      if ( sendCmd(command, result) == TCL_OK )
	retval = 0;
    }
  
  return retval;
}

////////////////////////////////////////////////////////////////////////////////
//
// Direct access to Access
//
////////////////////////////////////////////////////////////////////////////////

int QualityCockpitUI::sendCmd(genString command, genString &result)
{
  Initialize(QualityCockpitUI::sendCmd(genString, genString &));

  int retval = TCL_ERROR;

  Interpreter *i = GetActiveInterpreter();
  if ( i )
    {
      if ( debuggingAccess() )
	msg("QualityCockpitUI: Sending command: $1") << command.str() << eom;

      cli_eval_string(command);  
      retval = i->GetResult(result);  
      
      if ( debuggingAccess() )
	msg("QualityCockpitUI: Received result: $1") << result.str() << eom;

      if (retval == TCL_ERROR) 
	notifyTclError();
    }

  return retval;
}

void QualityCockpitUI::notifyTclError(void)
{
  Initialize(QualityCockpitUI::notifyTclError(void));

  Interpreter *i = GetActiveInterpreter();
  if ( i )
    {
      char *err_info = Tcl_GetVar(i->interp, "errorInfo", TCL_GLOBAL_ONLY);
      msg("ERROR: The following error occured:\n\n$1") << err_info << eom;
    }
}
