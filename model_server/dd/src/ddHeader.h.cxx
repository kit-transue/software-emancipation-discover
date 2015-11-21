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
#include <objOper.h>
#include <objOperate.h>
#include "ddHeader.h"
#include "smt.h"
#include "ddict.h"

init_relational(ddHeader,app);

void rtl_delete_obj (RTLNodePtr rtl, ObjPtr obj);
void remove_dd_el(ddElement* );

void ddHeader::set_name( char *p)
{
  Initialize(ddHeader::set_name);
  strcpy(name, p);
  return;
}

void ddHeader::insert_obj(objInserter * , objInserter * )
{
  return;
}

void ddHeader::remove_obj( objRemover *ro, objRemover *)
{
  Initialize(ddHeader::remove_obj);

  RelationalPtr sob = ro->src_obj;

  if ( is_ddElement( sob) )
      remove_dd_el (ddElementPtr(sob));
}

void ddHeader::assign_obj(objAssigner *old_assigner, objAssigner *new_assigner)
{
    Initialize(ddHeader::assign_obj);
        
    objTree *src_obj = old_assigner->src_obj;
    new_assigner->slot = 0;
    new_assigner->src_obj = 0;
    if (src_obj && is_smtTree(src_obj))
    {
	ddElementPtr dd = smt_get_dd(checked_cast(smtTree,src_obj));
	new_assigner->slot = (dd ? old_assigner->slot : 0);
	new_assigner->src_obj = dd;
    }
    Return    
}

