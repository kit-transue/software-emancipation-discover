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
#ifndef _RTL_externs_h
#define _RTL_externs_h

#ifndef _RTL_h
#include <RTL.h>
#endif

#ifndef _RegExp_h
#include <RegExp.h>
#endif

#ifndef _BooleanTree_h
#include <BooleanTree.h>
#endif

#ifndef _symbolPtr_h
#include <symbolPtr.h>
#endif

class symbolTreeHeader;
class symbolScope;

symbolPtr rtl_find_obj(RTLNode*, char* name);
void rtl_delete(RTLNode*);
void rtl_merge(RTLNode* dest_rtl, RTLNode* rtl);
void rtl_intersect(RTLNode* dest_rtl, RTLNode* rtl);
void rtl_subtract(RTLNode* dest_rtl, RTLNode* rtl);
void rtl_add_obj(RTLNode*, symbolPtr);
void rtl_delete_obj(RTLNode*, Obj*);

int  rtl_compare_elems(symbolPtr, symbolPtr, char* criteria);
int  rtl_compare_indices(const void* one_index, const void* two_index);
int* rtl_sort_indices(RTLNode*);
int rtl_init_sort(RTLNode *);

extern "C"
{
    // All c++ functions called from C are declared here.

    // Declarations from file RTL.C

    int rtl_obj_includes (Obj *obj, RTL *rtl, int dep_index);
    int rtl_obj_match_name (Obj *obj, char *pattern);
    int rtl_evaluate_search_criteria (Obj *obj, RTL *rtl);
    void rtl_generate_function(
	RTL *rtl, BooleanNode *root, RTLBooleanOperators new_op);

    // Declarations from file REGEXP_apl.C

    int apl_REGEXP_match_regexp (Regexp *regexp, char *string);
    Regexp *apl_REGEXP_create (char *pattern);
    int apl_REGEXP_match_pattern (char *name, char *pattern);
    void apl_REGEXP_filter(
	RTL *rtl, FilterDomainType criteria, char *filter);
    void apl_REGEXP_add(
	RTL *node, FilterDomainType criteria, char *filter);
    void apl_REGEXP_clear_filter (RTL *rtl);
    int rtl_evaluate_filter (Obj *obj, RTLPtr rtl);
}

void xref_notifier_report (int, const symbolPtr &);
void xref_notifier_apply ();

void xref_notifier_add (RTL*);
void xref_notifier_add(symbolScope*);

void xref_notifier_rem (RTL*);
void xref_notifier_rem(symbolScope*);

symbolTreeHeader* find_symbolTreeHeader (const symbolPtr &);

#endif

