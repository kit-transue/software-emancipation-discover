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

#include "genError.h"
#include <ddict.h>
#include "symbolPtr.h"
#include <objOperate.h>
#include "ldrNode.h"
#include "ldrSelection.h"
#include "symbolLdrHeader.h"
#include "symbolLdrTree.h"
#include "lde-hierarchy.h"
// #include "lde_call_tree.h"
#include "lde_symbol.h"
#include "OperPoint.h"


RelClass(ddElement);
RelClass(smtTree);

ddElementPtr smt_get_dd(smtTreePtr);
// xrefEntryPtr get_xref_entry(ddElementPtr);

const char* xref_c_proto(const symbolPtr &src);
extern void modify_tree_names(symbolLdrTree *, int );

init_relational(symbolLdrHeader,ldrHeader);

symbolLdrHeader::symbolLdrHeader (RTLPtr rtl, ddKind k, 
				  linkType ch_lt, linkType par_lt, int level)
: obj_type (k), child_lt (ch_lt), parent_lt (par_lt), disp_arg(0), 
    default_no_levels (level)
{
   Initialize(symbolLdrHeader::symbolLdrHeader);

   symbolLdrTree* root = NULL;
   ldr_put_app(this, rtl);
   RTLNodePtr app_root = checked_cast(RTLNode, rtl->get_root());
   symbolArr& array = app_root->rtl_contents ();
   Assert(array.size ());
   fsymbolPtr  xr = array[0].get_xrefSymbol();
   IF (xr->xrisnull())
       return;
   array.removeAll ();
   array.insert_last (xr);
   root = db_new (symbolLdrTree, (xr, child_lt));
   this->put_root (root);
   lde_extract_symbol (root, child_lt, k, 0, 0);
   root->set_appTree (app_root);
}

symbolLdrHeader::symbolLdrHeader (RTLPtr rtl)
{ 
    Initialize(symbolLdrHeader::symbolLdrHeader);

    symbolLdrTree* root = NULL;
    ldr_put_app(this, rtl);
    RTLNodePtr app_root = checked_cast(RTLNode, rtl->get_root());
    symbolArr& array = app_root->rtl_contents ();
    Assert(array.size ());
    fsymbolPtr  xr = array[0].get_xrefSymbol();
    IF (xr->xrisnull())
	return;
    array.removeAll ();
    array.insert_last (xr);
    default_no_levels = MAX_LEVEL_DEPTH;
    disp_arg = 0;

    char* name_tmp = xr.get_name();
    switch (xr.get_kind ()) {
      case DD_FUNC_DECL:
	obj_type = DD_FUNC_DECL;
	child_lt = is_using;
	parent_lt = used_by;
//	name_tmp = "Call Tree";
	break;

      case DD_CLASS:	
	obj_type = DD_CLASS;
	child_lt = has_subclass;
	parent_lt = has_superclass;
//	name_tmp = "Inheritance Tree";
	break;

      case DD_MODULE:
      case DD_PROJECT:
	obj_type = DD_UNKNOWN;
	child_lt = is_including;
	parent_lt = included_by;
//	name_tmp = "Inclusion Tree";
	break;

      default:
	obj_type = DD_UNKNOWN;
	child_lt = is_using;
	parent_lt = used_by;
//	name_tmp = "Usage Tree";
	break;
    }

    rtl->set_name (name_tmp);

    root = db_new (symbolLdrTree, (xr, child_lt));
    this->put_root (root);
    lde_extract_symbol (root, child_lt, obj_type, 0, 0);
    // Now add special attributes for duplicated nodes
    symbolArr contents;
    lde_setup_recursion_attributes(root, contents);
    root->set_appTree (app_root);
}

#if 0
symbolLdrHeader::symbolLdrHeader (appPtr app_header, appTreePtr sub_tree)
{
   Initialize (symbolLdrHeader::symbolLdrHeader);


   set_type(Rep_Tree);
   if (sub_tree == NULL)
     sub_tree = checked_cast(appTree, app_header->get_root());
   ldr_put_app(this, app_header);
   put_root(checked_cast(objTree, lde_smt_tree(sub_tree, 0)));
   return;
}
#endif
// Optimal implementation  of  insert_obj  is   too
// complicated for now --  just redo the whole ldr.
// Once ldr for  Tree  is settled, this method
// should be rewritten to run faster.

void symbolLdrHeader::insert_obj(objInserter* ,
				 objInserter* new_inserter)
{
   Initialize(symbolLdrHeader::insert_obj);

   redo_insert(new_inserter);
}

// Real removal is done by lde_detach_tree
// which knows more about the structure of  the ldr
// tree.

void
symbolLdrHeader::remove_obj(objRemover* old_remover, objRemover* new_remover)
{
   Initialize(symbolLdrHeader::remove_obj);

   ldrNodePtr src_ldr;

// the only use of this call is with ldr node itself since there is
// no appTree attached

/*   src_ldr = checked_cast(ldrNode,find_ldr(old_remover->src_obj)); */

   src_ldr = checked_cast(ldrNode, old_remover->src_obj); 
   if (src_ldr != NULL) {
      src_ldr->remove_from_tree();
      new_remover->src_obj = src_ldr;
   }

}

void symbolLdrHeader::assign_obj(objAssigner* /* old_assigner */, objAssigner*  /* new_assigner */)
{
#if 0
    Initialize(symbolLdrHeader::assign_obj);

    objTree *src_obj = old_assigner->src_obj;
    ldrTreePtr src_ldr = checked_cast(ldrTree,find_ldr(checked_cast(appTree,src_obj)));
    if (!src_ldr && src_obj && is_smtTree(src_obj))
    {
      ddElementPtr dd = smt_get_dd(checked_cast(smtTree,src_obj));
      xrefEntryPtr xe = dd ? get_xref_entry(dd) : NULL;
      src_ldr = xe ? checked_cast(ldrTree,find_ldr(checked_cast(appTree,xe))) : NULL;
    }
    new_assigner->slot = (src_ldr ? old_assigner->slot : 0);
    new_assigner->src_obj = src_ldr;
#endif
}

// Instead   of  making local    tree  surgery, the
// following functions  just rederives   the  whole
// tree. It then  changes the inserter  to  that of
// replace of  the  whole tree  so  the  change  is
// propagated properly.

void symbolLdrHeader::redo_insert(objInserter* new_inserter)
{
    Initialize(symbolLdrHeader::redo_insert);

    RTL* rtl = checked_cast (RTL, ldr_get_app (this));
    RTLNodePtr app_root = checked_cast (RTLNode, rtl->get_root ());
    symbolArr& array = app_root->rtl_contents ();
    if (!array.size ())
	return;
    fsymbolPtr  xr = array[0].get_xrefSymbol();
    array.removeAll ();
    array.insert_last (xr);
    IF (xr->xrisnull())
	return;

    ldrNode* old_root = checked_cast(ldrNode, get_root());
    symbolLdrTree* new_root = db_new (symbolLdrTree, (xr, child_lt));
    this->put_root(new_root);
    lde_extract_symbol (new_root, child_lt, obj_type, 0, 0);
    objDeleter exterminator;
    obj_delete (*(RelationalPtr) old_root, &exterminator);
    new_root->set_appTree (app_root);
    new_inserter->type = REPLACE;
    new_inserter->src_obj = new_root;
    new_inserter->targ_obj = old_root;
}

void symbolLdrHeader::build_selection(const ldrSelection& ldr_inp, OperPoint& app_point)
{
   Initialize(symbolLdrHeader::build_selection);

   ldrNodePtr ldr_node = ldr_inp.ldr_symbol;
   app_point.type = AFTER;
   symbolLdrTree* ldr_parent = (symbolLdrTree*)ldr_node->get_parent ();
   app_point.node = ldr_parent->get_xref ();
}

void symbolLdrHeader::set_default_no_levels (int ll)
     { default_no_levels = ll; } 


//  walks thru all ldrFuncCallNode's and add or remove args to function name

void symbolLdrHeader::modify_names () {

    Initialize(symbolLdrHeader::modify_names);

    symbolLdrTree* root = checked_cast(symbolLdrTree, get_root());
    modify_tree_names(root, disp_arg);    
}
 
void modify_tree_names(symbolLdrTree *root, int flag) {

    Initialize (modify_tree_names);

    ldrFuncCallNode *first = checked_cast(ldrFuncCallNode, root->get_first());
    
    if (first) {
	symbolPtr &fn_sym   = root->get_xref ();
	genString full_name = fn_sym.get_name();
	char *p             = strchr((char *)full_name, '(');
	if(p == NULL){
	    char *c_args = (char *)xref_c_proto(fn_sym);
	    full_name   += c_args;
	}
        first->modif_arg((char *)full_name, flag);

        objTree *next = first->get_next();

        for (; next; next = next->get_next ()) 
               if ( is_symbolLdrTree(next) ) {
                    symbolLdrTree *tmp = checked_cast (symbolLdrTree, next);
                    modify_tree_names( tmp, flag);
               }
    }
}
 

/*
   START-LOG-------------------------------------------

   $Log: symbolLdrHeader.cxx  $
   Revision 1.10 1998/03/31 11:13:35EST azaparov 
   
// Revision 1.15  1994/02/25  17:08:33  davea
// Bug track: 6476
// In symbolLdrHeader::redo_insert(), add test:
// If nothing in the rtl, simply return.
//
// Revision 1.14  1993/12/16  13:49:40  azaparov
// Bug track: 4418
// Fixed bug 4418
//
// Revision 1.13  1993/06/03  16:29:54  sergey
// Put distinguishable name in rtl for call trees. Fix of bug #2828.
//
// Revision 1.12  1993/05/23  21:17:29  sergey
// Changed default in call tree for displaying arguments tp no display.
//
// Revision 1.11  1993/04/14  18:44:10  sergey
// Added show/hide args support for C++ call tree. Fixed bug #3305.
//
// Revision 1.10  1993/03/15  21:51:26  davea
// change xrefSymbol* to fsymbolPtr
//
// Revision 1.9  1993/03/05  21:51:42  sergey
// Added set_default_no_levels method.
//
// Revision 1.8  1993/03/03  21:35:39  aharlap
// deleted unnecessary cast symbolPtr to Obj*
//
// Revision 1.7  1993/02/26  03:06:18  aharlap
// *** empty log message ***
//
// Revision 1.6  1993/02/17  22:24:40  kol
// 
//
// Revision 1.5  1993/02/17  22:22:07  kol
// *** empty log message ***
//
// Revision 1.4  1993/02/17  22:19:40  kol
// *** empty log message ***
//
// Revision 1.3  1993/02/17  20:27:52  sergey
// Fix bug #1887.
//
// Revision 1.2  1993/02/17  20:20:35  sergey
// Minor clean up.
//
// Revision 1.1  1993/02/17  19:58:03  sergey
// Uncommented and corrected remove_obj. Part of bug #1887.
//
// Revision 1.2  1993/01/03  20:39:53  aharlap
// impemented symbolLdrHeader::insert_obj
//
// Revision 1.1  1992/12/21  23:05:34  aharlap
// Initial revision
//

   END-LOG---------------------------------------------

*/




