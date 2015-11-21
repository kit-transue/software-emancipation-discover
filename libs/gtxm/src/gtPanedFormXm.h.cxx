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
// gtPanedFormXm.h.C
//------------------------------------------
// synopsis:
// 
//------------------------------------------

// INCLUDE FILES

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
#include <gtBaseXm.h>
#include <genError.h>
#include <gtPanedFormXm.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>

// FUNCTION DEFINITIONS

#define SPACE 4

GC         PanedFormGC;
static int gc_initialized = 0;

static void initialize_gc(Widget form)
{
    XGCValues values;

    values.foreground     = 0xEEEEEE;
    values.function       = GXxor;
    values.subwindow_mode = IncludeInferiors;
    PanedFormGC    = XtGetGC(form, GCForeground | GCFunction | GCSubwindowMode, &values);
    gc_initialized = 1;
}

gtPanedForm* gtPanedForm::create(gtBase *parent, const char* name)
{
    return new gtPanedFormXm(parent, name);
}

gtPanedFormXm::gtPanedFormXm(gtBase* parent, const char* name)
{
    form = gtForm::create(parent, name);
    delete r;
    attach_rep(form->rep());
    if(!gc_initialized)
	initialize_gc(form->rep()->widget());
    last_x = 0;
    XtAddEventHandler(form->rep()->widget(), StructureNotifyMask, FALSE,
		      (XtEventHandler)form_resize, (XtPointer)this);
} 

gtPanedFormXm::~gtPanedFormXm()
{
    for(int i = 0; i < childs.size(); ++i){
	child_infoPtr ptr = *childs[i];
	if(ptr->button){
	    XtRemoveEventHandler(ptr->button, ButtonPressMask, FALSE, (XtEventHandler)start_dragging, 
			      (XtPointer)ptr);
	    XtRemoveEventHandler(ptr->button, PointerMotionMask, FALSE, (XtEventHandler)do_dragging, 
			      (XtPointer)ptr);
	    XtRemoveEventHandler(ptr->button, ButtonReleaseMask, FALSE, (XtEventHandler)end_dragging,
			      (XtPointer)ptr);
	}
	delete ptr;
    }
}

void gtPanedFormXm::add_child(gtBase *child, int _width)
{
    child_infoPtr ptr = new child_info;
    if(ptr){
	ptr->button     = NULL;
	ptr->gt_element = child;
	ptr->width      = _width;
	ptr->form       = this;
	ptr->index      = childs.size();
	ptr->rel_x      = 0;
	childs.append(&ptr);
    }
}

void gtPanedFormXm::calculate_relative(void)
{
    Dimension actual_width;
    
    XtVaGetValues(form->rep()->widget(), XmNwidth, &actual_width, NULL);
    float one_percent = (float)actual_width / 100;
    if(actual_width != width){ // We cannot calculate positions in that case (for example .Xdefaults overrides geometry)
	if(one_percent <= 0.01)
	    return;
	for(int i = 0; i < childs.size() - 1; ++i){
	    child_infoPtr ptr = *childs[i];
	    if(ptr->rel_x >= 0.0001){
		XtVaSetValues(ptr->button, XmNleftOffset, (int)(ptr->rel_x * one_percent), NULL);
		XtVaSetValues(ptr->separator, XmNleftOffset, (int)(ptr->rel_x * one_percent) + SPACE, NULL);
	    }
	}
	width = actual_width;
    } else {
	for(int i = 0; i < childs.size() - 1; ++i){
	    int x;
	    child_infoPtr ptr = *childs[i];
	    XtVaGetValues(ptr->button, XmNleftOffset, &x, NULL);
	    if(one_percent)
		ptr->rel_x = (float)x / one_percent;
	    else
		ptr->rel_x = 0;
	}
    }
}


void gtPanedFormXm::layout(void)
{
    int offset = 0;

    int i;
    for(i = 0; i < childs.size(); ++i){
	child_infoPtr ptr  = *childs[i];
	Widget        elem = ptr->gt_element->rep()->widget();
	XtVaSetValues(elem,
		      XmNtopAttachment, XmATTACH_FORM, 
		      XmNbottomAttachment, XmATTACH_FORM, 
		      NULL);
	offset += ptr->width;
	if(i == 0){
	    XtVaSetValues(elem,
			  XmNleftAttachment, XmATTACH_FORM,
			  NULL);
	}else{
	    child_infoPtr prev = *childs[i - 1];
	    XtVaSetValues(elem,
			  XmNleftAttachment, XmATTACH_WIDGET,
			  XmNleftWidget, prev->separator,
			  XmNleftOffset, SPACE,
			  NULL);
	    
	}
	if(i != childs.size() - 1){
	    ptr->button = XtVaCreateManagedWidget("sep_button", xmPushButtonWidgetClass, r->widget(),
						  XmNtopAttachment, XmATTACH_FORM,
						  XmNwidth, SPACE * 2 + 1,
						  XmNheight, 10,
						  XmNleftAttachment, XmATTACH_FORM,
						  XmNleftOffset, offset,
						  XmNfillOnArm, 0,
						  XmNtraversalOn, False,
						  NULL);
	    ptr->separator = XtVaCreateManagedWidget("sep_sep", xmSeparatorWidgetClass, r->widget(),
						     XmNtopAttachment, XmATTACH_WIDGET,
						     XmNtopWidget, ptr->button,
						     XmNbottomAttachment, XmATTACH_FORM,
						     XmNorientation, XmVERTICAL,
						     XmNleftAttachment, XmATTACH_FORM,
						     XmNleftOffset, offset + SPACE,
						     NULL);
	    XtVaSetValues(elem,
			  XmNrightAttachment, XmATTACH_WIDGET,
			  XmNrightWidget, ptr->separator,
			  XmNrightOffset, SPACE,
			  NULL);

	    XtAddEventHandler(ptr->button, ButtonPressMask, FALSE, (XtEventHandler)start_dragging, 
			      (XtPointer)ptr);
	    XtAddEventHandler(ptr->button, PointerMotionMask, FALSE, (XtEventHandler)do_dragging, 
			      (XtPointer)ptr);
	    XtAddEventHandler(ptr->button, ButtonReleaseMask, FALSE, (XtEventHandler)end_dragging,
			      (XtPointer)ptr);
	    
	    offset += SPACE + 2 + 1;
	}else{
	    XtVaSetValues(elem,
			  XmNwidth, ptr->width,
			  XmNrightAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
    width = offset;
    XtAddEventHandler(form->rep()->widget(), StructureNotifyMask, FALSE, (XtEventHandler)gtPanedFormXm::map_callback,
		      (XtPointer)this);
    float one_percent = (float)width / 100;
    for(i = 0; i < childs.size() - 1; ++i){
	int x;
	child_infoPtr ptr = *childs[i];
	XtVaGetValues(ptr->button, XmNleftOffset, &x, NULL);
	if(one_percent)
	    ptr->rel_x = (float)x / one_percent;
	else
	    ptr->rel_x = 0;
    }
}

void gtPanedFormXm::map_callback(Widget, void *data, XEvent *ev)
{
    if(ev->type != MapNotify)
	return;
    gtPanedFormXm *form = (gtPanedFormXm *)data;
    form->calculate_relative();
}

void gtPanedFormXm::start_dragging(Widget W, void *data, XEvent *)
{
    Dimension     height;
    child_infoPtr ptr   = (child_infoPtr)data;
    gtPanedFormXm *form = ptr->form;
    Widget        form_widget = form->rep()->widget();
    
    XtVaGetValues(form_widget, XmNheight, &height, NULL);
    form->last_height = height;
    XtVaGetValues(W, XmNleftOffset, &form->last_x, NULL);
    form->last_x += SPACE;
    XDrawLine(XtDisplay(form_widget), XtWindow(form_widget), PanedFormGC, form->last_x, 11, 
	      form->last_x, form->last_height);
}

void gtPanedFormXm::do_dragging(Widget W, void *data, XEvent *ev)
{
    Dimension     width;
    child_infoPtr ptr   = (child_infoPtr)data;
    gtPanedFormXm *form = ptr->form;
    Widget        form_widget = form->rep()->widget();

    if(form->last_x == 0)
	return;
    XDrawLine(XtDisplay(form_widget), XtWindow(form_widget), PanedFormGC, form->last_x, 11, 
	      form->last_x, form->last_height);
    XtVaGetValues(form_widget, XmNwidth, &width, NULL);
    int x;
    XtVaGetValues(W, XmNleftOffset, &x, NULL);
    form->last_x = x + ev->xbutton.x;
    int index    = ptr->index;
    // Check boundaries for left corner
    int left_panes = ptr->index + 1;
    if(form->last_x < (SPACE * 2) * left_panes + SPACE)
	form->last_x = (SPACE * 2) * left_panes + SPACE;
    // Check boundaries for right corner
    int right_panes = form->childs.size() - index;
    if(form->last_x > width - (SPACE * 2) * right_panes - SPACE)
	form->last_x = width - (SPACE * 2) * right_panes - SPACE;
    XDrawLine(XtDisplay(form_widget), XtWindow(form_widget), PanedFormGC, form->last_x, 11, 
	      form->last_x, form->last_height);
}

void gtPanedFormXm::end_dragging(Widget W, void *data, XEvent *)
{
    child_infoPtr ptr   = (child_infoPtr)data;
    gtPanedFormXm *form = ptr->form;
    Widget        form_widget = form->rep()->widget();

    if(form->last_x == 0)
	return;
    XDrawLine(XtDisplay(form_widget), XtWindow(form_widget), PanedFormGC, form->last_x, 11, 
	      form->last_x, form->last_height);
    
    int current_x = form->last_x - SPACE;
    int index     = ptr->index;
    
    // Check the position of all other panes, ajust if necessary.
    int i;
    for(i = index - 1; i >= 0; i--){
	child_infoPtr p = *form->childs[i];
	int x;
	XtVaGetValues(p->button, XmNleftOffset, &x, NULL);
	if(current_x - SPACE * 2 <= x){
	    x = current_x - SPACE * 2;
	    XtVaSetValues(p->button, XmNleftOffset, x, NULL);
	    XtVaSetValues(p->separator, XmNleftOffset, x + SPACE, NULL);
	    current_x = x;
	}
	else
	    break;
    }
    current_x = form->last_x - SPACE;
    int size  = form->childs.size() - 1;
    for(i = index + 1; i < size; i++){
	child_infoPtr p = *form->childs[i];
	int x;
	XtVaGetValues(p->button, XmNleftOffset, &x, NULL);
	if(current_x + SPACE * 2 >= x){
	    x = current_x + SPACE * 2;
	    XtVaSetValues(p->button, XmNleftOffset, x, NULL);
	    XtVaSetValues(p->separator, XmNleftOffset, x + SPACE, NULL);
	    current_x = x;
	}
	else
	    break;
    }

    XtVaSetValues(W, XmNleftOffset, form->last_x - SPACE,
		  NULL);
    XtVaSetValues(ptr->separator, XmNleftOffset, form->last_x,
		  NULL);
    form->last_x = 0;
    form->calculate_relative();
}

void gtPanedFormXm::form_resize(Widget, void *data, XEvent *ev)
{
    if(ev->type != ConfigureNotify)
	return;
    
    gtPanedFormXm *form = (gtPanedFormXm *)data;
    int           width = ev->xconfigure.width;

    if(form->width == width)
	return;
    form->width     = width;
    float one_percent = (float)width / 100;
    if(one_percent <= 0.01)
	return;

    for(int i = 0; i < form->childs.size() - 1; ++i){
	child_infoPtr ptr = *form->childs[i];
	if(ptr->rel_x >= 0.0001){
	    XtVaSetValues(ptr->button, XmNleftOffset, (int)(ptr->rel_x * one_percent), NULL);
	    XtVaSetValues(ptr->separator, XmNleftOffset, (int)(ptr->rel_x * one_percent) + SPACE, NULL);
	}
    }
}
