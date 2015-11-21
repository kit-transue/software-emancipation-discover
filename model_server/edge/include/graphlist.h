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
#ifndef _GRAPHLIST_H
#define _GRAPHLIST_H

/* 	class definition for a list of graphs (see slist.h, slist.C)
 */

class glist : public slist {
public:
    glist()		{}
    glist(graph* a):slist(a)	{}
    ~glist()		{slist::clear();}
    void insert(graph* a){slist::insert(a);}
    void insertlist(glist* ll) {slist::insertlist((slist*) ll);}
    void append(graph* a){slist::append(a);}
    void append_unique(graph* a){slist::append_unique(a);}
    void appendlist(glist* ll) {slist::appendlist((slist*) ll);}
    int find(graph* a)   {return slist::find(a);}
    int addbefore(graph* a, graph*b)   {return slist::addbefore(a,b);}
    void del(graph* a) 	{slist::del(a);}
    graph* get()		{return(graph *)slist::get();}
    graph* gethead()	{return(graph *)slist::gethead();}
    graph* first()       {return(graph *)slist::first();}
    graph* second()      {return(graph *)slist::second();}
    graph *getgraphbefore(graph *n)	{return(graph*)slist::getbefore(n);}
    void newlist()      {slist::newlist();}
    int length()	{return slist::length();}
    void clean()	{slist::clear();}
    int operator==(glist& ll) {return slist::operator==((slist&) ll);}
    int equal(glist& ll) {return slist::equal((slist&) ll);}
    int includes(glist& ll) {return slist::includes((slist&) ll);}
};
/*
   START-LOG-------------------------------------------

   $Log: graphlist.h  $
   Revision 1.1 1993/07/28 19:45:43EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  19:03:23  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
