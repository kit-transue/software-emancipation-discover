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
// -- ldr tree node corresponding to a smt Region                  
//

#include "SmtSte.h"
#include "objOper.h"
#include "genError.h"
#include "smt.h"
#include "ldrSmodNode.h"
#include "steHeader.h"
#include "steDisplayNode.h"
#include "ste_smod.h"

init_relational(ldrSmodNode,steDisplayNode);
init_relation(ldr_of_smtend,MANY,NULL,smtend_of_ldr,1,NULL);

void ldrSmodNode::print( ostream& os, int level ) const {
 Initialize(ldrSmodNode::print);
   gen_print_indent(os, level);
   os << "end smtTreePtr "  << (void *) end_smt() << " text `";
   send_chars( os, 1 );
   os << "'end of text ";
}

ldrSmodNode::ldrSmodNode (smtTreePtr st, smtTreePtr en) :
    end_node(0)
{
  Initialize(ldrSmodNode::ldrSmodNode);
   end_smt(checked_cast(smtTree,en->get_last_leaf()));
   st = st->get_first_leaf();
   set_appTree (st);   
   set_node_type (steREG);
   smod_vision = SmodRegion;
}

smtTreePtr ldrSmodNode::end_smt () const
{
    Initialize(ldrSmodNode::end_smt);
    smtTreePtr en = checked_cast(smtTree,get_relation(smtend_of_ldr,this));
    return en;
}

void ldrSmodNode::end_smt (smtTreePtr en) 
{
    Initialize(ldrSmodNode::end_smt);
    if (en == NULL) {
	en = checked_cast(smtTree,get_relation(smtend_of_ldr,this));
	if (en)
	    rem_relation(smtend_of_ldr,this,en);
    } else
	put_relation(smtend_of_ldr,this,en);
    
}

void ldrSmodNode::send_string( ostream& os ) const 
{
    Initialize(ldrSmodNode::send_string);
    smtTreePtr st = checked_cast(smtTree,ldrtree_get_apptree( this ));
    int len = smt_send_region( os, st, end_smt(), 0 ); // format = 0; 
    if ( len == 0 ){
        steHeader(Message)
	    << "ldrSmodeNode::send_string - empty region."
	    << steEnd;
    }
}

int ldrSmodNode::send_chars( ostream& os, int format ) const {
  Initialize(ldrSmodNode::send_chars);
     smtTreePtr st = checked_cast(smtTree,ldrtree_get_apptree( this ));
     int len = smt_send_region (os, st, end_smt(), format);
     return len;
}
