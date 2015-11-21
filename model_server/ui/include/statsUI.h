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
#ifndef _statsUI_h
#define _statsUI_h

#include <gt.h>
#include <projectBrowser.h>
#include <statistics.h>


class statsUI {
 protected:
    gtDialogTemplate * dlg;
    gtOptionMenu * output_sel;
    gtStringEditor * scope;
    gtStringEditor * criteria;
    gtStringEditor * cc_thresh;
    gtStringEditor * switch_thresh;
    gtStringEditor * output_file;

    gtToggleButton * file_rebuild_but;
    gtToggleButton * proj_rebuild_but;

    gtToggleButton * shallow_but;
    gtToggleButton * std_dev_but;
    gtToggleButton * proj_info_but;
    gtToggleButton * file_info_but;
    gtToggleButton * var_info_but;
    gtToggleButton * class_info_but;
    gtToggleButton * funct_info_but;
    gtToggleButton * temp_info_but;
    gtToggleButton * macro_info_but;
    gtToggleButton * enum_info_but;
    gtToggleButton * union_info_but;
    gtToggleButton * typedef_info_but;

    gtLabel * cc_thresh_lbl;
    gtLabel * switch_thresh_lbl;

    projectBrowser * pb;

    static int current_format;

  public:
    static void apply_CB(gtPushButton*, _XEvent*, void*, gtReason);
    static void cancel_CB(gtPushButton*, _XEvent*, void*, gtReason);
    static void save_set_CB(gtPushButton*, _XEvent*, void*, gtReason);
    static void load_set_CB(gtPushButton*, _XEvent*, void*, gtReason);
    static void output_sel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    void build_interface();

    static statsUI * instance;
    
    static int write_prefs_to_file(char* filename);
    static int read_prefs_from_file(char* filename);

    static void go(projectBrowser * browser);

    statsUI();
};

#endif









