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
// gtListXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtList class in OSF Motif
//------------------------------------------

// INCLUDE FILES
#include <machdep.h>
#include <psetmem.h>

#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#include <cstdio>
#endif /* ISO_CPP_HEADERS */

#include <genString.h>
#include <systemMessages.h>
#include <top_widgets.h>
#include <ui-globals.h>
#include <gtString.h>
#include <gtNoParent.h>
#include <gtLabel.h>
#include <gtForm.h>
#include <gtPushButton.h>
#include <gtArrowButton.h>
#include <gtBaseXm.h>
#include <gtListXm.h>
#include <gtListXm_internal.h>
#include <Xm/List.h>
#include <genError.h>
#include <gtRTL.h>
#include <ldrList.h>
#include <viewList.h>
#include <dialog.h>

#define curr_width 16
#define curr_height 16
static char curr_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0xfc, 0x3f, 0x0c, 0x30, 0x0c, 0x30,
   0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30,
   0xfc, 0x3f, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x00};

#define lowercase(c)	\
	((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c))

// FUNCTION DEFINITIONS

static char* findstr (const char* const haystack, const char* const needle)
{
    const char* const	needle_end = strchr (needle, '\0');
    const char* const	haystack_end = strchr (haystack, '\0');
    const size_t	needle_len = needle_end - needle;
    const size_t	needle_last = needle_len - 1;
    const char*		begin;

    if (needle_len == 0)
	return (char *) haystack;
    if ((size_t) (haystack_end - haystack) < needle_len)
	return NULL;

    const char*	n;
    const char* h;
    char	nc;
    char	hc;

    for (begin = &haystack[needle_last]; begin < haystack_end; ++begin) {

	n = &needle[needle_last];
	h = begin;

	do {
	    nc = *n;
	    hc = *h;
	    if (lowercase (nc) != lowercase (hc))
		goto loop;
	} while (--n >= needle && --h >= haystack);

	return (char*) h;

	loop:;
    }

  return NULL;
}

static int xm_sel_policy(gtListStyle ltype)
{
    switch(ltype)
    {
      case gtSingle:   return XmSINGLE_SELECT;
      case gtMultiple: return XmMULTIPLE_SELECT;
      case gtBrowse:   return XmBROWSE_SELECT;
      case gtExtended: return XmEXTENDED_SELECT;
    }
    return -1;
}

void gtListXm::select_CB(gtList* list, gtEventPtr, void*, gtReason)
{
    gtListXm* listxm = (gtListXm*)list;
    
    int *pos_list;
    int pos_count;
    listxm->get_selected_pos(&pos_list, &pos_count);
    if(pos_list){
	listxm->last_selected = *pos_list;
	gtFree(pos_list);
    }
}

void gtListXm::update_CB(gtList* list, gtEventPtr, void*, gtReason)
{
    gtListXm* listxm = (gtListXm*)list;

    char buffer[50];
    sprintf(buffer, "%d of %d", listxm->num_selected(), listxm->num_items());

    if(listxm->is_button)
	listxm->list_status_button->title(buffer);
    else
	listxm->list_status_label->text(buffer);
}


gtList* gtList::create(gtBase *parent, const char* name,
		       const char *title, gtListStyle ltype,
		       const char **entries, int num_entries)
{
    return new gtListXm(parent, name, title, ltype, entries, num_entries,
			NULL, NULL, NULL, NULL, 
			NULL, NULL, 1, NULL);
}

gtList* gtList::create(gtBase *parent, const char* name,
		       const char *title, gtListStyle ltype,
		       const char **entries, int num_entries, int show_status)
{
    return new gtListXm(parent, name, title, ltype, entries, num_entries,
			NULL, NULL, NULL, NULL, 
			NULL, NULL, show_status, NULL);
}


gtList* gtList::create(gtBase *parent, const char* name,
                       const char *title, gtListStyle ltype,
                       const char **entries, int num_entries,
                       gtPushCB status_line_callback, void *client_data,
		       gtListCB update_status_callback, void *status_data)
{
    return new gtListXm(parent, name, title, ltype, entries, num_entries,
			status_line_callback, client_data,
			update_status_callback, status_data, 
			NULL, NULL, 1, NULL);
}

gtList* gtList::create(gtBase *parent, const char* name,
                       const char *title, gtListStyle ltype,
                       const char **entries, int num_entries,
                       gtPushCB status_line_callback, void *client_data,
		       gtListCB update_status_callback, void *status_data,
		       gtPushCB reset_callback, void *reset_data)
{
    return new gtListXm(parent, name, title, ltype, entries, num_entries,
			status_line_callback, client_data,
			update_status_callback, status_data, 
			reset_callback, reset_data, 1, NULL);
}

gtList* gtList::create(gtBase *parent, const char* name,
                       const char *title, gtListStyle ltype,
                       const char **entries, int num_entries,
                       gtPushCB status_line_callback, void *client_data,
		       gtListCB update_status_callback, void *status_data,
		       gtPushCB reset_callback, void *reset_data, gtRTL* rtl)
{
    return new gtListXm(parent, name, title, ltype, entries, num_entries,
			status_line_callback, client_data,
			update_status_callback, status_data, 
			reset_callback, reset_data, 1, rtl);
}


static char		buffered_keys[32];
static short		buffered_keys_count;
static int		buffered_keys_found;
static Bool		buffered_keys_failed;
static Bool		buffered_keys_unanchored;
static Widget		buffered_keys_widget;
static Time		buffered_keys_time;
static const Time	buffered_keys_timeout = 2500;

inline void reset_buffered_keys()
{
    buffered_keys_count = 0;
    buffered_keys_found = 0;
    buffered_keys_failed = False;
    buffered_keys_unanchored = False;
}


inline int max(int a, int b) { return (a > b) ? a : b; }


static void XmList_keyboard_scroll(Widget w, XtPointer ptr, XEvent* e, Boolean*)
//
// Do Macintosh-style list scrolling using keyboard input.
//
{
    Initialize (XmList_keyboard_scroll);

    char	input = 0;
    gtListXm*	glp = (gtListXm*) ptr;

    if (!glp)
	return;

    // Extract input character, but ignore event if:
    if(// It is not a key-press, or
       (e->xany.type != KeyPress) ||

       //  It does not map to character input, or
       (XLookupString(&e->xkey, &input, 1, NULL, NULL) <= 0) ||

       // Character input is not ASCII.
       !isascii(input))
	return;

    // Reset internal state if the widget changed.
    if(w != buffered_keys_widget)
	reset_buffered_keys();

    int idx = -1;		// Where to begin the search.
    int search_inc = 1;		// Search forwards or backwards.

    // Interpret input character.
    if(iscntrl(input))
    {
	switch(input)
	{
	  case ('B' & 0x1F):	// control-B => Backward anchored search
	    buffered_keys_unanchored = False;
	    idx = buffered_keys_found - 1;
	    search_inc = -1;
	    break;

	  case ('F' & 0x1F):	// control-F => Forward anchored search
	    buffered_keys_unanchored = False;
	    idx = buffered_keys_found - 1;
	    break;

	  case ('R' & 0x1F):	// control-R => Reverse unanchored search
	    buffered_keys_unanchored = True;
	    idx = buffered_keys_found - 1;
	    search_inc = -1;
	    break;

	  case ('S' & 0x1F):	// control-S => Forward unanchored search
	    buffered_keys_unanchored = True;
	    idx = buffered_keys_found - 1;
	    break;

	  default:		// Ignore all other control-characters.
	    return;
	}
	if(buffered_keys_found == 0)	// Complain if nothing found last time.
	{
	    XBell(XtDisplay(w), 100);
	    return;
	}
    }
    else
    {
	// Reset internal state if timeout has elapsed.
	if((e->xkey.time - buffered_keys_time) > buffered_keys_timeout)
	    reset_buffered_keys();
	else if (buffered_keys_failed) {
	    XBell (XtDisplay (w), 100);
	    return;
	}

	// Extend the matching string with lower-case characters.
	if(buffered_keys_count < (sizeof(buffered_keys) - 1))
	{
	    buffered_keys[buffered_keys_count++] =
		(isupper(input) ? input + 'a' - 'A' : input);
	    buffered_keys[buffered_keys_count] = '\0';
	}
    }

    push_busy_cursor();

    // Look for the first item that starts with "buffered_keys",
    // storing the matching item number in "found".
    int found = 0;

    // Also look for the first item that contains "buffered_keys",
    // storing the matching item number in "contains".
    int contains = 0;

    int			itemCount;
    SortedList*		slp = NULL;
    XmStringTable	items;

    view* vp = glp->get_view ();
    if (vp && is_viewListHeader (vp)) {
	viewListHeader* vlp = checked_cast (viewListHeader, vp);
	ldr* lp = view_get_ldr (vp);
	if (lp && is_ldrList (lp)) {
	    ldrList* llp = checked_cast (ldrList, lp);
	    slp = llp->get_list (vlp->strSort(), vlp->strFormat(),
			vlp->strShow(), vlp->strHide(), vlp->descending());
	}
    }

    if (slp)
	itemCount = slp->sorted.size ();
    else XtVaGetValues(w, XmNitemCount, &itemCount, XmNitems, &items, NULL);

    for(int i = itemCount; i > 0  &&  !found; --i)
    {
	idx += search_inc;
	if(idx >= itemCount)
	    idx = 0;
	else if(idx < 0)
	    idx = itemCount - 1;

	char* string = NULL;

	if (slp)
	    string = slp->text[slp->sorted[idx]->offset];
	else XmStringGetLtoR (items[idx], XmSTRING_DEFAULT_CHARSET, &string);

	if (string) {
	    // Look for a matching sub-string.
	    const char* substr = findstr (string, buffered_keys);

	    if(!buffered_keys_unanchored  &&  substr == string)
	    {
		found = idx + 1;	// Found anchored match.
	    }
	    if(!contains  &&  substr)
	    {
		contains = idx + 1;	// Found unanchored match.

		if(buffered_keys_unanchored)
		    found = contains;	// Force success on unanchored match.
	    }

	    if (!slp)
	        XtFree (string);
	}
    }

    pop_cursor();

    // If anchored match failed, substitute containing match.
    if(!found)
	found = contains;

    if(!found)			// Complain if nothing found this time.
    {
	XBell(XtDisplay(w), 100);
	buffered_keys_failed = True;
    }
    else if(found != buffered_keys_found) // If a new match was found:
    {
	// Select the item and make sure it is visible.
	XmListSelectPos(w, found, 1);

	// Center the list on the selected item if it is not visible.
	int topItemPosition, visibleItemCount;
	XtVaGetValues(w, XmNtopItemPosition, &topItemPosition,
		      XmNvisibleItemCount, &visibleItemCount, NULL);
	if((found < topItemPosition)  ||
	   (found >= topItemPosition + visibleItemCount))
	    XmListSetPos(w, max(1, found - (visibleItemCount / 2)));
    }

    // Save state for next time.
    buffered_keys_widget = w;
    buffered_keys_time = e->xkey.time;
    buffered_keys_found = found;
}


gtListXm::gtListXm(gtBase* parent, const char* name,
		   const char* title, gtListStyle lt,
		   const char** entries, int num_entries,
		   gtPushCB status_line_callback, void* client_data,
		   gtListCB update_status_callback, void* status_data,
		   gtPushCB reset_callback, void *reset_data,
		   int show_status, gtRTL* rtl)
: ltype(lt), is_button(status_line_callback != NULL), gtrtl (rtl)
{
    genString form_name;
    form_name.printf("%s_form", name);

    num_buttons     = 0;
    special_buttons = 0;
    last_selected   = -1;
    
    gtForm* vb = gtForm::create(parent, form_name);
    delete r;
    attach_rep(vb->rep());

    label = NULL;
    if(title)
    {
	label = gtLabel::create(vb, "listLabel", title);
	label->attach_tb(NULL, label);
	label->manage();
    }
    
    button_form = gtForm::create(vb, "list_button");
    
    // Status line below the list
    if(is_button) {
	list_status_button = gtPushButton::create(
	    button_form, "list_status", TXT("? of ?"),
	    status_line_callback, client_data);
	list_status_button->attach(gtTop);
	list_status_button->attach(gtLeft);
	list_status_button->attach(gtBottom);
	list_reset_button = gtPushButton::create(
	    button_form, "list_reset", TXT("R"),
	    reset_callback, reset_data);
	list_reset_button->attach(gtTop);
	list_reset_button->attach(gtRight);
	list_reset_button->attach(gtBottom);
	list_reset_button->manage();
	list_status_button->attach(gtRight, list_reset_button);
	list_status_button->manage();
    }
    else {
	list_status_label = gtLabel::create(
	    button_form, "list_status", TXT("? of ?"));
	list_status_label->attach(gtTop);
	list_status_label->attach(gtLeft);
	list_status_label->attach(gtRight);
	list_status_label->attach(gtBottom);
	list_status_label->manage();
	list_reset_button = NULL;
    }
    vb->release_rep();
    delete vb;			// Do not use vb after this point.

    Arg args[13];

    // Narrow button margins
    XtSetArg(args[0], XmNmarginTop, 0);
    XtSetArg(args[1], XmNmarginBottom, 0);

    button_form->attach_tb(button_form);
    button_form->manage();

    /*
    if(is_button)
    {
	XtSetValues(list_status_button->rep()->widget(), args, 2);
	list_status_button->attach_tb(list_status_button);
        list_status_button->manage();
    }
    else
    {
	XtSetValues(list_status_label->rep()->widget(), args, 2);
	list_status_label->attach_tb(list_status_label);
        list_status_label->manage();
    }
    */

    int policy = xm_sel_policy(ltype);
    if(policy < 0)
	policy = XmSINGLE_SELECT;

    int n = 0;
    XtSetArg(args[n], XmNselectionPolicy, policy); n++;
    XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmAS_NEEDED); n++;
    XtSetArg(args[n], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); n++;

    XtSetArg(args[n], XmNleftOffset, 0); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;

    XtSetArg(args[n], XmNrightOffset, 0); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;

    XtSetArg(args[n], XmNbottomOffset, 0); n++;
    if (show_status)
    {
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
//        XtSetArg(args[n], XmNbottomWidget, list_status_label->rep()->widget()); n++;
        XtSetArg(args[n], XmNbottomWidget, button_form->rep()->widget()); n++;
    } else {
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	list_status_label->unmanage();
    }

    XtSetArg(args[n], XmNtopOffset, 0); n++;
    if(label)
    {
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, label->rep()->widget()); n++;
    }
    else
    {
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    }

//    Widget lst = XmCreateScrolledList(rep()->widget(), (char*)name, args, n);
    lst = XmCreateScrolledList(rep()->widget(), (char*)name, args, n);

    // Initialize key-press handler.
    XtAddEventHandler(
	lst, KeyPressMask, False, XmList_keyboard_scroll, XtPointer(this));

    list_container = gtNoParent::create(lst);
    list_container->manage();

    if(num_entries > 0)
    {
	XmStringTable table =
	    (XmStringTable)psetmalloc(num_entries * sizeof(XmString));
	int i;
	for(i = 0; i < num_entries; i++)
	    table[i] = XmStringCreateSimple((char*)entries[i]);

	XtSetArg(args[0], XmNitems, table);
	XtSetArg(args[1], XmNitemCount, num_entries);
	XtSetValues(list_container->rep()->widget(), args, 2);

	for(i = 0; i < num_entries; i++)
	    XmStringFree(table[i]);
	OSapi_free(table);
    }

    num_rows(7);
    if(update_status_callback)
    {
	select_callback(select_CB, this);
	select_callback(update_status_callback, status_data);
	update_override = update_status_callback;
	update_cd = status_data;
    }
    else
    {
	select_callback(select_CB, this);
	select_callback(update_CB, this);
	update_override = NULL;
	update_cd = NULL;
    }
    update_status();
}    


gtListXm::~gtListXm()
{}

void gtListXm::reset_pos(void)
{
    last_selected = 1;
}

void gtListXm::upArrowCB(gtArrowButton*, gtEvent*, void* l, gtReason)
{
    gtListXm *list = (gtListXm *)l;
    if(list->last_selected != -1){
	if(list->last_selected > 1){
	    if(list->top_item_position() >= list->last_selected)
		list->set_pos(list->last_selected - 1);
	    if(list->top_item_position() + list->num_visible() - 1 <= list->last_selected)
		list->set_pos(list->last_selected - list->num_visible() + 1);
	    list->select_pos(list->last_selected - 1, 1);
	}
    }
}

void gtListXm::downArrowCB(gtArrowButton* b, gtEvent*, void* l, gtReason)
{
  gtListXm *list = (gtListXm *)l;
  if(list->last_selected != -1){
    if(list->num_items() > list->last_selected){
      if(list->top_item_position() >= list->last_selected)
	list->set_pos(list->last_selected);
      if(list->top_item_position() + list->num_visible() - 1 <= list->last_selected)
	list->set_pos(list->last_selected - list->num_visible() + 2);
      list->select_pos(list->last_selected + 1, 1);
    }
  }
}

void gtListXm::currentButtonCB(gtPushButton*, gtEvent*, void* l, gtReason)
{
    gtListXm *list = (gtListXm *)l;
    if(list->last_selected != -1){
	if(list->top_item_position() > list->last_selected)
	    list->set_pos(list->last_selected);
	if(list->top_item_position() + list->num_visible() - 1 < list->last_selected)
	    list->set_pos(list->last_selected - list->num_visible() + 1);
	list->select_pos(list->last_selected, 1);
    }
}

gtBase *gtListXm::get_button_form(void)
{
    return button_form;
}

void gtListXm::add_button(gtPrimitive *button)
{
    if(num_buttons == 7)
	return;
    additional_buttons[num_buttons] = button;
    list_status_button->detach(gtLeft);
    if(special_buttons){
	button->attach(gtLeft, additional_buttons[num_buttons - 1], 3);
    } else {
	button->attach(gtLeft);
    }
    button->attach(gtTop);
    button->attach(gtBottom);
    button->manage();
    list_status_button->attach(gtLeft, button, 3);
    num_buttons++;
}

void gtListXm::set_reset_sensitive(int flag)
{
    if(list_reset_button){
	list_reset_button->set_sensitive(flag);
    }
}

void gtListXm::set_special_buttons(int flag)
{
    gtPrimitive *button = (gtPrimitive *)list_status_button;
    if(flag && !special_buttons){
	button->detach(gtLeft);

	up_arrow = gtArrowButton::create(button_form, "up_arrow", gtDirLeft,
					 upArrowCB, this);
	up_arrow->attach(gtLeft);
	up_arrow->attach(gtTop);
	up_arrow->attach(gtBottom);
	up_arrow->manage();

	icon_curr = gtBitmap::create(button_form, "icon", curr_bits, curr_width, curr_height);

	current_button = gtPushButton::create(button_form, "current_button", icon_curr,
					      currentButtonCB, this);
	current_button->attach(gtLeft, up_arrow, 3);
	current_button->attach(gtTop);
	current_button->attach(gtBottom);
	current_button->manage();

	down_arrow = gtArrowButton::create(button_form, "down_arrow", gtDirRight,
					   downArrowCB, this);
	down_arrow->attach(gtLeft, current_button, 3);
	down_arrow->attach(gtTop);
	down_arrow->attach(gtBottom);
	down_arrow->manage();

	button->attach(gtLeft, down_arrow, 3);
	
	additional_buttons[0] = up_arrow;
	additional_buttons[1] = current_button;
	additional_buttons[2] = down_arrow;
	num_buttons           = 3;
	special_buttons       = 1;
    }else
	if(!flag){
	    for(int i = 0; i < num_buttons; i++){
		additional_buttons[i]->unmanage();
		delete additional_buttons[i];
	    }
	    button->attach(gtLeft);
            if(special_buttons)
		delete icon_curr;
	    special_buttons = 0;
	    num_buttons     = 0;
	}
}

gtBase* gtListXm::container()
{
    return list_container;
}

void gtListXm::background(unsigned long c)
{
    Arg args[1];
    XtSetArg(args[0], XmNbackground, c);
    XtSetValues(lst, args, 1);
}

void gtListXm::background(const char *c)
{
    XtVaSetValues(lst, XtVaTypedArg, XmNbackground, XmRString, c,
		  strlen(c), NULL);
}

unsigned long gtListXm::background()
{
    unsigned long ans;
    Arg args[1];	
    XtSetArg(args[0], XmNbackground, &ans);
    XtGetValues(lst, args, 1);
    return ans;
}

char** gtListXm::selected()
{
    Arg args[2];
    int num;
    XmStringTable sel;

    XtSetArg(args[1], XmNselectedItemCount, &num);
    XtSetArg(args[0], XmNselectedItems, &sel);
    XtGetValues(list_container->rep()->widget(), args, 2);

	
    char** final;

    if (!num) {   // If the num = 0, calloc will return an address whose contents
	// points to an indeterminate location.
	final = (char**)calloc(1, sizeof(char*));
	final[0]= '\0';
    }
    else {
        final = (char**)calloc(num, sizeof(char*));
    }
    for(int i = 0; i < num; i++)
	XmStringGetLtoR(sel[i], XmSTRING_DEFAULT_CHARSET, final + i);
    
    return final;
}


int gtListXm::num_selected()
{
    int ans;
    Arg	args[1];

    XtSetArg(args[0], XmNselectedItemCount, &ans);
    XtGetValues(list_container->rep()->widget(), args, 1);

    return ans;
}


char* gtListXm::item_at_pos(int n)
{
    Arg args[2];
    int num;
    XmStringTable items;

    XtSetArg(args[1], XmNitemCount, &num);
    XtSetArg(args[0], XmNitems, &items);
    XtGetValues(list_container->rep()->widget(), args, 2);

    char* final = NULL;

    if ((n >= 0) && (n < num))
	XmStringGetLtoR(items[n], XmSTRING_DEFAULT_CHARSET, &final);

    return final;
}


char** gtListXm::all_items()
{
    Arg args[2];
    int num;
    XmStringTable items;

    XtSetArg(args[1], XmNitemCount, &num);
    XtSetArg(args[0], XmNitems, &items);
    XtGetValues(list_container->rep()->widget(), args, 2);

    char** final = (char**)calloc(num, sizeof(char*));

    for(int i = 0; i < num; i++)
	XmStringGetLtoR(items[i], XmSTRING_DEFAULT_CHARSET, final + i);

    return final;
}


void gtListXm::add_item(const char *item, int loc)
{
    XmString str = XmStringCreateSimple((char*)item);
    if(str)
    {
	XmListAddItemUnselected(list_container->rep()->widget(), str, loc);
	XmStringFree(str);
    }
    update_status();
}


void gtListXm::add_item(gtString *item, int loc)
{
    XmString str = (XmString)(item->rep());
    if(str)
	XmListAddItemUnselected(list_container->rep()->widget(), str, loc);
    update_status();
}


void gtListXm::add_item_unselected(const char *item, int loc)
{
    add_item((char*)item, loc);
}

    
void gtListXm::add_items(const char **items, int num_items, int loc)
{
    XmString* strs = (XmString *)calloc(num_items, sizeof(XmString));
    int i;
    for(i = 0; i < num_items; i++)
	strs[i] = XmStringCreateSimple((char*)items[i]);

    XmListAddItems(list_container->rep()->widget(), strs, num_items, loc);

    for(i = 0; i < num_items; i++)
	XmStringFree(strs[i]);
    OSapi_free(strs);

    update_status();
}

void gtListXm::replace_items(const char **items, const char **new_items, int num_items)
{
  XmString* item_strs = (XmString *)calloc(num_items, sizeof(XmString));
  XmString* new_item_strs = (XmString *)calloc(num_items, sizeof(XmString));
  int i;
  for(i = 0; i < num_items; i++)
    {
      item_strs[i] = XmStringCreateSimple((char*)items[i]);
      new_item_strs[i] = XmStringCreateSimple((char*)new_items[i]);
    }
  
  XmListReplaceItemsUnselected(list_container->rep()->widget(), item_strs, num_items, new_item_strs);
  
  for(i = 0; i < num_items; i++)
    {
      XmStringFree(item_strs[i]);
      XmStringFree(new_item_strs[i]);
    }
  OSapi_free(item_strs);
  OSapi_free(new_item_strs);

  update_status();
}
  
void gtListXm::delete_all_items()
{
    last_selected = 1;
    XmListDeleteAllItems(list_container->rep()->widget());
    update_status();
}


void gtListXm::delete_pos(int pos)
{
    XmListDeletePos(list_container->rep()->widget(), pos);
    update_status();
}


void gtListXm::deselect_all()
{
    Widget w = list_container->rep()->widget();
    int    top;
    XtVaGetValues(w, XmNtopItemPosition, &top, NULL);
    XmListDeselectAllItems(w);
    XmListWidget lw = (XmListWidget)list_container->rep()->widget();
    list_clear_after_deselect(lw);
    XtVaSetValues(w, XmNtopItemPosition, top, NULL);
    update_status();
}

void gtListXm::deselect_pos(int pos)
{
    XmListDeselectPos(list_container->rep()->widget(), pos);
    update_status();
}


void gtListXm::get_selected_pos(int **pos_list, int *pos_count)
{
    if(!XmListGetSelectedPos(list_container->rep()->widget(),
			     pos_list, pos_count))
    {
	*pos_list = NULL;
	*pos_count = 0;
    }
}


int gtListXm::item_pos(const char *item)
{
    XmString str = XmStringCreateSimple((char *)item);
    int ans = XmListItemPos(list_container->rep()->widget(), str);
    XmStringFree(str);

    return ans;
}


void gtListXm::select_item(const char *item, int notify)
{
    XmString str = XmStringCreateSimple((char *)item);
    XmListSelectItem(list_container->rep()->widget(), str, notify);
    last_selected = XmListItemPos(list_container->rep()->widget(), str);   
    XmStringFree(str);
    update_status();
}


void gtListXm::select_pos(int pos, int notify)
{
    XmListSelectPos(list_container->rep()->widget(), pos, notify);
    last_selected = pos;
    update_status();
}


void gtListXm::set_bottom_item(const char *item)
{
    XmString str = XmStringCreateSimple((char *)item);
    XmListSetBottomItem(list_container->rep()->widget(), str);
    XmStringFree(str);
}


void gtListXm::set_top_item(const char *item)
{
    XmString str = XmStringCreateSimple((char *)item);
    XmListSetItem(list_container->rep()->widget(), str);
    XmStringFree(str);
}


void gtListXm::num_rows(int n)
{
    Arg	args[1];
    XtSetArg(args[0], XmNvisibleItemCount, n);
    XtSetValues(list_container->rep()->widget(), args, 1);
}

    
void gtListXm::ui_parse(struct ui_list *list)
{
    int i;
    char last_icon = '\0';
    XmString icon_string = NULL;
    XmString name_string = NULL;
    XmString* string_array = (XmString*)psetmalloc(list->num * sizeof(XmString));
    
    if(string_array == NULL)
    {
        XmListDeleteAllItems(list_container->rep()->widget());
        return;
    }
    
    for(i = 0; i < list->num; i++)
    {
        char* string = list->strs[i];
        char* icon = string + strlen(string) + 1;

        if(string[0] == '\0')
            string = (char*)gettext(TXT("<noname>"));

	name_string = XmStringCreate(string, XmSTRING_DEFAULT_CHARSET);
	if(icon[0])
	{
	    if(icon[0] != last_icon) // Use cached icon if possible.
	    {
		if(last_icon && icon_string)
		    XmStringFree(icon_string);
		icon_string = XmStringCreate(icon, "aseticons");
		last_icon = icon[0];
	    }
	    string_array[i] = XmStringConcat(icon_string, name_string);
	    XmStringFree(name_string);
	}
	else
	    string_array[i] = name_string;
    }

    XmListDeleteAllItems(list_container->rep()->widget());
    XmListAddItems(list_container->rep()->widget(), string_array, list->num, 0);

    if(last_icon)
	XmStringFree(icon_string);
    for(i = 0; i < list->num; i++)
	XmStringFree(string_array[i]);
    OSapi_free(string_array);

    update_status();
}


void gtListXm::set_callback(gtListCB callback, void *client_data)
{
    select_callback(callback, client_data);
}

	
void gtListXm::select_callback(gtListCB callback, void *client_data)
{
    const char* cr = NULL;
    switch(ltype)
    {
      case gtSingle:   cr = XmNsingleSelectionCallback;   break;
      case gtMultiple: cr = XmNmultipleSelectionCallback; break;
      case gtBrowse:   cr = XmNbrowseSelectionCallback;   break;
      case gtExtended: cr = XmNextendedSelectionCallback; break;
    }
    if(cr)
	add_callback(cr, this,
		     gtCallbackFunc(callback), client_data,
		     list_container->rep());
}

	
void gtListXm::action_callback(gtListCB callback, void *client_data)
{
    add_callback(XmNdefaultActionCallback, this,
		 gtCallbackFunc(callback), client_data,
		 list_container->rep());
}
	

void gtListXm::set_pos(int pos)
{
    Initialize (gtListXm::set_pos);
    Assert (0 < pos <= num_items());
    XmListSetPos(list_container->rep()->widget(), pos);
}


void gtListXm::set_bottom_pos(int pos)
{   
    Initialize (gtListXm::set_bottom_pos);
    Assert (0 < pos <= num_items());
    XmListSetBottomPos(list_container->rep()->widget(), pos);
}


void gtListXm::select_all(int notify)
{
    int cnt;

    XtVaGetValues(list_container->rep()->widget(), XmNitemCount, &cnt, 0);
    XtVaSetValues(list_container->rep()->widget(), XmNselectionPolicy, XmMULTIPLE_SELECT, 0);

    for(int i = 1; i <= cnt; ++i)
      XmListSelectPos(list_container->rep()->widget(), i, notify);
    
    sel_type(ltype);
    update_status();
}


int gtListXm::num_items()
{
    int cnt;

    XtVaGetValues(list_container->rep()->widget(), XmNitemCount, &cnt, 0);

    return cnt;
}    


void gtListXm::sel_type(gtListStyle sty)
{
    int policy = xm_sel_policy(sty);
    if(policy >= 0)
    {
	ltype = sty;

	Arg args[1];
	XtSetArg(args[0], XmNselectionPolicy, policy);
	XtSetValues(list_container->rep()->widget(), args, 1);
    }
}


int gtListXm::top_item_position()
{
    int ans;
    XtVaGetValues(list_container->rep()->widget(),
		  XmNtopItemPosition, &ans, 0);
    return ans;
}


int gtListXm::num_visible()
{
    int visibleItemCount;
    XtVaGetValues(list_container->rep()->widget(),
		  XmNvisibleItemCount, &visibleItemCount,
		  NULL);

    return visibleItemCount;
}


void gtListXm::font(const char *font_name)
{
    XtVaSetValues(list_container->rep()->widget(),
		  XtVaTypedArg, XmNfontList,
		  XmRString, font_name, strlen(font_name) + 1,
		  NULL);
}


void gtListXm::update_status()
{
    int itemCount, selectedItemCount;
    XtVaGetValues(list_container->rep()->widget(),
		  XmNitemCount, &itemCount,
		  XmNselectedItemCount, &selectedItemCount,
		  NULL);

    if(saved_update.itemCount != itemCount  ||
       saved_update.selectedItemCount != selectedItemCount)
    {
	saved_update.itemCount = itemCount;
	saved_update.selectedItemCount = selectedItemCount;

	if(update_override)
	    (*update_override)(this, NULL, update_cd, gtReason(0));
	else 
	    update_CB(this, NULL, update_cd, gtReason(0));
    }
}


gtPushButton* gtListXm::status_button()
{
    return is_button ? list_status_button : NULL;
}

view* gtListXm::get_view (void) const
{
    if (gtrtl && gtrtl->dial())
	return gtrtl->dial()->get_view_ptr (0);

    return NULL;
}

/*
   START-LOG-------------------------------------------

   $Log: gtListXm.h.C  $
   Revision 1.20 2001/01/12 10:57:04EST sschmidt 
   Port to new SGI compiler
Revision 1.2.1.16  1994/01/28  23:45:47  jon
Added constructor with arg for disabling '0 of 0' label

Revision 1.2.1.15  1994/01/13  02:46:20  kws
Use psetmalloc

Revision 1.2.1.14  1993/10/29  13:09:06  himanshu
Bug track: 5102
Fixed calloc call in gtListXm::selected

Revision 1.2.1.13  1993/10/29  13:01:02  himanshu
Bug track: 5102
refixed gtListXm::selected

Revision 1.2.1.12  1993/10/07  16:09:28  himanshu
Bug track: 4712
It was reproduced after several tries. Bug was in a motif func. Fixed.

Revision 1.2.1.11  1993/07/07  23:42:54  jon
Added item_at_pos and all_items

Revision 1.2.1.10  1993/06/02  20:18:08  glenn
Remove Initialize and Return macros.  Remove genError.h.

Revision 1.2.1.9  1993/05/11  18:10:24  glenn
Do not complain (ring bell) when user presses <Return> in a list.
Speed up search by removing call to strncmp().
Do not call update-callback if number of items, or number of selected
items has not changed since last call.

Revision 1.2.1.8  1993/05/07  19:36:48  glenn
Add KeyPress event-handler for lists.

Revision 1.2.1.7  1993/04/27  21:50:57  glenn
Delete the initial gtRep instead of using release_rep.

Revision 1.2.1.6  1993/04/14  15:53:41  glenn
Give unique name to overall form used by gtList to layout its components.
The name is constructed by appending "_form" to the name used for the
XmScrolledList widget.

Revision 1.2.1.5  1993/02/04  16:03:28  jon
Fixed memory leak

Revision 1.2.1.4  1993/01/26  04:32:20  glenn
Use gtCallbackMgrXm.
Remove Return macros.
Remove get_last_item_position, manage.

Revision 1.2.1.3  1992/10/27  09:29:20  oak
Treat list filter button the same as
a list label.  The difference is
hardly noticable but prevents a posible
memory error.

Revision 1.2.1.2  92/10/09  19:03:34  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/



