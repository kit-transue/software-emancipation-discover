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
//////////////////////   FILE SoftAssoc.h.C  ///////////////////////
// synopsis :
//     
#include "SoftAssoc.h"
#include "objOper.h"
#include "Relational.h"

#include <objCollection.h>

init_relation (el_of_softassoc, MANY, 0, softassoc_of_el, MANY, 0);
init_relation (softid_of_softassoc, 1, 0, softassoc_of_softid, MANY, relationMode::D);


//****************************
// notify_softassoc  public 
//***************************
void notify_softassoc (appTree *node, objOperType oper, int flags)
{
    if (!node) return;

    SoftNotifierInfo sn (node, oper, flags);
    sn . notify_softassoc (node);  // private
}

//****************************
// notify_softassoc  public 
//***************************
void notify_softassoc (appTree *node, int flags)
{
    if (!node) return;

    SoftNotifierInfo sn (node, NULLOP, flags);
    sn . notify_softassoc (node);  // private
}

//**************************
// notify_softassoc private
//**************************
extern void obj_notify(int, Relational*);
void SoftNotifierInfo::notify_softassoc (appTree *node)
{
    Initialize(SoftNotifierInfo::notify_softassoc );

// first call generic (non-softassoc) notify;
    obj_notify(flags, node);

    appTree *parent = checked_cast(appTree, node->get_parent());

    if (parent)
	this->notify_softassoc (parent);  // private
}

//**************************
// notify_softassoc private
//**************************
void SoftNotifierInfo::notify_softassoc (Obj *obj, appTree *node)
{

    if (obj->collectionp()) {
        Obj *el;
	ForEach(el, *obj) {
	    this->notify_softassoc (el, node);
	}
    }
}

/*
   START-LOG-------------------------------------------

   $Log: SoftAssoc.h.cxx  $
   Revision 1.4 2002/03/04 17:24:14EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.5  1993/12/06  19:11:26  mg
Bug track: 4466
obsoleted assoc

Revision 1.2.1.4  1993/10/04  21:15:31  trung
Bug track: 4909
fix deleting a file

Revision 1.2.1.3  1992/11/23  18:49:54  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:53:03  boris
*** empty log message ***

   Revision 1.2.1.1  92/10/07  20:49:58  smit
   *** empty log message ***

   Revision 1.2  92/10/07  20:49:56  smit
   *** empty log message ***

   Revision 1.1  92/10/07  18:21:14  smit
   Initial revision

   Revision 1.1  92/10/07  17:56:52  smit
   Initial revision

   Revision 1.4  92/08/11  11:44:15  mg
   notify
   
   Revision 1.3  92/06/25  01:02:48  aharlap
   cleanup for GNU 
   
   Revision 1.2  92/05/21  07:44:56  builder
   Comment out HyperText stuff for now.
   
   Revision 1.1  92/05/20  18:39:33  hagit
   Initial revision
   
   END-LOG---------------------------------------------

*/
