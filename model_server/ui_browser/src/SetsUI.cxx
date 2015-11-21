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
#ifndef NEW_UI

//
// "SetsUI.C"
//
// Description: 
//
//     Dialog box for set-theoretic operations.
//     Manipulates Access/sw "sets."
//

#include <msg.h>
#include <SetsUI.h>
#include <genError.h>
#include <gtDlgTemplate.h>
#include <gtForm.h>
#include <gtList.h>
#include <gtTogB.h>
#include <gtMainWindow.h>
#include <gtMenuBar.h>
#include <gtCascadeB.h>
#include <gtOptionMenu.h>
#include <gtTopShell.h>
#include <gtStringEd.h>
#include <gtPushButton.h>
#include <gtSepar.h>
#include <help.h>
#include <Interpreter.h>
#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <limits.h>
#else /* ISO_CPP_HEADERS */
#include <limits>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <messages.h>
#include <ldrList.h>
#include <groupHdr.h>
#include <externApp.h>
#include <browserShell.h>

#define FATAL_ERROR \
  if (1) { \
    msg("An unexpected error occured in file: $1 Line $2.\nPlease notify technical support.", catastrophe_sev) << __FILE__ << eoarg << __LINE__ << eom; \
    Assert(0); \
  } else

gtList *SetsUI::sets_list = NULL;
SetsUI *SetsUI::instance = NULL;
int SetsUI::name_idx = 1;
gtStringEditor *SetsUI::name_txt = NULL;
gtTopLevelShell *SetsUI::toplev = NULL;
const int SetsUI::max_name_idx = INT_MAX - 1;
const char *SetsUI::api_cmd_prefix = "_smgr";
bool SetsUI::debug_flag = false;
const char *SetsUI::default_title = "Group Manager";

//
// The order of these matter; they correspond to the CmdCode enum in SetsUI.h:
//
const char *SetsUI::api_cmd[] = { 
  "size",
  "lindex",
  "find",
  "modelbrowse",
  "minibrowse",
  "iview",
  "create",
  "rename",
  "delete",
  "save",
  "print",
  "intersection -mktmp",
  "union -mktmp",
  "difference -mktmp",
  "add sel",
  "remove sel",
  "move sel"
};

SetsUI::SetsUI(const char *title)
{
    Initialize(SetsUI::SetsUI);
    
    checkDebug();
    
    buildInterface(title);
    invoked = 0;
}

SetsUI::~SetsUI()
{
    Initialize(SetsUI::~SetsUI);
    
    toplev->popdown();
    delete toplev;
    instance = NULL;
}

void SetsUI::Invoke(const char *title)
{
  Initialize(SetsUI::Invoke);

  if (instance) {
      if(!instance->invoked){
	  instance->toplev->popup();
	  instance->invoked = 1;
      }
      instance->toplev->bring_to_top();
  } else {
    if (!title)
	title = default_title;
    instance = new SetsUI(title);
    instance->toplev->popup();
    instance->toplev->bring_to_top();
    instance->invoked = 1;
  }
}

void SetsUI::CaptureAndInvoke(const char *title, symbolArr& syms)
{
    Initialize(SetsUI::CaptureAndInvoke);
    
    if(instance == NULL) {
	if (!title)
	    title = default_title;
	instance = new SetsUI(title);
	instance->toplev->popup();
	instance->invoked = 1;
    } else {
	if(!instance->invoked){
	    instance->toplev->popup();
	    instance->invoked = 1;
	}
        instance->toplev->bring_to_top();
    }

    char *name       = instance->name_txt->text();
    symbolArr *arr   = new symbolArr;
    extGroupApp* app = (extGroupApp*) extGroupApp::create_tmp(name, arr);
    *arr             = syms;
    app->set_status(externApp::TEMP);
    gtFree(name);
    instance->Refresh();
    instance->setDefaultName();
}

char *SetsUI::GetGroupName(void)
{
    static genString grp_name;

    if(instance == NULL) {
	instance = new SetsUI(default_title);
    } 
    char *name = instance->name_txt->text();
    grp_name   = name;
    gtFree(name);
    return (char *)grp_name;
}

void SetsUI::UpdateGroupName(void)
{
    if(instance == NULL) {
	instance = new SetsUI(default_title);
    } 
    instance->setDefaultName();
}

void SetsUI::buildInterface(const char *title)
{
  Initialize(SetsUI::buildInterface);

  // Top level, main window:
  //

  gtShell *gtsh = NULL;
  genArrOf(browserShellPtr) *instance_array = browserShell::get_browsers_list();
  if (instance_array && instance_array->size()) {
    browserShell *bsh = *((*instance_array)[0]);
    if (bsh)
      gtsh = bsh->top_level();
  }

  toplev = gtTopLevelShell::create(gtsh, title);
  toplev->title(title);
  toplev->override_WM_destroy(SetsUI::destroy_CB);
  gtMainWindow *main_window = gtMainWindow::create(toplev, "main_window", 0);
  main_window->manage();

  // Menu bar:
  //
  gtMenuBar *menu_bar = main_window->menu_bar();

  // Menus:
  //
  gtCascadeButton *cascade_btn;

  // "Manage" menu:
  cascade_btn = gtCascadeButton::create(menu_bar, "manage_menu", TXT("Group"),
				        NULL, NULL);
  cascade_btn->pulldown_menu("manage_menu",
			     gtMenuStandard, "create", TXT("Create"), this, SetsUI::create_CB,
			     gtMenuStandard, "rename", TXT("Rename"), this, SetsUI::rename_CB,
			     gtMenuStandard, "delete", TXT("Delete"), this, SetsUI::delete_CB,
			     gtMenuSeparator, "sep",
			     gtMenuStandard, "save", TXT("Save"), this, SetsUI::save_CB,
			     gtMenuStandard, "print", TXT("Print"), this, SetsUI::print_CB,
			     gtMenuSeparator, "sep",
			     gtMenuStandard, "refresh", TXT("Refresh"), this, SetsUI::refresh_CB,
			     gtMenuSeparator, "sep",
			     gtMenuStandard, "close", TXT("Close"), this, SetsUI::quit_CB,
			     NULL);
  cascade_btn->manage();

  // "Elements" menu:
  cascade_btn = gtCascadeButton::create(menu_bar, "elements_menu", TXT("Elements"),
					NULL, NULL);
  cascade_btn->pulldown_menu("elements_menu",
			     gtMenuStandard, "add", TXT("Add"), this, SetsUI::ElementsAdd_CB,
			     gtMenuStandard, "remove", TXT("Remove"), this, SetsUI::ElementsRemove_CB,
			     gtMenuStandard, "move", TXT("Replace"), this, SetsUI::ElementsMove_CB,
			     NULL);
  cascade_btn->manage();


  // "Operator" menu:
  cascade_btn = gtCascadeButton::create(menu_bar, "operator_menu", TXT("Operator"),
					NULL, NULL);
  cascade_btn->pulldown_menu("operator_menu",
			     gtMenuStandard, "intersection", TXT("Intersection"), this, SetsUI::intersection_CB,
			     gtMenuSeparator, "sep",
			     gtMenuStandard, "union", TXT("Union"), this, SetsUI::union_CB,
			     gtMenuSeparator, "sep",
			     gtMenuStandard, "diff_ab", TXT("A - B"), this, SetsUI::diff_ab_CB,
			     gtMenuStandard, "diff_ba", TXT("B - A"), this, SetsUI::diff_ba_CB,
			     NULL);
  cascade_btn->manage();

  // "View" menu:
  cascade_btn = gtCascadeButton::create(menu_bar, "browse_menu", TXT("Browse"),
					NULL, NULL);
  cascade_btn->pulldown_menu("browse_menu",
			     gtMenuStandard, "model_browser", TXT("Model Browser"), this, SetsUI::ModelBrowser_CB,
			     gtMenuStandard, "mini_browser", TXT("Minibrowser"), this, SetsUI::MiniBrowser_CB,
			     gtMenuStandard, "instances", TXT("Instances"), this, SetsUI::instances_CB,
			     NULL);
  cascade_btn->manage();

  // "Help" menu:
  cascade_btn = gtCascadeButton::create(menu_bar, "help_menu", TXT("Help"),
					NULL, NULL);
  menu_bar->set_help(cascade_btn);
  cascade_btn->pulldown_menu("help_menu",
			     gtMenuStandard, "help_set_manager", TXT("Contents"), this,
			     gtBase::help_button_callback, NULL);
  REG_HELP(cascade_btn->button("help_set_manager"), "Browser.Y2K.SetsManager.Help");
  cascade_btn->manage();

  // Create the form:
  //
  gtForm *form = gtForm::create(main_window, "form");
  form->attach_tblr();
  form->manage();

  // Label for textbox:
  //
  gtLabel *name_lbl = gtLabel::create(form, "name_lbl", TXT("Name:"));
  name_lbl->attach(gtBottom, 0, 17);
  name_lbl->attach(gtLeft, 0, 5);
  name_lbl->manage();

  // Textbox:
  //
  name_txt = gtStringEditor::create(form, "name_txt", NULL);
  name_txt->attach(gtBottom, 0, 10);
  name_txt->attach(gtLeft, name_lbl, 5);
  name_txt->attach(gtRight);
  name_txt->manage();

  // Separator:
  //
  gtSeparator *h_sep = gtSeparator::create(form, "h_sep", gtHORZ);
  h_sep->manage();
  h_sep->attach(gtLeft);
  h_sep->attach(gtRight);
  h_sep->attach(gtBottom, name_txt, 10);

  // List of Sets:
  //
  sets_list = gtList::create(form, "sets_list", TXT("Active Groups"), gtExtended,
			    NULL, 0);
  sets_list->width(250);
  sets_list->height(300);
  sets_list->attach(gtTop, 0, 15);
  sets_list->attach(gtBottom, h_sep, 10);
  sets_list->attach(gtLeft, 0, 5);
  sets_list->attach(gtRight, 0, 5);
  sets_list->action_callback(SetsUI::list_double_click_CB, this);
  sets_list->manage();

  setDefaultName();       // Put some default string in the name box

  Refresh();              // Fill in the list
}

void SetsUI::checkDebug()
{
  if (OSapi_getenv("PSET_SETS_DEBUG"))
    debug_flag = true;
}

//
// Update the graphical list:
//
void SetsUI::Refresh()
{
  Initialize(SetsUI::Refresh);

  if (!sets_list)
    return;

  //
  // Add any new items that may have been created:
  //

  genString cmd;
  cmd.printf("%s", api_cmd[SIZE]);

  genString result;
  int code = sendCmd(cmd, result);

  if (code == TCL_ERROR)
    return;

  int size = OSapi_atoi(result);  

  int i;
  for (i = 0; i < size; i++) {
    genString num;
    num.printf("%d", i);
    cmd.printf("%s %s", api_cmd[LINDEX], (char *)num);

    int code = sendCmd(cmd, result);
    
    if (code == TCL_ERROR)
      return;
    
    if (!inList(result)) {
      genString pix;
      pix.printf("%c%s", PIX_SET, (char *)result);
      sets_list->add_item_unselected(pix, 0);
    }
  }

  //
  // Remove any old items that may have been deleted:
  //

  size = sets_list->num_items();

  for (i = size - 1; i >= 0; i--) {
    char *item = sets_list->item_at_pos(i);
    cmd.printf("%s %s", api_cmd[SEARCH], ((char *)item) + 1);

    genString result;

    int code = sendCmd(cmd, result);

    if (code == TCL_ERROR)
      return;

    gtFree(item);

    if (result == "0") {
      // Account for incompatibility between item_at_pos()
      // and delete_pos(). The latter uses traditional Motif
      // numbering convention (1, 2, ..., 0); the former uses
      // intuitive numbering scheme (0, 1, 2, ...).
      //
      int pos;
      int new_size = sets_list->num_items();

      if (i == new_size - 1)
        pos = 0;
      else
	pos = i + 1;

      sets_list->delete_pos(pos);
    }
  }

  setDefaultName();       // Put some default string in the name box
}

//
// Is the specified string in our list?
//
bool SetsUI::inList(const char *name)
{
  Initialize(SetsUI::inList);

  if(!name)
    return false;

  int sz = sets_list->num_items();  
  bool collision = false;

  for (int j = 0; j < sz; j++) {
    char *str = sets_list->item_at_pos(j);
    if (!(strcmp(str + 1, name))) {
      collision = true;
      gtFree(str);
      break;
    }
    gtFree(str);
  }

  return collision;
}

//
// Put some default value into the name field:
//
bool SetsUI::setDefaultName()
{
  Initialize(SetsUI::setDefaultName);

  genString name;
  bool valid = false;

  while ((name_idx < max_name_idx) && (valid == false)) {
    name.printf("RESULT_%d", name_idx);
    if (!inList(name))
      valid = true;
    else
      ++name_idx;
  }

  if (valid == true)
    name_txt->text(name);
  else
    msg("Exceeded maximum index number.", error_sev) << eom;

  return valid;
}

void SetsUI::quitWindow()
{
    Initialize(SetsUI::quitWindow);
    
    toplev->popdown();
    instance->invoked = 0;
}

//
// Get the list of groups currently selected:
//
void SetsUI::ListSelected(TclList &list)
{
  Initialize(SetsUI::ListSelected);

  if (instance)
      instance->tclList2(list);
}

//
// Use new data structure to capture list of selected items:
//
void SetsUI::tclList2(TclList &list)
{
  Initialize(SetsUI::tclList2);

  int n_items = sets_list->num_selected();
  char **v_items = sets_list->selected();

  for (int i = 0; i < n_items; i++)
    list += (v_items[i]) + 1;

  gtFree(v_items);
}

//
// Convert the selections in our graphical list to a Tcl list:
//
void SetsUI::tclList(genString& list, ListDirCode dir = FORWARD)
{
  Initialize(SetsUI::tclList);

  int n_items = sets_list->num_selected();
  char **v_items = sets_list->selected();

  if (dir == FORWARD)
    for (int i = 0; i < n_items; i++) {
      list += "{";
      list += (v_items[i]) + 1;
      list += "} ";
    }
  else if (dir == BACKWARD)
    for (int i = n_items - 1; i >= 0; i--) {
      list += "{";
      list += (v_items[i]) + 1;
      list += "} ";
    }
  else
    // Shouldn't get here:
    FATAL_ERROR;

  gtFree(v_items);
}

int SetsUI::sendCmd(genString& cmd)
{
  Initialize(SetsUI::sendCmd);

  genString result;
  return sendCmd(cmd, result);
}

int cli_eval_string(const char*);

int execManageTool(genString cmd, int outFlag)
{
    return SetsUI::execCmd(cmd, outFlag);
}
int SetsUI::execCmd(genString cmd, int outFlag )
{
    msg("execTool: Sending command: \"$1\".", normal_sev) << (char *)cmd << eom;
    cli_eval_string((char*)cmd);

    genString result;
    int code = interpreter_instance->GetResult(result);  

  if (code == TCL_ERROR && outFlag ) 
    notifyTclError();

  return code;
}

//
// Send a command to the interpreter; do error checking:
//
int SetsUI::sendCmd(const genString& cmd, genString& result)
{
  Initialize(SetsUI::sendCmd);

  genString full_cmd;
  full_cmd.printf("%s { %s }", (char *)api_cmd_prefix, (char *)cmd);

  if (debug_flag)
    msg("Status: Sets: Sending command: \"$1\".", normal_sev) << (char *)full_cmd << eom;

  cli_eval_string(full_cmd);  

  int code = interpreter_instance->GetResult(result);  

  if (code == TCL_ERROR) 
    notifyTclError();

  return code;
}

//
// Display Tcl error message:
//
void SetsUI::notifyTclError()
{
  Initialize(SetsUI::notifyTclError);

  char *err_info = Tcl_GetVar(interpreter_instance->interp, "errorInfo", TCL_GLOBAL_ONLY);
  msg("The following error occured:\n$1", error_sev) << err_info << eom;
}

//
// Check to see that the numbers of items selected
// equals the necessary number of operands. Notify
// user of any errors:
//
bool SetsUI::checkOperands(OperandNumCode num)
{
  Initialize(SetsUI::checkOperands);

  bool retval = false;

  int num_sel = sets_list->num_selected();

  switch (num)
  {
    case UNARY:         // One operand:         
                        //
                        if (num_sel != 1)
	                  msg("This operation requires exactly one selection.", error_sev) << eom; 
			else
			  retval = true;
			break;

    case BINARY:        // Two operands:
			//
                        if (num_sel != 2)
	                  msg("This operation requires exactly two selections.", error_sev) << eom; 
			else
			  retval = true;
			break;

    case ONE_OR_MORE:   // One or more operands:
			//
			if (num_sel < 1)
			  msg("This operation requires one or more selections.", error_sev) << eom;
			else
			  retval = true;
			break;

    case TWO_OR_MORE:   // Two or more operands:
			//
                        if (num_sel < 2)
	                  msg("This operation requires two or more selections.", error_sev) << eom;
			else
			  retval = true;
			break;

    default:            // Shouldn't get here:
			//
			FATAL_ERROR;
			break;
  }

  return retval;
}

void SetsUI::sendToBrowser(BrowseCode bc = MINI_B)
{
  Initialize(SetsUI::sendToBrowser);

  if (!checkOperands(ONE_OR_MORE))
    return;

  genString list;
  tclList(list);

  genString cmd;
  CmdCode api;

  switch (bc)
  {
    case INSTANCE_B: api = IVIEW;
		     break;
    case MINI_B:                   // Slide down
		     api = MINIBROWSE;
		     break;
    default:         api = MODELBROWSE;
		     break;
  }

  cmd.printf("%s %s", api_cmd[api], (char *)list);

  sendCmd(cmd);
}

char *SetsUI::targetName()
{
  Initialize(SetsUI::targetName());

  char *name = name_txt->text();

  if (!(*name))
    if (!setDefaultName()) {
      msg("Error occured while trying to get default group name.", error_sev) << eom;
      name = NULL;
    }
    else
      name = name_txt->text();

  if (inList(name)) {
    msg("Duplicate group name specified.", error_sev) << eom;
    name = NULL;
  }

  return name;
}

//
// Callbacks:
//

int SetsUI::destroy_CB(void *)
{
  Initialize(SetsUI::destroy_CB);

  quitWindow();
  instance = NULL;
  sets_list = NULL;

  return 1;
}

void SetsUI::ElementsAdd_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::ElementsAdd_CB);

  if (!checkOperands(UNARY))
    return;  

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s", api_cmd[ADD], (char *)list);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
    setDefaultName();
}

void SetsUI::ElementsRemove_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::ElementsRemove_CB);

  if (!checkOperands(UNARY))
    return;  

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s", api_cmd[REMOVE], (char *)list);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
    setDefaultName();
}

void SetsUI::ElementsMove_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::ElementsMove_CB);

  if (!checkOperands(UNARY))
    return;  

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s", api_cmd[MOVE], (char *)list);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
    setDefaultName();
}

void SetsUI::create_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::create_CB);

  char *name = targetName();
  if (!name)
    return;
  
  genString cmd;
  cmd.printf("%s %s", api_cmd[CREATE], (char *)name);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
      setDefaultName();
}

void SetsUI::rename_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::rename_CB);

  if (!checkOperands(UNARY))
    return;  

  char *name = targetName();
  if (!name)
    return;

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s %s", api_cmd[RENAME], (char *)name, (char *)list);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
    setDefaultName();
}

void SetsUI::delete_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::delete_CB);

  if (!checkOperands(ONE_OR_MORE))
    return;

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s", api_cmd[DELETE], (char *)list);

  sendCmd(cmd);
}

void get_new_subsys_name(genString &, genString &);

void SetsUI::save_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::save_CB);

  if (!checkOperands(ONE_OR_MORE))
    return;

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s", api_cmd[SAVE], (char *)list);

  sendCmd(cmd);
}

void SetsUI::print_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::print_CB);

  if (!checkOperands(ONE_OR_MORE))
    return;

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s", api_cmd[PRINT], (char *)list);

  sendCmd(cmd);
}

void SetsUI::refresh_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::refresh_CB);

  Refresh();
}

void SetsUI::quit_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::quit_CB);

  quitWindow();
}

void SetsUI::intersection_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::intersection_CB);

  if (!checkOperands(TWO_OR_MORE))
    return;

  char *name = targetName();
  if (!name)
    return;
  
  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s %s", api_cmd[INTERSECTION], name, (char *)list);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
    setDefaultName();
}

void SetsUI::union_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::union_CB);

  if (!checkOperands(TWO_OR_MORE))
    return;

  char *name = targetName();
  if (!name)
    return;

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s %s", api_cmd[UNION], name, (char* )list);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
    setDefaultName();
}

void SetsUI::diff_ab_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::diff_ab_CB);

  if (!checkOperands(BINARY))
    return;

  char *name = targetName();
  if (!name)
    return;

  genString list;
  tclList(list);

  genString cmd;
  cmd.printf("%s %s %s", api_cmd[DIFFERENCE], name, (char *)list);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
    setDefaultName();
}

void SetsUI::diff_ba_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::diff_ba_CB);

  if (!checkOperands(BINARY))
    return;

  char *name = targetName();
  if (!name)
    return;

  genString list;
  tclList(list, BACKWARD);

  genString cmd;
  cmd.printf("%s %s %s", api_cmd[DIFFERENCE], name, (char *)list);

  int code = sendCmd(cmd);

  if (code != TCL_ERROR)
    setDefaultName();
}

void SetsUI::instances_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::instances_CB);
  
  sendToBrowser(INSTANCE_B);
}

void SetsUI::ModelBrowser_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::instances_CB);
  
  sendToBrowser(MODEL_B);
}

void SetsUI::MiniBrowser_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::instances_CB);
  
  sendToBrowser(MINI_B);
}

void SetsUI::browse_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
  Initialize(SetsUI::browse_CB);

  sendToBrowser();
}

void SetsUI::list_double_click_CB(gtList*, gtEventPtr, void *, gtReason)
{
  Initialize(SetsUI::list_double_click_CB);

  sendToBrowser(MODEL_B);
}

#endif /* ! NEW_UI */
