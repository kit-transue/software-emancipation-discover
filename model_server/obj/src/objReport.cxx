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
// objReport.C
//
// description : a set of calls to notify related objects about
//               certain actions which can not be performed by
//               standard propagation mechanism (obj_delete,
//               obj_insert, etc.)
//
#include "Object.h"
#include "objRelation.h"
#include "objTree.h"

// obj_report_arg_rm  - reports removal of argument 
//                      notifying related RTL's

extern void obj_notify(int,Relational*);

// Deleted used argument del_ptr, to elimiate compiler warning.
void obj_report_arg_rm(Obj *arg_ob, objDeleter *)
{
   obj_notify(relationMode::D,(Relational *)arg_ob);
}
/*
   START-LOG-------------------------------------------

   $Log: objReport.cxx  $
   Revision 1.3 2002/03/04 17:24:30EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.2  1992/10/09  18:55:29  boris
Fix comments

   Revision 1.2.1.1  92/10/07  20:51:15  smit
   *** empty log message ***

   Revision 1.2  92/10/07  20:51:14  smit
   *** empty log message ***

   Revision 1.1  92/10/07  18:21:21  smit
   Initial revision

   Revision 1.1  92/10/07  17:56:58  smit
   Initial revision

   Revision 1.3  92/08/11  11:45:50  mg
   notify
   
   Revision 1.2  92/01/02  11:09:14  mg
   *** empty log message ***
   
   Revision 1.1  91/11/05  14:15:33  sergey
   Initial revision
  
   History:   11/05/91     S.Spivakovsky     Initial coding.

   END-LOG---------------------------------------------

*/


