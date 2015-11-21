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
// lde-browser-hierarchy.C
//------------------------------------------
// synopsis:
// function to extract a class browser ldr tree from the data dictionary
// and/or XREF
//
// description:
//
// Walks the DD/XREF structure from the specified node, creating ldr symbol
// nodes and connectors as needed.
//
//------------------------------------------
// Restrictions:
// Does not currently handle bases of virtual bases correctly.
//------------------------------------------

// include files
#include <cLibraryFunctions.h>
#include <msg.h>
#include <math.h>
#include "ldrTypes.h"
#include "ldrNode.h"
#include "lde-browser-hierarchy.h"
#include "ddict.h"
#include "oodt_class_member_sorter.h"
#include "oodt_relation.h"

#ifndef _groupHdr_h
#include <groupHdr.h>
#endif

#ifndef _groupTree_h
#include <groupTree.h>
#endif

#include <objCommon.h>
#include <xref.h>
#include <dd_or_xref_node.h>
#include <RTL.h>
#include <cmd.h>
#include <messages.h>

#ifndef __symbolTreeHeader_h_
#include <symbolTreeHeader.h>
#endif

enum context_type {
      TOP_LEVEL_CLASS,
      SUPERCLASS,
      SUBCLASS,
      RELATED_CLASS,
      CONTAINING_CLASS,
      CONTAINED_CLASS
};

static RTLNodePtr symbol_root;

//------------------------------------------
// new_node(fsymbolPtr)
//------------------------------------------

static dd_or_xref_nodePtr new_node(symbolPtr sym, int do_look = 0) {
   Initialize(new_node);

   dd_or_xref_nodePtr node = db_new(dd_or_xref_node, (sym));
   symbol_root->put_first(node);
   symbol_root->rtl_insert(sym, do_look);
   return node;
}

//------------------------------------------
// new_node(ddElementPtr)
//------------------------------------------

static dd_or_xref_nodePtr new_node(ddElementPtr elem, int do_look = 0) {
   Initialize(new_node);

   dd_or_xref_nodePtr node = db_new(dd_or_xref_node, (elem));
   symbol_root->put_first(node);
   fsymbolPtr  sym = elem->get_xrefSymbol();
   if (sym->xrisnotnull())
      symbol_root->rtl_insert(sym, do_look);
   return node;
}

/* The "min" and "max" in the following functions appear to be reversed,
 * but that is because larger enum values afford less access -- that is,
 * minimum access means maximum ordinal value and vice versa.
 */

//------------------------------------------
// min_access(...)
//------------------------------------------

inline member_access_sort_types min_access(member_access_sort_types a,
      member_access_sort_types b) {
   return (a > b) ? a : b;
}

//------------------------------------------
// max_access(...)
//------------------------------------------

inline member_access_sort_types max_access(member_access_sort_types a,
      member_access_sort_types b) {
   return (a < b) ? a : b;
}

//------------------------------------------
// extract_member(...)
//------------------------------------------

static void extract_member(fsymbolPtr  member_xref, ddElement* dd_member, 
      const char* qual, member_access_sort_types containing_access, 
      ldrClassNodePtr the_class, ldrMemberNodePtr& last, 
      ldrConnectionNodePtr& conn, objArr* added_members) {
   Initialize(extract_member);

   ldrMemberNodePtr member = NULL;

   if (member_xref->xrisnotnull() && 
       (dd_member == 0) && member_xref.is_loaded())
     dd_member = member_xref->get_def_dd();
   member_access_sort_types local_access;
   member_access_sort_types resolved_access;
   if (dd_member) {
      if (dd_member->is_private())
	 local_access = PRIVATE_MEMBERS;
      else if (dd_member->is_package_prot())
	 local_access = PACKAGE_PROT_MEMBERS;
      else if (dd_member->is_protected())
	 local_access = PROTECTED_MEMBERS;
      else local_access = PUBLIC_MEMBERS;                           
      resolved_access = min_access(containing_access, local_access);
      ldrSymbolNodePtr last_parm = NULL;
      ddElementPtr dd_parm;

      if (dd_is_var_declaration(dd_member->get_kind()))
      {
	 member = db_new(ldrMemberNode, (ldrPUBLIC_MEMBER, resolved_access,
	       new_node(dd_member), qual));
	 member->set_symbol_type(ldrSymbolType(ldrPUBLIC_MEMBER + resolved_access));
      }
      else if (dd_is_function_declaration(dd_member->get_kind()))
      {
	 member = db_new(ldrMemberNode, (ldrPUBLIC_METHOD, resolved_access,
	       new_node(dd_member), qual));
	 member->set_symbol_type(ldrSymbolType(ldrPUBLIC_METHOD + resolved_access));
	 for (dd_parm = checked_cast(ddElement, dd_member->get_first());
               dd_parm; dd_parm = checked_cast(ddElement,
               dd_parm->get_next())) {
	    char buff[1024];
	    buff[0] = 0;
	    char* tmp_def = dd_parm->get_def();
	    if (tmp_def)
	       strncat(buff, tmp_def, 1022);
	    strcat(buff, " ");
	    ldrMemberNodePtr parm = db_new(ldrMemberNode,
		  (ldrPUBLIC_METHOD_PARAMETER, resolved_access,
                  new_node(dd_parm), buff));
	    parm->set_symbol_type(ldrSymbolType(ldrPUBLIC_METHOD_PARAMETER + resolved_access));
	    if (last_parm)
	       last_parm->put_after(parm);
	    else member->put_first(parm);
	    last_parm = parm;
	 }
      }
      else if (dd_is_nested_type(dd_member->get_kind()))
      {
	 member = db_new(ldrMemberNode, (ldrPUBLIC_NESTED_TYPE,
               resolved_access, new_node(dd_member), qual));
	 member->set_symbol_type(ldrSymbolType(ldrPUBLIC_NESTED_TYPE + resolved_access));
      }

   }
   else {
      if (member_xref->get_attribute(PRIV_ATT, 1))
	 local_access = PRIVATE_MEMBERS;
      else if (member_xref->get_attribute(PAKG_ATT, 1))
	 local_access = PACKAGE_PROT_MEMBERS;
      else if (member_xref->get_attribute(PROT_ATT, 1))
	 local_access = PROTECTED_MEMBERS;
      else local_access = PUBLIC_MEMBERS;                           
      resolved_access = min_access(containing_access, local_access);
      
      if (dd_is_function_declaration(member_xref.get_kind()))
      {
	 member = db_new(ldrMemberNode, (ldrPUBLIC_METHOD, resolved_access,
               new_node(member_xref), qual));
	 member->set_symbol_type(ldrSymbolType(ldrPUBLIC_METHOD + resolved_access));
//=============================================
// Need to handle arguments here, if we keep the old symbology for arguments.
//=============================================	 
      }
      else if (dd_is_var_declaration(member_xref.get_kind()))
      {
	 member = db_new(ldrMemberNode, (ldrPUBLIC_MEMBER, resolved_access,
               new_node(member_xref), qual));
	 member->set_symbol_type(ldrSymbolType(ldrPUBLIC_MEMBER + resolved_access));
      }
      else if (dd_is_nested_type(member_xref.get_kind()))
      {
	 member = db_new(ldrMemberNode, (ldrPUBLIC_NESTED_TYPE, resolved_access,
	       new_node(member_xref), qual));
	 member->set_symbol_type(ldrSymbolType(ldrPUBLIC_NESTED_TYPE + resolved_access));
      }      
   }

   if (member) {
      if (added_members) {
	 added_members->insert_last(member);
      }
      else {
	 if (last) {
	    last->put_after(member);
	 }
	 else the_class->put_first(member);
	 last = member;
      }
      if (!conn) {
	 conn = db_new(ldrConnectionNode, (ldrcMEMBER_LIST));
	 conn->add_input(the_class);
      }
      conn->add_output(member);
   }
}

struct vbase {
   vbase(vbase*& top, fsymbolPtr  cl, member_access_sort_types ac):
         next(top), class_xref(cl), access(ac) {
      top = this;
   }
   vbase* next;
   fsymbolPtr  class_xref;
   member_access_sort_types access;
};

//------------------------------------------
// get_bases(...)
//------------------------------------------

static symbolArr& get_bases(linkType lk, symbolPtr class_xref,
      symbolArr& bases) {
   Initialize(get_bases);

   bases.removeAll();
   class_xref->get_link(lk, bases);

   if (lk == has_pub_superclass && bases.size() == 0) {	// check for old pmod
      symbolArr all_bases;
      if (class_xref->get_link(has_superclass, all_bases)) {
	 symbolArr all_nonpub_or_virt_bases;
	 class_xref->get_link(has_pro_superclass, all_nonpub_or_virt_bases);
	 class_xref->get_link(has_pri_superclass, all_nonpub_or_virt_bases);
	 class_xref->get_link(has_vpub_superclass, all_nonpub_or_virt_bases);
	 class_xref->get_link(has_vpro_superclass, all_nonpub_or_virt_bases);
	 class_xref->get_link(has_vpri_superclass, all_nonpub_or_virt_bases);
	 if (all_nonpub_or_virt_bases.size() == 0) {	// is old pmod
	    bases = all_bases;
	 }
      }
   }

   bases.sort();
   return bases;
}

//------------------------------------------
// get_derived(...)
//------------------------------------------
static symbolArr& get_derived(linkType lk, symbolPtr class_xref,
      symbolArr& deriveds) {
   Initialize(get_derived);

   deriveds.removeAll();
   class_xref->get_link(lk, deriveds);

   if (lk == has_pub_subclass && deriveds.size() == 0) { // check for old pmod
      symbolArr all_deriveds;
      if (class_xref->get_link(has_subclass, all_deriveds)) {
	 symbolArr all_nonpub_or_virt_deriveds;
	 class_xref->get_link(has_pro_subclass, all_nonpub_or_virt_deriveds);
	 class_xref->get_link(has_pri_subclass, all_nonpub_or_virt_deriveds);
	 class_xref->get_link(has_vpub_subclass, all_nonpub_or_virt_deriveds);
	 class_xref->get_link(has_vpro_subclass, all_nonpub_or_virt_deriveds);
	 class_xref->get_link(has_vpri_subclass, all_nonpub_or_virt_deriveds);
	 if (all_nonpub_or_virt_deriveds.size() == 0) {	// is old pmod
	    deriveds = all_deriveds;
	 }
      }
   }

   deriveds.remove_dup_syms();
   deriveds.usort();
   return deriveds;
}

//------------------------------------------
// add_or_update_vbases(...)
//------------------------------------------

static void add_or_update_vbases(symbolArr base_classes,
      member_access_sort_types access, vbase*& vbases) {
   Initialize(add_or_update_vbases);

   symbolArr bases;
   symbolPtr curr;
   ForEachS(curr, base_classes) {
      curr = curr.get_xrefSymbol();
      add_or_update_vbases(get_bases(has_vpub_superclass,
            curr, bases),
            min_access(access, PUBLIC_MEMBERS), vbases);
      add_or_update_vbases(get_bases(has_vpro_superclass,
	    curr, bases),
            min_access(access, PROTECTED_MEMBERS), vbases);
      add_or_update_vbases(get_bases(has_vpri_superclass,
	    curr, bases),
            min_access(access, PRIVATE_MEMBERS), vbases);
      vbase *p;
      for (p = vbases; p && p->class_xref != curr;
            p = p->next) ;
      if (p)
	 p->access = max_access(access, p->access);
      else vbases = new vbase(vbases, curr, access);
   }

   return;
}
// prototype for function used only in this file
static void extract_members_of(fsymbolPtr, const char*, 
      member_access_sort_types, vbase*&, ldrClassNodePtr, 
      ldrMemberNodePtr&, ldrConnectionNodePtr&, objArr*);

//------------------------------------------
// extract_superclass_members(...)
//------------------------------------------

static void extract_superclass_members(symbolArr base_classes, const char* qual,
      member_access_sort_types containing_access, vbase*& virtual_bases,
      ldrConnectionNodePtr conn, objArr& added_members,
      bool extract_local_members) {
   Initialize(extract_superclass_members);

   ldrMemberNodePtr last = NULL;
   symbolPtr curr;
   ForEachS(curr, base_classes) {
      symbolPtr  base_xref = curr.get_xrefSymbol();
      char* new_qual;
      char* allocated_qual = NULL;
      smtLanguage smtLang = smtLanguage(base_xref.get_language());
      if (extract_local_members && is_C_or_CPLUSPLUS(smtLang)) {
	 char* class_name = base_xref.get_name();
	 allocated_qual = new char[strlen(qual) + strlen(class_name) + 3];
	 new_qual = allocated_qual;
	 strcpy(new_qual, qual);                                        
	 strcat(new_qual, class_name);
	 strcat(new_qual, "::");
      }
      else new_qual = "";
      
      symbolArr bases;
      extract_superclass_members(get_bases(has_pub_superclass, base_xref, bases),
	    new_qual, min_access(PUBLIC_MEMBERS, containing_access),
	    virtual_bases, conn, added_members, true);
      extract_superclass_members(get_bases(has_pro_superclass, base_xref, bases),
	    new_qual, min_access(PROTECTED_MEMBERS, containing_access),
	    virtual_bases, conn, added_members, true);
      extract_superclass_members(get_bases(has_pri_superclass, base_xref, bases),
	    new_qual, min_access(PRIVATE_MEMBERS, containing_access),
	    virtual_bases, conn, added_members, true);
      add_or_update_vbases(get_bases(has_vpub_superclass, base_xref, bases),
	    min_access(PUBLIC_MEMBERS, containing_access), virtual_bases);
      add_or_update_vbases(get_bases(has_vpro_superclass, base_xref, bases),
	    min_access(PROTECTED_MEMBERS, containing_access), virtual_bases);
      add_or_update_vbases(get_bases(has_vpri_superclass, base_xref, bases),
	    min_access(PRIVATE_MEMBERS, containing_access), virtual_bases);

      if (extract_local_members) {
	 extract_members_of(base_xref, new_qual, containing_access,
               virtual_bases, NULL, last, conn, &added_members);
         delete allocated_qual;
      }
   }                    

   if (strlen(qual) == 0) {	// original level -- process virtuals
      while(virtual_bases) {
	 const char* base_name = virtual_bases->class_xref.get_name();
	 char* vqual = new char[strlen(base_name) + 3];
	 strcpy(vqual, base_name);
	 strcat(vqual, "::");
	 if (virtual_bases->class_xref.is_loaded()) {
	    ddElementPtr dd_base = virtual_bases->class_xref->get_def_dd();
            objArr ddArr;
            Obj   *ob;

            dd_base->class_get_members(ddArr);

            ForEach(ob, ddArr) {
               ddElementPtr dd_member;
	       dd_member = checked_cast(ddElement, ob);

	       extract_member(NULL_symbolPtr, dd_member, vqual,
                     virtual_bases->access, NULL, last, conn,
		     &added_members);
            }
	 }
	 else {
	    symbolArr members;
	    if (virtual_bases->class_xref->get_link(is_using, members)) {
	       symbolPtr mem;
	       ForEachS(mem, members) {
		  extract_member(mem.get_xrefSymbol(), 0, vqual,
                        virtual_bases->access, NULL, last, conn,
			&added_members);
	       }
	    }
	 }
	 delete vqual;
	 vbase* next = virtual_bases->next;
	 delete virtual_bases;
	 virtual_bases = next;
      }
   }
}

//------------------------------------------
// extract_members_of(...)
//------------------------------------------

static void extract_members_of(symbolPtr class_xref, const char* qual,
      member_access_sort_types containing_access, 
      vbase*& /* virtual_bases */,
      ldrClassNodePtr target_class, ldrMemberNodePtr& last,
      ldrConnectionNodePtr& conn, objArr* added_members) {
   Initialize(extract_members_of);

   ddElementPtr dd_class = NULL;
   if (class_xref.is_loaded()) {
      dd_class = class_xref->get_def_dd();
      if (dd_class == NULL) {
	 msg("ERROR: Internal error: cannot get definition DD from XREF symbol $1") << class_xref.get_name() << eom;
      }
   }
   if (dd_class) {
      objArr members;
      Obj* member;

      dd_class->class_get_members(members);
      ForEach(member, members) {
	 ddElementPtr dd_member = checked_cast(ddElement, member);
	 if (!dd_member->is_comp_gen()) {
	    extract_member(NULL_symbolPtr, dd_member, qual, containing_access,
			target_class, last, conn, added_members);
	 }
      }
   }
   else {
      symbolArr members;
      if (class_xref->get_link(is_using, members)) {
	 symbolPtr mem;
	 char *class_name = class_xref.get_name();
	 ForEachS(mem, members) {
	   char * mem_name = mem.get_name();
	   if (class_name && mem_name && (strstr(mem_name, class_name) == mem_name)) {
	      if (!mem->get_attribute(COMP_GEN_ATT, 1)) {
		 extract_member(mem.get_xrefSymbol(), 0, qual, containing_access,
				target_class, last, conn, added_members);
	      }
	   }
	 }
      }
   }
}

typedef void (ldrClassNode::*ldrClassNode_fcn)(ldrNodePtr);

static ldrClassNodePtr extract_class(fsymbolPtr , member_access_sort_types,
      int, bool);

//------------------------------------------
// process_inheritance(...)
//------------------------------------------

static void process_inheritance(ldrClassNodePtr this_class,
      symbolArr classes, ldrConnectionNodePtr& connector,
      member_access_sort_types containing_access, int depth, bool is_virtual,
      ldrConnectorType connector_type, ldrClassNode_fcn add) {
   Initialize(process_inheritance);

   symbolPtr class_sym;
   ForEachS(class_sym, classes) {
      ldrClassNodePtr ldr_class = extract_class(class_sym.get_xrefSymbol(),
            containing_access, depth, is_virtual);
      if (ldr_class) {
	 if (!connector) {
	    connector = db_new(ldrConnectionNode, (connector_type));
	    connector->add_input(this_class);
	 }
	 (this_class->*add)(ldr_class);
	 connector->add_output(ldr_class);
      }
   }

   Return;
}

//------------------------------------------
// extract_relations(...)
//------------------------------------------

static void extract_relations(ldrClassNodePtr this_class, objArr relations,
      ldrRole role, objArr& nodes) {
   Initialize(extract_relations);

   ldrConnectorType connector1_type;
   ldrConnectorType connector2_type;
   auto fsymbolPtr  (oodt_relation::* get)();
   ldrClassNode_fcn add;
   if (role == ldrOUTGOING_RELATION) {
      connector1_type = ldrcREL_LIST_OUT;
      connector2_type = ldrcREL_OUT_1_OPT;
      get = &oodt_relation::get_trg_xrefSymbol;
      add = &ldrClassNode::add_outgoing_relation;
   }
   else {
      connector1_type = ldrcREL_LIST_IN;
      connector2_type = ldrcREL_IN_1_OPT;
      get = &oodt_relation::get_src_xrefSymbol;
      add = &ldrClassNode::add_incoming_relation;
   }

   ldrConnectionNodePtr connector1 = NULL;
   Obj* curr;
   ForEach(curr, relations) {
      RelationalPtr rp = RelationalPtr(curr);
      oodt_relationPtr curr_relation = NULL;
      if (is_oodt_relation(rp)) {
	 curr_relation = oodt_relationPtr(rp);
      }
      else if (is_ddElement(rp)) {
	 curr_relation = checked_cast(oodt_relation,
				      dd_get_def_assocnode(ddElementPtr(rp)));
      }
      Assert(curr_relation);
      int modifier = 2 * (curr_relation->required() != 0);
      if (role == ldrOUTGOING_RELATION) {
	 if (curr_relation->cardinality() == rel_one_to_many ||
               curr_relation->cardinality() == rel_many_to_many)
            modifier += 1;
      }
      else {
	 if (curr_relation->cardinality() == rel_many_to_one ||
	       curr_relation->cardinality() == rel_many_to_many)
	    modifier += 1;
      }
      ldrSymbolNodePtr ldr_rel = db_new(ldrRelationNode, (ldrRELATION));
      nodes.insert_last (ldr_rel);
      ldrtree_put_apptree(ldr_rel, curr_relation);
      ldr_rel->set_role(role);
      if (!connector1) {
	 connector1 = db_new(ldrConnectionNode, (connector1_type));
	 connector1->add_input(this_class);
      }
      connector1->add_output(ldr_rel);
      (this_class->*add)(ldr_rel);
      ldrClassNodePtr targ_class = extract_class((curr_relation->*get)(),
            PUBLIC_MEMBERS, 1, false);
      ldrConnectionNodePtr connector2 = db_new(ldrConnectionNode,
            (ldrConnectorType(connector2_type + modifier)));
      connector2->add_input(ldr_rel);
      connector2->add_output(targ_class);
      ldr_rel->put_first(targ_class);
      targ_class->put_after(connector2);
   }
   if (connector1)
      (this_class->*add)(connector1);

   return;
}

static context_type class_context = TOP_LEVEL_CLASS;

//------------------------------------------
// extract_class(...)
//------------------------------------------

static ldrClassNodePtr extract_class(fsymbolPtr  class_xref,
      member_access_sort_types containing_access, int depth, bool is_virtual) {
   Initialize(extract_class);

   context_type my_context = class_context;

   ldrClassNodePtr this_class;
   if (is_virtual)
      this_class = db_new(ldrClassNode,
	    ((ldrSymbolType)(ldrPUBLIC_VIRTUAL_CLASS +
	    containing_access)));
   else this_class = db_new(ldrClassNode,
	 ((ldrSymbolType)(ldrPUBLIC_CLASS +
	 containing_access)));
   this_class->set_graphlevel(depth);

   if (my_context != SUPERCLASS && my_context != RELATED_CLASS) {
      class_context = SUBCLASS;
      ldrConnectionNodePtr subclass_connection = NULL;
      symbolArr deriveds;
      symbolArr subclasses;
      process_inheritance(this_class, get_derived(has_pub_subclass,
	    class_xref, deriveds), subclass_connection,
	    PUBLIC_MEMBERS, depth + 1, false, ldrcSUBCLASS_LIST,
	    &ldrClassNode::add_subclass);
      process_inheritance(this_class, get_derived(has_pro_subclass,
	    class_xref, deriveds), subclass_connection,
	    PROTECTED_MEMBERS, depth + 1, false, ldrcSUBCLASS_LIST,
	    &ldrClassNode::add_subclass);
      process_inheritance(this_class, get_derived(has_pri_subclass,
	    class_xref, deriveds), subclass_connection,
	    PRIVATE_MEMBERS, depth + 1, false, ldrcSUBCLASS_LIST,
	    &ldrClassNode::add_subclass);
      process_inheritance(this_class, get_derived(has_vpub_subclass,
	    class_xref, deriveds), subclass_connection,
	    PUBLIC_MEMBERS, depth + 1, true, ldrcSUBCLASS_LIST,
	    &ldrClassNode::add_subclass);
      process_inheritance(this_class, get_derived(has_vpro_subclass,
	    class_xref, deriveds), subclass_connection,
	    PROTECTED_MEMBERS, depth + 1, true, ldrcSUBCLASS_LIST,
	    &ldrClassNode::add_subclass);
      process_inheritance(this_class, get_derived(has_vpri_subclass,
	    class_xref, deriveds), subclass_connection,
	    PRIVATE_MEMBERS, depth + 1, true, ldrcSUBCLASS_LIST,
	    &ldrClassNode::add_subclass);
      if (subclass_connection)
	 this_class->add_subclass(subclass_connection);
   }

   if (my_context != SUBCLASS && my_context != RELATED_CLASS) {
      class_context = SUPERCLASS;
      ldrConnectionNodePtr superclass_connection = NULL;
      symbolArr bases;
      process_inheritance(this_class, get_bases(has_pub_superclass,
	    class_xref, bases), superclass_connection, PUBLIC_MEMBERS,
	    depth + 1, false, ldrcSUPERCLASS_LIST,
	    &ldrClassNode::add_superclass);
      process_inheritance(this_class, get_bases(has_pro_superclass,
	    class_xref, bases), superclass_connection, PROTECTED_MEMBERS,
	    depth + 1, false, ldrcSUPERCLASS_LIST,
	    &ldrClassNode::add_superclass);
      process_inheritance(this_class, get_bases(has_pri_superclass,
	    class_xref, bases), superclass_connection, PRIVATE_MEMBERS,
	    depth + 1, false, ldrcSUPERCLASS_LIST,
	    &ldrClassNode::add_superclass);
      process_inheritance(this_class, get_bases(has_vpub_superclass,
	    class_xref, bases), superclass_connection, PUBLIC_MEMBERS,
	    depth + 1, true, ldrcSUPERCLASS_LIST,
	    &ldrClassNode::add_superclass);
      process_inheritance(this_class, get_bases(has_vpro_superclass,
	    class_xref, bases), superclass_connection, PROTECTED_MEMBERS,
	    depth + 1, true, ldrcSUPERCLASS_LIST,
	    &ldrClassNode::add_superclass);
      process_inheritance(this_class, get_bases(has_vpri_superclass,
	    class_xref, bases), superclass_connection, PRIVATE_MEMBERS,
	    depth + 1, true, ldrcSUPERCLASS_LIST,
	    &ldrClassNode::add_superclass);
      if (superclass_connection)
	 this_class->add_superclass(superclass_connection);
   }

   vbase* virtual_bases = NULL;
   ldrMemberNodePtr last = NULL;
   ldrConnectionNodePtr conn = NULL;
   extract_members_of(class_xref, "", PUBLIC_MEMBERS, virtual_bases,
	 this_class, last, conn, NULL);
   if (conn)
      last->put_after(conn);

   class_context = my_context;
   if (this_class) {
      this_class->set_appTree(new_node(class_xref));
   }
   return this_class;
}

//------------------------------------------
// add_to_subsys(...)
//------------------------------------------

static void add_to_subsys(ldrMemberNodePtr ldr_mem, ldrClassNodePtr subsys,
      ldrMemberNodePtr& last, ldrConnectionNodePtr& conn) {
   Initialize(add_to_subsys);

   if (last)
      last->put_after(ldr_mem);
   else subsys->put_first(ldr_mem);
   last = ldr_mem;
   if (!conn) {
      conn = db_new(ldrConnectionNode, (ldrcMEMBER_LIST));
      conn->add_input(subsys);
   }
   conn->add_output(ldr_mem);

   return;
}

//------------------------------------------
// extract_subsys_member(...)
//------------------------------------------

static void extract_subsys_member(symbolPtr mem_xref, member_access_sort_types prot,
				  ldrClassNodePtr the_class, ldrMemberNodePtr& last,
				  ldrConnectionNodePtr& conn) {
   Initialize(extract_subsys_member);

   ldrMemberNodePtr ldr_mem = NULL;
   switch(mem_xref.get_kind()) {
   case DD_TYPEDEF:
      ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_TYPEDEF :
				       ldrPRIVATE_SUBSYS_TYPEDEF, prot, new_node(mem_xref),
				       "", false));
      break;

   case DD_MACRO:
      ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_MACRO :
				       ldrPRIVATE_SUBSYS_MACRO, prot, new_node(mem_xref),
				       "", false));
      break;

   case DD_VAR_DECL:
      ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_VAR :
				       ldrPRIVATE_SUBSYS_VAR, prot, new_node(mem_xref),
				       "", false));
      break;

   case DD_FUNC_DECL:
      ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_FUNC :
				       ldrPRIVATE_SUBSYS_FUNC, prot, new_node(mem_xref),
				       "", false));
      break;

   case DD_ENUM:
      ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_ENUM :
				       ldrPRIVATE_SUBSYS_ENUM, prot, new_node(mem_xref),
				       "", false));
      break;

   case DD_CLASS:
      ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_CLASS :
				       ldrPRIVATE_SUBSYS_CLASS, prot, new_node(mem_xref),
				       "", false));
      break;

   case DD_UNION:
      ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_UNION :
				       ldrPRIVATE_SUBSYS_UNION, prot, new_node(mem_xref),
				       "", false));
      break;

   case DD_MODULE:
      switch (mem_xref.get_language_local()) {
      case FILE_LANGUAGE_UNKNOWN:
      default:
	 ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_FILE_UNKNOWN :
					  ldrPRIVATE_SUBSYS_FILE_UNKNOWN, prot, new_node(mem_xref),
					  "", false));
	 break;

// The following cases may need to change to split out the ESQL versions separately, but for now
// we will just treat them like the non-ESQL variant.  -- wmm, 970430

      case FILE_LANGUAGE_CPP:
      case FILE_LANGUAGE_ESQL_CPP:
	 ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_FILE_CPP :
					  ldrPRIVATE_SUBSYS_FILE_CPP, prot, new_node(mem_xref),
					  "", false));
	 break;

      case FILE_LANGUAGE_C:
      case FILE_LANGUAGE_ESQL_C:
	 ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_FILE_C :
					  ldrPRIVATE_SUBSYS_FILE_C, prot, new_node(mem_xref),
					  "", false));
	 break;

      case FILE_LANGUAGE_EXT:
	 ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_FILE_FRAME :
					  ldrPRIVATE_SUBSYS_FILE_FRAME, prot, new_node(mem_xref),
					  "", false));
	 break;

      case FILE_LANGUAGE_MAKEFILE:
	 ldr_mem = db_new(ldrMemberNode, ((prot == PUBLIC_MEMBERS) ? ldrPUBLIC_SUBSYS_FILE_MAKEFILE :
					  ldrPRIVATE_SUBSYS_FILE_MAKEFILE, prot, new_node(mem_xref),
					  "", false));
	 break;

      }
      break;

   default:
      break;
   }

   if (ldr_mem) {
      if (last) {
	 last->put_after(ldr_mem);
      }
      else the_class->put_first(ldr_mem);
      last = ldr_mem;
      if (!conn) {
	 conn = db_new(ldrConnectionNode, (ldrcMEMBER_LIST));
	 conn->add_input(the_class);
      }
      conn->add_output(ldr_mem);
   }
}


//------------------------------------------
// lde_extract_browser_hierarchy(symbolPtr, RTLNodePtr)
//------------------------------------------

ldrNodePtr lde_extract_browser_hierarchy(symbolPtr tp, RTLNodePtr sym_root) {
   Initialize(lde_extract_browser_hierarchy);

   ldrNodePtr root = new ldrNode;   // ldr trees expect empty topmost node

   context_type saved_class_context = class_context;
   RTLNodePtr saved_symbol_root = symbol_root;
   class_context = TOP_LEVEL_CLASS;
   symbol_root = sym_root;
   ldrClassNodePtr clp = 0;
   ldrSymbolType top_level_type;
   fsymbolPtr  tp_xref = tp.get_xrefSymbol();
   clp = extract_class(tp_xref, PUBLIC_MEMBERS, 0, false);
   top_level_type = ldrCURRENT_CLASS;
   if (clp) {
      root->put_first(clp);
      clp->set_symbol_type(top_level_type);
      root->set_appTree(clp->get_appTree());
   }

   class_context = saved_class_context;
   symbol_root = saved_symbol_root;
   ReturnValue(root);

}

//--------------------------------
// lde_browser_add_relations (...)
//--------------------------------

void lde_browser_add_relations (ldrClassNodePtr the_class, objArr& added)
{
    Initialize (lde_browser_add_relations);

    if (!the_class || the_class->has_relations ())
	return;

    context_type	ctxt = class_context;
    RTLNodePtr		symroot = symbol_root;
    dd_or_xref_nodePtr	nodep;
    symbolPtr		class_xref;
    symbolArr		irels, orels;
    ldrPtr		ah;
    appPtr		sh;

    appTreePtr apptr = the_class->get_appTree();
    nodep = (apptr && is_dd_or_xref_node(apptr)) ?
	  dd_or_xref_nodePtr(apptr) : NULL;
    if (nodep == NULL)
	return;

    class_xref = nodep->get_xrefSymbol ();
    if (class_xref.xrisnull ())
	return;

    ah = the_class->get_header ();
    if (ah == NULL)
	return;

    sh = checked_cast (app, get_relation (app_of_ldr, ah));
    if ((sh == NULL) || !is_symbolTreeHeader(sh))
	return;

    class_context	= RELATED_CLASS;
    symbol_root		= checked_cast (RTLNode, sh->get_root ());

    oodt_relation::get_outgoing_relations_of (class_xref, orels);
    extract_relations (the_class, orels, ldrOUTGOING_RELATION, added);

    oodt_relation::get_incoming_relations_of (class_xref, irels);
    extract_relations (the_class, irels, ldrINCOMING_RELATION, added);

    the_class->set_relations ();

    class_context	= ctxt;
    symbol_root		= symroot;
}

//------------------------------------------
// lde_browser_add_inherited_members(...)
//------------------------------------------

void lde_browser_add_inherited_members(ldrClassNodePtr the_class,
				       objArr& added_members) {
   Initialize(lde_browser_add_inherited_members);

   if (!the_class || the_class->has_inherited_members()) {
      return;		// nothing to do
   }
   ldr* ah = the_class->get_header();
   if (ah && (symbol_root == 0)) {
     app* sh = checked_cast(app, get_relation(app_of_ldr, ah));
     if (sh && is_symbolTreeHeader(sh))
       symbol_root = checked_cast(RTLNode, sh->get_root());
   }
   dd_or_xref_nodePtr nodep = checked_cast(dd_or_xref_node,
					   the_class->get_appTree());
   if (nodep) {
      symbolPtr class_xref = nodep->get_xrefSymbol();
      if (class_xref.xrisnotnull()) {
	 symbolArr this_class;
	 this_class.insert_last(class_xref);
	 vbase* vbases = NULL;
	 ldrConnectionNodePtr conn = NULL;
	 bool add_connector = true;
	 objTreePtr last_member = the_class->get_last();
	 if (last_member && is_ldrConnectionNode(last_member)) {
	    conn = ldrConnectionNodePtr(last_member);
	    add_connector = false;
	 }
	 extract_superclass_members(this_class, "", PUBLIC_MEMBERS, vbases,
	       conn, added_members, false);
	 Obj* added_member;
	 ForEach(added_member, added_members) {
	    ldrMemberNodePtr inh_mem = checked_cast(ldrMemberNode,
						    added_member);
	    const char* inh_name = inh_mem->get_member_name();
	    size_t inh_name_len = strlen(inh_name);
	    for (ldrNodePtr nodep = checked_cast(ldrNode, the_class->get_first());
		 nodep; nodep = checked_cast(ldrNode, nodep->get_next())) {
	       if (!is_ldrMemberNode(nodep)) {
		  continue;
	       }
	       ldrMemberNodePtr loc_mem = ldrMemberNodePtr(nodep);
	       const char* loc_name = loc_mem->get_member_name();
	       size_t loc_name_len = strlen(loc_name);
	       if (inh_name_len > loc_name_len &&
		   inh_name[inh_name_len - loc_name_len - 1] == ':' &&
		   strcmp(inh_name + inh_name_len - loc_name_len, loc_name) == 0) {
		  switch (inh_mem->get_symbol_type()) {
		  case ldrPUBLIC_METHOD:
		  case ldrPROTECTED_METHOD:
		  case ldrPACKAGE_PROT_METHOD:
		  case ldrPRIVATE_METHOD:
		  case ldrXREF_METHOD: {
		     inh_mem->set_symbol_type(ldrHIDDEN_METHOD);
		     for (ldrMemberNodePtr p = checked_cast(ldrMemberNode,
							    inh_mem->get_first());
			  p; p = checked_cast(ldrMemberNode, p->get_next())) {
			p->set_symbol_type(ldrHIDDEN_METHOD_PARAMETER);
		     }
		  }
		     break;
		     
		  case ldrPUBLIC_MEMBER:
		  case ldrPROTECTED_MEMBER:
		  case ldrPACKAGE_PROT_MEMBER:
		  case ldrPRIVATE_MEMBER:
		  case ldrXREF_MEMBER:
		     inh_mem->set_symbol_type(ldrHIDDEN_MEMBER);
		     break;
		     
		  case ldrPUBLIC_NESTED_TYPE:
		  case ldrPROTECTED_NESTED_TYPE:
		  case ldrPACKAGE_PROT_NESTED_TYPE:
		  case ldrPRIVATE_NESTED_TYPE:
		  case ldrXREF_NESTED_TYPE:
		     inh_mem->set_symbol_type(ldrHIDDEN_NESTED_TYPE);
		     break;
		     
		  default:
		     break;
		  }
		  break;
	       }
	    }
	 }
	 
	 if (conn && added_members.size() && add_connector) {
	    added_members.insert_last(conn);
	 }
	 
	 ldrMemberNodePtr last = NULL;
	 for (size_t i = 0; i < added_members.size(); i++) {
	    if (last) {
	       last->put_after(objTreePtr(added_members[i]));
	    }
	    else the_class->put_first(objTreePtr(added_members[i]));
	    last = checked_cast(ldrMemberNode, added_members[i]);
	 }
	 the_class->set_inherited_members();
      }
   }
}

/*
   START-LOG-------------------------------------------

   $Log: lde-browser-hierarchy.cxx  $
   Revision 1.27 2000/11/29 11:39:30EST ktrans 
   Merge from bug20022: remove subsystems
 * Revision 1.1  1994/09/21  18:43:36  jerry
 * Initial revision
 *
Revision 1.2.1.44  1994/07/26  15:53:06  jerry
Bug track: 7918
When getting subclass list, remove duplicate symbols.  This fixes bug 7918.

Revision 1.2.1.43  1994/07/20  23:19:45  mg
Bug track: NA
cleanup

Revision 1.2.1.42  1994/02/02  00:54:16  trung
Bug track: 6233
set symbol_root in lde show inherited member

Revision 1.2.1.41  1994/01/19  23:15:42  davea
bug tracking: 5918
Use sym_compare() to compare symbols that
might be in different pmods.

Revision 1.2.1.40  1994/01/10  14:40:57  wmm
Bug track: 5834
Fix bug 5834 (subsystem map extraction was being invoked recursively as a result
of kws's propagation of load_module events and it was using static variables).

Revision 1.2.1.39  1993/09/04  00:42:45  trung
fix get members

Revision 1.2.1.38  1993/08/15  17:48:37  smit
Fix bug#4115

Revision 1.2.1.37  1993/08/06  14:57:30  wmm
Fix bug 4191 (by ignoring .pset files during extraction and always presenting
blue classes and members).

Revision 1.2.1.36  1993/07/02  13:55:45  wmm
Fix bug 3810 by only extracting superclass members if requested by the
user, and then only for the selected classes.

Revision 1.2.1.35  1993/06/11  16:22:59  dmitry
Remove obsolete save system

Revision 1.2.1.33  1993/06/07  16:36:49  bakshi
Bug #3588

Revision 1.2.1.32  1993/06/04  13:55:55  bakshi
bracket var decl inside case: c++3.0.1 port

Revision 1.2.1.31  1993/05/25  18:25:38  wmm
Fix bug 3474.

Revision 1.2.1.30  1993/05/17  17:32:36  wmm
Performance tuning, etc., for subsystem extraction.

Revision 1.2.1.29  1993/04/26  20:25:40  wmm
Fix bug 3192 (actually, a related bug, but the original bug appears to have disappeared or been fixed by other changes since it was reported).

Revision 1.2.1.28  1993/04/09  00:36:19  sergey
Put trap to prevent a crash. The bug #3203 still the fix.

Revision 1.2.1.27  1993/04/08  18:18:25  wmm
Fix bugs 3153 and 3154 (check if subsystem member's xrefSymbol has
get_has_def_file() set).

Revision 1.2.1.26  1993/03/30  22:13:30  davea
get_xrefSymbol() calls added, for bug 2927

Revision 1.2.1.25  1993/03/29  19:47:23  trung
rm call to get_real_sym

Revision 1.2.1.24  1993/03/27  02:19:48  davea
changes needed because of symbolPtr

Revision 1.2.1.23  1993/03/23  21:25:27  sergey
Added level arg to extract_subsystem(). Fixed bug #2636.

Revision 1.2.1.22  1993/03/15  21:42:31  davea
change xrefSymbol* to fsymbolPtr

Revision 1.2.1.21  1993/03/08  00:07:29  davea
replace xrefSymbol* with symbolPtr

Revision 1.2.1.20  1993/03/05  14:51:32  wmm
Fix bug that put NULL xrefSymbol*s into an RTL.  (This may be
related to several of the problems in the ttt demo.)

Revision 1.2.1.19  1993/03/01  21:50:25  wmm
Use get_real_sym() to ensure correct extraction after rename.

Revision 1.2.1.18  1993/02/24  02:09:20  wmm
Fix bug 2573 (adapt to DD change that makes ddElement::get_def()
return NULL sometimes [it never did before]).

Revision 1.2.1.17  1993/02/23  17:32:22  wmm
Allow class qualification to appear in subsystem map members.

Revision 1.2.1.16  1993/02/18  23:39:33  trung
change dd_parm-> get_def

Revision 1.2.1.15  1993/02/03  23:45:35  wmm
Load DD for symbols if definition file has .pset file.

Revision 1.2.1.14  1993/02/02  16:11:57  wmm
Temporary arrangement to allow ParaSET to be used on files with an XREF
but without corresponding .pset files.

Revision 1.2.1.13  1993/01/29  20:38:58  wmm
Fix bug 2344.

Revision 1.2.1.12  1993/01/28  02:42:28  efreed
patches to account for dd class/member connection mods.

Revision 1.2.1.11  1993/01/25  23:56:40  wmm
Support XREF-based subsystem views.

Revision 1.2.1.10  1993/01/24  01:13:15  wmm
Support XREF-based class browser view.

Revision 1.2.1.9  1993/01/20  00:14:12  wmm
Adapt to new xref-based implementation of oodt_relation.

Revision 1.2.1.8  1993/01/09  23:52:35  wmm
Allow for cases in which there is no definition of a ddElement.

Revision 1.2.1.7  1993/01/09  18:57:16  wmm
Convert

*/


