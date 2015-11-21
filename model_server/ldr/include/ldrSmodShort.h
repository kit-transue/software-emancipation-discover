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
////////////////////////  FILE ldrSmodShort.h  ////////////////////////
//
// -- Contains  related ldrSmodShort Class Definition                  
//
// Relations : (standard for ldrHeader)
//
// Constructors :
//             ldrSmodShort(smtHeaderPtr, smtTreePtr root = NULL);
//
// History:   12/10/91     B. Furman      Initial coding
//----------------------------------------------------------

#ifndef _ldrSmodShort_h_
#define _ldrSmodShort_h_

#include "ldrSelection.h"
#include "ldrSmtSte.h"
#include "objOper.h"
#include "smt.h"

RelClass(smtTree);
RelClass(smtHeader);
RelClass(ldrSmodNode);

class ldrSmodShort : public ldrSmtSte {
  public:
    ldrSmodShort(smtHeaderPtr, smtTreePtr root = NULL);
    virtual void insert_obj(objInserter *, objInserter *);
    define_relational(ldrSmodShort,ldrSmtSte);
#ifdef __GNUG__
    ldrSmodShort(const ldrSmodShort& oo) { *this = oo; }
#endif
    copy_member(ldrSmodShort);
    ldrTree * generate_tree(appTree *, int = 0);
    int set_visible( smtTreePtr, int unparse_flag = 0 );
    void put_visible( smtTreePtr, int );
    int visible_split(void *, steDisplayNodePtr& src, steDisplayNodePtr& trg);
    steDisplayNodePtr tree_search(steDisplayNodePtr, int st, int en);
    steDisplayNodePtr region_search( smtTreePtr nd );

    objTreePtr find_ldr_to_rebuild(objTree *);

   virtual void build_selection(const ldrSelection&, OperPoint&);
   virtual void find_selection(const OperPoint&, ldrSelection&);
   virtual steDisplayNodePtr do_extract( smtTreePtr, steNodeType );
   ldrSmodNodePtr find_region (smtTreePtr, steDisplayNodePtr rt = NULL);
};

generate_descriptor(ldrSmodShort,steSmtSte);

#endif

