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
////////////////////////// ddict.h.C ////////////////////////////
//
// Synopsis:
//   These are the methods used in the data dictionary classes in 
//   memory.
//
// dictionary:
//
// created: 12/02/91    John M Cameron
//
// bugs:
//
#include "ddict.h"

#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <ctype.h>
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <cctype>
#include <iostream>
#endif /* ISO_CPP_HEADERS */

#include "RTL_externs.h"
#include "astTree.h"
#include "ast_cplus_tree.h"
#include "cLibraryFunctions.h"
#include "ddHeader.h"
#include "driver_mode.h"
#include "groupHdr.h"
#include "machdep.h"
#include "messages.h"
#include "msg.h"
#include "objRelation.h"
#include "oodt_relation.h"
#include "path.h"
#include "transaction.h"
#include "xref.h"

#ifndef TYPE_READONLY
#define TYPE_READONLY(NODE) ((NODE)->readonly_flag)
#endif
#ifndef TYPE_VOLATILE
#define TYPE_VOLATILE(NODE) ((NODE)->volatile_flag)
#endif

init_relational(ddRoot,ddNode);  
init_relational(ddElement,ddSymbol);
//init_relational(myTreeObj, Relational);
init_relation(id_of_decl, 1, relationMode::MW, decl_of_id, 1, relationMode::MW);

// -- NESTED:
init_relation(scope_of_entity,1,NULL,entity_of_scope,MANY,NULL);

smtTree* dd_get_any_smt_decl(ddElement*);
extern int dd_is_structure(ddKind);
extern int dd_is_scoped_entity(ddKind);
int strcmp_name(ddKind, char const *, char const *);
void handle_template_of_instance(Relational*,Relational*,int);
extern void handle_scope_of_entity(ddElement*, ddElement*, int);
extern void handle_property_of_possessor(ddElementPtr, ddElementPtr, int);

extern int xref_hook_turn_off;
int not_call;
objSet auto_rels; // set of programmatic relations

int ddElement::get_related_tokens(objSet& os)
{
  os |= get_relation(ref_smt_of_dd, this);
  os |= get_relation(cd_of_structure, this);
  return os.size();
}

// also put in for xrefsymbol
void ddElement::set_datatype(int dt)
{
  Initialize(ddElement::set_datatype);
  datatype = dt;
  if (not_call)
    return;
  if (xref_hook_turn_off) return;
  fsymbolPtr sym = get_xrefSymbol();
  if (sym.xrisnotnull())
    sym.set_datatype(dt);
}

void remove_dd_el(ddElement* ep)
{
  Initialize(remove_dd_el);

  if (!ep || !ep->get_ddname()) return;

  if (ep->get_id() < 1) return;
  ddRoot* dr = ep->get_dd_root();
  start_transaction() {      
    dr->remove_hash(ep, ep->get_ddname(), ep->get_kind());
    ep->set_local_hash(0);
    obj_delete(ep);
  }end_transaction();
}

char *strsav(char const *s)
{
  char *tmp = new char[strlen(s)+1];
  strcpy(tmp, s);
  return tmp;
}

init_rel_or_ptr(appTree,assocnode,0,ddElement,dd,relationMode::C);
init_rel_or_ptr(appTree,def_assocnode,0,ddElement,dd,relationMode::C);
init_rel_or_ptr(appTree,assoc_member,relationMode::MW,ddElement,dd,relationMode::C);
init_rel_or_ptr(appTree,app,relationMode::MW,ddElement,soft_assoc,relationMode::C);
init_rel_or_ptr(appTree,assoc_instance,0,ddElement,dd,relationMode::C);
init_rel_or_ptr(ddElement,instance,NULL,ddElement,assocType,NULL);
  
//----------------------------------------------------------------------------

class init_assoc_member {
public:
    init_assoc_member() {
 	assoc_member_of_dd->put_assoc_type(HARD_ASSOC);
 	dd_of_assoc_member->put_assoc_type(HARD_ASSOC);
    }
} init_assoc_member_instance;

init_relation(superclassof,MANY,relationMode::W,subclassof,MANY,relationMode::DW); // ddElement -- ddElement

// should friend be propagated, NULL or M?
  init_relation(has_friend,MANY,NULL,friend_of,MANY,NULL);

/* int f1() {f2();} int f2(){}
  f1 is using f2 and f2 is used by f1, from decl to decl
*/
  init_rel_or_ptr(appTree,user,NULL,appTree,used,NULL);
init_rel_or_ptr(app,appHeader,NULL,appTree,dd,NULL);

  init_relation(struct_ref_of_decl,MANY,relationMode::W,struct_decl_of_ref,MANY,relationMode::W);
  init_relation(vpub_baseclassof,MANY,relationMode::W,vpub_subclassof,MANY,relationMode::W);
  init_relation(pub_baseclassof,MANY,relationMode::W,pub_subclassof,MANY,relationMode::W);
  init_relation(vpri_baseclassof,MANY,relationMode::W,vpri_subclassof,MANY,relationMode::W);
  init_relation(pri_baseclassof,MANY,relationMode::W,pri_subclassof,MANY,relationMode::W);
  init_relation(vpro_baseclassof,MANY,relationMode::W,vpro_subclassof,MANY,relationMode::W);
  init_relation(pro_baseclassof,MANY,relationMode::W,pro_subclassof,MANY,relationMode::W);
  init_relation(smt_to_dd,1,relationMode::DMW,dd_to_smt,1,relationMode::MW);      // ddElement -- smtTree 
  init_relation(smt_header_to_dd_header,1,NULL,dd_header_to_smt_header,MANY,NULL);
  init_relation(smt_header_to_dd,1,NULL,dd_to_smt_header,1,NULL);
                                                       // ddheader -- smtheader
  init_relation(smt_tok_of,1,NULL,dd_elt_of,1,NULL); //ddToken--ddElement;
  init_relation(defined_in,1,NULL,elements_of,MANY,relationMode::D); //ddToken--ddElement;
  init_relation(template_of_instance,1,NULL,instance_of_template,MANY,NULL);
  init_relation(property_of_possessor,MANY,NULL,possessor_of_property,MANY,NULL);

  init_relation(smtHeader_of_ddRoot,1,NULL,ddRoot_of_smtHeader,1,relationMode::D);
  init_relation(cdtree_of_dd,1,NULL,dd_of_cdtree,1,NULL);



  init_relation(def_smt_of_dd, 1, relationMode::MW, def_dd_of_smt, 1, relationMode::MW);
  init_relation(smt_of_dd, MANY, relationMode::MW, dd_of_smt, 1, relationMode::W);
  init_relation(ref_smt_of_dd, MANY, relationMode::MW, ref_dd_of_smt, 1, relationMode::W);
  init_relation(decl_smt_of_dd, MANY, relationMode::MW, decl_dd_of_smt, 1, relationMode::W);

  init_relation(ref_app_of_dd, MANY, relationMode::MW, ref_dd_of_app, 1, relationMode::W);

  init_relation(macro_of_ddRoot, 1, NULL, ddRoot_of_macro, 1, NULL);
  init_relation(fdecl_of_ddRoot, 1, NULL, ddRoot_of_fdecl, 1, NULL);
  init_relation(vdecl_of_ddRoot, 1, NULL, ddRoot_of_vdecl, 1, NULL);
  init_relation(tdecl_of_ddRoot, 1, NULL, ddRoot_of_tdecl, 1, NULL);

  init_relation(xref_header_of_dd_header,1,0,dd_header_of_xref_header,1,relationMode::W);

  // create relationship between an SMT_decl node and its
  // semantic and syntactic type strings in the dd. the
  // semantic type string is derived from the AST while walking the
  // completed SMT. The syntactic type string is not yet implemented
  // (12/30/92), but is planned to be derived from the source tokens
  // of the declaration in the SMT, so the syntype/smt_decl relation
  // is not yet used.

  init_rel_or_ptr(appTree,smt_decl,NULL,ddElement,semtype,NULL);
  init_rel_or_ptr(appTree,smt_decl,NULL,ddElement,syntype,NULL);

  // relation between a typename for a structure (eg, class, struct),
  // and the structure itself. used between the typename smt token
  // and the smt cdecl node of the structure.

  init_rel_or_ptr(smtTree,typenm,relationMode::M,smtTree,classorstruct,NULL);
  init_rel_or_ptr(smtTree,cd,relationMode::M,ddElement,structure,relationMode::M);
  init_rel_or_ptr(ddElement,propagate,relationMode::M,ddElement,src,NULL);

  init_rel_or_ptr(ddElement,dd,NULL,appTree,ref_file,NULL);

static void put_dd_ref_file(ddElement *dd)
{
  Initialize(put_dd_ref_file);

  if (!dd) return;
  Obj *ref = dd_get_ref_file(dd);
  if (ref) return;
  app * ah = dd->get_main_header();
  if (!ah || !is_smtHeader(ah)) return;
  appTree *smt = checked_cast(appTree, ah->get_root());

  if (smt)
    dd_put_ref_file(dd, smt);
}

static app* finish_constructor(ddElement* dd, ddKind k, int xref_type, app*h, appTree*node, int hook_off)
{
    //boris 02/05/97. Fixed crash on HP for Denise
    if (!h && node && is_app(node))
      h = (app *)node;

    if (!h)
      h = node->get_main_header();
    if (!h)
      h = node->get_header();
    if(!h) 
      return h;

    ddRoot* dr = dd_sh_get_dr(h);
    dr->insert(dd, k);
    dr->insert_hash(dd);
    if (xref_type) 
      dd->set_def_file_with_good_name(h->get_filename());

    if(!hook_off)
      Xref_insert_symbol(dd, h);

    if (is_smtHeader(h)){
       dd->language = h->language;
    }

   return h;
}

ddElement::ddElement(ddKind k, char const *ns, int nl,
		     char const *, int , char const *as, int /*al*/)
{
    Initialize(ddElement::ddElement(ddKind, char const *, int, char const *, int, char const *, int));

    language = smt_UNKNOWN;
    int ll = strlen(as);
    aname = new char[ll+1]; strncpy(aname, as, ll); aname[ll] = 0;
    name = new char[nl+1]; strncpy(name, ns, nl); name[nl] = 0;
    def = 0;
    sname = NULL;
    sdef = NULL;
    def_file = NULL;
    last_field = checked_cast(ddElement, NULL);
    tid = checked_cast(smtTree, NULL);
#ifndef _WIN32
    struct_rel_type = UNKNOWN;
#else
    struct_rel_type = __UNKNOWN;
#endif
    kind= k;
    local_index = -1;
    xref_index = -1;
    local_hash = NULL;
    xref_hash = NULL;
    record= 0;
    enumerated= 0;
    private_member= 0;
    protected_member= 0;
    virtual_attr= 0;
    static_attr= 0;
    const_attr= 0;
    mangled_attr= 0;
    obsolete= 0;
    expect_friend = 0;
    pure_virtual_attr= 0;
    volatile_attr = 0;
    inline_attr= 0;
    extern_def = 0;
    is_def = 0;
    is_old = 0;
    cons_des = 0;
    kr = 0;
    drn = 1;
    from_method = 0;
    native_attr = 0;
    synchronized_attr = 0;
    changed_since_saved = 0;
    changed_since_checked_in = 0;
    // The next bit will be set to 1 when it is checked_in
    //  only when this ddElement belong to
    // a header file and it is changed. NOT WHEN ANY FILE IS CHECKED IN
    file_not_checked_in = 0;
    filler = 0;
    datatype = 0;

    ast_node=NULL;

    old_name = strsav(name);
}

ddElement::ddElement(char const *ns, appTree* node, int xref_type, ddKind k)
{
    Initialize(ddElement::ddElement(char const *,appTree*,int,ddKind));

    language = smt_UNKNOWN;
    int ll = strlen(ns);
    aname = new char[ll+1]; strncpy(aname, ns, ll); aname[ll] = 0;
    int nl = ll;
    name = new char[nl+1]; strncpy(name, ns, nl); name[nl] = 0;
    def = 0;
    sname = NULL;
    sdef = NULL;
    def_file = NULL;
    last_field = checked_cast(ddElement, NULL);
    tid = checked_cast(smtTree, NULL);
#ifndef _WIN32
    struct_rel_type = UNKNOWN;
#else
    struct_rel_type = __UNKNOWN;
#endif
    kind= k;
    local_index = -1;
    xref_index = -1;
    local_hash = NULL;
    xref_hash = NULL;
    record= 0;
    enumerated= 0;
    private_member= 0;
    protected_member= 0;
    virtual_attr= 0;
    static_attr= 0;
    const_attr= 0;
    mangled_attr= 0;
    obsolete= 0;
    expect_friend = 0;
    pure_virtual_attr= 0;
    volatile_attr = 0;
    inline_attr= 0;
    extern_def = 0;
    is_def = xref_type;
    is_old = 0;
    cons_des = 0;
    kr = 0;
    drn = 1;
    from_method = 0;
    native_attr = 0;
    synchronized_attr = 0;
    changed_since_saved = 0;
    changed_since_checked_in = 0;
    file_not_checked_in = 0;
    filler = 0;
    datatype = 0;

    ast_node=NULL;

    old_name = strsav(name);

    finish_constructor(this, k, xref_type, 0, node, 0);

    if (xref_type)
      def_assocnode_put_dd (node, this);
    else
      assocnode_put_dd (node, this);
}
 
ddElement::ddElement(char const *ns, appTree* node, int xref_type, 
	               ddKind k, RelType* rel, char const *as_name)
{
    Initialize(ddElement::ddElement(char const *,appTree*,int,ddKind,RelType*, char const *));

    language = smt_UNKNOWN;
    int ll = strlen(ns);
    if (as_name) {
	aname = new char[strlen(as_name)+1]; strcpy (aname, as_name);
    } else {
        aname = new char[ll+1]; strncpy(aname, ns, ll); aname[ll] = 0;
    }
    int nl = ll;
    name = new char[nl+1]; strncpy(name, ns, nl); name[nl] = 0;
    def = 0;
    sname = NULL;
    sdef = NULL;
    def_file = NULL;
    last_field = checked_cast(ddElement, NULL);
    tid = checked_cast(smtTree, NULL);
#ifndef _WIN32
    struct_rel_type = UNKNOWN;
#else
    struct_rel_type = __UNKNOWN;
#endif
    kind= k;
    local_index = -1;
    xref_index = -1;
    local_hash = NULL;
    xref_hash = NULL;
    record= 0;
    enumerated= 0;
    private_member= 0;
    protected_member= 0;
    virtual_attr= 0;
    static_attr= 0;
    const_attr= 0;
    mangled_attr= 0;
    obsolete= 0;
    expect_friend = 0;
    pure_virtual_attr= 0;
    volatile_attr = 0;
    inline_attr= 0;
    extern_def = 0;
    is_def = xref_type;
    is_old = 0;
    cons_des = 0;
    kr = 0;
    drn = 1;
    from_method = 0;
    native_attr = 0;
    synchronized_attr = 0;
    changed_since_saved = 0;
    changed_since_checked_in = 0;
    file_not_checked_in = 0;
    filler = 0;
    datatype = 0;

    ast_node=NULL;

    old_name = strsav(name);

    finish_constructor(this, k, xref_type, 0, node, 0);

    if (rel)
      put_relation(rel, node, this);
}

ddElement::ddElement(char const *ns, app* node, int xref_type, 
	               ddKind k, RelType* rel, char const *as_name)
{
    Initialize(ddElement::ddElement(char const *,app*, int, ddKind, RelType*, char const *));

    language = smt_UNKNOWN;
    int ll = strlen(ns);
    if (as_name) {
	aname = new char[strlen(as_name)+1]; strcpy (aname, as_name);
    } else {
        aname = new char[ll+1]; strncpy(aname, ns, ll); aname[ll] = 0;
    }
    int nl = ll;
    name = new char[nl+1]; strncpy(name, ns, nl); name[nl] = 0;
    def = 0;
    sname = NULL;
    sdef = NULL;
    def_file = NULL;
    last_field = checked_cast(ddElement, NULL);
    tid = checked_cast(smtTree, NULL);
#ifndef _WIN32
    struct_rel_type = UNKNOWN;
#else
    struct_rel_type = __UNKNOWN;
#endif
    kind= k;
    local_index = -1;
    xref_index = -1;
    local_hash = NULL;
    xref_hash = NULL;
    record= 0;
    enumerated= 0;
    private_member= 0;
    protected_member= 0;
    virtual_attr= 0;
    static_attr= 0;
    const_attr= 0;
    mangled_attr= 0;
    obsolete= 0;
    expect_friend = 0;
    pure_virtual_attr= 0;
    volatile_attr = 0;
    inline_attr= 0;
    extern_def = 0;
    is_def = xref_type;
    is_old = 0;
    cons_des = 0;
    kr = 0;
    drn = 1;
    from_method = 0;
    native_attr = 0;
    synchronized_attr = 0;
    changed_since_saved = 0;
    changed_since_checked_in = 0;
    file_not_checked_in = 0;
    filler = 0;
    datatype = 0;

    ast_node=NULL;

    old_name = strsav(name);

    finish_constructor(this, k, xref_type, node, 0, 0);

    if(rel)
      put_relation(rel, node, this);
}
  
static char *clone_str(char const *str) {
   if (str) {
      size_t len = strlen(str) + 1;
      char *new_str = new char[len];
      strcpy(new_str, str);
      return new_str;
   }

   char *null_str = new char;
   *null_str = 0;
   return null_str;
}

ddElement::ddElement(ddElementPtr org, appTree* node)
{
    Initialize(ddElement::ddElement(ddElementPtr,appTree*));

    language = org->language;

    aname = clone_str(org->aname);
    name = clone_str(org->name);
    def = NULL;		// Nobody looks at ddElement::def any more
    sname = NULL;
    sdef = NULL;
    app* h = node->get_main_header();
    if (!h) {
       h = (is_appTree(node)) ? node->get_header() : checked_cast(app, node);
    }

    def_file = clone_str(//(h && !is_oodt_relationHeader(h)) ?
			 //h->get_filename() :
			 org->get_def_file());
    last_field =  NULL;
    tid =  NULL;
#ifndef _WIN32
    struct_rel_type = UNKNOWN;
#else
    struct_rel_type = __UNKNOWN;
#endif
    kind= org->kind;
    local_index = -1;
    xref_index = -1;
    local_hash = NULL;
    xref_hash = NULL;
    record= org->record;
    enumerated= org->enumerated;
    private_member= org->private_member;
    protected_member= org->protected_member;
    virtual_attr= org->virtual_attr;
    static_attr= org->static_attr;
    const_attr= org->const_attr;
    mangled_attr= org->mangled_attr;
    obsolete= 0;
    expect_friend = 0;
    pure_virtual_attr= org->pure_virtual_attr;
    volatile_attr = org->volatile_attr;
    inline_attr= org->inline_attr;
    extern_def = org->extern_def;
    is_def = 0;
    is_old = 0;
    cons_des = 0;
    kr = 0;
    drn = 1;
    from_method = 0;
    native_attr = org->native_attr;
    synchronized_attr = org->synchronized_attr;
    changed_since_saved = 0;
    changed_since_checked_in = 0;
    file_not_checked_in = 0;
    filler = 0;
    datatype = 1;	// flag "cloned" reference ddElement
    ast_node= NULL;
    old_name = strsav(name);

    finish_constructor(this, kind, is_def, h, node, xref_hook_turn_off);

    if (is_smtHeader(h))
      put_dd_ref_file(this);
    else
      assocnode_put_dd (node, this);
}


ddElement::ddElement(symbolPtr any_sym, appTree* node)
{
    Initialize(ddElement::ddElement(symbolPtr,appTree*));
    symbolPtr sym = any_sym.get_xrefSymbol();
    Assert(sym.xrisnotnull());

    language = smtLanguage(sym.get_language());

    aname = clone_str(sym.get_name());
    name = clone_str(sym.get_name());
    def = NULL;		// Nobody looks at ddElement::def any more
    sname = NULL;
    sdef = NULL;
    symbolPtr def_file_sym = sym->get_def_file();
    if (def_file_sym.xrisnotnull()) {
       def_file = clone_str(def_file_sym.get_name());
    }
    else def_file = NULL;
    last_field =  NULL;
    tid =  NULL;
#ifndef _WIN32
    struct_rel_type = UNKNOWN;
#else
    struct_rel_type = __UNKNOWN;
#endif
    kind= sym->get_kind();
    local_index = -1;
    xref_index = -1;
    local_hash = NULL;
    xref_hash = NULL;
    record= 0;
    enumerated= 0;
    private_member= sym->get_attribute(PRIV_ATT, 1);
    protected_member= sym->get_attribute(PROT_ATT, 1);
    if (sym->get_attribute(PAKG_ATT, 1)) {
    	set_package_prot();
    }
    virtual_attr= sym->get_attribute(VIRT_ATT, 1);
    static_attr= sym->get_attribute(STAT_ATT, 1);
    const_attr= sym->get_attribute(CNST_ATT, 1);
    mangled_attr= 0;
    obsolete= 0;
    expect_friend = 0;
    pure_virtual_attr= sym->get_attribute(PVIR_ATT, 1);
    volatile_attr = 0;
    inline_attr= 0;
    extern_def = 0;
    is_def = 0;
    is_old = 0;
    cons_des = 0;
    kr = 0;
    drn = 1;
    from_method = 0;
    synchronized_attr = sym->get_attribute(SYNC_ATT, 1);
    native_attr = sym->get_attribute(NATV_ATT, 1);
    changed_since_saved = 0;
    changed_since_checked_in = 0;
    file_not_checked_in = 0;
    filler = 0;
    datatype = 1;	// flag "cloned" reference ddElement
    ast_node= NULL;
    old_name = strsav(name);

    app* h = finish_constructor(this, kind, is_def, 0, node, xref_hook_turn_off);
    
    if (xref_hook_turn_off)
      dd_sym = sym;	// temporary reference until saved in model

    if (is_smtHeader(h))
      put_dd_ref_file(this);
    else
      assocnode_put_dd (node, this);
}


ddElement::~ddElement()
{
  Initialize(ddElement::~ddElement);

    ddRoot* dr = get_dd_root();
    if (dr && is_ddRoot(dr) && (dr->get_id() >= 1)) {
	dr->remove_hash(this, get_ddname(), get_kind());
    }
    else if (local_hash)
      local_hash->el = 0;
    
 if(aname)
    delete aname;
  if(name)
    delete name;
  if(def)
    delete def;
  if(old_name)
    delete old_name;
  if(def_file)
    delete def_file;
}

bool ddElement::is_changed_since_saved()
{
    //Initialize(ddElement::is_changed_since_saved);
    return(changed_since_saved ? TRUE : FALSE);
}

bool ddElement::is_changed_since_checked_in()
{
    //Initialize(ddElement::is_changed_since_checked_in);
    return(changed_since_checked_in ? TRUE : FALSE);
}

bool ddElement::is_file_not_checked_in()
{
    //Initialize(ddElement::is_file_checked_in);
    return(file_not_checked_in ? TRUE : FALSE);
}

void ddElement::set_changed_since_saved(int flag)
{
    //Initialize(ddElement::set_changed_since_saved);
    changed_since_saved = flag;
}

void ddElement::set_changed_since_checked_in(int flag)
{
    //Initialize(ddElement::set_changed_since_checked_in);
    changed_since_checked_in = flag;
}

void ddElement::set_file_not_checked_in(int flag)
{
    //Initialize(ddElement::set_file_checked_in);
        file_not_checked_in = flag;
}

char const *ddElement::get_name() const
 { return name;}    // return name

void  ddElement::set_name(char const *sp)
{
  if (name) 
    delete name; 
  name = new char[strlen(sp)+1];
  strcpy(name, sp);
  if (old_name)
    delete old_name;
  old_name = strsav(name);
  return;
}

void  ddElement::set_sname(char const *sp)
{
  if (sname) 
    delete sname; 
  sname = new char[strlen(sp)+1];
  strcpy(sname, sp);
  return;
}

void  ddElement::set_sdef(char const *sp)
{
  if (sdef) 
    delete sdef; 
  sdef = new char[strlen(sp)+1];
  strcpy(sdef, sp);
  return;
}


void ddElement::set_assembler_name (char const *sp)  
{
  if (aname)
    delete aname; 
  aname = new char[strlen(sp)+1];
  strcpy(aname, sp);
}

void ddElement::set_def(char const *sp)
{
  if (def)
    delete def; 
  def = new char[strlen(sp)+1];
  strcpy(def, sp);
  return;
}
//
// Definition moved here from ddict.h See that file for explanation.
//
void ddElement::set_is_def()
{
    is_def = 1; 
    extern_def = 0;
    dd_sym = NULL_symbolPtr;
}

void ddElement::unset_is_def()
{
    is_def = 0; 
    dd_sym = NULL_symbolPtr;
}

char const *get_stripped_name(char const *fn)
{
  smtHeader* h = smtHeader::find (fn);
  if (h)
    return h->get_filename ();

  if (fn == 0) return 0;
  if (*fn == '-' || *fn == '+') fn++;
  if (*fn == '-' || *fn == '+') fn++;
  while (fn && *fn >= '0' && *fn <= '9')  ++fn;
  if (fn && *fn == ':') ++fn;
  return fn;
}

void ddElement::set_def_file(char const *fn)
{
  Initialize(ddElement::set_def_file);
  if (fn == 0) return;
  smtHeader* sh = smtHeader::find(fn);
  if (sh) {
    char const *filena = sh->get_filename();
    if (filena == 0) return;
    if (def_file)
      delete def_file;
    def_file = new char[strlen(filena)+1];
    strcpy(def_file, filena);
    return;
  }
  fn = get_stripped_name(fn);
  genString gs;
  if (projList::search_list) {
    projNode* pr = projList::search_list->get_proj(0);
    if (pr)
      pr->fn_to_ln(fn, gs);
    if (gs.length() == 0) 
      projHeader::fn_to_ln(fn, gs, &pr);
  }
  if (gs.length() == 0) gs = fn;
  if (def_file)
    delete def_file;
  def_file = new char[gs.length() + 1];
  strcpy(def_file, gs);
}

static ddHeader* dd_head = NULL;
static ddRoot* root_dd = NULL;

static void make_header_and_root() {
   Initialize(make_header_and_root);

   dd_head = new ddHeader("Data Dictionary");

   root_dd = new ddRoot();
   put_relation(header_of_tree, root_dd, dd_head);
} 

ddRoot::ddRoot(ddKind) 
{
  Initialize(ddRoot::ddRoot);
  num_defined = 1;
  macro_list = 0;
  fdecl_list = 0;
  vdecl_list = 0;
  tdecl_list = 0;

  if ((dd_head == 0) || (root_dd == 0) ) {
     make_header_and_root();
     root_dd->put_first(this);
  } 
  else {
    ddRoot* ld = checked_cast(ddRoot, root_dd->get_last());
    if (ld == 0)
      root_dd->put_first(this);
    else
     ld->put_after(this);
  }


  for (int i = 0; i < MAX_HASH; ++i) {
    dd_hash[i] = NULL;
  }
}

ddRoot::ddRoot(smtHeader* sh) 
{
  Initialize(ddRoot::ddRoot);

  num_defined = 1;  
  macro_list = new ddElement(DD_MACRO_LIST);
  fdecl_list = new ddElement(DD_FUNC_DECL_LIST);
  vdecl_list = new ddElement(DD_VAR_DECL_LIST);
  tdecl_list = new ddElement(DD_TYPEDEF_LIST);

  macro_list->set_language(smtLanguage(sh->get_language()));
  fdecl_list->set_language(smtLanguage(sh->get_language()));
  vdecl_list->set_language(smtLanguage(sh->get_language()));
  tdecl_list->set_language(smtLanguage(sh->get_language()));

  put_first(macro_list);
  macro_list->put_after(fdecl_list);
  fdecl_list->put_after(vdecl_list);
  vdecl_list->put_after(tdecl_list);

  if ((dd_head == 0) || (root_dd == 0) ) {
     make_header_and_root();
     root_dd->put_first(this);
  } 
  else {
    ddRoot* ld = checked_cast(ddRoot, root_dd->get_last());
    if (ld == 0)
      root_dd->put_first(this);
    else
     ld->put_after(this);
  }

  for (int i = 0; i < MAX_HASH; ++i) {
    dd_hash[i] = NULL;
  }
  put_relation(ddRoot_of_smtHeader, sh, this);
  char const *fname = sh->get_filename();
  ddElement *appdd;
   appdd = new ddElement(fname ? fname : "", 
			sh, 1, DD_MODULE, dd_of_appHeader);
  
  appdd->set_assembler_name ("");
}

ddRoot::ddRoot(app* sh) 
{
  Initialize(ddRoot::ddRoot);
  num_defined = 1;
  macro_list = new ddElement(DD_MACRO_LIST);
  fdecl_list = new ddElement(DD_FUNC_DECL_LIST);
  vdecl_list = new ddElement(DD_VAR_DECL_LIST);
  tdecl_list = new ddElement(DD_TYPEDEF_LIST);
  macro_list->set_xref_hash(sh);  
  fdecl_list->set_xref_hash(sh); 
  vdecl_list->set_xref_hash(sh);
  tdecl_list->set_xref_hash(sh);

  macro_list->set_language(smtLanguage(sh->get_language()));
  fdecl_list->set_language(smtLanguage(sh->get_language()));
  vdecl_list->set_language(smtLanguage(sh->get_language()));
  tdecl_list->set_language(smtLanguage(sh->get_language()));

  put_first(macro_list);
  macro_list->put_after(fdecl_list);
  fdecl_list->put_after(vdecl_list);
  vdecl_list->put_after(tdecl_list);

  if ((dd_head == 0) || (root_dd == 0) ) {
     make_header_and_root();
     root_dd->put_first(this);
  } 
  else {
    ddRoot* ld = checked_cast(ddRoot, root_dd->get_last());
    if (ld == 0)
      root_dd->put_first(this);
    else
     ld->put_after(this);
  }


  for (int i = 0; i < MAX_HASH; ++i) {
    dd_hash[i] = NULL;
  }
  char const *fname = sh->get_filename();
  put_relation(ddRoot_of_smtHeader, sh, this);
  ddElement *appdd;
  appdd = new ddElement(fname ? fname : "", 
			sh, 1, DD_MODULE, dd_of_appHeader);
  
  appdd->set_assembler_name (""); appdd->set_xref_hash(sh);
}

ddRoot::~ddRoot()
{
  hash_element* he;
  hash_element* ne;
  for (int i = 0; i < MAX_HASH; ++i) {
    ne = he = dd_hash[i];
    while (ne) {
      ne = ne->next;
      delete he;
      he = ne;
    }
  }
}


ddHeader* get_dd_header()
{
   Initialize(get_dd_header);

   if (!dd_head)
      make_header_and_root();
   return dd_head;
}

static void dd_indent( ostream &fl, int level)
{
  for ( int lvl= level; lvl > 0; lvl--)
    fl << "  ";
}

// walk through the c++ function decl element node to generate a "demangled" name


void ddElement::regenerate_name()
{
  char *p;
  char const *pc;
  int lpfix;
  int len;
  ddElementPtr parent;
  ddElementPtr ep;

  Initialize(ddElement::regenerate_name);

// how long will this name be????;

  p= strchr (get_ddname(), '(');
  if ( p == NULL )
    return;
  len= 0;
  parent=NULL;

  ep = member_get_class();

  if ( ep && dd_is_structure(ep->get_kind()) ) {

    len += ep->get_name_length();
    len+= 2;  // for the "::" ;
    parent= ep;
  }
  char const *pbasic= get_ddname();
  pc= strchr (get_ddname(), ':');
  if ( pc ) {
    pc++ ;
    pc++ ; // beginning of basic name;
    pbasic= pc;
  }
// when pc is NULL, set it to the beginning of array
  else
    pc = pbasic;
// + 1 is for the left parenthesis.
  lpfix= p - pc + 1;
  genString ppname;
  if ( parent ) {
    ppname += parent->get_ddname();
    ppname += "::";
  }
  int lpfixx = lpfix + 1;
  char *ttmp = new char[lpfixx];
  strncpy(ttmp, pbasic, lpfix);
  ttmp[lpfix] = 0;
  ppname += ttmp;
  for (ddElement* chp= checked_cast(ddElement,get_first());  chp; ) {
    ppname += chp->get_def();
    chp = checked_cast(ddElement,chp->get_next());
    if (chp)
      ppname += ", ";
  }
  ppname += ")";
  if ( is_const() ) {
    ppname += " const";
  }
  if ( is_volatile() ) {
    ppname += " volatile";
  }
  genString on = get_ddname();
  delete name;
  name = strsav(ppname);
  delete old_name;
  old_name = strsav(name);
  update_name(on, 0);
  return;
}


ddElementPtr ddElement::get_field(int index)
{
  Initialize(ddElement::get_field);
  ddElement *parg= (index > 0)? checked_cast(ddElement,this->get_first()) : NULL;
  for ( int i= 1; parg && i < index; i++ ) {
    parg= checked_cast(ddElement,parg->get_next());
  }
  return(parg);
}
#ifdef XXX_myTreeObj
myTreeObj::myTreeObj(smtTree* smt, ddElement* ep)
{
  ref_smt = smt;
  dd_node = ep;
  put_relation(cdtree_of_dd, ep, this);
}

int myTreeObj::compare(const Object& o) const 
{
    int ret = 0;
    const myTreeObj *from = (const myTreeObj *) &o;
    if (ref_smt == from->ref_smt && (void *)dd_node == (void *)from->dd_node)
	ret = 1;

    return ret;
}

unsigned myTreeObj::hash () const
{
    return (unsigned)ref_smt + (unsigned) dd_node;
}
#endif
static void ddKind_dump( ostream& fl, ddKind kind)
{
  char const *p;

  Initialize(ddKind_dump);
  switch ( kind){
  case DD_VAR_DECL:
    p= "Variable declaration.";
    break;
  case DD_TYPEDEF:
    p= "Type definition.";
    break;
  case DD_MACRO:
    p= "Macro definition.";
    break;
  case DD_FUNC_DECL:

    p= "Function declaration.";
    break;
  case DD_ENUM:
    p= "Enumeration type.";
    default:;
  case DD_UNKNOWN:
    p= "Unknown definition class.";
    break;
  }
  fl << p;
  return;
}

void ddElement::print( ostream& fl, int level) const
{
  ddElementPtr ep;	
  ddElementPtr ap;	
  objSet ss;	
  Obj *ob;
  char const *separator;
  
  Initialize(ddElement::print);
  dd_indent(fl,level);
  char const *tp = ddKind_name(kind);
  ddElementPtr mydd = checked_cast(ddElement,this);
  fl << tp << ' ' << get_ddname() << " is_def " << mydd->get_is_def();
  if(find_def())
      fl << " def " << find_def();
  fl  << endl;

// more info in some special cases

  switch((ddKind)kind){
  case DD_TYPEDEF:          // drop through to VAR_DECL
  case DD_VAR_DECL:         // except for "typedef"
    if ( record ){
      fl << "{\n";
      ep= checked_cast(ddElement,get_first());
      for ( ; ep; ep= checked_cast(ddElement,ep->get_next())) {
	ep->print( fl, level+1);
      }

      dd_indent(fl,level+1);
      fl << '}';
    }
    fl << " <Token id: " << tid << "> ";
    if ( is_protected() )
      fl << "<protected> ";
    if ( is_package_prot() )
      fl << "<package> ";
    if ( is_private() )
      fl << "<private> ";
    fl << ";\n";
    break;
  case DD_UNION:
    {
      dd_indent(fl,level+1);
      fl << "{\n";
      ep= checked_cast(ddElement,get_first());
      for ( ; ep; ep= checked_cast(ddElement,ep->get_next())) {
	ep->print( fl, level+1);
      }
      dd_indent(fl,level+1);
      fl << '}';
    }
    fl << " <Token id: " << tid << "> ";
    fl << ";\n";
    break;
  case DD_CLASS:
    {
      char const *vname;
      char const *pname;
      for ( int i= 6; i; i-- ) {
	switch (i) {
	case 1:
	  ss= get_relation(vpub_baseclassof,this);
	  vname= " virtual";
	  pname= " public ";
	  break;
	case 2:
	  ss= get_relation(pub_baseclassof,this);
	  vname= "";
	  pname= " public ";
	  break;
	case 3:
	  ss= get_relation(vpri_baseclassof,this);
	  vname= " virtual";
	  pname= " private ";
	  break;
	case 4:
	  ss= get_relation(pri_baseclassof,this);
	  vname= "";
	  pname= " private ";
	  break;
	case 5:
	  ss= get_relation(vpro_baseclassof,this);
	  vname= " virtual";
	  pname= " protected ";
	  break;
	case 6:
	  ss= get_relation(pro_baseclassof,this);
	  vname= "";
	  pname= " protected ";
	  break;
	}

	separator= ":\n";
	ForEach(ob,ss){
	  fl << separator;
	  separator= ",\n";
	  ap= checked_cast(ddElement,ob);
	  dd_indent(fl,level+1);
	  fl << vname << pname;
	  fl << ap->get_ddname();
	}
      }
      dd_indent(fl,level+1);
      fl << "{\n";
      if (get_kind() == DD_CLASS) {
       } else {
	ep= checked_cast(ddElement,get_first());
	for ( ; ep; ep= checked_cast(ddElement,ep->get_next())) {
	  ep->print( fl, level+1);
	}
      }
      dd_indent(fl,level+1);
      fl << '}';
    }
    fl << " <Token id: " << tid << "> ";
    fl << ";\n";
    break;
  case DD_PARAM_DECL:
  case DD_FIELD:
    fl << " <Token id: " << tid << "> ";
    if ( is_protected() )
      fl << "<protected> ";
    if ( is_package_prot() )
      fl << "<package> ";
    if ( is_private() )
      fl << "<private> ";
    fl << ";\n";
    break;
  case DD_MACRO:
    {
    fl << " <Token id: " << tid << "> " << endl;
    dd_indent(fl,level+1);
    fl << "file: ";
    ddRoot* dr;
    dr = get_dd_root();
    smtHeaderPtr sh = checked_cast(smtHeader, get_relation(smtHeader_of_ddRoot, dr));
    fl << sh->get_filename();
    fl << "\n";
    dd_indent(fl,level+1);
    fl << "def type: ";
    ddKind_dump( fl, (ddKind)kind );
    fl << "\n";
    dd_indent(fl,level+1);
    fl << "define " << find_def();
    fl << "\n";
    }
    break;
  case DD_UNKNOWN:
    fl << "Unknown definition class!\n";
    break;
  case DD_FUNC_DECL:
    fl << " <Token id: " << tid << "> ";
    if ( is_protected() )
      fl << "<protected> ";
    if ( is_package_prot() )
      fl << "<package> ";
    if ( is_private() )
      fl << "<private> ";
    fl << '(';
    ep= checked_cast(ddElement,get_first());
    if ( ep ) {
      fl << "\n";
      for ( ; ep; ep= checked_cast(ddElement,ep->get_next())) {
	ep->print( fl, level+1);
      }
      dd_indent(fl,level+1);
    }
    fl << ')';
    if ( is_pure_virtual() )
      fl << "= 0";
    fl << ";\n";
    break;

  default:
    break;
  }
  fl << flush;
  return;
}

void ddElement::send_string(ostream& stream) const
{
  char const *p;

  p= strchr(name, ':');
  if ( p == NULL )
    p= name;
  else
    p += 2;
  stream << name;
}

ddRoot* ddElement::get_dd_root() const
{
  Initialize(ddElement::get_dd_root);
  appTree* tmp;
  ddRoot* dr;
  tmp = checked_cast(appTree, this);
  while (tmp->get_parent() && (tmp->get_parent() != root_dd))
    tmp = checked_cast(appTree, tmp->get_parent());
  if (is_ddRoot(tmp))
    dr = checked_cast(ddRoot, tmp);
  else {
    app* ah = get_xref_hash();
    dr = checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, ah));
  }
    
  return(dr);
}
void ddElement::put_signature(ostream& os) const
{
  char const *nm = get_name();
  char const *kn = ddKind_name(get_kind());
  os << kn+3 << '.' << nm;
}

appPtr ddElement::get_main_header()
{
  Initialize(ddElement::get_main_header);
  ddRoot* dr;
  dr = get_dd_root();
  appPtr sh = checked_cast(app, get_relation(smtHeader_of_ddRoot, dr));
  return(sh);
}
  


smtHeaderPtr ddElement::get_smt_header()
{
  Initialize(ddElement::get_smt_header());

  ddRoot* dr;
  dr = get_dd_root();
  smtHeaderPtr sh = checked_cast(smtHeader, get_relation(smtHeader_of_ddRoot, dr));
  return(sh);
}


ddElement* ddElement::get_child(char const *ns, ddKind k)
{
  Initialize(ddElement::get_child);

  ddElement* child = checked_cast(ddElement, get_first());
  if (child == 0) return 0;
  while (child) {
    if ((child->get_kind() == k) &&
	(strcmp(ns, child->get_ddname()) == 0))
      return child;
    child = checked_cast(ddElement, child->get_next());
  }
  return child;
}
int ddElement::get_def_length() const
{
  Initialize(ddElement::get_def_length);
  char const *de = get_def();
  if (de)
    return strlen(de);
  else
    return 0;
}
char const *ddElement::get_def() const
{
  Initialize(ddElement::get_def);
  ddElement* dd = checked_cast(ddElement, this);
  int has_smt = is_smt_restore(dd);
  if (has_smt) {
    smtTree* smt_decl = dd_get_any_smt_decl(dd);
    if (smt_decl) {
      ddElement* dt = checked_cast(ddElement, get_relation(semtype_of_smt_decl, smt_decl));
      if (dt)
	return dt->get_ddname();
    }
  }
  if (ddKind_is_local(get_kind())) {
    ddElement* ty = checked_cast(ddElement, get_relation(semtype_of_smt_decl, this));
    if (ty) return ty->get_ddname();
    return 0;
  }
  fsymbolPtr sym = dd->get_xrefSymbol();
  if (sym.xrisnotnull() ){
    symbolArr as;
    if (sym->get_link(has_type, as)) {
      sym = as[0].get_xrefSymbol();
      return sym.get_name();
    }
  }

  return 0;
}

char const *ddElement::find_def() const
{
  Initialize(ddElement::get_def);
  ddElement* dd = checked_cast(ddElement, this);
  int has_smt = is_smt_restore(dd);
  if (has_smt) {
    smtTree* smt_decl = dd_get_any_smt_decl(dd);

    if (smt_decl) {
      ddElement* dt = checked_cast(ddElement, get_relation(semtype_of_smt_decl, smt_decl));
      if (!dt && (smt_decl->get_node_type() == SMT_fdef)) {
	smt_decl = smt_decl->get_first();
	dt = checked_cast(ddElement, get_relation(semtype_of_smt_decl, smt_decl));
      }
      if (dt)
	return dt->get_ddname();
    }
  }
  if (ddKind_is_local(get_kind())) {
    ddElement* ty = checked_cast(ddElement, get_relation(semtype_of_smt_decl, this));
    if (ty) return ty->get_ddname();
    return 0;
  }
  fsymbolPtr sym = dd->find_xrefSymbol();
  if (sym.xrisnotnull() ){
    symbolArr as;
    if (sym->get_link(has_type, as)) {
      sym = as[0].get_xrefSymbol();
      return sym.get_name();
    }
  }

  return 0;
}




/* just look at string as a number of base 128, and mod MAX_HASH which is a prime
   and not near power of 2
*/
void dd_get_short_name(char const *name, char const *sn);
int ddRoot::Rhash(char const *s1)
{
  Initialize(ddRoot::Rhash);

  char const *s = s1;
  unsigned int pow = 1;
  unsigned int val = 0;
  while (*s) {
    if (*s != ' ') {
      if (val > 0x00FFFFFF)
	val %= MAX_HASH;
      if (pow > 0x00FFFFFF)
	pow %= MAX_HASH;
      val += ((unsigned char)*s) * pow;
      pow <<= 7; // radix 128
    }
    s++;
  }
  val %= MAX_HASH;
  return(val);
}


//
// returns: -1: error, 0: the scoping is setup
//
int
dd_set_entity_scope(ddElement * scope, ddElement * entity)
{
    Initialize(dd_set_entity_scope);

    int ret_val = -1;

    if (scope!=0 && entity!=0) {
	if (dd_is_structure(scope->get_kind()) || scope->get_kind() == DD_PACKAGE) {
	    if (dd_is_scoped_entity(entity->get_kind())) {
		put_relation(entity_of_scope, scope, entity);
		ret_val = 0;
	    }
	}
    }

    return ret_val;
}

// Add the element into dd_root (according to kind) and connect it to 
// local hash table.
ddElementPtr ddRoot::add_def(ddKind knd, int nl, char const *ns,
			     int dl, char const *ds, int al, char const *an)
{
  Initialize(ddRoot::add_def);
  
  ddElementPtr ep = NULL;
  if (ep = lookup( knd, ns)) {
  }

  if(ep){
    if (ep->is_expect_friend())
      ep->clear_expect_friend();
  }
  else {
    ep = new ddElement(knd, ns, nl, ds, dl, an, al);
    insert(ep, knd);
    insert_hash(ep);
  }
  app* h = checked_cast(app, get_relation(smtHeader_of_ddRoot, this));
  if(h && is_smtHeader(h))
        ep->language = h->language;
  ep->set_xref_hash(h);
  if (dd_can_have_ref_file(ep->get_kind())) {
    put_dd_ref_file(ep);
  }
  return(ep);
}  

ddElementPtr ddRoot::add_field( ddElementPtr parent_def, 
			        ddElementPtr pred,
                                ddKind k, smtTreePtr /*token_id*/,
                                int nl, char const *ns, int dl, char const *ds,
			        int al, char const *an)
{
  Initialize(ddRoot::add_field);

  parent_def->set_drn();
  ddElement* newp = new ddElement(k, ns, nl, ds, dl, an, al);
  newp->set_language(parent_def->get_language());
  
  if (pred)
    pred->put_after(newp);
  else
    parent_def->put_first(newp);

  newp->set_xref_hash(checked_cast(app, get_relation(smtHeader_of_ddRoot, this)));
  if (newp && !ddKind_is_local(newp->get_kind()))
      put_dd_ref_file(newp);
  return(newp);
}


ddElementPtr ddRoot::add_field( ddElementPtr parent_def,
                                ddKind k, smtTreePtr ,
                                int nl, char const *ns, int dl, char const *ds,
			        int al, char const *an, int pos)
{
  Initialize(ddRoot::add_field);

  ddElementPtr newp = NULL;
  int pos_t = pos;
  if (!ddKind_is_local(k))
    newp = lookup( k, ns);
  if (newp) return newp;
  // -- DD_PARAM_DECL's get deleted before a new build is completed (ddRoot::init_dds())
  if (pos_t > 0) {
      for (newp = checked_cast(ddElement, parent_def->get_first()); newp!=NULL && pos_t>0; ) {
	  if (newp->get_id() >= 1)
	      pos_t--;
	  if (pos_t > 0)
	      newp = checked_cast(ddElement, newp->get_next());
      }
  }
  if (newp && (strcmp(newp->get_ddname(), ns)==0)) return newp;
  newp = new ddElement(k, ns, nl, ds, dl, an, al);
  newp->set_language(parent_def->get_language());
  parent_def->set_drn();
  ddElement* lc = checked_cast(ddElement, parent_def->get_last());
  if (pos == 0) {
    if (lc)
      lc->put_after(newp);
    else
      parent_def->put_first(newp);
  }
  else if (pos == 1)
    parent_def->put_first(newp);
  else {
    ddElement* fc = checked_cast(ddElement, parent_def->get_first());
    pos -= 2;
    while (pos && fc) {
      fc = checked_cast(ddElement, fc->get_next());
      pos--;
    }
    if (fc)
      fc->put_after(newp);
    else if (lc)		// if we fell off the end, put it after lastchild
      lc->put_after(newp);
  }
  if (!ddKind_is_local(k))
    insert_hash(newp);
  newp->set_xref_hash(checked_cast(app, get_relation(smtHeader_of_ddRoot, this)));
  if (newp && !ddKind_is_local(newp->get_kind()))
      put_dd_ref_file(newp);
  return(newp);
}

void ddRoot::insert(ddElementPtr ep, ddKind knd)
{
   
  Initialize(ddRoot::insert);
  ddElementPtr tep;

  tep = NULL;
  if (dd_is_function_declaration(knd))
    tep = fdecl_list;
  else if (dd_is_type_declaration(knd))
    tep = tdecl_list;
  else if (dd_is_var_declaration(knd))
    tep = vdecl_list;
  else if (knd == DD_MACRO)
    tep = macro_list;
  else 
    tep = macro_list;

  tep->put_first(ep);

  return;
}


void ddRoot::insert_hash(ddElementPtr ep)
{
  Initialize(ddRoot::insert_hash);

  int h_ind = Rhash(ep->get_ddname());
  hash_element* he = new hash_element(ep);
  if (dd_hash[h_ind]) {
    dd_hash[h_ind]->prev = he;
    he->next = dd_hash[h_ind];
  }
  he->prev = NULL;  
  dd_hash[h_ind] = he;

  return;
}
      

void ddRoot::insert_hash(hash_element* he)
{
  Initialize(ddRoot::insert_hash);

  ddElementPtr ep;
  ep = he->el;
  int h_ind = Rhash(ep->get_ddname());
  if (dd_hash[h_ind]) {
    dd_hash[h_ind]->prev = he;
    he->next = dd_hash[h_ind];
  }
  he->prev = NULL;
  dd_hash[h_ind] = he;

  return;
}
      

void ddRoot::update_hash(ddElement* ep, char const *ns, ddKind k)
{
  remove_hash(ep, ns, k);
  insert_hash(ep);
}
void ddRoot::remove_hash(hash_element* list, int ind)
{
  if (list->prev) {
    list->prev->next = list->next;
    if (list->next)
      list->next->prev = list->prev;
  }
  else {
    if (list->next)
      list->next->prev = NULL;
    if (list->el) {
      dd_hash[ind] = list->next;
    }
  }
  delete list;
}

void ddRoot::remove_hash(ddElement* ep, char const *ns, ddKind /*k*/)
{
  hash_element* list;
  ddElementPtr el;
  int ind = Rhash(ns);
  list = dd_hash[ind];
  while (list) {
    el = list->el;
    if (el == ep) {
      if (list->prev) {
	list->prev->next = list->next;
	if (list->next)
	  list->next->prev = list->prev;
      }
      else {
	if (list->next)
	  list->next->prev = NULL;
	dd_hash[ind] = list->next;
      }
      delete list;
      return;
    }
    list = list->next;
  }
}

ddElementPtr ddRoot::lookup(ddKind k, char const *ns)
{
  Initialize(ddRoot::lookup);

  hash_element* list;
  ddElementPtr el;
  int ind = Rhash(ns);
  list = dd_hash[ind];
  while (list) {
    el = list->el;
    if (el && (el->get_kind() == k) 
	   && (strcmp_name(k, el->get_ddname(), ns) == 0))
      return(el);
    list = list->next;
  }
  return(NULL);
}

HierarchicalPtr ddElement::get_header() const 
{
   Initialize(ddElement::get_header);

   if (kind == DD_SUBSYSTEM)
      return appTree::get_header();
   else return (checked_cast(Hierarchical,get_dd_header()));
} 

//------------------------------------------------------------------------------

appTreePtr ddElement::rel_copy_for_header(appPtr ah, objCopier *)
{
  Initialize(ddElement::rel_copy_for_header);
  appPtr oh = checked_cast(app, get_main_header());
  if (ah == oh) return this;
  
  ddRoot* dr = dd_sh_get_dr(ah);
  char const *t_name = name ? name : "";
  char const *t_def = def ? def : "";
  char const *t_aname = aname ? aname : "";
  ddElement *res = dr->add_def(kind, strlen(t_name), t_name, 
			       strlen(t_def), t_def, strlen(t_aname), t_aname );
  res->set_language(get_language());
  res->set_def_file_with_good_name(def_file ? def_file : "");
  res->set_struct_rel_type(struct_rel_type);
  return res;
}

// delete all element except of kind k (currently used for macro -- may not need though)
void ddRoot::tidy_els(ddKind k)
{
  Initialize(ddRoot::tidy_els);
  hash_element* hen;
  for (int i = 0; i < MAX_HASH; ++i ) {
    hash_element* he = dd_hash[i];
    while (he) {
      hen = he->next;
      ddElement* dd = he->el;
      if (dd && (dd->get_kind() == k || user_ddKind(dd->get_kind()))) {
	if (dd->get_kind() == DD_CLASS || dd->get_kind() == DD_UNION ||
	    dd->get_kind() == DD_ENUM) {
	  if (dd->get_first())
	    dd->splice();
	}
	else
	  dd->remove_from_tree();
	start_transaction() {      
	  remove_hash(he, i);
	  obj_delete(dd);
	}end_transaction();
	he = hen;
      }
    }
  }
}

void ddElement::notify(int actiontype, Relational* source_obj, objPropagator* , RelType* reltype)
{
  Initialize(ddElement::notify);
  DBG {
    msg("op $1 rel $2 src") << actiontype << eoarg << reltype->get_name() << eom;
    obj_prt_obj(source_obj);
    msg("") << eom;
  }

}

static int is_superclass_rel(RelType * rel)
{
  return (
     rel == vpub_subclassof||
    rel == vpri_subclassof||
    rel == pub_subclassof||
    rel == pri_subclassof||
    rel == vpro_subclassof||
    rel == pro_subclassof
    );
  
}

int clean_comp_header = 0;

//
//activate appropriate hooks function for rel
//
RelClass(elsElement);
void dd_rel_hook (int oper,
		Relational* src, Relational* targ, RelType* rel)
{	
    underInitialize (dd_rel_hook);

    if (oper == 0)
	return;
    if (clean_comp_header)
	return;
    if(is_elsElement(src) || is_elsElement(targ))
      return;
    DBG {
	msg("dd_rel_hook: $1") << (oper > 0 ? "put " : "rem ") << rel->get_name() << eom;

	src->Relational::print ();
	targ->Relational::print ();
	msg("") << eom;
    }

    if (xref_hook_turn_off && (oper > 0))
	return;

    int			is_dd_src = is_ddElement (src);
    int			is_dd_tar = is_ddElement (targ);
    ddElementPtr	dd = NULL;
    smtTreePtr		smt;
    appTreePtr		atp;

    if (is_dd_src && is_dd_tar) {

	ddElementPtr dd1 = checked_cast (ddElement, src);
	ddElementPtr dd2 = checked_cast (ddElement, targ);

	if (rel == user_of_used) {
	    if (dd1->get_kind() == DD_MODULE && dd2->get_kind() == DD_MODULE)
		handle_includes (dd2, dd1, oper);
	    else if (dd2->get_kind() == DD_SUBSYSTEM)
		handle_used_of_user (dd2, dd1, oper);
	    else if (dd2->get_kind() == DD_CLASS || dd2->get_kind() == DD_RELATION)
		handle_used_of_user (dd1, (smtTreePtr) dd2, oper);
	    else if (dd2->get_kind() && is_smtHeader(dd2->get_main_header()) )
		handle_used_of_user (dd1, (smtTreePtr) dd2, oper);
	    else if (oper > 0)
		handle_used_of_user (dd1, dd2, oper);	
	}

	else if (rel == smt_decl_of_semtype) {
	  dd = checked_cast (ddElement, src);
	  smt = (smtTree*) targ;
	  handle_smt_decl_of_semtype (dd, smt, oper);
	}

	else if (rel == has_friend) {
	    dd = checked_cast (ddElement, src);
	    smt = (smtTreePtr) targ;
	    handle_has_friend (dd, smt, oper);
	}

	else if (rel == superclassof) {
	    dd = checked_cast (ddElement, src);
	    smt = (smtTree *) targ;
	    handle_superclassof (dd, smt, oper);
	}

	else if (is_superclass_rel(rel)) {
	    dd1 = checked_cast (ddElement, src);
	    dd2 = checked_cast (ddElement, targ);
	    handle_superclassof_att (dd1, dd2, oper, rel);
	}

	else if (rel == struct_decl_of_ref)
	    handle_struct_decl_of_ref (dd1, dd2, oper);
#ifdef XXX_assoc
	else if (rel == dd_of_def_assocnode)
	    handle_used_of_user (dd2, dd1, oper);

	else if (rel == assoc_member_of_dd) {
	    ddElementPtr assoc_dd, entity_dd;
	    if (dd1->get_kind () == DD_SOFT_ASSOC) {
		assoc_dd = dd1;
		entity_dd = dd2;
	    }
	    else {
		assoc_dd = dd2;
		entity_dd = dd1;
	    }
	    handle_assoc (assoc_dd, entity_dd, oper);
	}
	
	else if (rel == instance_of_assocType) {
	    ddElementPtr assoc_dd, type_dd;
	    if (dd1->get_kind () == DD_SOFT_ASSOC_TYPE) {
		type_dd = dd1;
		assoc_dd = dd2;
	    }
	    else {
		type_dd = dd2;
		type_dd = dd1;
	    }
	    handle_instance_of_assocType (type_dd, assoc_dd, oper);
#endif //	}
	else if (rel == template_of_instance)
	  handle_template_of_instance(src,targ,oper);

	else if (rel == property_of_possessor)
	  handle_property_of_possessor(dd1, dd2, oper);
    }

    if (is_dd_src == is_dd_tar)
	return;
#ifdef XXX_assoc
    if (rel == dd_of_def_assocnode) {
	dd = checked_cast (ddElement, targ);
	if (is_oodt_relation (src))
	    handle_def_smt_of_dd (dd, 0, oper);
    } else
#endif
    if (rel == ref_smt_of_dd) {
	dd = checked_cast (ddElement, src);
	smt = checked_cast (smtTree, targ);
	handle_ref_smt_of_dd (dd, smt, oper);
    }

    else if (rel == decl_smt_of_dd) {
	dd = checked_cast (ddElement, src);
	smt = checked_cast (smtTree, targ);
	handle_decl_smt_of_dd (dd, smt, oper);
    }

    else if (rel == has_friend) {
	dd = checked_cast (ddElement, src);
	smt = checked_cast (smtTree, targ);
	handle_has_friend (dd, smt, oper);
    }

    else if (rel == def_smt_of_dd) {
	dd = checked_cast (ddElement, src);
	smt = checked_cast (smtTree, targ);
	handle_def_smt_of_dd (dd, smt, oper);
    }

    else if ((rel == user_of_used) && is_dd_tar) {
	dd = checked_cast (ddElement, targ);
	smt = checked_cast (smtTree, src);
	handle_user_of_used (dd, smt, oper); 
    }

    else if ((rel == user_of_used) && (is_dd_tar == 0)) {
	dd = checked_cast (ddElement, src);
	smt = (smtTreePtr) targ;
	handle_used_of_user (dd, smt, oper); 
    }

    else if (rel == superclassof) {
	dd = checked_cast (ddElement, src);
	smt = checked_cast (smtTree, targ);
	handle_superclassof (dd, smt, oper);
    }

    else if (rel == smt_decl_of_semtype) {
	dd = checked_cast (ddElement, src);
	smt = checked_cast (smtTree, targ);
	handle_smt_decl_of_semtype (dd, smt, oper);
    }

    else if (rel == ref_file_of_dd) {
	dd = checked_cast (ddElement, src);
	smt = checked_cast (smtTree, targ);
	handle_ref_file_of_dd (dd, smt, oper);
    }
    

    else if (rel == ref_app_of_dd) {
	dd = checked_cast (ddElement, src);
	atp = checked_cast (appTree, targ);
	handle_ref_app_of_dd(dd, atp, oper);
    }
}

//
// callback for setting up relation "scope_of_entity"
//      e.g., struct A { struct B {} }, A is scope of B
//
void
dd_rel_scope_hook(int oper, Relational *src, Relational *trg, RelType *)
{
    Initialize(dd_rel_scope_hook);

    if (!xref_hook_turn_off || oper<=0) {
	if (src!=0 && trg!=0) {
	    if (is_ddElement(src) && is_ddElement(trg)) {
		handle_scope_of_entity(checked_cast(ddElement,src),
				       checked_cast(ddElement,trg), oper);
	    }
	}
    }
}

// set hooks function for relations
int dd_set_hook()
{
  obj_rel_put_hook(has_friend, dd_rel_hook);
  obj_rel_put_hook(ref_smt_of_dd, dd_rel_hook);
  obj_rel_put_hook(decl_smt_of_dd, dd_rel_hook);
  obj_rel_put_hook(ref_app_of_dd, dd_rel_hook);
  obj_rel_put_hook(def_smt_of_dd, dd_rel_hook);
  obj_rel_put_hook(user_of_used, dd_rel_hook);
  obj_rel_put_hook(superclassof, dd_rel_hook);
  obj_rel_put_hook(smt_decl_of_semtype, dd_rel_hook);
  obj_rel_put_hook(id_of_decl, dd_rel_hook);
  obj_rel_put_hook(struct_decl_of_ref, dd_rel_hook);
  //  obj_rel_put_hook(dd_of_def_assocnode, dd_rel_hook);
  //  obj_rel_put_hook(assoc_member_of_dd, dd_rel_hook);
  //  obj_rel_put_hook(instance_of_assocType, dd_rel_hook);
  obj_rel_put_hook(vpub_subclassof , dd_rel_hook);  
  obj_rel_put_hook(pub_subclassof , dd_rel_hook);  
  obj_rel_put_hook(vpro_subclassof , dd_rel_hook);  
  obj_rel_put_hook(pro_subclassof , dd_rel_hook);    
  obj_rel_put_hook(vpri_subclassof , dd_rel_hook);  
  obj_rel_put_hook(pri_subclassof , dd_rel_hook);    
  obj_rel_put_hook(ref_file_of_dd, dd_rel_hook);
  obj_rel_put_hook(template_of_instance, dd_rel_hook);
  obj_rel_put_hook(property_of_possessor, dd_rel_hook);

  // -- NESTED:
  obj_rel_put_hook(scope_of_entity, dd_rel_scope_hook);
  
  auto_rels.insert(has_friend);
  auto_rels.insert(friend_of );
  auto_rels.insert(ref_smt_of_dd );
  auto_rels.insert(ref_dd_of_smt );
  auto_rels.insert(decl_smt_of_dd );
  auto_rels.insert(decl_dd_of_smt );
  auto_rels.insert(def_smt_of_dd );
  auto_rels.insert(def_dd_of_smt );
  auto_rels.insert(user_of_used );
  auto_rels.insert(used_of_user );
  auto_rels.insert(superclassof);
  auto_rels.insert(subclassof );  
  auto_rels.insert(smt_decl_of_semtype);
  auto_rels.insert(semtype_of_smt_decl );
  auto_rels.insert(id_of_decl );
  auto_rels.insert(decl_of_id );
  auto_rels.insert(struct_decl_of_ref );
  auto_rels.insert(struct_ref_of_decl );
  auto_rels.insert(vpub_baseclassof );
  auto_rels.insert(vpub_subclassof );
  auto_rels.insert(pub_baseclassof );
  auto_rels.insert(pub_subclassof );
  auto_rels.insert(pri_baseclassof );
  auto_rels.insert(pri_subclassof );
  auto_rels.insert(vpri_baseclassof );
  auto_rels.insert(vpri_subclassof );
  auto_rels.insert(vpro_baseclassof );
  auto_rels.insert(vpro_subclassof );
  auto_rels.insert(pro_baseclassof );
  auto_rels.insert(pro_subclassof );
  auto_rels.insert(smt_to_dd );
  auto_rels.insert(dd_to_smt );
  auto_rels.insert(defined_in );
  auto_rels.insert(elements_of );
  auto_rels.insert(cdtree_of_dd );
  auto_rels.insert(dd_of_cdtree );
  auto_rels.insert(smt_of_dd );
  auto_rels.insert(dd_of_smt );
  auto_rels.insert(typenm_of_classorstruct );
  auto_rels.insert(classorstruct_of_typenm );
  auto_rels.insert(cd_of_structure );
  auto_rels.insert(structure_of_cd );
  auto_rels.insert(ref_file_of_dd);
  auto_rels.insert(dd_of_ref_file);
  auto_rels.insert(template_of_instance);
  auto_rels.insert(instance_of_template);
  auto_rels.insert(property_of_possessor);
  auto_rels.insert(possessor_of_property);

  // -- NESTED:
  auto_rels.insert(scope_of_entity);
  auto_rels.insert(entity_of_scope);
  
  return 1;
}



cardObj::cardObj(Obj* o)
{
  Initialize(cardObj::cardObj);
  ob = o;
}
void cardObj::inc(appTree* at)
{
  if (!(os.includes(at)))
    os.insert(at);
}
void cardObj::dec(appTree* at)
{
  if (os.includes(at))
    os.remove(at);
}



// functions for handling class/member relations in the dd
//   given a class, get a list of its members
//   given a member, get its declaring class

// get the direct dd children of a class node in the dd

void ddElement::children_of_class(objArr& ddArr) {

	Initialize(ddElement::children_of_class);

	ddElementPtr p = this;

	if (p && p->get_kind() == DD_CLASS) {

		p = checked_cast(ddElement, p->get_first());
		if (p) {
			while (p) {
				ddArr.insert_last(p);
				p = checked_cast(ddElement, p->get_next());

			}
		}
	}
}

// given a dd node for a class, return a list (objArr)
// of the its member functions

void ddElement::func_members_of_class(objArr& ddArr) {

	Initialize(ddElement::func_members_of_class);

	// find the smt class declaration corresponding to
	// the input dd class entry

	smtTree* smt = checked_cast(smtTree, get_relation(def_smt_of_dd, this));

	smtTree* p = smt;

	if (!p || p->get_node_type() != SMT_cdecl) {
		return;
	}

	p = checked_cast(smtTree, p->get_first());

	if (!p || (p->get_node_type() != SMT_title && p->get_node_type() !=
		   SMT_superclass)) {
		return;
	}

	p = checked_cast(smtTree, p->get_next());

	if (!p || p->get_node_type() != SMT_cbody) {
		return;
	}

	p = checked_cast(smtTree, p->get_first());

	while (p) {
		smtTreePtr q;

		q = p;	// must not lose p if we walk around

		int nt = q->get_node_type();
		if (nt == SMT_list_decl) {
			q = checked_cast(smtTree, q->get_first());
			while (q) {
			  	if (q->get_node_type() == SMT_fdecl) {
					smtTreePtr   s;  // token for name of function
					ddElementPtr dd; // dd element for the token

					s  = checked_cast(smtTree, get_relation(id_of_decl, q));
					if (s) {
						dd = checked_cast(ddElement,
							get_relation(ref_dd_of_smt, s));

						if (dd) {
							ddArr.insert_last(dd); // put on list
						}
					}
				}
				q = checked_cast(smtTree, q->get_next());
			}
		}	
		q = p;
		if (nt == SMT_fdef) {
		  ddElement* dd = checked_cast(ddElement, get_relation(def_dd_of_smt,p));
		  if (dd)
		    ddArr.insert_last(dd);	// put on list
		}

		if (nt == SMT_fdecl) {
			smtTreePtr   s;		// token for name of function
			ddElementPtr dd;	// dd element for the token

			s  = checked_cast(smtTree, get_relation(id_of_decl, q));
			if (s) {
				dd = checked_cast(ddElement, get_relation(ref_dd_of_smt, s));
				if (dd) {
					ddArr.insert_last(dd);	// put on list
				}
			}
		}

		p = checked_cast(smtTree, p->get_next());
	}
}

// Given a dd element for a class (struct or union), find
// all of its top-level members (data and functions)
//
// (NOTE: It is no longer possible to obtain the methods
// of a class by looking at the children of its dd, since
// these have been detached. Hence two different methods
// are called to obtain the data field and the methods.)

void ddElement::class_get_members(objArr& ddArr) {

	Initialize(ddElement::class_get_members);

	children_of_class(ddArr);	// get fields (member data)

// Note: the following is needed to ensure that static data members are
// inserted into ddArr exactly once, regardless of whether static data
// members are kept as children of the class or not (a decision that
// keeps changing, breaking this function each time -- until now!).

	objArr static_data_members;
	get_loaded_refs_same_file(static_data_members, DD_VAR_DECL);
	Obj* static_data_member;
	ForEach(static_data_member, static_data_members) {
	   bool in_ddArr = false;
	   Obj* data_member;
	   ForEach(data_member, ddArr) {
	      if (data_member == static_data_member) {
		 in_ddArr = true;
		 break;
	      }
	   }
	   if (!in_ddArr) {
	      ddArr.insert_last(static_data_member);
	   }
	}
	get_loaded_refs_same_file(ddArr, DD_FUNC_DECL);
}

// given a dd element for a method (member function),
// return the class which declares the method
 
ddElementPtr ddElement::method_get_class() {

	Initialize(ddElement::method_get_class);

	ddElementPtr p           = this;
	ddElementPtr using_class = NULL;

	// the dd for a member function used to be
	// connected to its class via get_parent(),
	// but this connection is no longer made.

	if (p && p->get_kind() == DD_FUNC_DECL) {

		// one way to get to the declaring class' dd
		// element is through the xref.

		fsymbolPtr xMethod = p->get_xrefSymbol();

		if (xMethod.xrisnotnull()) {
			fsymbolPtr xClass = xMethod->get_parent_of();
			if (xClass.xrisnotnull()) {
				using_class = xClass->get_def_dd();
			}
		}

	}

	return using_class;
}


// given a dd element for a data member or a member function
// (method), return the class which declares the member

ddElementPtr ddElement::member_get_class() {

	Initialize(ddElement::member_get_class);

	ddElementPtr p = this;

	ddElementPtr using_class = NULL;

	if (p && p->get_kind() == DD_FIELD) {
		p = checked_cast(ddElement, p->get_parent());
		if (p) {
			using_class = p;
		}
	} else {
		if (p->get_kind() == DD_FUNC_DECL) {
			using_class = p->method_get_class();
		}
	}

	return using_class;
}


int ddElement::get_has_def()
{
  return has_def;
}
void ddElement::set_has_def(int i)
{
  has_def = i;
}

ddRoot::ddRoot (const ddRoot&)
{}

ddElement::ddElement (const ddElement&)
{}

int should_rm_during_co(ddElement *dd)
{
  Initialize(should_rm_during_co);
  if (!dd) return 0;
  if (dd_get_ref_file(dd) || (dd->get_kind() == DD_MODULE && dd->get_is_def()))
    return 0;
  return 1;
}

void ddRoot::setup_SWT_Entity_Status(int del)
{
  Initialize(ddRoot::setup_SWT_Entity_Status);

  app *ah = get_main_header();
  if (ah && !is_smtHeader(ah))
    del = 0;
  for (int i = 0; i < MAX_HASH; ++i ) {
    hash_element* he = dd_hash[i];
    while (he) {
      ddElement * dd = he->el;
      he = he->next;
      
      if (dd_is_SWT_entity(dd->get_kind()))
      {
	  if (del && should_rm_during_co(dd))
	      remove_dd_el(dd);
	  else if (dd->get_is_def()) {
	      dd->set_sw_tracking(SWT_UNCHANGE);
	      dd->set_lmd(0);
	  }
      }
#ifdef XXX_assoc
      else if (dd->get_kind() == DD_ASSOCLINK)
#endif //      {      }
    } 
  }
}


void ddRoot::init_dds()
{
  Initialize(ddRoot::init_dds);
  
  for (int i = 0; i < MAX_HASH; ++i ) {
    hash_element* he = dd_hash[i];
    while (he) {
      ddElement * dd = he->el;
      he = he->next;
      if (dd && (dd->get_id() > 0) ) {
	  if (dd_is_SWT_entity(dd->get_kind()))
	  {
	      dd->rem_rel_from(auto_rels);
	      if (dd->get_kind() != DD_MODULE)
		  dd->unset_is_def();
	      if (dd->get_kind() == DD_FUNC_DECL) {
		  if (dd->def) {
		      delete dd->def;
		      dd->def = 0;
		  }
		  for (ddElement *ch = checked_cast(ddElement, dd->get_first());
		       ch;) {
		      ddElement *nch = checked_cast(ddElement, ch->get_next());
		      remove_dd_el(ch);
		      ch = nch;
		  }
	      }
	  }
#ifdef XXX_assoc //	  else if (dd->get_kind() == DD_ASSOCLINK)
	  {	
	      if (dd_get_assoc_members(dd) == 0) {
		  if (is_model_build()) {
		      remove_hash(dd, dd->get_ddname(), dd->get_kind());
		      dd->set_local_hash(0);
		      obj_unload(dd);
		  }
		  else
		      dd->rem_rel_all();
	      }
#endif	  }
      }  // end of if(dd)
    }    // end of while
  }     // end of for
}

#define DD_DEFAULT_SIZE 50
#define LINK_SIZE 24

static int compute_one_dd_size(ddElement *dd)
{
  Initialize(compute_one_dd_size);
  if (dd == 0) return 0;
  Obj *ob;
  RelType * rel;
  Obj *rels;
  int current_size = DD_DEFAULT_SIZE;  // 50 is ok
  ForEach(ob, auto_rels) {
    rel = (RelType*) ob;
    rels = get_relation(rel, dd);
    if (rels == 0) continue;
    if (rels->collectionp()) {
      current_size += (((objSet *)rels)->size()) * LINK_SIZE;
    }
    else
      current_size += LINK_SIZE;
  }
  if (dd->get_ddname())
    current_size += strlen(dd->get_name());
  if (dd->get_kind() == DD_MACRO) {
    char const *df = dd->get_def_file();
    if (df)
      current_size += strlen(df);
  }
  return current_size;
}

int compute_dd_size(app * ah, int *num_syms)
{
  Initialize(compute_dd_size);
  if ( ah  == 0) return 0;
  ddRoot * dr = dd_sh_get_dr(ah);
  if (dr == 0) return 0;
  int cur_size = 0;
  for (int i = 0; i < MAX_HASH; ++i ) {
    hash_element* he = dr->dd_hash[i];
    while (he) {
      ddElement * dd = he->el;
      he = he->next;
      cur_size += compute_one_dd_size(dd);
      (*num_syms)++;
    }
  }
  return cur_size;
}

int ddElement::construct_symbolPtr(symbolPtr& sym) const
{
    sym = (Obj*)this;
    return 1;
}
