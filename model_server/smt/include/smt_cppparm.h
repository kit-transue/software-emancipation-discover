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
#ifndef _SMT_CPPPARM_H_
#define _SMT_CPPPARM_H_

#include "genArr.h"
#include "representation.h"

class genString;
class objArr_Int;
class projModule;
class genArrOf(genString);

extern "C" int smt_push_arg (void* array, char const *data);
int smt_make_cpp_parm(
    objArr_Int& parm, int& np, char const *lname, char const *filename,
    genString& lang);
void smt_clear_cpp_parm(char const ** parm, int maxpar);
void smt_set_new_language (projModule *mod, fileLanguage& language, genString& lang);
int smt_get_esql_params (projModule *mod, genString& txt);
int smt_get_esql_param_array (projModule *mod, int &verbatim, genString &txt, 
                              genArrOf(genString) &params);
int smt_get_cpp_params (projModule *mod, genString& txt);
int smt_get_cpp_param_array (projModule *mod, genArrOf(genString) &params);
int smt_make_all_parm (char const *fn, char const *language, genString &all_parm);
int smt_make_all_param_array (char const *fn, char const *language, genArrOf(genString) &all_parm);
int smt_get_comp_params (projModule *mod, genString& txt);
int smt_get_comp_param_array (projModule *mod, genArrOf(genString) &params);
projModule* api_find_module(char const *cur_dir, char const *str, genString& ln);
char const *els_include_path_ln(char const *path);

#endif /* _SMT_CPPPARM_H_ */
