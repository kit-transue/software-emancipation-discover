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
// File smt_db.C
#include <genError.h>
#include "smt.h"
#include "Relational.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <db.h>
#include <psetmem.h>

#include <machdep.h>

void smtTree::db_save(app*header, appTree*nd, db_app& dba, db_buffer& dbb)
{
    underInitialize(smtTree::db_save);
 
    smtTree * node = checked_cast(smtTree,nd);

    if (node->type == SMT_token) {
        dba.extype = node->extype - SMTT_FIRST_VALUE;

        smtHeader *h = checked_cast(smtHeader, header);
        char* buf = h->srcbuf;
        dbb.put(buf + node->start (), node->length ());
        dba.val = node->tlth;
        dba.type = node->spaces;
        dba.flag = node->newlines;
    } else {
        switch (node->type) {
          case SMT_decl:
            dba.flag = 2;
            break;
            
          case SMT_fdecl:
            dba.flag = 3;
            break;

          case SMT_pdecl:
            dba.flag = 4;
            break;

          case SMT_cdecl:
            dba.flag = 5;
            break;

          case SMT_edecl:
            dba.flag = 6;
            break;
            
          case SMT_sdecl:
            dba.flag = 7;
            break;
            
          default:
            break;
        }
        dba.extype = node->extype;
    }
}

static smtTree* smtTree_restore = 0;

static smtTree *db_restore_new_smtTree ()
{
    /* Assume that operators `new' for char and smtTree both based on
     * plain malloc
     */

    if (!smtTree_restore)
	smtTree_restore = db_new(smtTree,());

    char* ptr = new char[sizeof (smtTree)];
    OS_dependent::bcopy(smtTree_restore, ptr, sizeof (smtTree));
    return (smtTree*)ptr;
}
	
smtTree* smtTree::db_restore(app* header, appTree* parent,
                             db_app&dba, db_buffer&dbb)
{
    underInitialize(smtTree::db_restore);
    smtTree *node = db_restore_new_smtTree ();
    
    if (! parent) {  // root
        smtHeader * hd = checked_cast(smtHeader, header);
        hd->src_size = dbb.size();
        byte* ptr = dbb[0];
        IF(hd->srcbuf)
           psetfree(hd->srcbuf);
        hd->srcbuf = (char *)psetmalloc(hd->src_size);
        OS_dependent::bcopy(ptr, hd->srcbuf, hd->src_size);
    }
 
    if(dba.is_leaf){
        node->type = SMT_token;
        node->extype = dba.extype + SMTT_FIRST_VALUE;
        node->newlines = dba.flag;
        node->spaces = dba.type;
        dbb.skip(node->spaces);
        node->tbeg = dbb.get_position();
        node->tlth = dba.val;

	/* patch for additional RCS comments */
	if (node->extype == SMTT_comment) {
	    char* comment = (char*)dbb[node->tbeg];
	    if (comment[0] == '/' 
		&& comment[1] == '*'
		&& (comment[node->tlth - 2] != '*'
		    || comment[node->tlth - 1] != '/')) {
		char* end = strstr (comment + node->tlth - 1, "*/");
		Assert (end);
		node->tlth = (end - comment + 2);
	    }
	}
	/* end of patch */

        dbb.skip(node->tlth + node->newlines);
     } else {
         switch (dba.flag) {
           case 0:
             node->type = dba.extype;
	     node->was_type = node->type;
             break;

           case 1:
             break;

           case 2:
             node->type = SMT_decl;
             node->was_type = SMT_decl;
             break;
             
           case 3:
             node->type = SMT_fdecl;
             node->was_type = SMT_fdecl;
             break;

           case 4:
             node->type = SMT_pdecl;
             node->was_type = SMT_pdecl;
             break;

           case 5:
             node->type = SMT_cdecl;
             node->was_type = SMT_cdecl;
             break;

           case 6:
             node->type = SMT_edecl;
             node->was_type = SMT_edecl;
             break;

           case 7:
             node->type = SMT_sdecl;
             node->was_type = SMT_sdecl;
             break;
         }
         node->extype = dba.extype;
     }
     return node;
}

smtHeader* smtHeader::db_restore(char const* title, byte language, char const* ph_name)
{
    Initialize (smtHeader::db_restore);
    smtHeader* h = db_new (smtHeader, (title, ph_name, (smtLanguage)language));

    h->parsed = 1;
    h->set_arn(language != FILE_LANGUAGE_ELS);
    h->foreign = 0;

    smtTree * root = checked_cast(smtTree,h->get_root());
    if (root){
        rem_relation(tree_of_header, h);
        obj_delete(root);
    }
    return h;
}

void smtTree::db_after_restore(db_buffer&)
{
    Initialize(smtTree::db_after_restore);
    smtHeader *h = checked_cast(smtHeader, get_header());
    h->tidy_tree();
    h->just_restored = 1;
}

/*
   START-LOG-------------------------------------------

   $Log: smt_db.cxx  $
   Revision 1.6 2000/07/10 23:09:24EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
// Revision 1.18  1994/05/05  21:12:44  aharlap
// *** empty log message ***
//
// Revision 1.17  1994/01/13  03:04:03  kws
// Use psetmalloc
//
// Revision 1.16  1993/12/20  23:33:53  mg
// Bug track: 1
// underInitialize - mini version of Initialize
//
// Revision 1.15  1993/11/18  17:51:18  aharlap
// Bug track: N/A
// First stage of delayed smt mapping
//
// Revision 1.14  1993/10/21  01:33:54  builder
// Fix compilation error
//
// Revision 1.13  1993/07/20  18:38:18  harry
// HP/Solaris Port
//
// Revision 1.12  1993/04/29  22:23:06  aharlap
// set foreign flag to 0
//
// Revision 1.11  1993/04/22  13:26:51  aharlap
// added was_type field to smtTree
//
// Revision 1.10  1993/04/15  17:30:48  aharlap
// bug #3345
// fixed case of RCS massages in C-style comments
//
// Revision 1.9  1993/03/08  14:25:01  aharlap
// added phys_name argument to hd_restore ()
//
// Revision 1.8  1993/02/19  06:37:19  aharlap
// allow tab in smt_srcbuffer
//
// Revision 1.7  1993/02/12  00:45:34  aharlap
// changed tidy_tree () to produce normal srcbuf
//
// Revision 1.6  1993/01/28  18:32:39  aharlap
// `added incremental restoring
//
// Revision 1.5  1993/01/26  01:03:46  aharlap
// added some reserv
//
// Revision 1.4  1993/01/23  19:38:41  aharlap
// removed extra space at empty line
//
// Revision 1.3  1993/01/10  22:34:23  aharlap
// added categories
//
// Revision 1.2  1993/01/10  21:49:08  aharlap
// fixed bug
//
// Revision 1.1  1993/01/05  22:00:37  mg
// Initial revision
//


   END-LOG---------------------------------------------

*/
