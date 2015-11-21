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
// gtPushButtonXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtLabel class in OSF Motif
//
//------------------------------------------

// INCLUDE FILES

#include <motifString.h>

#include <gtNoParent.h>
#include <gtBaseXm.h>
#include <gtPushButtonXm.h>
#include <gtBitmapXm.h>

#include <Xm/PushB.h>


// VARIABLE DEFINITIONS

const char* gtPushButtonXm::saved_help_context;


// FUNCTION DEFINITIONS

gtPushButton* gtPushButton::create(
    gtBase *parent, const char* name, const char *contents,
    gtPushCB callback, void *client_data)
{
    return new gtPushButtonXm(parent, name, contents, callback, client_data);
}


gtPushButton* gtPushButton::create(
    gtBase *parent, const char* name, const char *contents,
    gtPushCB callback, void *client_data, gtAlign a)
{
    gtPushButtonXm* pb =
	new gtPushButtonXm(parent, name, contents, callback, client_data);
    pb->alignment(a);

    return pb;
}

gtPushButton* gtPushButton::create(
    gtBase *parent, const char* name, gtBitmap* contents,
    gtPushCB callback, void *client_data)
{
    return new gtPushButtonXm(parent, name, (gtBitmapXm*)contents, callback, client_data);
}

void gtPushButton::next_help_context(const char* context)	// class static
{
    gtPushButtonXm::saved_help_context = context;
}

const char * gtPushButton::next_help_context()	// class static
{
    return gtPushButtonXm::saved_help_context;
}


gtPushButtonXm::gtPushButtonXm(
    gtBase* parent, const char* name, const char* label,
    gtPushCB callback, void* client_data)
{
    if(parent)
    {
	if(name == NULL)
	    name = "pushbutton";

	motifString st((char*)gettext(label));

	Arg args[3];
	XtSetArg(args[0], XmNlabelString, (XmString)st);
	XtSetArg(args[1], XmNmarginTop, 3);
	XtSetArg(args[2], XmNmarginBottom, 3);

	r->widget(XmCreatePushButton(parent->container()->rep()->widget(),
				     (char*)name, args, XtNumber(args)));

	if(!strcmp(name, "help") || (callback && callback == help_button_callback))
	{
	    callback = help_button_callback;
            client_data = NULL;
	    if(saved_help_context)
	    {
		// help_context_name(saved_help_context);
		saved_help_context = NULL;
	    }
	}
	if(callback)
	{
	    set_callback(callback, client_data);
	}
    }
    destroy_init();
}

gtPushButtonXm::gtPushButtonXm(
    gtBase* parent, const char* name, gtBitmapXm* bitmap,
    gtPushCB callback, void* client_data)
{
    if(parent)
    {
	if(name == NULL)
	    name = "pushbutton";

	Pixmap pixmap;
	XtVaGetValues(bitmap->rep()->widget(), XmNlabelPixmap, &pixmap, NULL);

	Arg args[4];
	XtSetArg(args[0], XmNlabelType, XmPIXMAP);
	XtSetArg(args[1], XmNlabelPixmap, pixmap);
	XtSetArg(args[2], XmNmarginTop, 3);
	XtSetArg(args[3], XmNmarginBottom, 3);

	r->widget(XmCreatePushButton(parent->container()->rep()->widget(),
				     (char*)name, args, XtNumber(args)));

	if(!strcmp(name, "help") || (callback && callback == help_button_callback))
	{
	    callback = help_button_callback;
            client_data = NULL;
	    if(saved_help_context)
	    {
		// help_context_name(saved_help_context);
		saved_help_context = NULL;
	    }
	}
	if(callback)
	{
	    set_callback(callback, client_data);
	}
    }
    destroy_init();
}


gtPushButtonXm::~gtPushButtonXm()
{}


gtBase* gtPushButtonXm::container()
{
    return this;
}

 
void gtPushButtonXm::set_callback(gtPushCB callback, void *client_data)
{
    add_callback(XmNactivateCallback, this,
		 gtCallbackFunc(callback), client_data);
}

    
void gtPushButtonXm::shadow_thickness(int thick)
{
    Arg	args[1];

    XtSetArg(args[0], XmNdefaultButtonShadowThickness, thick);
    XtSetValues(r->widget(), args, 1);
}


void gtPushButtonXm::title(const char *t)
{
    motifString str((char*)gettext(t));

    Arg	args[1];
    XtSetArg(args[0], XmNlabelString, (XmString)str);

    XtSetValues(r->widget(), args, 1); 
}


char* gtPushButtonXm::title()
//
// Return portion (usually all) of title string that uses default
// character set.
//
// Caller is responsible for calling gtFree() on result.
//
{
    Arg	args[1];
    XmString str;
    XtSetArg(args[0], XmNlabelString, &str);

    XtGetValues(r->widget(), args, 1);

    char* ans = NULL;
    XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, &ans);

    return ans;
}   

void gtPushButton_next_help_context(const char* context)
{
    gtPushButton::next_help_context(context);
}

/*
   START-LOG-------------------------------------------

   $Log: gtPushButtonXm.h.C  $
   Revision 1.3 1996/04/22 16:55:32EDT aharlap 
   Bug track: N/A
   pmod_server submission
Revision 1.2.1.13  1994/02/11  03:51:16  builder
Port

Revision 1.2.1.12  1993/11/17  19:49:10  jon
Bug track: 0
added constructor for push buttons with gtBitmaps
added member to return help context

Revision 1.2.1.11  1993/06/02  20:39:31  glenn
Return "this" from container().

Revision 1.2.1.10  1993/06/02  20:18:08  glenn
Remove Initialize and Return macros.  Remove genError.h.

Revision 1.2.1.9  1993/04/28  12:33:31  jon
Changed saved_help_context to be a static data member for gtPushButtonXm

Revision 1.2.1.8  1993/04/25  22:15:51  jon
Fixed code that automatically set the help context name to
set the callback correctly for the help menu items (bug 3489)

Revision 1.2.1.7  1993/04/21  19:37:11  glenn
Ignore callback if it is equal to help_button_callback when
setting help-context and callback for help buttons.

Revision 1.2.1.6  1993/04/19  14:05:37  glenn
Add gtPushButton::saved_help_context, next_help_context.
Modify ctor to use saved_help_context for "help" buttons.

Revision 1.2.1.5  1993/04/18  18:51:44  glenn
Do not call XmStringFree on XmStrings obtained from XtGetValues.
Add comments about callers responsibility to call gtFree on results
of some calls.

Revision 1.2.1.4  1993/01/26  04:53:21  glenn
Use gtCallbackMgrXm.

Revision 1.2.1.3  1993/01/22  17:14:12  glenn
Add create function that specifies alignment.

Revision 1.2.1.2  1992/10/09  19:03:43  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------

*/
