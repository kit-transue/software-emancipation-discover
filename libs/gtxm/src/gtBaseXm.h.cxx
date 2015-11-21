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
// gtBaseXm
//------------------------------------------
// synopsis:
// gtBase and gtRep implementations in OSF Motif
//
//------------------------------------------

// INCLUDE FILES

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <psetmem.h> 

#include <Notifier.h>
#include <top_widgets.h>

#include <gtPushButton.h>
#include <gtBaseXm.h>
#include <gtAppShellXm.h>

#include <machdep.h>

// EXTERNAL VARIABLES

extern Display* UxDisplay;

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */
#include <genArr.h>

#include <genError.h>


// VARIABLE DEFINITIONS

static void (*help_button_CB)(gtBase*);	// Callback for Help gtPushButtons

double gtBase::ppmm_height;
double gtBase::ppmm_width;

static char pathname_buffer[1024];

typedef void (*CBfunc)(void*);
static CBfunc* flush_func_array;
static void** flush_data_array;
static int flush_count;

struct CB {
 CBfunc fun;
 void* data;
};

extern "C" void ping_paraset ();

#define MAX_ONESHOTS 20 // max oneshot array size without realloc
                        // gtBase::oneshot() is called from signal handler
                        // and must not allocate memory, because memory 
                        // allocation is not reenterable (causes mre Purify error)
genArr(CB);
static CBArr arr1(MAX_ONESHOTS);
static CBArr arr2(MAX_ONESHOTS);
static int CB_ind = 0;
static CBArr *bbb[2] = { &arr1, &arr2};

void gt_oneshot(void (*fun)(void*), void*data, int ping)
/*
 * inserts last hook into current oneshot array;
 * issues a warning if array\'s size is more or equal to MAX_ONESHOTS,
 * because, each grow() call causes memory reallocation and potentially,
 * memory overwriting and crash.
 *
 */
{
    if (bbb[CB_ind]->size() >= MAX_ONESHOTS) {
	msg("gtBase::oneshot(): oneshot array has overflowed\n") << eom;
    }
	
    CB * cb = bbb[CB_ind]->grow(1);
    
    cb->fun = fun;
    cb->data = data;
    if(ping)
      ping_paraset ();
}

void gtBase::oneshot(void (*fun)(void*), void*data)
{
  gt_oneshot(fun, data, 1);
}

void gtBase::flush_oneshot()
{
  CBArr * cb = bbb[CB_ind];
  CB_ind = 1 - CB_ind;
  int sz = cb->size();
  for(int ii=0; ii<sz; ++ii)
     (*cb)[ii]->fun((*cb)[ii]->data);
  cb->reset();
}
// FUNCTION DEFINITIONS

static void help_context_cleanup(Widget w, XtPointer, XtPointer)
{
    set_widget_help_context(w, NULL);
}

extern "C" void set_widget_help_context(Widget w, const char *name)
{
    if (!name)
    {
	if (notifier.get_prop(PROP_GTHELP, w))
	{
	    notifier.set_prop(PROP_GTHELP, w, NULL);
	    notifier.remove(w);
	}
    }
    else
    {
	if (!notifier.get_prop(PROP_GTHELP, w))
	    notifier.add(w);
	notifier.set_prop(PROP_GTHELP, w, (void *)name);
	if(XtIsWidget(w))
	    XtAddCallback(w, XmNdestroyCallback, help_context_cleanup, NULL);
    }
}

extern const char *get_widget_help_context(Widget w)
{
    return (const char*)notifier.get_prop(PROP_GTHELP, w);
}

void gtRep::ref(gtRep* rep)
{
    if(rep)
	++rep->ref_count;
}

void gtRep::unref(gtRep*& rep)
{
    if(rep)
    {
	if(--rep->ref_count == 0)
	{
	    if(rep->widget())
		XtDestroyWidget(rep->widget());
	    delete rep;
	}
	rep = NULL;
    }
}


typedef struct
{
    WidgetClass     superclass;		/* pointer to superclass ClassRec   */
    String          class_name;		/* widget resource class name       */
}
*StartOfWidgetClassPtr;


static void print_widget_path(Widget w)
{
    Widget p = XtParent(w);
    if(p)
    {
	print_widget_path(p);
	putc(' ', stderr);
	putc('-', stderr);
	putc('>', stderr);
	putc(' ', stderr);
    }
    fputs(StartOfWidgetClassPtr(XtClass(w))->class_name, stderr);
    putc(' ', stderr);
    putc('"', stderr);
    fputs(XtName(w), stderr);
    putc('"', stderr);
}


void gtRep::print_path()
{
    print_widget_path(widg);
    putc('\n', stderr);
}

static void build_widget_pathname(Widget w, char*& pathname, int& size)
{
    Widget p = XtParent(w);
    if(p)
    {
	build_widget_pathname(p, pathname, size);
	if(size > 1)
	{
	    OS_dependent::bcopy(".", pathname, 2);
	    pathname += 1;
	    size -= 1;
	}
	else
	    return;
    }
    const char* name = XtName(w);
    const int length = strlen(name);
    if(length < size)
    {
	// Copy the name and terminating NULL.
	OS_dependent::bcopy(name, pathname, length + 1);
	pathname += length;
	size -= length;
    }
    else
    {
	// Out of room: stop any further additions to the pathname.
	size = 0;
    }
}


extern char* widget_pathname(Widget w)
{
    int size = sizeof(pathname_buffer);
    char* bufptr = pathname_buffer;
    *bufptr = 0;

    build_widget_pathname(w, bufptr, size);

    return pathname_buffer;
}


char* gtRep::pathname()
{
    return widget_pathname(widg);
}


void gtRep::help_context_name(const char *name)
{
    set_widget_help_context(widg, name);
}


const char* gtRep::help_context_name()
{
    return get_widget_help_context(widg);
}


void build_help_pathname(Widget w, char*& pathname, int& size,
				       int implicit_name)
{
    const char* name = get_widget_help_context(w);
    if (!name)
    {
	Widget p = XtParent(w);
	if(p)
	{
	    build_help_pathname(p, pathname, size, implicit_name);
	    if(size > 1 && implicit_name)
	    {
		OS_dependent::bcopy(".", pathname, 2);
		pathname += 1;
		size -= 1;
	    }
	    else
		return;
	}
	if (implicit_name)
	    name = XtName(w);
	else
	    return;
    }

    const int length = strlen(name);
    if(length < size)
    {
	// Copy the name and terminating NULL.
	OS_dependent::bcopy(name, pathname, length + 1);
	pathname += length;
	size -= length;
    }
    else
    {
	// Out of room: stop any further additions to the pathname.
	size = 0;
    }
}

extern char* widget_help_pathname(Widget w, int implicit_name)
{
    int size = sizeof(pathname_buffer);
    char* bufptr = pathname_buffer;
    *bufptr = 0;

    build_help_pathname(w, bufptr, size, implicit_name);

    return pathname_buffer;
}

char* gtRep::help_pathname(int implicit_name)
{
    return widget_help_pathname(widg, implicit_name);
}

//_________________________________________________________________________
// Help functions

void gtBase::init_help_button(void (*cb)(gtBase*))
{
    help_button_CB = cb;
}

void gtBase::help_button_callback(gtPushButton* gt, gtEventPtr, void*, gtReason)
{
    if(help_button_CB)
	(*help_button_CB)(gt);
}

void gtBase::help_context_name(const char *name)
{
    r->help_context_name(name);
}

const char* gtBase::help_context_name()
{
    return r->help_context_name();
}

char *gtBase::help_pathname(int implicit_name)
{
    return r->help_pathname(implicit_name);
}

//_________________________________________________________________________
// Constructor

gtBase::gtBase()
: user_destroy_data(NULL), user_destroy_func(NULL), r(new gtRep)
{}

//_________________________________________________________________________
// Destructor and associated functions.

void gtBase::destroy_callback(void*, gtBase* base, void*)
{
    if(base->rep())
	base->rep()->widget(NULL);

    delete base;
}

void gtBase::destroy_init()
{
    attach_rep(r);
    calc_ppmm();
}

void gtBase::attach_rep(gtRep* rr)
{
    r = rr;
    gtRep::ref(r);
    if(r->widget())
    {
	XtAddCallback(r->widget(), XmNdestroyCallback,
		      XtCallbackProc(destroy_callback), (XtPointer)this);
    }
}

void gtBase::release_rep()
{
    if(r)
    {
	if(r->widget())
	{
	    XtRemoveCallback(r->widget(), XmNdestroyCallback,
			     XtCallbackProc(destroy_callback), (XtPointer)this);
	}
	gtRep::unref(r);
    }
}

gtBase::~gtBase()
{
    if(user_destroy_func)
	(*user_destroy_func)(this, user_destroy_data);

    release_rep();
}

//_________________________________________________________________________

const char* gtBase::name()
{
    return (r && r->widget()) ? XtName(r->widget()) : NULL;
}

void gtBase::pos(int x, int y)
{
    Arg	args[2];
    int n = 0;

    if (x >= 0) { XtSetArg(args[n], XmNx, x); n++; }
    if (y >= 0) { XtSetArg(args[n], XmNy, y); n++; }
    XtSetValues(r->widget(), args, n);
}

static Arg xt_get_args[1];

inline Dimension xt_get_Dimension(gtRep* r, char* name)
{
    Dimension ans;
    XtSetArg(xt_get_args[0], name, &ans);
    XtGetValues(r->widget(), xt_get_args, 1);
    return ans;
}

inline Position xt_get_Position(gtRep* r, char* name)
{
    Position ans;
    XtSetArg(xt_get_args[0], name, &ans);
    XtGetValues(r->widget(), xt_get_args, 1);
    return ans;
}

int gtBase::px()
{
    return xt_get_Position(r, XmNx);
}

int gtBase::py()
{
    return xt_get_Position(r, XmNy);
}

int gtBase::pwidth()
{
    return xt_get_Dimension(r, XmNwidth);
}

int gtBase::pheight()
{
    return xt_get_Dimension(r, XmNheight);
}

void gtBase::border(int b)
{
    Arg	args[1];
    XtSetArg(args[0], XmNborderWidth, b);
    XtSetValues(r->widget(), args, 1);
}

void gtBase::margins(int horz, int vert)
{
    Arg	args[2];
    XtSetArg(args[0], XmNmarginWidth, horz);
    XtSetArg(args[1], XmNmarginHeight, vert);
    XtSetValues(r->widget(), args, 2);
}

int gtBase::hund_to_pix_horiz(int hund)
{
    return (int)((hund * .254 * ppmm_width) + 0.5);
}

int gtBase::hund_to_pix_vert(int hund)
{
    return (int)((hund * .254 * ppmm_height) + 0.5);
}

int gtBase::pix_to_hund_horiz(int pix)
{
    return (int)((pix / (2.54 * ppmm_width)) + 0.5);
}

int gtBase::pix_to_hund_vert(int pix)
{
    return (int)((pix / (2.54 * ppmm_height)) + 0.5);
}

void gtBase::size(int h, int w)
{
    Arg	args[2];
    int n = 0;

    if (h >= 0)
    {
	h = hund_to_pix_vert(h);
	if(h <= 0) h = 1;
	XtSetArg(args[n], XmNheight, h); n++;
    }
    if (w >= 0)
    {
	w = hund_to_pix_horiz(w);
	if(w <= 0) w = 1;
	XtSetArg(args[n], XmNwidth, w);  n++;
    }
    if(n > 0)
	XtSetValues(r->widget(), args, n);
}

void gtBase::width(int w)
{
    w = hund_to_pix_horiz(w);
    if(w <= 0) w = 1;

    Arg	args[1];
    XtSetArg(args[0], XmNwidth, w);
    XtSetValues(r->widget(), args, 1);
}

void gtBase::height(int h)
{
    h = hund_to_pix_vert(h);
    if(h <= 0) h = 1;

    Arg	args[1];
    XtSetArg(args[0], XmNheight, h);
    XtSetValues(r->widget(), args, 1);
}

//_________________________________________________________________________


void gtBase::map()
{      
    XtMapWidget(r->widget());
}


void gtBase::unmap()
{
    XtUnmapWidget(r->widget());
}


static int map_state(Widget w)
{
    if(XtIsRealized(w))
    {
	XWindowAttributes xwa;
	XGetWindowAttributes(XtDisplay(w), XtWindow(w), &xwa);

	return xwa.map_state;
    }
    return IsUnmapped;
}


int gtBase::is_mapped()
{
    return map_state(r->widget()) != IsUnmapped;
}


int gtBase::is_visible()
{
    return map_state(r->widget()) == IsViewable;
}


void gtBase::calc_ppmm()
{
    if(ppmm_height == 0.0  &&  r  &&  r->widget())
    {
	Screen* scr = XtScreen(r->widget());
	ppmm_height =
	    double(HeightOfScreen(scr)) /
	    double(HeightMMOfScreen(scr));
	ppmm_width  =
	    double(WidthOfScreen(scr)) /
	    double(WidthMMOfScreen(scr));
    }
}


void gtBase::flush_output_callback(void (*func)(void*), void* data)
//
// Add another function to the flush callback list.
//
{
    if(flush_count++ == 0)
    {
	flush_func_array = (CBfunc*)psetmalloc(sizeof(CBfunc));
	flush_data_array = (void**)psetmalloc(sizeof(void*));
    }
    else
    {
	flush_func_array =
	    (CBfunc*)psetrealloc((char *)flush_func_array, flush_count * sizeof(CBfunc));
	flush_data_array =
	    (void**)psetrealloc((char *)flush_data_array, flush_count * sizeof(void*));
    }
    flush_func_array[flush_count - 1] = func;
    flush_data_array[flush_count - 1] = data;
}


void gtBase::flush_output()
//
// Call all functions in the flush callback list.
//
{
    flush_oneshot();
    for(int i = flush_count - 1; i >= 0 ; --i)
    {
	CBfunc func = flush_func_array[i];
	void* data = flush_data_array[i];
	(*func)(data);
    }
}

int (*gtBase::modal_event_playback)(int (*stop)(void* cd), void* cd) = 0;
void (*gtBase::modal_event_record_pre)(XEvent &, int) = 0;
void (*gtBase::modal_event_record_post)(XEvent &, int) = 0;

void gtBase::register_modal_patchpoints(
	int (*event_playback)(int (*stop)(void* cd), void* cd),
	void (*event_record_pre)(XEvent&, int),
	void (*event_record_post)(XEvent&, int))
{
    modal_event_playback = event_playback;
    modal_event_record_pre = event_record_pre;
    modal_event_record_post = event_record_post;
}

extern void driver_client_hook (XEvent *);

int gtxm_process_events(int max_events)
{
  int ii = 0;
  XtAppContext cont = gtApplicationShellXm::get_context();

//---------------------------------------------------------------------------
// The following kludgery is needed to write around a bug in the HP C compiler
// optimizer which crashes during compilation if you take the address of a union
// like XEvent.  We thus define a buffer of the right size and just make xe a
// pointer to the buffer instead of declaring xe as an XEvent directly and
// doing &xe in all the contexts in which an XEvent* is required.
//---------------------------------------------------------------------------

  char event_buf[sizeof(XEvent)];
  XEvent* xe = (XEvent*) event_buf;

//  XtInputMask mask = XtAppPending(cont);
//  if(!mask) return ii;

  while(XtAppPending(cont)){
    XtAppNextEvent(cont, xe);

    if (xe->type == ClientMessage)
		driver_client_hook (xe);
    else
		XtDispatchEvent(xe);

    ++ii;
    if(ii >= max_events) break;
  } 

  if(ii>0)
    gtBase::flush_output();
  
  return ii;
}

int gt_process_events()
{
  return
    gtxm_process_events(10000);
}

struct gt_ev_pusher{
 int ev_period;
 int ev_max;
 int ev_count;
 void push();
 gt_ev_pusher(int period, int max_event)
 {
  ev_period = period;
  ev_max = max_event;
  ev_count = 0;
 }
};

static gt_ev_pusher * ev_pusher = 0;

void gt_setup_pusher(int i, int j)
{
 if(ev_pusher)
    delete ev_pusher;
 ev_pusher = new gt_ev_pusher(i, j);
}

void gt_delete_pusher()
{
 if(ev_pusher)
    delete ev_pusher;

 ev_pusher = 0;
}

void gt_push()
{ 
  if(ev_pusher)
     ev_pusher->push();
}

void gt_ev_pusher::push()
{
  Initialize(gt_ev_pusher::push);
  ++ ev_count;
  if(ev_count < ev_period) return;

  ev_count = 0;

  DBG {
     msg("Process $1 events. Event period = $2\n") << ev_max << eoarg << ev_period << eom;
  }

  gtxm_process_events(ev_max);
}

void gtBase::take_control(int (*cond)(void *), void *cd)
//
// Continuously handle events until (*cond)(cd) returns 0.
//
// If modal_event_record returns a non-zero value, it is expected that
// it will take control and loop until (*cond)(cd) returns 0.  Then,
// when it is finished, the rest of the event loop will be skipped.
//
{
    gtBase::flush_output();
    push_cursor(None);

    int is_playback = 0;
    if (modal_event_playback)
	is_playback = (*modal_event_playback)(cond, cd);
    
    if(!is_playback)
    {
	while(!(*cond)(cd))
        {

	    flush_output();

//---------------------------------------------------------------------------
// The following kludgery is needed to write around a bug in the HP C compiler
// optimizer which crashes during compilation if you take the address of a union
// like XEvent.  We thus define a buffer of the right size and just make xe a
// pointer to the buffer instead of declaring xe as an XEvent directly and
// doing &xe in all the contexts in which an XEvent* is required.
//---------------------------------------------------------------------------

	    char event_buffer[sizeof(XEvent)];
	    XEvent* xe = (XEvent*) event_buffer;
	    XtAppNextEvent(gtApplicationShellXm::get_context(), xe);

	    if (modal_event_record_pre)
		(*modal_event_record_pre)(*xe, 1);

	    if (xe->type == ClientMessage)
		driver_client_hook (xe);
	    else
		XtDispatchEvent(xe);

	    if (modal_event_record_post)
		(*modal_event_record_post)(*xe, 1);
	}
    }

    pop_cursor();
}

void gtBase::take_control_top(int (*cond)(void *), void *cd)
//
// Continuously handle events until (*cond)(cd) returns 0.
//
// If modal_event_record returns a non-zero value, it is expected that
// it will take control and loop until (*cond)(cd) returns 0.  Then,
// when it is finished, the rest of the event loop will be skipped.
//
{
    push_cursor(None);
 
    int is_playback = 0;
    if (modal_event_playback)
        is_playback = (*modal_event_playback)(cond, cd);
 
    if(!is_playback)
    {
        while(!(*cond)(cd))
        {
            flush_output();

//---------------------------------------------------------------------------
// The following kludgery is needed to write around a bug in the HP C compiler
// optimizer which crashes during compilation if you take the address of a union
// like XEvent.  We thus define a buffer of the right size and just make xe a
// pointer to the buffer instead of declaring xe as an XEvent directly and
// doing &xe in all the contexts in which an XEvent* is required.
//---------------------------------------------------------------------------

	    char event_buffer[sizeof(XEvent)];
            XEvent* xe = (XEvent*) event_buffer;
            XtAppNextEvent(gtApplicationShellXm::get_context(), xe);
 
            if (modal_event_record_pre)
                (*modal_event_record_pre)(*xe, 1);
 
            if (xe->type == ClientMessage)
                driver_client_hook (xe);
	    else
                XtDispatchEvent(xe);

	    if (xe->type == Expose)
		((gtShell*)this)->bring_to_top ();

            if (modal_event_record_post)
                (*modal_event_record_post)(*xe, 1);
        }
    }
 
    pop_cursor();
}
 
extern "C" void ste_unfreeze();

void gtBase::epoch_take_control(int (*cond)(void *), void *cd)
//
// Continuously handle events until (*cond)(cd) returns 0.
//
// If modal_event_record returns a non-zero value, it is expected that
// it will take control and loop until (*cond)(cd) returns 0.  Then,
// when it is finished, the rest of the event loop will be skipped.
//
{
//    push_cursor(None);

    int is_playback = 0;
    if (modal_event_playback)
	is_playback = (*modal_event_playback)(cond, cd);
    
    if(!is_playback)
    {
	while(!(*cond)(cd))
	{

//---------------------------------------------------------------------------
// The following kludgery is needed to write around a bug in the HP C compiler
// optimizer which crashes during compilation if you take the address of a union
// like XEvent.  We thus define a buffer of the right size and just make xe a
// pointer to the buffer instead of declaring xe as an XEvent directly and
// doing &xe in all the contexts in which an XEvent* is required.
//---------------------------------------------------------------------------

            char event_buffer[sizeof(XEvent)];
 	    XEvent* xe = (XEvent*) event_buffer;

	    XtAppNextEvent(gtApplicationShellXm::get_context(), xe);
	    if (xe->type != ButtonPress && xe->type != ButtonRelease) {
		if (modal_event_record_pre)
		    (*modal_event_record_pre)(*xe, 1);

		if (xe->type == ClientMessage) {
		    driver_client_hook (xe);
		    ste_unfreeze (); // what we need from flush_output();
		} else 
		    XtDispatchEvent(xe);

		if (modal_event_record_post)
		    (*modal_event_record_post)(*xe, 1);
	    }
	}
    }

//    pop_cursor();
}

gtRep* gtBase::rep()
{
    return r;
}

char* gtBase::pathname()
{
    return r->pathname();
}

void gtBase::manage()
{
    XtManageChild(r->widget());
}

void gtBase::unmanage()
{
    XtUnmanageChild(r->widget());
}

int gtBase::is_managed()
{
    return XtIsManaged(r->widget());
}

gtBase* gtBase::container()
{
    return this;
}

void gtBase::cursor(gtCursor c)
{
    Widget w = r->widget();
    if(w && XtIsRealized(w))
    {
	Cursor xc = None;
	switch(c)
	{
	  case gtBusyCursor:
	    xc = get_busy_cursor();
	    break;

	  case gtDefaultCursor:
	    xc = None;
	    break;
	}
	XDefineCursor(UxDisplay, XtWindow(w), xc);
    }
}

void gtBase::update_display()
{
    XmUpdateDisplay(r->widget());
}

void gtBase::flush()
{
    XFlush(XtDisplay(r->widget()));
}

int gtBase::check_window_event(long mask, gtEventPtr eve)
{
    return XCheckWindowEvent(
	XtDisplay(r->widget()), XtWindow(r->widget()), mask, eve);
} 

int gtBase::check_mask_event(long mask, gtEventPtr eve)
{
    return XCheckMaskEvent(XtDisplay(r->widget()), mask, eve);
} 

void gtBase::event_handler(long m, int maskable, gtEventHandler e_h, void * cd)
{
    XtAddEventHandler(
	r->widget(), m, maskable, (XtEventHandler)e_h, (XtPointer)cd);
}

void gtBase::bell(int vol)
{
    XBell(XtDisplay(r->widget()), vol);
}

void gtBase::dispatch(gtEventPtr eve)
{
    XtDispatchEvent(eve);
}

void gtBase::user_destroy(void (*func)(gtBase*, void*), void* data)
{
    user_destroy_func = func;
    user_destroy_data = data;
}

int gtBase::my_window(gtEventPtr eve)
{
    return (eve->xany.window == XtWindow(r->widget()));
}

void *gtBase::get_window()
{
    return (void*)XtWindow(r->widget());
}

gtReasonXm::gtReasonXm(int XmReasonConstant)
{
    switch(XmReasonConstant)
    {
      case XmCR_ACTIVATE:	r_code = gtActivate;		break;
      case XmCR_VALUE_CHANGED:	r_code = gtValueChanged;	break;
      case XmCR_INCREMENT:	r_code = gtIncrement;		break;
      case XmCR_DECREMENT:	r_code = gtDecrement;		break;
      case XmCR_PAGE_DECREMENT:	r_code = gtPageDecrement;	break;
      case XmCR_PAGE_INCREMENT:	r_code = gtPageIncrement;	break;
      case XmCR_TO_TOP:		r_code = gtToTop;		break;
      case XmCR_TO_BOTTOM:	r_code = gtToBottom;		break;
      case XmCR_DRAG:		r_code = gtDrag;		break;
      case XmCR_EXPOSE:		r_code = gtExpose;		break;
      case XmCR_RESIZE:		r_code = gtResize;		break;
      case XmCR_SINGLE_SELECT:	r_code = gtSingleSel;		break;
      case XmCR_MULTIPLE_SELECT: r_code = gtMultipleSel;	break;
      case XmCR_EXTENDED_SELECT: r_code = gtExtendedSel;	break;
      case XmCR_BROWSE_SELECT:	r_code = gtBrowseSel;		break;
      case XmCR_MODIFYING_TEXT_VALUE: r_code = gtModifyText;	break;
      case XmCR_HELP:		r_code = gtHelp;		break;
      case XmCR_DEFAULT_ACTION:	r_code = gtDefaultAction;	break;
      case XmCR_FOCUS:		r_code = gtFocus;		break;
      case XmCR_LOSING_FOCUS:	r_code = gtUnfocus;		break;
      case XmCR_MAP:		r_code = gtMap;			break;	
      case XmCR_UNMAP:		r_code = gtUnmap;		break;	
      default:			r_code = gtNoMatch;		break;
    }
}

int gtReasonXm::reason_code()
{
    switch(r_code)
    {
      case gtActivate:		return XmCR_ACTIVATE;
      case gtValueChanged:	return XmCR_VALUE_CHANGED;
      case gtIncrement:		return XmCR_INCREMENT;
      case gtDecrement:		return XmCR_DECREMENT;
      case gtPageDecrement:	return XmCR_PAGE_DECREMENT;
      case gtPageIncrement:	return XmCR_PAGE_INCREMENT;
      case gtToTop:		return XmCR_TO_TOP;
      case gtToBottom:		return XmCR_TO_BOTTOM;
      case gtDrag:		return XmCR_DRAG;
      case gtExpose:		return XmCR_EXPOSE;
      case gtResize:		return XmCR_RESIZE;
      case gtSingleSel:		return XmCR_SINGLE_SELECT;
      case gtMultipleSel:	return XmCR_MULTIPLE_SELECT;
      case gtExtendedSel:	return XmCR_EXTENDED_SELECT;
      case gtBrowseSel:		return XmCR_BROWSE_SELECT;
      case gtModifyText:	return XmCR_MODIFYING_TEXT_VALUE;
      case gtHelp:		return XmCR_HELP;
      case gtDefaultAction:	return XmCR_DEFAULT_ACTION;
      case gtFocus:		return XmCR_FOCUS;
      case gtUnfocus:		return XmCR_LOSING_FOCUS;
      case gtMap:               return XmCR_MAP;
      case gtUnmap:             return XmCR_UNMAP;

      default:			;
    }
	return XmCR_NONE;
}

const char* gtReasonXm::reason_cb()
{
    switch(r_code)
    {
      case gtActivate:		return XmNactivateCallback;
      case gtValueChanged:	return XmNvalueChangedCallback;
      case gtDrag:		return XmNdragCallback;
      case gtIncrement:		return XmNincrementCallback;
      case gtDecrement:		return XmNdecrementCallback;
      case gtPageIncrement:	return XmNpageIncrementCallback;
      case gtPageDecrement:	return XmNpageDecrementCallback;
      case gtToTop:		return XmNtoTopCallback;
      case gtToBottom:		return XmNtoBottomCallback;
      case gtExpose:		return XmNexposeCallback;
      case gtResize:		return XmNresizeCallback;
      case gtDestroy:		return XmNdestroyCallback;
      case gtSingleSel:		return XmNsingleSelectionCallback;
      case gtMultipleSel:	return XmNmultipleSelectionCallback;
      case gtExtendedSel:	return XmNextendedSelectionCallback;
      case gtBrowseSel:		return XmNbrowseSelectionCallback;
      case gtModifyText:	return XmNmodifyVerifyCallback;
      case gtHelp:		return XmNhelpCallback;
      case gtDefaultAction:	return XmNdefaultActionCallback;
      case gtFocus:		return XmNfocusCallback;
      case gtUnfocus:		return XmNlosingFocusCallback;
      case gtMap:		return XmNmapCallback;
      case gtUnmap:		return XmNunmapCallback;

      default:			;
    }
    return NULL;
}

gtReason gtReasonXm::reason()
{
    return r_code;
}

#ifndef _ASET_OPTIMIZE
void ___oneshot_CB(void*xxx)
{
    cout << "one shot " << (int)xxx << endl;
}
extern "C" void  __oneshot_test()
{
  gtBase::oneshot(___oneshot_CB, (void*)33);
  gtBase::oneshot(___oneshot_CB, (void*)44);
  gtBase::oneshot(___oneshot_CB, (void*)55);
}
#endif

//------------------------------------------

void gtFree(void* ptr)
{
    XtFree((char*)ptr);
}

//------------------------------------------

/*
   START-LOG-------------------------------------------

   $Log: gtBaseXm.h.C  $
   Revision 1.11 2000/07/12 18:09:42EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.31  1994/07/28  19:19:13  jethran
removed quickhelp stuff as part of overall removal of quickhelp

Revision 1.2.1.30  1994/07/13  00:29:41  boris
Bug track: 7459
Fixed Purify mre error with double entering malloc

Revision 1.2.1.29  1994/04/04  18:40:19  boris
Added epoch macro scripting functionality

Revision 1.2.1.28  1994/03/18  22:19:50  aharlap
No more comments

Revision 1.2.1.27  1994/03/18  19:29:15  mg
Bug track: 1
oneshot implementation

Revision 1.2.1.26  1994/02/08  03:00:07  boris
commented out lines absolette code in ::epoch_take_control

Revision 1.2.1.25  1994/01/21  00:26:30  kws
New rtl display

Revision 1.2.1.24  1994/01/13  02:46:20  kws
Use psetmalloc

Revision 1.2.1.23  1993/11/30  16:39:31  aharlap
Bug track: # 5252
call bring_to_top from take_control for Expose

Revision 1.2.1.22  1993/09/10  20:55:04  boris
Fixed double click crash on OK in Rename dialog box

Revision 1.2.1.21  1993/09/09  23:08:33  boris
Fixed synchro problem while parsing

Revision 1.2.1.20  1993/08/28  00:04:31  boris
Fixed bug #4596 with ParasET hanging

Revision 1.2.1.19  1993/07/20  18:22:26  harry
HP/Solaris Port

Revision 1.2.1.18  1993/05/20  16:32:43  jon
Put in flag to cmd_handle_event_pre/post to indicate modal

Revision 1.2.1.17  1993/05/12  23:30:12  jon
Removed auto test tool and command journaling code and installed modal
patchpoints instead.

Revision 1.2.1.16  1993/05/12  00:29:54  jon
Use pre and post event handler for command journal recording

Revision 1.2.1.15  1993/05/07  19:47:26  jon
Added call to cmd_handle_event in take_control so we can record
actions in modal dialog boxes

Revision 1.2.1.14  1993/04/22  21:23:43  davea
bug 3461 - add returns to end of two functions

Revision 1.2.1.13  1993/04/14  18:07:44  glenn
Use XtAppNextEvent instead of XtAppProcessEvent.
(For MG).

Revision 1.2.1.12  1993/02/04  21:37:13  glenn
Add flush_output_callback, flush_output.

Revision 1.2.1.11  1993/01/28  19:10:33  oak
Fixed problem with take control.

Revision 1.2.1.10  1993/01/26  04:32:20  glenn
Remove realize, is_realized.
test ppmm_height in calc_ppmm.
Remove gtWindow class.  Just use XtWindow.

Revision 1.2.1.9  1993/01/25  21:06:58  jon
Added member functions for the help system, added patchpoints for
interfacing non-gt based widgets to the help system.

Revision 1.2.1.8  1993/01/22  17:14:12  glenn
Add border, margins.

Revision 1.2.1.7  1993/01/20  03:38:02  glenn
Add name.  Use hund_to_pix functions.
Trap dimensions <= 0 in width and height.

Revision 1.2.1.6  1993/01/11  16:31:20  oak
Changed gtBase::take_control to let the auto
test tool take back control.

Revision 1.2.1.5  1993/01/02  19:49:25  glenn
Add gtFree.

Revision 1.2.1.4  1992/10/23  17:37:12  glenn
Restore cursor to default during take_control.
Add cursor.

Revision 1.2.1.3  92/10/19  14:17:43  glenn
Add is_realized, is_mapped, is_visible, is_managed.

Revision 1.2.1.2  92/10/09  19:03:24  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/



