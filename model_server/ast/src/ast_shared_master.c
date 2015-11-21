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
#include <cLibraryFunctions.h>

#include <stdio.h>
#include <errno.h>
#include "tree.h"
#ifndef _WIN32
#include <sys/time.h>
#include <sys/resource.h>
#else
#include <process.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#endif
#include <signal.h>
#include <c_machdep.h>
#include "obstack.h"
#include "ast_shared.h"

extern char* psetmalloc();
#define ERROR_COUNT 4096
int errorcount = ERROR_COUNT;
tree signed_char_type_node;
tree unsigned_char_type_node;
tree error_mark_node;
tree double_type_node;
tree void_type_node;
tree float_type_node;
tree short_unsigned_type_node;
tree char_type_node;
tree integer_type_node;
tree unsigned_type_node;
tree long_integer_type_node;
tree short_integer_type_node;
tree long_double_type_node;
tree long_unsigned_type_node;
tree long_long_unsigned_type_node;
tree void_list_node;
static tree last_node = 0;

typedef void (*SIGNALTYPE) ();

static int flag_incremental_parsing;

void set_flag_incremental_parsing ()
{
   flag_incremental_parsing = 1;
}

char *
xmalloc (size)
     unsigned size;
{
  register char *value = (char *) psetmalloc (size);
  if (value == 0)
    {
      OSapi_printf ("virtual memory exhausted");
      abort ();
    }
  return value;
}

char *
xrealloc (ptr, size)
     char *ptr;
     unsigned size;
{
  register char *value = (char *) realloc (ptr, size);
  if (value == 0)
    {
      OSapi_printf ("virtual memory exhausted");
      abort ();
    }
  return value;
}


tree
get_global ()
{
  return ast_tree_ptr 
      ? ast_tree_ptr->ast_shared_decl.global_binding_level_save.names : 0;
}

void save_ast_shared_cpp (ptr)
    void* ptr;
{
/*
    if (ast_tree_ptr)
	ast_tree_ptr->ast_shared_toplev.unit_list = ptr;
    else 
	OSapi_fprintf (stderr, "ast_tree_ptr = 0\n");
*/
}

void* assign_ast_shared_cpp ()
{
    if (ast_tree_ptr)
	return ast_tree_ptr->ast_shared_toplev.unit_list;
    else 
    {
	OSapi_fprintf (stderr, "ast_tree_ptr = 0\n");
	return 0;
    }
}

tree* get_hashtable(p)
    void* p;
{
    tree* retval = 0;
    if(p) {
	struct ast_shared* ptr = (struct ast_shared*)p;
	retval = ptr->ast_shared_tree.hash_table;
    }
    return retval;
}
