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
/* $Id: subwin.cxx 1.10 1996/02/18 13:14:37EST kws Exp  $ */

#include <vport.h>		/* for v*HEADER */

#include vkindHEADER
#include vloadableHEADER
#include vdebugHEADER
#include vwindowHEADER
#include vdialogHEADER
#include vcontainerHEADER

#include <subwin.h>

#undef wchar_t
 
#include vx11xlibHEADER

#ifndef _WIN32
    XSetWindowAttributes         attributes;
#else 
    typedef void** XEvent;
#endif

#if (vdebugDEBUG)

/*
 * Override the vdebugTAG in such a way that dumb compilers which don't
 * uniquify string constants won't create tons of tags
 */

static const char * const       _subwinTAG = "subwin";
#define vdebugTAG               _subwinTAG

#endif


/*
 * Kids don't try this at home.  Our experts undergo years of training to
 * know just how Galaxy can be stressed beyond it's everyday uses.  Here,
 * they have reached into the internals to give us a way to ask for some
 * X specific information that Galaxy is using.  This is unsupported and
 * subject to future change at any time.
 */

typedef void (*veventsysXHandlerProc)(XEvent *event);
    
Window			vwindowGetXWindow( vwindow * );

extern veventsysXHandlerProc        _veventsysXHandlerProc;
extern Display                     *_vwindowXDisplay;
extern int                          _vwindowXScreenNumber;


/*
 * Old X Handler
 *
 * At startup we'll grab the old X Handler and stuff it here.
 * Any Windows that aren't ours we'll pass along to this handler.
 */

static veventsysXHandlerProc	_subwinOldXHandlerProc;


/*
 * Dict
 *
 * A dictionary to hold all the Window IDs that we are using and associate
 * them with the subwin * that own them.
 */

static vdict			*_subwinDict;


void ui_resize_window (Display* disp, Window win)
/*
// Resize the child window(s) of "win" to be the same size as "win".
*/
{
   
#ifndef _WIN32
    if(win)
    {
        Window          root, parent;
        Window*         children;
        unsigned int    nchildren;
 
        if(XQueryTree(disp, win, &root, &parent, &children, &nchildren) &&
           nchildren > 0)
        {
            int                 i, x, y;
            unsigned int        w, h, bw, d;
 
            XGetGeometry(disp, win, &root, &x, &y, &w, &h, &bw, &d);
 
            for(i = 0; i < nchildren; ++i)
	        XMoveResizeWindow(disp, children[i], 0, 0,
			  w, h);
            XFree((char*)children);
        }
    }
#endif
}

/*
 * Handle X Event
 *
 * Determine wether the given X event is for one of our windows and
 * handle appropriately
 */

static void _subwinHandleXEvent(XEvent *xevent)
{
    subwin	*subwin_;

#ifndef _WIN32
    if (xevent->xany.window == None)
    {
        //printf ("1-%08x = window\n", xevent->xany.window);
	//vdebugMWarn(("XEvent with 0 for window\n"));
	    (*_subwinOldXHandlerProc)(xevent);
    }
    else
    {
        subwin_ = (subwin *)
	  _subwinDict->GetElem((void *) xevent->xany.window);

	if (subwin_ != NULL)
	{
        printf ("2-%08x = window\n", xevent->xany.window);
	    //vdebugMWarn(("event type: %d\n", xevent->type));

	    // X Event Handling code for our subwin goes here
	}
	else
	{
	    // Not us.  Let Galaxy have it.
            //printf ("3-%08x = window\n", xevent->xany.window);

	    (*_subwinOldXHandlerProc)(xevent);
	}
    }
#else
	//(*_subwinOldXHandlerProc)(xevent);
#endif
}



/*
 * Shutdown
 */

static void _subwinShutdown(void)
{
    #ifndef _WIN32
    _veventsysXHandlerProc = _subwinOldXHandlerProc;
	#endif

    delete _subwinDict;
}


/*
 * Validate X Window
 *
 * Make sure there is a valid X Window available for our use
 */

static void _subwinValidateXWindow(subwin *subwin)
{

    if (_subwinOldXHandlerProc == NULL)
    {
	// Setup the Window to subwin * dictionary

        _subwinDict = new vdict(vdictHashGeneric);


	// Force vwindow to install it's X handler so that we can grab it

	vwindow::GetRoot()->Open();

	// Grab it

#ifndef _WIN32
    _subwinOldXHandlerProc = _veventsysXHandlerProc;
	_veventsysXHandlerProc = _subwinHandleXEvent;
#endif

	// Set up our shutdown procedure to undo the above

	vclientRegisterGlobalShutdown(_subwinShutdown);
    }

#ifndef _WIN32

    if (subwin->display == NULL)
    {
	subwin->display   = _vwindowXDisplay;
	subwin->screenNum = _vwindowXScreenNumber;

#if 0
        attributes.xattrmask_ | = CWEventMask;
        attributes.xattrs_.event_mask = (
        KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask |
        PointerMotionMask | PointerMotionHintMask |
        ExposureMask |
        StructureNotifyMask |
        FocusChangeMask |
        OwnerGrabButtonMask
    );
 
    /*
     * These events are caught at the top-level and not propagated
     * out to the root window (meaning the window manager).
     */
    attributes.xattrmask_ |= CWDontPropagate;
    attributes.xattrs_.do_not_propagate_mask = (
        KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask
    );
#endif

	subwin->xwindow = XCreateWindow(subwin->display,
					XRootWindow(subwin->display,
						    subwin->screenNum),
					-1000,-1000, 1, 1,
					0,
					CopyFromParent,
					InputOutput,
					CopyFromParent,
					None,
					&attributes);
        XSelectInput (_vwindowXDisplay, subwin->xwindow,
        KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
        ExposureMask | EnterWindowMask | LeaveWindowMask |
        Button1MotionMask | Button3MotionMask | Button3MotionMask );
        //StructureNotifyMask | SubstructureNotifyMask | 
	//SubstructureRedirectMask );

        printf ("xwin   =====    %08x ====== \n", subwin->xwindow);
	_subwinDict->SetElem((void *)subwin->xwindow, subwin);
    }
#endif
}


// implementation of the vkindTyped overrides for the subwin class
vkindTYPED_FULL_DEFINITION(subwin, vdialogItem, "subwin");


/*
 * Init
 *
 * Initializes the contents of this object to default values.
 * It's assumed that this is not called on an already initialized object.
 */

void subwin::Init(void)
{
    vdialogItem::Init();

    this->display = NULL;
    this->xwindow = NULL;
    this->screenNum = 0;
}


/*
 * Load Init
 *
 * Initializes the contents of this object to default values.
 * It's assumed that this is not called on an already initialized object.
 */

void subwin::LoadInit(vresource resource)
{
  vdialogItem::LoadInit(resource);

  this->display = NULL;
  this->xwindow = NULL;
  this->screenNum = 0;
}


/*
 * Copy Init
 *
 * Initializes the contents of this object to default values.
 * It's assumed that this is not called on an already initialized object.
 */

void subwin::CopyInit(vloadable *original)
{
  vdialogItem::CopyInit(original);

  this->display   = NULL;
  this->xwindow   = NULL;
  this->screenNum = 0;
}


/*
 * Destroy
 *
 * Gets rid of all contents and even free's the space occupied by this
 * object, if not static
 */

void subwin::Destroy(void)
{
#ifndef _WIN32
    XDestroyWindow(this->display, 
    this->xwindow);
#endif

    vdialogItem::Destroy();
}


/*
 * Set Rect
 *
 * Gets rid of all contents and even free's the space occupied by this
 * object, if not static
 */

void subwin::SetRect(const vrect *my_rect)
{
    //printf ("Subwin SetRect called\n");
    vdialogItem::SetRect(my_rect);

    // !!! must handle subcontainers - do full transform

    if (this->IsOpen())
    {
        vrect xrect;

	// Transform our rect to the parent X Window's coordinate system
	// !!! doesn't handle containers yet

        vdialogItemList *my_itemList = this->GetItemListIn();
	my_itemList->TransformRect(my_rect, &xrect);

	vwindow *window = this->DetermineWindow();
	
	xrect.y = window->GetHeight() - (xrect.y + xrect.h);

#ifndef _WIN32
	XMoveResizeWindow(this->display, this->xwindow,
			  xrect.x, xrect.y,
			  xrect.w, xrect.h);
    ui_resize_window (this->display, this->xwindow);
#endif
    }
}


/*
 * Open
 *
 * Called when the dialog item is being placed into view.
 * We use this moment to insert our X window into the parent X window
 * that our parent vwindow is using
 */

void subwin::Open(void)
{
    vrect		 my_rect;

    vdialogItem::Open();

    // Now that the parent vwindow is open, there is definitely an X window
    // that we can use to place our subwindow inside of

    _subwinValidateXWindow(this);

    // Transform our rect to the parent X Window's coordinate system
    // !!! doesn't handle containers yet
    vdialogItemList *my_itemList = this->GetItemListIn();
    my_itemList->TransformRect(this->GetRect(), &my_rect);

    vwindow *window = this->DetermineWindow();
    my_rect.y = window->GetHeight() - (my_rect.y + my_rect.h);

#ifndef WIN32
    printf ("%08x -> %08x\n", vwindowGetXWindow(window),  this->xwindow);
    XReparentWindow(this->display, this->xwindow,
		    vwindowGetXWindow(window),
		    my_rect.x, my_rect.y);
    XResizeWindow(this->display, this->xwindow, my_rect.w, my_rect.h);
    XMapWindow(this->display, this->xwindow);

    unsigned long root;
    int x, y;
    unsigned w, h, bw, depth;
    XGetGeometry (this->display, this->xwindow, &root, &x, &y, &w, &h, &bw, &depth);
    //printf ("r=%d,x=%d,y=%d,w=%d,h=%d,bw=%d,d=%d\n", root,x,y,w,h,bw,depth);
#endif
}



/*
 * Close
 *
 * Called when the dialog item is being taken out of view.
 * We use this moment to grab our X window before its parent xwindown
 * is possibly destroyed
 */

void subwin::Close()
{
    // remove our x subwindow from the vwindow

#ifndef WIN32
    XUnmapWindow(this->display, this->xwindow);
    XReparentWindow(this->display, this->xwindow,
		    XRootWindow(this->display, this->screenNum),
		    -1000, -1000);
#endif

    vdialogItem::Close();
}

void subwin::Draw()
{
/*   vdraw::GSave();
   DrawDevice();
   vdraw::SetColor(DetermineForeground());
   vdraw::RectFill(GetX(), GetY(), GetWidth(), GetHeight());
   vdraw::GRestore();  */
}
