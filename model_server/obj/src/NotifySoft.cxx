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
//////////////////////   FILE NotifySoft.C  ///////////////////////
// synopsis :
//     

#include "objOper.h"
#include "SoftAssoc.h"
#include "RelationMonitor.h"

#include <objOperate.h>

//---- INSERT -----

void app::notify_softassoc_insert (objInserter *, objInserter *ni)
{
  Initialize (app::notify_softassoc_insert);

  if (is_RelationMonitor(this)) Return; 
  if (is_RTL(this)) Return; 


  if (ni->type == NULLOP) // No changes.
      Return;

  notify_softassoc (checked_cast(appTree,ni->targ_obj), ni->type, relationMode::M);

  if (ni->src_obj != ni->targ_obj)
        notify_softassoc (checked_cast(appTree,ni->src_obj), ni->type, relationMode::M);

   return;

}

  
//---------- DELETE --------------
void app::notify_softassoc_remove (objRemover *, objRemover *nd)
{
  Initialize (app::notify_softassoc_remove);

  if (is_RelationMonitor(this)) Return; 
  if (is_RTL(this)) Return; 

   notify_softassoc (checked_cast(appTree,nd->src_obj), relationMode::D);

   return;

}

  
//---------- MERGE --------------

void app::notify_softassoc_merge (objMerger *, objMerger *nm)
{
  Initialize (app::notify_softassoc_merge);

  if (is_RelationMonitor(this)) Return; 
  if (is_RTL(this)) Return; 


  notify_softassoc (checked_cast(appTree,nm->src_obj), relationMode::M);
  notify_softassoc (checked_cast(appTree,nm->targ_obj), relationMode::M);

   return;

}



//---------- SPLIT --------------

void app::notify_softassoc_split (objSplitter *os, objSplitter *ns)
{
  Initialize (app::notify_softassoc_split);
  Ignore (os);

  if (is_RelationMonitor(this)) Return; 
  if (is_RTL(this)) Return; 

  notify_softassoc (checked_cast(appTree,ns->bottom_obj), relationMode::M);

  return;

}
/*
   START-LOG-------------------------------------------

   $Log: NotifySoft.cxx  $
   Revision 1.4 2002/03/04 17:24:09EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.3  1992/11/23  18:49:54  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:52:59  boris
*** empty log message ***


   Revision 1.2.1.1  92/10/07  20:49:46  smit
   *** empty log message ***

   Revision 1.2  92/10/07  20:49:45  smit
   *** empty log message ***

   Revision 1.1  92/10/07  18:21:13  smit
   Initial revision

   Revision 1.1  92/10/07  17:56:52  smit
   Initial revision

   Revision 1.2  92/08/11  11:43:45  mg
   dont call for RTL
   
   Revision 1.1  92/05/21  09:03:57  hagit
   Initial revision
   
   END-LOG---------------------------------------------

*/
