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
#ifndef _targetProject_h
#define _targetProject_h

#include <proj.h>

class targetProject
{
  public:
    targetProject(projModule *mod, objArr &projs, int force_display);
    ~targetProject();
    projModule *get_module() { return mod; }
    gtDialogTemplate *get_dlg() { return dlg; }
    int get_proj_list_len() { return proj_list_len; }
    projNode **get_proj() { return proj; }
    char **get_proj_list() { return proj_list; }
    void set_ans(int val) { ans = val; }

  private:
    gtForm *target_select_form;
    gtList *select_target_list;
    gtDialogTemplate* dlg;

    char **proj_list;
    projNode **proj;
    int  proj_list_len;
    projModule *mod;
    int ans;

    static void cancel_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void ok_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static int answer_func(void*);
    void build_interface(const char *fn, int force_display);
};
#endif

/*
$Log: targetProject.h  $
Revision 1.1 1994/05/12 15:02:25EDT builder 
made from unix file
 * Revision 1.1  1994/05/12  16:29:03  so
 * Initial revision
 *
*/
