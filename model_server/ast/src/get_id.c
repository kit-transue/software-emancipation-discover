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
#include <string.h>
#include "tree.h"
#include "ast_shared.h"

#define MAX_HASH_TABLE 1009
#define HASHBITS 30

static tree *hash_names;	/* id hash buckets */
static tree *hash_table;

/* to search given text in a given hash table; return 0, if not found */

static tree get_id_internal (text, table)
    register char *text;
    register tree *table;
{
  register int hi;
  register int i;
  register tree idp;
  register int len, hash_len;

  /* Compute length of text in len.  */
  for (len = 0; text[len]; len++);

  /* Decide how much of that length to hash on */
  hash_len = len;

#if 0
  if (warn_id_clash && len > id_clash_len)
    hash_len = id_clash_len;
#endif

  /* Compute hash code */
  hi = hash_len * 613 + (unsigned)text[0];
  for (i = 1; i < hash_len; i += 2)
    hi = ((hi * 613) + (unsigned)(text[i]));

  hi &= (1 << HASHBITS) - 1;
  hi %= MAX_HASH_TABLE;
  
  /* Search table for identifier */
  for (idp = table[hi]; idp; idp = TREE_CHAIN (idp))
    if (IDENTIFIER_LENGTH (idp) == len
	&& IDENTIFIER_POINTER (idp)[0] == text[0]
	&& !strncmp (IDENTIFIER_POINTER (idp), text, len))
      return idp;		/* <-- return if found */

#if 0
  /* Not found, create one, add to chain */
  idp = make_node (IDENTIFIER_NODE);
  IDENTIFIER_LENGTH (idp) = len;
#ifdef GATHER_STATISTICS
  id_string_size += len;
#endif

  IDENTIFIER_POINTER (idp) = text; /*obstack_copy0 (&permanent_obstack, text, len);*/


  TREE_CHAIN (idp) = table[hi];
  table[hi] = idp;
  return idp;			/* <-- return if created */
#else
  return 0;
#endif
}

/* to search first table */
tree get_id (text)
    char* text;
{
    return get_id_internal (text, hash_names);
}

/* same */
tree check_id (text)
    char* text;
{
    return get_id_internal (text, hash_names);
}

/* to assign first table */
void assign_get_id (ptr)
    tree* ptr;
{
    hash_names = ptr;
}

/* to assign second table */
void assign_check_identifier (ptr)
    tree* ptr;
{
    hash_table = ptr;
}

/* to search second table */
tree check_identifier(text)
    char* text;
{
    return get_id_internal (text, hash_table);
}

#define MAX_HASH_FOR_HOOKS 101

struct linenum_list {
  struct linenum_list *next;	/* Next structure in the bucket.  */
  tree ast;                     /* ast node - hash & search key */
  int linenum;
  int start_line;
  int end_line;
  int debug_line;
};
#define HOOKLINE_NEXT(NODE) ((NODE)->next)
#define HOOKLINE_AST(NODE)  ((NODE)->ast)
#define HOOKLINE_LINE(NODE) ((NODE)->linenum)

static int flag_linenum_hash_table;
void reset_flag_linenum_hash_table () 
{
    hash_table = 0;
    flag_linenum_hash_table = 0; 
}
struct linenum_list **linenum_hash_table;

static void restore_ast_linenum_hash_for_hooks ()
{
  tree id_for_hooks;

  if (hash_table == 0 && ast_tree_ptr)
    assign_check_identifier (ast_tree_ptr->ast_shared_tree.hash_table);

  if (hash_table)
    id_for_hooks = check_identifier ("______linenum_hash_table_for_hooks______");
  if (id_for_hooks == NULL_TREE)
    linenum_hash_table = (struct linenum_list**) 0;
  else
    linenum_hash_table = (struct linenum_list**) TREE_TYPE (id_for_hooks);
}

static struct linenum_list *check_linenum_hash (ast)
     tree ast;
{
  struct linenum_list *llist;
  unsigned int hi = (unsigned int) ast;

  hi &= 0x0fffffff;
  hi %= MAX_HASH_FOR_HOOKS;

    
  if (flag_linenum_hash_table++ == 0)
    restore_ast_linenum_hash_for_hooks ();

  if (linenum_hash_table != (struct linenum_list**) 0)
    for (llist = linenum_hash_table [hi]; llist; llist = HOOKLINE_NEXT (llist))
      if (HOOKLINE_AST (llist) == ast)
	return llist;
  
  return (struct linenum_list *)0;
}

void ast_set_linenum (ast, line)
     tree ast;
     int line;     
{}

int ast_get_linenum (ast)
     tree ast;
{
  struct linenum_list *llist = check_linenum_hash (ast);

  if (llist)    
    return HOOKLINE_LINE (llist);
  else
    return DECL_SOURCE_LINE (ast);
}



