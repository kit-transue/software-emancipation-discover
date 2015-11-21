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
// File smt_tcons.C - SMODE 
//
// History:   01/13/92   M.Furman       Start of initial coding

#include <msg.h>
#include "smt.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <customize.h>

#include <machdep.h>

static void ctset(int, ...);
static void ctset_s(int, int);
static void closure();
static unsigned char * smt_ctable;
static int Mapping_DebugLevel;

#define STMT SMT_block,SMT_stmt,SMT_empty,SMT_if,SMT_nstdloop,SMT_switch,\
        SMT_label,SMT_break,SMT_continue,SMT_macrostmt,SMT_goto

#define COMPSTMT SMT_nstdloop, SMT_then_clause, SMT_else_clause,\
                 SMT_else_if_clause

#define IFCLAUSE SMT_then_clause, SMT_else_clause, SMT_else_if_clause

#define DECL SMT_decl,SMT_pdecl,SMT_cdecl,SMT_edecl,SMT_fdecl

#define MACRO SMT_m_include,SMT_m_define,SMT_m_if,SMT_m_else,\
	      SMT_m_endif,SMT_m_undef,SMT_m_pragma,SMT_m_gen

#define S_DECL SMT_fdecl, SMT_cdecl, SMT_edecl

#define BLOCKS SMT_block,SMT_cbody,SMT_ebody

static void init_ctable()
  {
  smt_ctable = new unsigned char[128 * 128];
  OS_dependent::bzero(smt_ctable, 128 * 128);
  ctset_s(SMT_expr, SMT_expr);
  ctset_s(SMT_if, SMT_then_clause);
  ctset_s(SMT_title, SMT_macrocall);
  ctset_s(SMT_stmt, SMT_expr);

  ctset(0, SMT_file, 0,  -1, 0);// file may include everything non strictly
  ctset(0, -1, 0, SMT_comment, MACRO, 0);

  ctset(0, SMT_block, SMT_case_clause, 0, SMT_list_decl, STMT, 0);

  ctset(0, -1, 0, SMT_expr, 0);
  ctset(0, -1, 0, SMT_macrostmt, 0);
  ctset(0, -1, 0, SMT_macrocall, 0);
  ctset(0, SMT_title, 0, SMT_macrostmt, 0);
  ctset(0, SMT_stmt, 0, SMT_macrostmt, 0);

  // Declarations
  ctset(1, SMT_cbody, 0, SMT_fdef, 0);
  ctset(0, SMT_fdef, 0, SMT_fdecl, 0);
  ctset(1, SMT_cdecl, 0, SMT_cbody, 0);
  ctset(1, SMT_edecl, 0, SMT_ebody, 0);
  ctset(1, SMT_fdef, 0, SMT_block, 0);
  ctset(1, SMT_fdecl, 0, SMT_list_decl, 0);
  ctset(0, SMT_fdecl, 0, SMT_declspec, 0);
  ctset(0, SMT_fdecl, 0, SMT_title, 0);
  ctset(0, SMT_cdecl, 0, SMT_title, 0);
  ctset(0, SMT_edecl, 0, SMT_title, 0);
  ctset(0, SMT_declspec, 0, SMT_edecl, SMT_cdecl, 0);
  ctset(1, SMT_title, 0, SMT_pdecl, 0);
  ctset(1, SMT_title, 0, SMT_superclass, 0);
  ctset(1, SMT_decl, 0, SMT_cdecl, SMT_edecl, 0);
  ctset(1, SMT_pdecl, 0, SMT_cdecl, SMT_edecl, 0);
  ctset(0, SMT_list_decl, 0, SMT_declspec, 0);
  ctset(1, SMT_list_decl, 0, DECL, 0);
  ctset(1, SMT_title, 0, SMT_list_decl, 0);
  ctset(1, BLOCKS, 0, SMT_list_decl, 0);
  ctset(1, BLOCKS, 0, SMT_stmt, 0);
  ctset(1, SMT_block, 0, SMT_goto, 0);
  ctset(1, SMT_ebody, 0, SMT_enum_field, 0);
  ctset(1, SMT_macrocall, 0, -1, 0);  
  
  // IF structure
  ctset(1, SMT_if, 0, IFCLAUSE, 0);
  ctset(0, SMT_if, 0, SMT_then_clause, 0);

  // compaund statements
  ctset(1, COMPSTMT, 0, STMT, 0);
  ctset(0, COMPSTMT, SMT_switch, 0, SMT_title, 0);
  ctset(1, SMT_switch, 0, SMT_case_clause, 0);

  closure();

  Mapping_DebugLevel = customize::getIntPref("DIS_internal.Mapping_DebugLevel");
  }

static void closure()
  {
  int flag, i, j, k, x, y, z, w;
  do
    {
    flag = 0;
    for(i = 0; i < 128; i++)
      for(j = 0; j < 128; j++)
        {
	x = smt_ctable[i + j * 128];
	if(x)
	  {
	  for(k = 0; k < 128; k++)
	    {
	    y = smt_ctable[j + k * 128] & x;
	    z = smt_ctable[i + k * 128];
	    w = z | y;
	    if(w != z)
	      {
	      smt_ctable[i + k * 128] = w;
	      flag++;
	      }
	    }
	  }
        }
    }
  while(flag);
  }

static void ctset1(int m, int f, int t1, int t2)
  {
  int i;
  if(t1 == -1)
    {
    for(i = 0; i < 128; i++)
      ctset1(m, f, i, t2);
    }
  else if(t2 == -1)
    {
    for(i = 0; i < 128; i++)
      ctset1(m, f, t1, i);
    }
  else
    {
    int i = t1 + t2 * 128;
    smt_ctable[i] &= ~m;
    smt_ctable[i] |= f & m;
    }
  }

static void ctset(int strictly, ...)
  {
  va_list args;
  int left[50], right[50];
  int nl, nr, a;
  nl = 0;
  va_start(args, strictly);
  for(; (a = va_arg(args, int)) != 0; )
    {
    left[nl++] = a;
    }
  nr = 0;
  while((a = va_arg(args, int)) != 0)
    {
    right[nr++] = a;
    }
  for(int i = 0; i < nl; i++)
    for(int j = 0; j < nr; j++)
      {
      if(strictly)
        ctset1(0x0C, 0x0C, left[i], right[j]);
      else
        ctset1(0x0E, 0x0E, left[i], right[j]);
      }
  }

// allow number of exactly the same structures
static void ctset_s(int t1, int t2)
  {
  ctset1(1, 1, t1, t2);
  ctset1(1, 1, t2, t1);
  }


int smt_test_consistency(int type1, int type2, int itype)
  {
  // itype: 1, 2 , 3 - type1 wider then type 2
  //        0 - equal exactly
  //        1, -1 - the same left tokens
  //        2, -2 - the same right tokens
  // Return: 0 - ok; 1 - kill type1
  int w, i;

  if(smt_ctable == 0)
    init_ctable();

  w = type1;

  if(itype < 0)
    {
    type1 = type2;
    type2 = w;
    itype = -itype;
    }

  if (itype)
    i = ! (smt_ctable[type1 + 128 * type2] & (001 <<  itype));
  else 
    i = ! (smt_ctable[type1 + 128 * type2] & 0x6);

  if ((Mapping_DebugLevel > 0) && i)
    {
    msg("killing: $1 --- $2$3$4\n") << smt_token_name(w) << eoarg << smt_token_name(type1) << eoarg << "=->>"[itype] << eoarg << smt_token_name(type2) << eom;
    }

  return i;
  }

// 1 - new type has higher priority
int smt_stmt_priority(int old_type, int new_type)
  {
  if(new_type == SMT_expr && old_type == SMT_stmt)
    return 0;
  if (new_type == SMT_stmt && old_type == SMT_expr)
    return 1;
  if ( (new_type == SMT_cdecl && old_type == SMT_declspec) ||
      (new_type == SMT_edecl && old_type == SMT_declspec) )
    return 0;

  if(new_type == SMT_clause || new_type == SMT_title ||
     new_type == SMT_else_clause || new_type == SMT_else_if_clause ||
     new_type == SMT_then_clause || new_type == SMT_case_clause)
    return 0;
  return 1;
  }




