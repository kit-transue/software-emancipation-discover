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
// objRegenerator.h.C
//
// Contains regenerator related methods.
//
#include "objCollection.h"
#include "objPropagate.h"
#include "objOperate.h"     
#include "app.h"  //#include <ldr.h>

// class objRegenerator methods :


  int objRegenerator::apply(RelationalPtr )
      {return 0;}

  objPropagator * objRegenerator::accept(RelationalPtr hdr, 
                                        int )
  {
      int cont;
     Initialize(objRegenerator::accept);
      HierarchicalPtr  hd = checked_cast(Hierarchical, hdr);
      objRegenerator * nextRegenerator = new objRegenerator(*this);
      cont = hd->regenerate_obj(this->targ_obj);
      ReturnValue(cont==0 ? NULL : nextRegenerator);
 }


// Generic header methods related to regenerator

#ifdef XXX_ldr__regenerate_obj
   int ldr::regenerate_obj(objTreePtr ob)
   {
    appTreePtr curr_node;
    Initialize(ldr::regenerate_obj);

//  call obj_modify with dummy data to regenerate ldr on
//  first visible node

    curr_node = checked_cast(appTree, ob);
    while (curr_node){
         if ( this->find_ldr(curr_node) )
              break;
         else
              curr_node = checked_cast(appTree, (curr_node->get_parent()));
    }
    if(curr_node != NULL)
    {
       objInserter   ins_ptr(REPLACE, curr_node, curr_node, NULL);
       obj_modify(*this, &ins_ptr);
    }

//  stop propagation

  ReturnValue(0);
}
#endif
 
 int app::regenerate_obj(objTreePtr ob)
 {
    Initialize(app::regenerate_obj);

//  do nothing for application 

    Ignore(ob);
    ReturnValue((int) this);
}


void obj_regenerate(appPtr hdr, appTreePtr targ)
{
   Initialize(obj_regenerate);

    objRegenerator reg_ptr(targ);

    obj_modify(*hdr, &reg_ptr);
}
