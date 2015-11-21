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
#ifndef _Browser_Display_Selection_h
#define _Browser_Display_Selection_h

// Browser_Display_Selection.h
//------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif
#ifndef _gtTogB_h
#include <gtTogB.h>
#endif
#ifndef _gtHorzBox_h
#include <gtHorzBox.h>
#endif
#ifndef _gtLabel_h
#include <gtLabel.h>
#endif
#ifndef _gtStringEd_h
#include <gtStringEd.h>
#endif
#ifndef _gtPushButton_h
#include <gtPushButton.h>
#endif
#ifndef _gtToggleBox_h
#include <gtToggleBox.h>
#endif
#ifndef _oodt_ui_decls_h
#include <oodt_ui_decls.h>
#endif

#ifndef __viewGraHeader_h
#include <viewGraHeader.h>
#endif

#ifndef _psetNotifyView_h
#include <psetNotifyView.h>
#endif


BEGIN_C_DECL
void popup_Browser_Display_Selection PROTO((void *arg_viewp, void *popup_node));
END_C_DECL

class Browser_Display_Selection {
    gtLabel	*toplabel;
    gtHorzBox	*togslot, *togslot2, *filtslot;
    gtToggleBox	*tog1, *tog2, *tog3;
    gtToggleButton	*tb1, *tb2, *tb3;
    gtStringEditor	*text1;
    void       *popup_node, *viewp;
    psetNotifyView* notifier;

  public:
    gtDialogTemplate	*shell;
    Browser_Display_Selection(viewGraHeader* viewp);//argument distinguishes caller/type of view.
    void init(void *, void *);
    static void sanity_CB(gtToggleButton *tb, gtEvent *, void *cd, gtReason);
    static void OK_CB(gtPushButton *tb, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void text_CB(gtStringEditor *, gtEvent *, void *cd, gtReason);
	static void view_deleted_CB(void*, psetNotifyView*);
    void sanity_check();
    void get_current_options();
    void do_it();
};

typedef Browser_Display_Selection* Browser_Display_Selection_Ptr;

#endif // _Browser_Display_Selection_h
