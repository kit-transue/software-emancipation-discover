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
#ifndef _Subsys_Member_Sorting_Order_h
#define _Subsys_Member_Sorting_Order_h

// Subsys_Member_Sorting_Order.h
//------------------------------------------
// synopsis:
// ...
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#ifndef prototypes_h
#include "prototypes.h"
#endif

#include <gtDlgTemplate.h>
#include <gtRadioBox.h>
#include <gtTogB.h>
#include <gtList.h>
#include <gtLabel.h>
#include <gtVertBox.h>
#include "oodt_ui_decls.h"
#ifndef ISO_CPP_HEADERS
#include "string.h"
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */

BEGIN_C_DECL
void popup_Subsys_Member_Sorting_Order PROTO((void *, void *));
END_C_DECL

class Subsys_Member_Sorting_Order {
    gtList		*sel_list;
    gtRadioBox		*rbox;
    gtVertBox		*boxslot;
    gtLabel		*label1;
    int 	map_array[4];
    int		map_count;
    int 	sort_type;
    int		min_map_index;
    
  public:
    gtDialogTemplate	*shell;
    void		*i_popup_node, *i_viewp;
    int			sort_depth;

    Subsys_Member_Sorting_Order();
    void init_fields(void *, void *);
    void fill_selection_box();
    void init_selection_box(int, int, int);
    void clear_fields();
    void clear_pane_if_none();
    void do_it();
    void process_selection();
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Reset_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void List_CB(gtList *, gtEvent *, void *cd, gtReason);
    static void Type_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Ascii_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Declaration_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
};

typedef Subsys_Member_Sorting_Order* Subsys_Member_Sorting_Order_Ptr;

#endif // _Subsys_Member_Sorting_Order_h

/*
    START-LOG-------------------------------

    $Log: Subsys_Member_Sorting_Order.h  $
    Revision 1.3 2000/07/10 23:11:00EDT ktrans 
    mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2.1.3  1993/01/28  16:18:09  wmm
 * Fix bug 2329.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:22  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

