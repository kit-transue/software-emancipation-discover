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
#include "ldrNode.h"
#include "ldrApplTypes.h"
#include "ldrSmtHierarchy.h"
#include "smt.h"
#include <ddict.h>
#include "lde-decorate.h"
#include <cmd.h>
#include <cmd_enums.h>
ldrSmtHierarchyNodePtr lde_build_smt_hierarchy (smtTreePtr);

ldrSmtHierarchyNodePtr lde_extract_smt_hierarchy (smtTreePtr tr)
  {
  ldrSmtHierarchyNodePtr new_ldr;
  Initialize(lde_extract_smt_hierarchy);
  new_ldr = lde_build_smt_hierarchy (tr);
  if(new_ldr)
    lde_decorate(new_ldr);
  return (new_ldr);
  }

enum nmode {plain, all, head, hide};

static struct symtab {
  byte type;
  ldrSymbolType stype;
  enum nmode mode;
} stab[] = {
    {SMT_comment,       (ldrSymbolType)0,hide},
    {SMT_file,		ldrBLOCK,	all},
    {SMT_macrocall,	ldrSOMETHING,	plain},
    {SMT_for,		ldrLOOP,   	head},
    {SMT_while, 	ldrLOOP,   	head},
    {SMT_do, 		ldrLOOP,   	head},
    {SMT_nstdloop, 	ldrLOOP,   	head},
    {SMT_if, 		ldrMULTIPLE_IF, all},
    {SMT_switch,        ldrIF,          head},
    {SMT_else_clause, 	ldrIF,  	head},
    {SMT_else_if_clause,ldrIF,  	head},
    {SMT_then_clause,   ldrIF,  	head},
    {SMT_try_catch,     ldrMULTIPLE_IF, all},
    {SMT_try_clause,    ldrIF,          head},
    {SMT_catch_clause,  ldrIF,          head},
    {SMT_title,         ldrSOMETHING,   plain},

    {SMT_case_clause,   ldrSEM_TYPE,  	head},
    {SMT_clause, 	ldrSEM_TYPE,  	head},
    {SMT_block, 	ldrSEM_TYPE,  	all},
    {SMT_group, 	ldrSEM_TYPE,   	plain},
    {SMT_stmt,          ldrSEM_TYPE, 	plain},
    {SMT_macrostmt,     ldrSEM_TYPE, 	plain},
    {SMT_fdef, 	        ldrFUNCTION_TYPE,    head},
    {SMT_cdecl,         ldrCLASS_TYPE,       head},
    {SMT_cbody,         ldrSEM_TYPE,       all},
    {SMT_edecl,         ldrENUM_TYPE,       head},
    {SMT_ebody,         ldrSEM_TYPE,       all},
    {SMT_sdecl,         ldrSEM_TYPE,       head},
    {SMT_fdecl,         ldrSOMETHING,   plain},
    {SMT_list_decl,     ldrSOMETHING,   plain},
    {SMT_enum_field,    ldrSOMETHING,   plain},
    {SMT_goto,	        ldrUNKNOWN_TYPE,	plain},
    {SMT_label,	        ldrSEM_TYPE,	plain},

    {SMT_break,	        ldrSOMETHING,	plain},
    {SMT_expr,          ldrSOMETHING,   plain},
    {SMT_m_define,	ldrSOMETHING,	plain},
    {SMT_temp,	        ldrSOMETHING,   plain},
    {0, 		ldrSOMETHING, 	hide}
  };

inline int single_stmt(int type)
{
 return
  (type == SMT_then_clause)
 ||   (type == SMT_else_clause)
 ||   (type == SMT_else_if_clause)
 ||   (type == SMT_nstdloop)
 ||   (type == SMT_switch)
 ||   (type == SMT_fdef)
 ||   (type == SMT_edecl)
 ||   (type == SMT_cdecl);
}

static ldrSmtHierarchyNodePtr 
internal_hierarchy (smtTreePtr root, nmode mode, ldrSymbolType type);

static ldrSmtHierarchyNodePtr lde_smt_hierarchy_list_decl (smtTreePtr root)
{
  if(!root) return NULL;
  nmode mode = plain;
  ldrSymbolType type = ldrVARIABLE_TYPE;

  smtTree*spec = root->get_first();
  smtTree*decl = NULL;
  if(spec && spec->type==SMT_declspec){
    smtTree*first_child=spec->get_first();
    if(!first_child)
       return 0;
    if(first_child->type==SMT_cdecl){
      mode =  all;
      decl = first_child;
    } else {
      decl = first_child->get_next();
      if(decl && decl->type==SMT_cdecl){
         mode = head;
         if(first_child->type == SMT_token && first_child->extype == SMTT_kwd){
            smtHeader * h = (smtHeader*) root->get_header();
            char * str = h->srcbuf + first_child->tbeg;
	    if(strncmp("typedef", str, 7)==0)
	      type = ldrTYPEDEF_TYPE;
         }
      } else {
         decl = NULL;
      }
    }
    if(decl){
      smtTree*title = decl->get_first();
      if(!title) return NULL;
      smtTree*body = title->get_next();
      if(!(body && body->type==SMT_cbody))
	return NULL;
    }
  }
  if(!decl) return NULL;

  ldrSmtHierarchyNode* new_ldr = new ldrSmtHierarchyNode(type);

  if(mode == plain)
      new_ldr->string_mode = full; 
  else if(mode == all)
      new_ldr->string_mode = null; 
  else
      new_ldr->string_mode = first; 

  new_ldr->set_appTree(spec);

  ldrSmtHierarchyNodePtr ldecl = lde_build_smt_hierarchy(decl);
  if(!ldecl)
    return NULL;

  new_ldr->put_first(ldecl);
  decl = spec;
  while(decl=decl->get_next()){
    if(decl->type == SMT_decl || decl->type == SMT_cdecl){
      ldrSmtHierarchyNode* ndecl = internal_hierarchy(decl, plain, ldrSOMETHING);
      if(ndecl){
	ldecl->put_after(ndecl);
	ldecl = ndecl;
      }
    }     
  }
  return new_ldr;
}

ldrSmtHierarchyNodePtr lde_build_smt_hierarchy (smtTreePtr root)
{
  Initialize(lde_build_smt_hierarchy);
  if(!root) return NULL;
  int i;
  ldrSymbolType type;

  // Find table entry for type of given node
  for(i=0; stab[i].type && stab[i].type != root->type; i++);
  type = stab[i].stype;
  if(stab[i].mode == hide)
    return 0;

  if (root->type == SMT_m_define) {
    ddElement* ep = checked_cast(ddElement, get_relation(def_dd_of_smt, root));
    if (!ep)
      return 0;
  }

  // Do not display group in some cases:
  //  1. if it is a first 1-token group which has the value "{";
  //  2. if it is a last 1-token group which has the value "}".
  //  3. if it is a 2-tokens group which has the value "{}".
  smtHeader * header = checked_cast(smtHeader,root->get_header());
  if (root->type == SMT_token && 
      (root->extype == SMTT_begin || root->extype == SMTT_end))
      return 0;

  enum nmode mode = stab[i].mode;
  smtTree* subroot = root;

  if(cmd_available_p(cmdSymbolAttr)){
    if(root->type == SMT_if) {
      smtTree*then = root->get_first();
      if(then && then->type==SMT_then_clause && ! then->get_next()){
	mode = head;
	subroot = then;
        type = ldrIF;
      }
    } else if(root->type == SMT_list_decl) {
      ldrSmtHierarchyNode* new_ldr = lde_smt_hierarchy_list_decl (root);
      if(new_ldr)
	return new_ldr;
    }
  }
  return
    internal_hierarchy(subroot, mode, type);
}
static int block_is_empty(smtTree*block)
{
  if(!block)
    return 1;
  smtTree* start = block->get_first();
  if(!(start && start->type ==SMT_token && start->extype==SMTT_begin ))
    return 1;

  smtTree* next = start->get_next();
 
  if(!next)
    return 1;

  return
    ! next->get_next();
}
static ldrSmtHierarchyNode* 
internal_hierarchy(smtTree* root, nmode mode, ldrSymbolType type)
  {

  ldrSmtHierarchyNodePtr new_ldr, prev_ldr, ldr;
  int is_single = 0;
  if(cmd_available_p(cmdSymbolAttr))
    is_single = single_stmt(root->type);

  // create new LDR node of appropriate type
  new_ldr = new ldrSmtHierarchyNode(type);

  if(mode == plain)
      new_ldr->string_mode = full; 
  else if(mode == all)
      new_ldr->string_mode = null; 
  else
      new_ldr->string_mode = first; 

  prev_ldr = 0;
  if(mode != plain) {
    int first = 1;
    for(smtTree*s = root->get_first();  s;  s = s->get_next(), first = 0) {
      if(first && (mode == head))
        continue;
      else if(is_single && (s->type==SMT_block 
		|| s->type==SMT_cbody || s->type==SMT_ebody)){
        is_single = 0;
        if(!block_is_empty(s)){ 
          smtTree * after = s->get_next();
	  if(!after || (after->type == SMT_token  // struct AA{}; do{}while(1)
              && after->extype == SMTT_op && !after->get_next()))
	    s = s->get_first();
	}
      }
      ldr = lde_build_smt_hierarchy(s);
      if(ldr == 0)
         continue;
      if(prev_ldr)
        prev_ldr->put_after(ldr);
      else new_ldr->put_first(ldr);
      prev_ldr = ldr;

    } 
  }
  new_ldr->set_appTree(root);
  return new_ldr;
}

/*
   START-LOG-------------------------------------------

   $Log: lde_smt_hierarchy.cxx  $
   Revision 1.9 2000/07/07 08:11:09EDT sschmidt 
   Port to SUNpro 5 compiler

   END-LOG---------------------------------------------

*/





