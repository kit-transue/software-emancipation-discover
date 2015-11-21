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
//
// API typedefs and function call declarations. 
//
// IMPORTANT: WHENEVER $mp/api/src/SPEC FILE IS CHANGED, THIS FILE HAS TO
// REFLECT THE CHANGES.
//

#include "ddSelector.h"
#include "genString.h"
#include "symbolArr.h"
#include "tcl.h"

typedef Tcl_Obj *DI_object;
typedef DI_object* DI_objectp;

typedef int * intp;
typedef char * charp;

#ifdef __cplusplus
extern "C" {
#endif
void DI_object_create(DI_objectp  s);
void DI_object_delete(DI_object s);
void DI_object_copy(DI_object source,DI_object target);
void DI_object_append(DI_object source,DI_object target);
void DI_object_name(DI_object object, genString& name);
void DI_object_lookup(DI_object object, int kind, charp  name, DI_object def_file);
void DI_object_kind(DI_object object,intp i);
void DI_object_size(DI_object object,intp i);
void DI_array_set(DI_object source, int position , DI_object target);
void DI_array_get(DI_object source, int position , DI_object target);
void DI_array_union(DI_object source, DI_object target);
void DI_array_subtract(DI_object source, DI_object target);
void DI_array_intersect(DI_object source, DI_object target);
void DI_where_defined(DI_object source, DI_object target);
void DI_where_referenced(DI_object source, DI_object target);
void DI_files_where_included(DI_object source, DI_object target);
void DI_where_used(DI_object source, DI_object target);
void DI_uses_functions(DI_object source, DI_object target);
void DI_uses_variables(DI_object source, DI_object target);
void DI_uses_classes(DI_object source, DI_object target);
void DI_uses_structures(DI_object source, DI_object target);
void DI_uses_unions(DI_object source, DI_object target);
void DI_uses_enums(DI_object source, DI_object target);
void DI_uses_typedefs(DI_object source, DI_object target);
void DI_uses_macros(DI_object source, DI_object target);
void DI_show_include_files(DI_object source, DI_object target);
void DI_show_member_functions(DI_object source, DI_object target);
void DI_show_member_data(DI_object source, DI_object target);
void DI_show_friends(DI_object source, DI_object target);
void DI_show_super_classes(DI_object source, DI_object target);
void DI_show_sub_classes(DI_object source, DI_object target);
void DI_show_defined_as(DI_object source, DI_object target);
void DI_show_decl_usage(DI_object source, DI_object target);
void DI_show_associations(DI_object source, DI_object target);
void DI_show_assocd_entities(DI_object source, DI_object target);
void DI_show_documents(DI_object source, DI_object target);
void DI_show_declared_in(DI_object source, DI_object target);
void DI_show_declares(DI_object source, DI_object target);
void DI_instance_of_template(DI_object source, DI_object target);
void DI_template_of_instance(DI_object source, DI_object target);
void DI_subsystems(DI_object source, DI_object target);
void DI_find_child_projects(DI_object source, DI_object target);
void DI_find_parent_project(DI_object source, DI_object target);
void DI_defines_functions(DI_object source, DI_object target);
void DI_defines_variables(DI_object source, DI_object target);
void DI_defines_classes(DI_object source, DI_object target);
void DI_defines_structures(DI_object source, DI_object target);
void DI_defines_unions(DI_object source, DI_object target);
void DI_defines_enums(DI_object source, DI_object target);
void DI_defines_typedefs(DI_object source, DI_object target);
void DI_defines_macros(DI_object source, DI_object target);
void DI_find_files(DI_object source, DI_object target);

void DI_uses_query(DI_object source, DI_object target, ddSelector& sel);
void DI_perform_query (DI_object source, DI_object target, ddSelector& sel);

void DI_roots (DI_object target);
void DI_find_contents(DI_object source, DI_object target);
void DI_deep_find_files(DI_object source, DI_object target, int depth);
int DI_create_group(DI_object source, const char * name);
int DI_create_flat_group(DI_object source, const char * name);
int DI_get_group(char * name, DI_object target);
int DI_save_group(char * name);


#ifdef __cplusplus
}
#endif

int       newClient();
void      removeClient(int cl);
void      setClient(int cl);
symbolArr *get_DI_symarr(int obj);
symbolArr *get_DI_symarr(DI_object obj);
#define    GET_DI_SET_INDEX(obj) (int)obj->internalRep.longValue

