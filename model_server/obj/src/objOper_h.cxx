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
// objOper_h.C
//------------------------------------------
// synopsis : Contains methods related to propagation.
//
// classes :
//     objInserter, objRemover, objSplitter, objHiliter, objFocuser, objMerger
//     Hierarchical, app, commonTree, appTree.
//
// description : Methods for the above classes.
#include "objOperate.h"

#include "OperPoint.h"
#include "RTL.h"
#include "appMarker.h"
#include "projHeader.h"  //#include "assoc.h"
#include "cLibraryFunctions.h"
#include "machdep.h"
#include "msg.h"
#include "objCollection.h"
#include "objPropagate.h"
#include "reference.h"
#include "symbolArr.h"
#include "symbolPtr.h"
//#include "ldr.h"  //#include "view.h"

// new db interface

class db_buffer;
RelClass(ddElement);
RelClass(objRawApp);
void appTree::db_after_restore(db_buffer&)
{}

init_rel_or_ptr(Hierarchical,header, relationMode::D,commonTree,tree, relationMode::D);
///init_rel_or_ptr(ldr,ldr, relationMode::DMW,app,app, NULL);
//init_rel_or_ptr(view,view, relationMode::DMW,ldr,ldr, NULL);
///init_rel_or_ptr(ldrTree,ldrtree, relationMode::D,appTree,apptree, 0);
//init_rel_or_ptr(viewTree,viewtree, relationMode::D,ldrTree,ldrtree,NULL);

init_relation(CallFunctions, 1, NULL, CalledBy, MANY, NULL);
init_relation(RTLOfArgList, 1, 0, FuncArgListOf, 1, relationMode::D);
init_relation(cursor_of_view, 1, relationMode::D, view_of_cursor, 1, 0);

init_relation(rtl_of_app, MANY, NULL, app_of_rtl, MANY, NULL);

init_relation(boolean_of_rtl, MANY, NULL, booleaned_by_rtl, MANY, relationMode::M);
init_relation(styles_rtl_of_document, 1, relationMode::D, document_of_styles_rtl, 1, NULL);

init_relation(browselist_of_object, 1, relationMode::D, owner_of_browselist, 1, NULL);

init_abstract_relational(Hierarchical,commonTree);
init_abstract_relational(app,Hierarchical);
///init_abstract_relational(ldr,Hierarchical);
//init_abstract_relational(view,Hierarchical);

init_abstract_relational(commonTree,objTree);
init_abstract_relational(appTree,commonTree);
///init_abstract_relational(ldrTree,commonTree);
//init_abstract_relational(viewTree,commonTree);

// init soft associativity RelType's

// init_unirel(SoftAssoc, MANY, relationMode::DMW);

int is_real_rename( appTreePtr, char const *);
void commonTree::set_read_only( unsigned fl)
{
    Initialize(commonTree::set_read_only);

    read_only = fl;
    commonTreePtr cur = checked_cast(commonTree,this->get_first());
    for( ; cur; cur = checked_cast(commonTree,cur->get_next()) ) 
	cur->set_read_only( fl );
}

void commonTree::send_string(ostream& stream) const 
{
    stream << "send_string called" << endl;
}

boolean commonTree::is_RefNode() const { return 0; }
boolean commonTree::is_steRefNode() const { return 0; }
boolean commonTree::is_steRefTextNode() const { return 0; }
boolean commonTree::is_steRefGraNode() const { return 0; }
boolean commonTree::is_ldrRefGraNode() const { return 0; }
boolean commonTree::is_wrong_ref () const { return 0; }

HierarchicalPtr Hierarchical::get_header_virtual () const
{
    const HierarchicalPtr to_ret = (const HierarchicalPtr)this;
    return to_ret;
}

char const *Hierarchical::get_name() const { return title; }
int   Hierarchical::get_node_type() { return steREF; }

objPropagator *  objInserter::accept( RelationalPtr doc, int)
{
    Initialize(objInserter::accept);

    HierarchicalPtr  hd = checked_cast(Hierarchical, doc);
    objInserter * nextInserter = new objInserter(*this);
    hd->insert_obj(this, nextInserter);
    hd->notify_softassoc_insert (this, nextInserter);
    return nextInserter->type==NULLOP ? NULL : nextInserter;
}

objRemover::objRemover(objTreePtr x, char f ) : src_obj(x), flags(f) 
{
  src_parent = src_obj ? src_obj->get_parent() : 0;
}

objPropagator *  objRemover::accept(RelationalPtr  doc, int)
{
    Initialize(objRemover::accept);

    HierarchicalPtr  hd = checked_cast(Hierarchical, doc);
    objRemover * nextRemover = new objRemover(this->src_obj);  // GNU
    hd->remove_obj(this, nextRemover);
    hd->notify_softassoc_remove (this, nextRemover);
    return nextRemover->src_obj ? nextRemover : NULL;
}

objPropagator *  objMerger::accept(RelationalPtr  doc, int)
{
    Initialize(objMerger::accept);

    objMerger* yield;
    HierarchicalPtr  mrgr = checked_cast(Hierarchical, doc);
    objMerger * nextMerger = new objMerger(*this);
    yield = mrgr->merge_obj(this, nextMerger);
    mrgr->notify_softassoc_merge (this, nextMerger);
    return yield;
}

objPropagator *  objSplitter::accept(RelationalPtr  doc, int)
{
    Initialize(objSplitter::accept);

    objSplitter* yield;
    HierarchicalPtr  splt = checked_cast(Hierarchical, doc);
    objSplitter * nextSplitter = new objSplitter(*this);
    yield = splt->split_obj(this, nextSplitter);
    splt->notify_softassoc_split (this, nextSplitter);
    return yield;
}

objPropagator *  objHiliter::accept(RelationalPtr  doc, int)
{
    Initialize(objHiliter::accept);

    HierarchicalPtr  hd = checked_cast(Hierarchical, doc);
    objHiliter * nextHiliter =                       // GNU
	new objHiliter(src_obj, type);
    hd->hilite_obj(this, nextHiliter);
    return nextHiliter->type==0 ? NULL : nextHiliter;
}

objPropagator *  objFocuser::accept(RelationalPtr  doc, int)
{
    Initialize(objFocuser::accept);

    HierarchicalPtr  hd = checked_cast(Hierarchical, doc);
    objFocuser * nextFocuser =                      // GNU
	new objFocuser(src_obj, type);
    hd->focus_obj(this, nextFocuser);
    return nextFocuser->type == 0 ? NULL : nextFocuser;
}

objPropagator *  objAssigner::accept(RelationalPtr  doc, int)
{
    Initialize(objAssigner::accept);

    HierarchicalPtr  hd = checked_cast(Hierarchical, doc);
    objAssigner * nextAssigner =                      // GNU
	new objAssigner(src_obj, slot, operation);
    hd->assign_obj(this, nextAssigner);
    return nextAssigner->slot == NULL ? NULL : nextAssigner;
}

void obj_insert(appPtr root, objOperType type,
                appTreePtr x, appTreePtr y, void *data)
{
    obj_insert(root, type, x, y, data, NULL);
}

void obj_insert(appPtr root, objOperType type,
                appTreePtr x, appTreePtr y, void *data, void const *pt)
{
    obj_insert(root, type, x, y, data, pt, (objInserter *)NULL);
}

void obj_insert(appPtr root, objOperType type,appTreePtr x, appTreePtr y,
                void *data, void const *pt, objInserter *prev)
{
    Initialize(obj_insert);

#ifndef _WIN32
    DBG {
	msg("dt $1 tp $2 pt $3 prev $4\n") << (char *) data << eoarg << type << eoarg << pt << eoarg << prev << eom;
	obj_prt_obj(root);
	msg(" src ") << eom;
	obj_prt_obj(x);
	msg(" trg ") << eom;
	obj_prt_obj(y);
	msg("") << eom;
    }
#endif

    // make sure there is NO infinite loop

    objTree* wnode = (type == AFTER) ? y->get_parent() : y;
    if( (x != NULL && type != REMOVE) && 
        !(type == REPLACE && y == x)  &&
        !(type == REPLACE_REGION && y == x)  &&
        wnode->subtree_of(x))
    {
	Error(ERR_INPUT);
	return;
    }

    objInserter   ins_ptr(type, x, y, data, pt, prev);
    obj_modify(*root, &ins_ptr);
}

void obj_remove(appPtr root,  appTreePtr node)	// Redundant: to be deleted
{
    obj_remove(root, node, 0);
}

void obj_remove(appPtr root,  appTreePtr node, int flags)
{
    objRemover ro(node, flags);

    start_transaction()
    {
	obj_modify(*root, &ro);
	// Dispose only if it is removing from own APP
	if(is_appTree(node) && 
           node->get_header() == 0 &&
           (flags & 0x01) == 0)
	  obj_dispose(node);
    }  end_transaction();

}

void obj_assign(appPtr root, appTreePtr x, steSlot* slot, short op)
{
    objAssigner oc(x, slot, op);
    obj_walk(*root, &oc, MANY);
}

// Merge protocol: each node in the path from the bottom's
// parent to top inclusively is duplicated and inserterted
// AFTER the original node.

// Following code hopefully will be deleted after merging all obj_ functions 
// in one obj_insert (BF 07/02/92).

// Split protocol: each node in the path from the bottom's
// parent to top inclusively is duplicated and inserterted
// AFTER the original node.

void obj_split(appPtr root, appTreePtr bottom, appTreePtr top)
{
    objSplitter os(bottom, top);

    obj_modify(*root, &os);

}


//------------------------------------------
// Member functions for class commonTree
//------------------------------------------

HierarchicalPtr commonTree::get_header_virtual() const
{ 
   Initialize(commonTree::get_header);

   return tree_get_header(checked_cast(commonTree,get_root()));
}

commonTree::commonTree() : type(0), extype(0), raw_flag(0), read_only(0),
    smod_vision(0), reserv(0), language(smt_UNKNOWN)
{}

char const *commonTree::get_name() const
{
    Initialize(commonTree::get_name);

    return checked_cast(Hierarchical,get_header()) -> get_name();
}

objArr* commonTree::get_tok_list(int)
{
    return NULL;
}

void commonTree::commonTree_stab1_virtual ()
{ 
   Initialize(commonTree::commonTree_stab1_virtual);
   Error(NOT_IMPLEMENTED_YET);
}

void commonTree::commonTree_stab2_virtual ()
{ 
   Initialize(commonTree::commonTree_stab2_virtual);
   Error(NOT_IMPLEMENTED_YET);
}

void commonTree::commonTree_stab3_virtual ()
{ 
   Initialize(commonTree::commonTree_stab3_virtual);
   Error(NOT_IMPLEMENTED_YET);
}

void commonTree::commonTree_stab4_virtual ()
{ 
   Initialize(commonTree::commonTree_stab4_virtual);
   Error(NOT_IMPLEMENTED_YET);
}

void commonTree::commonTree_stab5_virtual ()
{ 
   Initialize(commonTree::commonTree_stab5_virtual);
   Error(NOT_IMPLEMENTED_YET);
}

void commonTree::commonTree_stab6_virtual ()
{ 
   Initialize(commonTree::commonTree_stab6_virtual);
   Error(NOT_IMPLEMENTED_YET);
}

void commonTree::commonTree_stab7_virtual ()
{ 
   Initialize(commonTree::commonTree_stab7_virtual);
   Error(NOT_IMPLEMENTED_YET);
}

void commonTree::commonTree_stab8_virtual ()
{ 
   Initialize(commonTree::commonTree_stab8_virtual);
   Error(NOT_IMPLEMENTED_YET);
}

//------------------------------------------
// Member functions for class Hierarchical
//------------------------------------------

Hierarchical::Hierarchical()
{
    valid_filename = 0;
    date = 0;
    modified = imported = 0;
    src_modified = reserve_space = 0;
}

Hierarchical::Hierarchical(char const *name) : title(name)
{
    valid_filename = 0;
    date = 0;
    modified = imported = 0;
    src_modified  = reserve_space = 0;
}

void Hierarchical::set_name(char const *name)
{
    title = name;
}

objMerger* Hierarchical::merge_obj(objMerger *, objMerger *)
{
    Initialize(Hierarchical::merge_obj);
    Error(NOT_IMPLEMENTED_YET);
    return (objMerger *)NULL;
}

objSplitter* Hierarchical::split_obj(objSplitter *, objSplitter *)
{
    Initialize(Hierarchical::split_obj);
    Error(NOT_IMPLEMENTED_YET);
    return (objSplitter *)NULL;
}

int Hierarchical::regenerate_obj(objTreePtr)
{
    return 0;
}

void Hierarchical::set_type( int t )
{
    type = t;
}

int Hierarchical::get_type()
{
    return type;
}

//------------------------------------------
// [static] Hierarchical::prop_rtl_contents()
//------------------------------------------

symbolArr& Hierarchical::prop_rtl_contents() {
   Initialize(Hierarchical::prop_rtl_contents);

   return prop_rtl()->rtl_contents();
}

//------------------------------------------
// [static] Hierarchical::clear_prop_rtl()
//------------------------------------------

void Hierarchical::clear_prop_rtl() {
   Initialize(Hierarchical::clear_prop_rtl);

// NOTE: this operation uses symbolArr::removeAll() instead of RTLNode::clear()
// because 1) this is a private RTL and doesn't need to worry about selections
// and the other operations performed in RTLNode::clear(), and 2) RTLNode::clear()
// does an obj_insert(), which could result in an infinite loop.

   prop_rtl()->rtl_contents().removeAll();
}

//------------------------------------------
// Hierarchical::add_to_prop_rtl(Hierarchical*)
//------------------------------------------

void Hierarchical::add_to_prop_rtl() {
   Initialize(Hierarchical::add_to_prop_rtl);

   prop_rtl()->rtl_insert(this, true);
}

//------------------------------------------
// [static] Hierarchical::prop_rtl()
//------------------------------------------

RTLNodePtr Hierarchical::prop_rtl() {
   Initialize(Hierarchical::prop_rtl);

   if (!prop_rtlnode) {
      RTLPtr rh = db_new(RTL, ("Modified Headers Pending Notification"));
      prop_rtlnode = checked_cast(RTLNode, rh->get_root());
   }
   return prop_rtlnode;
}

RTLNodePtr Hierarchical::prop_rtlnode;

void app::set_filename (char const *file_name, appType atype, smtLanguage lang)
{
    Initialize(app::set_filename(char const *file_name, int lang));

    set_type(atype);
    language = lang;
    set_filename(file_name);
}

void app::set_filename (char const *fn)
{
    Initialize(app::set_filename);
    if(fn && fn[0]) {
	filename = fn;
    } else {
	filename = 0;
    }
}

char const *app::get_phys_name ()
{
    return ph_name;
}

char const *app::get_filename ()
{
   return filename;
}

int app::get_language()
{
  return language;
}

int Hierarchical::save_app()
{
   return 0;
}

//------------------------------------------
// Member functions of class appTree
//------------------------------------------

appTree::appTree()
{}

appTree::~appTree()
{}

void appTree::description(genString& str)
{
//  str = "appTree";
    str.put_value (0);
}

// copies appTree for specified header.
appTreePtr appTree::rel_copy_for_header (appPtr header, objCopier*)
{
    Initialize(appTree::rel_copy_for_header );

    // if src and destination headers are same return
    if(header == get_header())
	return this;
    else
	return checked_cast(appTree,rel_copy());
}

// Get main header (for save/restore identification)
// by default the main header does not exist
app * appTree::get_main_header()
{
    return 0;
}

// Non-member functions

ostream &operator << (ostream& stream, commonTree& t)
{
    t.send_string(stream);
    return stream;
}

objInserter::objInserter(const objInserter &os)
{
   objInserter *old = (objInserter *)&os;
   type     = old->type;
   src_obj  = old->src_obj;
   targ_obj = old->targ_obj;
   data     = old->data;
   pt       = old->pt;
   prev_ins = old;
}

// Report "remove" operation to appMarker subsystem 
void appTree::report_remove(int t)
{
    Obj * m;
    Obj * am = get_relation(marker_of_node, this); // Get set of markers
    if(am)
    {
	objSet set = am;
	ForEach(m, set)
	{
	    ((appMarker *)m)->report_remove(t);
	}
    }
}

// Report "split" operation to appMarker subsystem 
void appTree::report_split(int b, appTree * t1, appTree * t2)
{
    Obj * m;
    Obj * am = get_relation(marker_of_node, this); // Get set of markers
    if(am)
    {
	objSet set = am;
	ForEach(m, set)
	{
	    ((appMarker *)m)->report_split(b, t1, t2);
	}
    }
}

int appTree::get_linenum () const
{
    return 0;
}

// temp to avoid recompilation
char const *Hierarchical::get_filename()
{
    Initialize(Hierarchical::get_filename);

    Assert(is_app(this));
    return appPtr(this)->app::get_filename();
}

void Hierarchical::set_filename(char const *fn)
{
    Initialize(Hierarchical::set_filename);

    Assert(is_app(this));
    appPtr(this)->app::set_filename(fn);
}

void ste_interface_modified( HierarchicalPtr, boolean );

int Hierarchical::is_modified()     
{
    return modified;
}

void Hierarchical::clear_modified() 
{ 
    Initialize (Hierarchical::clear_modified);

    if (modified || imported)
	ste_interface_modified (this, 0);     

    if (! modified) return;

    modified = 0;

    add_to_prop_rtl();
}

void Hierarchical::set_modified()   
{
    Initialize (Hierarchical::set_modified);

    if (!(modified || imported))
	ste_interface_modified (this, 1);     

    if (    modified) return;

    modified = 1;

    add_to_prop_rtl();
}

int Hierarchical::is_src_modified()     
{
    return src_modified;
}

void Hierarchical::clear_src_modified() 
{ 
    Initialize (Hierarchical::clear_src_modified);

    if (!    src_modified) return;

    src_modified = 0;

    add_to_prop_rtl();
}

void app_set_just_restored(Hierarchical * ah, int val);

void Hierarchical::set_src_modified()   
{ 
    Initialize (Hierarchical::set_src_modified);

    if (src_modified) return;

    src_modified = 1;
    app_set_just_restored (this, 0);

    add_to_prop_rtl();
}

int Hierarchical::is_imported()     
{
    return imported;
}

void Hierarchical::clear_imported() 
{
    Initialize(Hierarchical::clear_imported);

    if (!    imported) return;

    imported = 0;

    add_to_prop_rtl();
}

void Hierarchical::set_imported()   
{
    Initialize(Hierarchical::set_imported);

    if (    imported) return;

    imported = 1;

    add_to_prop_rtl();
}

int app_construct_symbolPtr(const app*, symbolPtr& sym);

int app::construct_symbolPtr(symbolPtr& sym) const 
{
  return app_construct_symbolPtr(this, sym);
}

int appTree_construct_symbolPtr(const appTree*, symbolPtr& sym);

int
appTree::construct_symbolPtr(symbolPtr& sym) const
{
  return appTree_construct_symbolPtr(this, sym);
}

ddKind app::get_symbolPtr_kind () const 
{
    return DD_UNKNOWN; 
}

int app::is_symbolPtr_def() const 
{
    return 0; 
}

app *app::get_symbolPtr_def_app()
{
    return this; 
}

void app::set_phys_name (char const *fn, projNode* pr)
{
    Initialize(app::set_phys_name(charPtr,projNodePtr));

    ph_name = fn;
    if (pr) 
	projectNode_put_appHeader(pr,this);
}

void app::set_phys_name (char const *fn)
{
    Initialize (app::set_phys_name);

    projNode *pr;

    // if new name same as old, no real work to do
    if ((char *)ph_name && fn && !strcmp(ph_name,fn)) {
	return;
    }
    // if there already was a filename here, carefully forget it
    if ((char *)ph_name) {
	pr = projHeader::fn_to_project(ph_name, 1);
	if (pr) projectNode_rem_appHeader(pr,this);
    }
    if(fn && fn[0]) {
	ph_name = fn;
	pr = projHeader::fn_to_project(ph_name, 1);
	if (pr) projectNode_put_appHeader(pr,this);
    } else {
	ph_name = 0;
    }
}

class dbNull : public objTree {
  public:
    define_relational(dbNull, objTree);
    virtual void  print(ostream& st = cout, int level = 0) const;
};

generate_descriptor(dbNull, objTree);


static dbNull db_null_orig;
static dbNull db_null_copy;

Relational* DB_NULL = &db_null_orig;

#undef implement_new
#define implement_new(Clas,sz) \
  {return ::operator new(sz);}

#undef implement_delete
#define implement_delete(Clas,ptr) \
 {  OS_dependent::bcopy (&db_null_copy, ptr, sizeof (dbNull));}

init_relational(dbNull, objTree);

void dbNull::print(ostream& st, int) const 
{
  st << "Unresolved Relation";
}
