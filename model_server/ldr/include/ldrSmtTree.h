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
#ifndef _ldrSmtTree_h_
#define _ldrSmtTree_h_

#include "genError.h"
#include "objRelation.h"
#include "smt.h"
#include "ldrHeader.h"
class symbolPtr;

// This is ldr header for Tree diagram of smt Tree.


class ldrSmtTree : public ldrHeader {
 public:
   ldrSmtTree(appTreePtr tr);
   ldrSmtTree(symbolPtr tr);
   ldrSmtTree(appPtr app_header, appTreePtr sub_tree = 0);
   ldrSmtTree(smtHeaderPtr smt_header, smtTreePtr sub_tree = 0);
   ldrSmtTree(const ldrSmtTree &);
   void build_selection(const ldrSelection& olds, OperPoint& app_point);
   virtual void insert_obj(objInserter*, objInserter *);
   virtual void remove_obj(objRemover *, objRemover *);
   copy_member(ldrSmtTree);
   define_relational(ldrSmtTree,ldrHeader);
   virtual void assign_obj(objAssigner*, objAssigner*);
 private:
   void redo_insert(objInserter*);
};

generate_descriptor(ldrSmtTree,ldrHeader);


#endif

