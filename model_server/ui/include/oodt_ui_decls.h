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
/* oodt_ui_decls.h
 *------------------------------------------
 * Synopsis:
 * Declarations used by OODT user interface functions
 *
 * description:
 * Declares constants and the like for OODT ui functions.
 *------------------------------------------
 * Restrictions:
 * Must be includable in C programs
 *------------------------------------------
 */

#ifndef __oodt_ui_decls_h_
#define __oodt_ui_decls_h_

typedef enum {
   INHERITANCE_SORTER,
   KIND_SORTER,
   CATEGORY_SORTER,
   ACCESS_SORTER,
   ALPHA_SORTER,
   DECL_ORDER_SORTER,
   SUBSYS_KIND_SORTER
} oodt_sorter_types;

typedef enum {
   CLASS_SELECTED = 1,
   MEMBER_FUNCTION_SELECTED = 2,
   DATA_MEMBER_SELECTED = 4,
   CURRENT_CLASS_SELECTED = 8,
   ARGUMENT_SELECTED = 16
} oodt_object_selections;

#define DEFAULT_MEMBER_NAME "member"
#define DEFAULT_MEMBER_NAME_LEN  6

typedef enum {
   MEMBERS_EXPLODED = 1,
   SUPERCLASSES_EXPLODED = 2,
   SUBCLASSES_EXPLODED = 4,
   RESPONSIBILITIES_EXPLODED = 8,
   INVARIANTS_EXPLODED = 16,
   CONTAINING_EXPLODED = 32,
   RELATIONS_EXPLODED = 64,
   NOTES_EXPLODED = 128
} oodt_explode_selections;

struct define_class_args {
   char* p_alias;
   int super_sub_flag;  /* -1 => creating subclass, 1 => creating superclass,
                           0 => creating standalone unrelated class */
};

typedef enum {
   rel_one_to_one,
   rel_one_to_many,
   rel_many_to_one,
   rel_many_to_many
} rel_cardinality;

typedef enum {
   add_subsys,
   add_class,
   add_function,
   add_object
} subsys_add_type;

struct oodt_relation_ui_info {
   char* name;
   char* inv_name;
   rel_cardinality card;
   int required;
   char* smbr_decl;
   char* shdr_decl;
   char* sdef;
   char* tmbr_decl;
   char* thdr_decl;
   char* tdef;
};

typedef enum {
   contains_rel	=		0x0001,
   points_to_rel =		0x0002,
   refers_to_rel =		0x0004,
   method_type_rel = 		0x0008,
   method_argument_rel = 	0x0010,
   has_friend_rel =		0x0020,
   contained_in_rel =		0x0040,
   pointed_to_by_rel = 		0x0080,
   referred_to_by_rel =		0x0100,
   returned_by_function_rel =	0x0200,
   is_argument_rel =		0x0400,
   is_friend_rel =		0x0800,
   nested_in_rel =              0x1000,
   container_of_rel =           0x2000
} builtin_rel_types;

/* The following enum reflects the member_attributes enum in ldrNode.h: */

typedef enum {
   show_inherited =		0x0001,
   show_package_prot_members =  0x0002,
   show_methods =		0x0004,
   show_data =			0x0008,
   show_nested_types =		0x0010,
   show_class_members =		0x0020,
   show_instance_members =	0x0040,
   show_virtual_members =	0x0180,
   show_public_members =	0x0200,
   show_protected_members =	0x0400,
   show_private_members =	0x0800,
   show_arguments =		0x1000
} member_display_types;

#endif

/*
    START-LOG-------------------------------

    $Log: oodt_ui_decls.h  $
    Revision 1.3 1998/09/07 10:31:44EDT Scott Turner (sturner) 
    adding support for Java "package"-level accessibility
 * Revision 1.2.1.3  1993/05/28  22:22:34  wmm
 * Fix bug 3241.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:07  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

