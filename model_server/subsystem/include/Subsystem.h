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
// subsystem
//------------------------------------------
// synopsis:
// Class declaration for OODT subsystems
//
// description:
// Subsystems have containing and contained subsystems and "member" entries
// in the data dictionary.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#ifndef __subsystem
#define __subsystem

// include files

#include "objOper.h"
#include "ddict.h"
#include "subsys_header.h"
#include "steDocument.h"
#include "steTextNode.h"
#include "ddict.h"

#define SUBSYS_HEADER_NAME "ALL SUBSYSTEMS"

RelClass(Subsystem);

class Subsystem: public ddElement {
   friend class subsys_header;
public:
   define_relational(Subsystem, ddElement);
   copy_member(Subsystem);
   Subsystem(const char* name, appPtr smthdr = NULL);
   Subsystem(const Subsystem&);
   void print(ostream&, int) const;
   void send_string(ostream&) const;
   objTreePtr get_root() const;
   int save(void*, int);
   static Relational* restore(void*, app*, char* = 0);
   app* get_main_header();

   bool is_contained_in(SubsystemPtr) const;
   void add_to_subsys(SubsystemPtr, bool is_public);
   void add_subsys(SubsystemPtr, bool is_public);
   void add_to_subsys_nochk(SubsystemPtr, bool is_public);
   void add_subsys_nochk(SubsystemPtr, bool is_public);
   void add_element(HierarchicalPtr, bool is_public);
   void add_element(ddElementPtr, bool is_public);
   void add_selected_items(bool is_public);
   void make_public(RelationalPtr);
   void make_private(RelationalPtr);
private:
   Subsystem();
};

generate_descriptor(Subsystem, ddElement);

define_relation(pub_containing_subsys, pub_contained_subsys);
define_relation(pri_containing_subsys, pri_contained_subsys);
define_relation(pub_ddElement_of_subsys, pub_subsys_of_ddElement);
define_relation(pri_ddElement_of_subsys, pri_subsys_of_ddElement);

defrel_one_to_one(Subsystem,subsys,smtHeader,smthdr);
defrel_many_to_many(Subsystem,subsys,steTextNode,textnode);
defrel_many_to_many(Subsystem,subsys,steDocument,document);

#endif

/*
    START-LOG-------------------------------

    $Log: Subsystem.h  $
    Revision 1.1 1993/02/22 00:06:11EST builder 
    made from unix file
 * Revision 1.2.1.2  1992/10/09  19:56:23  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

