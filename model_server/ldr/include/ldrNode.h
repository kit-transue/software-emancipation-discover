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
#ifndef __ldrNode_h_
#define __ldrNode_h_

#ifndef _genString_h
#include "genString.h"
#endif

#ifndef _objRelation_h
#include "objRelation.h"
#endif

#include <ldr.h>

#ifndef _ldrTypes_h
#include "ldrTypes.h"
#endif

#ifndef _odt_class_member_sorter_h
#include "oodt_class_member_sorter.h"
#endif

#ifndef oodt_ui_decls_h
#include "oodt_ui_decls.h"
#endif

// This file describes all types (classes) of nodes in ldr trees.

// Relation types

define_relation(InputConnections, OutputConnections);
define_relation(exit_node,  exit_from);
define_relation(entry_node, entry_into);
define_relation(superclass_of, subclass_of);
define_relation(note_of_object, object_of_note);
define_relation(outgoing_relation_of_class, class_of_outgoing_relation);
define_relation(incoming_relation_of_class, class_of_incoming_relation);
define_relation(container_of_object, object_contained);
define_relation(invariant_of_class, class_of_invariant);
define_relation(responsibility_of_class, class_of_responsibility);
define_relation(ldrnode_of_ldrerdbox, ldrerdbox_of_ldrnode);

// Define all ldr node types - so we can symbolically reference
//  a node's class id.

enum ldrNodeTypes {
   ldrNodeType,
   ldrContainerNodeType,
   ldrSymbolNodeType,
   ldrConnectionNodeType,
   ldrClassNodeType,
   ldrMemberNodeType,
   ldrRelationNodeType,
   ldrFuncCallNodeType,
   ldrERDConnectionNodeType,
   ldrERDBoxNodeType
};


//=================================================
// ldrNode is the most general type.  graphlevel is
// the depth  of the node  in the tree.   It is set
// for the benefit of View.
class symbolPtr;
class ldrNode : public ldrTree {
   
 private:
   int     graphlevel;
   ldrRole role;
 protected:
   virtual void print_ldr(ostream& =cout, int level=0) const;
 public:
   ldrNode ();
#ifdef __GNUG__
   ldrNode(const ldrNode &ll) { *this = ll;}
#endif
   void    set_role(ldrRole new_role) ;
   ldrRole get_role() const;
   virtual int ldrIsA(ldrNodeTypes) const;
   void set_graphlevel(int Level);
   int  get_graphlevel() const;
   virtual void print(ostream& /*const*/=cout, int level=0) const;
   virtual void send_string(ostream& stream) const;
   virtual objArr* get_tok_list(int mode=0);

   declare_copy_member(ldrNode);
   define_relational(ldrNode,ldrTree);

   virtual int get_symbolPtr(symbolPtr*) const;

};

generate_descriptor(ldrNode,ldrTree);


//=================================================
// ldrSymbolNode corresponds  to "visible" nodes of
// ldr trees.  It does  not  include  the connector
// class, though, only "real"  nodes -- those which
// are  drawn as nodes.   In the application world,
// symbols usually  correspond to application nodes
// --  this  is not true   for   all  ldr nodes  --
// ldeConnectionNode

RelClass(ldrSymbolNode);

class ldrSymbolNode : public ldrNode {
   
 private:
   ldrSymbolType  symbol_type;
 public:
   ldrSymbolNode();
   ldrSymbolNode(ldrSymbolType);
#ifdef __GNUG__
   ldrSymbolNode(const ldrSymbolNode &ll) { *this = ll;}
#endif
   
   virtual int ldrIsA(ldrNodeTypes) const;
   void set_symbol_type(ldrSymbolType);
   ldrSymbolType  get_symbol_type() const;
   virtual objSet* get_output_connectors();
   virtual objSet* get_input_connectors();
   virtual ldrSymbolNodePtr get_entry_symbol();
   virtual ldrSymbolNodePtr get_exit_symbol();
   virtual void print(ostream& =cout, int level=0) const;
   virtual void send_string(ostream& stream) const;
   
   declare_copy_member(ldrSymbolNode);
   define_relational(ldrSymbolNode,ldrNode);
   virtual objArr* get_tok_list(int mode=0);
   virtual int get_symbolPtr(symbolPtr*) const;

};

generate_descriptor(ldrSymbolNode,ldrNode);


//=================================================//
// ldrConnectionNode usually corresponds to   lines
// on   diagrams.   In  the applications world,  it
// usually  corresponds  to relations       between
// application nodes.

class ldrConnectionNode : public ldrNode {
 private:
   ldrConnectorType symbol;
 public:
   ldrConnectionNode(ldrConnectorType con_type);
#ifdef __GNUG__
   ldrConnectionNode(const ldrConnectionNode &ll) { *this = ll;}
#endif
   virtual int ldrIsA(ldrNodeTypes) const;
   void set_symbol_type(ldrConnectorType);
   ldrConnectorType get_symbol_type();
   void add_input(ldrSymbolNodePtr);
   void add_output(ldrSymbolNodePtr);
   int is_output_p(ldrSymbolNodePtr) const;
   int is_input_p (ldrSymbolNodePtr) const;
   void remove_input(ldrSymbolNodePtr);
   void remove_output(ldrSymbolNodePtr);
   void replace_output(ldrSymbolNodePtr oldnode,
		       ldrSymbolNodePtr newnode);
   void replace_input(ldrSymbolNodePtr oldnode,
		       ldrSymbolNodePtr newnode);
   virtual objSet* get_output_symbols() const;
   virtual objSet* get_input_symbols() const;
   virtual void print(ostream& =cout, int level=0) const;
   
   declare_copy_member(ldrConnectionNode);
   define_relational(ldrConnectionNode, ldrNode);
};

generate_descriptor(ldrConnectionNode, ldrNode);


//=================================================
// Container nodes correspond to boxes on flowchart
// diagrams.   They    normally  do   not   have  a
// counterpart in the application world.

class ldrContainerNode : public ldrNode {
 public:
   ldrContainerNode();
#ifdef __GNUG__
   ldrContainerNode(const ldrContainerNode &ll) { *this = ll;}
#endif
   virtual int ldrIsA(ldrNodeTypes) const;
   virtual void print(ostream& =cout, int level=0) const;
   declare_copy_member(ldrContainerNode);
   define_relational(ldrContainerNode, ldrNode);
};

generate_descriptor(ldrContainerNode,ldrNode);


//========================================================
// Chart nodes are build for flowchart diagrams. They know
// and can manipulate their entry and exit nodes.

RelClass(ldrChartNode);

class ldrChartNode : public ldrSymbolNode {
 public:
   ldrChartNode(ldrSymbolType Sym) : ldrSymbolNode(Sym){}
#ifdef __GNUG__
   ldrChartNode(const ldrChartNode &ll) { *this = ll;}
#endif
   virtual ldrSymbolNodePtr get_entry_symbol();
   virtual ldrSymbolNodePtr get_exit_symbol();
   virtual void set_exit_symbol (ldrSymbolNodePtr);
   virtual void set_entry_symbol(ldrSymbolNodePtr);
   virtual void share_exit(ldrChartNodePtr Buddy); // Share this's entry/exit
   virtual void share_entry(ldrChartNodePtr Buddy);//  with Buddy
   virtual objSet* get_output_connectors();
   virtual objSet* get_input_connectors();
   virtual void print(ostream& =cout, int level=0) const;
   declare_copy_member(ldrChartNode);
   define_relational(ldrChartNode,ldrSymbolNode);
};

generate_descriptor(ldrChartNode,ldrSymbolNode);

//=======================================
class ldrHierarchyNode : public ldrNode {
 private:
   genString the_text;
   ldrSymbolType symbol_type;
 public:
   ldrHierarchyNode(ldrSymbolType symbol_type = ldrSOMETHING, char *st=NULL);
#ifdef __GNUG__
   ldrHierarchyNode(const ldrHierarchyNode &ll) { *this = ll;}
#endif
   void set_symbol_type (ldrSymbolType symbol);
   ldrSymbolType get_symbol_type ();
   char *get_the_text();
   void set_the_text(char *);
   virtual void print(ostream& =cout, int level=0) const;
   virtual void send_string(ostream&) const;
   declare_copy_member(ldrHierarchyNode);
   define_relational(ldrHierarchyNode, ldrNode);
};

generate_descriptor(ldrHierarchyNode, ldrNode);


//==========================================================================
// Class nodes represent OODT classes; they are called out specially because
// of the many relations with other objects they have, which could not be
// represented in the simpler ldr objects.

class ldrClassNode: public ldrSymbolNode {
public:
   define_relational(ldrClassNode, ldrSymbolNode);
#ifdef __GNUG__
   ldrClassNode(const ldrClassNode& ll) { *this = ll; }
#endif
   declare_copy_member(ldrClassNode);

   ldrClassNode(ldrSymbolType);

   virtual int ldrIsA(ldrNodeTypes) const;
   virtual objArr* get_tok_list(int mode=0);
   virtual objTreePtr get_root() const;
   virtual void set_appTree(appTreePtr);
   virtual void send_string(ostream& stream) const;

   const char* class_name();

   objSet* get_superclasses();
   void add_superclass(ldrNodePtr);

   objSet* get_subclasses();
   void add_subclass(ldrNodePtr);

   objSet* get_notes();
   void add_note(ldrNodePtr);

   objSet* get_invariants();
   void add_invariant(ldrNodePtr);

   objSet* get_containers();
   void add_container(ldrNodePtr);

   objSet* get_incoming_relations();
   void add_incoming_relation(ldrNodePtr);

   objSet* get_outgoing_relations();
   void add_outgoing_relation(ldrNodePtr);

   void set_inherited_members();
   bool has_inherited_members();

   void set_relations();
   bool has_relations();

private:
   char* name;
   bool inherited_members;
   bool relations;
};

generate_descriptor(ldrClassNode, ldrSymbolNode);


//===========================================================================
// Relation nodes represent relations between OODT classes.  This type exists
// mainly to provide its own implementation of get_tok_list and get_root.

class ldrRelationNode: public ldrSymbolNode {
public:
   define_relational(ldrRelationNode, ldrSymbolNode);
#ifdef __GNUG__
   ldrRelationNode(const ldrRelationNode& ll) { *this = ll; }
#endif
   declare_copy_member(ldrRelationNode);
   ldrRelationNode(ldrSymbolType);
   virtual int ldrIsA(ldrNodeTypes) const;
   virtual objArr* get_tok_list(int mode = 0);
   virtual objTreePtr get_root() const;
};

generate_descriptor(ldrRelationNode, ldrSymbolNode);


//===========================================================================
// Member nodes represent members (data, function, and type) of OODT classes.
// They contain their own textual representation (rather than just passing
// along the text from the app level) to allow for qualified naming of base
// class members when they appear in derived classes.  They also have
// attributes to allow for sorting and selective display.

enum member_attributes {
   INHERITED_MEMBER = 		0x0001,
   PACKAGE_PROT_MEMBER =	0x0002,
   FUNCTION_MEMBER = 		0x0004,
   DATA_MEMBER = 		0x0008,
   TYPE_MEMBER = 		0x0010,
   STATIC_MEMBER = 		0x0020,
   ORDINARY_MEMBER = 		0x0040,
   VIRTUAL_MEMBER = 		0x0080,
   PURE_VIRTUAL_MEMBER =	0x0100,
   PUBLIC_MEMBER = 		0x0200,
   PROTECTED_MEMBER = 		0x0400,
   PRIVATE_MEMBER = 		0x0800,
   FUNCTION_ARGUMENT =		0x1000,
   CONST_MEMBER =		0x2000,
   VOLATILE_MEMBER = 		0x4000,
   DOCUMENT_MEMBER =		0x8000,
   SYNCHRONIZED_MEMBER =       0x10000,
   NATIVE_MEMBER =             0x20000
};


RelClass(ddElement);
RelClass(steTextNode);
RelClass(steDocument);
RelClass(dd_or_xref_node);

class ldrMemberNode: public ldrSymbolNode {
public:
   ldrMemberNode(int, member_access_sort_types, dd_or_xref_nodePtr, const char* qual, bool strip_qual = true,
		 bool for_subsys = false);
   ldrMemberNode(ldrSymbolType, steTextNodePtr);
   ldrMemberNode(ldrSymbolType, steDocumentPtr);
#ifdef __GNUG__
   ldrMemberNode(const ldrMemberNode& ll) { *this = ll; }
#endif
   declare_copy_member(ldrMemberNode);
   define_relational(ldrMemberNode, ldrSymbolNode);

   virtual int ldrIsA(ldrNodeTypes) const;
   virtual void print(ostream& = cout, int level = 0) const;
   virtual void send_string(ostream& stream) const;
   virtual objArr* get_tok_list(int mode=0);
   virtual objTreePtr get_root() const;

   member_attributes get_attribs() const { return attribs; }

   const char* get_member_name() const { return (const char*) name; }

private:
   genString name;
   member_attributes attribs;
};

generate_descriptor(ldrMemberNode, ldrSymbolNode);



//===========================================================================
// ERD connection nodes are just like connection nodes except that they
// reflect attributes of relations: multiple labels, cardinality, and
// whether the relation is required to exist or not.

class ldrERDConnectionNode: public ldrConnectionNode {
public:
   define_relational(ldrERDConnectionNode, ldrConnectionNode);
   declare_copy_member(ldrERDConnectionNode);

   ldrERDConnectionNode(ldrConnectorType, const char* i_label,
         const char* o_label, const char* label, const char* clr_rsrc,
         rel_cardinality, bool required, bool arrows);
   ldrERDConnectionNode(const ldrERDConnectionNode&);

   virtual void print(ostream&, int) const;
   virtual int ldrIsA(ldrNodeTypes) const;

   const char* input_label() const;
   const char* output_label() const;
   const char* label() const;
   void append_to_label(const char*);
   const char* color_resource() const;
   rel_cardinality cardinality() const;
   bool required() const;
   bool do_arrows() const;

private:
   genString ilab;
   genString olab;
   genString lab;
   const char* clr;
   rel_cardinality card;
   bool req;
   bool arrow;
};

generate_descriptor(ldrERDConnectionNode, ldrConnectionNode);

//========================================================================
// ldrERDBoxNodes correspond to subsystems and aggregations on ERDs.  They
// contain other ldrNodes, including connectors.

class ldrERDBoxNode: public ldrSymbolNode {
public:
   ldrERDBoxNode(ldrSymbolType);
   define_relational(ldrERDBoxNode, ldrSymbolNode);
#ifdef __GNUG__
   ldrERDBoxNode (const ldrERDBoxNode & ll) { *this = ll; }
#endif
   declare_copy_member(ldrERDBoxNode);

   int ldrIsA(ldrNodeTypes) const;
   void print(ostream& = cout, int level = 0) const;
   void add_node(ldrNodePtr);
   objSet* get_contents();
};

generate_descriptor(ldrERDBoxNode, ldrSymbolNode);


//===========================================

// ldrFuncCallNodes correspond to Call Tree diagram nodes.

#define FUNCCALL_RECURSIVE 1
#define FUNCCALL_SYSTEM    2

class ldrFuncCallNode: public ldrSymbolNode 
{
 public:
   ldrFuncCallNode(ldrSymbolType, appTreePtr );
 private:
   ldrFuncCallNode(const ldrFuncCallNode& ll);// { *this = ll; }
 public:
   ldrFuncCallNode(ldrSymbolType, const char*);
   declare_copy_member(ldrFuncCallNode);
   define_relational(ldrFuncCallNode, ldrSymbolNode);

   virtual int ldrIsA(ldrNodeTypes) const;
   virtual void send_string(ostream& stream) const;
   virtual objArr* get_tok_list(int mode=0);

   const char* get_func_name() const { return (const char*) name; }

   void modif_arg(char *, int );
   void add_call_type(int call_type) { func_type |= call_type; }
   int  get_call_type(void) { return func_type; }
   void clear_call_type(int call_type) { func_type &= ~call_type; }

 private:
   genString name;
 public:
   int top_end;
   int bottom_end;
   int upper_tree;            // if =1 , belongs to upper part of 
                              // tree (parents) in Call Tree diagram

                              // this flag, if set, overwrites ldr_hdr flag
   int disp_arg;              // if = 0, no arguments displayed for C++ in call tree.
                              // if = 1, arguments displayed 
                              // if = -1 (default) - individual flag not set
   int func_type;
};

generate_descriptor(ldrFuncCallNode, ldrSymbolNode);

//=========================================================================
// The following class is identical to ldrNode except for a different
// implementation of get_tok_list: the one in ldrNode uses build_selection,
// which will throw an error if there is no corresponding appNode, while
// this one always returns NULL.  ldrNodeNoTokens should, therefore, be
// used for ldr tree roots and the like where there is no corresponding
// appNode.

class ldrNodeNoTokens: public ldrNode {
public:
   ldrNodeNoTokens() { }
 private:
   ldrNodeNoTokens(const ldrNodeNoTokens& other); //: ldrNode(other) { }
 public:
   define_relational(ldrNodeNoTokens, ldrNode);
   declare_copy_member(ldrNodeNoTokens);
   objArr* get_tok_list(int mode=0);
};

generate_descriptor(ldrNodeNoTokens, ldrNode);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrNode.h  $
   Revision 1.9 1998/09/18 11:32:57EDT Scott Turner (sturner) 
   adding support for Java attribute, "synchronized"
 * Revision 1.1  1994/09/21  18:43:57  jerry
 * Initial revision
 *
 * Revision 1.2.1.16  1993/12/16  13:47:50  azaparov
 * Bug track: 4418
 * Fixed bug 4418
 *
 * Revision 1.2.1.15  1993/10/05  13:28:56  builder
 * Reduce number of include files
 *
 * Revision 1.2.1.14  1993/07/12  18:39:03  aharlap
 * put #ifdef __GNUG__ around copy-constr
 *
 * Revision 1.2.1.13  1993/07/02  13:56:21  wmm
 * Fix bug 3810 by only extracting superclass members if requested by the
 * user, and then only for the selected classes.
 *
 * Revision 1.2.1.12  1993/06/21  19:36:54  wmm
 * Fix bugs 3768, 3718, and 2899.
 *
 * Revision 1.2.1.11  1993/05/25  13:57:27  sergey
 * Added disp_arg data field to ldrFuncCallNode class. Part of fix of bug #3319.
 *
 * Revision 1.2.1.10  1993/04/13  23:41:48  sergey
 * Added modif_arg method to ldrFUncCallNode. Part of bug #3305.
 *
 * Revision 1.2.1.9  1993/03/18  20:28:05  aharlap
 * cleanup for paraset compatibility
 *
 * Revision 1.2.1.8  1993/02/23  17:33:35  wmm
 * Allow class qualification to appear in subsystem map members.
 *
 * Revision 1.2.1.7  1993/02/17  21:22:07  kol
 * commented wrong const (in arg of print)
 *
 * Revision 1.2.1.6  1993/01/22  22:48:07  wmm
 * Support XREF-based ERDs and DCs.
 *
 * Revision 1.2.1.5  1993/01/14  15:56:03  jon
 * Added generate_descriptor() for ldrChartNode
 *
 * Revision 1.2.1.4  1992/12/21  23:09:23  aharlap
 * added constructor for ldrFuncCallNode
 *
 * Revision 1.2.1.3  1992/12/01  23:24:33  smit
 * Fixed bug#2074
 *
 * Revision 1.2.1.2  1992/10/09  19:50:22  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
