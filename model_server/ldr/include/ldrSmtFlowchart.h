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
#ifndef _ldrSmtflowchart_h_
#define _ldrSmtflowchart_h_

#include "genError.h"
#include "objRelation.h"
#include "smt.h"
#include "ldrHeader.h"
#include <symbolPtr.h>

// This is ldr header for flowchart diagrams of
// Abstract Syntax Tree (smt).


class ldrSmtFlowchart : public ldrHeader {
 public:
   ldrSmtFlowchart(smtTreePtr tr);
   ldrSmtFlowchart(smtHeaderPtr app_header, smtTreePtr sub_tree = 0);
   ldrSmtFlowchart(const ldrSmtFlowchart &);
   void build_selection(const ldrSelection&, OperPoint& app_point);
   virtual void insert_obj(objInserter*, objInserter *);
   virtual void remove_obj(objRemover *, objRemover *);
   copy_member(ldrSmtFlowchart);
   define_relational(ldrSmtFlowchart,ldrHeader);
 private:
   symbolPtr symbol;
   void redo_insert(objInserter*);
};

generate_descriptor(ldrSmtFlowchart,ldrHeader);


#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrSmtFlowchart.h  $
   Revision 1.4 1998/04/21 13:21:36EDT aharlap 
   Reviewed by: mg \n Test log: /net/oxen/vol02/tmp/ah.tmp/test_outdir/test_outdir/test.log \n Update for els
 * Revision 1.2.1.3  1993/03/18  20:28:05  aharlap
 * cleanup for paraset compatibility
 *
 * Revision 1.2.1.2  1992/10/09  19:50:29  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
