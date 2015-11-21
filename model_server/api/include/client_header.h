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

 typedef char *charp; 
 typedef int DI_object; 
 typedef int * intp; 
 typedef DI_object * DI_objectp;
 int DI_connect(); 
 extern void (*api_error_handler)();void DI_object_create();
void DI_object_delete();
void DI_object_append();
void DI_object_copy();
void DI_object_name();
void DI_object_lookup();
void DI_object_size();
void DI_object_kind();
void DI_array_set();
void DI_array_get();
void DI_array_union();
void DI_array_subtract();
void DI_array_intersect();
void DI_where_defined();
void DI_where_referenced();
void DI_files_where_included();
void DI_where_used();
void DI_uses_functions();
void DI_uses_variables();
void DI_uses_classes();
void DI_uses_structures();
void DI_uses_unions();
void DI_uses_enums();
void DI_uses_typedefs();
void DI_uses_macros();
void DI_show_include_files();
void DI_show_member_functions();
void DI_show_member_data();
void DI_show_friends();
void DI_show_super_classes();
void DI_show_sub_classes();
void DI_show_defined_as();
void DI_show_decl_usage();
void DI_show_associations();
void DI_show_assocd_entities();
void DI_show_documents();
void DI_instance_of_template();
void DI_template_of_instance();
void DI_subsystems();
void DI_find_child_projects();
void DI_find_parent_project();
void DI_defines_functions();
void DI_defines_variables();
void DI_defines_classes();
void DI_defines_structures();
void DI_defines_unions();
void DI_defines_enums();
void DI_defines_typedefs();
void DI_defines_macros();
void DI_find_files();
void DI_open_view();
void DI_perform_query ();
void DI_roots ();
void DI_find_contents();
void DI_deep_find_files();
DI_object DI_binary_set_union ();
DI_object DI_binary_set_difference ();
DI_object DI_binary_set_intersection ();
