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
#ifndef __proj_save_h__
#define __proj_save_h__

#include <genString.h>
#include <representation.h>

#define prjDELEMETER "_FLDX_"

enum PRJ_field {
    PRJ_phys,
    PRJ_model,
    PRJ_Field_Last
};

enum PRJ_flag {
    PRJ_fok,
    PRJ_lok,
    PRJ_dok,
    PRJ_xok,
    PRJ_Flag_Last
};

class projLine {
  public:
    int is_proj;
    int proj_size;
    fileLanguage lang;
    genString left_name;
    genString phys_name;
    genString right_name;
    genString model_name;
    char phys_flags [PRJ_Flag_Last + 1];
    char model_flags[PRJ_Flag_Last + 1];

    int included;
    int pmod_type;
    char sep;

    void reset();
    int is_flag_ok (PRJ_field fld, PRJ_flag flg);
};

extern char const *noprj_flag;

char const*str_find_char( char const *pszText, char c );
char      *str_find_char( char *pszText, char c );
genString  quote_str_if_chars (char const *pszIn, char *pszSpec );
genString  quote_str_if_space (char const *pszIn );
genString  dequote_str (char const* pszIn );
int        is_str_quoted(char const* s);
char       proj_save_get_separator ();

int proj_save (char const *, int);
int proj_save_all (int);

class projNode;
class projModule;

int proj_get_text_to_save (projNode *prj, int lvl, genString& txt);
int proj_get_text_to_save (projModule *mod, int lvl, genString& txt);

// Enumeration for fields for int info[3] in proj_save_recursive()
enum {
    LVL,
    MOD,
    PRJ
};

#define LVL_SPACES 2
#define MAX_LVL 20

#define PROJ_ADMINDIR_TAG "#ADMINDIR="
#define PROJ_HOST_TAG     "#HOST="

#endif
