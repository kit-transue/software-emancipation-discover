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
// gtFormXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the Form class (gtFormXm)
//------------------------------------------

// INCLUDE FILES

#include <gtNoParent.h>
#include <gtBaseXm.h>
#include <gtFormXm.h>

#include <Xm/Form.h>


// FUNCTION DEFINITIONS

gtForm* gtForm::create(gtBase* parent, const char* name)
{
    return new gtFormXm(parent, name);
}

gtForm* gtForm::create(void *parent, const char* name)
{
    return new gtFormXm((Widget)parent, name);
}

gtFormXm::gtFormXm(gtBase* parent, const char* name)
{
    if(parent)
    {
	if(name == NULL)
	    name = "form";
	r->widget(XmCreateForm(parent->container()->rep()->widget(),
			       (char*)name, NULL, 0));
    }
    destroy_init();
}

gtFormXm::gtFormXm(Widget parent, const char* name)
{
    if(parent)
    {
	if(name == NULL)
	    name = "form";
	r->widget(XmCreateForm(parent,
			       (char*)name, NULL, 0));
    }
    destroy_init();
}


gtFormXm::~gtFormXm()
{}


void gtFormXm::help_callback(gtFormCB callback, void* client_data)
{
    add_callback(XmNhelpCallback, this,
		 gtCallbackFunc(callback), client_data);
}


/*
   START-LOG-------------------------------------------

   $Log: gtFormXm.h.C  $
   Revision 1.2 1995/03/27 18:07:08EST azaparov 
   Bug track: N/A
   Minibrowser stuff & fixes around
Revision 1.2.1.4  1993/06/02  20:28:52  glenn
Remove Initialize macros.  Remove genError.h.

Revision 1.2.1.3  1993/01/26  04:32:20  glenn
Add dtor.

Revision 1.2.1.2  1992/10/09  19:03:31  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
