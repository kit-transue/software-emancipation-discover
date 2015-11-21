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
#include "objOperate.h"
#include "deepCopier.h"
#include "steTextNode.h"
#include "steStyle.h"
#include "steCategory.h"

RelationalPtr deepCopier::apply (RelationalPtr  obj)
{
    Initialize (deepCopier::apply);

    if (!is_appTree(obj)) ReturnValue(NULL);

    RelationalPtr new_obj = checked_cast(appTree,obj)->rel_copy_for_header
	(checked_cast(app, head), (objCopier *) this);

    if (!head && new_obj) {
	RelationalPtr st = get_starter_obj();
	if (obj == st) {
	    HierarchicalPtr h =
		checked_cast(Hierarchical, checked_cast(commonTree,new_obj)->get_header());
	    if (h)
		put_header_obj (h);
	}
    }

    ReturnValue (new_obj);
}

objPropagator *deepCopier::accept (RelationalPtr  obj, int /*level*/)
{
#if 0
    RelationalPtr st = this->get_starter_obj();

    if (st && is_objTree(st) &&
	obj == ( checked_cast(objTree, st ))->get_next())

	return NULL;
    else
#endif
	return this;
}

/*
   START-LOG-------------------------------------------

   $Log: deepCopier.cxx  $
   Revision 1.2 1996/04/01 12:51:25EST mg 
   cleanup
Revision 1.2.1.5  1993/04/05  21:53:55  aharlap
part of bug #2749

Revision 1.2.1.4  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.3  1992/10/15  14:43:35  mg
cut/paste modification

Revision 1.2.1.2  92/10/09  18:42:36  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
