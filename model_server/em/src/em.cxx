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
/*_______________________________________________________________________
!
! em.C	
!
! Synopsis:	Functions for manipulating the user interface from
!		automated test tools.
!
_______________________________________________________________________*/
#define _cLibraryFunctions_h
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "driver.h"
#include "graWorld.h"
#include "viewGraHeader.h"
#include "viewerShell.h"
#include "gtAppShellXm.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <math.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <vpopen.h>
#include <x11_intrinsic.h>
#include <Xm/Xm.h>
#include <X11/X.h>
#include <Xm/List.h>
#include <em.h>
#include <em_popups.h>
#ifndef ISO_CPP_HEADERS
#include <sys/time.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
#endif /* ISO_CPP_HEADERS */

#include <machdep.h>

static int speed = 72; 
static long starttime = 0; // for constructing timestamps

viewerShellPtr em_get_current_viewer()
{
    Initialize(em_get_current_viewer);
    return viewerShell::get_current_vs();
}

viewPtr em_get_current_view (viewerShellPtr viewer)
{
    Initialize(em_get_current_view);
    if (!viewer) return 0;
    return (viewer)->get_current_view();
}

Window em_get_view_window (viewerShellPtr viewer, viewPtr view)
{
    Initialize(em_get_view_window);
    Window win = 0;
    int trys = 0;

    if (!viewer || !view) return 0;

    em_flush_events();
    win = (Window) viewer->getDialog()->window_of_view (view);

    while (!win) {
       trys++;
       win = (Window) viewer->getDialog()->window_of_view (view);
    }

    return win;
}

Widget em_get_viewer_widget (viewerShellPtr viewer)
{
    Initialize(em_get_viewer_widget);
    return viewer->get_widget();
}

Widget em_get_viewer_menu (viewerShellPtr viewer)
{
    Initialize(em_get_viewer_menu);
    Widget viewerShell_widget = viewer->get_widget();
    return(XtNameToWidget(viewerShell_widget, "*form1.menubar"));
}

void *em_get_dialog (char *name)
{
    Initialize(em_get_dialog);
    return NULL;
}


void em_dump_window (Window window, char *file)
{
    Initialize(em_dump_window);
    char command[1024];

    sprintf (command, "xwd -id %d -out %s -xy", window, file);

    em_flush_events();

    v_system (command);
}

void em_flush_events()
{
    Initialize(em_flush_events);

//    XSync (UxDisplay, False);
   
    driver_instance->graphics_world->process_pending_events();
}

void em_query_tree (Widget w, int level)
{
    Initialize(em_query_tree);
    Window root;
    Window parent;
    Window *children;
    unsigned int n;
    Arg args[10];
    Widget submenu;


    if (!w) return;

    if (!XQueryTree (UxDisplay, XtWindow(w), &root, &parent, &children, &n))
	return;

    for (int i=0; i<n; i++) {
        Widget child = XtWindowToWidget(UxDisplay, children[i]);

        for (int k=0; k<level; k++) cout << ' ';
	cout << children[i] << " ";
	cout << (char*)XtName(child) << endl;
        int j = 0;
        XtSetArg(args[j], XmNsubMenuId, &submenu);
        XtGetValues (child, args, ++j);
        
        em_query_tree (submenu, level+1);
    }
}

void em_set_speed (int sp)
{
    Initialize(em_set_speed);
    if (sp < 0) sp = 0;
    speed = sp;
}

extern int em_get_speed ()
{
    Initialize(em_get_speed);
    return speed;
}

void em_moveto_widget(Widget w)
{
    Initialize(em_moveto_widget);
    if (w)
	em_moveto_window (XtWindow(w), 10, 10);
}

void em_moveto_window(Window window, int nx, int ny)
{
    Initialize(em_moveto_window);
    Window root, child;
    int rootx, rooty, x, y;
    unsigned int but;

    if (!window) 
	return;
    if (speed > 0) {
        XQueryPointer (UxDisplay, window, &root, &child,
	&rootx, &rooty, &x, &y, &but);
	int a = abs(x - nx);
	int b = abs(y - ny);
	int distance = (int) sqrt((double)((a*a)+(b*b)));
	(abs(x - nx) > abs(ny)) ? abs(nx) : abs(ny); 
        if (distance > 0) {
            for (int i = distance ; i > 0; i -= 5) {
                XWarpPointer (UxDisplay, None, window, 0, 0, 0, 0, 
			      (int)(x + (nx - x)*(distance - i)/distance), 
			      (int)(y + (ny - y)*(distance - i)/distance));
                em_flush_events();
	        OS_dependent::usleep (1000000/speed);
            }
        }
    }
    XWarpPointer (UxDisplay, None, window, 0, 0, 0, 0, nx, ny);
    em_flush_events();
}

/*_______________________________________________________________________
!
! Function:	em_send_event & em_send_detailed_event
!
! Arguments:	Window win
!		int event
!		unsigned int modifiers 
!		     Bit wise OR the following modifiers for held
!		     down keys:
!	     		Shift	0x0001
!	     		Lock	0x0002	
!	     		Control	0x0004	
!	     		Mod1	0x0008	(Alt or Meta)	
!	     		Mod2	0x0010	
!	     		Mod3	0x0020	
!	     		Mod4	0x0040
!	     		Mod5	0x0080
!		int mouse_button	between 1 and 3
!       
! Returns:	void
!
! Synopsis:	Passes each character in the string as an event to the 
!			passed window.		
!
_______________________________________________________________________*/ 
void em_send_detailed_event (Window win, int event, 
			     unsigned int modifiers = 0, 
			     int mouse_button = 1)
{
    Initialize(em_send_detailed_event);
    XEvent ev;
    OS_dependent::bzero((char*)&ev, sizeof(XEvent));
    if (!win) return;
    if (mouse_button < 1) 
	mouse_button = 1;
    if (mouse_button > 3) 
	mouse_button = 3;
    int button_number;
    switch(mouse_button) {
      case 1:
	button_number = Button1;
	break;
      case 2:
	button_number = Button2;
	break;
      case 3:
	button_number = Button3;
	break;
      case 4:
	button_number = Button4;
	break;
      case 5:
	button_number = Button5;
	break;
      default:
	button_number = Button1;
	break;
    }

    /* Construct an artifical timestamp in microseconds. */
    struct timeval time_values;
    struct timezone time_zones;

    OSapi_gettimeofday(&time_values, &time_zones);
    if (starttime == 0)
	starttime = time_values.tv_sec - 10;
    unsigned long timestamp = (time_values.tv_sec - starttime) * 1000 +
	(time_values.tv_usec / 1000);  /* timestamp in milliseconds */

    /* Get the cursor position. */
    Window root, child;
    int rootx, rooty, x, y;
    unsigned int but;
    XQueryPointer (UxDisplay, win, &root, &child,
		   &rootx, &rooty, &x, &y, &but);
    switch (event) {
    case PUSH:
	ev.xbutton.type = ButtonPress;
        ev.xbutton.serial = 0;
        ev.xbutton.send_event = FALSE;
        ev.xbutton.display = UxDisplay;
        ev.xbutton.window = win;
        ev.xbutton.root = win;
        ev.xbutton.subwindow = child;
        ev.xbutton.time = timestamp;
        ev.xbutton.x = x;
        ev.xbutton.y = y;
        ev.xbutton.x_root = rootx;
        ev.xbutton.y_root = rooty;
        ev.xbutton.state = modifiers;
  	ev.xbutton.button =  button_number;
        ev.xbutton.same_screen = True;

	XSendEvent (UxDisplay, win, True, 0, &ev);
	OS_dependent::usleep(100000);
	break;

    case RELEASE:
	ev.xbutton.type = ButtonRelease;
        ev.xbutton.serial = 0;
        ev.xbutton.send_event = FALSE;
        ev.xbutton.display = UxDisplay;
        ev.xbutton.window = win;
        ev.xbutton.root = win;
        ev.xbutton.subwindow = child;
        ev.xbutton.time = timestamp;
        ev.xbutton.x = x;
        ev.xbutton.y = y;
        ev.xbutton.x_root = rootx;
        ev.xbutton.y_root = rooty;
        ev.xbutton.state = modifiers;
  	ev.xbutton.button =  button_number;
        ev.xbutton.same_screen = True;

	XSendEvent (UxDisplay, win, True, 0, &ev);
	OS_dependent::usleep(100000);
	break;

    case CLICK:
        em_send_detailed_event (win, PUSH, modifiers, mouse_button);
        em_send_detailed_event (win, RELEASE, modifiers, mouse_button);
	break;
    }
}
void em_send_event (Window win, int event, int modifiers)
{
    Initialize(em_send_event);
    em_send_detailed_event(win, event, (unsigned int)modifiers, 1);
}



Widget em_get_menu_item (Widget menu, char* item_name)
{
    Initialize(em_get_menu_item);
    Arg  args[10];
    int i, j, n;
    WidgetList children;
    char *string, *name;

    i = 0;
    XtSetArg (args[i], XmNchildren, &children); i++;
    XtSetArg (args[i], XmNnumChildren, &n); i++;
    XtGetValues (menu, args, i);

    if (isdigit(*item_name)) {
        int index = atoi(item_name);
        if (index < 0 || index >= n)
	    return 0;
	else
	    return children[index];
    }

    for (i=0; i<n; i++) {
        Widget child = children[i];
        j = 0;
        string = 0;
        XtSetArg(args[j], XmNlabelString, &string); j++;
        XtGetValues (child, args, j);

        name = XtName(child); 

	if (name && strcmp(item_name, name) == 0)
	    return child;
	if (string && strcmp(item_name, string) == 0)
	    return child;
    }
    return NULL;   // error return
}

Widget em_get_item_submenu (Widget item)
{
    Initialize(em_get_item_submenu);
    Widget submenu;
    int i;
    Arg args[10];

    i = 0;
    XtSetArg(args[i], XmNsubMenuId, &submenu); i++;
    XtGetValues (item, args, i);

    return submenu;
}

Widget em_find_widget (Widget top, char *name)
{
    Initialize(em_find_widget);
    int i, n;
    WidgetList children;
    char *wname;
    Widget w;
    Arg args[10];

    if (!top)
	top = gtApplicationShellXm::find_existing();

    wname = XtName(top);
    if (strcmp(wname, name) == 0)
	return top;
    
    if (!XtIsComposite(top))
	return 0;

    w = 0;
    i = 0;
    XtSetArg (args[i], XmNchildren, &children); i++;
    XtSetArg (args[i], XmNnumChildren, &n); i++;
    XtGetValues (top, args, i);
    for (i=0; i<n; i++) {
	w = em_find_widget (children[i], name);
	if (w)
	    return w;
    }
    return w;
}

Widget em_lookup(char *name)
{
    Initialize(em_lookkup);
    return XtNameToWidget(gtApplicationShellXm::find_existing(), name);
}

Window em_get_window (Widget w)
{
    Initialize(em_get_window);
    return XtWindow(w);
}

void em_new_query_tree (Widget top, int level)
{
    Initialize(em_new_query_tree);
    WidgetList children;
    unsigned int n, i;
    char *wname;
    Arg args[10];

    if (!top) 
        return;

    wname = XtName(top);
    for (int k=0; k<level; k++) cout << ' ';
 	cout << wname << endl;

    if (!XtIsComposite(top))
    	return;

    // Search through composite children.
    i = 0;
    XtSetArg (args[i], XmNchildren, &children); i++;
    XtSetArg (args[i], XmNnumChildren, &n); i++;
    XtGetValues (top, args, i);
    for (i=0; i<n; i++) {
	em_new_query_tree (children[i], level + 1);
    }

    // Search through popup children.
    get_popup_data(top, children, n);
    for (i = 0; i < n; i++) {
	em_new_query_tree (children[i], level + 1);
    }
    return;
}


// get menubar widget for all TopShells including ViewerShell

Widget em_get_menubar (Widget top_shell)
{
    Initialize(em_get_menubar);
    return  em_find_widget(top_shell, "menubar");
}




//
// List manipulation commands
//


// For lists, items are at position 1, 2, 3, etc. but the last
// item is at position 0.

void em_add_item(Widget theWidget, char* item_name, int item_position) 
{
    Initialize(em_add_item);
    XmString special_string = XmStringCreateSimple(item_name);
    XmListAddItem(theWidget, special_string, item_position);
    XmStringFree(special_string);
}

void em_select_item(Widget theWidget, char* item_name, bool notify)
{
    Initialize(em_select_item);
    XmString special_string = XmStringCreateSimple(item_name);
    XmListSelectItem(theWidget, special_string, notify);
    XmStringFree(special_string);
}

void em_deselect_item(Widget theWidget, char* item_name) 
{
    Initialize(em_deselect_item);
    XmString special_string = XmStringCreateSimple(item_name);
    XmListDeselectItem(theWidget, special_string);
    XmStringFree(special_string);
}

void em_select_pos(Widget theWidget, int item_position, bool notify)
{
    Initialize(em_select_pos);
    XmListSelectPos(theWidget, item_position, notify);
}

void em_deselect_pos(Widget theWidget, int item_position) 
{
    Initialize(em_deselect_pos);
    XmListDeselectPos(theWidget, item_position);
}

void em_deselect_all(Widget theWidget) 
{
    Initialize(em_deselect_all);
    XmListDeselectAllItems(theWidget);
}

void em_delete_item(Widget theWidget, char* item_name) 
{
    Initialize(em_delect_item);
    XmString special_string = XmStringCreateSimple(item_name);
    XmListDeleteItem(theWidget, special_string);
    XmStringFree(special_string);
}

void em_delete_pos(Widget theWidget, int item_position) 
{
    Initialize(em_delete_pos);
    XmListDeletePos(theWidget, item_position);
}

void em_delete_items_at_pos(Widget theWidget, int number, int item_position) 
{
    Initialize(em_delete_items_at_pos);
    XmListDeleteItemsPos(theWidget, number, item_position);
}

int em_item_exists(Widget theWidget, char* item_name) 
{
    Initialize(em_item_exists);
    XmString special_string = XmStringCreateSimple(item_name);
    int result =  XmListItemExists(theWidget, special_string);
    XmStringFree(special_string);
    return result;
}


/*_______________________________________________________________________
!
! Function:	em_enter_string
!
! Arguments:	Window win
!		char* theString
!		unsigned int modifiers 
!		     Bit wise OR the following modifiers for held
!		     down keys:
!	     		Shift	0x0001
!	     		Lock	0x0002	
!	     		Control	0x0004	
!	     		Mod1	0x0008	(Alt or Meta)	
!	     		Mod2	0x0010	
!	     		Mod3	0x0020	
!	     		Mod4	0x0040
!	     		Mod5	0x0080
!       
! Returns:	void
!
! Synopsis:	Passes each character in the string as an event to the 
!			passed window.		
!
_______________________________________________________________________*/ 
void em_enter_string(Window win, char* theString, unsigned int modifier_keys = 0) {
    Initialize(em_enter_string);
    if ((theString) && (win)){
	
//	XSynchronize(UxDisplay, TRUE);

	char current_char = '\0';
	XEvent ev;
	int row_size = 0;
	KeySym key_symbol = 0;
	int min_keycode = 0;
	int max_keycode = 0;
	XDisplayKeycodes(UxDisplay, &min_keycode, &max_keycode);
	KeySym * key_table = XGetKeyboardMapping(UxDisplay, min_keycode, 
						 max_keycode - min_keycode, &row_size);
	int length = strlen(theString);
	
	/* send an event for each character. */
	for(int i = 0; i < length; i++) {
	    current_char = theString[i];

	    /* the auto_test parser recognizes a backslash and inserts */
	    /* a second backslash to negate it.  We must remove this. */
	    if (current_char == '\\') {
		if (i +1 < length) {
		    switch (theString[i+1]) {
		      case 'r':
		      case 'n':
			current_char = '\n';
			i += 1;
			break;
		      case 't':
			current_char = '\t';
			i += 1;
			break;
		      case 'b':
			current_char = '\b';
			i += 1;
			break;
		    }
		}		
	    }

	    /* Get the correct keycode and any additional modifier keys, such as <SHIFT>. */
	    /* See X.h */
	    /* ShiftMask	(1<<0)	*/
	    /* LockMask		(1<<1)	*/
	    /* ControlMask	(1<<2)	*/
	    /* Mod1Mask		(1<<3)	*/
	    /* Mod2Mask		(1<<4)	*/
	    /* Mod3Mask		(1<<5)	*/
	    /* Mod4Mask		(1<<6)	*/
	    /* Mod5Mask		(1<<7)	*/
	    /* Important note: The key map does not contain lower case letters and the */
	    /* upper case letters are in the first (unmodified) column.  So letters must */
	    /* be handled differently.  Try executing xmodmap -pk in a shell. */
	    unsigned int key_code = XKeysymToKeycode(UxDisplay, (KeySym)current_char);
	    bool AnEscapeChar = FALSE;
	    switch (current_char) {
	      case '\a' :
		break;
	      case '\b' : /* Backspace */
		key_code = XKeysymToKeycode(UxDisplay, (KeySym)0xFF08);
		AnEscapeChar = TRUE;
		break;
	      case '\f' :
		break;
	      case '\n' :
	      case '\r' : /* Return */
		key_code = XKeysymToKeycode(UxDisplay, (KeySym)0xFF0D);
		AnEscapeChar = TRUE;
		break;
	      case '\t' : /* Tab */
		key_code = XKeysymToKeycode(UxDisplay, (KeySym)0xFF09);
		AnEscapeChar = TRUE;
		break;
	      case '\v' :
		break;
	      case '\0' :
		break;
	      default:		
		break;
	    }
	    unsigned int additional_modifiers = 0;
	    if ((key_code >= min_keycode) && (key_code <= max_keycode)) {
		if(isalpha(current_char)) {
		    if(isupper(current_char))
			additional_modifiers = 1;
		} else if(!AnEscapeChar){
		    for(int column = 1; column < row_size; column++)  
			if(key_table[(key_code - min_keycode) * row_size + column] == current_char)
			    additional_modifiers |= 1<<(column - 1);
		}

		/* Get the time of day in microseconds. */
		struct timeval time_values;
		struct timezone time_zones;
		OSapi_gettimeofday(&time_values, &time_zones);
		if (starttime == 0)
		    starttime = time_values.tv_sec - 10;
		unsigned long timestamp = (time_values.tv_sec - starttime) * 1000 +
		    (time_values.tv_usec / 1000);  /* timestamp in milliseconds */

		/* Get the cursor position. */
		Window root = NULL, child = NULL;
		int rootx = 0, rooty = 0, x= 0, y = 0;
		unsigned int but = 0;
		XQueryPointer (UxDisplay, win, &root, &child,
			       &rootx, &rooty, &x, &y, &but);
    
		/* Send the event. */
		ev.xkey.type = KeyPress; 
		ev.xkey.serial = 0; /* # of last request processed by server */
		ev.xkey.send_event = FALSE; /* true if this came from a SendEvent request */
		ev.xkey.display = UxDisplay; /* Display the event was read from */
		ev.xkey.window = win;	     /* "event" window it is reported relative to */
		ev.xkey.root = win;	     /* root window that the event occured on */
		ev.xkey.subwindow = child;     /* child window */
		ev.xkey.time = timestamp;
		ev.xkey.x = x; 
		ev.xkey.y = y;	/* pointer x, y coordinates in event window */
		ev.xkey.x_root = rootx; 
		ev.xkey.y_root = rooty; /* coordinates relative to root */
		ev.xkey.state = (modifier_keys | additional_modifiers);
		ev.xkey.keycode = key_code; /* detail */
		ev.xkey.same_screen = True; /* same screen flag */
	
       		XSendEvent (UxDisplay, child, True, 0, &ev);

		OSapi_usleep(1000); /* insure that the timestamp changes (just in case) */
		OSapi_gettimeofday(&time_values, &time_zones);
		timestamp = (time_values.tv_sec - starttime) * 1000 +
		    (time_values.tv_usec / 1000);  /* timestamp in milliseconds */

		ev.xkey.type = KeyRelease; 
		ev.xkey.serial = 0; /* # of last request processed by server */
		ev.xkey.send_event = FALSE; /* true if this came from a SendEvent request */
		ev.xkey.display = UxDisplay; /* Display the event was read from */
		ev.xkey.window = win;	     /* "event" window it is reported relative to */
		ev.xkey.root = win;	     /* root window that the event occured on */
		ev.xkey.subwindow = child;     /* child window */
		ev.xkey.time = timestamp;
		ev.xkey.x = x; 
		ev.xkey.y = y;	/* pointer x, y coordinates in event window */
		ev.xkey.x_root = rootx; 
		ev.xkey.y_root = rooty; /* coordinates relative to root */
		ev.xkey.state = (modifier_keys | additional_modifiers);
		ev.xkey.keycode = key_code; /* detail */
		ev.xkey.same_screen = True; /* same screen flag */
		
		XSendEvent (UxDisplay, win, True, 0, &ev);
	    }
	}

//	XSynchronize(UxDisplay, FALSE);

    }
}



/*_______________________________________________________________________
!
! Member:	em_set_string_editor
!
! Arguments:	Widget widget 
!		const char *contents 
!       
! Synopsis:	Replaces the string in a gtStringEditor widget.
!
_______________________________________________________________________*/ 
void em_set_string_editor(Widget widget, const char *contents)
{
    Initialize(em_set_string_editor);

    if((!widget) || (!contents))
	return;
    Arg arg[1];
    XtSetArg(arg[0], XmNvalue, contents);
    XtSetValues(widget, arg, 1);
}



/*_______________________________________________________________________
!
! Member:	em_name_to_nth_child
!
! Arguments:	Widget parent 
!		char *child_name
!		int occurance		1 or higher
!
! Returns:	Widget			NULL if unsucessful.
!       
! Synopsis:	Replaces the string in a gtStringEditor widget.
!
_______________________________________________________________________*/ 
Widget em_name_to_nth_child(Widget parent, char* child_path, 
			    int occurance = 1)
{ 
    Initialize(em_name_to_nth_child);
    WidgetList children;
    unsigned int num_children, i, count;
    Arg args[10];

    // Handle bad input.
    if ((!parent) || (!XtIsComposite(parent)) || (occurance < 1)
	|| (!child_path))
        return(NULL);

    // Get the child widget's short name, (no path). 
    Widget named_widget = XtNameToWidget(parent, child_path);
    if (named_widget) {
	char* child_name = XtName(named_widget); 

	// Get the nth child of the same name.
	i = 0;
	XtSetArg (args[i], XmNchildren, &children); i++;
	XtSetArg (args[i], XmNnumChildren, &num_children); i++;
	XtGetValues (parent, args, i);
	
	// Search through composite (normal) children
	for (count = i = 0; i < num_children; i++) {
	    if(strcmp(child_name, XtName(children[i])) == 0)
		count++;
	    if (count == occurance) 
		return(children[i]);
	}
	
	// Search through popup children.
	get_popup_data(parent, children, num_children);
	for (i = 0; i < num_children; i++) {
	    if(strcmp(child_name, XtName(children[i])) == 0)
		count++;
	    if (count == occurance) 
		return(children[i]);
	}
    }
    return(0);
}

/*_______________________________________________________________________
  !
  ! Member:	em_name_to_nth_widget
  !
  ! Arguments:	Widget parent 
  !		char *widget_path
  !		int occurance		1 or higher
  !
  ! Returns:	Widget			NULL if unsucessful.
  !       
  ! Synopsis:	Given a tree with multiple leaves, under one branch, 
  !		having the same name, the function finds the nth leaf.  
  !
  _______________________________________________________________________*/ 
Widget em_name_to_nth_widget(Widget top, char* widget_path, 
			     int occurance = 1)
{
    Initialize(em_name_to_nth_widget);
    if((top) && (widget_path)) {
	Widget named_widget = XtNameToWidget(top, widget_path);
	if (named_widget) {
	    Widget parent = XtParent(named_widget);
	    char child_name[512];
	    strcpy(child_name, XtName(named_widget));
	    return em_name_to_nth_child(parent, child_name, occurance);
	}
    }
    return(0);
}


/*_______________________________________________________________________
  !
  ! Member:	em_indexed_path_to_widget
  !
  ! Arguments:	Widget top
  !		char *widget_path
  !		int index_num		Child number for the last
  !					widget name in widget_path.
  !					Ignored if widget_path ends
  !					with an index.
  !
  ! Returns:	Widget			NULL if unsucessful.
  !       
  ! Synopsis:	Function finds a widget given a widget given a path
  !		with indicies of the form "(n)" specifying the
  !		nth sibling of the same name. 
  !
  !		Normal widget paths:
  !			"*browser*file_browser.hbox.form.shadow"
  !			"*preferences.form*frame*form.string_editor"
  !
  !		Indexed widget paths:
  !			"*browser*file_browser(2).hbox.form.shadow"
  !			"*preferences.form*frame(2)*form(3).string_editor"
  !
  _______________________________________________________________________*/ 
Widget em_indexed_path_to_widget(Widget top, char* widget_path, int index_num)
{
    Initialize(em_indexed_path_to_widget);
    if ((!widget_path) || (!top))
	return(0);
    
    int length = strlen(widget_path);
    Widget current_highest_widget = top;
    char* path_copy = new char[length + 1];
    strcpy(path_copy, widget_path);
    char* path_token = new char[length + 1];
    strcpy(path_token, widget_path);
    char* index_token = new char[length + 1];
    strcpy(index_token, "\0");
    char* frontPtr = path_copy;
    char* backPtr = path_copy;
    bool done = FALSE;
    bool error_occured = FALSE;
    
    // Find each path, index pair.  Pass them to em_name_to_nth_widget.
    while ((!done) && (!error_occured)) {
	frontPtr = strchr(backPtr, '(');
	if (frontPtr == NULL){
	    
	    // No indicies found, separate the path token.
	    strcpy(path_token, backPtr);
	    
	    // Goto the child specified by index_num.
	    // This is really added here for the auto test
	    // tool to get to other BrowserShells, project browsers, etc.
	    current_highest_widget = em_name_to_nth_widget(current_highest_widget,
							   path_token,
							   index_num);
	    done = TRUE;
	} else {
	    
	    // Separate the path token
	    strncpy(path_token, backPtr, (frontPtr - backPtr));
	    path_token[(frontPtr - backPtr)] = '\0';
	    
	    // Separate the index token 
	    if (strlen(frontPtr) > 1) {
		frontPtr++;
		backPtr = frontPtr;
		frontPtr = strchr(backPtr, ')');
		if (frontPtr != NULL) {
		    strncpy(index_token, backPtr, (frontPtr - backPtr));
		    index_token[(frontPtr - backPtr)] = '\0';
		    int child_num = atoi(index_token);
		    
		    // Change top level widget.
		    current_highest_widget = em_name_to_nth_widget(current_highest_widget,
								   path_token,
								   child_num);
		    if (!current_highest_widget)
			error_occured = TRUE;
		    frontPtr++;
		    if (*frontPtr == '.')
			*frontPtr = '*';
		    backPtr = frontPtr;
		    if (*frontPtr == '\0')
			done = TRUE; // There is nothing after the last '\)'
		} else 
		    error_occured = TRUE;
	    } else 
		error_occured = TRUE;
	}
    }
    
    free(path_copy);
    free(path_token);
    free(index_token);
    
    if (error_occured)
	return(0);
    return(current_highest_widget);
}

/*_______________________________________________________________________
!
! Member:	em_get_child
!
! Arguments:	Widget parent 
!		char *child_name
!		int occurance		1 or higher
!
! Returns:	Widget			NULL if unsucessful.
!       
! Synopsis:	Replaces the string in a gtStringEditor widget.
!
_______________________________________________________________________*/ 
Widget em_get_child(Widget parent, char* child_name, 
		    int occurance = 1)
{ 
    Initialize(em_name_to_nth_child);
    WidgetList children;
    unsigned int num_children, i, count;
    Arg args[10];

    // Handle bad input.
    if ((!parent) || (!XtIsComposite(parent)) || (occurance < 1)
	|| (!child_name))
        return(NULL);

    
    // Get the nth child of the same name.
    i = 0;
    XtSetArg (args[i], XmNchildren, &children); i++;
    XtSetArg (args[i], XmNnumChildren, &num_children); i++;
    XtGetValues (parent, args, i);
    
    // Search through composite (normal) children
    for (count = i = 0; i < num_children; i++) {
	if(strcmp(child_name, XtName(children[i])) == 0)
	    count++;
	if (count == occurance) 
	    return(children[i]);
    }	
    
    // Search through popup children.
    get_popup_data(parent, children, num_children);
    for (i = 0; i < num_children; i++) {
	if(strcmp(child_name, XtName(children[i])) == 0)
	    count++;
	if (count == occurance) 
	    return(children[i]);
    }	
    return(0);
}	


/*
  START-LOG-------------------------------------------
  
  $Log: em.C  $
  Revision 1.5 2000/07/10 23:03:40EDT ktrans 
  mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.22  1994/02/11  15:50:00  builder
Port

Revision 1.2.1.21  1993/12/16  23:00:39  jon
Bug track: -
Oops, fixed a type with the definition of em_get_speed()

Revision 1.2.1.20  1993/12/16  19:40:58  jon
Bug track: -
Added em_get_speed so I could check the speed value in cmd.C

Revision 1.2.1.19  1993/07/20  18:17:41  harry
HP/Solaris Port

Revision 1.2.1.18  1993/05/25  13:52:16  jon
Sped up the cursor for pushing buttons

Revision 1.2.1.17  1993/05/24  17:10:48  jon
Fixed timestamp for emulating keystrokes
Compute a more sensible distance for pointer warping

Revision 1.2.1.16  1993/05/20  16:33:55  jon
Fixed timestamp for generated events

Revision 1.2.1.15  1993/05/18  22:04:24  jon
Zeroed out rest of event record before sending

Revision 1.2.1.14  1993/04/27  16:10:12  davea
bug 3510 - Add return to em_get_menu_item

Revision 1.2.1.13  1993/02/15  21:38:09  oak
Small changes.

Revision 1.2.1.12  1993/01/15  20:30:18  oak
Fixed small bug with em_moveto_window.

Revision 1.2.1.11  1993/01/14  15:10:17  oak
Minor changes.

Revision 1.2.1.10  1993/01/11  16:29:33  oak
Major changes to auto_test using routines.

Revision 1.2.1.9  1993/01/04  16:18:13  oak
Added new error checking.

  Revision 1.2.1.8  1992/12/18  18:49:52  oak
  Added em_indexed_path_to_widget.
  
  
  END-LOG---------------------------------------------
  */

