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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <gtPushButton.h>
#include <gtForm.h>
#include <gtDlgTemplate.h>
#include <gtList.h>
#include <messages.h>
#include <genError.h>
#include <genString.h>
#include <getFileSelector.h>
#include <psetmem.h>

//#define TXT(a) a

#define NUM_ROWS 8

getFileSelector::getFileSelector(genString &date, projModule *mod, getType type)
{
    Initialize(getFileSelector::getFileSelector);

    module = 0; // initialized
    ver_list = mod->get_version_list(date, ver_list_len);

    if (ver_list == 0)
        return;

    module = mod;
    build_interface(type);
    dlg->popup(0);
}

getFileSelector::~getFileSelector()
{
    Initialize(getFileSelector::~getFileSelector);

    for (int i=0;i<ver_list_len;i++)
        psetfree(ver_list[i]);

    psetfree(ver_list);
}

void getFileSelector::build_interface(getType type)
{
    Initialize(getFileSelector::build_interface);

    genString title;
    if (type == GET_BY_VERSION)
        title = TXT("Copy by Version");
    else if (type == GET_BY_DATE)
        title = TXT("Copy by Date");

    dlg = gtDialogTemplate::create(NULL, "get_file_selector", (const char*)title);

    dlg->add_button("ok", TXT("OK"), ok_CB, this);
    dlg->add_button("cancel", TXT("Cancel"), cancel_CB, this);
    dlg->add_help_button();
    dlg->help_context_name("Pset.Help.Get.FileSelector");
    dlg->default_button(0);
    dlg->manage();

//-----------------------------------------------------------------

    file_select_form = gtForm::create(dlg, "file_select_form");
    file_select_form->attach(gtTop);
    file_select_form->attach(gtBottom);
    file_select_form->attach(gtLeft);
    file_select_form->attach(gtRight);
    file_select_form->manage();

//----------------------------------------------------------------

    select_file_list = gtList::create((gtBase *)file_select_form, (const char *)"select_file_list",
                                      (const char *)"Select a file", (gtListStyle)gtSingle, (const char **)ver_list, (int)ver_list_len);
    select_file_list->attach(gtLeft);
    select_file_list->attach(gtRight);
    select_file_list->attach(gtTop);
    select_file_list->attach(gtBottom);
    select_file_list->num_rows(NUM_ROWS);
    select_file_list->manage();
}

void getFileSelector::cancel_CB(gtPushButton *, XEvent *, void *data, gtReason)
{
    Initialize(getFileSelector::cancel_CB);

    getFileSelector* gfs = (getFileSelector*)data;
    gfs->get_dlg()->popdown();
}

void getFileSelector::ok_CB(gtPushButton *, XEvent *, void *data, gtReason)
{
    Initialize(getFileSelector::ok_CB);

    getFileSelector* gfs = (getFileSelector*)data;
    char** selected_items = gfs->select_file_list->selected();
    genString ver = selected_items[0];

    psetfree(selected_items);

    if (!ver.length())
    {
	msg("ERROR: Please select one version before selecting the OK button.") << eom;
	return;
    }
    psetfree(selected_items[0]);

    // New code suppose to work for clearCase too
    // looking for space, not only digits
    char *p = strchr(ver, ' ');
    if(p)
	*p = '\0';

    gfs->module->get_prev_module(ver);

    gfs->get_dlg()->popdown();
}

