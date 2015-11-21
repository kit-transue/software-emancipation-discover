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
#ifndef _EDGELIST_H
#define _EDGELIST_H

/* 	class definition for a list of edges (see slist.h, slist.C)
 */

#include "edge.h"

class elist : public slist{
public:
    elist()		{}
    elist(edge* a):slist(a)	{}
    ~elist()		{slist::clear();}
    void insert(edge* a){slist::insert(a);}
    void insertlist(elist* ll) {slist::insertlist((slist*) ll);}
    void append(edge* a){slist::append(a);}
    void appendlist(elist* ll) {slist::appendlist((slist*) ll);}
    void del(edge* a) 	{slist::del(a);}
    edge* get()		{return(edge *)slist::get();}
    void clean()	{slist::clear();}
};

/*
   START-LOG-------------------------------------------

   $Log: edgelist.h  $
   Revision 1.1 1993/07/28 19:45:43EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  19:03:21  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/
#endif
