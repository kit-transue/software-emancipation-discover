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
#include <genError.h>
#include <steRefNode.h>

#include "reference.h"
#include "lde_reference.h"
#include "lde_smt_text.h"
#include "ste_smod.h"

// Attention!
// This LDR generation function is table driven;
// all modifications should be reacheble by modification of the table!
// Look carefully before touching a program code!

// LDR Generation Table
// (1) type     - SMT node type
// (2) stype    - generated LDR node type
//                steNULL a real splice (LDR will not be jenerated
//	          for this node, but for children)	
// (3) s_header - 1 means that the first child will be a header
// (4) s_footer - 1 means that the last child will be a footer
// (5) s_plain  - 1 means that all children will be single paragraph (stePAR)

static symtab stab[] =
  {
    {SMT_file,          steSTR,		0,0,0},
    {SMT_macrocall,	stePAR,		0,0,0},
    {SMT_macrostmt,	steSTR,		0,0,1},
    {SMT_stmt,          steSTR,		0,0,1},
    {SMT_empty,         steSTR,		0,0,1},
    {SMT_comment,       steSTR,		0,0,1},
    {SMT_m_include,     steSTR,		0,0,1},
    {SMT_m_define,     	steSTR,		0,0,1},
    {SMT_m_if,     	steSTR,		0,0,1},
    {SMT_m_else,     	steSTR,		0,0,1},
    {SMT_m_endif,     	steSTR,		0,0,1},
    {SMT_m_undef,     	steSTR,		0,0,1},
    {SMT_m_pragma,     	steSTR,		0,0,1},
    {SMT_m_gen,     	steSTR,		0,0,1},
    {SMT_break,         steSTR,		0,0,1},
    {SMT_continue,      steSTR,		0,0,1},
    {SMT_return,        steSTR,		0,0,1},
    {SMT_goto,          steSTR,		0,0,1},
    {SMT_label,         steSTR,		0,0,1},
    {SMT_cdecl,		steSTR,		1,0,0},
    {SMT_decl,		steSTR,		0,0,1},
    {SMT_list_decl,	steSTR,		0,0,1},
    {SMT_nstdloop,	steSTR,		1,0,0},
    {SMT_switch,	steSTR,		1,0,0},
    {SMT_clause,	steSTR,		1,0,0},
    {SMT_then_clause,	steSTR,		1,0,0},
    {SMT_else_clause,	steSTR,		1,0,0},
    {SMT_else_if_clause,steSTR,		1,0,0},
    {SMT_case_clause,	steSTR,		1,0,0},
    {SMT_block,		steSTR,		1,1,0},
    {SMT_cbody,		steSTR,		1,1,0},
    {SMT_ebody,		steSTR,		1,1,0},
    {SMT_if,		steSTR,		0,0,0},
    {SMT_fdef,		steSTR,		1,0,0},
    {SMT_fdecl,		steSTR,		0,0,1},
    {SMT_group,		stePAR,		0,0,0},    
//    {0, 		steNULL,	0,0,0}
    {0, 		steSPLICE,	0,0,0}
  };

static steDisplayNodePtr extract(smtTreePtr, steNodeType);
void lde_smt_decorate(steDisplayNode *);

// Extract LDR for SMT text (STE) representation
steDisplayNodePtr lde_extract_smt_text(smtTreePtr tr, steNodeType type)
  {
  Initialize(lde_extract_smt_text);
  steDisplayNode * n = extract(tr, type);
  if(n->get_node_type() == steNULL)
    n->set_node_type(steSTR);
  lde_smt_decorate(n);
  ReturnValue(n);
  }

// Finds table entry for type of given node
//
//
symtab lde_smt_get_type( smtTreePtr tr, steNodeType& type, int& itab ) 
{
 Initialize(lde_smt_get_type);
  itab = -1;
  if(type == steSTR){
    for(itab=0; stab[itab].type && stab[itab].type != tr->type; itab++);
    type = stab[itab].stype;
  }
  if ( itab > 0 )
     return stab[itab];
  else
     return stab[0];
}

static steDisplayNodePtr extract(smtTreePtr tr, steNodeType type)
  {
  Initialize(extract);

  steDisplayNode * nl ;
  tr->ndm = 0;
  if (is_ReferenceNode(tr))
  {
#if 0
//  smtTree* refnode;
//  if (refnode = tr->get_refnode()) {
//      nl = extract_reference (checked_cast(steRefNode, refnode));
//      nl -> set_appTree (refnode); 
//      return nl;
#endif
      nl = extract_reference_ldr_with_header(tr);
      return nl;
  }

  nl = db_new(steDisplayNode,());
  nl->smod_vision = SmodFull;

  if(tr->type == SMT_token)
  {
      nl->set_appTree(tr);
      return nl;
  }

  int itab;  
  symtab my_tab = lde_smt_get_type( tr, type, itab );
  nl->set_node_type(type);

  // Create subtree
  steDisplayNode * pl;
  smtTree * t;
  int regular = (type == steREG || type == stePAR || type == steHEAD ||
                 type == steFOOT || (itab >= 0 && my_tab.s_plain));

  for(pl = 0, t =  checked_cast(smtTree,tr->get_first());
      t; 
      t = checked_cast(smtTree,t->get_next()))
    {
    steNodeType ntype;
    if(regular)
      ntype = steREG;
    else if(itab >= 0 && my_tab.s_header && pl == 0)
      ntype = steHEAD;
    else if(itab >= 0 && my_tab.s_footer && t->get_next() == 0)
      ntype = steFOOT;
    else
      ntype = steSTR;

    steDisplayNode * l = extract(t, ntype);

    // Temporary  cludge while steFOOT not implemented it the STE staff
    if(l->get_node_type() == steFOOT)
      {
      steDisplayNode * tmp = db_new(steDisplayNode,());
      tmp->set_node_type(steSTR);
      tmp->put_first(l);
      l->set_node_type(steHEAD);
      l = tmp;
      }

    if(pl)
      pl->put_after(l);
    else
      nl->put_first(l);
    pl = l;

    // Splice node if type = 0
    if(l->get_node_type() == steNULL)
      {
      for(objTree * temp = l->get_first(); temp; temp = temp->get_next())
        pl = checked_cast(steDisplayNode,temp);
      l->splice();
      obj_delete(l);
      }
    }

  if(regular && (type == steSTR || type == steNULL))
    {
    steDisplayNode *temp = db_new(steDisplayNode, ());
    temp->set_node_type((steNodeType)nl->get_node_type());
    temp->put_first(nl);
    nl->set_node_type(stePAR);
    nl = temp;
    }

//  if(tr->type != SMT_group)
    nl->set_appTree(tr);
  return nl;
  }


static void decorate1(steDisplayNode *);
void lde_smt_decorate(steDisplayNode * t)
  {
  Initialize(lde_smt_decorate);

  steNodeType tp;
  steDisplayNode * t1;
  switch(t->get_node_type())
    {
    case steSTR:
      t1 = checked_cast(steDisplayNode,t->get_first());
      if ( !t1 )
         break;
      tp = (steNodeType)t1->get_node_type();
      if(tp == steHEAD || tp == steSTR)
	{
	decorate1(checked_cast(steDisplayNode,t1->get_next()));
        lde_smt_decorate(t1);
        }
      else if(tp == stePAR && t1->get_next() != 0)
	decorate1(t1);
      break;
    case stePAR:
    case steHEAD:
    case steREF:
    case steHOOK:
    case steGLUE:
    case steFOOT:
    case steSKIP:
    case steSPLICE:
    break;
    }
  }

static void decorate1(steDisplayNode * t)
  {
  Initialize(decorate1);

  steNodeType tp;
  steDisplayNode * t1;
  for(; t; t = checked_cast(steDisplayNode,t->get_next()))
    {
    tp = (steNodeType)t->get_node_type();
    switch(tp)
      {
      case steSTR:
        break;
     
      case steHEAD:
      case steREG:
      case stePAR:
        t->set_node_type(stePAR);
        t1 = new steDisplayNode;
        t1->set_node_type(steSTR);
        t->put_after(t1);
        t1->put_first(t);
        t = t1;
        break;


      }
    lde_smt_decorate(t);
    }
  }

/*
   START-LOG-------------------------------------------

   $Log: lde_smt_text.cxx  $
   Revision 1.2 2000/07/07 08:11:14EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.6  1993/03/26  04:33:35  boris
Added Smod Short

Revision 1.2.1.5  1993/01/07  23:54:33  boris
Fixed empty one-file-node view

Revision 1.2.1.4  1992/12/17  14:46:50  jon
Moved some reference code into new file lde_reference.
Updated reference code.

Revision 1.2.1.3  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:19:08  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
