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
#include "messages.h"
#include <machdep.h>
#include "smt.h"
#include "ddict.h"
#include <objArr.h>
ddRoot* dd_sh_get_dr(smtHeader*);

// This array  contains an indormation about macro calls which were processed
// during last SMT->AST translation. Each macro call in this array represented
// by group of values; length of this sequence depends on the number of 
// arguments.
//
// Each group has the following format:
//
//    0. (int)         - number of arguments;
//    1. (smtHeader *) - pointer to SMT header for definition (may be 0);
//    2. (smtTree *)   - pointer to SMT node for definition (may be 0);
//    3. (smtHeader *) - pointer to SMT header for call (may not be 0);
//    4. (smtTree *)   - pointer to SMT node for call (may not be 0);
//    5. (smtHeader *) - pointer to SMT header for end of call (may not be 0);
//    6. (smtTree *)   - pointer to SMT node for end of call (may not be 0);
//    7. for each argument:
//	   7.1. (smtHeader *) - pointer to SMT header for argument (may be 0);
//	   7.2. (smtTree *)   - pointer to SMT node for argument (may be 0);
//

smtTree* smt_mark_stree(astTree* root, int mode, int type,
        int  start_line,  char*  start_file, int  end_line, char*  end_file);
smtTree* smt_mark_stree(astTree* root, int mode, int type,
        int  start_line,  char*  start_file, int  end_line,
			char*  end_file, smtHeader* sh);
extern "C" void cpp_macros_prt();


int smt_is_global_scope(smtTree* smt)
{
  Initialize(smt_is_global_scope);

  if (smt == 0) return 0;
  smtTree* par = checked_cast(smtTree, smt->get_parent()); // 
  if (par == 0 || (par->get_node_type() == SMT_file)) return 1;
  par = checked_cast(smtTree, par->get_parent()); // 
  if (par == 0 || (par->get_node_type() == SMT_file)) return 1;
  return 0;
}

smtTree* smt_get_proper_scope(smtTree * ref_smt)
{
  Initialize(smt_get_proper_scope);
  if (ref_smt == 0) return 0;
  smtTree* smt = ref_smt;

  while (ref_smt && (ref_smt->get_node_type() != SMT_fdef) &&
	            (ref_smt->get_node_type() != SMT_cdecl)) {
    ref_smt = checked_cast(smtTree, ref_smt->get_parent());
  }
  if (ref_smt) 
    return ref_smt;
  ref_smt = smt;

  while (ref_smt && (ref_smt->get_node_type() != SMT_edecl) &&
		    (ref_smt->get_node_type() != SMT_decl) &&
		    ( ref_smt->get_node_type() != SMT_fdecl)) {
    ref_smt = checked_cast(smtTree, ref_smt->get_parent());
  }
  if (ref_smt == 0) {
    ref_smt = smt;
    while (ref_smt && (ref_smt->get_node_type() != SMT_declspec)) {
      ref_smt = checked_cast(smtTree, ref_smt->get_parent());
    }
    smtTree* next_smt = (ref_smt)?checked_cast(smtTree, ref_smt->get_next()):0;
    if (next_smt && ((next_smt->get_node_type() == SMT_fdecl) ||
		     (next_smt->get_node_type() == SMT_decl)))
      return next_smt;
    else return 0;
  }
  if (smt_is_global_scope(ref_smt)) return ref_smt;
  else return smt_get_proper_scope(ref_smt->get_parent());
}

