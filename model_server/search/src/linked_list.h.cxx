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
// linked_list
//------------------------------------------
// synopsis:
// linked list implementation
//
// description:
// This file includes implementation of singly- and doubly-linked
// list base classes.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#include "linked_list.h"

/***********************************************
 * slink::link_after(slink*, slink*&, slink*&) *
 ***********************************************/

void slink::link_after(slink* p, slink*& head, slink*& tail) {
   if (p) {
      fwd_link = p->fwd_link;
      p->fwd_link = this;
   }
   else {
      fwd_link = head;
      head = this;
   }

   if (!fwd_link)
      tail = this;
}

/************************************************
 * slink::unlink_from(slink*, slink*&, slink*&) *
 ************************************************/

void slink::unlink_from(slink* p, slink*& head, slink*& tail) {
   if (this == head)
      head = fwd_link;
   else p->fwd_link = fwd_link;

   if (this == tail)
      tail = p;
}

/************************************************
 * dlink::link_before(dlink*, dlink*&, dlink*&) *
 ************************************************/

void dlink::link_before(dlink* p, dlink*& head, dlink*& tail) {
   if (p) {
      bwd_link = p->bwd_link;
      p->bwd_link = this;
   }
   else {
      bwd_link = tail;
      tail = this;
   }

   if (bwd_link) {
      fwd_link = bwd_link->fwd_link;
      bwd_link->fwd_link = this;
   }
   else {
      fwd_link = head;
      head = this;
   }
}

/***********************************************
 * dlink::link_after(dlink*, dlink*&, dlink*&) *
 ***********************************************/

void dlink::link_after(dlink* p, dlink*& head, dlink*& tail) {
   if (p) {
      fwd_link = p->fwd_link;
      p->fwd_link = this;
   }
   else {
      fwd_link = head;
      head = this;
   }

   if (fwd_link) {
      bwd_link = fwd_link->bwd_link;
      fwd_link->bwd_link = this;
   }
   else {
      bwd_link = tail;
      tail = this;
   }
}

/***********************************
 * dlink::unlink(dlink*&, dlink*&) *
 ***********************************/

void dlink::unlink(dlink*& head, dlink*& tail) {
   if (bwd_link)
      bwd_link->fwd_link = fwd_link;
   else head = fwd_link;

   if (fwd_link)
      fwd_link->bwd_link = bwd_link;
   else tail = bwd_link;
}

/*
    START-LOG-------------------------------

    $Log: linked_list.h.cxx  $
    Revision 1.1 1993/07/28 19:53:37EDT builder 
    made from unix file
Revision 1.2.1.2  1992/10/09  19:14:48  builder
*** empty log message ***


    END-LOG---------------------------------
*/

