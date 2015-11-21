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
// spd_get_selection_array.C
//------------------------------------------
// synopsis:
//   Returns array of currently selected spd objects.
//
// description:
//   Gets selected objects from the global selection stack.
//
//------------------------------------------
// Fixed Syntax Errors
//
// Revision 1.2  91/09/16  14:44:15  sergey
// Added clear_selection().
//
// Revision 1.1  91/09/13  10:44:21  sergey
// Initial revision
//
//------------------------------------------

// include files

#include "genError.h"
#include "OperPoint.h"
#include "objArr.h"
#include "driver.h"
#include "symbolArr.h"

//------------------------------------------
// spd_get_selection_array
//
// Function to return all spd selected nodes
//------------------------------------------

#if 0
/* dead code */
symbolArr spd_get_selection_array()
{
    Initialize(spd_get_selection_array);

    symbolArr return_array;
    objArr tmp1 = (driver_instance->sel_list).oper_pts;

    Obj* el;
    ForEach (el, tmp1) {
	OperPoint* pt = (OperPoint*)el;
	return_array.insert_last(pt->node);
    }

    return return_array;
}
#endif

void spd_fill_selection_array (symbolArr &smt_objs)
{
    Initialize (spd_fill_selection_array);

    objArr& tmp1 = driver_instance->sel_list.oper_pts;

    Obj* el;
    ForEach (el, tmp1) {
	OperPoint* pt = (OperPoint*)el;
	if ((int)pt->subnode)
	    smt_objs.insert_last(pt->subnode);
	else
	    smt_objs.insert_last(pt->node);
    }
}

/*
   START-LOG-------------------------------------------

   $Log: spd_get_selection_array.cxx  $
   Revision 1.3 1994/08/01 18:30:57EDT builder 
   
Revision 1.2.1.4  1993/01/03  20:28:21  aharlap
changed objArr to symbolArr

Revision 1.2.1.3  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:44:25  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
