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
////////////////////   FILE ldrSmodNode.h  //////////////////////
//
// -- Contains  related Class Definition                  
//
#ifndef ldrSmodNode_h
#define ldrSmodNode_h

#include "objOper.h"
#include "genError.h"
#include "smt.h"
//#include "steDisplayNode.h"

define_relation(ldr_of_smtend,smtend_of_ldr);

//
//    Class ldrSmodNode
//

class ldrSmodNode : public steDisplayNode {
 public :
    smtTreePtr end_node;       // redundant; need to be deleted eventually

  ldrSmodNode(smtTreePtr, smtTreePtr);

#ifdef __GNUG__
  ldrSmodNode (const ldrSmodNode& oo) {*this = oo;}
#endif
  copy_member(ldrSmodNode);
  virtual void print(ostream&, int) const;
  define_relational(ldrSmodNode,steDisplayNode);

  virtual void send_string( ostream& ) const;
  int send_chars( ostream&, int format ) const;
  smtTreePtr end_smt() const;
  void end_smt (smtTreePtr en);
};
  generate_descriptor(ldrSmodNode,steDisplayNode);
#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrSmodNode.h  $
   Revision 1.1 1994/02/16 10:45:01EST builder 
   made from unix file
// Revision 1.2  1994/02/16  15:21:46  boris
// Bug track: #6248
// Move ste_finalize() before real_stmt execution
//
// Revision 1.1  1993/03/26  04:37:41  boris
// Initial revision
//

   END-LOG---------------------------------------------

*/
