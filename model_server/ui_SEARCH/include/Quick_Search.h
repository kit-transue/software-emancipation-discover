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
#ifndef _Quick_Search_h
#define _Quick_Search_h

// Quick_Search.h
//------------------------------------------
// synopsis:
// Quick_Search dialog in libGT
//------------------------------------------

#ifndef  _prototypes_h
#include <prototypes.h>
#endif
#ifndef  _gt_h
#include <gt.h>
#endif
#ifndef  _objArr_h
#include <objArr.h>
#endif
#ifndef  _Search_h
#include <Search.h>
#endif

BEGIN_C_DECL
void popup_quick_search ();
END_C_DECL

typedef struct
{
    int prev_constraints; // toggle button settings 
    void *prev_view; // the view on which the previous search was conducted
    char *prev_string;//the string with which the previous search was conducted
    int match_count; // number of matches found during last search
    int current_count; // current match count
} instance_data;

class Quick_Search : public Obj {
    
  public:
    Quick_Search();
    ~Quick_Search();

    static void update_search_dialogs(void *);
    static viewPtr curr_view;

  private:
    Search *search; // the Search object that does the real work here
    static objArr instance_list;
    gtLabel	*search_lab, *matches, *matches_label;
    instance_data	inst;		
    gtToggleBox		*tbox1, *tbox2;
    gtDialogTemplate	*shell;
    gtStringEditor	*search_text;	
    boolean last_attributes_setting;
    boolean last_enable_dialog;
    int theConstraints();
    boolean new_search_needed(boolean);
    int set_widgets();
    void view_changed();
    void init_search();
    void do_it(int);
    void set_match_count (int);

    static void Next_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Previous_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Done_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Case_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Match_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Constraint_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Changed_CB(gtStringEditor *, gtEvent *, void *cd, gtReason);
};

typedef Quick_Search* Quick_Search_Ptr;


/*
    START-LOG-------------------------------

   $Log: Quick_Search.h  $
   Revision 1.2 1994/12/06 10:43:39EST mg 
   Bug track: 8638
   reimplemented
 * Revision 1.2.1.11  1993/12/31  19:43:04  boris
 * Bug track: Test Validation
 * Test Validation
 *
 * Revision 1.2.1.10  1993/12/11  16:51:20  jon
 * Bug track: many
 * Fixed Quick_Search to work with the new search object.
 *
 * Revision 1.2.1.9  1993/11/24  20:38:57  azaparov
 * Fixed bug 5339
 *
 * Revision 1.2.1.8  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.7  1993/04/29  18:24:46  jon
 * Made the quick help dialog disable itself when a raw view is selected
 * (bug 3031)
 *
 * Revision 1.2.1.6  1993/02/10  23:02:15  jon
 * Constructor no longer needs a viewershell. Added static curr_view var.
 *
 * Revision 1.2.1.5  1993/01/29  15:35:08  smit
 * *** empty log message ***
 *
 * Revision 1.2.1.4  1993/01/21  15:04:45  jon
 * Fixed to work with new STE/S-Mode selection mech.
 *
 * Revision 1.2.1.3  1992/11/12  22:12:43  jon
 * Changed init_search to return number of matches.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:03  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _Quick_Search_h
