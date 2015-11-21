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
// lde_erd
//------------------------------------------
// Synopsis:
// ldr extractor for Entity-Relation Diagrams (ERDs) and Data Charts (DCs)
//
// description:
// Extracts specified classes from the data dictionary, creating connectors for
// inheritance and other relations among them.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include <cLibraryFunctions.h>
#include <msg.h>
#include <math.h>
#include <ldrTypes.h>
#include "Object.h"
#include "oodt_ui_decls.h"
#include "ldrNode.h"
#include "ddict.h"
#include "RTL_externs.h"
#include "RTL_Names.h"
#include "RTL_apl_extern.h"
#include "oodt_relation.h"
#include "graResources.h"
#include <xref.h>
#include <symbolTreeHeader.h>
#include <dd_or_xref_node.h>
#include <lde_erd.h>

#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <messages.h>
#include <groupHdr.h>
#include <groupTree.h>

// local globals

static RTLNodePtr symbol_root;
static ldrNodePtr root;
static ldrNodePtr last;
static ldrERDConnectionNodePtr first_connector;
static bool is_ERD;
static unsigned num_classes;
static symbolArr classes_in_erd;
static symbolSet* classes_for_rels;

// forward function declarations

static ldrSymbolNodePtr get_ldr_class(dd_or_xref_nodePtr);

static char* get_main_name(const symbolPtr& sym)
{
  symbolPtr main_sym = sym;
  while (1)
    {
      symbolArr types;
      if (main_sym.get_kind() == DD_TYPEDEF && 
	  main_sym.get_link(has_type,types))
	main_sym = types[0];
      else
	break;
    }
  return main_sym.get_name();
}

//------------------------------------------
// find(symbolPtr )
//------------------------------------------

static dd_or_xref_nodePtr find(fsymbolPtr  sym)
{
    Initialize(find);

    for (objTreePtr p = symbol_root->get_first(); p; p = p->get_next()) {
	if (is_dd_or_xref_node(p)) {
	    symbolPtr p_sym = dd_or_xref_nodePtr(p)->get_xrefSymbol();
	    if (p_sym.xrisnotnull() && sym.xrisnotnull() &&
		(p_sym.get_kind() == sym.get_kind()) &&
		(strcmp(sym.get_name(), p_sym.get_name()) == 0) )
		return dd_or_xref_nodePtr(p);
	}
    }
    return NULL;
}

//------------------------------------------
// find(ddElementPtr)
//------------------------------------------

static dd_or_xref_nodePtr find(ddElementPtr elem) {
   Initialize(find);

   return find(elem->get_xrefSymbol());
}

//------------------------------------------
// new_node(symbolPtr )
//------------------------------------------

static dd_or_xref_nodePtr new_node(fsymbolPtr  sym) {
   Initialize(new_node);

   dd_or_xref_nodePtr node = db_new(dd_or_xref_node, (sym));
   symbol_root->put_first(node);
   symbol_root->rtl_insert(node->get_xrefSymbol(), true);
   return node;
}

//------------------------------------------
// new_node(ddElementPtr)
//------------------------------------------

static dd_or_xref_nodePtr new_node(ddElementPtr elem) {
   Initialize(new_node);

   dd_or_xref_nodePtr node = db_new(dd_or_xref_node, (elem));
   symbol_root->put_first(node);
   fsymbolPtr  sym = elem->get_xrefSymbol();
   if (sym->xrisnotnull())
      symbol_root->rtl_insert(sym, true);
   return node;
}

//------------------------------------------
// collect_base_classes(dd_or_xref_nodePtr, objArr&)
//------------------------------------------

static void collect_base_classes(dd_or_xref_nodePtr dd_class, objArr& bases) {
   Initialize(collect_base_classes);

   fsymbolPtr  xref_class = dd_class->get_xrefSymbol();
   symbolArr xref_bases;
   if (xref_class->get_link(has_superclass, xref_bases)) {
      xref_bases.sort();
      symbolPtr cl;
      ForEachS(cl, xref_bases) {
	 dd_or_xref_nodePtr node = find(cl.get_xrefSymbol());
	 if (node && node != dd_class)
	    bases.insert_last(node);
      }
   }
}


//------------------------------------------
// get_inheritance_depth(...)
//------------------------------------------

static int get_inheritance_depth(dd_or_xref_nodePtr dd_class, int this_depth,
      const objSet& loop_limiter) {
   Initialize(get_inheritance_depth);

   objArr bases;
   collect_base_classes(dd_class, bases);
   int max_depth = this_depth;
   objSet this_limiter = loop_limiter;
   this_limiter.insert(dd_class);

   for (int i = 0; i < bases.size(); i++) {
      if (!this_limiter.includes((Obj*) bases[i])) {
	 int new_depth = get_inheritance_depth(checked_cast(dd_or_xref_node,
               bases[i]), this_depth + 1, this_limiter);
	 max_depth = (new_depth > max_depth) ? new_depth : max_depth;
      }
   }

   return max_depth;
}

//------------------------------------------
// clone_predefined_relation(...)
//------------------------------------------

static void clone_predefined_relation(dd_or_xref_nodePtr src,
      dd_or_xref_nodePtr trg, ldrERDConnectionNodePtr& conn,
      const char* rel_name, const char* inv_name, const char* member_name,
      bool suppress_self_reference, bool do_fwd_rel, bool do_inv_rel) {
   Initialize(clone_predefined_relation);

   oodt_relationPtr rel = oodt_relation::find_builtin(rel_name);
   oodt_relationPtr inv = oodt_relation::find_builtin(inv_name);
   ldrSymbolNodePtr ldr_src = get_ldr_class(src);
   ldrSymbolNodePtr ldr_trg = get_ldr_class(trg);

   if (!rel || !inv || !src || !trg || !ldr_src || !ldr_trg)
      return;

   if (do_fwd_rel) {
      if (!conn) {
	 conn = db_new(ldrERDConnectionNode, (ldrcERD_RELATION, rel_name, "",
	       "", rel->color_resource(), rel->cardinality(), rel->required(),
	       true));
	 ldrtree_put_apptree(conn, rel);
	 conn->add_input(ldr_src);
	 if (last)
	    last->put_after(conn);
	 else root->put_first(conn);
	 last = conn;
	 if (!first_connector)
	    first_connector = conn;
      }
      conn->add_output(ldr_trg);
      conn->append_to_label(member_name);
   }

   if (suppress_self_reference && src == trg)
      return;

   bool found_trg = false;
   objSet* inv_conns = (objSet*) apptree_get_ldrtrees(inv);
   Obj* inv_conn;
   ForEach(inv_conn, *inv_conns) {
      if (checked_cast(objTree, inv_conn)->get_parent() == root) {
	 objSet* inv_inputs =
               checked_cast(ldrERDConnectionNode,inv_conn)->
	       get_input_symbols();
	 if (inv_inputs && inv_inputs->includes(ldr_trg)) {
	    ldrERDConnectionNodePtr ic = checked_cast(ldrERDConnectionNode, inv_conn);
	    ic->add_output(ldr_src);
	    ic->append_to_label(member_name);
	    found_trg = true;
	    break;
	 }
      }
   }

   if (!found_trg && do_inv_rel) {
      ldrERDConnectionNodePtr ic = db_new(ldrERDConnectionNode,
            (ldrcERD_RELATION, inv_name, "", "", inv->color_resource(),
            inv->cardinality(), inv->required(), true));
      ldrtree_put_apptree(ic, inv);
      ic->add_input(ldr_trg);
      ic->add_output(ldr_src);
      ic->append_to_label(member_name);
      if (last)
	 last->put_after(ic);
      else root->put_first(ic);
      last = ic;
      if (!first_connector)
         first_connector = ic;
   }
}

// Functions returns non zero if found class name as an argument to function "function"
static int is_argument(symbolPtr function, symbolPtr class_sym)
{
    Initialize(is_argument);
    
    char *c_name = class_sym.get_name();
    char *f_name = function.get_name();
    char *p      = strchr(f_name, '(');
    if(!p)
	return 0;
    while(*p){
	p = strstr(p, c_name);
	if(!p)
	    return 0;
	if(!isalpha(*(p - 1)) && !isalpha(*(p + strlen(c_name))))
	    return 1;
	p++;
    }
    return 0;
}

static void find_relations(symbolPtr xref_class, symbolArr& classes, objArr_Int& relations, symbolArr& resp_members)
{
    Initialize(find_relations);

    symbolArr xref_members;
    if (xref_class->get_link(is_using, xref_members)) {
        xref_members.sort();
	symbolPtr mem;
	ForEachS(mem, xref_members) {
	    if(dd_is_var_declaration(mem.get_kind())){
		symbolArr results;
		mem->get_link(have_arg_type, results);
		results.sort();
		symbolPtr res;
		ForEachS(res, results){
		    if(res.get_kind() == DD_CLASS || res.get_kind() == DD_SQL_TABLE){
			resp_members.insert_last(mem);
			classes.insert_last(res);
			symbolArr types;
			mem->get_link(has_type, types);
			types.sort();
			symbolPtr sym;
			ForEachS(sym, types){
			    char *name = get_main_name(sym);
			    if(strchr(name, '*'))
				relations.insert_last(POINTER_TO);
			    else
				if(strchr(name, '&'))
				    relations.insert_last(REFERENCE_TO);
				else
				    relations.insert_last(OBJECT_OF);
			}
		    }
		}
	    }
	    if(dd_is_function_declaration(mem.get_kind())){
		symbolArr results;
		mem->get_link(have_arg_type, results);
		results.sort();
		symbolPtr res;
		ForEachS(res, results){
		    if(res.get_kind() == DD_CLASS || res.get_kind() == DD_SQL_TABLE){
			symbolArr types;
			mem->get_link(has_type, types);
			types.sort();
			symbolPtr sym;
			ForEachS(sym, types){
			    char *name = get_main_name(sym);
			    // Check is it return type of the function
			    char* res_name = res.get_name();
			    int len = strlen(res_name);
			    if (!strncmp(name,res_name,len) && !isalnum(name[len])){
			        resp_members.insert_last(mem);
			        classes.insert_last(res);
			        relations.insert_last(FUNCTION_RETURNING);
			    }
			}
			// Check is it argument also
			if(is_argument(mem, res)){
			    resp_members.insert_last(mem);
			    classes.insert_last(res);
			    relations.insert_last(FUNCTION_TAKING_ARG);
			}
		    }
		}
	    }
	}
    }
}

//------------------------------------------
// extract_classes(objSet&, int, unsigned)
//------------------------------------------

static void extract_classes(objSet& relations, int filt,
      unsigned classes_per_level) {
   Initialize(extract_classes);

   unsigned classno = 0;
   for (ldrNodePtr nodep = checked_cast(ldrNode,root->get_first());
         nodep != first_connector; nodep = checked_cast(ldrNode,
         nodep->get_next()), classno++) {
      if (!nodep->ldrIsA(ldrERDBoxNodeType)) {
	 ldrSymbolNodePtr ldr_class = checked_cast(ldrSymbolNode, nodep);
	 dd_or_xref_nodePtr class_node =
	       checked_cast(dd_or_xref_node, ldr_class->get_appTree());
	 fsymbolPtr  xref_class = class_node->get_xrefSymbol();
	 ldrERDConnectionNodePtr contains_conn = NULL;
	 ldrERDConnectionNodePtr points_to_conn = NULL;
	 ldrERDConnectionNodePtr refers_to_conn = NULL;
	 ldrERDConnectionNodePtr function_returns_conn = NULL;
	 ldrERDConnectionNodePtr takes_argument_conn = NULL;
	 ldrERDConnectionNodePtr has_friend_conn = NULL;
	 ldrERDConnectionNodePtr container_conn = NULL;
	 ldrMemberNodePtr last_mem = NULL;

	 if (is_ERD) {
	    objSet loop_limiter;
	    ldr_class->set_graphlevel(get_inheritance_depth(class_node,
		  0, loop_limiter));
	 }
	 else ldr_class->set_graphlevel(classno / classes_per_level);

	 symbolArr xref_members;

	 symbolArr  related_classes;
	 objArr_Int relation_types;
	 symbolArr  responsible_members;
	 find_relations(xref_class, related_classes, relation_types, responsible_members);
	 for(int i = 0; i < related_classes.size(); i++){
	     symbolPtr trg               = related_classes[i];
	     dd_or_xref_nodePtr trg_node = find(trg);
	     dd_or_xref_nodePtr src_node = find(xref_class);
	     switch(relation_types[i]){
	       case OBJECT_OF:
		 if (filt & (contains_rel | contained_in_rel))
		     clone_predefined_relation(src_node, trg_node, contains_conn,
					       "has_member_of_type", "type_of_member", responsible_members[i].get_name(),
					       false, filt & contains_rel,
					       filt & contained_in_rel);
		 break;
	       case POINTER_TO:
		 if (filt & (points_to_rel | pointed_to_by_rel)){
		     clone_predefined_relation(src_node, trg_node, points_to_conn,
					       "points_to", "pointed_to_by", responsible_members[i].get_name(),
					       true, filt & points_to_rel,
					       filt & pointed_to_by_rel);
		 }
		 break;
	       case FUNCTION_RETURNING:
		 if (is_ERD && (filt & (method_type_rel | returned_by_function_rel)))
		     clone_predefined_relation(src_node, trg_node, function_returns_conn, 
					       "function_returns", "returned_by_function",
					       responsible_members[i].get_name(),
					       false, filt & method_type_rel,
					       filt & returned_by_function_rel);
		 break;
	       case FUNCTION_TAKING_ARG:
		 if (is_ERD && (filt & (method_argument_rel | is_argument_rel)))
		     clone_predefined_relation(src_node, trg_node, takes_argument_conn, 
					       "takes_argument", "is_argument",
					       responsible_members[i].get_name(),
					       false, filt & method_argument_rel,
					       filt & is_argument_rel);
		 break;
	     }
	 }


	 if (xref_class->get_link(is_using, xref_members)) {
	    xref_members.sort();
	    symbolPtr mem;
	    ForEachS(mem, xref_members) {
	       ldrMemberNodePtr ldr_mem = NULL;
	       fsymbolPtr  xref_mem = mem.get_xrefSymbol();
	       member_access_sort_types prot;
	       if (dd_is_var_declaration(xref_mem.get_kind())){
		   if(xref_mem->get_attribute(PRIV_ATT, 1))
		       prot = PRIVATE_MEMBERS;
		   else
		       if(xref_mem->get_attribute(PROT_ATT, 1))
			   prot = PROTECTED_MEMBERS;
		       else
			   if(xref_mem->get_attribute(PAKG_ATT, 1))
			       prot = PACKAGE_PROT_MEMBERS;
			   else
			       prot = PUBLIC_MEMBERS;
		   ldr_mem = db_new(ldrMemberNode, (ldrPUBLIC_MEMBER,
						    prot, new_node(xref_mem), ""));
	       }
	       else if (dd_is_function_declaration(xref_mem.get_kind())){
		   if(xref_mem->get_attribute(PRIV_ATT, 1))
		       prot = PRIVATE_MEMBERS;
		   else
		       if(xref_mem->get_attribute(PROT_ATT, 1))
			   prot = PROTECTED_MEMBERS;
		       else
			   if(xref_mem->get_attribute(PAKG_ATT, 1))
			       prot = PACKAGE_PROT_MEMBERS;
			   else
			       prot = PUBLIC_MEMBERS;
		   if (!xref_mem->get_attribute(COMP_GEN_ATT, 1)) ldr_mem = db_new(ldrMemberNode, (ldrPUBLIC_METHOD,
						    prot, new_node(xref_mem), ""));
	       }
	       if (ldr_mem) {
//		     ldr_mem->set_symbol_type(ldrXREF_ERD_MEMBER);
		  ldr_mem->set_symbol_type(ldrERD_MEMBER);
		  if (last_mem)
		     last_mem->put_after(ldr_mem);
		  else ldr_class->put_first(ldr_mem);
		  last_mem = ldr_mem;
	       }
	    }
	  }

	 if (is_ERD && (filt & (has_friend_rel | is_friend_rel))) {
	   dd_or_xref_nodePtr src_node = find(xref_class);	      
	   symbolArr friend_classes;
	   xref_class->get_link(have_friends, friend_classes);
	   friend_classes.sort();
	   symbolPtr friend_class;
	    ForEachS(friend_class, friend_classes) {
	      clone_predefined_relation(src_node,
					find(friend_class), has_friend_conn,
					"has_friend", "is_friend", "", false, filt & has_friend_rel,
					filt & is_friend_rel);
	    }
	 }

	 if (is_ERD && (filt & (nested_in_rel | container_of_rel))) {
	     dd_or_xref_nodePtr src_node = find(xref_class);
	     symbolArr nested_objects;
	     xref_class->get_link(member_of, nested_objects);
    	     nested_objects.sort();
	     symbolPtr nested_object;
	     ForEachS(nested_object, nested_objects) {
		 clone_predefined_relation(src_node,
					   find(nested_object), container_conn,
					   "container_of", "nested_in", "", false, filt & nested_in_rel,
					   filt & container_of_rel);
	     }
	 }

	 symbolArr rels;
	 size_t idx;
	 for (idx = 0; idx < classes_in_erd.size() && classes_in_erd[idx] != xref_class;
	      idx++) { }
	 if (idx < classes_in_erd.size()) {
	    symbolPtr cl;
	    ForEachT(cl, classes_for_rels[idx]) {
	       oodt_relation::get_outgoing_relations_of(cl, rels);
	       oodt_relation::get_incoming_relations_of(cl, rels);
	    }
	    symbolPtr rel;
	    ForEachS(rel, rels) {
	       fsymbolPtr  xr_rel = rel.get_xrefSymbol();
	       ddElementPtr dd_rel = xr_rel->get_def_dd();
	       if (dd_rel) {
		  oodt_relationPtr oo_rel = checked_cast(oodt_relation,
			dd_get_def_assocnode(dd_rel));
		  if (oo_rel) {
		     relations.insert(oo_rel);
		  }
	       }
	    }
	 }

// For DCs, add "members" representing functions that use the struct

	 if (!is_ERD) {
	    symbolArr ref_fcns;
	    xref_class->get_link_chase_typedefs(used_by, ref_fcns);
	    symbolPtr ref_fcn;
	    //ref_fcns.remove_dup_syms();
	    ref_fcns.usort();
	    ForEachS(ref_fcn, ref_fcns) {
	       fsymbolPtr  fcn_xref = ref_fcn.get_xrefSymbol();
	       if (fcn_xref.get_kind() == DD_FUNC_DECL) {
		  ldrMemberNodePtr ldr_mem = db_new(ldrMemberNode,
			(ldrPUBLIC_METHOD, PUBLIC_MEMBERS,
			new_node(fcn_xref), ""));
		  if (ldr_mem) {
		     if (fcn_xref.is_loaded_or_cheap())
			ldr_mem->set_symbol_type(ldrERD_MEMBER);
		     else ldr_mem->set_symbol_type(ldrXREF_ERD_MEMBER);
		     if (last_mem)
			last_mem->put_after(ldr_mem);
		     else ldr_class->put_first(ldr_mem);
		     last_mem = ldr_mem;
		  }
	       }
	    }
	 }
      }
   }
}

//------------------------------------------
// add_inheritance()
//------------------------------------------

static void add_inheritance() {
   Initialize(add_inheritance);

   for (ldrNodePtr nodep = checked_cast(ldrNode, root->get_first());
         nodep != first_connector; nodep = checked_cast(ldrNode,
         nodep->get_next())) {
      if (!nodep->ldrIsA(ldrERDBoxNodeType)) {
	 ldrSymbolNodePtr ldr_class = checked_cast(ldrSymbolNode, nodep);
	 dd_or_xref_nodePtr cur_class = checked_cast(dd_or_xref_node,
	       ldr_class->get_appTree());
	 objArr bases;
	 collect_base_classes(cur_class, bases);
	 if (bases.size()) {

   /* Here is where we optimize the creation of connection nodes: if there
    * is already a connector attached to exactly the same set of base classes,
    * we just add the current class as an output to that connector.  Otherwise,
    * it is necessary to create a new connector with the specified list of
    * bases as inputs.
    */

	    ldrSymbolNodePtr ldr_base =
		  get_ldr_class(checked_cast(dd_or_xref_node, bases[0]));
	    ldrERDConnectionNodePtr inh_conn;
	    bool connector_found = false;
	    if (ldr_base) {
	       objSet* connectors = ldr_base->get_output_connectors();
	       Obj* connector;
	       ForEach(connector, *connectors) {
		  inh_conn = checked_cast(ldrERDConnectionNode,connector);
		  connector_found = (inh_conn->get_symbol_type() ==
			ldrcERD_INHERITANCE &&
			inh_conn->get_input_symbols()->size() == bases.size());
		  for (int i = 0; connector_found && i < bases.size(); i++) {
		     connector_found = inh_conn->is_input_p(get_ldr_class(
			   checked_cast(dd_or_xref_node, bases[i])));
		  }
		  if (connector_found)
		     break;
	       }
	    }
	    if (!connector_found) {
	       inh_conn = db_new(ldrERDConnectionNode, (ldrcERD_INHERITANCE, "",
		     "", "", INHERITANCE_CLR, rel_many_to_many, false, false));
	       if (!first_connector)
		  first_connector = inh_conn;
	       for (int i = 0; i < bases.size(); i++) {
		  ldr_base = get_ldr_class(checked_cast(dd_or_xref_node,
			bases[i]));
		  inh_conn->add_input(ldr_base);
	       }
	       if (last)
		  last->put_after(inh_conn);
	       else root->put_first(inh_conn);
	       last = inh_conn;
	    }
	    inh_conn->add_output(ldr_class);
	 }
      }
   }
}

//------------------------------------------
// add_relations(const objSet&, Regexp*)
//------------------------------------------

static void add_relations(const objSet& relations, Regexp* relname_filt)
{
   Initialize(add_relations);

   Obj* cur;
   ForEach(cur, relations) {
      oodt_relationPtr curr_rel = checked_cast(oodt_relation,cur);
      Obj* dds = def_assocnode_get_dds(curr_rel);
      Assert(dds && dds->relationalp());
      ddElementPtr dd = checked_cast(ddElement, dds);
      symbol_root->rtl_insert(dd->get_xrefSymbol(), true);

/* Here we merge a relation and its inverse into a single connector;
 * basically, we check to see if the relations inverse already has an
 * ldr connector in this ldr and, if so, do nothing (the connector will
 * have already been labelled with the name of this relation, and we
 * assume that the inputs and outputs of inverse relations are strictly
 * mirror images).
 */

       const char* inv_name = curr_rel->inv_name();
       bool already_processed = false;
       if (inv_name && *inv_name) {
	  oodt_relationPtr inv_rel = oodt_relation::find(inv_name);
	   if (inv_rel) {
	       objSet* ldr_connectors = (objSet*) apptree_get_ldrtrees(
		   inv_rel);
	       Obj* ldr_connector;
	       ForEach(ldr_connector, *ldr_connectors) {
		   if (checked_cast(ldrNode,ldr_connector)->get_parent()
		          == root) {
		       already_processed = true;
		       break;
		   }
	       }
	   }
       }
       char* curr_name = curr_rel->get_name();
       if (!already_processed && 
	   (!relname_filt
	    || relname_filt->Match(curr_name, strlen(curr_name), 0) > 0))
       {
	   int num_inputs = 0;
	   int num_outputs = 0;
	   if (!inv_name)
	       inv_name = "";
	   ldrERDConnectionNodePtr ldr_rel =
	       db_new(ldrERDConnectionNode,
		      (ldrcERD_RELATION, curr_rel->get_name(), inv_name, "",
		       NULL, curr_rel->cardinality(), curr_rel->required(),
		       !*inv_name));

	   ldrtree_put_apptree(ldr_rel, curr_rel);
	   size_t idx;
	   symbolPtr src = curr_rel->get_src_xrefSymbol();
	   for (idx = 0; idx < classes_in_erd.size(); idx++) {
	      if (classes_for_rels[idx].includes(src)) {
		 dd_or_xref_nodePtr rel_input = find(classes_in_erd[idx]);
		 if (rel_input) {
		     ldrSymbolNodePtr ldr_class = get_ldr_class(rel_input);
		     if (ldr_class) {
			 ldr_rel->add_input(ldr_class);
			 num_inputs++;
		     }
		 }
	      }
	   }
	   symbolPtr trg = curr_rel->get_trg_xrefSymbol();
	   for (idx = 0; idx < classes_in_erd.size(); idx++) {
	      if (classes_for_rels[idx].includes(trg)) {
		 dd_or_xref_nodePtr rel_output = find(classes_in_erd[idx]);
		 if (rel_output) {
		     ldrSymbolNodePtr ldr_class = get_ldr_class(rel_output);
		     if (ldr_class) {
			 ldr_rel->add_output(ldr_class);
			 num_outputs++;
		     }
		 }
	      }
	   }
	   if (num_inputs && num_outputs) {
	       if (last)
		   last->put_after(ldr_rel);
	       else root->put_first(ldr_rel);
	       last = ldr_rel;
	       if (!first_connector)
		   first_connector = ldr_rel;
	   }
	   else obj_delete(ldr_rel);
       }
   }
}

//------------------------------------------
// is_struct(symbolPtr)
//
// Returns 1 if argument has no C++-specific features (inheritance,
// member functions, access control, references).  This test will be
// incorrect and unneeded if/when the DD can tell the difference between
// classes and structs.
//------------------------------------------

static bool is_struct(symbolPtr class_sym) {
   Initialize(is_struct);

   bool no_class_features = 1;
   fsymbolPtr class_xref = class_sym.get_xrefSymbol();
//   ddElementPtr dd_class = class_xref->is_loaded_or_cheap() ?
//         class_xref->get_def_dd() : NULL;
   ddElementPtr dd_class = NULL;
   if (dd_class) {

   // check for inheritance (in any of the six flavors)

      if (get_relation(pub_baseclassof, dd_class))
	 no_class_features = 0;
      else if (get_relation(pro_baseclassof, dd_class))
	 no_class_features = 0;
      else if (get_relation(pri_baseclassof, dd_class))
	 no_class_features = 0;
      else if (get_relation(vpub_baseclassof, dd_class))
	 no_class_features = 0;
      else if (get_relation(vpro_baseclassof, dd_class))
	 no_class_features = 0;
      else if (get_relation(vpri_baseclassof, dd_class))
	 no_class_features = 0;

   // check for friend declarations

      if (get_relation(has_friend, dd_class))
	 no_class_features = 0;

      // new way of scanning dd class members

      objArr ddArr;
      Obj   *ob;

      dd_class->class_get_members(ddArr);

      ForEach(ob, ddArr) {
         if (!no_class_features)
            break;

         ddElementPtr mem = checked_cast(ddElement, ob);


	 if (!mem->is_public())
	    no_class_features = 0;
	 else if (mem->get_kind() != DD_FIELD)
	    no_class_features = 0;
	 else {
	    const char* type = mem->get_def();
	    if (type && strchr(type, '&'))	// has reference type
	       no_class_features = 0;
	 }
      }
   }
   else {

// We can only do a more limited version of the testing if we're working
// from the XREF, since we don't have access categories, return types, etc.

      symbolArr bases;
      if (class_xref->get_link(has_superclass, bases)) {
	 if (!(bases.size() == 1 && bases[0].get_xrefSymbol() == class_xref))
            no_class_features = 0;
      }

      symbolArr members;
      if (class_xref->get_link(is_using, members)) {
         members.sort();
	 symbolPtr member;
	 ForEachS(member, members) {
	     if(member->get_attribute(PRIV_ATT, 1) || member->get_attribute(PROT_ATT, 1)){
		 no_class_features = 0;
		 break;
	     }
	     fsymbolPtr  member_xref = member.get_xrefSymbol();
	     if (member_xref.get_kind() == DD_FUNC_DECL &&
		 strstr(member_xref.get_name(), "::")) {
		 no_class_features = 0;
		 break;
	     }
	 }
      }
   }

   return no_class_features;
}

//------------------------------------------
// create_ldr_class(symbolPtr)
//------------------------------------------

static ldrSymbolNodePtr create_ldr_class(symbolPtr class_sym) {
   Initialize(create_ldr_class);

   fsymbolPtr  class_xref = class_sym.get_xrefSymbol();
   if (   class_xref->xrisnotnull()
       && (   class_xref.get_kind() == DD_CLASS
           || class_xref.get_kind() == DD_SQL_TABLE
           || class_xref.get_kind() == DD_INTERFACE)) {
      if (!is_ERD && !is_struct(class_sym)) {
	 msg("ERROR: Only structs can be displayed in data charts - class $1 ignored") << class_xref.get_name() << eom;
	 return NULL;
      }
      ldrSymbolNodePtr ldr_class = get_ldr_class(find(class_xref));
      if (!ldr_class) {
	 classes_in_erd.insert_last(class_xref);
//	 if (class_xref->is_loaded_or_cheap())
	    ldr_class = db_new(ldrSymbolNode, (ldrERD_CLASS));
//	 else ldr_class = db_new(ldrSymbolNode, (ldrXREF_ERD_CLASS));
	 ldr_class->set_appTree(new_node(class_xref));
	 if (last)
	    last->put_after(ldr_class);
	 else root->put_first(ldr_class);
	 last = ldr_class;
	 num_classes++;
      }
      return ldr_class;
   }

   return NULL;
}

//------------------------------------------
// find_undisplayed_bases
//------------------------------------------

static void find_undisplayed_bases(symbolPtr this_class, symbolSet& all_bases,
				   symbolSet& classes_for_rels) {
   Initialize(find_undisplayed_bases);

   if (!all_bases.includes(this_class)) {
      all_bases.insert(this_class);
      if (!classes_in_erd.includes(this_class)) {
	 classes_for_rels.insert(this_class);
      }
      symbolArr bases;
      this_class->get_link_chase_typedefs(has_superclass, bases);
      symbolPtr base;
      ForEachS(base, bases) {
	 find_undisplayed_bases(base, all_bases, classes_for_rels);
      }
   }
}

//------------------------------------------
// setup_classes_for_rels
//------------------------------------------

static void setup_classes_for_rels(bool disp_inh_rels) {
   Initialize(setup_classes_for_rels);

   classes_for_rels = new symbolSet[classes_in_erd.size()];
   size_t i = 0;
   symbolPtr class_in_erd;
   ForEachS(class_in_erd, classes_in_erd) {
      classes_for_rels[i].insert(class_in_erd);
      if (disp_inh_rels && is_ERD) {
	 symbolSet all_bases;
	 find_undisplayed_bases(class_in_erd, all_bases, classes_for_rels[i]);
      }
      i++;
   }
}

//------------------------------------------
// do_extraction(const symbolArr&, int, Regexp*, RTLNodePtr)
//------------------------------------------

static ldrNodePtr do_extraction(const symbolArr& contents, int filt,
      Regexp* relname_filt, bool inh_rels, RTLNodePtr sym_root) {
   Initialize(do_extraction);

   RTLNodePtr saved_symbol_root = symbol_root;
   ldrNodePtr saved_root = root;
   ldrNodePtr saved_last = last;
   ldrERDConnectionNodePtr saved_first_connector = first_connector;
   unsigned saved_num_classes = num_classes;
   symbolArr saved_classes_in_erd = classes_in_erd;
   symbolSet* saved_classes_for_rels = classes_for_rels;
   
   root = db_new(ldrNodeNoTokens, ());
   symbol_root = sym_root;
   last = NULL;
   first_connector = NULL;
   num_classes = 0;
   classes_in_erd.removeAll();

/* The extractor can be parameterized with any of a number of different
 * kinds of app nodes; the code below determines the kind of node and
 * adds whatever classes are associated with that node.
 */
   
   symbolPtr cur;
   ForEachS(cur, contents) {
      fsymbolPtr  cur_xref = cur.get_xrefSymbol();
      ddKind kind = cur_xref.get_kind();

      if (kind == DD_CLASS || kind == DD_SQL_TABLE || kind == DD_INTERFACE) {
	 create_ldr_class(cur);
      }

      // A relation: add the input and output classes.

      else if (kind == DD_RELATION) {
	 oodt_relationPtr rel = checked_cast(oodt_relation, (appTreePtr)(cur));
	 create_ldr_class(rel->get_src_xrefSymbol());
	 create_ldr_class(rel->get_trg_xrefSymbol());
      }

      // A typedef: add the target class

      else if (kind == DD_TYPEDEF) {
	 symbolArr classes;
	 cur_xref->get_link(have_arg_type, classes);
	 symbolPtr cur_class;
	 ForEachS(cur_class, classes) {
	    create_ldr_class(cur_class);
	 }
      }
   }
   
   objSet relations;
   unsigned classes_per_level = unsigned(ceil(sqrt(double(num_classes))));
   setup_classes_for_rels(inh_rels);
   extract_classes(relations, filt, classes_per_level);
   if (is_ERD)
      add_inheritance();
   add_relations(relations, relname_filt);

   ldrNodePtr ret_val = root;
   root = saved_root;
   symbol_root = saved_symbol_root;
   last = saved_last;
   first_connector = saved_first_connector;
   num_classes = saved_num_classes;
   classes_in_erd = saved_classes_in_erd;
   delete [] classes_for_rels;
   classes_for_rels = saved_classes_for_rels;

   return ret_val;
}

//------------------------------------------
// lde_extract_erd(...)
//------------------------------------------

ldrNodePtr lde_extract_erd(const symbolArr& contents, int filt, Regexp* relname_filt,
			   bool inherited_rels, RTLNodePtr sym_root) {
   Initialize(lde_extract_erd);

   bool saved_is_ERD = is_ERD;
   is_ERD = 1;
   ldrNodePtr ret_val = do_extraction(contents, filt, relname_filt, inherited_rels,
				      sym_root);
   is_ERD = saved_is_ERD;
   return ret_val;
}


//------------------------------------------
// lde_extract_dc(...)
//------------------------------------------

ldrNodePtr lde_extract_dc(const symbolArr& contents, int filt, Regexp* relname_filt,
			  bool inherited_rels, RTLNodePtr sym_root) {
   Initialize(lde_extract_dc);

   bool saved_is_ERD = is_ERD;
   is_ERD = 0;
   ldrNodePtr ret_val = do_extraction(contents, filt, relname_filt, inherited_rels,
				      sym_root);
   is_ERD = saved_is_ERD;
   return ret_val;
}

//------------------------------------------
// get_ldr_class(dd_or_xref_nodePtr)
//------------------------------------------

static ldrSymbolNodePtr get_ldr_class(dd_or_xref_nodePtr dd_class) {
   Initialize(get_ldr_class);

   if (dd_class) {
      objSet* ldrnodes = (objSet*) apptree_get_ldrtrees(dd_class);
      Obj* ldrnode;
      ForEach(ldrnode, *ldrnodes) {
	 ldrNodePtr par = checked_cast(ldrNode, checked_cast(ldrSymbolNode,ldrnode)->get_parent());
         while (par != root)
            par = checked_cast(ldrNode, checked_cast(ldrNode,par)->get_parent());
         if (par == root)
	    return checked_cast(ldrSymbolNode,ldrnode);
      }
   }

   return NULL;
}

/*
   START-LOG-------------------------------------------

   $Log: lde_erd.cxx  $
   Revision 1.32 2000/11/29 11:39:33EST ktrans 
   Merge from bug20022: remove subsystems
Revision 1.2.1.55  1994/08/01  14:36:07  azaparov
Fxied to support ERD & Data charts without loading .psets

Revision 1.2.1.54  1994/04/11  14:57:35  trung
bugs 6869, 6871, 6937 & 6909, 6939

Revision 1.2.1.53  1994/03/24  14:38:18  bhowmik
Bug track: 6705
Check for duplicates in DCs.

Revision 1.2.1.52  1994/02/01  16:14:58  trung
Bug track: 6065
change new_node in lde_erd

Revision 1.2.1.51  1994/01/26  23:01:19  davea
bug track: 6065
partial fix

Revision 1.2.1.50  1994/01/24  19:34:41  davea
bug 6065
In do_subsys_classes_and_boxes, don't
convert to xrefSymbol.  Just store objects
directly.

Revision 1.2.1.49  1994/01/17  16:57:24  trung
put subsystem in symbol tree header

Revision 1.2.1.48  1993/12/17  21:15:49  wmm
Bug track: 5676
Fix bug 5676 (allow recursive invocation of extractor resulting from obj_insert() during
restoration of file).

Revision 1.2.1.47  1993/12/08  22:38:48  bella
Bug track: 5422
testing

Revision 1.2.1.46  1993/12/02  16:31:05  wmm
Bug track: 5127
Fix bug 5127.

Revision 1.2.1.45  1993/10/18  23:17:33  wmm
Bug track: 5013
Fix "bug" (enhancement request) 5013 -- allow opening Data Chart using
typedef synonym of struct instead of struct tag.

Revision 1.2.1.44  1993/09/28  00:37:30  pero
Bug track: 9999
find(fsymbolPtr) has to be modified to accomodate better
comparison of two symbols; name + kind

Revision 1.2.1.43  1993/09/07  21:59:35  trung
fix crashing viewing subsys

Revision 1.2.1.42  1993/07/07  14:33:22  wmm
Fix bug 3931.

Revision 1.2.1.41  1993/06/28  15:22:45  wmm
Fix bug 3847.

Revision 1.2.1.40  1993/06/21  19:36:38  wmm
Fix bugs 3768, 3718, and 2899.

Revision 1.2.1.39  1993/05/28  22:21:35  wmm
Fix bug 3241.

Revision 1.2.1.38  1993/05/17  17:32:42  wmm
Performance tuning, etc., for subsystem extraction.

Revision 1.2.1.37  1993/05/12  20:29:01  sergey
Added graph level setting in do_subsys_or_...(). Fix of the bug #3322.

Revision 1.2.1.36  1993/05/02  23:31:00  wmm
Find more struct-to-function relationships via typedefs.  (There
are still some, e.g., sockaddr_in in the Summa code, that are not
found by this code, presumably due to XREF bugs/deficiencies.)

Revision 1.2.1.35  1993/05/01  20:58:00  wmm
Partial fix for bug 3569 (add typedef'ed references to operations
on DCs).

Revision 1.2.1.34  1993/04/02  14:09:15  wmm
Fix bug 3096.

Revision 1.2.1.33  1993/04/01  20:21:15  wmm
*Partial* fix for bug 3060 (rest of fix will be submitted by
Trung): allow for static members having ddKind of DD_VAR_DECL
instead of just DD_FIELD.

Revision 1.2.1.32  1993/03/30  22:14:48  davea
get_xrefSymbol() calls added, for bug 2927

Revision 1.2.1.31  1993/03/29  19:47:23  trung
rm call to get_real_sym

Revision 1.2.1.30  1993/03/15  21:45:24  davea
Change xrefSymbol* to fsymbolPtr

Revision 1.2.1.29  1993/03/08  00:07:58  davea
replace xrefSymbol* with symbolPtr

Revision 1.2.1.28  1993/03/05  14:51:32  wmm
Fix bug that put NULL xrefSymbol*s into an RTL.  (This may be
related to several of the problems in the ttt demo.)

Revision 1.2.1.27  1993/03/01  21:50:25  wmm
Use get_real_sym() to ensure correct extraction after rename.

Revision 1.2.1.26  1993/03/01  19:40:22  wmm
Roll back incorrect bug fix.

Revision 1.2.1.24  1993/02/24  20:56:05  wmm
Fix bug 2666 (adapt to DD change that can return SMT nodes as
friends of ddElements).

Revision 1.2.1.23  1993/02/24  02:13:56  wmm
Prevent crash (adapt to DD change that makes ddElement::get_def()
return NULL sometimes [it never did before]).

Revision 1.2.1.22  1993/02/23  17:32:22  wmm
Allow class qualification to appear in subsystem map members.

Revision 1.2.1.21  1993/02/03  23:45:35  wmm
Load DD for symbols if definition file has .pset file.

Revision 1.2.1.20  1993/02/02  16:11:57  wmm
Temporary arrangement to allow ParaSET to be used on files with an XREF
but without corresponding .pset files.

Revision 1.2.1.19  1993/01/29  20:38:58  wmm
Fix bug 2344.

Revision 1.2.1.18  1993/01/28  02:43:26  efreed
patch to account for dd class/member connection mods.

Revision 1.2.1.17  1993/01/26  15:17:32  wmm
Fix bug 2300, write around impact of bug 2301 on ERD/DC.

Revision 1.2.1.16  1993/01/26  00:18:28  wmm
Add related functions as DC "members", partially support
opening subsystems in ERDs (again).

Revision 1.2.1.15  1993/01/22  22:47:14  wmm
Support XREF-based ERDs and DCs.

Revision 1.2.1.14  1993/01/20  00:14:12  wmm
Adapt to new xref-based implementation of oodt_relation.

Revision 1.2.1.13  1993/01/09  23:52:35  wmm
Allow for cases in which there is no definition of a ddElement.

Revision 1.2.1.12  1993/01/06  23:24:51  wmm
Use new XREF to pursue inheritance relationships across file
boundaries.

Revision 1.2.1.11  1993/01/06  20:24:40  wmm
Support new subsystem implementation.

Revision 1.2.1.10  1992/12/28  19:18:36  wmm
Support new subsystem implementation.

Revision 1.2.1.9  1992/12/18  16:49:56  glenn
Fix symbol cast problem.

Revision 1.2.1.8  1992/12/17  21:29:59  wmm
Add lde_extract_dc for Data Charts.

Revision 1.2.1.7  1992/12/11  14:55:05  aharlap
call function instead direct access to xref

Revision 1.2.1.6  1992/12/09  21:09:34  aharlap
added interface for new xref

Revision 1.2.1.5  1992/11/24  23:13:48  wmm
Fix some casting errors in ERD extraction.

Revision 1.2.1.4  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.3  1992/11/18  14:39:55  trung
 change dd_of_xref to function

Revision 1.2.1.2  92/10/09  19:18:57  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/

