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
#include <xref.h>
#include <xref_priv.h>
#include <RTL.h>
#include <RTL_externs.h>

#include <cLibraryFunctions.h>
#include <groupHdr.h>


int appTree_construct_symbolPtr(const appTree* that, symbolPtr& sym) 
{
    int retval = 1;
    Obj* obj = def_assocnode_get_dds (that);

    if (obj && obj->size() == 1) {
	Obj * el;
	ForEach (el,*obj)
	    break;
	
	obj = el ;
    } else
	obj = get_relation(ref_dd_of_app, that);

    if (obj) {
	sym = obj;
	retval = 1;
    }

    return retval;
}

int app_construct_symbolPtr(const app* that, symbolPtr& sym) 
{
    int retval = 0;

    Obj *dd = get_relation(dd_of_appHeader,that);

    if (dd) {
	sym = dd;
	retval = 1;
    }

    return retval;
}

void do_touch_ddElement(smtTree * trg)
{
  Initialize(do_touch_ddElement);
  if (!trg) return;
  
  ddElement * dm = checked_cast(ddElement, get_relation(def_dd_of_smt, trg));
  if (!dm)
      dm = checked_cast(ddElement, get_relation(decl_dd_of_smt, trg));
  // ddElement *dm = smt_get_dd(trg);
  if (dm)
  {
    dm->set_changed_since_saved(1);
    dm->set_changed_since_checked_in(1);
    dm->set_file_not_checked_in(1);
    dm->set_sw_tracking(1);
  }
}
