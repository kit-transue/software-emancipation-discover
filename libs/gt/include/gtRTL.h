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
#ifndef _gtRTL_h
#define _gtRTL_h

//------------------------------------------
// gtRTL.h
//------------------------------------------
// synopsis:
// View of a run-time list
//
//------------------------------------------

#ifndef _Relational_h
#include <Relational.h>
#endif
#ifndef _gtPrim_h
#include <gtPrim.h>
#endif
#ifndef _gtList_h
#include <gtList.h>
#endif
#ifndef _genString_h
#include <genString.h>
#endif


struct ui_list;
class objArr;
class symbolArr;
class symbolPtr;
RelClass(RTL);
RelClass(appTree);
RelClass(dialog);


class gtRTL : public gtPrimitive
{
    gtList*	list;
    dialogPtr	dlg;
    ui_list*	ui_list_data;
    RTLPtr 	rtl;
    int         handle_buttons;
    gtListStyle ltype;

  private:
    genString sort_spec;
    genString format_spec;
    genString show_filter;
    genString hide_filter;
    long      old_background;

  public:
    static gtRTL *create(
	gtBase *parent, const char *name, const char *title,
	gtListStyle ltype, const char **entries, int num_entries)
    { return new gtRTL(parent, name, title, ltype, entries, num_entries); }

    static void util_CB(gtPushButton *,gtEvent *,void *,gtReason) ;
    static void reset_CB(gtPushButton *,gtEvent *,void *,gtReason) ;
    static void update_CB(gtList *,gtEvent *,void *,gtReason) ;

    ~gtRTL();

    static int get_filtered_color();

    void update_status();

    char **selected() { return list->selected(); }
    int num_selected() { return list->num_selected(); }

    // adds item at location loc
    void add_item(const char *item, int loc) { list->add_item(item, loc); }

    // like add_item, but the new entry is unselected
    void add_item_unselected(const char *item, int loc) { list->add_item_unselected(item, loc); }

    // add items at location loc
    void add_items(const char **items, int num_items, int loc) { list->add_items(items, num_items, loc); }

    // removes all items from the gtList
    void delete_all_items() { list->delete_all_items(); }

    // removes the item at pos from the gtList
    void delete_pos(int pos) { list->delete_pos(pos); }

    // clears all selections
    void deselect_all() { list->deselect_all(); }

    // clears the selection at location pos
    void deselect_pos(int pos) { list->deselect_pos(pos); }

    // returns via its arguments the positions of all of the selections.
    void get_selected_pos(int **pos_list, int *pos_count) { list->get_selected_pos(pos_list, pos_count); }

    // returns the location of item
    int  item_pos(const char *item) { return list->item_pos(item); }

    // sets a selection on item, and if notify is set will call appropriate callbacks
    void select_item(const char *item, int notify) { list->select_item(item, notify); }
    
    // like select_item, only at a location
    void select_pos(int post, int notify) { list->select_pos(post, notify); }

    // sets the top or bottom item of the list
    void set_bottom_item(const char *item) { list->set_bottom_item(item); }
    void set_top_item(const char *item) { list->set_top_item(item); }

    // makes n of the rows at any one time
    void num_rows(int n) { list->num_rows(n); }

    // performs appropriate ui parsing functions on the list
    void ui_parse(struct ui_list *l) { list->ui_parse(l); }

    // set a callback to be triggered when the List is activated
    void set_callback(gtListCB callback, void *cd) { list->set_callback(callback, cd); }

    // set a callback to be triggered when the List is activated
    void select_callback(gtListCB callback, void *cd)
    { list->select_callback(callback, cd); }

    // set a callback to be triggered when the List is activated
    void action_callback(gtListCB callback, void *cd)
    { list->action_callback(callback, cd); }

    // scroll the list so that the item at location pos is at the top of the
    // list.
    void set_pos(int pos) { list->set_pos(pos); }

    // returns the position of the top item in the list
    int top_item_position() { return list->top_item_position(); }

    // returns the number of visible items in the list
    int num_visible() { return list->num_visible(); }
    
    // set the associated RTL of the list
    void set_rtl(RTLPtr, int sort_order = 1);
    void set_rtl(symbolArr &, int sort_order = 1);
    void set_rtl_reset_filters(RTLPtr);

    // force to replace rtl
    void regenerate_rtl (RTLPtr, int sort_order = 1, int reset_filters = 0);

    void set_format (char* fmt);
    void set_sort (char* srt);

    // return an objArr with the contents of the list
    objArr* app_nodes(objArr*);
        symbolArr* app_nodes(symbolArr*);

    // sets the selection type of the list
    void sel_type(gtListStyle sty) { list->sel_type(sty); }

    // returns the RTL of the gtRTL
    RTLPtr RTL() { return rtl; }

    gtRep* rep() { return list->rep(); }

    void manage() { list->manage(); }
    void remanage() { list->remanage(); }
    void unmanage() { list->unmanage(); }

    void attach(gtCardinalDir dir, gtBase *attachee = NULL,
		int offset = 0, int opposite = 0)
    { list->attach(dir, attachee, offset, opposite); }

    void attach_tblr(gtBase* T=0, gtBase* B=0, gtBase* L=0, gtBase* R=0)
    { list->attach_tblr(T,B,L,R); }

    void attach_tb(gtBase* T=0, gtBase* B=0) { list->attach_tb(T,B); }
    void attach_lr(gtBase* L=0, gtBase* R=0) { list->attach_lr(L,R); }

    void attach_pos(gtCardinalDir dir, int pos) { list->attach_pos(dir, pos); }
    void detach(gtCardinalDir dir) { list->detach(dir); }
    void attach_offset(gtCardinalDir dir, int offset)
    { list->attach_offset(dir, offset); }

    // These 4 functions attach the primitives directly to other 
    // primitives.
    void attach_top(gtBase *attachee) { attach(gtDirUp, attachee); }
    void attach_bottom(gtBase *attachee) { attach(gtDirDown, attachee); }
    void attach_left(gtBase *attachee) { attach(gtDirLeft, attachee); }
    void attach_right(gtBase *attachee) { attach(gtDirRight, attachee); }

    // These 4 connect the primitive to the sides of the form that is 
    // its parent.
    void attach_side_top() { attach(gtDirUp); }
    void attach_side_bottom() { attach(gtDirDown); }
    void attach_side_left() { attach(gtDirLeft); }
    void attach_side_right() { attach(gtDirRight); }

    // These 4 functions specify the primitives location in terms of
    // percentage of the total distance along that dimension.
    void attach_pos_top(int pos) { attach_pos(gtDirUp, pos); }
    void attach_pos_bottom(int pos) { attach_pos(gtDirDown, pos); }
    void attach_pos_left(int pos) { attach_pos(gtDirLeft, pos); }
    void attach_pos_right(int pos) { attach_pos(gtDirRight, pos); }

    // These 4 functions set the offset of distance between the object
    // and its neighbor in a given direction
    void attach_offset_top(int off) { attach_offset(gtDirUp, off); }
    void attach_offset_bottom(int off) { attach_offset(gtDirDown, off); }
    void attach_offset_left(int off) { attach_offset(gtDirLeft, off); }
    void attach_offset_right(int off) { attach_offset(gtDirRight, off); }

    // return the item position which triggered the most recent CB.
    int get_item_last_position();

    gtList *gt_list() { return list; }

    void dialog_create_window(dialog *dlg, int n);

    // returns the total number of items in the list
    int num_items() { return list->num_items(); }

    void dlg_create_window_instance(dialog *dlg, int n);

    // sets the sensitivity of the list
    void set_sensitive(int sens) { list->set_sensitive(sens); }

    dialogPtr dial() { return dlg; }

    // select the given appnode
    void select_appnode(appTreePtr node, int notify);

    // select the given symbols
    void select_symbols (const symbolArr&, int notify);

    // deferred selection happens after all oustanding events are processed
    void post_selection (const symbolArr& syms, int notify);
    void post_selection (const symbolPtr& sym, int notify);
    
    // Enable displaying of special buttons.
    void handle_special_buttons(int flag);

  protected:
    gtRTL(gtBase*, const char*, const char*, gtListStyle, const char**, int);
};     


/*
   START-LOG-------------------------------------------

   $Log: gtRTL.h  $
   Revision 1.11 1998/07/15 13:31:34EDT omazor 
   Bug Track: N/A
   Added set_format() and set_sort() to gtRTL
 * Revision 1.2.1.9  1994/03/20  23:17:43  boris
 * Bug track: #6721
 * Fixed filter reset on Edit operations and "blinking" problem in viewerShell
 *
 * Revision 1.2.1.8  1994/01/21  00:22:01  kws
 * New rtl display
 *
 * Revision 1.2.1.7  1993/11/16  01:00:41  trung
 * Bug track: 0
 * propagation project
 *
 * Revision 1.2.1.6  1993/04/30  18:51:49  glenn
 * Break gtRTL up into gtRTL.h.C, gtRTL_util.h.C, and gtRTL_cfg.h.C
 *
 * Revision 1.2.1.5  1993/04/27  21:43:54  glenn
 * Remove gtLabel.h.
 * Rename get_entry_by_name to get_entry.
 *
 * Revision 1.2.1.4  1993/03/26  18:59:11  smit
 * Attach right position now attached the right side instead of bottom!!
 *
 * Revision 1.2.1.3  1992/12/15  20:05:35  aharlap
 * changed for new xref
 *
// Revision 1.16  92/09/28  15:37:11  glenn
// Remove unnecessary call to list->container->manage().
// 
// Revision 1.15  92/09/11  11:37:28  bakshi
// protected ctor; added gtRTL_cfg::save_entries for more reliable saves
// 
// Revision 1.14  92/09/11  04:07:40  builder
// Add ctor and dtor.
// 
// Revision 1.13  92/09/10  12:42:21  bakshi
// ability to save/restore/configure filters
// 
// Revision 1.12  92/09/03  13:22:00  bakshi
// add decl of select_appnode
// revamp interface again
// add new format /sort fields
// 
// Revision 1.11  92/08/28  19:05:18  bakshi
// define gtRTL_util class
// 
// Revision 1.10  92/08/27  19:36:49  bakshi
// add sort/format/filter
// 
// Revision 1.9  92/08/15  09:07:07  rfermier
// added new argument (w/default) to set_rtl
// 
// Revision 1.8  92/08/06  18:22:54  rfermier
// added num_items, sensitivity, dlg_create_window_instance
// 
// Revision 1.7  92/07/30  23:44:55  glenn
// GT method name changes
// 
// Revision 1.6  92/07/29  15:54:17  rfermier
// removed useless attachments, new get_last_item_position inline
// 
// Revision 1.5  92/07/28  17:27:31  rfermier
// add attachments
// 
// Revision 1.4  92/07/28  11:52:22  rfermier
// added simpler manage functions
// 
// Revision 1.3  92/07/24  15:39:16  builder
// Add rep() function.
// 
// Revision 1.2  92/07/22  13:37:52  rfermier
// added more container functions to match new List functions
// 
// Revision 1.1  92/07/14  16:12:20  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtRTL_h
