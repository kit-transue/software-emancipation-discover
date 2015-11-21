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
#ifndef _iffYacc
#define _iffYacc
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cstdio>
#include <cstdarg>
#endif /* ISO_CPP_HEADERS */

struct iffPosition {
  int  line;         /* starts with 1 */
  int  column;      /* length if line==0 */
};

struct iffRegion {
  struct iffPosition st;
  struct iffPosition en;
  short file_ind;
  short is_macro;
};

struct iffRefId {
  unsigned int implicit : 8;
  unsigned int id : 24;
  struct iffRegion* region;
};

struct iffKeyword {
  char* code;
  struct iffRegion* region;
};

struct iffSymAtr {
  unsigned int cg : 1;
  unsigned int def: 1;
  unsigned int dummy0 : 6;
  unsigned int au : 1;
  unsigned int ctor : 1;
  unsigned int dtor : 1;
  unsigned int inln : 1;
  unsigned int linkc  : 1;
  unsigned int ellipsis : 1;
  unsigned int virt : 1;
  unsigned int dummy : 17;
};

struct iffRel {
  unsigned int ppp : 2;
  unsigned int virt  : 1;
  unsigned int constant : 1;
  unsigned int pointer  : 1;
  unsigned int reference :1;
  unsigned int dummy   : 2;
  unsigned int  src : 24;
  int   propagate:8; /* -1 = back */
  unsigned int  trg : 24;
  const char* code;
  char* atr; 
};

struct iffAstNode {
  unsigned int cg : 1;
  unsigned int def: 1;
  unsigned int dummy: 6;

  unsigned int ind : 24;
  
  char* code;

  struct iffAstNode* first;
  struct iffAstNode* next;
  struct iffRegion * region;
};

#ifdef __cplusplus
extern "C" {
#endif

extern int ifp_skip_flag;
extern int ifp_sym_offset;
extern int ifp_line;
extern int ast_level;
void ifp_skip_block (int);
void ifp_skip_rest_of_line ();

void ifp_init_input (FILE *);
void ifp_reset_input ();
extern int ifp_use_mmap;
void ifp_set_output (char*);
struct iffRegion* reg_make(struct iffPosition*pos);
struct iffRegion* reg_end(struct iffRegion*reg, struct iffPosition*pos);

struct iffSymAtr* sym_make(int ind, char* code);
void sym_add_name(struct iffSymAtr* sym, char*name);
void sym_add_reg(struct iffSymAtr* sym, struct iffRegion*reg);
struct iffSymAtr* sym_get_atr(int ind);
void sym_add_atr(struct iffSymAtr* sym, char*atr);
struct iffRel* sym_add_rel(int id, const char *nm, int trg);
char* ifp_rel_get_code(char*);
void rel_add_atr(struct iffRel*rel, char*atr);

void err_make(int, char*);

struct iffAstNode* ast_make_node(char* code);
struct iffAstNode* ast_make_string(char* str);
struct iffAstNode* ast_add_extern_region(struct iffAstNode*, char*, struct iffRegion*);
void ast_add_roots(struct iffAstNode* root);

void ref_make(struct iffRefId*ref);

struct iffKeyword* key_make(char*);
void key_add_region(struct iffKeyword*, struct iffRegion*);

void file_make(char*fn);
void language(char*fn);

void map_copy_pppp(struct iffPosition*p1, struct iffPosition*p2, struct iffPosition*p3, struct iffPosition*p4);
void map_lines(int st, int en, int new_st);
void map_fixed_pppp(struct iffPosition*p1, struct iffPosition*p2, struct iffPosition*p3, struct iffPosition*p4);
void map_copy_pcp(struct iffPosition*p1, int len, struct iffPosition*p3);

void *start_estring(void);
void  append_estring(void *, char);
    void  vsprintf_estring(void *obj, const char* fmt, va_list ap1, va_list ap2);
char *finish_estring(void *);
void  free_estring(void *);

void parser_error(int line_num, char *formatted, char *context, char *column_locator);

#ifdef __cplusplus
}
#endif
#define YYDEBUG 0
/* #define YYUSEMMAP 0 */

#endif


