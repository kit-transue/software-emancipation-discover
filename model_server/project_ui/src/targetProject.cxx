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
#include <systemMessages.h>
#include <genError.h>
#include <genString.h>
#include <targetProject.h>
#include <psetmem.h>
#include <machdep.h>

#define NUM_ROWS 3

targetProject::targetProject(projModule *m, objArr &projs, int force_display) : mod(m)
{
    Initialize(targetProject::targetProject);

    proj_list_len = projs.size();
    proj_list = (char **)psetmalloc(sizeof(char *) * proj_list_len);
    proj      = (projNode **)psetmalloc(sizeof(projNode *) * proj_list_len);
    for(int i=0;i<proj_list_len;i++)
    {
        proj[i] = checked_cast(projNode, projs[i]);
        proj_list[i] = strdup(proj[i]->get_ln());
    }

    genString fn;
    mod->get_phys_filename(fn);
    build_interface(fn, force_display);
    ans = 0;
    dlg->popup(1);
    dlg->take_control_top(answer_func, this);
}

targetProject::~targetProject()
{
    Initialize(targetProject::~targetProject);

    for (int i=0;i<proj_list_len;i++)
        psetfree(proj_list[i]);

    psetfree(proj_list);
    psetfree(proj);
}

int targetProject::answer_func(void* cd)
{
    Initialize(TargetProject::answer_func);

    return ((targetProject *)cd)->ans;
}

void targetProject::build_interface(const char *fn, int force_display)
{
    Initialize(targetProject::build_interface);

    dlg = gtDialogTemplate::create(NULL, "targetProject", "Show Target Project");

    dlg->add_button("ok", TXT("OK"), ok_CB, this);
    dlg->add_button("cancel", TXT("Cancel"), cancel_CB, this);
    if (!force_display)
        dlg->button("cancel")->set_sensitive(0);
    dlg->add_help_button();
    dlg->help_context_name("Pset.Help.Config.SelectTarget");
    dlg->default_button(0);
    dlg->manage();

//-----------------------------------------------------------------

    target_select_form = gtForm::create(dlg, "target_select_form");
    target_select_form->attach(gtTop);
    target_select_form->attach(gtBottom);
    target_select_form->attach(gtLeft);
    target_select_form->attach(gtRight);
    target_select_form->manage();

//----------------------------------------------------------------

    // set up title
    const char *dub;
    if (proj_list_len == 1)
        dub = "";
    else
        dub = "s";
    projNode *tp = mod->get_target_project(-1);
    genString ts = "";
    if (tp)
        ts.printf(TXT("The selected target project is \"%s\"."),tp->get_ln());
    genString title;
    title.printf(TXT("Show target project%s for module\n%s\n%s\n"), dub, fn, ts.str());

    select_target_list = gtList::create((gtBase *)target_select_form, (const char *)"select_target_list",
                                        (const char *)title, (gtListStyle)gtSingle, (const char **)proj_list, (int)proj_list_len);
    select_target_list->attach(gtLeft);
    select_target_list->attach(gtRight);
    select_target_list->attach(gtTop);
    select_target_list->attach(gtBottom);
    select_target_list->num_rows(NUM_ROWS);
    select_target_list->manage();
}

void targetProject::cancel_CB(gtPushButton *, XEvent *, void *data, gtReason)
{
    Initialize(targetProject::cancel_CB);

    targetProject* gfs = (targetProject*)data;
    gfs->get_dlg()->popdown();
    gfs->set_ans(-1);
}

void targetProject::ok_CB(gtPushButton *, XEvent *, void *data, gtReason)
{
    Initialize(targetProject::ok_CB);

    targetProject* gfs = (targetProject*)data;
    char** selected_items = gfs->select_target_list->selected();
    genString proj_name = selected_items[0];
    psetfree(selected_items);

    int len = gfs->get_proj_list_len();

    if (!proj_name.length() && len > 1)
    {
        gtPushButton::next_help_context("Pset.Help.Config.SelectOneProject");
        msg("ERROR: Please select one project before selecting the OK button.") << eom;
        return;
    }

    if (proj_name.length())
        psetfree(selected_items[0]);

    if (len > 1)
    {
        projNode *pn = 0;
        char **proj_name_list = gfs->get_proj_list();
        projNode **projs = gfs->get_proj();
        for (int i=0;i<len;i++)
        {
            if (strcmp(proj_name_list[i], proj_name) == 0)
            {
                pn = projs[i];
                break;
            }
        }
        Assert(pn);
        gfs->get_module()->set_target_project(pn);
    }

    gfs->get_dlg()->popdown();
    gfs->set_ans(1);
}

