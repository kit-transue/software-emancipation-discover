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
#ifndef _gtListXm_h
#define _gtListXm_h

// gtListXm.h
//------------------------------------------
// synopsis:
// Class description for the gtList object in OSF Motif
//------------------------------------------

#include <gtList.h>
#include <gtCallbackMgrXm.h>
#include <gtBitmap.h>

class view;
class gtRTL;

class gtListXm : public gtList, public gtCallbackMgrXm
{
  public:
    gtListXm(gtBase *parent, const char *name, const char *title,
	     gtListStyle ltype, const char **entries, int num_entries,
	     gtPushCB status_line_callback, void *client_data,
             gtListCB update_status_callback, void *status_data,
	     gtPushCB reset_callback, void *reset_data, int show_status,
	     gtRTL* rtl);
    ~gtListXm();

    void sel_type(gtListStyle);
    void font(const char*);

    void add_item(const char *item, int loc);
    void add_item(gtString *item, int loc);
    void add_item_unselected(const char *item, int loc);
    void add_items(const char **items, int num_items, int loc);

    void replace_items(const char **items, const char **new_items, int num_items);

    void delete_all_items();
    void delete_pos(int pos);

    void select_item(const char *item, int notify);
    void select_pos(int post, int notify);
    void select_all(int notify);

    void deselect_all();
    void deselect_pos(int pos);
    
    /* resets the internal last selected position value */
    void reset_pos();

    char **selected();
    int  num_selected();
    void get_selected_pos(int** pos_list, int* pos_count);

    char *item_at_pos(int n);
    char **all_items();
    int  num_items();

    void num_rows(int);
    int  num_visible();

    void set_top_item(const char *item);
    int  top_item_position();
    void set_bottom_item(const char *item);

    void set_pos          (int);
    void set_bottom_pos   (int);
    int  item_pos         (const char *item);

    void update_status();
    gtPushButton* status_button();

    void ui_parse(struct ui_list *list);

    gtBase* container();

    void set_callback(gtListCB, void*);
    void select_callback(gtListCB, void*);
    void action_callback(gtListCB, void*);
    // specify and retrieve the background color of the object
    unsigned long background();
    void background(unsigned long c);
    void background(const char*);
    
    gtBase *get_button_form(void);
    void   add_button(gtPrimitive *button);
    void   set_special_buttons(int flag);
    void   set_reset_sensitive(int flag);
    
    gtLabel* get_label() { return label; };
    view* get_view (void) const;

  private:
    static void select_CB(gtList* list, gtEventPtr, void*, gtReason);
    static void update_CB(gtList*, gtEventPtr, void*, gtReason);
    static void upArrowCB(gtArrowButton*, gtEvent*, void* l, gtReason);
    static void downArrowCB(gtArrowButton*, gtEvent*, void* l, gtReason);
    static void currentButtonCB(gtPushButton*, gtEvent*, void* l, gtReason);

    gtForm*       button_form;
    gtBase*       list_container;

    gtLabel*      label;
    gtArrowButton *up_arrow;
    gtArrowButton *down_arrow;
    gtPushButton  *current_button;
    gtBitmap      *icon_curr;
    gtPrimitive   *additional_buttons[7];
    int           num_buttons;

    gtListStyle	ltype;
    
    gtListCB update_override;
    void*    update_cd;

    struct saved_update_struct
    {
	int itemCount;
	int selectedItemCount;

	saved_update_struct() : itemCount(-1), selectedItemCount(-1) {}
    }
    saved_update;

    union
    {
        gtLabel*      list_status_label;
        gtPushButton* list_status_button;
    };
    gtPushButton *list_reset_button;
    unsigned int is_button : 1;
    unsigned int special_buttons : 1;
    int      last_selected;
    Widget   lst;
    gtRTL*	gtrtl;
};


/*
   START-LOG-------------------------------------------

// $Log: gtListXm.h  $
// Revision 1.9 1999/07/19 10:44:31EDT rpasseri 
// adding "replace_items()" method
 * Revision 1.2.1.6  1994/01/28  23:46:42  jon
 * Added constructor with arg for disabling '0 of 0' label
 *
 * Revision 1.2.1.5  1993/07/07  23:51:42  jon
 * Added item_at_pos, num_items, all_items
 *
 * Revision 1.2.1.4  1993/05/11  18:12:47  glenn
 * Add saved_update for use in update_status.
 *
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
// Revision 1.17  92/09/28  15:38:47  glenn
// Reorganize members and replace Widget with gtBase* list_container.
// 
// Revision 1.16  92/09/03  13:24:39  bakshi
// added ability to override update callback
// 
// Revision 1.15  92/08/28  19:02:26  bakshi
// consolidate status line buttons into one.
// 
// Revision 1.14  92/08/27  19:38:32  bakshi
// add button and status line capability: fix bugs 887, 935
// 
// Revision 1.13  92/08/06  18:13:29  rfermier
// *** empty log message ***
// 
// Revision 1.12  92/07/31  08:13:42  builder
// Fix ctor bug.
// 
// Revision 1.11  92/07/30  23:55:52  rfermier
// GT method name changes
// 
// Revision 1.10  92/07/30  09:18:17  rfermier
// Added list-specific management function
// 
// Revision 1.9  92/07/29  15:53:12  rfermier
// added get_last_position
// 
// Revision 1.8  92/07/28  13:53:22  glenn
// Add select_callback, action_callback.  Make font take const char*.
// Add add_callback (internal).
// 
// Revision 1.7  92/07/28  11:36:14  rfermier
// Added font function
// 
// Revision 1.6  92/07/28  00:12:49  glenn
// Remove unused member variables.  Add add_item with gtString.
// Use const char* for title and entries.
// 
// Revision 1.5  92/07/22  13:38:44  rfermier
// added sel_type, top_item_position, and num_visible
// 
// Revision 1.4  92/07/15  14:51:36  rfermier
// added select_all and num_items
// 
// Revision 1.3  92/07/14  16:12:56  rfermier
// RTL functions removed
// 
// Revision 1.2  92/07/10  15:32:17  rfermier
// basic RTL support added
// 
// Revision 1.1  92/07/08  20:41:00  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtListXm_h


