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

/*******************************************************************************
	viewer.c
	(Generated from interface file viewer.if)
*******************************************************************************/

#include <stdio.h>
#include "UxLib.h"
#include "UxDrawnB.h"
#include "UxDrArea.h"
#include "UxScrBar.h"
#include "UxMainW.h"
#include "UxRowCol.h"
#include "UxPushB.h"
#include "UxTogB.h"
#include "UxForm.h"

/*******************************************************************************
	Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

/*gbldecl*/
#include <varargs.h>
#include <systemMessages.h>
#include <representation.h>
#include <perspective.h>
#include <paraDebug_vs_c.h>
char *ste_c_editor_background ();
void  viewer_indirectly_map_history_pulldown(/* Widget */);
#include <uimx_context.h>
#define CONTEXT_PTR CONTEXT_PTR_NAME(viewer)
#define CURRENT_CONTEXT CURRENT_CONTEXT_NAME(Viewer)
/* To change these bitmaps, copy the three components of the bitmap	*/
/* (width, height, bits) into a separate file and run "bitmap" on it.	*/
/* Then paste the contents of the saved bitmap file back in here.	*/
#define target_off_width 16
#define target_off_height 16
static char target_off_bits[] = {
   0xe0, 0x07, 0x18, 0x18, 0x04, 0x20, 0xc2, 0x43, 0x22, 0x44, 0x11, 0x88,
   0x89, 0x91, 0xc9, 0x93, 0xc9, 0x93, 0x89, 0x91, 0x11, 0x88, 0x22, 0x44,
   0xc2, 0x43, 0x04, 0x20, 0x18, 0x18, 0xe0, 0x07};
#define target_on_width 16
#define target_on_height 16
static char target_on_bits[] = {
   0xe0, 0x47, 0x18, 0xe8, 0x04, 0x71, 0x42, 0x3b, 0x22, 0x5f, 0x91, 0x8f,
   0x89, 0x9f, 0x89, 0xbf, 0xc9, 0x87, 0x89, 0x90, 0x11, 0x88, 0x22, 0x44,
   0xc2, 0x43, 0x04, 0x20, 0x18, 0x18, 0xe0, 0x07};
Pixmap target_off_pixmap, target_on_pixmap;

/*******************************************************************************
	The definition of the context structure:
	If you create multiple instances of your interface, the context
	structure ensures that your callbacks use the variables for the
	correct instance.

	For each swidget in the interface, each argument to the Interface
	function, and each variable in the Instance Specific section of the
	Declarations Editor, there is an entry in the context structure.
	and a #define.  The #define makes the variable name refer to the
	corresponding entry in the context structure.
*******************************************************************************/

typedef	struct
{
	swidget	Uxviewer;
	swidget	Uxviewer_form;
	swidget	Uxtarget_toggle;
	swidget	Uxreparse_button;
	swidget	Uxbuttons_rowcol;
	swidget	UxviewOptionsPane;
	swidget	Uxdummy;
	swidget	UxviewOptions;
	swidget	UxmainWindow;
	swidget	UxHorizScrollBar;
	swidget	UxVertScrollBar;
	swidget	Uxform;
	swidget	Uxview_parent;
	swidget	Uxview_frame;
	swidget	Uxparent;
	void	*Uxviewer_obj;
	int	Uxbeen_exposed;
	Window	Uxview_window;
	Widget	Uxarg_parent_w;
	void	*Uxarg_viewer_obj;
	int	Uxarg_number;
} _UxCviewer;

#define viewer                  UxViewerContext->Uxviewer
#define viewer_form             UxViewerContext->Uxviewer_form
#define target_toggle           UxViewerContext->Uxtarget_toggle
#define reparse_button          UxViewerContext->Uxreparse_button
#define buttons_rowcol          UxViewerContext->Uxbuttons_rowcol
#define viewOptionsPane         UxViewerContext->UxviewOptionsPane
#define dummy                   UxViewerContext->Uxdummy
#define viewOptions             UxViewerContext->UxviewOptions
#define mainWindow              UxViewerContext->UxmainWindow
#define HorizScrollBar          UxViewerContext->UxHorizScrollBar
#define VertScrollBar           UxViewerContext->UxVertScrollBar
#define form                    UxViewerContext->Uxform
#define view_parent             UxViewerContext->Uxview_parent
#define view_frame              UxViewerContext->Uxview_frame
#define parent                  UxViewerContext->Uxparent
#define viewer_obj              UxViewerContext->Uxviewer_obj
#define been_exposed            UxViewerContext->Uxbeen_exposed
#define view_window             UxViewerContext->Uxview_window
#define arg_parent_w            UxViewerContext->Uxarg_parent_w
#define arg_viewer_obj          UxViewerContext->Uxarg_viewer_obj
#define arg_number              UxViewerContext->Uxarg_number

static _UxCviewer	*UxViewerContext;


/*******************************************************************************
	The following are translation tables.
*******************************************************************************/

static char	*view_frame_xlate = "#override\n\
<Key>Tab:view_frame_tab()\n";

/*******************************************************************************
	Forward declarations of functions that are defined later in this file.
*******************************************************************************/

Widget	create_viewer();

/*******************************************************************************
	Auxiliary code from the Declarations Editor:
*******************************************************************************/

/*auxdecl*/

void viewer_manage(UxWidget)
    Widget UxWidget;
/*
// Manage a viewer widget, then adjust its contraints.
*/
{
    if(!XtIsManaged(UxWidget))
    {
	int		n;
	Arg		args[2];
	Position	y;
	Dimension	height;

	PUSH_CONTEXT

	XtManageChild(UxWidget);

	/* Reset the minimum height to always keep */
	/* (at least) the top line of widgets visible. */
	n = 0;
	XtSetArg(args[n], XmNy, &y), n++;
	XtSetArg(args[n], XmNheight, &height), n++;
	XtGetValues(UxGetWidget(reparse_button), args, n);

	n = 0;
	XtSetArg(args[n], XmNpaneMinimum, (y * 2) + height); n++;
	XtSetArg(args[n], XmNpaneMaximum, 65000); n++;
	XtSetValues(UxWidget, args, n);

	POP_CONTEXT
    }
}

static void view_frame_event_handler(UxWidget, client_data, event, cont)
    Widget	UxWidget;
    XtPointer	client_data;
    XEvent*	event;
    Boolean*	cont;
/*
// Delegate events in view_frame to child window of view_parent.
*/
{
    Window		rt, prnt;
    Window*		children;
    unsigned int	nchildren;

    PUSH_CONTEXT

    if(XQueryTree(UxDisplay, view_window, &rt, &prnt, &children, &nchildren) &&
       nchildren > 0)
    {
	XSendEvent(UxDisplay, children[0], False, NoEventMask, event);
	XFree(children);
    }

    POP_CONTEXT
}

void viewer_help_mode(UxWidget)
    Widget UxWidget;
{
    PUSH_CONTEXT

    XtVaSetValues(UxGetWidget(viewOptions),
	XmNleftAttachment, XmATTACH_FORM,
	XmNleftWidget, NULL,
	XmNleftOffset, 90,
	NULL);
    XtUnmanageChild(UxGetWidget(reparse_button));
    XtUnmanageChild(UxGetWidget(target_toggle));

    POP_CONTEXT
}

static void manage_widget_list(va_alist)
    va_dcl
/*
   Call either XtUnmanageChildren or XtManageChildren on a list of
   swidgets.  Converts NULL-terminated variadic list of swidget pointers to
   array of Widgets.  All (s)widgets in the argument list must have the
   same parent.
*/
{
    va_list  pvar;
    int      flag;
    swidget* sw_ptr;
    Widget   w_array[32];
    Widget*  w_array_ptr = w_array;
    void (*func)();

    va_start(pvar);

    flag = va_arg(pvar, int);
    func = (flag ? XtManageChildren : XtUnmanageChildren);
    while((sw_ptr = va_arg(pvar, swidget*)) != NULL)
	*w_array_ptr++ = UxGetWidget(*sw_ptr);
    (*func)(w_array, w_array_ptr - w_array);

    va_end(pvar);
}

void viewer_indirectly_map_history_pulldown(UxWidget)
    Widget UxWidget;
{
    PUSH_CONTEXT
    XtUnmanageChild(UxGetWidget(buttons_rowcol));
    manage_widget_list(
        0,
        NULL);
    POP_CONTEXT
}

void viewer_activate(UxWidget)
    Widget UxWidget;
{
    if(UxWidget)
    {
	PUSH_CONTEXT

	if(been_exposed)
	    viewer_open_window(viewer_obj);

	POP_CONTEXT
    }
}

void viewer_show_current(UxWidget, value)
    Widget UxWidget;
    int    value;
{
    PUSH_CONTEXT

    if(value)
    {
	XmProcessTraversal(UxGetWidget(view_frame), XmTRAVERSE_CURRENT);
	XtVaSetValues(UxGetWidget(view_frame),
		      XmNshadowType, XmSHADOW_IN, NULL);
	if (is_emacs_editor_view(viewer_obj)) { 
		XtUnmapWidget(UxGetWidget(HorizScrollBar));
		XtUnmapWidget(UxGetWidget(VertScrollBar));
	} else {
		XtMapWidget(UxGetWidget(HorizScrollBar));
		XtMapWidget(UxGetWidget(VertScrollBar));
	}
    }
    else
    {
	XtVaSetValues(UxGetWidget(view_frame),
		      XmNshadowType, XmSHADOW_ETCHED_IN, NULL);
	XtUnmapWidget(UxGetWidget(HorizScrollBar));
	XtUnmapWidget(UxGetWidget(VertScrollBar));
    }

    POP_CONTEXT
}

void viewer_show_parse_button(UxWidget, can_parse, need_parse)
    Widget UxWidget;
    int can_parse;
    int need_parse;
{
    PUSH_CONTEXT

    XtSetSensitive(UxGetWidget(reparse_button), can_parse && need_parse);

    POP_CONTEXT
}

void viewer_set_target_toggle(UxWidget, set)
    Widget UxWidget;
    int set;
{
    Widget w;

    PUSH_CONTEXT

    w = UxGetWidget(target_toggle);
    if(XmToggleButtonGetState(w) != set)
    {
	XmToggleButtonSetState(w, set, 0);
	/* Force a redraw to work around Motif bug. */
	if(XtIsManaged(w)){
 		XtUnmapWidget(w);
		XtMapWidget(w);
	}
    }

    POP_CONTEXT
}

static int is_epoch_view(vwr)
    void* vwr;
{
    int t = viewer_get_type(vwr);
    return t == -1 || t == Rep_TextText  ||  t == Rep_SmtText  ||   t == Rep_RawText;
}

static int is_emacs_editor_view(vwr)
    void* vwr;
{
    int t = viewer_get_type(vwr);
    return t == -1 || t == Rep_SmtText || t == Rep_RawText;
}

static void set_epoch_screen(vwr)
    void* vwr;
{
    viewer_set_screen(vwr);
}

/*******************************************************************************
	The following are Action functions.
*******************************************************************************/

static void	action_view_frame_tab( UxWidget, UxEvent, UxParams, p_UxNumParams )
	Widget	UxWidget;
	XEvent	*UxEvent;
	String	*UxParams;
	int	*p_UxNumParams;
{
	int			UxNumParams = *p_UxNumParams;
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{}
	UxViewerContext = UxSaveCtx;
}

/*******************************************************************************
	The following are callback functions.
*******************************************************************************/

static void	valueChangedCB_target_toggle( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	viewer_target_lock(viewer_obj, XmToggleButtonGetState(UxWidget));
	}
	UxViewerContext = UxSaveCtx;
}

static void	activateCB_reparse_button( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	viewer_finalize(viewer_obj, UxWidget);
	viewer_reparse(viewer_obj);
	viewer_post_hook(viewer_obj, UxWidget);
	}
	UxViewerContext = UxSaveCtx;
}

static void	pageIncrementCB_HorizScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    XtCallCallbacks (UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	}
	UxViewerContext = UxSaveCtx;
}

static void	pageDecrementCB_HorizScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    XtCallCallbacks (UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	}
	UxViewerContext = UxSaveCtx;
}

static void	decrementCB_HorizScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    XtCallCallbacks (UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	}
	UxViewerContext = UxSaveCtx;
}

static void	incrementCB_HorizScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    XtCallCallbacks (UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	}
	UxViewerContext = UxSaveCtx;
}

static void	dragCB_HorizScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	/*
	    XtCallCallbacks (UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	*/
	}
	UxViewerContext = UxSaveCtx;
}

static void	valueChangedCB_HorizScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	   if(!is_epoch_view(viewer_obj)) {
	      int new_value = ((XmScrollBarCallbackStruct*)UxCallbackArg)->value;
	      viewer_manipulate_perspective(viewer_obj, VIEW_PAN_X, new_value);
	   }
	}
	UxViewerContext = UxSaveCtx;
}

static void	pageIncrementCB_VertScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	      XtCallCallbacks(UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	}
	UxViewerContext = UxSaveCtx;
}

static void	pageDecrementCB_VertScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	      XtCallCallbacks(UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	}
	UxViewerContext = UxSaveCtx;
}

static void	incrementCB_VertScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    if(is_epoch_view(viewer_obj))
		viewer_manipulate_perspective(viewer_obj, VIEW_PAN_SMALL_Y, 1);
	    else
		XtCallCallbacks(UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	}
	UxViewerContext = UxSaveCtx;
}

static void	decrementCB_VertScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    if(is_epoch_view(viewer_obj))
		viewer_manipulate_perspective(viewer_obj, VIEW_PAN_SMALL_Y, -1);
	    else
		XtCallCallbacks(UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	}
	UxViewerContext = UxSaveCtx;
}

static void	dragCB_VertScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    if(is_epoch_view(viewer_obj))
		XtCallCallbacks(UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	/*
	    if(!is_epoch_view(viewer_obj))
		XtCallCallbacks(UxWidget, XmNvalueChangedCallback, UxCallbackArg);
	*/
	}
	UxViewerContext = UxSaveCtx;
}

static void	valueChangedCB_VertScrollBar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    int new_value = ((XmScrollBarCallbackStruct*)UxCallbackArg)->value;
	    viewer_manipulate_perspective(viewer_obj, VIEW_PAN_Y, new_value);
	}
	UxViewerContext = UxSaveCtx;
}

static void	resizeCB_view_parent( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    if(XtIsRealized(UxWidget))
	    {
		XWindowAttributes xwa;
		XGetWindowAttributes(XtDisplay(UxWidget), XtWindow(UxWidget), &xwa);
		if(xwa.map_state != IsUnmapped)
		    ui_resize_window(XtWindow(UxWidget));
	    }
	}
	UxViewerContext = UxSaveCtx;
}

static void	exposeCB_view_parent( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCviewer              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerContext;
	UxViewerContext = UxContext =
			(_UxCviewer *) UxGetContext( UxThisWidget );
	{
	    char *color_name = ste_c_editor_background (); 
	    int color_size = strlen(color_name) + 1; 
	    XtVaSetValues(UxGetWidget(view_parent),
	 	XtVaTypedArg, XmNbackground, XmRString, color_name, color_size,
		NULL);
	    XClearWindow(UxDisplay, XtWindow(UxGetWidget(view_parent)));
	    if(is_epoch_view(viewer_obj))
	    {
		XtVaSetValues(UxGetWidget(HorizScrollBar),
			      XmNvalue, 0, XmNsliderSize, 100, NULL);
	    }
	    if(!been_exposed)	/* Call viewer_open_window once only. */
	    {
		been_exposed = 1;
		set_epoch_screen(viewer_obj);
		viewer_open_window(viewer_obj);
	    }
	}
	UxViewerContext = UxSaveCtx;
}

/*******************************************************************************
	The 'init_' function sets the private properties for all the
	swidgets to the values specified in the Property Table.
	Some properties need to be set after the X widgets have been
	created and the setting of these properties is done in the
	'build_' function after the UxCreateWidget call.
*******************************************************************************/

static void	_Uxinit_viewer()
{
	UxPutShadowThickness( viewer, 0 );
	UxPutNavigationType( viewer, "tab_group" );
	UxPutMarginWidth( viewer, 0 );
	UxPutMarginHeight( viewer, 0 );
	UxPutCreateManaged( viewer, "false" );
	UxPutHeight( viewer, 8100 );
	UxPutWidth( viewer, 11400 );
	UxPutUnitType( viewer, "100th_millimeters" );

	UxPutWidth( target_toggle, 750 );
	UxPutHeight( target_toggle, 750 );
	UxPutRecomputeSize( target_toggle, "false" );
	UxPutMarginBottom( target_toggle, 0 );
	UxPutMarginWidth( target_toggle, 0 );
	UxPutMarginTop( target_toggle, 0 );
	UxPutMarginHeight( target_toggle, 0 );
	UxPutLabelType( target_toggle, "pixmap" );
	UxPutShadowThickness( target_toggle, 60 );
	UxPutIndicatorOn( target_toggle, "false" );

	UxPutLabelString( reparse_button, gettext(TXT("Update")) );

	UxPutSpacing( buttons_rowcol, 0 );
	UxPutMarginWidth( buttons_rowcol, 0 );
	UxPutMarginHeight( buttons_rowcol, 0 );
	UxPutAdjustLast( buttons_rowcol, "false" );
	UxPutOrientation( buttons_rowcol, "horizontal" );

	UxPutRowColumnType( viewOptionsPane, "menu_pulldown" );

	UxPutLabelString( dummy, gettext(TXT("No View")) );

	UxPutLabelString( viewOptions, gettext(TXT("")) );
	UxPutSpacing( viewOptions, 0 );
	UxPutMarginWidth( viewOptions, 0 );
	UxPutMarginHeight( viewOptions, 0 );
	UxPutRowColumnType( viewOptions, "menu_option" );

	UxPutSpacing( mainWindow, 0 );
	UxPutHeight( mainWindow, 5400 );
	UxPutWidth( mainWindow, 10200 );

	UxPutOrientation( HorizScrollBar, "horizontal" );

	UxPutOrientation( VertScrollBar, "vertical" );

	UxPutTraversalOn( view_parent, "false" );
	UxPutNavigationType( view_parent, "none" );
	UxPutResizePolicy( view_parent, "resize_none" );

	UxPutTranslations( view_frame, view_frame_xlate );
	UxPutShadowType( view_frame, "shadow_etched_in" );
	UxPutLabelString( view_frame, "NO VIEW" );
	UxPutShadowThickness( view_frame, 120 );
	UxPutRecomputeSize( view_frame, "false" );
	UxPutMultiClick( view_frame, "multiclick_discard" );
	UxPutHighlightThickness( view_frame, 90 );

}

/*******************************************************************************
	The 'build_' function creates all the swidgets and X widgets,
	and sets their properties to the values specified in the
	Property Editor.
*******************************************************************************/
extern int isUpdateAvailable();
static swidget	_Uxbuild_viewer()
{
	/* Create the swidgets */

	viewer = UxCreateForm( "viewer", parent );
	UxPutContext( viewer, UxViewerContext );
	UxPutDefaultShell( viewer, "transientShell" );

	viewer_form = UxCreateForm( "viewer_form", viewer );
	target_toggle = UxCreateToggleButton( "target_toggle", viewer_form );
	reparse_button = UxCreatePushButton( "reparse_button", viewer_form );
	buttons_rowcol = UxCreateRowColumn( "buttons_rowcol", viewer_form );
	viewOptionsPane = UxCreateRowColumn( "viewOptionsPane", viewer_form );
	dummy = UxCreatePushButton( "dummy", viewOptionsPane );
	viewOptions = UxCreateRowColumn( "viewOptions", viewer_form );
	mainWindow = UxCreateMainWindow( "mainWindow", viewer );
	HorizScrollBar = UxCreateScrollBar( "HorizScrollBar", mainWindow );
	VertScrollBar = UxCreateScrollBar( "VertScrollBar", mainWindow );
	form = UxCreateForm( "form", mainWindow );
	view_parent = UxCreateDrawingArea( "view_parent", form );
	view_frame = UxCreateDrawnButton( "view_frame", form );

	_Uxinit_viewer();

	/* Create the X widgets */

	UxCreateWidget( viewer );
	UxPutRightAttachment( viewer_form, "attach_form" );
	UxPutLeftAttachment( viewer_form, "attach_form" );
	UxPutTopAttachment( viewer_form, "attach_form" );
	UxCreateWidget( viewer_form );

	UxPutTopOffset( target_toggle, 60 );
	UxPutTopAttachment( target_toggle, "attach_form" );
	UxPutLeftOffset( target_toggle, 90 );
	UxPutLeftAttachment( target_toggle, "attach_form" );
	UxCreateWidget( target_toggle );
	
	if(isUpdateAvailable()) {
	  UxPutTopOffset( reparse_button, 60 );
	  UxPutTopAttachment( reparse_button, "attach_form" );
	  UxPutLeftWidget( reparse_button, "target_toggle" );
	  UxPutLeftOffset( reparse_button, 150 );
	  UxPutLeftAttachment( reparse_button, "attach_widget" );
	}
	UxCreateWidget( reparse_button );
	if(!isUpdateAvailable())   XtUnmanageChild(UxGetWidget(reparse_button));
	
	UxPutRightOffset( buttons_rowcol, 0 );
	UxPutRightAttachment( buttons_rowcol, "attach_form" );
	UxPutTopOffset( buttons_rowcol, 0 );
	UxPutTopAttachment( buttons_rowcol, "attach_widget" );
	UxPutLeftOffset( buttons_rowcol, 0 );
	UxPutLeftAttachment( buttons_rowcol, "attach_form" );
	UxPutBottomOffset( buttons_rowcol, 0 );
	UxPutBottomAttachment( buttons_rowcol, "attach_form" );
	UxCreateWidget( buttons_rowcol );

	UxCreateWidget( viewOptionsPane );
	UxCreateWidget( dummy );
	if(isUpdateAvailable()) {
	  UxPutLeftWidget( viewOptions, "reparse_button" );
	} else {
	  UxPutLeftWidget( viewOptions, "target_toggle" );
	}
	UxPutTopOffset( viewOptions, 60 );
	UxPutLeftOffset( viewOptions, 30 );
	UxPutTopAttachment( viewOptions, "attach_form" );
	UxPutLeftAttachment( viewOptions, "attach_widget" );
	UxPutSubMenuId( viewOptions, "viewOptionsPane" );
	UxCreateWidget( viewOptions );

	UxPutTopOffset( mainWindow, 30 );
	UxPutRightOffset( mainWindow, 90 );
	UxPutRightAttachment( mainWindow, "attach_form" );
	UxPutBottomOffset( mainWindow, 90 );
	UxPutBottomAttachment( mainWindow, "attach_form" );
	UxPutTopWidget( mainWindow, "viewer_form" );
	UxPutTopAttachment( mainWindow, "attach_widget" );
	UxPutLeftOffset( mainWindow, 90 );
	UxPutLeftAttachment( mainWindow, "attach_form" );
	UxCreateWidget( mainWindow );

	UxCreateWidget( HorizScrollBar );
	UxCreateWidget( VertScrollBar );
	UxCreateWidget( form );
	UxPutTopOffset( view_parent, 210 );
	UxPutTopAttachment( view_parent, "attach_form" );
	UxPutRightOffset( view_parent, 210 );
	UxPutRightAttachment( view_parent, "attach_form" );
	UxPutLeftOffset( view_parent, 210 );
	UxPutLeftAttachment( view_parent, "attach_form" );
	UxPutBottomOffset( view_parent, 210 );
	UxPutBottomAttachment( view_parent, "attach_form" );
	UxCreateWidget( view_parent );

	UxPutTopOffset( view_frame, 0 );
	UxPutLeftOffset( view_frame, 0 );
	UxPutTopAttachment( view_frame, "attach_form" );
	UxPutRightOffset( view_frame, 0 );
	UxPutRightAttachment( view_frame, "attach_form" );
	UxPutLeftAttachment( view_frame, "attach_form" );
	UxPutBottomOffset( view_frame, 0 );
	UxPutBottomAttachment( view_frame, "attach_form" );
	UxCreateWidget( view_frame );


	UxAddCallback( target_toggle, XmNvalueChangedCallback,
			valueChangedCB_target_toggle,
			(XtPointer) UxViewerContext );

	UxAddCallback( reparse_button, XmNactivateCallback,
			activateCB_reparse_button,
			(XtPointer) UxViewerContext );

	UxAddCallback( HorizScrollBar, XmNpageIncrementCallback,
			pageIncrementCB_HorizScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( HorizScrollBar, XmNpageDecrementCallback,
			pageDecrementCB_HorizScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( HorizScrollBar, XmNdecrementCallback,
			decrementCB_HorizScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( HorizScrollBar, XmNincrementCallback,
			incrementCB_HorizScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( HorizScrollBar, XmNdragCallback,
			dragCB_HorizScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( HorizScrollBar, XmNvalueChangedCallback,
			valueChangedCB_HorizScrollBar,
			(XtPointer) UxViewerContext );

	UxAddCallback( VertScrollBar, XmNpageIncrementCallback,
			pageIncrementCB_VertScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( VertScrollBar, XmNpageDecrementCallback,
			pageDecrementCB_VertScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( VertScrollBar, XmNincrementCallback,
			incrementCB_VertScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( VertScrollBar, XmNdecrementCallback,
			decrementCB_VertScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( VertScrollBar, XmNdragCallback,
			dragCB_VertScrollBar,
			(XtPointer) UxViewerContext );
	UxAddCallback( VertScrollBar, XmNvalueChangedCallback,
			valueChangedCB_VertScrollBar,
			(XtPointer) UxViewerContext );

	UxAddCallback( view_parent, XmNresizeCallback,
			resizeCB_view_parent,
			(XtPointer) UxViewerContext );
	UxAddCallback( view_parent, XmNexposeCallback,
			exposeCB_view_parent,
			(XtPointer) UxViewerContext );


	/* Finally, call UxRealizeInterface to create the X windows
	   for the widgets created above. */

	UxRealizeInterface( viewer );

	UxMainWindowSetAreas( mainWindow, NULL_SWIDGET, NULL_SWIDGET,
			HorizScrollBar, VertScrollBar, form );
	return ( viewer );
}

/*******************************************************************************
	The following function includes the code that was entered
	in the 'Initial Code' and 'Final Code' sections of the
	Declarations Editor. This function is called from the
	'Interface function' below.
*******************************************************************************/

static Widget	_Ux_create_viewer( _Uxarg_parent_w, _Uxarg_viewer_obj, _Uxarg_number )
	Widget	_Uxarg_parent_w;
	void	*_Uxarg_viewer_obj;
	int	_Uxarg_number;
{
	swidget                 rtrn;
	_UxCviewer              *UxContext;

	UxViewerContext = UxContext =
		(_UxCviewer *) UxMalloc( sizeof(_UxCviewer) );

	arg_parent_w = _Uxarg_parent_w;
	arg_viewer_obj = _Uxarg_viewer_obj;
	arg_number = _Uxarg_number;
	{
		/*icode*/
		Widget vwr_widget, vo_widget, tt_widget;
		int n;
		Arg args[8];
		Screen* screen;
		Pixel fore_color, back_color;
		Dimension height;
		int depth;
		XtWidgetGeometry geom;
		parent = UxWidgetToSwidget(arg_parent_w);
		if(!UxIsValidSwidget(parent))
		{
		    fprintf(stderr, "viewer.if: invalid parent widget\n");
		    return NULL;
		}
		rtrn = _Uxbuild_viewer();

		/*fcode*/
		viewer_obj = arg_viewer_obj;
		been_exposed = 0;
		view_window = XtWindow(UxGetWidget(view_parent));
		/* INITIALIZE VIEWER OBJECT */
		viewer_init(viewer_obj, XtWindow(UxGetWidget(view_parent)),
			    UxGetWidget(viewOptions),
			    UxGetWidget(HorizScrollBar),
			    UxGetWidget(VertScrollBar),
		            UxGetWidget(viewer_form));
		/* LOCAL INTERFACE STATE */
		vwr_widget = UxGetWidget(rtrn);
		viewer_show_current(vwr_widget, 0);
		viewer_show_parse_button(vwr_widget, 0, 0);
		viewer_indirectly_map_history_pulldown(vwr_widget);
		/* VIEW_FRAME EVENT-HANDLER */
		XtAddEventHandler(UxGetWidget(view_frame),
			KeyPressMask|KeyReleaseMask,
			False, view_frame_event_handler, viewer_obj);
		/* LIST-OF-VIEWS OPTION-MENU */
		vo_widget = UxGetWidget(viewOptions);
		n = 0;
		XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING), n++;
		XtSetValues(XmOptionButtonGadget(vo_widget), args, n);
		/* TARGET TOGGLE BUTTON */
		tt_widget = UxGetWidget(target_toggle);
		n = 0;
		XtSetArg(args[n], XmNscreen, &screen), n++;
		XtSetArg(args[n], XmNforeground, &fore_color), n++;
		XtSetArg(args[n], XmNbackground, &back_color), n++;
		XtSetArg(args[n], XmNdepth, &depth), n++;
		XtGetValues(tt_widget, args, n);
		if(target_off_pixmap == NULL)
		    target_off_pixmap = XCreatePixmapFromBitmapData(
			UxDisplay, RootWindowOfScreen(screen),
			target_off_bits, target_off_width, target_off_height,
			fore_color, back_color, depth);
		if(target_on_pixmap == NULL)
		    target_on_pixmap = XCreatePixmapFromBitmapData(
			UxDisplay, RootWindowOfScreen(screen),
			target_on_bits, target_on_width, target_on_height,
			fore_color, back_color, depth);
		n = 0;
		if(target_off_pixmap)
		    XtSetArg(args[n], XmNlabelPixmap, target_off_pixmap), n++;
		if(target_on_pixmap)
		    XtSetArg(args[n], XmNselectPixmap, target_on_pixmap), n++;
		XtSetArg(args[n], XmNunitType, XmPIXELS), n++;
		XtSetArg(args[n], XmNwidth, target_off_width + 10), n++;
		XtSetArg(args[n], XmNheight, target_off_height + 10), n++;
		XtSetValues(tt_widget, args, n);
		/* PANED-WINDOW HEIGHT CONSTRAINT */
		n = 0;
		XtSetArg(args[n], XmNheight, &height), n++;
		XtGetValues(arg_parent_w, args, n);
		n = 0;
		XtSetArg(args[n], XmNallowResize, True); n++;
		XtSetValues(arg_parent_w, args, n);
		/* Set the minimum height to force the initial size of the viewer. */
		if(arg_number <= 0)
		   arg_number = 1;
		n = 0;
		XtSetArg(args[n], XmNpaneMinimum, height / (1 << (arg_number - 1)) - 100); n++;
		XtSetArg(args[n], XmNpaneMaximum, height / (1 << (arg_number - 1))); n++;
		/*XtSetArg(args[n], XmNpaneMaximum, 65000); n++;*/
		XtSetArg(args[n], XmNallowResize, True); n++;
		XtSetValues(vwr_widget, args, n);
		return vwr_widget;
	}
}

/*******************************************************************************
	The following is the 'Interface function' which is the
	external entry point for creating this interface.
	This function should be called from your application or from
	a callback function.
*******************************************************************************/

Widget	create_viewer( _Uxarg_parent_w, _Uxarg_viewer_obj, _Uxarg_number )
	Widget	_Uxarg_parent_w;
	void	*_Uxarg_viewer_obj;
	int	_Uxarg_number;
{
	Widget			_Uxrtrn;
	static int		_Uxinit = 0;

	if ( ! _Uxinit )
	{
		static XtActionsRec	_Uxactions[] = {
				{ "view_frame_tab", action_view_frame_tab }
				};

		XtAppAddActions( UxAppContext,
				_Uxactions,
				XtNumber(_Uxactions) );

		_Uxinit = 1;
	}

	_Uxrtrn = _Ux_create_viewer( _Uxarg_parent_w, _Uxarg_viewer_obj, _Uxarg_number );

	return ( _Uxrtrn );
}

/*******************************************************************************
	END OF FILE
*******************************************************************************/

