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
  generic-popup-menu.c
  
  goes with generic-popup-menu.h
  encapsulates popup menu widget, built with Motif
  this file\'s functionality is to be replaced by gt\'s
  
*/

#include <cLibraryFunctions.h>

#include <stdio.h>
#include <varargs.h>

#include <Xm/Xm.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/SeparatoG.h>
#include <Xm/RowColumn.h>

#include <generic-popup-menu.h>


char* gettext(/* char* */);
void create_menu_children(/* ... */);
typedef void (*funcptr)();
void ui_set_child_sensitive(parent, childname, sens)
    Widget	parent;
    char*	childname;
    int		sens;		/* boolean */
/*
// make a named child of a widget sensitive/unsensitive
*/
{
    Widget child = XtNameToWidget(parent, childname);
    if (child != NULL)
	XtVaSetValues(child, XmNsensitive, sens, NULL);
    else
	OSapi_fprintf(stderr, "ui_set_child_sensitive -- child %s not found\n",
		childname);
}


void ui_set_toggle_child(parent, childname, set)
    Widget	parent;
    char*	childname;
    int		set;		/* boolean */
{
    Widget child = XtNameToWidget(parent, childname);
    if (child != NULL)
	XtVaSetValues(child, XmNset, set, NULL);
    else
        OSapi_fprintf(stderr, "ui_set_toggle_child -- %s not found\n", childname);
}


void popup_context_menu(popx, popy, menu)
    int		popx;
    int		popy;
    Widget	menu;
/*
// popup a popup-menu widget at the given coordinates
*/
{
    XtVaSetValues(menu, XmNx, popx, XmNy, popy, NULL);
    XtManageChild(menu);
}


/**************************************************************************/
/*									  */
/*			    HOW TO MAKE A POPUP MENU			  */
/*									  */
/* make_popup_widget(parent, callback_data, ...);	                  */
/*									  */
/* Widget parent: the widget to parent the popup menu from                */
/*									  */
/* void *callback_data: a pointer to be passed to all callback functions  */
/*	as client data (consult X documentation for further info)         */
/*									  */
/*									  */
/* Items are specified in any of the following forms:			  */
/*									  */
/* PUSH, labelstring, callback	  specifies a pushbutton		  */
/*    char *labelstring: the labelstring of the pushbutton                */
/*    void (*callback)(): the funtion to be called when the pushbutton    */
/*		is activated.						  */
/*									  */
/* TOGGLE, labelstring, callback  specifies a toggle button               */
/*    char *labelstring: the labelstring of the toggle button             */
/*    void (*callback)(): the funtion to be called when the toggle button's */
/*		value is changed.
/*									  */
/* SEPARATOR			specifies a separator widget              */
/*									  */
/* CASCADE, labelstring		specifies a cascade button		  */
/*	char *labelstring
/*									  */
/* END_SUBMENU 			specifies the end of a sub menu		  */
/*	Note that all items declared between the CASCADE and END_SUBMENU  */
/*	declarations will belong to the cascade button's sub-menu.	  */
/*									  */
/**************************************************************************/

static Widget make_submenu(parent, callback_data, argptr)
    Widget	parent;
    void*	callback_data;
    va_list*	argptr;
{
    static int subnum = 0;

    Widget sub;
    char subname[50];

    sprintf(subname, "submenu%d", subnum++);
    sub = XmCreatePulldownMenu(parent, subname, NULL, 0);
    create_menu_children(sub, callback_data, argptr);

    return sub;
}


void create_menu_children(parent, callback_data, argptr)
    Widget	parent;
    void*	callback_data;
    va_list*	argptr;
{
    int		type;
    int		num = 0;

    while((type = va_arg((*argptr), int)) != END_SUBMENU)
    {
	Widget wij_child = NULL;
	if(type == SEPARATOR)
	{
	    wij_child = (void*)XmCreateSeparator(parent, "sep", NULL, 0);
	}
	else
	{
	    Arg		xt_args[2];
	    char	name[128];
	    XmString	xmstr = NULL;
	    char*	callback_type = NULL;

	    char* label = gettext(va_arg((*argptr), char*));
	    if(label == NULL  ||  label[0] == '\0')
	    {
		/* Just in case someone forgot to name it. */
		sprintf(name, "item%d", num++);
		label = name;
	    }
	    else
	    {
		int i;
		for(i = 0; label[i]  &&  (i < sizeof(name) - 1); ++i)
		{
		    /* Translate "bad" characters in widget name. */
		    switch(label[i])
		    {
		      case '.': name[i] = ','; break;
		      case '*': name[i] = '+'; break;
		      case ':': name[i] = ';'; break;
		      case '?': name[i] = '/'; break;
		      case '!': name[i] = '|'; break;
		      default:  name[i] = label[i]; break;
		    }
		}
		name[i] = '\0';
	    }
	    xmstr = XmStringCreateSimple(gettext(label));
	    XtSetArg(xt_args[0], XmNlabelString, xmstr);

	    switch(type)
	    {
	      case TOGGLE:
		wij_child = XmCreateToggleButton(parent, name, xt_args, 1);
		callback_type = XmNvalueChangedCallback;
		break;

	      case PUSH:
		wij_child = XmCreatePushButton(parent, name, xt_args, 1);
		callback_type = XmNactivateCallback;
		break;

	      case CASCADE:
		XtSetArg(xt_args[1], XmNsubMenuId,
			 make_submenu(parent, callback_data, argptr));
		wij_child = XmCreateCascadeButton(parent, name, xt_args, 2);
		break;
	    }
	    if(callback_type)
	    {
		funcptr callback = va_arg((*argptr), funcptr);
		if(callback)
		    XtAddCallback(
			wij_child, callback_type, callback, callback_data);
	    }
	    XmStringFree(xmstr);
	}
	if(wij_child)
	    XtManageChild(wij_child);
    }
}


/*
   START-LOG-------------------------------------------

   $Log: generic-popup-menu.c  $
   Revision 1.7 2000/11/01 07:54:15EST sschmidt 
   General cleanup
 * Revision 1.2.1.9  1994/07/29  16:22:09  builder
 * Xm/Xm.h instead of Xm.h
 *
 * Revision 1.2.1.8  1994/07/27  00:09:59  jethran
 * added block header comment,
 * and changed #include uimx to #include X
 *
 * Revision 1.2.1.7  1993/10/04  20:30:38  kws
 * Port
 *
 * Revision 1.2.1.6  1993/08/26  23:35:13  jon
 * Fixed generate_menu to take a parent argument.  Added menu_muffler widget
 * as parent of popup menus so we could delete it when the memnu was unmapped.
 * This prevents the server from grabbing when it sees the 3rd mouse button.
 * bug 3642
 *
 * Revision 1.2.1.5  1993/05/10  23:10:40  glenn
 * Filter out bad characters in string used for widget name in
 * create_menu_children to fix bug #1152.
 *
 * Revision 1.2.1.4  1993/02/10  20:21:32  oak
 * Added ui_set_toggle_indexed_child.
 *
 * Revision 1.2.1.3  1992/12/18  02:38:21  smit
 * fix yet another memory problem.
 *
   END-LOG---------------------------------------------
*/
