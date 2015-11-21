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
//------------------------------------------
// objRawLdr_h.C
//------------------------------------------
//
// class:
//     objRawLdr
//
// description: ldr header for arbitrary Raw UNIX ascii file.
//

#include "objOperate.h"


#include "ldrHeader.h"
#include "objRawApp.h"
#include "objRawLdr.h"
#include "steDisplayNode.h"
#include <objOperate.h>
#include <messages.h>

init_relational(objRawLdr,ldrHeader);

void objRawLdr::build_selection(const ldrSelection&, OperPoint&){
#ifdef  _PSET_DEBUG
   dis_message (C_LDR, MSG_ERROR, "M_BUILDSELNOTIMPL");
#endif
}

objRawLdr::objRawLdr(objRawAppPtr appl, void *){
 Initialize(objRawLdr::objRawLdr);
   steDisplayNodePtr root = db_new(steDisplayNode,());
   root->set_node_type(steSTR);
   appTreePtr ap_r = checked_cast(appTree,appl->get_root());
   put_relation(apptree_of_ldrtree,root, ap_r);
   header_put_tree(this,root);
   put_relation(app_of_ldr,this,appl);
}

void objRawLdr::insert_obj(objInserter *oi, objInserter *ni)
{
    switch (oi->type) {
      case CHANGE_NAME:
	break;
      default:
#ifdef  _PSET_DEBUG
	dis_message (C_LDR, MSG_ERROR, "M_INSERTOBJNOTIMPL");
#endif
	ni->type = NULLOP;
    }
}
void objRawLdr::remove_obj(objRemover *, objRemover *){
#ifdef _PSET_DEBUG
   dis_message (C_LDR, MSG_ERROR, "M_REMOVEOBJNOTIMPL");
#endif
}

/*
   START-LOG-------------------------------------------

   $Log: objRawLdr_h.cxx  $
   Revision 1.7 1997/05/21 12:17:08EDT twight 
   Bug track: 14168
   Reviewed by: mb
   
   Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
// Revision 1.3  1993/08/27  21:05:39  davea
// bug 4467 - make the "not implemented" messages conditional,
// so they only show up in sun4-g version.
//
// Revision 1.2  1993/01/03  15:53:27  mg
// consolidated
//
// Revision 1.1  1992/12/11  22:17:33  boris
// Initial revision
//

   END-LOG---------------------------------------------

*/
