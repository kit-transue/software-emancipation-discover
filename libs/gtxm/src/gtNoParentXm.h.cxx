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
// gtNoParentXm.h.C
//------------------------------------------

// INCLUDE FILES

#include <Xm/Xm.h>
#include <gtAppShellXm.h>
#include <gtNoParent.h>
#include <gtBaseXm.h>


// VARIABLE DEFINITIONS

static gtNoParent* no_parent_app_shell = NULL;


// FUNCTION DEFINITIONS

gtNoParent* gtNoParent::create()
{
    return new gtNoParent;
}


gtNoParent* gtNoParent::create(void* w)
{
    return new gtNoParent(w);
}


gtNoParent::gtNoParent()
{
    delete r;
    r = NULL;
}


gtNoParent::gtNoParent(void* w)
{
    r->widget(Widget(w));
    attach_rep(r);
}


gtNoParent* gtNoParent::app_shell()
{
    if(!no_parent_app_shell)
    {
	no_parent_app_shell = new gtNoParent;
	no_parent_app_shell->attach_rep(
	    new gtRep(gtApplicationShellXm::find_existing()));
    } 
    return no_parent_app_shell;
}


/*
   START-LOG-------------------------------------------

   $Log: gtNoParentXm.h.C  $
   Revision 1.1 1993/01/25 23:40:56EST builder 
   made from unix file
Revision 1.2.1.3  1993/01/26  04:32:20  glenn
Add create/ctor that accepts void* widget.
Either delete or attach rep in ctor.

   END-LOG---------------------------------------------
*/
