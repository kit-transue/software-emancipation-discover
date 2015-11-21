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
// FILE: getUI.h.C
// Implements a dialog box asking for either only a comment, or options and a 
// comment (for a get command).
//
 
#include <gtDlgTemplate.h>
#include <gtPushButton.h>
#include <gtForm.h>
#include <gtLabel.h>
#include <gtSepar.h>
#include <gtTextEd.h>
#include <gtStringEd.h>
#include <getUI.h>
#include <genError.h>
#include <systemMessages.h>
#include <gtPushButton.h>
#include <genString.h>

genString getUI::comment_text;

getUI::getUI(void) {
  Initialize(getUI::getUI);
  
  build_interface(0);
}

getUI::getUI(int show_opts) {
  Initialize(getUI::getUI);
  
  build_interface(show_opts);
}

getUI::~getUI() {
    Initialize(getUI::~getUI);
}

void getUI::build_interface(int show_opts) {
  Initialize(getUI::build_interface);
  
  shell = gtDialogTemplate::create(NULL, "get_dlg", TXT("Comments for Get"));
  
  shell->add_button("ok", "OK", ok_CB, this);
  shell->add_button("cancel", "Cancel", cancel_CB, this);
  gtPushButton::next_help_context("Browser.Manage.Get.WithOptions.Help");
  shell->add_help_button();
  
  gtForm* comment_form = gtForm::create(shell, "comment_form");

  if (show_opts) {

    gtForm* options_form = gtForm::create(shell, "options_form");
    options_form->attach(gtTop);
    options_form->attach(gtLeft);
    options_form->attach(gtRight);
    options_form->manage();
    
    gtLabel* options_label = gtLabel::create(options_form, "label", 
					     TXT("Enter options for Get command:"));
    options_label->attach(gtTop);
    options_label->attach(gtLeft);
    options_label->manage();
    
    options = gtStringEditor::create(options_form, "options", NULL);
    options->attach(gtTop);
    options->attach(gtRight);
    options->attach(gtLeft, options_label, 10);
    options->manage();

    gtSeparator* sep = gtSeparator::create(shell, "separator", gtHORZ);
    sep->attach(gtTop, options_form, 5);
    sep->attach(gtLeft);
    sep->attach(gtRight);
    sep->manage();
    
    comment_form->attach(gtTop, sep, 5);
  } else comment_form->attach(gtTop);

  comment_form->attach(gtLeft);
  comment_form->attach(gtRight);
  comment_form->manage();

  gtLabel* comment_label = gtLabel::create(comment_form, "label",
		   TXT("Enter comments:"));
  comment_label->attach(gtTop);
  comment_label->attach(gtLeft);
  comment_label->manage();

  gtPushButton* clear_but = gtPushButton::create(comment_form, "clear", TXT("Clear"),
						 clear_CB, this, gtCenter);
  clear_but->attach(gtTop);
  clear_but->attach(gtRight);
  clear_but->set_sensitive(1);
  clear_but->manage();

  comment = gtTextEditor::create(comment_form, "comment", NULL);
  comment -> attach(gtTop, clear_but, 5);
  comment -> attach(gtLeft);
  comment -> attach(gtRight);
  comment -> num_rows(10);
  comment -> num_columns(30);
  if (comment_text.str()) comment -> text(comment_text);
  comment -> manage();  

}

void getUI::ok_CB(gtPushButton *, gtEventPtr, void *data, gtReason) {
  Initialize(getUI::ok_CB);
  
  comment_text = ((getUI *)data)->comment->text();

  ((getUI *)data)->done = 1;
}

void getUI::cancel_CB(gtPushButton *, gtEventPtr, void *data, gtReason) {
  Initialize(getUI::cancel_CB);

  ((getUI *)data)->done = -1;
} 

void getUI::clear_CB(gtPushButton *, gtEventPtr, void *data, gtReason) {
  Initialize(getUI::clear_CB);
  
  getUI* This = (getUI*) data;

  This->comment->text(0);
  This->comment->flush();
  This->comment->update_display();
}

int getUI::take_control_CB(void *data) {
  Initialize(getUI::take_control_CB);

  return ((getUI *)data)->done;
}


int getUI::ask(genString &comm) {
  Initialize(getUI::ask);
  
  if (comm.str()) comment->text(comm);
  done = 0;

  shell->popup(3);
  shell->take_control(&getUI::take_control_CB, this);
  shell->popdown();
  
  if (done>0) {
    comm = comment->text();
    comm.r_trim();
  }
  return done;
}
