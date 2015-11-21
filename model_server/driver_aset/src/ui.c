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
/*
// ui.c
//------------------------------------------
// synopsis:
// UI functions that are best done in C rather than C++.
//------------------------------------------
*/

/* INCLUDE FILES */

#ifndef NEW_UI
#define Object XObject
#include <UxLib.h>
#undef Object

#include <ui.h>


void ui_resize_window(win)
    Window	win;
/*
// Resize the child window(s) of "win" to be the same size as "win".
*/
{
    if(win)
    {
	Window		root, parent;
	Window*		children;
	unsigned int	nchildren;

	if(XQueryTree(UxDisplay, win, &root, &parent, &children, &nchildren) &&
	   nchildren > 0)
	{
	    int			i, x, y;
	    unsigned int	w, h, bw, d;

	    XGetGeometry(UxDisplay, win, &root, &x, &y, &w, &h, &bw, &d);

	    for(i = 0; i < nchildren; ++i)
		XResizeWindow(UxDisplay, children[i], w, h);

	    XFree(children);
	}
    }
}


psetCONST char* ui_get_resource(rsrc_name, rsrc_class, valuePtr)
    psetCONST char* rsrc_name;
    psetCONST char* rsrc_class;
    uiValuePtr valuePtr;
/*
// Retrieve a resource value from the X Intrisics database.
*/
{
    char* type;
    XrmValue xrm_value;
    XrmDatabase db;
	
    if (!UxDisplay)
	return NULL;
    
    db = XtDatabase(UxDisplay);

    if(XrmGetResource(db, rsrc_name, rsrc_class, &type, &xrm_value))
    {
	valuePtr->size = xrm_value.size;
	valuePtr->addr = xrm_value.addr;
	return type;
    }
    else
    {
	valuePtr->size = 0;
	valuePtr->addr = NULL;
	return NULL;
    }
}


char *ui_get_value (name, option)
    char *name;
    char *option;
/*
// Retrieves resource value
*/
{
    char *r_val = NULL;

    if(name && option && UxDisplay)
	r_val = (char*)XGetDefault(UxDisplay, name, option);

    return r_val;
}


static int ui_get_scroll_value(scroll)
    Widget scroll;
/*
// Gets scroll bar value
*/
{
    return scroll ? UxGetValue(UxWidgetToSwidget(scroll)) : 0;
}



void ui_set_scroll_range (scroll, min, max)
    Widget scroll;
    int min, max;
/*
// Sets scroll bar range
*/
{
    if (scroll)
    {
	int value;
	swidget sw_scroll;
	int slider_size;

	/* get scroll bar swidget */
	sw_scroll = UxWidgetToSwidget (scroll);

	/* Make sure current value slider size are within the new range */
	if(min >= max)
	    max = min + 1;

	slider_size = UxGetSliderSize(sw_scroll);
	if(slider_size > max - min)
	    slider_size = max - min;

	value = ui_get_scroll_value(scroll);
	if(value > max - slider_size)
	    value = max - slider_size;
	else if(value < min)
	    value = min;

	/* We need to set all the values at once to avoid getting an */
	/* XtWarning if one of the old values is out of range.  -jef */
	XtVaSetValues(scroll,
		      XmNmaximum, max,
		      XmNminimum, min,
		      XmNvalue, value,
		      XmNsliderSize, slider_size,
		      NULL);
    }
}


void ui_get_scroll_range (scroll, min, max)
    Widget scroll;
    int *min, *max;
{
    if (scroll)
    {
	/* get scroll bar swidget */
	swidget sw_scroll;
	sw_scroll = UxWidgetToSwidget (scroll);

	*min = UxGetMinimum (sw_scroll);
	*max = UxGetMaximum (sw_scroll);
    }
}


void ui_set_scroll_value (scroll, val)
    Widget scroll;
    int    val;
/*
// Sets scroll bar value
*/
{
    swidget sw_scroll;
    int     min;
    int     max;
    int     slider_size;

    if (scroll)
    {
	/* get scroll bar swidget */
	sw_scroll = UxWidgetToSwidget (scroll);
	
	/* Get minimum, maximum and slider size */
	min = UxGetMinimum (sw_scroll);
	max = UxGetMaximum (sw_scroll);
	slider_size = UxGetSliderSize(sw_scroll);
	
	/* Set Value after validation */
	if (val > min)
	{
	    if ((val -= slider_size) > min)
		if(val <= max)
		    UxPutValue (sw_scroll, val);
		else
		    UxPutValue (sw_scroll, max);
	    else
		UxPutValue (sw_scroll, min);
	}
	else
	    UxPutValue (sw_scroll, min);
    }
}


void ui_set_slider_size(scroll, val)
    Widget scroll;
    int    val;
/*
// Sets slider size
*/
{
    if (scroll)
    {
	swidget sw_scroll;
	int     min;
	int     max;
	int     old_val;
	int     old_slider_size;
	int     new_val;

	/* get scroll bar swidget */
	sw_scroll = UxWidgetToSwidget (scroll);

	/* Get minimum and maximum */
	min = UxGetMinimum (sw_scroll);
	max = UxGetMaximum (sw_scroll);
	old_val = UxGetValue (sw_scroll);
	old_slider_size = UxGetSliderSize (sw_scroll);

	/* Set value */
	if (val > 0)
	{
	    UxDelayUpdate (sw_scroll);

	    if (val > (max - min))
	    {
		UxPutSliderSize (sw_scroll, max-min);
		UxPutPageIncrement (sw_scroll, max-min);
		UxPutValue (sw_scroll, min);
	    }
	    else
	    {
		UxPutSliderSize (sw_scroll, val);
		UxPutPageIncrement (sw_scroll, val);
		if ((new_val = old_val + old_slider_size - val) > min)
		{
		    if (new_val < (max - val))
			UxPutValue (sw_scroll, new_val);
		    else
			UxPutValue (sw_scroll, max - val);
		}
		else
		    UxPutValue (sw_scroll, min);
	    }
	    UxUpdate (sw_scroll);
	}
    }
}


void ui_get_window_size(parent_wind, win_width, win_height)
    void* parent_wind;
    int* win_width;
    int* win_height;
{
    XWindowAttributes win_attr;

    if(XGetWindowAttributes(UxDisplay, (Window)parent_wind, &win_attr))
    {
	*win_width = win_attr.width;
	*win_height = win_attr.height;
    }
    else
    {
	*win_width = -100000;
	*win_height = -100000;
    }
}


/*
 * $Log: ui.c  $
 * Revision 1.4 2001/01/12 10:56:59EST sschmidt 
 * Port to new SGI compiler
 * Revision 1.2.1.8  1994/01/04  23:51:56  boris
 * Bug track: Macro CONST
 * Renamed macro CONST into psetCONST
 *
 * Revision 1.2.1.7  1993/10/23  00:35:18  aharlap
 * Bug track: # 5045
 * check UxDisplay to zero
 *
 * Revision 1.2.1.6  1993/05/12  22:30:07  glenn
 * Remove unused functions, reformat.
 *
 * Revision 1.2.1.5  1993/03/03  00:52:24  jon
 * Changed set_scroll_range to make sure scrollbar value and size
 * will be within the limits of the new range so we do not get the
 * XtWarning messages.
 *
 * Revision 1.2.1.4  1993/03/01  00:25:54  glenn
 * Fix ui_resize_window to work for windows with height == 1
 * by calling XQueryTree instead of XTranslateCoordinates.
 * Move Log to bottom of file.
 *
 * Revision 1.2.1.3  1992/12/11  14:17:05  oak
 * Added ui_get_srcoll_value.
 *
 * Revision 1.2.1.2  1992/10/29  10:58:45  smit
 * fixes bug #1746 and 1748
 *
 * Revision 1.15  92/09/29  10:02:44  smit
 * fix scrollbars
 * 
 * Revision 1.14  92/09/24  11:08:45  smit
 * make scroll bars work better
 * 
 * Revision 1.13  92/09/18  16:53:41  glenn
 * Initialize childwin to avoid Purify error.
 * 
 * Revision 1.12  92/08/23  17:49:38  builder
 * defined Object
 * 
 * Revision 1.11  92/07/23  17:10:45  smit
 * added ui_synchronize for automatic testing.
 * 
 * Revision 1.10  92/03/27  11:42:59  builder
 * Fixed include path.
 * 
 * Revision 1.9  92/02/26  18:32:57  sergey
 * Added get_window_size call to get current width and height.
 * 
 * Revision 1.8  92/02/11  17:11:43  glenn
 * Implement ui_get_resource.
 * 
 * Revision 1.7  92/01/07  14:53:32  smit
 * Put in workaround for motif problem. 
 * 
 * Revision 1.6  91/12/20  19:34:47  smit
 * Added code for sliders.
 * 
 * Revision 1.5  91/12/19  18:48:43  smit
 * Fixed declaration
 * 
 * Revision 1.4  91/12/19  16:56:49  smit
 * Added functions to set scroll bar values.
 * 
 * Revision 1.3  91/11/27  11:29:52  aharlap
 * Function popup_transfer provides right return value type for dialog constuctors
 * 
 * Revision 1.2  91/11/13  12:39:48  smit
 * Added new function ui_get_value.
 * 
 * Revision 1.1  91/09/01  23:32:57  kws
 * Initial revision
 * 
 */
#endif
