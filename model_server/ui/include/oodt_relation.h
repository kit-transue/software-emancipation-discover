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
#ifndef _oodt_relation_h
#define _oodt_relation_h

// oodt_relation
//------------------------------------------
// synopsis:
// Class declaration for OODT relations
//
// description:
// An OODT relation has a name, a cardinality, six strings specifying the
// declaration to be used in the source and target class, header file, and
// definition file, and a boolean specifying whether the source class is
// required to have a related target class.  (Revision 1.3 added a color
// resource name.)
//------------------------------------------

// include files

#ifndef _objOper_h
#include <objOper.h>
#endif
#ifndef __oodt_ui_decls_h_
#include <oodt_ui_decls.h>
#endif
#ifndef _symbolPtr_h_
#include <symbolPtr.h>
#endif

// incomplete class declarations

RelClass(oodt_relationHeader);
RelClass(oodt_relation);
RelClass(ddElement);
class db_buffer;
class db_app;
//class xrefSymbol;

// oodt_relationHeader

class oodt_relationHeader: public app {
public:
   define_relational(oodt_relationHeader, app);
   copy_member(oodt_relationHeader);
   oodt_relationHeader(const char* name);
   oodt_relationHeader(const oodt_relationHeader& other);
   void insert_obj(objInserter*, objInserter*);
   void remove_obj(objRemover*, objRemover*);
   static oodt_relationHeaderPtr db_restore(char* title, byte language, char*);
};
generate_descriptor(oodt_relationHeader, app);

// oodt_relation (root node)

class oodt_relation: public appTree {
public:
   virtual void print(ostream&, int) const;
   define_relational(oodt_relation, appTree);
   copy_member(oodt_relation);
   void send_string(ostream&) const;
   void set_name(char*);

   appPtr get_main_header();
   static void db_save(appPtr h, appTreePtr node, db_app&, db_buffer&);
   static oodt_relationPtr db_restore(appPtr h, appTreePtr parent,
         db_app&, db_buffer&);

   oodt_relation(const char* name, const char* inv_name, rel_cardinality,
         bool required, bool is_main, const char* src_member_decl,
         const char* src_header_decl, const char* src_definition,
         const char* trg_member_decl, const char* trg_header_decl,
         const char* trg_definition, const char* color = NULL,
	 oodt_relationHeaderPtr = NULL);
   oodt_relation(const oodt_relation&);
   ~oodt_relation();
   static void create_defaults();
   static oodt_relationPtr find_builtin(const char*);

   const char* inv_name() const;
   void inv_name(const char*);

   rel_cardinality cardinality() const;
   void cardinality(rel_cardinality);

   bool required() const;
   void required(bool);

   const char* src_member_decl() const;
   void src_member_decl(const char*);

   const char* src_header_decl() const;
   void src_header_decl(const char*);

   const char* src_definition() const;
   void src_definition(const char*);

   const char* trg_member_decl() const;
   void trg_member_decl(const char*);

   const char* trg_header_decl() const;
   void trg_header_decl(const char*);

   const char* trg_definition() const;
   void trg_definition(const char*);

   const char* color_resource() const;
   void color_resource(const char*);

   void remove_implementation(ddElementPtr);
   bool add_implementation(ddElementPtr, const char* trg_name);

   ddElementPtr get_src();
   fsymbolPtr get_src_xrefSymbol();
   void put_src(ddElementPtr);

   ddElementPtr get_trg();
   fsymbolPtr get_trg_xrefSymbol();
   void put_trg(ddElementPtr);

   bool main_rel() const;

// NOTE: the following DO NOT empty the array before adding the relations.
// This is to facilitate, for example, getting all the input and output
// relations of a particular class into one array.

   static void get_outgoing_relations_of(ddElementPtr, objArr&);
   static void get_outgoing_relations_of(fsymbolPtr, symbolArr&);
   static void get_incoming_relations_of(ddElementPtr, objArr&);
   static void get_incoming_relations_of(fsymbolPtr, symbolArr&);

   static oodt_relationPtr find(const char*);
   
   void files_needed( symbolArr&, symbolPtr&old_src_sym, symbolPtr&old_trg_sym, ddElementPtr new_src, ddElementPtr new_trg);

private:
   char* inv_nm;
   rel_cardinality card;
   bool req;
   bool is_main_rel;
   char* smbr_dcl;
   char* shdr_dcl;
   char* sdef;
   char* tmbr_dcl;
   char* thdr_dcl;
   char* tdef;
   const char* clr;
   static objArr builtins;

   enum is_rel_restore { RESTORE_REL };
   oodt_relation(const char* name, const char* inv_name, rel_cardinality,
         bool required, bool main_rel, const char* src_member_decl,
         const char* src_header_decl, const char* src_definition,
         const char* trg_member_decl, const char* trg_header_decl,
         const char* trg_definition, const char* color,
	 oodt_relationHeaderPtr, is_rel_restore);  // for use by db_restore
	       
   void add_file_sym(symbolArr& files);
   void add_impl_files(symbolArr&, ddElementPtr new_dd);
};

generate_descriptor(oodt_relation, appTree);

/*
    START-LOG-------------------------------

   $Log: oodt_relation.h  $
   Revision 1.4 1995/10/26 21:42:29EDT wmm 
   Bug track: 10077,11055
   Also fix unscriptable bugs 10381 (situation cannot arise after these changes), 11009 (requires WM actions, not available in scripts), and
   10078 (can't script to check for nonexistence of UDR connector).
 * Revision 1.2.1.10  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.9  1993/04/01  16:49:15  wmm
 * Fix bug 3074 (mark one rel as "main" and always present "main"
 * for editing, regardless of which is selected).
 *
 * Revision 1.2.1.8  1993/03/11  14:45:16  davea
 * Replace xrefSymbol* type with symbolPtr
 *
 * Revision 1.2.1.7  1993/03/08  14:29:00  aharlap
 * added phys_name argument to hd_restore ()
 *
 * Revision 1.2.1.6  1993/02/09  17:18:21  wmm
 * Fix name conflict with subsystems.
 *
 * Revision 1.2.1.5  1993/02/09  14:30:04  wmm
 * Enable restoring of oodt_relations (add private constructor for
 * use by db_restore).
 *
 * Revision 1.2.1.4  1993/01/22  22:54:15  wmm
 * Support XREF-based ERDs and DCs.
 *
 * Revision 1.2.1.3  1993/01/20  00:18:56  wmm
 * Support new xref-based implementation of oodt_relations.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:06  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif
