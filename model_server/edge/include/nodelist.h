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
#ifndef _NODELIST_H
#define _NODELIST_H

/* 	class definition for a list of nodes (see slist.h, slist.C)
 */

#include "node.h"

class nlist : public slist {
public:
    nlist()		{}
    nlist(node* a):slist(a)	{}
    ~nlist()		{slist::clear();}
    void insert(node* a){slist::insert(a);}
    void insertlist(nlist* ll) {slist::insertlist((slist*) ll);}
    void append(node* a){slist::append(a);}
    void append_unique(node* a){slist::append_unique(a);}
    void appendlist(nlist* ll) {slist::appendlist((slist*) ll);}
    int find(node* a)   {return slist::find(a);}
    int addbefore(node* a, node*b)   {return slist::addbefore(a,b);}
    void del(node* a) 	{slist::del(a);}
    node* get()		{return(node *)slist::get();}
    node* gethead()	{return(node *)slist::gethead();}
    node* gettail()     {return(node*)slist::gettail();}
    node* first()       {return(node *)slist::first();}
    node* second()      {return(node *)slist::second();}
    node *getnodebefore(node *n)	{return(node*)slist::getbefore(n);}
    void newlist()      {slist::newlist();}
    int length()	{return slist::length();}
    void clean()	{slist::clear();}
    int operator==(nlist& ll) {return slist::operator==((slist&) ll);}
    int equal(nlist& ll) {return slist::equal((slist&) ll);}
    int includes(nlist& ll) {return slist::includes((slist&) ll);}
};
/*
   START-LOG-------------------------------------------

   $Log: nodelist.h  $
   Revision 1.1 1993/07/28 19:45:45EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  19:03:31  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
