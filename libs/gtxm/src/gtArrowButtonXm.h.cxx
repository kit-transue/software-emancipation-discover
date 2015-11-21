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
// gtArrowButtonXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtLabel class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtArrowButtonXm.h>

#include <Xm/ArrowB.h>


// FUNCTION DEFINITIONS

gtArrowButton* gtArrowButton::create(
    gtBase *parent, const char* name, gtCardinalDir dir,
    gtArrowCB callback, void *client_data)
{
    return new gtArrowButtonXm(parent, name, dir, callback, client_data);
}


gtArrowButtonXm::gtArrowButtonXm(
    gtBase* parent, const char* name, gtCardinalDir dir,
    gtArrowCB callback, void* client_data)
{
    if(parent)
    {
	if(name == NULL)
	    name = "arrowbutton";

	r->widget(XmCreateArrowButton(parent->container()->rep()->widget(),
				      (char*)name, NULL, 0));
	set_direction(dir);
	if(callback)
	    set_callback(callback, client_data);
    }
    destroy_init();
}


gtArrowButtonXm::~gtArrowButtonXm()
{}


gtBase *gtArrowButtonXm::container()
{
    return NULL;
}


void gtArrowButtonXm::set_direction(gtCardinalDir dir)
{
    int arrow_dir = -1;
    switch(dir)
    {
      case gtDirUp:	arrow_dir = XmARROW_UP;		break;
      case gtDirDown:	arrow_dir = XmARROW_DOWN;	break;
      case gtDirLeft:	arrow_dir = XmARROW_LEFT;	break;
      case gtDirRight:	arrow_dir = XmARROW_RIGHT;	break;
    }
    if(arrow_dir >= 0)
    {
	Arg args[1];
	XtSetArg(args[0], XmNarrowDirection, arrow_dir);
	XtSetValues(r->widget(), args, 1);
    }
}

 
void gtArrowButtonXm::set_callback(gtArrowCB callback, void* client_data)
{
    add_callback(XmNactivateCallback, this,
		 gtCallbackFunc(callback), client_data);
}
    
void gtArrowButtonXm::shadow_thickness(int thick)
{
    Arg	args[1];
    XtSetArg(args[0], XmNdefaultButtonShadowThickness, thick);
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtArrowButtonXm.h.C  $
   Revision 1.1 1993/06/02 16:18:27EDT builder 
   made from unix file
Revision 1.2.1.4  1993/06/02  20:18:08  glenn
Remove Initialize and Return macros.  Remove genError.h.

Revision 1.2.1.3  1993/01/26  04:32:20  glenn
Use gtCallbackMgrXm.

Revision 1.2.1.2  1992/10/09  19:03:23  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------

*/
