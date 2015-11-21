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
#include <representation.h>
#include <fileLanguageDlg.h>
#include <gtDlgTemplate.h>
#include <gtPushButton.h>
#include <gtRadioBox.h>
#include <gtTogB.h>
#include <genError.h>

fileLanguageDlg::fileLanguageDlg(gtBase *parent)
{
    Initialize (fileLanguageDlg::fileLanguageDlg);
#ifndef NEW_UI
    dlg = gtDialogTemplate::create (parent, "file_language", "Set Language");

    dlg->add_button("ok", "OK", fileLanguageDlg::ok_CB, this);

    dlg->add_button("cancel", "Cancel", fileLanguageDlg::cancel_CB, this);
    dlg->add_help_button();

    box = gtRadioBox::create(
        dlg, "file_lang_box", "Language:",
        "C Source Code",   "file_lang_c",          NULL, NULL,
        "C++ Source Code", "file_lang_cplusplus",  NULL, NULL,
        "Raw Text",        "file_lang_raw",        NULL, NULL,
        "Unknown",         "file_lang_unknown",    NULL, NULL,
        NULL);
    box->manage();
#endif 
}

fileLanguageDlg::~fileLanguageDlg()
{
#ifndef NEW_UI
    dlg->popdown();
    delete dlg;
#endif
}

void fileLanguageDlg::ok_CB(gtPushButton*, gtEventPtr, void* data, gtReason)
{
    Initialize(fileLanguageDlg::ok_CB);
#ifndef NEW_UI 
    fileLanguageDlg* ftd = (fileLanguageDlg*)data;

    if(ftd->box->button("file_lang_c")->set())
	ftd->language = FILE_LANGUAGE_C;
    else if (ftd->box->button("file_lang_cplusplus")->set())
	ftd->language = FILE_LANGUAGE_CPP;
    else if(ftd->box->button("file_lang_raw")->set())
        ftd->language = FILE_LANGUAGE_RAW;
    else if(ftd->box->button("file_lang_unknown")->set())
        ftd->language = FILE_LANGUAGE_UNKNOWN;

    ftd->confirm = 1;
#endif
}

void fileLanguageDlg::cancel_CB(gtPushButton*, gtEventPtr, void* data, gtReason)
{
    Initialize(fileLanguageDlg::cancel_CB);
#ifndef NEW_UI 
    fileLanguageDlg* ftd = (fileLanguageDlg*)data;
    ftd->confirm = -1;
#endif
}

int fileLanguageDlg::confirm_func (void *cd)
{
    fileLanguageDlg* ftd = (fileLanguageDlg*)cd;

    return (ftd->confirm != 0);
}

fileLanguage fileLanguageDlg::get_language (gtBase *parent)
{
    Initialize (fileLanguageDlg::get_language);
    fileLanguage language = FILE_LANGUAGE_UNKNOWN;
#ifndef NEW_UI
    fileLanguageDlg *dlg = new fileLanguageDlg(parent);

    dlg->dlg->popup(0);

    dlg->confirm = 0;

    gtBase::take_control(&fileLanguageDlg::confirm_func, dlg);

    if (dlg->confirm == 1)
	language = dlg->language;

    delete dlg;

#endif

    return language;
}
