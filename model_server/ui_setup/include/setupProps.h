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
#ifndef _setupProps_h
#define _setupProps_h

// setupProps.h
//------------------------------------------
// synopsis:
// Initial setup screen
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

class setupProps
{
  public:
    static setupProps* instance();
    void popup();
    int is_modified();
    static void widget_destroy(gtBase *dlg, void *data);

  private:
    gtDialogTemplate* dlg;

    gtToggleButton* enable_rtl_stats;
    
    gtStringEditor* miniBrowser_history;
    gtToggleButton* miniBrowser_show_titles;
    gtRadioBox*     ste_edit_mode;

    int modified;

    static void ok_button(gtPushButton*, gtEvent*, void*, gtReason);
    static void cancel_button(gtPushButton*, gtEvent*, void*, gtReason);

    setupProps();
    ~setupProps();

    void build_interface();
    void store();
    void reset();
};

#endif // _setupProps_h

