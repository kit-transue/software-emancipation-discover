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
//
// smt_get_node_type
//
// synopsis : Gets smt nodes of given type in the tree 
//
// description: Starts from smt (sub)root and walks the tree. Collects all
//              smt nodes of given type and puts them into the Set. The call
//              does not go recursively for the sake of performance. So, 
//              from the block in block construction only the top one
//              will be returned.

#include "smt.h"
#include <objSet.h>

void smt_get_node_type(smtTreePtr smt_root, int smt_type, objSet &smt_nodes)
{
   Initialize(smt_get_node_type);

  smtTreePtr  next_child = checked_cast(smtTree, smt_root->get_first());

  if (smt_root->get_node_type() == smt_type){
        smt_nodes.insert( smt_root );
        return;               // if type is found don't go deeper 
  }

  while ( next_child ){
        smt_get_node_type( next_child, smt_type, smt_nodes );
        next_child = checked_cast(smtTree, next_child->get_next());
  }
  return;
}

