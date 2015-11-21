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
#include <symbolTreeHeader.h>

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <ctype.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <genError.h>

#include <RTL_externs.h>
#include <representation.h>
#include <objOper.h>

init_relational (symbolTreeHeader, RTL);

init_relation(symtree_of_ddheader,MANY,relationMode::DM,ddheader_of_symtree,1,NULL);

symbolTreeHeader::symbolTreeHeader (char* name) : RTL (name)
{
    Initialize (symbolTreeHeader::symbolTreeHeader);
}

void symbolTreeHeader::xref_notify (const symbolArr& modified,
                       const symbolArr& added, const symbolArr& deleted)
{
    Initialize(symbolTreeHeader::xref_notify);

    RTLNode* rtl_node = checked_cast (RTLNode, get_root ());

    symbolArr my_deleted(deleted); // Make a copy so we can modify it.
    symbolArr &cont = rtl_node->rtl_contents ();
    int touched = 0;

    for(int index = 0; index < cont.size(); ++index) {
	fsymbolPtr  xsym = cont[index].get_xrefSymbol();

	for(int m = 0; !touched  &&  m < modified.size(); ++m)
	    if(xsym == modified[m].get_xrefSymbol())
		touched = 1;

	for(int d = 0; d < my_deleted.size(); ++d)
	{
	    if(xsym == my_deleted[d].get_xrefSymbol())
	    {
		touched = 1;
		my_deleted.remove_index(d);
		cont.remove_index(index);
		index--;	// Avoid skipping next element of contents.
		break;
	    }
	}
    }

    if (touched)
	::obj_insert (this, REPLACE, rtl_node, rtl_node, 0);

    DBG {
        obj_prt_obj(this);
        msg("  mad\n") << eom;
        modified.print ();
        added.print ();
        deleted.print ();
    }
}

/*
   START-LOG-------------------------------------------

   $Log: symbolTreeHeader.h.cxx  $
   Revision 1.5 2002/03/04 17:25:39EST ktrans 
   Change M/C/D/W/S macros to enums
// Revision 1.5  1993/06/07  19:23:01  sergey
// Rewrote xref_notify(). Part of fix of bug #2174.
//
// Revision 1.4  1993/03/10  19:54:23  davea
// converted xrefSymbol to symbolPtr
//
// Revision 1.3  1993/02/01  18:09:37  wmm
// Fix bug 2347.
//
// Revision 1.2  1993/01/31  22:44:52  aharlap
// added one more break
//
// Revision 1.1  1993/01/03  20:44:40  aharlap
// Initial revision
//

   END-LOG---------------------------------------------

*/
