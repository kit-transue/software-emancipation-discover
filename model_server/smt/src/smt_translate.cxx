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
// File smt_translate.C
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cassert>
#endif /* ISO_CPP_HEADERS */

#include "machdep.h"
#include "smt.h"

#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <cmd.h>
#include "astTree.h"
#include <MemSeg.h>
#include <setprocess.h>
#include <objCollection.h>
#include <transaction.h>
int smt_to_ast(smtHeader *, smtTree *, int); 
static smtTree * find_arn_node(smtTree *);
static smtHeader* last_error_header;
int smt_editing_level = 0;
boolean smt_is_statement (smtTree*);
extern "C" int is_ast_good_for_incremental(astTree*);


void start_smt_editing()
{
    if(smt_editing_level == 0)
      	obj_transaction_start();
    smt_editing_level++;
}

void commit_smt_editing()
{
    smt_editing_level--;
    if(smt_editing_level == 0) {
      smtHeader::modify_view();
	obj_transaction_end();
    }
}

void try_modify_ast()
{}

astTree* smt_get_ast (smtTree*);
astTree* smt_get_ast (smtTree*);

static void clear_vrn_down (smtTree* root)
{
    Initialize(clear_vrn_down);
    smtTree* x;

    if (root->vrn)
	for(x = root->get_first(); x; x = x->get_next())
	    x->clear_vrn();
    else
	for(x = root->get_first(); x; x = x->get_next())
	    clear_vrn_down (x);
}

// Returns amount of temp(s)
int clear_temp (smtTree* t)
{
  int res = 0;
  if (t->type == SMT_temp) {
    t->type = t->was_type;
    t->vrn = res = 1;
  }
  
  int cur_res;
  for (smtTree* x = t->get_first (); x; x = x->get_next ()) {
    cur_res = clear_temp (x);
    res = res + cur_res;
  }
  
  return res;
}

extern "C"  char * smt_truncate_filename(char * name);

int smt_modify_ast(smtHeader * h, int flag)
{
/* only called from smtHeader::make_ast if language != FILE_LANGUAGE_ELS. */
/* ALL languages should be ELS at this point, but I don't know about some of
 these old define-relation things.  So this hasn't yet been removed. 
 000130 Kit Transue */
	assert(0);
	return 0;
}

static smtTree * find_arn_node(smtTree * root)
  {
   Initialize(find_arn_node);

  if(root->type != SMT_group && root->arn)
    return(root);
  for(smtTree * n = root->get_first(); n; n = n->get_next())
    {
    smtTree * x;
    if((x = find_arn_node(n)) != 0)
      return x;
    }
  return 0;
  }









