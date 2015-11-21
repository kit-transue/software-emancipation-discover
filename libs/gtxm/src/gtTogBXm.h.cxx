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
// gtTogBXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtToggleButton class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtTogBXm.h>

#include <Xm/ToggleB.h>


// FUNCTION DEFINITIONS

gtToggleButton* gtToggleButton::create(
    gtBase *parent, const char* name, const char* contents,
    gtToggleCB callback, void *client_data)
{
    return new gtToggleButtonXm(parent, name, contents, callback, client_data);
}

gtToggleButtonXm::gtToggleButtonXm(
    gtBase* parent, const char* name, const char* contents,
    gtToggleCB callback, void *client_data)
{
    if(parent)
    {
	XmString st = XmStringCreateSimple((char*)gettext(contents));

	Arg args[2];
	XtSetArg(args[0], XmNlabelString, st);
	XtSetArg(args[1], XmNindicatorType, XmN_OF_MANY);

	r->widget(XmCreateToggleButton(parent->container()->rep()->widget(),
				       (char*)name, args, 2));
	XmStringFree(st);

	set_callback(callback, client_data);
    }
    destroy_init();
}


gtToggleButtonXm::~gtToggleButtonXm()
{}


gtBase *gtToggleButtonXm::container()
{
    return NULL;
}

 
void gtToggleButtonXm::set(int state, int notify)
{
    XmToggleButtonSetState(r->widget(), state, notify);
}


int gtToggleButtonXm::set()
{
    return XmToggleButtonGetState(r->widget());
}


void gtToggleButtonXm::pixmap(const char *pm_file)
{
    Arg args[1];
    XtSetArg(args[0], XmNlabelPixmap, pm_file);
    XtSetValues(r->widget(), args, 1);
}


void gtToggleButtonXm::set_callback(gtToggleCB callback, void* client_data)
{
    add_callback(XmNvalueChangedCallback, this,
		 gtCallbackFunc(callback), client_data);
}    

void gtToggleButtonXm::title(const char *t)
{
    XmString str = XmStringCreateSimple((char*)gettext(t));

    Arg	args[1];
    XtSetArg(args[0], XmNlabelString, str);
    XtSetValues(r->widget(), args, 1); 

    XmStringFree(str);
}   

char *gtToggleButtonXm::title()
//
// Return portion (usually all) of title string that uses default
// character set.
//
// Caller is responsible for calling gtFree() on result.
//
{
    XmString str;

    Arg	args[1];
    XtSetArg(args[0], XmNlabelString, &str);
    XtGetValues(r->widget(), args, 1);

    char *ans = NULL;
    XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, &ans);

    return ans;
}   


/*
   START-LOG-------------------------------------------

   $Log: gtTogBXm.h.C  $
   Revision 1.1 1993/04/18 14:51:41EDT builder 
   made from unix file
Revision 1.2.1.4  1993/04/18  18:51:44  glenn
Do not call XmStringFree on XmStrings obtained from XtGetValues.
Add comments about callers responsibility to call gtFree on results
of some calls.

Revision 1.2.1.3  1993/01/26  05:01:11  glenn
Use gtCallbackMgrXm.
Remove construct member.
Add dtor.

Revision 1.2.1.2  1992/10/09  19:03:57  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
