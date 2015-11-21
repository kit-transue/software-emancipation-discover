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
/*
 * S Q L _ A S T . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This file contains the routines to create and linkup the nodes
 * of the Abstract Syntax Tree (AST) built during parsing.
 */

#include "sql_all.h"		/* EXTERN_C */
#include "sql_symbol.h"	/* symbol table struct (SQL_SYM) */
#include "sql_ast.h"		/* ast structs */

//
// Blank ast_info structure.
//
// Used to initialize the info part of newly created ast_tree nodes.
//
// Elements:
// rk, relation_attr_mask, size, scale, arg
// defined, reference, keyword, relation, decldef
// (see sql_ast.h for more info)
//
// This is not a class because it needs to be used in C lex and yacc
// routines.
//
static ast_info ast_blank_info = {
   rk_none,		/* rk */
   RELMASK_NONE,	/* relation_attr_mask */
   0,			/* size */
   0,			/* scale */
   0,			/* arg */
   0,			/* defined */
   0,			/* reference */
   0,			/* keyword */
   0,			/* relation */
   0,			/* decldef */
   0			/* replace */
};


//
// ast_new_tree - Create a new ast tree node and initialize it
//
EXTERN_C ast_tree * ast_new_tree (
   ast_node_kind ank,
   SQL_SYM *sym,
   ast_loc start,
   ast_loc end,
   const char * text)
{
   // TBD: add routine to create tree nodes and track them for delete
   ast_tree * at_new = new ast_tree;

   if (at_new) {
      at_new->ank = ank;
      at_new->sym = sym;
      at_new->start = start;
      at_new->end = end;
      at_new->text = text;
      
      at_new->info = ast_blank_info;
      
      at_new->head = 0;
      at_new->tail = 0;
   }

   return (at_new);
}

//
// ast_child_tree - append a child tree to parent node
//
EXTERN_C void ast_child_tree (ast_tree * at, ast_tree * at_child)
{
   //
   // Important check, this routine will often be called
   // with an empty child tree (zero pointer) as in the
   // case of optional grammar rules.  Checking here
   // instead of calling is_empty() for every bison rule
   // allows for cleaner bison action code.
   //
   if (!at || !at_child) {
      return;
   }

   // TBD: put this allocation in a routine
   ast_treelist * atl = new ast_treelist;
   
   if (atl) {
      atl->ast = at_child;
      atl->next = 0;
      
      // append to parent list of children
      if ( ! (at->head)) {
	 // empty list
	 at->head = atl;
	 at->tail = atl;
	 atl->prev = 0;
      } else {
	 atl->prev = at->tail;
	 at->tail->next = atl;
	 at->tail = atl;
      }

      // See if parent's location endpoints need to be expanded
      //
      // first check if child's starting location is before parent
      ast_extend_start (at, at_child->start);

      // next check if child's endpoint is after parent
      ast_extend_end (at, at_child->end);
   }
}

// Check if starting location is before parent tree
// if so change.
EXTERN_C void ast_extend_start (ast_tree * at, ast_loc start)
{
   if (start.lineno && at) {
      if ((start.lineno < at->start.lineno) ||
	  (at->start.lineno == 0)) {
	 at->start.lineno = start.lineno;
	 at->start.column = start.column;
      } else if ((start.lineno == at->start.lineno) &&
		 (start.column < at->start.column)) {
	 at->start.column = start.column;
      }
   }
}

// Check if endpoint is after parent tree
// if so change.
EXTERN_C void ast_extend_end (ast_tree * at, ast_loc end)
{
   if (end.lineno && at) {
      if (end.lineno > at->end.lineno) {
	 at->end.lineno = end.lineno;
	 at->end.column = end.column;
      } else if ((end.lineno == at->end.lineno) &&
		 (end.column > at->end.column)) {
	 at->end.column = end.column;
      }
   }
}

// Recurvely walk tree to find the node of a given kind
EXTERN_C ast_tree * ast_find_node_kind (ast_tree * atroot, ast_node_kind k)
{
   ast_tree * at = 0;
   
   if (atroot) {
      if (atroot->ank == k) {
	 at = atroot;
      } else {
	 for (ast_treelist* atl = atroot->head; atl && !at; atl = atl->next) {
	    at = ast_find_node_kind (atl->ast, k);
	 }
      }
   }
   return (at);
}
