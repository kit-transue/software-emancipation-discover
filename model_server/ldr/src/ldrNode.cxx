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
//#include <iostream.h>
//#include <strstream.h>
#include <cLibraryFunctions.h>
#include "ldrNode.h"
#include "ldrSelection.h"
#include "genError.h"
#include "objRelation.h"
#include "objOper.h"
#include "objTree.h"
#include "ldrError.h"
#include "ldrHeader.h"
#include "steTextNode.h"
#include "steDocument.h"
#include "ddict.h"
#include <dd_or_xref_node.h>

static objTreePtr find_root_superclass(objTreePtr);
static objTreePtr find_root_subclass(objTreePtr);
ddElement* smt_get_dd(smtTree*);

int const BAD_LEVEL = -3;

init_relation( InputConnections, MANY, 0, OutputConnections, MANY, 0 );

init_relational(ldrNode,           ldrTree);
init_relational(ldrContainerNode,  ldrNode);
init_relational(ldrSymbolNode,     ldrNode);
init_relational(ldrConnectionNode, ldrNode);
init_relational(ldrChartNode,      ldrSymbolNode);
init_relational(ldrHierarchyNode,  ldrNode);
init_relational(ldrClassNode,      ldrSymbolNode);
init_relational(ldrRelationNode,   ldrSymbolNode);
init_relational(ldrMemberNode,     ldrSymbolNode);
init_relational(ldrFuncCallNode,   ldrSymbolNode);
init_relational(ldrERDConnectionNode, ldrConnectionNode);
init_relational(ldrERDBoxNode, 	   ldrSymbolNode);
init_relational(ldrNodeNoTokens,   ldrNode);

  implement_copy_member(ldrNode);
  implement_copy_member(ldrSymbolNode);
  implement_copy_member(ldrConnectionNode);
  implement_copy_member(ldrContainerNode);
  implement_copy_member(ldrChartNode);
  implement_copy_member(ldrHierarchyNode);
  implement_copy_member(ldrRelationNode);
  implement_copy_member(ldrMemberNode);
  implement_copy_member(ldrERDConnectionNode);
  implement_copy_member(ldrERDBoxNode);
  implement_copy_member(ldrFuncCallNode);
  implement_copy_member(ldrNodeNoTokens);
  implement_copy_member(ldrClassNode);


int ldrNode::get_symbolPtr(symbolPtr*) const
{
  return 0;
}

int ldrSymbolNode::get_symbolPtr(symbolPtr*symp) const
{
  appTree*node = get_appTree();
  if(!node) return 0;
  if(node->get_first()) return 0;
  *symp = node;
  return 1;
}

ldrNode::ldrNode()
{
   graphlevel = BAD_LEVEL;
   role       = ldrUNCLEAR;
}

ldrRole ldrNode::get_role() const
{
   return(role);
}

void ldrNode::set_role(ldrRole r)
{
   role = r;
}

void ldrNode::set_graphlevel(int level)
{
   graphlevel = level;
}

int ldrNode::get_graphlevel() const
{
   return graphlevel;
}

// General send_string for ldr_nodes delivers a string only when
// a corresponding application node exists.
objArr * ldrNode::get_tok_list(int mode)
 {
  Initialize(ldrNode::send_string);
  objArr *arr=NULL;
  appTreePtr app_node = get_appTree();
  if (app_node) 
    arr = app_node->get_tok_list(mode);
  return arr;
 }

void ldrNode::send_string(ostream& stream) const
{
   Initialize(ldrNode::send_string);
   appTreePtr app_node = get_appTree();
   if (app_node)
     app_node->send_string(stream);
}

//========================================================= ldrSymbolNode 

ldrSymbolNode::ldrSymbolNode() : ldrNode()
{
   symbol_type = ldrSOMETHING;
}

ldrSymbolNode::ldrSymbolNode(ldrSymbolType st) : ldrNode()
{
   symbol_type = st;
}

void ldrSymbolNode::set_symbol_type(ldrSymbolType st)
{
   symbol_type = st;
}

ldrSymbolType ldrSymbolNode::get_symbol_type() const
{
   return(symbol_type);
}

objSet* ldrSymbolNode::get_input_connectors()
{
   return((objSet*)get_relation(InputConnections, this));
}

objSet* ldrSymbolNode::get_output_connectors()
{
   return((objSet*) get_relation(OutputConnections, this));
}

ldrSymbolNodePtr ldrSymbolNode::get_entry_symbol()
{
   return(this);
}

ldrSymbolNodePtr ldrSymbolNode::get_exit_symbol()
{
   return(this);
}

// Symbol Nodes normally do not have a relation to an
// application node, but use that of their parent. It is
// an error if neither the symbol node nor its parent have
// an application node.

void ldrSymbolNode::send_string( ostream &stream ) const
{
   Initialize(ldrSymbolNode::send_string);

// special provision to send nothing for entry/exit Flowchart nodes

  ldrNodePtr ldr_parent = checked_cast(ldrNode, get_parent());
  if (ldr_parent != NULL && is_ldrChartNode(ldr_parent) == 1)  {
    ldrRole tmp_role = get_role();
    if (tmp_role == ldrENTRY || tmp_role == ldrEXIT ||
        tmp_role == ldrLAST_ELSE || tmp_role == ldrFAKE_LABEL ||
        tmp_role == ldrIF_YES )
      
      return;
  }

  appTreePtr app_node = get_appTree();
  if ( !app_node ){
    if(ldr_parent)
      ldr_parent->send_string(stream);
    return;
  }

  if(is_steTextNode(app_node)){
     steTextNodePtr txt = steTextNodePtr(app_node);
     if(txt->get_node_type() != steREG)
       app_node = txt->get_title();
  }

   app_node->send_string(stream);
 }

// General get_tok_list uses app get_tok_list
// MG: sinchronoized with send string

objArr * ldrSymbolNode::get_tok_list(int mode){
 Initialize(ldrSymbolNode::get_tok_list);
 objArr * tarr = NULL;

   ldrNodePtr ldr_parent = checked_cast(ldrNode, get_parent());
   if (ldr_parent != NULL && is_ldrChartNode(ldr_parent) == 1)  {
       ldrRole tmp_role = get_role();
       if (tmp_role == ldrENTRY || tmp_role == ldrEXIT ||
        tmp_role == ldrLAST_ELSE || tmp_role == ldrFAKE_LABEL ||
        tmp_role == ldrIF_YES )
 
           return NULL;
   }

   appTreePtr app_node = get_appTree();
   if ( !app_node ){
      if(ldr_parent)
          tarr = ldr_parent->get_tok_list(mode);
      return tarr;
   }

   if(is_steTextNode(app_node)){
     steTextNodePtr txt = steTextNodePtr(app_node);
     if(txt->get_node_type() != steREG)
       app_node = txt->get_title();
   }
   if(app_node)
     tarr = app_node->get_tok_list(mode);
   
 return tarr;
}



//======================================================== ldrConnectionNode


ldrConnectionNode::ldrConnectionNode(ldrConnectorType ct) : ldrNode()
{
   symbol = ct;
}


ldrConnectorType ldrConnectionNode::get_symbol_type()
{
   return(symbol);
}

void ldrConnectionNode::set_symbol_type(ldrConnectorType ct)
{
   symbol = ct;
}

void ldrConnectionNode::add_input(ldrSymbolNodePtr new_input)
{
   put_relation(InputConnections, this, new_input);
}

void ldrConnectionNode::add_output(ldrSymbolNodePtr new_output)
{
   put_relation(OutputConnections, this, new_output);
}

int ldrConnectionNode::is_input_p(ldrSymbolNodePtr snode) const
{
   return(test_relation(InputConnections, this, snode));
}

int ldrConnectionNode::is_output_p(ldrSymbolNodePtr snode) const
{
   return(test_relation(OutputConnections, this, snode));
}

void ldrConnectionNode::remove_output(ldrSymbolNodePtr old_output)
{
   rem_relation(OutputConnections, this, old_output);
}

void ldrConnectionNode::remove_input(ldrSymbolNodePtr old_output)
{
   rem_relation(InputConnections, this, old_output);
}

void ldrConnectionNode::replace_output(ldrSymbolNodePtr old_output,
				       ldrSymbolNodePtr new_output)
{
   remove_output(old_output);
   add_output(new_output);
}

void ldrConnectionNode::replace_input(ldrSymbolNodePtr old_input,
				       ldrSymbolNodePtr new_input)
{
   add_input(new_input);
   remove_input(old_input);
}

objSet* ldrConnectionNode::get_input_symbols() const
{
   return((objSet*) get_relation(InputConnections, this));
}

objSet* ldrConnectionNode::get_output_symbols() const
{
   return((objSet*) get_relation(OutputConnections, this));
}

//=========================================================== ldrContainerNode

ldrContainerNode::ldrContainerNode() : ldrNode()
{
// Do nothing -- just create ldrNode.
}

//============================================================== ldrChartNode:

init_relation(exit_node,  1, 0,  exit_from,  MANY, 0);
init_relation(entry_node, 1, 0,  entry_into, MANY, 0);

ldrSymbolNodePtr ldrChartNode::get_entry_symbol ()
{
   Initialize(ldrChartNode::get_entry_symbol);

   return(checked_cast(ldrSymbolNode, get_relation(entry_node, this)));
}

ldrSymbolNodePtr ldrChartNode::get_exit_symbol ()
{
   Initialize(ldrChartNode::get_exit_symbol);

   return(checked_cast(ldrSymbolNode, get_relation(exit_node, this)));
}


void ldrChartNode::set_entry_symbol (ldrSymbolNodePtr es)
{
   put_relation(entry_node, this, es);
}

void ldrChartNode::set_exit_symbol (ldrSymbolNodePtr es)
{
   put_relation(exit_node, this, es);
}


void ldrChartNode::share_exit(ldrChartNodePtr buddy)
{
   buddy->set_exit_symbol(get_exit_symbol());
}

void ldrChartNode::share_entry(ldrChartNodePtr buddy)
{
   buddy->set_entry_symbol(get_entry_symbol());
}


objSet* ldrChartNode::get_input_connectors()
{
   return((objSet*)get_relation(InputConnections,
				get_entry_symbol()));
}

objSet* ldrChartNode::get_output_connectors()
{
   return((objSet*)get_relation(OutputConnections,
				get_exit_symbol()));
}

//======================================================= ldrHierarchyNode:

ldrHierarchyNode::ldrHierarchyNode(ldrSymbolType symbol, char *st)
		 : the_text(st)
{
   set_symbol_type (symbol);
}

void ldrHierarchyNode::set_symbol_type(ldrSymbolType symbol)
{
   this->symbol_type = symbol;
}

ldrSymbolType ldrHierarchyNode::get_symbol_type()
{
   return this->symbol_type;
}

void ldrHierarchyNode::set_the_text(char* s) {
//   if ( the_text ) delete the_text;
   the_text = s;
}

char *ldrHierarchyNode::get_the_text() {
   return the_text;
}

// Hierarchy nodes which do not correspond to leaves of the
// tree should not send any strings even when the corresponding
// application node exists.

void ldrHierarchyNode::send_string( ostream &stream ) const
{
   Initialize(ldrHierarchyNode::send_string);

   if (the_text.length () > 0) {
     stream << (char *)the_text;
     return;
   }
	
   boolean have_subtree_p = 0;
   for(objTreePtr child = get_first(); child;  child = child->get_next()){
     if(ldrNodePtr(child)->get_appTree()){
       have_subtree_p = 1;
       break;
     }
   }
   appTreePtr app_node = get_appTree();
   if ( app_node ) {
     if ( is_steTextNode(app_node) ) {
       steTextNodePtr title_ptr = (checked_cast(steTextNode,app_node))->get_title();
       if (title_ptr)
	 title_ptr->send_string (stream);
     } else if (! have_subtree_p) {
       app_node->send_string(stream);
     }
   }
}

//=========================================================== ldrClassNode:

init_relation(superclass_of, MANY, 0, subclass_of, MANY, 0);
init_relation(note_of_object, MANY, 0, object_of_note, 1, 0);
init_relation(outgoing_relation_of_class, MANY, 0, class_of_outgoing_relation,
      1, 0);
init_relation(incoming_relation_of_class, MANY, 0, class_of_incoming_relation,
      1, 0);
init_relation(container_of_object, 1, 0,  object_contained, MANY, 0);
init_relation(invariant_of_class, 1, 0,  class_of_invariant, 1, 0);
init_relation(responsibility_of_class, 1, 0, class_of_responsibility, 1, 0);

ldrClassNode::ldrClassNode(ldrSymbolType sym): ldrSymbolNode(sym),
      name(NULL), inherited_members(false), relations(false) { }

objArr * ldrClassNode::get_tok_list(int)
  {
  Initialize(ldrClassNode::get_tok_list);
  // TEMPORARY to avoid trouble until change_name is in place:
  return NULL;
}

objTreePtr ldrClassNode::get_root() const {
   Initialize(ldrClassNode::get_root);

   objTreePtr root = objTree::get_root();
   objTreePtr super_root;
   objTreePtr sub_root;

   if (!root || checked_cast(ldr,tree_get_header(checked_cast(commonTree,root))))
      ReturnValue(root);

   if ((super_root = find_root_superclass(root)) != NULL)
      ReturnValue(super_root);

   if ((sub_root = find_root_subclass(root)) != NULL)
      ReturnValue(sub_root);

   ReturnValue(root);
}

void ldrClassNode::set_appTree(appTreePtr tp) {
   Initialize(ldrClassNode::set_appTree);

   delete name;
   char *at_name;
   at_name = tp->get_name();

   name = new char[strlen(at_name) + 1];
   strcpy(name, at_name);

   ldrSymbolNode::set_appTree(tp);
}

void ldrClassNode::send_string(ostream& os) const {
   Initialize(ldrClassNode::send_string);
   appTree* at = ldrtree_get_apptree(this);
   ldrNode::send_string(os);
}

const char* ldrClassNode::class_name() {
   return name;
}
   

objSet* ldrClassNode::get_superclasses() {
   return (objSet*) get_relation(superclass_of, this);
}

void ldrClassNode::add_superclass(ldrNodePtr sym) {
   put_relation(superclass_of, this, sym);
}

objSet* ldrClassNode::get_subclasses() {
   return (objSet*) get_relation(subclass_of, this);
}

void ldrClassNode::add_subclass(ldrNodePtr sym) {
   put_relation(subclass_of, this, sym);
}

objSet* ldrClassNode::get_notes() {
   return (objSet*) get_relation(note_of_object, this);
}

void ldrClassNode::add_note(ldrNodePtr sym) {
   put_relation(note_of_object, this, sym);
}

objSet* ldrClassNode::get_invariants() {
   return (objSet*) get_relation(invariant_of_class, this);
}

void ldrClassNode::add_invariant(ldrNodePtr sym) {
   put_relation(invariant_of_class, this, sym);
}

objSet* ldrClassNode::get_containers() {
   return (objSet*) get_relation(container_of_object, this);
}

void ldrClassNode::add_container(ldrNodePtr sym) {
   put_relation(container_of_object, this, sym);
}

objSet* ldrClassNode::get_incoming_relations() {
   return (objSet*) get_relation(incoming_relation_of_class, this);
}

void ldrClassNode::add_incoming_relation(ldrNodePtr sym) {
   put_relation(incoming_relation_of_class, this, sym);
}

objSet* ldrClassNode::get_outgoing_relations() {
   return (objSet*) get_relation(outgoing_relation_of_class, this);
}

void ldrClassNode::add_outgoing_relation(ldrNodePtr sym) {
   put_relation(outgoing_relation_of_class, this, sym);
}

void ldrClassNode::set_inherited_members() {
   Initialize(ldrClassNode::set_inherited_members);

   inherited_members = true;
}

void ldrClassNode::set_relations()
{
    Initialize (ldrClassNode::set_relations);

    relations = true;
}

bool ldrClassNode::has_inherited_members() {
   Initialize(ldrClassNode::has_inherited_members);

   return inherited_members;
}

bool ldrClassNode::has_relations()
{
    Initialize (ldrClassNode::has_relations);

    return relations;
}


//=========================================================== ldrMemberNode:

ldrMemberNode::ldrMemberNode(int sym_val, member_access_sort_types access_mod, dd_or_xref_nodePtr node,
			     const char* qual, bool strip_qual, bool for_subsys):
         ldrSymbolNode(ldrSymbolType(sym_val)) {
   ldrSymbolType sym = ldrSymbolType(sym_val);

   attribs = (strlen(qual)) ? INHERITED_MEMBER : member_attributes(0);
   if (access_mod == PUBLIC_MEMBERS)
      attribs = member_attributes(attribs | PUBLIC_MEMBER);
   else if (access_mod == PROTECTED_MEMBERS)
      attribs = member_attributes(attribs | PROTECTED_MEMBER);
   else if (access_mod == PACKAGE_PROT_MEMBERS)
      attribs = member_attributes(attribs | PACKAGE_PROT_MEMBER);
   else if (access_mod == PRIVATE_MEMBERS)
      attribs = member_attributes(attribs | PRIVATE_MEMBER);
   switch (sym) {
   case ldrPUBLIC_NESTED_TYPE:
   case ldrXREF_NESTED_TYPE:
      attribs = member_attributes(attribs | TYPE_MEMBER);
      break;

   case ldrPUBLIC_MEMBER:
   case ldrXREF_MEMBER:
      attribs = member_attributes(attribs | DATA_MEMBER);
      break;

   case ldrPUBLIC_METHOD:
   case ldrXREF_METHOD:
      attribs = member_attributes(attribs | FUNCTION_MEMBER);
      break;

   case ldrPUBLIC_METHOD_PARAMETER:
      attribs = member_attributes(attribs | FUNCTION_ARGUMENT);

   default:
      break;
   }

   if (!for_subsys) {
      ddElementPtr dd_member = node->get_dd_if_loaded();
      if (dd_member) {
	 if (dd_member->is_pure_virtual())
	    attribs = member_attributes(attribs | PURE_VIRTUAL_MEMBER);
	 else if (dd_member->is_virtual())
	    attribs = member_attributes(attribs | VIRTUAL_MEMBER);
	 else if (dd_member->is_static())
	    attribs = member_attributes(attribs | STATIC_MEMBER);
	 else attribs = member_attributes(attribs | ORDINARY_MEMBER);

	 if (dd_member->is_const())
	    attribs = member_attributes(attribs | CONST_MEMBER);
	 if (dd_member->is_volatile())
	    attribs = member_attributes(attribs | VOLATILE_MEMBER);
	 if (dd_member->is_synchronized())
	    attribs = member_attributes(attribs | SYNCHRONIZED_MEMBER);
	 if (dd_member->is_native())
	    attribs = member_attributes(attribs | NATIVE_MEMBER);
      }
      else {
	 symbolPtr xref_member = node->get_xrefSymbol();
	 if (xref_member->get_attribute(PVIR_ATT, 1)) {
	    attribs = member_attributes(attribs | PURE_VIRTUAL_MEMBER);
	 }
	 else if (xref_member->get_attribute(VIRT_ATT, 1)) {
	    attribs = member_attributes(attribs | VIRTUAL_MEMBER);
	 }
	 else if (xref_member->get_attribute(STAT_ATT, 1)) {
	    attribs = member_attributes(attribs | STATIC_MEMBER);
	 }
	 else attribs = member_attributes(attribs | ORDINARY_MEMBER);

	 if (xref_member->get_attribute(CNST_ATT, 1)) {
	    attribs = member_attributes(attribs | CONST_MEMBER);
	 }
	 // "volatile" is not in the pmod
    	 // sturner 980914: The preceding comment appears false.
	 if (xref_member->get_attribute(SYNC_ATT, 1)) {
	    attribs = member_attributes(attribs | SYNCHRONIZED_MEMBER);
	 }
	 if (xref_member->get_attribute(NATV_ATT, 1)) {
	    attribs = member_attributes(attribs | NATIVE_MEMBER);
	 }
      }
   }

   ostrstream os;
   node->send_string(os);
   os << ends;
   char* ddname = os.str();
   const char* after_qual = strstr(ddname, "::");
   if (after_qual && strip_qual)
      after_qual += 2;
   else after_qual = ddname;
   char* qualified_name = new char[strlen(qual) + strlen(after_qual) + 1];
   strcpy(qualified_name, qual);
   strcat(qualified_name, after_qual);
   char* paren = strchr(qualified_name, '(');
   if (paren)
      *paren = 0;
   name = qualified_name;
   delete [] qualified_name;
   delete [] ddname;

   ldrtree_put_apptree(this, node);
}

const int MAX_TN_STR_LEN = 32;

ldrMemberNode::ldrMemberNode(ldrSymbolType sym, steTextNodePtr tn):
      ldrSymbolNode(sym) {
   ostrstream os;
   tn->send_string(os);
   os << ends;
   char* tn_str = os.str();
   if (strlen(tn_str) > MAX_TN_STR_LEN) {
      tn_str[MAX_TN_STR_LEN - 2] = tn_str[MAX_TN_STR_LEN - 1] =
            tn_str[MAX_TN_STR_LEN] = '.';
      tn_str[MAX_TN_STR_LEN + 1] = 0;
   }
   name = tn_str;
   attribs = DOCUMENT_MEMBER;
   ldrtree_put_apptree(this, tn);
   delete [] tn_str;
}

ldrMemberNode::ldrMemberNode(ldrSymbolType sym, steDocumentPtr /* doc */):
      ldrSymbolNode(sym) {
}

void ldrMemberNode::send_string(ostream& os) const {
   os << (char*) name;
}

objArr * ldrMemberNode::get_tok_list(int)
  {
  Initialize(ldrMemberNode::get_tok_list);

  // TEMPORARY to avoid trouble until change_name is available:
 return (NULL);
}

objTreePtr ldrMemberNode::get_root() const {
   Initialize(ldrMemberNode::get_root);

   objTreePtr root = objTree::get_root();
   objTreePtr super_root;
   objTreePtr sub_root;

   if (!root || checked_cast(ldr,tree_get_header(checked_cast(commonTree,root))))
      ReturnValue(root);

   if ((super_root = find_root_superclass(root)) != NULL)
      ReturnValue(super_root);

   if ((sub_root = find_root_subclass(root)) != NULL)
      ReturnValue(sub_root);

   ReturnValue(root);
}

//=========================================================== ldrRelationNode:

ldrRelationNode::ldrRelationNode(ldrSymbolType sym): ldrSymbolNode(sym) { }

objArr* ldrRelationNode::get_tok_list(int) {
   return NULL;
}

objTreePtr ldrRelationNode::get_root() const {
   Initialize(ldrRelationNode::get_root);

   ldrClassNodePtr clp =
         checked_cast(ldrClassNode,get_relation(class_of_outgoing_relation, this));
   if (clp)
      ReturnValue(clp->get_root());

   clp = checked_cast(ldrClassNode,get_relation(class_of_incoming_relation, this));
   if (clp)
      ReturnValue(clp->get_root());

   ReturnValue(NULL);
}

//=================================================== ldrERDConnectionNode:

ldrERDConnectionNode::ldrERDConnectionNode(ldrConnectorType type,
      const char* i_label, const char* o_label, const char* label,
      const char* clr_rsrc, rel_cardinality crd, bool rq, bool arrows):
      ldrConnectionNode(type), ilab((char*) i_label), olab((char*) o_label),
      lab((char*) label), clr(clr_rsrc), card(crd), req(rq), arrow(arrows) { }

ldrERDConnectionNode::ldrERDConnectionNode(const ldrERDConnectionNode& other):
      ldrConnectionNode(other) {
   ilab = other.ilab;
   olab = other.olab;
   lab = other.lab;
   clr = other.clr;
   card = other.card;
   req = other.req;
   arrow = other.arrow;
}

const char* ldrERDConnectionNode::input_label() const {
   Initialize(ldrERDConnectionNode::input_label);

   ReturnValue((const char*) ilab);
}

const char* ldrERDConnectionNode::output_label() const {
   Initialize(ldrERDConnectionNode::output_label);

   ReturnValue((const char*) olab);
}

const char* ldrERDConnectionNode::label() const {
   Initialize(ldrERDConnectionNode::label);

   ReturnValue((const char*) lab);
}

void ldrERDConnectionNode::append_to_label(const char* nm) {
   Initialize(ldrERDConnectionNode::append_to_label);

   const char* cur_lab = lab;
   const char* sub = strstr(cur_lab, nm);
   size_t nm_len = strlen(nm);
   if (sub && (sub == cur_lab || (sub[-2] == ';' && sub[-1] == ' ')) &&
       (sub[nm_len] == 0 || sub[nm_len] == ';')) {
      return;		// already there
   }
   if (lab.length() > 0) {
      lab += "; ";
   }
   lab += nm;
}

const char* ldrERDConnectionNode::color_resource() const {
   Initialize(ldrERDConnectionNode::color_resource);

   ReturnValue(clr);
}

rel_cardinality ldrERDConnectionNode::cardinality() const {
   Initialize(ldrERDConnectionNode::cardinality);

   ReturnValue(card);
}

bool ldrERDConnectionNode::required() const {
   Initialize(ldrERDConnectionNode::required);

   ReturnValue(req);
}

bool ldrERDConnectionNode::do_arrows() const {
   Initialize(ldrERDConnectionNode::do_arrows);

   ReturnValue(arrow);
}

//============================================================ ldrERDBoxNode:

init_relation(ldrnode_of_ldrerdbox, MANY, 0, ldrerdbox_of_ldrnode, MANY, 0);

ldrERDBoxNode::ldrERDBoxNode(ldrSymbolType tp): ldrSymbolNode(tp) { }

void ldrERDBoxNode::add_node(ldrNodePtr node) {
   Initialize(ldrERDBoxNode::add_node);

   if (node)
      put_relation(ldrnode_of_ldrerdbox, this, node);

   Return
}

objSet* ldrERDBoxNode::get_contents() {
   Initialize(ldrERDBoxNode::get_contents);

   ReturnValue((objSet*) get_relation(ldrnode_of_ldrerdbox, this));
}

//=========================================================== ldrNodeNoTokens:

objArr* ldrNodeNoTokens::get_tok_list(int) {
   return NULL;
}

//=========================================================== Print Methods:

extern void gen_print_indent (ostream&, int);

void ldrNode::print_ldr(ostream& ostr, int ) const
{
   ostr << "(" << graphlevel << ")\"";
   send_string(ostr);
   ostr << "\"";
}

void ldrNode::print(ostream& ostr, int level) const
{
//   gen_print_indent(ostr, level);
   print_ldr(ostr,level);
}

void ldrContainerNode::print(ostream& ostr, int level) const
{
//   gen_print_indent(ostr, level);
   print_ldr(ostr,level);
}

void ldrSymbolNode::print(ostream& ostr, int level) const
{
//   gen_print_indent(ostr, level);
   print_ldr(ostr,level);
   ostr << "<" << (int)symbol_type << ">";
}

void ldrConnectionNode::print(ostream& ostr, int /*level*/) const
{
   Initialize(ldrConnectionNode::print);

   Obj* Outputs = get_output_symbols();
   Obj* Inputs  = get_input_symbols();
   int no_inputs = (Inputs != NULL) ? Inputs->size() : 0;
   int no_outputs =  (Outputs != NULL) ? Outputs->size() : 0;
   ostr << "Connection i/o " << no_inputs << '/' << no_outputs << " ";
}

void ldrChartNode::print(ostream& ostr, int level) const
{
   print_ldr(ostr,level);
}

void ldrHierarchyNode::print(ostream& ostr, int level) const
{
   gen_print_indent(ostr, level);
   print_ldr(ostr,level);
}

void ldrMemberNode::print(ostream& ostr, int level) const {
   ldrSymbolNode::print(ostr, level);
   ostr << ' ';
   if (attribs & INHERITED_MEMBER)
      ostr << "[INHERITED]";
   if (attribs & FUNCTION_MEMBER)
      ostr << "[FUNCTION]";
   if (attribs & DATA_MEMBER)
      ostr << "[DATA]";
   if (attribs & TYPE_MEMBER)
      ostr << "[TYPE]";
   if (attribs & STATIC_MEMBER)
      ostr << "[STATIC]";
   if (attribs & ORDINARY_MEMBER)
      ostr << "[ORDINARY]";
   if (attribs & VIRTUAL_MEMBER)
      ostr << "[VIRTUAL]";
   if (attribs & PURE_VIRTUAL_MEMBER)
      ostr << "[PURE]";
   if (attribs & PUBLIC_MEMBER)
      ostr << "[PUBLIC]";
   if (attribs & PROTECTED_MEMBER)
      ostr << "[PROTECTED]";
   if (attribs & PACKAGE_PROT_MEMBER)
      ostr << "[PACKAGE]";
   if (attribs & PRIVATE_MEMBER)
      ostr << "[PRIVATE]";
   if (attribs & FUNCTION_ARGUMENT)
      ostr << "[ARGUMENT]";
}

void ldrERDConnectionNode::print(ostream& ostr, int level) const {
   ldrConnectionNode::print(ostr, level);
   ostr << " Relation " << (const char*) ilab << "/" << (const char*) lab <<
         "/" << (const char*) olab ;
}

void ldrERDBoxNode::print(ostream& ostr, int level) const {
   ldrSymbolNode::print(ostr, level);
}

//=========================================================== ldrIsA methods:

int ldrSymbolNode::ldrIsA(ldrNodeTypes id) const
{
   return ldrSymbolNodeType == id;
}

int ldrConnectionNode::ldrIsA( ldrNodeTypes id ) const
{
   return ldrConnectionNodeType == id;
}

ldrContainerNode::ldrIsA( ldrNodeTypes id ) const
{
   return ldrContainerNodeType == id;
}

int ldrNode::ldrIsA( ldrNodeTypes id ) const
{
    return ldrNodeType == id;
}

int ldrClassNode::ldrIsA(ldrNodeTypes type) const {
   return type == ldrClassNodeType;
}

int ldrMemberNode::ldrIsA(ldrNodeTypes type) const {
   return type == ldrMemberNodeType;
}

int ldrRelationNode::ldrIsA(ldrNodeTypes type) const {
   return type == ldrRelationNodeType;
}

int ldrERDConnectionNode::ldrIsA(ldrNodeTypes type) const {
   return type == ldrERDConnectionNodeType;
}

int ldrERDBoxNode::ldrIsA(ldrNodeTypes type) const {
   return type == ldrERDBoxNodeType;
}

//========== helper routines for OODT get_root() overrides =================

static objTreePtr find_root_superclass(objTreePtr cls) {
   Initialize(find_root_superclass);

   objTreePtr root = NULL;
   Obj* curr;
   objSet superclasses;
   superclasses = get_relation(superclass_of, cls);

   ForEach(curr, superclasses) {
      objTreePtr trial_root = checked_cast(objTree,curr)->objTree::get_root();
      if (checked_cast(ldr,tree_get_header(checked_cast(commonTree,trial_root))))
         root = trial_root;
      else root = find_root_superclass(checked_cast(objTree,curr));
      if (root)
         break;
   }

   ReturnValue(root);
}

static objTreePtr find_root_subclass(objTreePtr cls) {
   Initialize(find_root_subclass);

   objTreePtr root = NULL;
   Obj* curr;
   objSet subclasses;
   subclasses = get_relation(subclass_of, cls);

   ForEach(curr, subclasses) {
      objTreePtr trial_root = checked_cast(objTree,curr)->objTree::get_root();
      if (checked_cast(ldr,tree_get_header(checked_cast(commonTree,trial_root))))
         root = trial_root;
      else root = find_root_subclass(checked_cast(objTree,curr));
      if (root)
         break;
   }

   ReturnValue(root);
}


/*
   START-LOG-------------------------------------------

   $Log: ldrNode.cxx  $
   Revision 1.16 2000/11/29 11:39:42EST ktrans 
   Merge from bug20022: remove subsystems
 * Revision 1.1  1994/09/21  18:44:21  jerry
 * Initial revision
 *
Revision 1.2.1.17  1994/07/20  23:21:03  mg
Bug track: NA
cleanup

Revision 1.2.1.16  1994/03/05  14:41:44  trung
Bug track: 0
fixes for prop to subsys, notify rtl, restoring subsys

Revision 1.2.1.15  1993/07/02  13:56:52  wmm
Fix bug 3810 by only extracting superclass members if requested by the
user, and then only for the selected classes.

Revision 1.2.1.14  1993/06/23  16:56:22  bakshi
brackets on var decl inside case for c++3.0.1 comformity

Revision 1.2.1.13  1993/06/21  19:37:10  wmm
Fix bugs 3768, 3718, and 2899.

Revision 1.2.1.12  1993/04/22  19:02:49  smit
fix bug#3474

Revision 1.2.1.11  1993/03/18  20:24:39  aharlap
cleanup for paraset compatibility

Revision 1.2.1.10  1993/02/23  17:34:03  wmm
Allow class qualification to appear in subsystem map members.

Revision 1.2.1.9  1993/02/09  18:21:45  wmm
Clean up a few places that should have handled XREF-based ldrNodes.

Revision 1.2.1.8  1993/01/24  01:15:34  wmm
Support XREF-based class browser view.

Revision 1.2.1.7  1993/01/22  22:48:28  wmm
Support XREF-based ERDs and DCs.

Revision 1.2.1.6  1992/12/01  23:24:10  smit
Fixed bug #2074

Revision 1.2.1.5  1992/11/23  22:48:58  glenn
include ldrSelection.h

Revision 1.2.1.4  1992/11/23  19:29:58  aharlap
fixed warnings

Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:53  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
