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
// baseBrowser.h.C
//------------------------------------------
// synopsis:
// Common base class for file and project browsers.
//
//------------------------------------------

// INCLUDE FILES

#include <Interpreter.h>
#include <browserShell.h>
#include <baseBrowser.h>

#include <gtForm.h>
#include <gtStringEd.h>
#include <gtOptionMenu.h>
#include <mpb.h>

// VARIABLE DEFINITIONS

static const char* string_editor_label;

// FUNCTION DEFINITIONS

baseBrowser::baseBrowser(browserShell* b, const char* label)
: bsh(*b), gt_form(NULL), gt_string_ed(NULL), scope_ed(NULL), scope_mode_menu(NULL)
{
    Initialize(baseBrowser::baseBrowser);

    string_editor_label = label; // Used later in build_interface().
}

baseBrowser::~baseBrowser()
{ Initialize(baseBrowser::~baseBrowser); }

gtPrimitive* baseBrowser::top()
{
    Initialize(baseBrowser::top);
#ifndef NEW_UI
    return gt_form;
#else
    return NULL;
#endif
}

void baseBrowser::filter_CB(
    gtStringEditor* se, gtEventPtr, void* bb, gtReason)
{
    Initialize(baseBrowser::filter_CB);
#ifndef NEW_UI
    char* pathname = se->text();
    ((baseBrowser*)bb)->filter(pathname);
    gtFree(pathname);
#endif
}

void baseBrowser::list_action(gtList*, gtEventPtr, void* bb, gtReason)
{
    ((baseBrowser*)bb)->bsh.default_action();
}

extern int scope_get_global_mode();
extern const char* scope_get_string();

#ifndef NEW_UI
static char* scope_modes[] = {"off ", "wide", "deep"};
static void scope_mode_CB(void*, gtEventPtr, void* data, gtReason)
{
  char* mode = (char*) data;

  genString cmd = "set_mode ";
  cmd += mode;
  cli_eval_string(cmd);
}

void static scope_txt_CB( gtStringEditor* se, gtEventPtr, void* bb, gtReason)
{
  Initialize(baseBrowser::filter_CB);
  char* pathname = se->text();
  
  genString cmd = "set_scope ";
  cmd += pathname;
  cli_eval_string((char*)cmd);

  gtFree(pathname);
}
extern int scope_is_on;
#endif

void baseBrowser::build_interface(const char* name)
{
    Initialize(baseBrowser::build_interface);
#ifndef NEW_UI
    int is_proj_browser = 0;
    if(!name) {
	name = "project_browser";
	if(scope_is_on)
	    is_proj_browser = 1;
    }
    gt_form = gtForm::create(bsh.container(), name);
    
    if(is_proj_browser) {  // new look to support scope
	gtForm *str_form = gtForm::create(gt_form, "zigzag");
	
	gtLabel* label = gtLabel::create(str_form, "browser_label", "Project: ");
	string_editor_label = NULL;
	label->alignment(gtBeginning);
	label->attach(gtTop);
	label->attach(gtBottom);
	label->attach(gtLeft);
	
	gt_string_ed = gtStringEditor::create(str_form, "gt_string_ed", "<>");
	gt_string_ed->activate_callback(filter_CB, this);
	gt_string_ed->hide_unfocused_cursor();
	gt_string_ed->attach(gtTop);
	gt_string_ed->attach(gtLeft, label);
	gt_string_ed->attach(gtRight, label, -300, 1);
	
	gtLabel* scan_label = gtLabel::create(str_form, "scan_label", "   Scope: ");
	scan_label->attach(gtLeft, gt_string_ed);
	scan_label->attach(gtTop);
	scan_label->attach(gtBottom);
	
	scope_ed = gtStringEditor::create(str_form, "scope_ed", "/");
	scope_ed->activate_callback(scope_txt_CB, this);
	scope_ed->hide_unfocused_cursor();
	scope_ed->attach(gtLeft, scan_label);
	scope_ed->attach(gtTop);
	const char* txt = scope_get_string();
	scope_ed->text(txt);
	scope_mode_menu = gtOptionMenu::create(str_form, "scope_mode", "Scope Mode:", NULL);
	
	for(int ii=0; ii<3; ++ii) {
	    char* nm = scope_modes[ii];
	    scope_mode_menu->insert_entries(-1, gtMenuStandard, nm, nm, nm, scope_mode_CB, NULL);
	}
	int cur_mode = scope_get_global_mode();
	scope_mode_menu->menu_history(scope_modes[cur_mode]);
	scope_mode_menu->attach(gtTop);
	scope_mode_menu->attach(gtLeft, NULL, -200, 1);
	
	scope_ed->attach(gtRight, scope_mode_menu);
	
	gtPrimitive* body = build_body(gt_form);
	
	gtForm::vertStack(str_form, body);
	body->attach(gtBottom);
	
	label->manage();
	gt_string_ed->manage();
	scope_ed->manage();
	scope_mode_menu->manage();
	body->manage();
	str_form->manage();
	
    } else {  // old look
	
	gtLabel* label = gtLabel::create(
	    gt_form, "browser_label",
	    string_editor_label ? string_editor_label : "");
	string_editor_label = NULL;
	label->alignment(gtBeginning);
	
	gt_string_ed = gtStringEditor::create(gt_form, "gt_string_ed", "<>");
	gt_string_ed->activate_callback(filter_CB, this);
	gt_string_ed->hide_unfocused_cursor();
	
	gtPrimitive* body = build_body(gt_form);
	
	gtForm::vertStack(label, gt_string_ed, body);
	body->attach(gtBottom);
	
	gt_string_ed->manage();
	body->manage();
    }
    gt_form->manage();
    
    mpb_incr();

#endif
}

/*
   START-LOG-------------------------------------------

   $Log: baseBrowser.h.C  $
   Revision 1.11 2001/01/12 10:57:18EST sschmidt 
   Port to new SGI compiler
Revision 1.2.1.5  1993/10/19  13:52:21  kws
Bug track: 4918
Make sure directory is updated after browser is created

Revision 1.2.1.4  1993/01/10  06:04:25  glenn
Add string_editor_label to ctor and interface.

Revision 1.2.1.3  1993/01/07  14:36:15  glenn
New browserShell interface.

Revision 1.2.1.2  1992/10/09  20:13:00  smit
*** empty log message ***

   END-LOG---------------------------------------------
*/
