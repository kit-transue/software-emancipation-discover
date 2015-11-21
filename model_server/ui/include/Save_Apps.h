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
#ifndef _Save_Apps_h
#define _Save_Apps_h

// Save_Apps.h
//------------------------------------------
// synopsis:
//  define the offer_to_save dialog box
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

enum dlgFlavor
{
    Flavor_Quitting,
    Flavor_Saving,
    Flavor_Unloading,
    Flavor_Putting,
    Flavor_ViewerClosing,
    Flavor_Parsing,
    Flavor_Hiding,
    Flavor_Browsing_Groups
};

class SaveDialog;
typedef int (*SaveDialogCB)(SaveDialog*, void* obj_array, int* pos, int cnt);

class SaveDialog
{
  public:
    gtDialogTemplate 	*shell;
    
    SaveDialog(gtBase* parent, void* obj_array, dlgFlavor flavor,
	       SaveDialogCB select, SaveDialogCB save, SaveDialogCB cleanup,
	       void (*ok)(void*), void* ok_data, SaveDialogCB notsave);
    virtual ~SaveDialog();

    void add_modified(const char *);
    void clear_to_be_saved();
    void add_to_be_saved(const char *);
    int popup(bool wait);
    dlgFlavor   get_flavor() { return our_flavor; }
    int prepare_for_update_perm_pmod(int *selected, int count,
				     int restore_not_sel);
    static int state;
  protected:
    static void ok_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void save_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void name_CB(gtList*, gtEventPtr, void*, gtReason);
    static int check_state(void*);

    SaveDialogCB client_select, client_save, client_cleanup;
    void	(*client_ok)(void*);
    void*       client_ok_data;
    void*	modified;
    dlgFlavor   our_flavor;
    gtForm*     name_form;
    gtForm*     depend_form;
    gtLabel*    name_label;
    gtLabel*    depend_label;
    gtList*	name_list;
    gtList*	depend_list;
    SaveDialogCB client_notsave; // call when hitting unload without save
    void call_client_select();
    int  save_selected_items();
    int  notsave_selected_items();
};

#endif // _Save_Apps_h

/*
    START-LOG-------------------------------

    $Log: Save_Apps.h  $
    Revision 1.4 1995/12/15 08:12:52EST wmm 
    Bug track: N/A
    Fix bugs 11345, 11352, 11353, 11360.
 * Revision 1.2.1.12  1994/07/11  17:17:00  so
 * Bug track: n/a
 * project for re-load pdf
 *
 * Revision 1.2.1.11  1994/03/09  17:08:37  trung
 * Bug track: 6597
 * unload all files belong to projects being hided
 *
 * Revision 1.2.1.10  1994/01/31  15:55:59  trung
 * Bug track: 0
 * saving xref processing
 *
 * Revision 1.2.1.9  1993/10/28  20:28:47  trung
 * Bug track: 4993
 * make unload without save to copy pset from permanent xref to memory
 *
 * Revision 1.2.1.8  1993/04/30  01:34:55  kws
 * Added more flavors
 *
 * Revision 1.2.1.7  1993/04/02  13:47:09  kws
 * Added Flavor_Putting
 *
 * Revision 1.2.1.6  1993/02/15  18:54:40  jon
 * Added name_form, depend_form, name_label, depend_label
 *
 * Revision 1.2.1.5  1993/02/11  01:31:53  jon
 * Changed SaveDialogCB typedef and save_selected_items to return
 * a success/fail value.  (part of bug 2436)
 *
 * Revision 1.2.1.4  1993/02/10  01:00:19  jon
 * Added flavor and ok_data parameters to constructor. (part of bug 2379)
 *
 * Revision 1.2.1.3  1992/11/01  18:59:29  sharris
 * made destructor virtual; made private members protected so I could use as \
 * base class for CommitDialog
 *
 * Revision 1.2.1.2  1992/10/09  19:56:14  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/
