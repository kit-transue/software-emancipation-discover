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
#ifndef _gtList_h 
#define _gtList_h

// gtList.h
//------------------------------------------
// synopsis:
// class description of generic List (gtList)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

#ifndef _gtLabel_h
#include <gtLabel.h>
#endif

class gtList : public gtPrimitive
{
  public:
    static gtList* create(
	gtBase *parent, const char *name, const char *title, gtListStyle ltype,
	const char **entries, int num_entries);

    // This constructor has a additional parameter to specify if you actually
    // want the status label/button.
    static gtList* create(
	gtBase *parent, const char *name, const char *title, gtListStyle ltype,
	const char **entries, int num_entries, int show_status);

    // Same as above, except that the status line below the list
    // becomes a pushbuton connected to status_line_callback
    // with client data client_data
    // in addition, when the status of the list changes,
    // a call is made to update_status_callback with
    // client data status data.  This allows for customization of
    // the status label
    static gtList* create(
        gtBase *parent, const char *name, const char *title, gtListStyle ltype,
        const char **entries, int num_entries,
	gtPushCB status_line_callback, void *client_data,
	gtListCB update_status_callback, void *status_data);

    static gtList* create(
        gtBase *parent, const char *name, const char *title, gtListStyle ltype,
        const char **entries, int num_entries,
	gtPushCB status_line_callback, void *client_data,
	gtListCB update_status_callback, void *status_data,
	gtPushCB reset_callback, void *reset_data);
    
    static gtList* create(
        gtBase *parent, const char *name, const char *title, gtListStyle ltype,
        const char **entries, int num_entries,
	gtPushCB status_line_callback, void *client_data,
	gtListCB update_status_callback, void *status_data,
	gtPushCB reset_callback, void *reset_data, gtRTL* gtrtl);
    ~gtList();

    // Returns the currently selected string(s).
    // Caller must use gtFree() to deallocate storage in return value.
    virtual char **selected() = 0;

    // Returns number of items currently selected.
    virtual int num_selected() = 0;

    // Adds item at location loc.
    virtual void add_item(const char *item, int loc) = 0;

    // Adds gtString at location loc.
    virtual void add_item(gtString*, int loc) = 0;

    // Like add_item, but the new entry is unselected.
    // SAME AS add_item(), see above.
    virtual void add_item_unselected(const char *item, int loc) = 0;

    // Add items at location loc.
    virtual void add_items(const char **items, int num_items, int loc) = 0;

    // Replaces the items from items with new_items
    virtual void replace_items(const char **items, const char **new_items, int num_items) = 0;

    // Removes all items from the gtList.
    virtual void delete_all_items() = 0;

    // Removes the item at pos from the gtList.
    virtual void delete_pos(int pos) = 0;

    // Clears all selections.
    virtual void deselect_all() = 0;

    // Clears the selection at location pos.
    virtual void deselect_pos(int pos) = 0;

    // Sets its arguments to the positions of all of the selections.
    // Caller must use gtFree() to deallocate storage in pos_list.
    virtual void get_selected_pos(int **pos_list, int *pos_count) = 0;

    // Returns the location of item in the list.
    virtual int  item_pos(const char *item) = 0;

    // Sets a selection on item, and if notify is set will call
    // appropriate callbacks.
    virtual void select_item(const char *item, int notify) = 0;
    
    // Like select_item, only at a location
    virtual void select_pos(int post, int notify) = 0;

    // Select all items, like select_item.
    virtual void select_all(int notify) = 0;

    // Sets the top or bottom item of the list.
    virtual void set_bottom_item(const char *item) = 0;
    virtual void set_top_item(const char *item) = 0;

    // Attempts to resize the list so that n rows are visible.
    virtual void num_rows(int n) = 0;

    // Performs appropriate ui parsing functions on the list.
    virtual void ui_parse(struct ui_list *list) = 0;

    // Set a callback to be triggered when the List is activated.
    // SAME AS selection_callback, see below.
    virtual void set_callback(gtListCB, void *) = 0;

    // Scroll the list so that the item at location pos is at the top.
    virtual void set_pos(int pos) = 0;

    // Scroll the list so that the item at locaiton pos is at the bottom.
    virtual void set_bottom_pos(int pos) = 0;
    
    // Scroll the list so that the item at location pos is visible.
    void make_pos_visible(int pos);

    // Returns the total number of items in the list.
    virtual int num_items() = 0;

    // Sets the selection style of the list.
    virtual void sel_type(gtListStyle) = 0;

    // Returns the position of the top item in the list.
    virtual int top_item_position() = 0;

    // Returns the number of visible items in the list.
    virtual int num_visible() = 0;

    // Sets the default font of the list.
    virtual void font(const char *) = 0;

    // Set the selection callback for the current selection-mode.
    virtual void select_callback(gtListCB, void *) = 0;

    // Set the callback used when an item is double-clicked.
    virtual void action_callback(gtListCB, void *) = 0;

    // Update the status line that appears below the list.
    virtual void update_status() = 0;

    // Access function for the status line pushbutton
    virtual gtPushButton *status_button() = 0;

    // Returns the nth item in the list.
    // Caller must use gtFree() to deallocate storage in return value.
    virtual char *item_at_pos(int n) = 0;

    // Returns all the items in the list.
    // Caller must use gtFree() to deallocate storage in return value.
    virtual char **all_items() = 0;
    
    // Get the list label.
    virtual gtLabel *get_label() = 0;

    // specify and retrieve the background color of the object
    virtual unsigned long background() = 0;
    virtual void background(unsigned long) = 0;
    virtual void background(const char*) = 0;
    
    virtual void reset_pos(void) = 0;
    virtual gtBase *get_button_form(void) = 0;
    virtual void    add_button(gtPrimitive *button) = 0;
    virtual void set_special_buttons(int flag) = 0;
    virtual void set_reset_sensitive(int flag) = 0;
    
  protected:
    gtList();
};


/*
   START-LOG-------------------------------------------

 $Log: gtList.h  $
 Revision 1.9 1999/07/19 10:43:08EDT rpasseri 
 adding "replace_items()" method
 * Revision 1.2.1.6  1994/01/28  23:42:45  jon
 * Added constructor with arg to disable '0 of 0' label
 *
 * Revision 1.2.1.5  1993/07/13  12:27:15  builder
 * do all_items pure virtual
 *
 * Revision 1.2.1.4  1993/07/07  23:52:57  jon
 * Added item_at_pos, num_items, all_items
 *
 * Revision 1.2.1.3  1993/01/26  05:07:33  glenn
 * cleanup.
 *
// Revision 1.17  92/09/11  11:12:08  builder
// Make destructor public.
// 
// Revision 1.16  92/09/11  04:07:26  builder
// Add ctor and dtor.
// 
// Revision 1.15  92/09/03  13:20:13  bakshi
// added ability to customize update of status button
// 
// Revision 1.14  92/08/28  19:04:45  bakshi
// added status line pushbutton capability
// 
// Revision 1.13  92/08/27  19:36:32  bakshi
// add sort/filter/format: fix bug #197
// 
// Revision 1.12  92/07/31  08:08:39  builder
// One more const char* to fix.
// 
// Revision 1.11  92/07/30  23:44:43  glenn
// GT method name changes
// 
// Revision 1.10  92/07/30  09:18:31  rfermier
// Added list-specific management function
// 
// Revision 1.9  92/07/29  15:54:03  rfermier
// added get_last_item_position
// 
// Revision 1.8  92/07/28  13:51:51  glenn
// Add select_callback, action_callback.
// Make font take const char*.
// 
// Revision 1.7  92/07/28  11:36:28  rfermier
// added font function
// 
// Revision 1.6  92/07/28  00:09:50  glenn
// Use const char* for title and entries.
// Add add_item with gtString.
// 
// Revision 1.5  92/07/22  13:38:50  rfermier
// added sel_type, top_item_position, and num_visible
// 
// Revision 1.4  92/07/15  14:51:46  rfermier
// added select_all and num_items
// 
// Revision 1.3  92/07/14  16:13:08  rfermier
// RTL functions removed
// 
// Revision 1.2  92/07/10  15:32:42  rfermier
// basic RTL support added, more documentation
// 
// Revision 1.1  92/07/08  20:39:01  rfermier
// Initial revision

   END-LOG---------------------------------------------
*/

#endif // _gtList_h

