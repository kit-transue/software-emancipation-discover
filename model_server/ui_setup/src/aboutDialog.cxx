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
// aboutDialog.C
//------------------------------------------
// synopsis:
// Initial "splash screen" during startup
//
//------------------------------------------


// INCLUDE FILES
#include <top_widgets.h>
#include <gtDlgTemplate.h>
#include <gtForm.h>
#include <gtFrame.h>
#include <gtLabel.h>
#include <gtVertBox.h>
#include <gtPixmap.h>
#include <gtPushButton.h>

#include <systemMessages.h>
#include <customize.h>
#include <aboutDialog.h>
#include <genString.h>
#include <genError.h>

#include "about.xpm"

// VARIABLE DECLARATIONS

extern "C" char const build_date[];
extern "C" char const build_version[];
extern "C" char const build_full_version[];

// VARIABLE DEFINITIONS

static aboutDialog* the_aboutDialog = 0;

// FUNCTION DEFINITIONS

aboutDialog* aboutDialog::instance()
{
    Initialize(aboutDialog::instance);

    if(the_aboutDialog == NULL)
	new aboutDialog();

    return the_aboutDialog;
}

aboutDialog::aboutDialog()
{
    Initialize(aboutDialog::aboutDialog);

    the_aboutDialog = this;
    build_interface();
}

aboutDialog::~aboutDialog()
{
    Initialize(aboutDialog::~aboutDialog);

    if(the_aboutDialog == this)
	the_aboutDialog = NULL;

    delete shell;
}

void aboutDialog::popup()
{
    Initialize(aboutDialog::popup);

    shell->popup(0);
    shell->bring_to_top();
}

void aboutDialog::ok_CB(gtPushButton*, gtEventPtr, void* ss_data, gtReason)
{
    Initialize(aboutDialog::ok_CB);
    aboutDialog* ss = (aboutDialog*)ss_data;

    ss->shell->popdown();
}

void aboutDialog::build_interface()
{
    Initialize(aboutDialog::build_interface);

    const double MARGIN = 25.0;

    shell = gtDialogTemplate::create(
	NULL, "about_ParaSET", TXT("About ParaSET"), MARGIN, MARGIN);

    shell->add_button("ok", TXT("OK"), ok_CB, this);

    gtPushButton* ok_button = shell->button((unsigned int)0);
    ok_button->attach_pos(gtLeft, 85);
    ok_button->attach_pos(gtRight, 98);

    gtFrame* frame = gtFrame::create(shell, "frame", gtETCHED_IN, 2, 0);
    frame->margins(10, 10);
    frame->attach_tblr();
    frame->manage();

    gtForm* form = gtForm::create(frame, "form");
    form->attach_tblr();
    form->width(440);
    form->manage();

// More of the about_logo
    gtPixmap* logo = gtPixmap::create(form, "logo", (char const **) about_xpm, 150 , 125);
    logo->alignment(gtBeginning);
    logo->attach(gtLeft);
    logo->manage();

    genString install_dir_string;
    install_dir_string.printf(gettext(TXT("Installed in: %s")),
			     customize::install_root());
    gtLabel* install_dir = gtLabel::create(
	form, "install_dir", install_dir_string);
    install_dir->manage();
    install_dir->alignment(gtBeginning);

    genString build_date_string;
    build_date_string.printf("DIScover\nVersion %s\nBuild: %s  built on: %.6s %.4s %.5s",
	 build_version, build_full_version, build_date + 4, build_date + 24, build_date + 11);

    gtLabel* build_date = gtLabel::create(
	form, "build_date", build_date_string);
    build_date->manage();
    build_date->alignment(gtBeginning);

    gtLabel* legal_notice = gtLabel::create(
	form, "legal_notice",
#include "legal.h"
	);
    legal_notice->manage();
    legal_notice->alignment(gtBeginning);

    gtForm::vertStack(
	logo, build_date, install_dir, legal_notice);
}
