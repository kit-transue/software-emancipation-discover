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
// ldrRTLNodePtr.h.C
//---------------------------------------------
// synopsis:
// Member functions for RTL ldr
//---------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <strstream>
using namespace std;
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include <objOper.h>
#include <RTL.h>
#include <ldrHeader.h>
#include <ldrHierarchy.h>
#include <ldrRTLDisplayType.h>
#include <ldrRTLHierarchy.h>
#include <ldrRTLNode.h>
#include <lde-RTL.h>
#include <ldrError.h>
#include <steTextNode.h>
#include <graResources.h>
#include <assoc.h>

init_relational(ldrRTLNode, ldrHierarchyNode);

void gen_print_indent(ostream&, int);

implement_copy_member(ldrRTLNode);

int ldrRTLNode::get_symbolPtr(symbolPtr*symp) const
{
   *symp = get_obj();
    return 1;
}

void ldrRTLNode::send_string(ostream& stream) const
{
    Initialize (ldrRTLNode::send_string);

    steTextNodePtr title_ptr = NULL;
    const char* format = NULL;

    if(int(obj))
    {
	ldrRTLHierarchyPtr ldr_header =
	    checked_cast(ldrRTLHierarchy, ldrRTLNodePtr(this)->get_header());
	if(ldr_header == NULL)
	    return;

	RTLPtr app_rtl = checked_cast(RTL, ldr_header->get_appHeader());
	RTLNodePtr app_rtl_node = checked_cast(RTLNode, app_rtl->get_root());

	if (ldr_header)
	{
	    if (format = app_rtl_node->get_format_spec())
	    {
		app_rtl_node->rtl_format_string(obj, (char*)format, stream);
		return;
	    }
	}

	symbolPtr sym = obj;
	RelationalPtr rel_obj = RelationalPtr(sym);

	if ( is_assocNode(rel_obj) )
	{
	    format = ui_get_value (ALPHASET_CLASS, RTL_ASSOC_FORMAT);
	    if (!format)
		format = "%os";
	    app_rtl_node->rtl_format_string(
		commonTreePtr(rel_obj), (char*)format, stream);
	    return;
	}
	else if ( is_steTextNode(rel_obj) )
	{
	    // Handle send_string for STE node differently.

	    if (title_ptr = steTextNodePtr(rel_obj)->get_title())
	    {
		title_ptr->send_string (stream);
		return;
	    }
	}

	switch (get_display_flag())
	{
	  default:
	    stream << "UNIMPLEMENTED (vestigial code)";
	    break;
	}
    }
}


void ldrRTLNode::print(ostream& stream, int level) const
{
    Initialize(ldrRTLNode::print);

    gen_print_indent(stream, level);
    send_string(stream);
}

int ldrRTLNode::set_old_name ()
{
    Initialize(ldrRTLNode::set_old_name);

    if(obj.isnull())
	return 0;

    if (obj.is_xrefSymbol())
	return 0;

    RelationalPtr rel_obj = RelationalPtr(obj);

    if (!rel_obj)
	return 0;

    switch (get_display_flag()) {
      case rtlDisplayUnknown:
	old_name = (char*)rel_obj->get_name();
	return 1;
	
      default:
	break;
    }
    return 0;
}

int ldrRTLNode::is_rename_occurred ()
{
    Initialize(ldrRTLNode::is_rename_occurred);
    if (((char *)old_name) == 0)
	return 1;

    genString old_old = old_name;
    set_old_name ();
    int res = 1;
    if (old_old == old_name)
	res = 0;

    return res;
}
/*
   START-LOG-------------------------------------------

   $Log: ldrRTLNode.cxx  $
   Revision 1.4 2000/07/10 23:06:03EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.14  1994/02/01  16:12:59  builder
Port

Revision 1.2.1.13  1993/06/10  13:46:00  aharlap
do not do unwanted loading

Revision 1.2.1.12  1993/06/09  22:25:57  boris
Fixed bug #3641 with viewerShell blinking

Revision 1.2.1.11  1993/03/18  20:24:39  aharlap
cleanup for paraset compatibility

Revision 1.2.1.10  1993/02/25  22:55:10  glenn
Call ui_get_value with const char* arguments.

Revision 1.2.1.9  1992/12/17  23:35:50  kws
Added format for ASSOC apps

Revision 1.2.1.8  1992/12/16  01:59:40  glenn
Fix problems with casting symbols to relationals.

Revision 1.2.1.7  1992/12/15  20:00:27  aharlap
changed for new xref

Revision 1.2.1.5  1992/12/09  21:15:49  aharlap
added interface for new xref

Revision 1.2.1.4  1992/11/23  22:48:58  glenn
Move variable declarations to point of initialization.
Cast away const-ness to suppress compiler warning when calling
get_header from send_string (a const function).
Remove superfluous uses of "this->".  Re-indent.
Remove default: case from switch, per coding conventions.
Remove Return macros.  Add Initialize macro to print member function.

Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:57  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
