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
#ifndef _setupConfig_h
#define _setupConfig_h

// setupConfig.h
//------------------------------------------
// synopsis:
// Initial setup screen 
// Defines class for the dialog popup that allows user to enter auxillary command
// data to be passed to the configurator.
//------------------------------------------

#ifndef  _gt_h
#include <gt.h>
#endif

#ifndef  _genString_h
#include <genString.h>
#endif

class setupConfig
{
  public:
    static setupConfig* instance();
    void popup();
    int is_modified();
    static void widget_destroy(gtBase *dlg, void *data);

  private:
    gtDialogTemplate* dialog;
    
    gtToggleButton* typ_rcs;
    gtToggleButton* typ_sccs;
    gtToggleButton* typ_clearcase;
    gtToggleButton* typ_continuus;
    gtToggleButton* typ_cvs;
    gtToggleButton* typ_no_cm;
    gtToggleButton* typ_custom;
    gtLabel*        typ_custom_dir_label;
    gtStringEditor* typ_custom_dir;
    gtToggleButton* gen_track;
    gtToggleButton* gen_lock;
    gtToggleButton* put_lock;
    gtToggleButton* gen_softlinks;
    gtToggleButton* gen_dobackups;
    gtToggleButton* gen_rmpsets;
    gtToggleButton* gen_fast_putmerge;
    gtToggleButton* gen_get_comments;
    gtStringEditor* mis_optscripts_dir;
    gtStringEditor* mis_configurator;
    gtStringEditor* opt_get;
    gtStringEditor* opt_put;
    gtStringEditor* opt_copy;
    gtStringEditor* opt_lock;
    gtStringEditor* opt_unlock;
    gtStringEditor* opt_diff;
    gtStringEditor* opt_stat;
    gtTextEditor*   mak_targets;

    int       custom_dir_is_enabled;
    genString custom_dir_value;

    int       get_comments_is_enabled;
    int       get_comments_is_set;
    int       put_fast_is_enabled;

    int modified;

    setupConfig();
    ~setupConfig();

    void build_interface();
    gtDialogTemplate* build_interface_dialog();
    gtPrimitive* build_interface_title(gtBase* parent);
    gtPrimitive* build_interface_type(gtBase* parent);
    gtPrimitive* build_interface_general(gtBase* parent);
    gtPrimitive* build_interface_misc(gtBase* parent);
    gtPrimitive* build_interface_options(gtBase* parent);
    gtPrimitive* build_interface_targets(gtBase* parent);

    void ok();
    void cancel();
    void store();
    void reset();
    void save();
    void type_toggle(gtToggleButton*);
 
    static void type_toggle_CB(gtToggleButton*, gtEvent*, void*, gtReason);
    static void ok_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void cancel_CB(gtPushButton*, gtEvent*, void*, gtReason);
};

#endif // _setupConfig_h
