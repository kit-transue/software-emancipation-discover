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
// File assoc_db.C


#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <Relational.h>
#include <assoc.h>
#include <db.h>

void assocNode::db_save(app*header, appTree*, db_app&, db_buffer& dbb)
{
    Initialize(assocNode::db_save);
    
    if (is_assocType (header)) {
	assocType* h = checked_cast (assocType, header);
	dbb.put ((char*)h->unique_name);
	dbb.put ((char*)h->uname);
	dbb.put ((char*)h->cat_name);
	dbb.put ((char*)h->program_string);
	dbb.put_byte (h->impact_on);
	dbb.put_byte (h->propagate_flag);
  	dbb.put_byte (h->program_type);
    } else if (is_assocInstance (header)) {
	assocInstance* h = checked_cast (assocInstance, header);
	dbb.put ((char*)h->uniq_name);
	dbb.put ((char*)h->type_name);
	dbb.put ((char*)h->desc);
    } else {
	Error (ERR_INPUT);
    }	
}

assocNode* assocNode::db_restore(app* hd, appTree* parent, db_app&, 
				 db_buffer& dbb)
{
    Initialize(assocNode::db_restore);

    if (!parent) { /* root */
	assocNode* root = checked_cast(assocNode, hd->get_root());
        if(root) {
            rem_relation(tree_of_header, hd, 0);
            obj_delete(root);
        }

	if (is_assocType (hd)) {
	    assocType* h = checked_cast (assocType, hd);
	    char* str;
	    dbb.get (str);
	    h->unique_name = str;
	    dbb.get (str);
	    h->cat_name = str;
	    dbb.get (str);
	    h->program_string = str;
	    dbb.get (str);
	    byte bt;
	    dbb.get_byte (bt);
	    h->impact_on = bt;
	    dbb.get_byte (bt);
	    h->propagate_flag = (AssocProgram)bt;
	    dbb.get_byte (bt);
	    h->program_type =  (AssocProgram)bt;
	} else if (is_assocInstance (hd)) {
	    assocInstance* h = checked_cast (assocInstance, hd);
	    char* str;
	    dbb.get (str);
	    h->uniq_name = str;
	    dbb.get (str);
	    h->type_name = str;
	    dbb.get (str);
	    h->desc = str;
	} else {
	    Error (ERR_INPUT);
	}
    }
    assocNode *node = db_new (assocNode, ());
    return node;
}

assocType* assocType::db_restore(char* title, byte, char* /* ph_name */)
{
    Initialize(assocType::db_restore);

    assocType* h = db_new(assocType, (title));
    return h;
}

assocInstance* assocInstance::db_restore(char* title, byte, char* /* ph_name */)
{
    Initialize(assocType::db__restore);

    assocInstance* h = db_new(assocInstance, (title));
    return h;
}

/*
   START-LOG-------------------------------------------

   $Log: assoc_db.cxx  $
   Revision 1.2 2000/07/10 23:00:41EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
// Revision 1.2  1993/03/08  14:28:15  aharlap
// added phys_name argument to hd_restore ()
//
// Revision 1.1  1993/01/16  19:05:39  aharlap
// Initial revision
//

   END-LOG---------------------------------------------

*/
