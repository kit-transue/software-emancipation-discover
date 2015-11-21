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
#include "objOperate.h"
#include "ldrError.h"
#include "ldrNode.h"
#include "ldrSmtHierarchy.h"
#include "lde-hierarchy.h"
#include "lde_smt_hierarchy.h"
#include "viewNode.h"
#include "viewGraHeader.h"

#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <strstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

bool smtTree_get_symbol(smtTree*node, symbolPtr& sym);
smtTree* symbol_get_smtTree(symbolPtr& sym);

init_relational(ldrSmtHierarchy,ldrHierarchy);

objTree * smt_find_ldr(ldr *, objTree *);
ldrSmtHierarchy::ldrSmtHierarchy(smtHeaderPtr h, smtTreePtr t)
  {
  Initialize(ldrSmtHierarchy::ldrSmtHierarchy);

  smtTreePtr  root = checked_cast(smtTree,h->get_root());
  ldr_put_app(this, h);
  if(!t)
    t = root;
  Assert(smtTree_get_symbol(t, symbol));
  put_root(checked_cast(objTree, lde_extract_smt_hierarchy(t)));
  set_type(Rep_VertHierarchy);
  }

ldrTree * ldrSmtHierarchy::generate_tree(appTree * t, int )
  {
  Initialize(ldrSmtHierarchy::generate_tree);
  return lde_extract_smt_hierarchy(checked_cast(smtTree,t));
  }

void ldrSmtHierarchy::insert_obj(objInserter* oi,
				 objInserter* ni)
{
    Initialize(ldrSmtHierarchy::insert_obj);
    smtTree* smt_root = symbol_get_smtTree(symbol);
    ldrNode* new_root = NULL;
    ldrNode * old_root = checked_cast(ldrNode, get_root());
    Assert(old_root);
    if (smt_root)
      {
	new_root = lde_extract_smt_hierarchy(smt_root);
      }
    ni->src_obj = new_root;

    if(!new_root){
      ni->type = NULLOP;
      obj_delete(this);
      return;
    }

    ni->targ_obj = old_root;
    ni->type = REPLACE;
    ni->data = 0;

    put_root(new_root);
    
    obj_delete(old_root);
}


void ldrSmtHierarchy::remove_obj(objRemover*  old_remover,
				 objRemover*  new_remover)
{
    Initialize(ldrSmtHierarchy::remove_obj);
    objTreePtr src_ldr;
    if (src_ldr = ldrHeader::find_ldr (old_remover->src_obj)) {
	lde_detach (checked_cast(ldrNode,src_ldr));
	new_remover->src_obj = src_ldr;
    } else {
	smtTree* par = checked_cast(smtTree,old_remover->src_parent);
	if (par) {
            objInserter mod (REPLACE, par, par, NULL);
            obj_modify (*this, &mod);
	    new_remover->src_obj = NULL;
	} else
	    Error(MISSING_LDR);
    }
}

void ldrSmtHierarchy::assign_obj(objAssigner *old_assigner,
				 objAssigner *new_assigner)
{
    Initialize(ldrSmtHierarchy::assign_obj);

    smtTreePtr smt_tree = checked_cast(smtTree,old_assigner->src_obj);
    ldrTreePtr src_ldr = NULL;
    
    while (smt_tree && !(src_ldr=checked_cast(ldrTree,find_ldr(checked_cast(appTree,smt_tree)))))
	smt_tree = checked_cast(smtTree,smt_tree->get_parent());
    
    new_assigner->slot = (src_ldr ? old_assigner->slot : 0);
    new_assigner->src_obj = src_ldr;
    
}

static void out_string(smtTree *, ostream &);
void ldrSmtHierarchyNode :: send_string(ostream& s) const
{
    ostrstream temp;
    Initialize(ldrSmtHierarchy::send_string);
    smtTree * t = checked_cast(smtTree,get_appTree());
    char *v;
    if(t == 0) Return;
    switch(string_mode)
    {
      case full:
	out_string(t, s);
	break;
      case first:
	t = checked_cast(smtTree,t->get_first());
	if(t) out_string(t, temp);
	temp << ends;
	v = temp.str();
	{
	int l = strlen(v) - 1;
	
	// This ending space may be needed.
	//      if(v[l] == ' ') v[l--] = 0;
	if(v[l] == '{') v[l] = 0;
        }
	   s << v;
	   delete v;
	   break;
       }
	Return
	}
    
    static void out_string(smtTree * t, ostream & s)
    {
        Initialize(out_string);

	if(t)
	{
	    for(smtTree * x = checked_cast(smtTree,t->get_first_leaf());
		x && x->subtree_of(t);
		x = checked_cast(smtTree,x->get_next_leaf()))
	    {
		x->send_string(s);
	    }
	}
    }
    
    objArr * ldrSmtHierarchyNode :: get_tok_list(int mode)
    {
	Initialize(ldrSmtHierarchyNode::get_tok_list);
	smtTree * t = checked_cast(smtTree,get_appTree());
	if(t == 0 || string_mode == null)
	    ReturnValue(0);
	if(mode == 0 || string_mode == full)
	    ReturnValue(t->get_tok_list());
	t = checked_cast(smtTree,t->get_first());
	objArr * tarr = t->get_tok_list();
	int i = tarr->size();
	if(i > 0)
	{
	    t = checked_cast(smtTree,((*tarr)[i - 1]));
	    smtHeader * h = checked_cast(smtHeader,t->get_header());
	    if(h->srcbuf[t->tbeg] == '{')
	       tarr->remove(t);
	   }
	    ReturnValue(tarr);
	}
	
	/*
	  START-LOG-------------------------------------------
	  
	  $Log: ldrSmtHierarchy.cxx  $
	  Revision 1.7 2000/07/10 23:06:06EDT ktrans 
	  mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.16  1994/07/26  22:35:44  boris
Bug track: 7617
Fixed crash in uotline view while editting SMODE file

Revision 1.2.1.15  1993/08/25  01:12:53  boris
Fixed #4475 with not regenerating outline view on REPARSE

Revision 1.2.1.14  1993/08/19  13:43:56  sergey
Corrected REMOVE and AFTER cases in insert_obj. Fixed bug #4135.

Revision 1.2.1.13  1993/08/03  23:32:15  boris
added REPLACE_REGION functionality

Revision 1.2.1.12  1993/06/23  18:01:25  boris
Fixed bug #3804 with outline view blinking

Revision 1.2.1.11  1993/06/23  17:33:41  bakshi
bracket var decls inside case stmts for c++3.0.1 comformity

Revision 1.2.1.10  1993/06/14  23:43:11  sergey
Removed an old ldr node in insert_obj on AFTER/FIRST. Fixed bug #1987.

Revision 1.2.1.9  1993/05/14  17:55:59  sergey
Correction in insert_obj (AFTER case) to fix bug #3058.

Revision 1.2.1.8  1993/03/26  04:39:05  boris
Skip Smod Split call

Revision 1.2.1.7  1993/02/19  21:04:10  boris
Fixed bug #2500

Revision 1.2.1.6  1993/02/12  17:09:13  boris
Added smt cut without view regeneration

Revision 1.2.1.5  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.4  1992/11/02  18:16:48  aharlap
changed ldrSmtHierarchy::remove_obj

Revision 1.2.1.3  92/10/28  14:22:35  oak
Commented out a line that removes the trailing
space from a send_string selection.  This fixes
a major crash with the hierarchy mode.

	  Revision 1.2.1.2  92/10/09  19:44:00  jon
	  RCS History Marker Fixup
	  
	  
	  
	  END-LOG---------------------------------------------
	  
	  */
