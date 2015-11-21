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
// setupProps.h.C
//------------------------------------------
// synopsis:
// Initial setup screen
//
//------------------------------------------

// include files
#include <cLibraryFunctions.h>
#include <msg.h>
#include <gtDlgTemplate.h>
#include <gtForm.h>
#include <gtLabel.h>
#include <gtFrame.h>
#include <gtTogB.h>
#include <gtRadioBox.h>
#include <gtScrollWin.h>
#include <gtStringEd.h>
#include <gtTextEd.h>
#include <gtVertBox.h>
#include <gtPushButton.h>

#include <messages.h>
#include <genError.h>

#include <setupProps.h>
#include <customize.h>
#include <smt.h>
#include <help.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>
class gtPushButton;

// variable definitions

static setupProps* the_setupProps;

// function definitions

extern "C" void popup_setupProps()
{
    setupProps::instance()->popup();
}

setupProps* setupProps::instance()
{
    if(the_setupProps == NULL)
	new setupProps;

    return the_setupProps;
}

setupProps::setupProps()
{
    the_setupProps = this;
    build_interface();

    modified = 0;
}

setupProps::~setupProps()
{
    if(the_setupProps == this)
	the_setupProps = NULL;
    if(dlg){
	delete dlg;
	dlg = NULL;
    }
}

void setupProps::popup()
{
    reset();
    dlg->popup(0);
    dlg->bring_to_top();
}

void setupProps::ok_button(gtPushButton*, gtEventPtr, void*, gtReason)
{
    instance()->dlg->popdown();
    instance()->store();
    instance()->modified = 0;
}

void setupProps::cancel_button(gtPushButton*, gtEventPtr, void*, gtReason)
{
    instance()->dlg->popdown();
}


static gtStringEditor* make_string_editor(
    const char* label_string, gtBase* parent, int indent,
    gtPrimitive*& container)
{
    Initialize(make_string_editor);

    gtForm* form = gtForm::create(parent, "form");
    form->attach_tb(container, form);
    form->manage();

    gtLabel* label = gtLabel::create(form, "label", label_string);
    label->alignment(gtBeginning);
    label->attach_tblr();
    label->attach(gtRight, NULL, -indent, 1);
    label->manage();

    gtStringEditor* se = gtStringEditor::create(form, "string_editor", "");
    se->columns(40);
    se->attach_tblr(NULL, se, label, NULL);
    se->manage();

    container = form;
    return se;
}

static gtStringEditor* make_string_editor(
    const char* label_string, gtBase* parent, int right_indent, int top_indent,
    gtPrimitive*& container)
{
    Initialize(make_string_editor);

    gtForm* form = gtForm::create(parent, "form");
    form->attach_tb(container, form);
    form->attach_offset(gtTop, top_indent);
    form->manage();

    gtLabel* label = gtLabel::create(form, "label", label_string);
    label->alignment(gtBeginning);
    label->attach_tblr();
    label->attach(gtRight, NULL, -right_indent, 1);
    label->manage();

    gtStringEditor* se = gtStringEditor::create(form, "string_editor", "");
    se->columns(40);
    se->attach_tblr(NULL, se, label, NULL);
    se->manage();

    container = form;
    return se;
}

static gtToggleButton* make_toggle(
    const char* label_string, gtBase* parent, gtPrimitive*& container,
    gtToggleCB callback)
{
    Initialize(make_toggle);

    gtToggleButton* toggle = gtToggleButton::create(
	parent, "toggle", label_string, callback, NULL);
    toggle->attach_tblr(container, toggle, NULL, toggle);
    toggle->manage();

    container = toggle;
    return toggle;
}

static gtRadioBox* make_radio_box(
    gtBase* parent, int indent, gtPrimitive*& container, const char *label_string,
    const char *label1, const char *label2)
{
    Initialize(make_radio_box);

    gtForm* form = gtForm::create(parent, "form");
    form->attach_tb(container, form);
    form->manage();

    gtLabel* label = gtLabel::create(form, "label", label_string);
    label->alignment(gtBeginning);
    label->attach_tblr();
    label->attach(gtRight, NULL, -indent, 1);
    label->manage();

    gtRadioBox* radio_box = gtRadioBox::create(form, "radio", NULL,
				       label1, "one", NULL, NULL,
				       label2, "two", NULL, NULL, NULL);
    radio_box->attach_tblr(NULL, radio_box, label, radio_box);
    radio_box->num_columns(2);
    radio_box->manage();

    container = form;
    return radio_box;
}

static gtForm* make_frame(gtBase* parent, gtPrimitive*& container)
{
    Initialize(make_frame);

    gtFrame* frame = gtFrame::create(parent, "frame");
    frame->attach_tb(container, frame);
    frame->attach_offset(gtTop, 10);
    frame->manage();

    gtForm* form = gtForm::create(frame, "frame_form");
    form->manage();

    container = frame;
    return form;
}

static gtForm* make_foldup(gtBase* parent, int indent, gtPrimitive*& container)
{
    Initialize(make_fold_up);

    gtForm* form = gtForm::create(parent, "foldup_form");
    form->attach_tb(container, form);
    form->attach_offset(gtLeft, indent);
    // NOTE: form is not immediately managed.

    container = form;
    return form;
}

void setupProps::widget_destroy(gtBase *, void *data)
{
    Initialize(setupProps::widget_destroy);
    
    setupProps *instance = (setupProps *)data;
    instance->dlg        = NULL;
    delete instance;
}

void setupProps::build_interface()
{
    Initialize(setupProps::build_interface);

    dlg = gtDialogTemplate::create(
	NULL, "preferences", TXT("Preferences"));
    
    dlg->user_destroy(widget_destroy, this);

    dlg->add_button("ok",     TXT("OK"),     setupProps::ok_button, this);
    dlg->add_button("cancel", TXT("Cancel"), setupProps::cancel_button, this);
    dlg->add_help_button();
    REG_HELP(dlg->button("help"), "Browser.Preferences.Help");

    gtForm* hbox = gtForm::create(dlg, "form");
    hbox->attach_tblr();
    hbox->manage();

    gtForm* column1 = gtForm::create(hbox, "form");
    column1->attach_lr(NULL, column1);
    column1->manage();

    gtForm* column2 = gtForm::create(hbox, "form");
    column2->attach_lr(column1, NULL);
    column2->attach_offset(gtLeft, 10);
    column2->manage();

    const int INDENT = 190;
    const int FOLDUP_INDENT = 15;
    const int INDENT2 = INDENT - FOLDUP_INDENT;
    const int INDENT3 = 145;

    gtPrimitive* container = NULL;

    //////////////////////////////////////////////////////////////


    gtForm* frame3 = make_frame(column1, container);
    {
	gtPrimitive* container = NULL;

	gtBase* foldup = make_foldup(frame3, 0, container); 
	{
	    gtPrimitive* container = NULL;

	    gtPrimitive* c = container;
	    container = NULL;
	    ste_edit_mode = make_radio_box(foldup, INDENT, container, 
			     TXT("Editor Mode"),
                             TXT("Emacs"),    // Btn #0 = Emacs.
                             TXT("Vi"));      // Btn #1 = Vi.
	    container->attach(gtLeft, c, 0); 
	    container->set_sensitive (1);
	    
	}
	foldup->manage();
    }

    //////////////////////////////////////////////////////////////

    gtForm* frame3_5 = make_frame(column1, container);
    {
	gtPrimitive* container = NULL;

	
        enable_rtl_stats = make_toggle( "Enable RTL File Info", frame3_5, 
		container, NULL);
        enable_rtl_stats->set(customize::rtl_file_stats(), 1);
    }
     
    //////////////////////////////////////////////////////////////

    gtForm* frame3_6 = make_frame(column1, container);
    {
	gtPrimitive* container = NULL;

	miniBrowser_history = make_string_editor(
	    TXT("MiniBrowser History Length"), frame3_6, INDENT2, container);
	miniBrowser_history->columns(3);
	genString txt;
	txt.printf("%d", customize::miniBrowser_history_length());
	miniBrowser_history->text(txt);
	container->detach(gtRight);

	miniBrowser_show_titles = make_toggle("MiniBrowser List Titles", frame3_6,
					      container, NULL);
	miniBrowser_show_titles->set(customize::miniBrowser_show_titles(), 1);
    }
    
    //////////////////////////////////////////////////////////////

    reset();
}

void setupProps::reset()
{
    Initialize(setupProps::reset);

    //////////////////////////////////////////////////////////////
    int modeBtn = 0;    // Btn #0 = Emacs, Btn #1 = Vi.
    if (customize::editorMode() == customize::VI_EDITOR_MODE)
        modeBtn = 1;
    ste_edit_mode->item_set(modeBtn, true, true);

}
extern int gen_strcmp(const char* p1, const char* p2);  // should be in genString

#define ASSIGN(subr,gt)\
text = gt->text(); \
if(gen_strcmp(text,customize::subr()) != 0){\
customize::subr(text); modified = 1;} \
free(text);

void setupProps::store()
{
    Initialize(setupProps::store);

    char* text = NULL;

    //////////////////////////////////////////////////////////////

    if (enable_rtl_stats->set() != customize::rtl_file_stats() ) {
        customize::rtl_file_stats(enable_rtl_stats->set() );
		modified = true;
	}

    if (miniBrowser_show_titles->set() != customize::miniBrowser_show_titles() ){
        customize::miniBrowser_show_titles(miniBrowser_show_titles->set() );
		modified = true;
	}

    //////////////////////////////////////////////////////////////
    // item_set returns 1 if Emacs btn is set, 2 if Vi btn is set.
    int screenMode = customize::EMACS_EDITOR_MODE;
    if (ste_edit_mode->item_set() == 2)
        screenMode = customize::VI_EDITOR_MODE;
    if (customize::editorMode() != screenMode) {
        customize::editorMode(screenMode);
        modified = true;
    }
    //////////////////////////////////////////////////////////////
    
	bool  validText = true;
	int oldLen = customize::miniBrowser_history_length();
	int newLen = oldLen;
	char *history_len_txt = miniBrowser_history->text();

	// Make sure text can be converted into an integer.
	for(int i = strlen(history_len_txt) - 1; i >= 0; i--)
		if(!isdigit(history_len_txt[i])){
		    validText = false;
			msg("The MiniBrowser History Length must be an integer, not \'$1\'.", error_sev) << history_len_txt << eom;
			break;
		}
	if (validText) {
		newLen = OSapi_atoi(history_len_txt);
		if (newLen < 2) {
		    msg("The MiniBrowser History Length must be greater than 1", error_sev) << eom;
            newLen = oldLen;
		} else if (newLen > 30) {
		    msg("The MiniBrowser History Length must be less than 31", error_sev) << eom;
            newLen = oldLen;
		}
    }

	if (newLen != oldLen) {
		customize::miniBrowser_history_length(newLen);
		modified = true;
		free(history_len_txt);
	}
}

int setupProps::is_modified()
{
    return modified;
}

