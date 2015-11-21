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
#ifndef _subsys_h
#define _subsys_h

// subsys.h
//------------------------------------------
// synopsis:
// 
// Subsystem header and node class declarations
//------------------------------------------

#ifndef _RTL_h
#include <RTL.h>
#endif

#ifndef __symbolTreeHeader_h_
#include <symbolTreeHeader.h>
#endif

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif

class db_buffer;
class db_app;

RelClass(ddElement);
RelClass(ddNode);
RelClass(subsysHeader);

//------------------------------------------
// class subsysHeader: header for subsystem.  There is one header for
// each subsystem.
//------------------------------------------

class subsysHeader: public RTL {
public:
   define_relational(subsysHeader, RTL);
   copy_member(subsysHeader);

    subsysHeader(const char* name);        // use this one when creating a subsystem
    subsysHeader(const char* name, const char* ph_name);    // use this one when restoring one
   subsysHeader(const subsysHeader& other);// : RTL(other) { }

   void insert_obj(objInserter* oi, objInserter* ni);
   void remove_obj(objRemover *, objRemover* nr);
   void notify(int, RelationalPtr, objPropagator*, RelType*);

   int save(void* buffer, int maxlength);
   static RelationalPtr restore(void* rec, appPtr h, char* txt = 0);

   void print(ostream& = cout, int level = 0) const;

   void rename(const char* new_name);

   static subsysHeaderPtr main_header();
   static appPtr db_restore(char*title, byte language, char*);
   void db_after_restore();	// called after restore
   void add_contents(ddNodePtr);	// called after restore
   
   ~subsysHeader();
private:
   static subsysHeaderPtr main_hdr;
};
generate_descriptor(subsysHeader, RTL);

defrel_one_to_many(subsysHeader,subsys,symbolTreeHeader,symtree);

//------------------------------------------
// class subsysRoot: root for subsystem.  Each subsystem consists of
// a header and a root, with the objects contained in the subsystem
// indicated by dd_of_assocnode relations.
//------------------------------------------

RelClass(subsysRoot);

class subsysRoot: public RTLNode {
public:
   define_relational(subsysRoot, RTLNode);
   copy_member(subsysRoot);

   subsysRoot(subsysHeaderPtr);
   subsysRoot(const subsysRoot& other);//: RTLNode(other) { }

   int save(void* buffer, int maxlength);
   static RelationalPtr restore(void* rec, appPtr h, char* txt = 0);
   appPtr get_main_header();

   static void db_save(app*h, appTree*node, db_app&, db_buffer&);
   static appTree* db_restore(app*h, appTree *parent, db_app&, db_buffer&);

   void get_parents(objArr& parents) const;
   void get_children(objArr& children) const;
   void get_members(symbolArr& members) const;
   bool is_contained_in(subsysRootPtr) const;
   void add_subsys(subsysRootPtr);
   void remove_subsys(subsysRootPtr);
   void add_element(ddElementPtr);
   void add_element(fsymbolPtr);
   void add_elements(const symbolArr&);
   void add_initial_elements(const symbolArr&);
   void remove_element(ddElementPtr);
   void remove_element(fsymbolPtr);
   void remove_elements(const symbolArr&);
   void add_selected_items();

   static void find_containers_of(ddElementPtr, objArr& containers);
   static void find_containers_of(fsymbolPtr, objArr& containers);

   ~subsysRoot();
private:
   enum is_subsys_restore { RESTORE_SUBSYS };

   subsysRoot(subsysHeaderPtr, is_subsys_restore);  // for use by db_restore
   void add_symbol(symbolPtr, bool guaranteed_unique);
   void remove_symbol(symbolPtr);
};
generate_descriptor(subsysRoot, RTLNode);

/*
   START-LOG-------------------------------------------

   $Log: subsys.h  $
   Revision 1.6 2001/07/25 20:43:41EDT Sudha Kallem (sudha) 
   Changes to support model_server.
// Revision 1.15  1994/01/28  18:49:31  davea
// bug 6194
// when restoring existing subsystem, use known
// physical path, instead of requiring it to be in
// home or default project.
//
// Revision 1.14  1993/12/15  18:43:29  wmm
// Bug track: 5520
// Fix bug 5520.
//
// Revision 1.13  1993/12/08  21:37:03  wmm
// Bug track: 5043
// Fix bug 5043.
//
// Revision 1.12  1993/12/03  13:21:30  wmm
// Bug track: 5250
// Fix bug 5250: don't allow the user to begin subsystem extraction or dormant
// code analysis if the home project doesn't support subsystems (i.e., have a
// mapping for /ParaSET-Subsystems logical names).
//
// Revision 1.11  1993/05/25  18:29:11  wmm
// Support extraction of sub-subsystems from a subsystem.
//
// Revision 1.10  1993/05/17  17:33:02  wmm
// Performance tuning, etc., for subsystem extraction.
//
// Revision 1.9  1993/03/08  15:27:34  davea
// change xrefSymbol* to symbolPtr
//
// Revision 1.8  1993/03/08  14:26:50  aharlap
// added phys_name argument to hd_restore ()
//
// Revision 1.7  1993/02/21  16:20:25  mg
// void db_after_restore();   // called after restore
//      void add_contents(ddNode)
//
// Revision 1.6  1993/02/09  17:14:47  wmm
// Fix name conflict with oodt_relation.
//
// Revision 1.5  1993/02/08  23:57:10  wmm
// Enable restoring of subsystems (add subsysHeader::add_contents()
// to populate the RTL; add a private constructor for use by
// subsysRoot::db_restore()).
//
// Revision 1.4  1993/01/26  00:03:58  wmm
// Support XREF-based subsystem views
//
// Revision 1.3  1993/01/13  20:04:59  wmm
// Add "rename" entry.
//
// Revision 1.2  1993/01/11  00:15:02  aharlap
// added db
//
// Revision 1.1  1992/12/28  19:26:40  wmm
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _subsys_h

