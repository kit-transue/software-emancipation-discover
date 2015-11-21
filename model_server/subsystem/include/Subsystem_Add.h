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
#ifndef _Subsystem_Add_h
#define _Subsystem_Add_h

// Subsystem_Add.h
//------------------------------------------
// synopsis:
// Conversion of Subsystem_Add to GT
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#include <gtVertBox.h>
#include <gtTogB.h>
#include <gtPushButton.h>
#include <gtRadioBox.h>
#include <gtDlgTemplate.h>
#include <gtList.h>
#include "string.h"
#include "oodt_ui_decls.h"
#include "ui-globals.h"
#include <objRelation.h>

RelClass(groupHdr);
RelClass(viewSubsysNode);

extern "C" void popup_Subsystem_Add(void *, viewSubsysNodePtr);

class Subsystem_Add {
    gtVertBox 	*radio_slot;
    gtRadioBox	*radio1, *radio2;
    gtList	*list1;
    int 	containing;

  public:
    groupHdrPtr 	my_popup;
    void*		my_view_hdr;
    gtDialogTemplate	*shell;
    Subsystem_Add();
    void init_fields(viewSubsysNodePtr, void *);
    static void Subsystems_CB(gtToggleButton *tb, gtEvent *, void *cd, gtReason);
    static void Functions_CB(gtToggleButton *tb, gtEvent *, void *cd, gtReason);
    static void Classes_CB(gtToggleButton *tb, gtEvent *, void *cd, gtReason);
    static void Objects_CB(gtToggleButton *tb, gtEvent *, void *cd, gtReason);
    static void Containing_CB(gtToggleButton *tb, gtEvent *, void *cd, gtReason);
    static void Contained_CB(gtToggleButton *tb, gtEvent *, void *cd, gtReason);
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    void do_it();
    void fill_list(gtToggleButton *, void (*fcn)(struct ui_list *));
};

typedef Subsystem_Add* Subsystem_AddPtr;
#endif // _Subsystem_Add_h

/*
    START-LOG-------------------------------

    $Log: Subsystem_Add.h  $
    Revision 1.2 1995/08/26 09:32:53EDT wmm 
    New group/subsystem implementation
 * Revision 1.2.1.5  1993/01/28  16:18:09  wmm
 * Fix bug 2329.
 *
 * Revision 1.2.1.4  1992/12/28  19:26:40  wmm
 * Support new subsystem implementation.
 *
 * Revision 1.2.1.3  1992/10/22  11:37:15  oak
 * Added call backs to remember state of
 * "containing" radio button.
 *
 * Revision 1.2.1.2  92/10/09  19:56:26  kws
 * Fix comments
 * 

    END-LOG---------------------------------
*/

