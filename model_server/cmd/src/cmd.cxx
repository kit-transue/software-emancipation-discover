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
// cmd.C
//------------------------------------------
// synopsis:
//    A group of calls for simulating menu related UI functions
//
// description:
// ...
//------------------------------------------
// 
// Restrictions:
// ...
//------------------------------------------

// include files
#include "cLibraryFunctions.h"

#include "representation.h"
#include "genError.h"
#include "cmd_internal.h"
#include "viewerShell.h"
#include "viewList.h"
#include "gtAppShellXm.h"
#include "cmd_shell.h"
#include "em.h"
#include "ste_interface.h"

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Core.h>
#include <X11/Composite.h>

#include <Xm/List.h>
#include <Xm/ArrowB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>

#include "viewerShell.h"
#include "browserShell.h"

#include "machdep.h"
#include "msg.h"
#include "steScreen.h"
#include "assist.h"
#include "HelpUI.h"

#define XK_Return 96

static int epoch_macro_recording = 0;
static int last_help_was_browser;

int cmd_is_epoch_recording ()
{
    return epoch_macro_recording;
}

// returns old flag value;
int cmd_set_epoch_recording (int new_val)
{
    int old_val = epoch_macro_recording;
    epoch_macro_recording = new_val;
    return old_val;
}

int ste_get_last_command ();

int cmd_is_no_epoch_macro ()
{
    int res = 1;
    if (epoch_macro_recording) {
	int last_comm = ste_get_last_command ();
	if (last_comm > -1) {
	    if (last_comm == 64 || last_comm == 1) // mouse or 
		res = 1;
	    else
		res = 0;
	}
    }
    return res;
}

extern void epoch_report_current_view (viewPtr, steScreenPtr);

class appTree;
typedef appTree* appTreePtr;

extern "C"
{
    appTreePtr C_node(appType, char*, ...);
}

int cmd_exec_next_stmt();
extern int em_get_speed();

static Widget last_text = 0;
static Widget last_widget = 0;

static Widget top_level_widget = 0;
static Widget composite_widget = 0;
static Widget viewer_widget = 0;

static int help_tracking_on = 0;
static int tracking_on = 0;
static int tracked = 0;
static int track_button = 0;
static int track_first = 0;
static int track_tossfirst = 0;
static Widget last_label = 0;  // for tracking
static int is_a_list = 0;
static int is_text;
static char *top_level_wg_name = 0;

static const char *cmdNoView = "No View";
static int skip_button_post = 0;

void cmd_gra_popup_begin(Widget)
{
 if (cmd_current_journal_name){
  
   tracking_on = 1;
   tracked = 0;
   last_label = 0;  // for tracking
   track_button = 3;
   track_first = 0;
   tracked = 0;
   track_tossfirst = 0;
 }
}

extern "C" int synchronize_now(char*)
{
 return
  cmd_exec_pause();
}

void cmd_synchronize(const char * prompt)
{
    if(cmd_current_journal_name)
	cmd_gen_write_force("synchronize_now(%s);", prompt);
    else if (cmd_execute_journal_name)
	cmd_exec_resume();
}


void cmd_synchronize_parser (const char* prompt)
{
	cmd_synchronize (prompt);
}

int find_top_widget(Widget w);
Widget find_top_widget(char * name, int ind);

void cmd_init()
{
    last_text = 0;
    last_widget = 0;
    top_level_widget = 0;
    composite_widget = 0;
    viewer_widget = 0;
}

extern "C" void cmd_do_flush()
{
    cmd_flush();
    if (UxDisplay == 0) {
    	return;
    }
    XSync(UxDisplay, 0);
    XFlush(UxDisplay);
    XSync(UxDisplay, 0);
    XFlush(UxDisplay);
    while (XtAppPending(UxAppContext))
    {
//      cout << "Snagged one!" << endl;
	cmd_flush();
	XSync(UxDisplay, 0);
	XFlush(UxDisplay);
	XSync(UxDisplay, 0);
	XFlush(UxDisplay);
    }
}

extern "C" void cmd_list_cr(Widget wg)
{
  cmd_do_flush();

  XEvent ev;

  ev.type = KeyPress;

  ev.xkey.window = XtWindow(wg);
  ev.xkey.display = XtDisplay(wg);

  ev.xkey.type = KeyPress;
  ev.xkey.keycode = XK_Return;

  XtCallActionProc(wg, "ListKbdActivate", &ev, NULL, 0);
  cmd_do_flush();
}

extern "C" void cmd_text_cr(Widget wg)
{
    cmd_do_flush();
    em_enter_string(XtWindow(wg), (char *)"\n", 0);

    cmd_do_flush();
}

static Widget find_numbered_child(Widget root, char* name, int number)
{
//    cmd_do_flush();
    WidgetList wl;
    int no_ch;

    XtVaGetValues(root, XtNchildren, &wl, XtNnumChildren, &no_ch, 0);

    for(int ii=0; ii<no_ch; ++ii){
	Widget ob = wl[ii];

	String wnm = XtName(ob);
	if (!strcmp(name, wnm) && (!--number))
	    return ob;
    }
    return 0;
}

static Widget get_widget(Widget root, char* name)
{
    char* buff = strdup(name);
    char* curr = buff;
    Widget wg = NULL;
    char* single;
    char* multi;
    char* num;
    int first = 1;
    int finished = 0;

    while (curr /*&& *curr*/)
    {
	if (wg)
	    root = wg;
	wg = NULL;
	single = NULL;
	multi = NULL;
	char* hash = strchr(curr, '#');
	if (hash)
	{
	    *hash = NULL;
	    char* dot = strrchr(curr, '.');
	    if (dot)		// name...name#num...
	    {
		*dot = NULL;
		multi = dot +1;
		single = curr;
	    } else {		// name#num...
		multi = curr;
		single = NULL;
	    }
	    num = hash + 1;
	    char* end = strchr(num, '.');
	    if (end) {          // more to follow
		*end = NULL;
		curr = end + 1;
	    } else
	    {
		curr = NULL;
		finished = 1;
	    }
	} else {		// name...
	    single = curr;
	    multi = NULL;
	    curr = NULL;
	    finished = 1;
	}

	if (single)
	{
	    if (first)
	    {
		genString tmp("*.");
		tmp += single;
		wg = XtNameToWidget(root, (char*)tmp);
	    } else
	    {
		if (*single)
		    wg = XtNameToWidget(root, single);
		else  // Searching for "", but need to trick XtNameToWidget
		    wg = XtNameToWidget(root, ".");
	    }
	    if (!wg)
		break;
	}
	first = 0;

	if (multi)
	{
	    if (wg)
		root = wg;
	    int child_num = OSapi_atoi(num);
	    wg = find_numbered_child(root, multi, child_num);
	    if (!wg)
		break;
	}
	if (finished) break;
    }
    free(buff);

    return wg;
}

static Widget save_top_level;
static Widget save_viewer;
void cmd_save_env()
{
   save_viewer = viewer_widget;
   save_top_level = top_level_widget;
}
void cmd_restore_env()
{
   viewer_widget = save_viewer;
   top_level_widget = save_top_level;
}

Widget cmd_get_widget(char* name)
{
    Initialize(cmd_get_widget);
    Widget wg = 0;
    if (viewer_widget)
	wg = get_widget(viewer_widget, name);
    if (!wg)
	wg = get_widget(top_level_widget, name);

    if (!wg)
 {
	genString err;
	err.printf ("could not look up widget: %s", name);
	cmd_validation_error(err);
    }

    return wg;
}

// manually moves cursor to appropriate place in viewer window

void cmd_move_view_arrow(viewPtr vp, int x, int y)
{
    viewerShell *vsh = viewerShell::viewerShell_of_view (vp);
    viewer *viewer_of_vp = (vsh) ? vsh->viewer_of_view (vp) : NULL;

    if (viewer_of_vp) {
	Widget v_wg = viewer_of_vp->get_widget();
	Window win = viewer_of_vp->viewer::get_win_behind_epoch();
	em_moveto_window(win, x, y);
    }
}

void cmd_moveto_center_gadget(Widget item, Widget ga)
{
      Window win = XtWindow(item);
      int nx, ny;

// This assumes the returned values will be in pixels, not something like mm/100.
             Position  x, y;
             Dimension h, w;
             XtVaGetValues(ga, 
                XmNx, &x, XmNy, &y, XmNheight, &h, XmNwidth, &w, 0);
      nx = x +  w / 2;
      ny = y +  h / 2;            

      em_moveto_window(win, nx, ny);
}

void cmd_moveto_center_widget(Widget item)
{
      Window win = XtWindow(item);
      Window root;
     int nx=0, ny=0;

     int x, y;
      unsigned int w, h;
      unsigned int border, depth;
      XGetGeometry(UxDisplay, win, &root, &x, &y, &w, &h, &border, &depth);
//             Position  x, y;
//             Dimension h, w;
//             XtVaGetValues(item,
//                XmNx, &x, XmNy, &y, XmNheight, &h, XmNwidth, &w, 0);
      nx = w / 2;
      ny = h / 2;            
      em_moveto_window(win, nx, ny);
}

// MakePosVisible positions the list so that the item is always visible 
// before selecting.
// Return values:           item_no -> index of item is correct
//                          1 -> list was scrolled down & index set to 1  
//                          visible -> list was scrolled up & index set to
//                                     visible
 
int MakePosVisible(Widget wg, int item_no)
{
    int top, visible, ret_val = 0;
    
    XtVaGetValues(wg, XmNtopItemPosition, &top, XmNvisibleItemCount, &visible, 0);
    if (item_no < top) {
	XmListSetPos(wg, item_no);
	ret_val = 1;
    }
    else if (item_no >= (top + visible - 1)) {
	XmListSetBottomPos(wg, item_no);
	ret_val = visible;
    }
    // this used to return just item_no, and script would crash for selection
    // to item next to last in third collumn.
    else ret_val = item_no - top + 1;
    return ret_val;
}
    
// cmd_moveto_list_item manually moves the cursor to the selected item
// in the list widget.

void cmd_moveto_list_item(Widget item, int item_index, XmFontList fontlist)
{
    Window win = XtWindow(item);
    Window root;
    int nx = 0, ny = 0;
    XmStringTable items;
    int size_of_one, new_index, top, total = 0;
    int x = 0, y = 0;
    unsigned int w, h;
    unsigned int border, depth;
    XGetGeometry(UxDisplay, win, &root, &x, &y, &w, &h, &border, &depth);
    nx = w / 2;   
    new_index = MakePosVisible(item, item_index);
    XtVaGetValues(item, XmNitems, &items, XmNtopItemPosition, &top, 0);
    for (int i = top; i < (top + new_index); i++) {
	size_of_one = XmStringHeight(fontlist, items[i - 1]); 
 	total += size_of_one + 3;
    }
    ny = total - (size_of_one / 2);
    em_moveto_window(win, nx, ny);
}

extern "C" void cmd_push_button(Widget item)
{
    cmd_do_flush();
    cmd_moveto_center_widget(item);
    cmd_do_flush();
    int speed = em_get_speed();
    //if (speed > 0)
	OS_dependent::usleep(1000);
    em_send_event (XtWindow(item), CLICK, 0);
    //if (speed > 0)
	OS_dependent::usleep(1000);
    cmd_do_flush();
}

extern "C" void cmd_button_event(Widget item, int button, int ev_type)
{
      em_moveto_widget(item);

      if(ev_type == 0) return;

      int evt = CLICK;
      switch(ev_type) {
	case 1:
	  evt = PUSH;
	  break;
	case -1:
	  evt = RELEASE;
	  break;
	default:
	  evt = CLICK;
	  break;
      }

      em_send_detailed_event (XtWindow(item), evt, 0, button);
}


extern "C" void cmd_push_gadget(Widget item, Widget ga)
{
      cmd_do_flush();
      cmd_moveto_center_gadget(item, ga);
      Window win = XtWindow(item);
      em_send_event (win, CLICK, 0);
      cmd_do_flush();
}

int cmd_select_list(Widget wg, char *text, int multiple)
{
    cmd_do_flush();
    int found_it = 0;
    XmStringTable items;
    XmFontList fontlist;
    int itemCount;
    XtVaGetValues(wg, XmNitemCount, &itemCount, XmNitems, &items,  
		  XmNfontList, &fontlist, 0);
    for (int i = 0; i < itemCount; i++)
    {
	char* one_item;
	XmStringGetLtoR(items[i], XmSTRING_DEFAULT_CHARSET, &one_item);
        char * ttt = one_item;
	while (*ttt && isicon (*ttt))
		   ttt++;
        if(strcmp(text, ttt) == 0){
            cmd_moveto_list_item(wg, i +1, fontlist);
	    
	    cmd_do_flush();
	    int modifier = multiple ? 4 : 0; // calculate modifier
	    int speed = em_get_speed();
	    //if (speed > 0)
		OS_dependent::usleep(1000);
	    em_send_event(XtWindow(wg), CLICK, modifier); // 4 indicates 
	    //if (speed > 0)
		OS_dependent::usleep(1000);                 // CONTROL bit set
	    cmd_do_flush();
	    
	    XtFree(one_item);
	    found_it = 1;
	    break;
	}
	XtFree(one_item);
    }
    cmd_do_flush();
    return found_it;
}

extern "C" Widget ui_select_list(char * name, char *text)
{
  cmd_do_flush();
  Widget wg = cmd_get_widget(name);

  if(! wg) 
 return 0;
  
  XmListDeselectAllItems(wg);
  int res =  cmd_select_list(wg, text, 0);

  if (res == 0) {
      genString err;
      err.printf ("Failed selecting. Domain: %s, Item: %s", name, text);
      cmd_validation_error (err);
  } else
  cmd_do_flush();

  return wg;
}

extern "C" Widget ui_addselect(char * name, char *text)
{
  cmd_do_flush();
  Widget wg = cmd_get_widget(name);

  if(! wg) return 0;

  int res =  cmd_select_list(wg, text, 1);
  if (res == 0) {
      genString err;
      err.printf ("Failed selecting. Domain: %s, Item: %s", name, text);
      cmd_validation_error (err);
  } else
  cmd_do_flush();

  return wg;
}

extern "C" Widget ui_list_action(char* name)
{
  if(cmd_current_journal_name)  // -rr
    cmd_gen_write_force("ui_list_action(%s);", name);

    cmd_do_flush();
    Widget wg = cmd_get_widget(name);
    if (!wg) return 0;
    cmd_list_cr(wg);
    cmd_do_flush();
    return wg;
}

Widget cmd_find_label(Widget wg, char * ttt);
Widget cmd_find_gadget(Widget wg, char * ttt);

extern "C" Widget ui_select_label(char * name, char *text, int ev_type)
{
  Initialize(ui_select_label);

  cmd_do_flush();
  Widget wg = cmd_get_widget(name);
  IF(! wg) return 0;
  
  Widget lb = cmd_find_label(wg, text);
  IF(!lb) return 0;
  cmd_button_event(lb, 1, ev_type);
  
  cmd_do_flush();

  return lb;
}

extern "C" Widget ui_select_gadget(char * name, char *text)
{
 Initialize(ui_select_gadget);

  cmd_do_flush();

  Widget wg = cmd_get_widget(name);
  IF(! wg) return 0;
  
  Widget ga = cmd_find_gadget(wg, text);

  IF(!ga) return 0;
  cmd_push_gadget(wg, ga);
  
  cmd_do_flush();

  return ga;
}

static int is_button_event;
static int is_push_button;

extern "C" Widget ui_push_button(char * name)
{
  cmd_do_flush();
  Widget wg = cmd_get_widget(name);
  if(! wg) return 0;

is_push_button = 1;
  cmd_push_button(wg);
  cmd_do_flush();
is_push_button = 0;
  return wg;
}

extern "C" Widget ui_button_event(char * name, int button, int ev_type)
{
  cmd_do_flush();
  Widget wg = cmd_get_widget(name);
  if(! wg) return 0;
  
is_button_event = 1;
  cmd_button_event(wg, button, ev_type);
  cmd_do_flush();
is_button_event = 0;
  return wg;
}

extern "C" Widget ui_select_top(char * name, int ind)
 { 
  if(cmd_current_journal_name)
    cmd_gen_write_force("ui_select_top(%s, %d);", name, ind);
  cmd_do_flush();
  Widget tpw =  find_top_widget(name, ind);
  if(tpw)
  {
      top_level_widget = tpw;
      top_level_wg_name = XtName(top_level_widget);
      viewer_widget = 0;
      XWindowChanges changes;
      changes.stack_mode = TopIf;
      XReconfigureWMWindow(XtDisplay(tpw), XtWindow(tpw),
			   DefaultScreen(XtDisplay(tpw)),
			   CWStackMode, &changes);
      XSetInputFocus(XtDisplay(tpw), XtWindow(tpw),
		     RevertToParent, CurrentTime);
  }
  else
      if(cmd_execute_journal_name){ // If journal is running, but widget is not found
	  genString msg;
	  
	  msg.printf("Widget: %s not found.", name);
	  cmd_validation_error((char *)msg);
      }
  cmd_do_flush();
  return tpw;
}

extern "C" Widget  ui_replace_text(char * name, char * text, int cr)
{
    cmd_do_flush();
    Widget wg = cmd_get_widget(name);
    if(!wg) return 0;

    ui_push_button(name);
    
    XmProcessTraversal(wg, XmTRAVERSE_CURRENT);
 
    XmIsText(wg) ? XmTextSetString(wg, text) : XmTextFieldSetString(wg, text);
    
    cmd_do_flush();

    if(cr)
         cmd_text_cr(wg);

    cmd_do_flush();
    
    return wg;
}

Widget cmd_find_label(Widget wg, char * ttt)
{
         WidgetList wl;
          int no_ch;

      XtVaGetValues(wg, XtNchildren, &wl, XtNnumChildren, &no_ch, 0);

      for(int ii=0; ii<no_ch; ++ii){
          Widget ob = wl[ii];

          if (! XmIsLabel(ob)) continue;
          XmString label;
          char * text = 0;
          XtVaGetValues(ob, XmNlabelString, &label, 0);
          if(! label) continue;

          XmStringGetLtoR(label, XmSTRING_DEFAULT_CHARSET, &text);
          char *p = text;
          if (strchr(ttt,'/') == 0)
          {
              p = strrchr(text,'/');
              if (!p)
                  p = text;
              else
                  p++;
          }

          if(strcmp(p, ttt) == 0)
	  {
	      XtFree(text);
	      return ob;
	  }
	  XtFree(text);
      }
      return 0;
}

Widget cmd_find_gadget(Widget wg, char * ttt)
{
         WidgetList wl;
          int no_ch;

      XtVaGetValues(wg, XtNchildren, &wl, XtNnumChildren, &no_ch, 0);

      for(int ii=0; ii<no_ch; ++ii){
          Widget ob = wl[ii];

//          if (! XmIsGadget(ob)) continue;

          char * text = XtName(ob);

          if(text && strcmp(text, ttt) == 0)
               return ob;
      }
      return 0;
}

extern "C" void prt_widget(XButtonEvent & be, Widget wg){
   if(XtIsComposite(wg)){
          WidgetList wl;
          int no_ch;
          
          
          XtVaGetValues(wg, XtNchildren, &wl, XtNnumChildren, &no_ch, 0);
          for(int ii=0; ii<no_ch; ++ii){
             Widget ob = wl[ii];
             int isrect = XtIsRectObj(ob);
             if(! isrect) continue;

             char * on = XtName(ob);
             msg(" $1", normal_sev) << on << eom;

             Position  x, y;
             Dimension h, w;
             XtVaGetValues(ob, 
                XmNx, &x, XmNy, &y, XmNheight, &h, XmNwidth, &w, 0);
            
             if(be.x >= x && be.x <= x+w && be.y >= y && be.y <= y+h) {
                 msg("Error in prt_widget", catastrophe_sev) << eom;
             }
             prt_widget(be, ob);

          }
     } else if (XmIsLabel(wg)){
          XmString label;
          char * text = 0;
          XtVaGetValues(wg, XmNlabelString, &label, 0);
          if(label){
            XmStringGetLtoR(label, XmSTRING_DEFAULT_CHARSET, &text);
          }
          msg("($1)", normal_sev) << text << eom;
     }
}

Widget cmd_find_gadget(XButtonEvent & be, Widget wg){
   if( XtIsComposite(wg)){
          WidgetList wl;
          int no_ch;
          
          
          XtVaGetValues(wg, XtNchildren, &wl, XtNnumChildren, &no_ch, 0);
          for(int ii=0; ii<no_ch; ++ii){
             Widget ob = wl[ii];
             int isrect = XtIsRectObj(ob);
             if(! isrect) continue;

             Position  x, y;
             Dimension h, w;
             XtVaGetValues(ob, 
                XmNx, &x, XmNy, &y, XmNheight, &h, XmNwidth, &w, 0);
            
             if(be.x >= x && be.x <= x+w && be.y >= y && be.y <= y+h) {
               return ob;  
             }
          }
     }
     return 0;
}

static int count_children_with_name(Widget wg, char* name, Widget cwg, int& pos)
{
    WidgetList children;
    int count = 0;
    int num_ch;
    XtVaGetValues(wg, XtNchildren, &children, XtNnumChildren, &num_ch, 0);
    pos = 0;

    for(int ii=0; ii<num_ch; ++ii){
	Widget child = children[ii];
	String cname = XtName(child);
	if (!strcmp(cname, name))
	    count++;
	if (child == cwg)
	    pos = count;
    }

    return count;
}

static int choose_name(Widget root, Widget wg, genString& pname)
{
    int wg_found = 0;
    Widget curr;
    pname = NULL;

    for (curr = wg; curr; curr = XtParent(curr)) {
	if (curr == root)
	    break;
	String name = XtName(curr);
	int pos;
	Widget parent = XtParent(curr);
	int num_twins;
	if (parent)
	    num_twins = count_children_with_name(parent, name, curr, pos);
	else
	    num_twins = 1;
	genString id = name;
	if (num_twins > 1)  // This guy needs additional identification
	{
	    char buf[11];
	    OSapi_sprintf(buf, "#%d", pos);
	    id += buf;
	}

	if (curr == wg)
	    pname = id;
	else {
	    genString tmp = pname;
	    pname = id;
	    pname += ".";
	    pname += tmp;
	}

	wg_found = (wg==get_widget(root, (char*)pname));
	if(wg_found) break;
    }
    return wg_found;
}

extern "C" int cmd_choose_name(Widget wg, genString& pname)
{
    int wg_found = 0;
    if (viewer_widget)
	wg_found = choose_name(viewer_widget, wg, pname);
    if (!wg_found)
	wg_found = choose_name(top_level_widget, wg, pname);

    if(!wg_found)
	msg("**cmd_choose_name: could not choose name $1", error_sev) << (char*)pname << eom;

    return wg_found;
}

extern "C" void cmd_record_top_level(Widget wg)
{
    Widget top_lev = 0;
    Widget par = wg;
    for (par = wg; (par && XtParent(par)); par = XtParent(par))
	if (find_top_widget(par))
	{
	    top_lev = par;
	    break;
	}
    if (!top_lev)
	return;
    char * top_name = XtName(top_lev);
    
    if(top_lev != top_level_widget){
	int top_ind = find_top_widget(top_lev);

	int top_found = 0;
	if(top_ind){
	    Widget tpw = find_top_widget(top_name, top_ind);
	    top_found = (top_lev == tpw);
	}
	if (top_found)
	{
	    cmd_gen_write_force("ui_select_top(%s, %d);", top_name, top_ind);
	    top_level_widget = top_lev;
            top_level_wg_name = XtName(top_level_widget);
            viewer_widget = 0;
	} else {
	    msg("TOP WIDGET NOT FOUND: [$1] $2", catastrophe_sev) << top_ind << eoarg << top_name << eom;
        }   
    }
}

viewer* cur_viewer = 0;
 
extern "C" void cmd_moveto_ev(XEvent * e)
{
  Window  win = e->xbutton.window;
  em_moveto_window(win, e->xmotion.x, e->xmotion.y);
  OS_dependent::sleep(1);
}

extern "C" viewer* ui_select_viewer(int vi)
{
   if(cmd_current_journal_name)  // -rr
      cmd_gen_write_force("ui_select_viewer(%d);", vi);
   cmd_do_flush();
   viewerShell * cvs = viewerShell::get_current_vs();
   viewer *curv = cvs->get_viewer(vi - 1);
   if(curv){
       cur_viewer = curv;
       viewer_widget = cvs->get_viewer(vi-1)->get_widget();
       viewPtr v= curv->get_view_ptr();
       steScreenPtr scr = curv->get_screen();
       epoch_report_current_view (v, scr);	   
   }   
   cmd_do_flush();
   return curv;
}

extern "C" viewer* cmd_get_cur_viewer()
{
  return cur_viewer;
}

view* cmd_get_cur_view()
{
    viewer * vr = cmd_get_cur_viewer();
    if (vr)
        return vr->get_view_ptr();
    else
        return (NULL);
}
 
// returns: 0 - OK
//          1 - wrong view title
//          2 - wrong view represenation type
//          -1  Unknown
static int ui_validate (viewer *vwr, char *old_ttl, int old_tp, 
			genString& ttl, int& tp)
{
    Initialize(ui_validate);
    int res = -1;
    ttl = NULL;
    tp = -1;

    if (old_ttl && old_ttl[0] && vwr) {
	viewPtr v = vwr->get_view_ptr();
	if (v == NULL) {
	    if (strcmp(old_ttl, cmdNoView) == 0)   // No View case
		res = 0;
	    else
		res = 1;
	} else {
	    ttl = v->get_name();
	    tp  = v->get_type();
	    if (strcmp((char *)ttl, old_ttl) != 0)
		res = 1;
	    else if (tp != old_tp)
		res = 2;
	    else
		res = 0;
	}
    }
    return res;
}

extern "C" void ui_viewer_validate (int vi, char *old_ttl, int old_tp)
{
    Initialize(ui_viewer_validate);
    if (! cmd_validate_p())
	return;
    
    if (cmd_begin()) {
	viewerShell * cvs = viewerShell::get_current_vs();
	viewer *curv = cvs->get_viewer(vi - 1);
	int res = -1;
	genString the_ttl;
	int the_tp = -1;
//
// ui_validate provides	view representation test, but does incorrect view title
// comparison (does not discard directory name, as it should). That is why 
// additional comparison is performed (via cmd_validate_string_value).
//	
	if (curv) 
	{
	    res = ui_validate (curv, old_ttl, old_tp, the_ttl, the_tp);
	    if (res == 1 && (char *)the_ttl && old_ttl)
		res = cmd_validate_string_value((char *)the_ttl, old_ttl);
	}
	genString txt;
	if (res == 1) {
	    txt.printf("Wrong View Title. Expected: %s; Real: %s", (char *)old_ttl, (char *)the_ttl);
	    cmd_validation_error ((char *)txt);
	} else if (res == 2) {
	    const char *old_nm = repType_get_name (old_tp);
	    const char *new_nm = repType_get_name (the_tp);
	    const char *o_nm = (old_nm) ? old_nm : "(NotFound)";
	    const char *n_nm = (new_nm) ? new_nm : "(NotFound)";
	    txt.printf("Wrong View Type. Expected: %d = %s; Real: %d = %s", 
		       old_tp, o_nm, the_tp, n_nm);	cmd_validation_warning ((char *)txt); 
	}
    }cmd_end();
}

void cmd_record_viewer_validate (viewer *vwr, int ind)
{
    Initialize(cmd_record_viewer_validate);
    if (vwr) {
	viewPtr v = vwr->get_view_ptr ();

	int old_tp = (v) ? v->get_type() : Rep_UNKNOWN;
	const char *old_ttl = (v) ? (const char *)v->get_name() : cmdNoView;
	
	cmd_gen_write_force("ui_viewer_validate(%d,%s,%d);", 
			    ind + 1, old_ttl, old_tp);
    }
}

extern "C" viewer* cmd_record_viewer(Widget wg)
{
    viewerShell * cvs = viewerShell::get_current_vs(1);

    if (! cvs) return 0;

    Widget vw = 0;
    int current_viewer = 0;

    if (wg)
    {
	Widget path[60];
	int wind = 0;
	Widget par = wg;
	while(wind < 60 && par){
	    path[wind] = par;
	    par = XtParent(par);
	    ++wind;
	}
	wind -= 2;
	
	int ii;
	
	for(ii=wind - 1; ii>=0; --ii){
	    par = path[ii];
	    String pn = XtName(par);
	    if(strcmp(pn, "viewer") == 0){
		vw = par;
	    }
	}

	if(! vw) return 0;

	for(ii=0; ii<cvs->get_num_viewers(); ++ii){
	    if(cvs->get_viewer(ii)->get_widget() == vw){
		current_viewer = ii;
		break;
	    }
	}
    } else {
	current_viewer = cvs->get_current_viewer();
	vw = cvs->get_viewer(current_viewer)->get_widget();
    }

//    if ((!viewer_widget) || (viewer_widget != vw)) {
	 cmd_gen("ui_select_viewer(%d);", current_viewer + 1);
         viewer_widget = vw;
//    }
    cur_viewer = cvs->get_viewer(current_viewer);
    return cur_viewer;
}

Widget widget_of_view (view *vvv)
{
    viewerShell * cvs = viewerShell::viewerShell_of_view(vvv);
    int no_viewers = (cvs) ? cvs->get_num_viewers() : 0;
    Widget vw = 0;
    for(int ii=0; ii<no_viewers; ++ii)
      {
        viewer* vr = cvs->get_viewer(ii);
        if(vr->get_view_ptr() == vvv)
          {
            vw = vr->get_widget();
            break;
          }
      }
    return vw;
}

extern "C" viewer * gra_focus_viewer()
{
    viewerShell * cvs = viewerShell::get_current_vs();
    cur_viewer = cmd_get_cur_viewer();
    viewerShell::focused_viewer(cur_viewer);
    return cur_viewer;
}
 
int cmd_gra_set_cur_view(Widget wg)
{
    Initialize(cmd_gra_set_cur_view);
 
    cmd_record_top_level(wg);
 
    viewer * curv = cmd_record_viewer(wg);
 
    if (curv != cur_viewer)
    {
        cmd_gen("gra_focus_viewer();");
        cur_viewer = curv;
        return 1;
    }
    return 0;
}
 
void cmd_record_view(view* vvv)
{
    Widget vw = widget_of_view (vvv);
 
    if(vw)
        cmd_gra_set_cur_view(vw);
}
 
void cmd_record_viewer_validate (viewer *vwr)
{
    Initialize(cmd_record_viewer_validate);
    if (! vwr || !cmd_validate_p())
	return;

    viewerShell * cvs = viewerShell::get_current_vs(1);
    if (! cvs)
	return;

    int cur_vwr = -1;
    for(int ii=0; ii<cvs->get_num_viewers(); ++ii) {
	if (cvs->get_viewer(ii) == vwr) {
	    cur_vwr = ii;
	    break;
    }    }
 
    if(cur_vwr > -1) {
	cur_viewer = vwr;
	cmd_record_viewer_validate (vwr, cur_vwr);
    }
}

static void  cmd_text_exit(Widget wg, int cr) {
    last_text = 0;
    if (wg) {
	char * txt = XmIsText(wg) ?
	    XmTextGetString(wg) : XmTextFieldGetString(wg);

	genString name;
	cmd_choose_name(wg, name);
    
	if (txt && name.length())
	    cmd_gen("ui_replace_text(%s, %s, %d);", (char *)name, txt, cr);
    }
}

extern "C" void cmd_prt_widget_path(Widget wg, int lev = 0)
{
    if((! wg) || lev > 4) {
	return;
    }
    char * nm = XtName(wg);
    char * text = 0;
    if (XmIsLabel(wg)){
          XmString label;
          char * text = 0;
          XtVaGetValues(wg, XmNlabelString, &label, 0);
          if(label)
              XmStringGetLtoR(label, XmSTRING_DEFAULT_CHARSET, &text);
    }

    msg("($1 $2 $3)", normal_sev) << wg << eoarg << nm << eoarg << text << eom;
    wg = XtParent(wg);
    cmd_prt_widget_path(wg, lev+1);
}

void cmd_button_event_pre(Widget wg, genString& pname, XButtonEvent& be)
{
  Initialize(cmd_button_event_pre);
  
  if(last_text && (last_text != wg)){
      cmd_text_exit(last_text, 0);
  }
  
  char *text;
  
  Widget old_last = last_widget;
  last_widget = wg;

  if(XmIsList(wg)){
      if(be.type == ButtonPress)
      {
	  static unsigned long last_select_time = 0;
	  unsigned long select_delta = be.time - last_select_time;
	  last_select_time = be.time;
	  
	  int dci;
	  XtVaGetValues(wg, XmNdoubleClickInterval, &dci, 0);
	  if (select_delta <= dci) {
	      DBG msg("----DOUBLE CLICK", normal_sev) << eom;
	      cmd_gen("ui_list_action(%s);", (char*)pname);
	      skip_button_post = 1;    // skip next event_post;
	  }
      }
  } else if (XmIsArrowButton(wg)) {
      if (be.type == ButtonPress) {
	  tracked = 0;
	  tracking_on = 1;
	  track_tossfirst = 1;
	  last_label = 0;
	  track_button = 1;
	  track_first = 1;
      }
      else {
      text = XtName(wg);
      tracking_on = 0;
      cmd_gen("ui_push_button(%s, %s);",(char *)pname, text);
  }
  } else if (XmIsText(wg)){
      text = XmTextFieldGetString(wg);
      last_text = wg;
  } else if (XmIsTextField(wg)){
      text = XmTextGetString(wg);
      last_text = wg;
  } else if(XmIsLabel(wg)){
      XmString label;
      XtVaGetValues(wg, XmNlabelString, &label, 0);
      if(label){
	  XmStringGetLtoR(label, XmSTRING_DEFAULT_CHARSET, &text);
	  DBG msg("--- LABEL: $1", normal_sev) << text << eom;
	  
	  if(XtParent(wg) == composite_widget){

	      int ev_type = 0;
	      switch(be.type) {
		case ButtonPress:
		  ev_type = 1;
		  break;
		case ButtonRelease:
		  ev_type = -1;
		  break;
		default:
		  ev_type = 0;
		  break;
	      }

	      if(wg == old_last) return;
	      Widget fl = cmd_find_label(composite_widget, text);
	      
	      genString pname;
	      cmd_choose_name(XtParent(wg), pname);
              char *p = strrchr(text,'/');
              if (!p)
                  p = text;
              else
                  p++;

              cmd_gen("ui_select_label(%s, %s, %d);", (char*)pname, p, ev_type);

	      wg = composite_widget;
	      
	  } else {
	      if(be.button == Button1) {
		  if (be.type == ButtonPress) {
		      tracking_on = 1;
		      track_button = 1;
		      tracked = 0;
		      track_first = 1;
		      track_tossfirst = 1;
		      last_label = 0;  // for tracking
		  } else {
		      if (tracked)
			  cmd_gen_write_force("ui_button_event(%s, %d, %d);", (char*)pname, 1, -1);
		      else
			  cmd_gen_write_force("ui_push_button(%s, %s);", (char*)pname, text);
		      tracking_on = 0;
		  }			
	      } else {
		  int button_number = 1;
                    if(be.button == Button2)
                        button_number = 2;
                    else if(be.button == Button3)
                        button_number = 3;
		  
		  int ev_type;
		  if(be.type == ButtonPress)
		      ev_type = 1;
		  else
		      ev_type = -1;
		  
//                    cout << " BUTTON_EVENT "<< endl;
//                    cmd_prt_widget_path(wg, -2);
		  tracking_on = 0;
		  cmd_gen_write_force("ui_button_event(%s, %d, %d);", 
                              (char*)pname, button_number, ev_type);
	      }
	  }
      }
  } else if(XmIsRowColumn(wg)) {
      if(wg == old_last)
	  return;
      
      XmString label = 0;
      Widget subid; 
      unsigned char tp;

      XtVaGetValues(wg, XmNrowColumnType, 
                    &tp, XmNsubMenuId, &subid, 0);
      text = 0;            
      
      Widget ga;
      if(tp == XmMENU_OPTION){
	  if (!label || (!text && !(ga = cmd_find_gadget (be, wg))) ) {
	      composite_widget = subid;
//              cmd_gen("ui_push_button(%s);", (char*)pname);
	      cmd_gen("ui_button_event(%s,%d,%d);", (char*)pname, 1, 1);
	  }
	  else {
	      DBG msg("--- COMPOSITE: looking for gadget ", normal_sev) << eom;
	      text = XtName(ga);
	      cmd_gen("ui_select_gadget(%s, %s);", (char*)pname, text);
	      DBG msg("$1 $2", normal_sev) << ga << eoarg << text << eom;
	  }	
      }
  } else if (XtIsComposite(wg)) {  // composite: find which gadget
          if(wg == old_last) return;
//          if(be.type == ButtonRelease) return;
	  
          Widget ga = cmd_find_gadget(be, wg);
          DBG msg("--- COMPOSITE: looking for gadget ", normal_sev) << eom;
          if(ga){
	      text = XtName(ga);
	      cmd_gen("ui_select_gadget(%s, %s);", (char*)pname, text);
	      DBG msg("$1 $2", normal_sev) << ga << eoarg << text << eom;
          }
          
      }
  DBG{
      char * nm = XtName(wg);
      msg("----MOUSE: wg $1 $2 $3", normal_sev) << wg << eoarg << be.type << eoarg << nm << eom;
      msg(" path: $1\n") << (char *) pname << eom;
  }
  
}

void cmd_button_event_post(Widget wg, genString& pname, XButtonEvent& be)
{
	int no_sel, no_item;
	int *sel = 0;
	char *text;
	XmString *str_list;

	if(XmIsList(wg)){
          if(be.type == ButtonPress)
	      return;
          Boolean is_list = XmListGetSelectedPos(wg, &sel, &no_sel);
          if(!skip_button_post &&is_list && sel){
             XtVaGetValues(wg, XmNitemCount, &no_item, XmNitems, &str_list, 0);

             for(int ii=0; ii<no_sel; ++ii){
               int jj = sel[ii];
               XmStringGetLtoR(str_list[jj-1], XmSTRING_DEFAULT_CHARSET, &text);
       
	       char* ptr = pname;
	       while (*ptr && isicon(*ptr))
		   ptr++;
	       while (*text && isicon (*text))
		   text++;
	       
               if(ii==0){
                  cmd_gen("ui_select_list(%s, %s);", (char*)ptr, text);
               } else {
                  cmd_gen("ui_addselect(%s, %s);", (char*)ptr, text);
               }
             }
//             cout << endl;
	  }
	  skip_button_post = 0;
        }
}

void cmd_key_event_post(Widget wg, genString& /*pname*/, XKeyEvent& ke)
{
    if( !wg || !(&ke) )
	return;
}

void cmd_key_event_pre(Widget wg, genString& /*pname*/, XKeyEvent& ke)
{
    if(!wg)
	return;

    if(ke.keycode == XK_Return) {
	if (XmIsText(wg)) {
	    int edit_mode;
	    XtVaGetValues(wg, XmNeditMode, &edit_mode, 0);
	    if (edit_mode == XmMULTI_LINE_EDIT)
		cmd_text_exit(wg, 0);
            else
		cmd_text_exit(wg, 1);
	} else 
	    cmd_text_exit(wg, 1);
    } else if (last_text != wg) {
	if(last_text)
	    cmd_text_exit(last_text, 0);

	if (XmIsText(wg))
	    last_text = wg;
    }
}

static Widget current_wg;
static genString current_pname;

void cmd_track_widget(XEvent& xe)
{
   Initialize(cmd_track_widget);
  
	    XButtonEvent& be = xe.xbutton;
	    Display * ds = be.display;
	    Window wd = be.window;

        current_wg = XtWindowToWidget(ds, wd);
        if(! current_wg) return;
 
        genString pname;
        if(xe.type == LeaveNotify) {
	    if (XmIsLabel(current_wg)) {
		last_label = current_wg;
		if (track_first && !track_tossfirst) {
		    cmd_choose_name(current_wg, pname);
		    cmd_gen("ui_button_event(%s, %d, 1);", (char*)pname, track_button);
		    last_label = 0;
		    tracked = 1;
		}
		track_first = 0;
		track_tossfirst = 0;
	   } else {
		if (last_label) {
		    cmd_choose_name(last_label, pname);
		    cmd_gen("ui_button_event(%s, %d, 1);", (char*)pname, track_button);
		    tracked = 1;
		}
		last_label = 0;
		track_first = 1;
	    }
	}

        DBG msg(" cmd_track_widget type $1 $2 $3  $4", normal_sev) << xe.type << eoarg << current_wg << eoarg << XtName(current_wg) << eoarg << XmIsLabel(current_wg) << eom;

}

void cmd_button_event_for_help(Widget wg, genString&, XButtonEvent& be)
{
    Initialize(cmd_button_event_for_help);
    
    if(XmIsLabel(wg)){
	XmString label;
//	char     *text;
     
	XtVaGetValues(wg, XmNlabelString, &label, 0);
	if(label){
//	    XmStringGetLtoR(label, XmSTRING_DEFAULT_CHARSET, &text);
	    if(be.button == Button1) {
		if(be.type == ButtonPress) {
		    help_tracking_on = 1;
		}
	    }
	}
    }
}

void cmd_track_widget_for_help(XEvent& xe)   
{
    Initialize(cmd_track_widget_for_help);
  
    XButtonEvent& be = xe.xbutton;
    Display * ds     = be.display;
    Window wd        = be.window;
    
    current_wg = XtWindowToWidget(ds, wd);
    if(! current_wg) return;
 
    if(xe.type == EnterNotify || (be.type == ButtonPress && be.button == Button1)) {
	if (XmIsLabel(current_wg) || 1) {
	    Widget  parent = XtParent(current_wg);
	    Boolean is_sensitive;
	    if(XmIsPushButton(current_wg)){
		XtVaGetValues(current_wg, XmNsensitive, &is_sensitive, 0);
		if(is_sensitive && parent)
		    XtVaGetValues(parent, XmNsensitive, &is_sensitive, 0);
//		if(!is_sensitive)
//		    cout << "Button unsensitive" << endl;
	    }
	    genString pname;
	    genString parent_name;
	    String name = XtName(current_wg);
	    pname       = name;
	    parent_name = "";
	    while(parent){
		name          = XtName(parent);
		genString tmp = name;
		tmp          += '.';
		tmp          += parent_name;
		parent_name   = tmp;
		parent        = XtParent(parent);
	    }
	    genString full_name;
            last_help_was_browser = 1;
	    if(strstr(parent_name.str(), ".browser."))
		full_name  = "Browser.";
	    else
		if(strstr(parent_name.str(), ".ViewerShell.")) {
		  last_help_was_browser = 0;
		  full_name = "ViewerShell.";
		} else full_name = "Browser.";
	    Widget parent_wg = XtParent(current_wg);
	    if(parent_wg != NULL) {
		full_name += XtName(parent_wg);
		full_name += '.';
	    }
	    full_name += pname;
	    load_assist_database();
	    char *assist = search_assist_database((char *)full_name);
	    if(HelpUI::get_show_pathnames())
		browserShell::display_assist_line((char *)gettext(full_name), last_help_was_browser);
	    else
		if(assist)
		    browserShell::display_assist_line( (char*)gettext(assist), last_help_was_browser);
	}
    }
}

void cmd_handle_event_pre(XEvent& xe, int is_modal)
{
    if(help_tracking_on){
	if(xe.type==EnterNotify || xe.type == LeaveNotify)	
		cmd_track_widget_for_help(xe);	
	else
	    if(xe.type == ButtonRelease){
		help_tracking_on = 0;
		browserShell::display_assist_line( " ", last_help_was_browser);
	    }
    }
    if(tracking_on && (xe.type==EnterNotify || xe.type == LeaveNotify)){
	if(! is_modal)
	    cmd_begin();
	cmd_track_widget(xe);
	if(! is_modal)
	    cmd_end();
	return;
    }

    if (xe.type != ButtonPress && 
             xe.type != ButtonRelease && xe.type != KeyPress){
	return;
    }

    if (!cmd_current_journal_name){
	if(xe.type == ButtonPress || xe.type == ButtonRelease){
	    XButtonEvent& be = xe.xbutton;
	    Display *ds      = be.display;
	    Window   wd      = be.window;
	    current_wg       = XtWindowToWidget(ds, wd);
	    if(!current_wg) return;
	    cmd_button_event_for_help(current_wg, current_pname, be);
	    cmd_track_widget_for_help(xe);
	}
	return;
    }
    
    if((!is_modal) && (xe.type == ButtonRelease)){
	viewerShell * vs = viewerShell::get_current_vs(1);
	view* vv = vs ? vs->get_current_view() : 0;
	if(vv && is_steView(vv) && top_level_widget && 
	   top_level_wg_name && !strcmp("ViewerShell", top_level_wg_name))
                ste_finalize(vv);
    }
    
    if (!is_modal)
	cmd_begin();
    current_wg = 0;
    
    Display* ds = 0;
    Window  wd = 0;
    if(xe.type == ButtonPress || xe.type == ButtonRelease){
	    XButtonEvent& be = xe.xbutton;
	    ds = be.display;
	    wd = be.window;
	} else if(xe.type == KeyPress){
	    XKeyEvent& ke = xe.xkey;
            ds = ke.display;
            wd = ke.window;
	}
    current_wg = XtWindowToWidget(ds, wd);
    if(!current_wg) return;

    cmd_record_top_level(current_wg);
    
    cmd_record_viewer(current_wg);

    cmd_choose_name(current_wg, current_pname);
 
    if (XmIsList(current_wg))
	is_a_list = 1;

    if ((XmIsText(current_wg)) || (XmIsTextField(current_wg)))
        is_text = 1;
    
    if(xe.type == ButtonPress || xe.type == ButtonRelease){
	XButtonEvent& be = xe.xbutton;
	cmd_button_event_pre(current_wg, current_pname, be);
    } else if (xe.type == KeyPress && is_text) {
	XKeyEvent& ke = xe.xkey;
	cmd_key_event_pre(current_wg, current_pname, ke);
    }
}

void cmd_handle_event_post(XEvent & xe, int is_modal)
{
    if (xe.type != ButtonPress && xe.type != ButtonRelease && xe.type != KeyPress)
	return;

    if (current_wg)
    {
	if(xe.type == ButtonPress || xe.type == ButtonRelease){
	    XButtonEvent& be = xe.xbutton;
	    if (is_a_list)
		cmd_button_event_post(current_wg, current_pname, be);
	} else if(xe.type == KeyPress){
	    XKeyEvent& ke = xe.xkey;
            if (is_text)
		cmd_key_event_post(current_wg, current_pname, ke);
	}
    }

    if (xe.type == ButtonRelease){
       tracking_on      = 0;
       if(help_tracking_on){
	   help_tracking_on = 0;
	   browserShell::display_assist_line( " ", last_help_was_browser);
       }
    }

    if(! is_modal)
       cmd_end();

    is_a_list = 0;
    is_text = 0;
}

int cmd_modal_playback(int (*cond)(void* cd), void* cd)
{
    int still_playingback = 0;

    while (!(*cond)(cd) && (still_playingback = cmd_exec_next_stmt())) ;

    return (still_playingback);
}

void cmd_init_hooks()
{
    gtBase::register_modal_patchpoints(cmd_modal_playback,
				       cmd_handle_event_pre,
				       cmd_handle_event_post);
}

extern int cmd_rerecording;
void cmd_prt_pause()
{
  msg("rerecord this command and resume", normal_sev) << eom;
  cmd_exec_pause();
}

void cmd_validation_error (const char *ss)
{
    Initialize(cmd_validation_error);
    if (ss && ss[0])
	msg("Validation error: $1", catastrophe_sev) << ss << eom;
    
    if(cmd_rerecording)
      cmd_prt_pause();
    else
      cmd_abort ();
}

void cmd_validation_warning (const char *ss)
{
    Initialize(cmd_validation_warning);
    if (ss && ss[0])
	msg("Validation warning: $1", warning_sev) << ss << eom;
}

void cmd_validation_message (const char *ss)
{
    Initialize(cmd_validation_message);
    if (ss && ss[0])
	msg("Validation message: $1", normal_sev) << ss << eom;
}

static int last_match_count = -1;

void cmd_report_last_count (int cnt) 
{
    Initialize(cmd_report_last_count);
    last_match_count = cnt;
    if (cmd_current_journal_name)
	cmd_gen_write_force("ui_validate_match_count(%d);", cnt);    
}

extern "C" void ui_validate_match_count (int cnt)
{
    Initialize(ui_validate_match_count);
    if (cmd_validate_p())
	if (cnt != last_match_count) {
	    genString txt;
	    txt.printf ("Wrong match count. Expected: %d; Real: %d", cnt, last_match_count);
	    cmd_validation_error ((char *)txt);
	}
}

static int filter_OK_CB_flag = 0;

void cmd_filter_OK_CB_report () { filter_OK_CB_flag = 1; }
    
extern void cmd_filter_record_count (int cnt)
{
    Initialize(cmd_filter_record_count);
    if (filter_OK_CB_flag) {
	filter_OK_CB_flag = 0;
	cmd_report_last_count (cnt);
    }
}
