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
/*  	class defs for a doubly linked list of pointers and
 *  	is basically copied out of Stroustup's C++ book Section 7.3.2
 *  	slists are used to implement lists of nodes and lists of edges,
 *  	in particular the successor and predecessor lists for nodes
 */

#ifndef slist_included
#define slist_included

typedef void* ent;

class slink {
    slink* next;
    slink* prev;
    ent e;
    slink(ent a, slink* n, slink* p) { e=a; next=n; prev=p; }
friend class slist;
friend class slist_iterator_forw;
friend class slist_iterator_back;
friend class slist_iterator;
friend class q_link;
friend class edge_queue;
friend class constr_queue;
};

class slist {
    slink* head;
    slink* tail;
    int del_link(slink* l);
public:
    int insert(ent a);
    int insertlist(slist* ll);
    int append(ent a);
    int append_unique(ent a);
    int appendlist(slist* ll);
    int find(ent a);
    int del(ent a);
    ent get();
    ent gethead();
    ent gettail();
    ent getbefore (ent a);
    int addbefore(ent a, ent b);
    ent first()         { return head ? head->e : 0; }
    ent second()        { return ((head)&&(head->next)) ? head->next->e : 0; }
    void clear();
    int length();
    void newlist()      { head=0; tail=0; }
    int operator==(slist& ll);
    int equal(slist& ll);
    int includes(slist& ll);
    void del_all();

    slist()             { head=0; tail=0; }
    slist(ent a)        { head=new slink(a, 0, 0); tail=head;}
    ~slist()            { clear(); }
friend class slist_iterator_forw;
friend class slist_iterator_back;
friend class slist_iterator;
friend class edge_queue;
friend class constr_queue;
};

class slist_iterator_forw {
    slink* ce;
    slist* cs;
public:
    slist_iterator_forw(slist& s)       { cs = &s; ce = 0; }

    ent operator()() { slink* ll;
        if (ce == 0) { ce = cs->head;  ll = ce; }
        else { ce = ce->next; ll = (ce==cs->head)?0:ce; }
        return ll ? ll->e : 0;
    }

    ent back() { slink* ll;
        if (ce == 0) { ce = cs->tail;  ll = ce; }
        else { ce = ce->prev; ll = (ce==cs->tail)?0:ce; }
        return ll ? ll->e : 0;
    }

};

class slist_iterator_back {
    slink* ce;
    slist* cs;
public:
    slist_iterator_back(slist& s)       { cs = &s; ce = 0; }

    ent operator()() { slink* ll;
        if (ce == 0) { ce = cs->tail;  ll = ce; }
        else { ce = ce->prev; ll = (ce==cs->tail)?0:ce; }
        return ll ? ll->e : 0;
    }
};

#endif


/*
   START-LOG-------------------------------------------

   $Log: slist.h  $
   Revision 1.2 2000/07/10 23:03:47EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2.1.5  1993/05/13  20:24:17  sergey
 * Initial Revision.
 *


   END-LOG---------------------------------------------

*/
