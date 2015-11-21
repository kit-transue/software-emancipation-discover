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
#include <stdio.h>
#include "galgen.yacc.h"

#ifdef __cplusplus
extern "C" {
#endif



#define PTR_TYPE_PREFIX "_"
#define PTR_TYPE_SUFFIX "ptr"
#define PTR_TYPE_PREFIX_LEN (sizeof(PTR_TYPE_PREFIX)-1)
#define PTR_TYPE_SUFFIX_LEN (sizeof(PTR_TYPE_SUFFIX)-1)

/* -------- YACC ------------------ */
void set_in( FILE* fin);
int  do_parse();

#define MAX_ARGS_TYPENAME 100
#define MAX_FUNC_TYPENAME 1000

typedef struct {
    int   type;
    char* typname;
    char* name;
    int   ptr;
    int   ref;
    int   isstatic;
    int   isconst;
    int   output;
    int   unsign;
} TYPENAME;

void inc_argcount();
void set_args_type( int type );
void set_args_name( char* name );
void set_args_typename( char* name );
void set_args_ptr();
void set_args_ref();
void set_args_isstatic();
void set_args_isconst();
void set_args_output();
void set_args_unsign();
TYPENAME* get_args();

void  set_service_name( char* name );
char* get_service_name();
void  set_exttype(char* p);

void init_args ();
void typename_print_converters (char* name);
void typename_fix (TYPENAME* thing);
void typename_printfn (int fn);
void typename_copy (TYPENAME* dest, TYPENAME* src);
void typename_print_type (FILE* file, TYPENAME* thing);
void typename_print_typeP (FILE* file, TYPENAME* thing);
void typename_print_argdecl (FILE* file, TYPENAME* thing);
void typename_print_typeptr (FILE* file, TYPENAME* thing);
void typename_print_typeptr_for_retval (FILE* file, TYPENAME* thing);
void typename_print_reference (FILE* file, TYPENAME* thing);
void typename_print_ptr (FILE* file, TYPENAME* thing);
void typename_print_ref (FILE* file, TYPENAME* thing);
void typename_print_name (FILE* file, TYPENAME* thing);
void typename_print_arg (FILE* file, TYPENAME* thing);
void gen_caller(char* body);
void gen_callee(char* body);
void gen_Tcl (char* body);

/* -------- LEX ------------------ */

void maketype (char* name, int fix);
int isfixable (char* name);
int istype (char* name);

void new_line();
void new_token( int len );
int get_line();
int get_curr_col();
int get_prev_col();

/* -------- IF ------------------ */
void  IF_init_all(int iin, int iif, char* argv[]);
void  IF_service_keyword();
void  IF_set_service_name(char* servname);
void  IF_destroy_all();
void  IF_eof();

void  IF_init_args(TYPENAME* args);
void  IF_set_type_pos(int iarg);
void  IF_set_name_pos(int iarg);

void  IF_print_func_def(int maxargs, char* preffix, char* arg1_type, char* arg1_name);



#ifdef __cplusplus
};
#endif
