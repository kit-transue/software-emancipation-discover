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
// File db_def.C
#include <cLibraryFunctions.h>

#include <Relational.h>
#include "smt.h"
//#include <assoc.h>
#include <dfa_db.h>
//#include "steDocument.h"
#include "db.h"
#include "db_intern.h"
#include "ddict.h"
//#include "steTextNode.h"
//#include "steRefTextNode.h"

#include <subsys.h>

#ifndef _groupHdr_h
#include <groupHdr.h>
#endif

#ifndef _groupTree_h
#include <groupTree.h>
#endif

#include "style_hooks.h"
//#include <oodt_relation.h>

#define te(h,r,s,n,i,j)                                  \
              {(class_descriptor *)& h::_descrpt,    \
               (class_descriptor *)& n::_descrpt,    \
               (db_save_node)& n::db_save,           \
	       (db_restore_node)& n::db_restore,     \
	       (db_restore_header)& h::db_restore,   \
	       (db_after_rest)& n::db_after_restore, \
	       (db_after_rest_head)& h::db_after_restore, \
               &r,s,i,j                                \
	      }

#define cr(r,s,n)                                    \
              {0,                                    \
               (class_descriptor *)& n::_descrpt,    \
               (db_save_node)& n::db_save,           \
	       (db_restore_node)& n::db_restore,     \
	       0,                                    \
	       (db_after_rest)& n::db_after_restore, \
	       0,                                    \
               &r,s,0,0                              \
	      }

// The definition of saved headers and trees
// header , relation from header to root , suffix, basic node

#define mh(h,s,n,i,j)  te(h,tree_of_header,s,n,i,j)
static db_def db_main_headers [] =
{
    mh(smtHeader, "", smtTree,1,1),
    //    mh(oodt_relationHeader, "", oodt_relation,0,0),
    //    mh(assocType, 0, assocNode,0,0),
    //    mh(assocInstance, 0, assocNode,0,0),
    // add to end
};

// roots shared by all saveable headers.
static db_def db_common_roots[] =
{
    cr(ddRoot_of_smtHeader, 0, ddNode),
    cr(astRoot_of_smtHeader, 0, astRoot),
    // add to end
// cr(notetree_of_app, 0, objUserNote),
};

#define d_tbl(t)     { t,  sizeof(t) / sizeof(db_def) }

db_tables db_def_tables [] = 
{
    d_tbl (db_main_headers),
    d_tbl (db_common_roots),
    // add to end
};
int db_def_tables_size = sizeof (db_def_tables) / sizeof (db_tables);

// The definition of saved relations
const char *db_rel_table[] = 
{
    "next_text_of_prev_text",
    "hypertext_of_paragraph",
    "ref_smt_of_dd",
    "smt_of_dd",
    "def_smt_of_dd",
    "defined_in",
    "pro_subclassof",
    "vpro_subclassof",
    "pri_subclassof",
    "vpri_subclassof",
    "pub_subclassof",
    "vpub_subclassof",
    "struct_decl_of_ref",
    "appHeader_of_dd",
    "has_friend",
    "subclassof",
    "assoc_instance_of_dd",
    "assoc_member_of_dd",
    "def_assocnode_of_dd",
    "assocnode_of_dd",
    "appnode_of_errnote",
    "appnode_of_usernote",
    "decl_of_id",
    "declaration_of_reference",
    "reference_of_root",
    "nodes_of_ddsty",
    "nodes_of_ddcat",
    "pri_subsys_of_ddElement",
    "pub_subsys_of_ddElement",
    "pri_containing_subsys",
    "pub_containing_subsys",
    "user_of_used",
    "semtype_of_smt_decl",
    "description_for_class",
    "responsibility_for_class",
    "responsibility_of_dd_class",
    "description_of_dd_class",
    "kr_pdecl_of_ref",
    "structure_of_cd",
    "function_of_undo",
    "extid_of_node",
    "ref_app_of_dd",
    "soft_assoc_of_app",
    "instance_of_assocType",
    "ref_file_of_dd",
    "ddLinkNode_of_srcAppTree",
    "ddLinkSpec_of_ddLinkNode",
    "trgAppTree_of_ddLinkNode",
    "ddRevLinkSpec_of_ddLinkSpec",
    "peer1_of_peer2",
    "server_grp_of_client_grp",
    "trans_mbr_of_grp",
    "pub_mbr_of_grp",
    "pri_mbr_of_grp",
    "decl_smt_of_dd",
    "template_of_instance",
    "scope_of_entity",
    // add to end and increment db_version in file db_read_src_from_pset.C
};

int db_rel_table_size = sizeof(db_rel_table) / sizeof(char*);

