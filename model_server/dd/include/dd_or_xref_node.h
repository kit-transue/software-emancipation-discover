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
#ifndef _dd_or_xref_node_h
#define _dd_or_xref_node_h

// dd_or_xref_node.h
//------------------------------------------
// synopsis:
// 
// This class forms the app nodes for ERD, inheritance browser, and other
// views that are logically based on ddElements.  Objects of this class
// appear as children of a symbolTreeHeader object.  If the corresponding
// ddElement is loaded, it is connected to the dd_or_xref_node object by
// an elem_of_dd_or_xref_node relation.  The xrefSymbol pointer is always
// valid regardless of whether the ddElement is loaded.
//------------------------------------------

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _xref_h
#include <xref.h>
#endif


RelClass(ddElement);

class dd_or_xref_node: public appTree
{
  public:
    define_relational(dd_or_xref_node, appTree);
    copy_member(dd_or_xref_node);
    void send_string(ostream& = cout) const;
    char const *get_name() const;

    dd_or_xref_node(ddElementPtr);
    dd_or_xref_node(symbolPtr);
    dd_or_xref_node(const dd_or_xref_node&);

    ddElementPtr get_dd();	// loads if necessary
    ddElementPtr get_dd_if_loaded();
    symbolPtr get_xrefSymbol() const;
   
    void print( ostream &str = cout, int level = 0)  const;
    virtual int construct_symbolPtr(symbolPtr&) const;
    private:

    fsymbolPtr xref_ptr;
};
generate_descriptor(dd_or_xref_node,appTree);

defrel_one_to_many(ddElement,elem,dd_or_xref_node,dd_or_xref_node);


/*
   START-LOG-------------------------------------------

   $Log: dd_or_xref_node.h  $
   Revision 1.3 1996/03/22 13:58:27EST aharlap 
   added construct_symbolPtr
// Revision 1.5  1993/06/25  13:53:13  bakshi
// remove redundant dd_or_xref_node constructor (c++3.0.1 port)
//
// Revision 1.4  1993/05/10  19:53:40  glenn
// Prevent repeat inclusion of nested header files.
//
// Revision 1.3  1993/03/15  21:30:07  davea
// Change xrefSymbol* to fsymbolPtr
//
// Revision 1.2  1993/01/24  01:11:39  wmm
// Add get_name() member function.
//
// Revision 1.1  1993/01/22  22:45:07  wmm
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _dd_or_xref_node_h
