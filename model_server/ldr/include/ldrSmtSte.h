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
////////////////////////  FILE ldrSmtSte.h  ////////////////////////
//
// -- Contains  related ldrSmtSte Class Definition                  
//
// Relations : (standard for ldrHeader)
//
// Constructors :
//             ldrSmtSte(smtHeaderPtr, smtTreePtr root = NULL);
//
// History:   12/10/91     A. Furman      Initial coding
//            12/20/91     M. Furman      Added declaration for insert_obj 
//----------------------------------------------------------

#ifndef _ldrSmtSte_h_
#define _ldrSmtSte_h_

#include "ldrSelection.h"
#include "objOper.h"
#include "smt.h"
#include "steDisplayHeader.h"

RelClass(smtTree);
RelClass(smtHeader);

class ldrSmtSte : public steDisplayHeader {
  public:
    ldrSmtSte() { }
    ldrSmtSte(smtHeaderPtr, smtTreePtr root = NULL);
    virtual void insert_obj(objInserter *, objInserter *);
    define_relational(ldrSmtSte,steDisplayHeader);
#ifdef __GNUG__
    ldrSmtSte(const ldrSmtSte& oo) { *this = oo; }
#endif
    copy_member(ldrSmtSte);
    ldrTree * generate_tree(appTree *, int = 0);

   virtual void build_selection(const ldrSelection&, OperPoint&);
   virtual void find_selection(const OperPoint&, ldrSelection&);
   virtual steDisplayNodePtr do_extract( smtTreePtr, steNodeType );
};

generate_descriptor(ldrSmtSte,steDisplayHeader);

#endif

