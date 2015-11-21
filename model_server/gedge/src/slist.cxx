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
/*  	The following implements a doubly linked list of pointers and
 *  	is basically copied out of Stroustup's C++ book Section 7.3.2
 *  	slists are used to implement lists of nodes and lists of edges,
 *  	in particular the successor and predecessor lists for nodes
 */

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#include <strstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "slist.h"

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */
#ifdef __cplusplus
}
#endif /* __cplusplus */

/*
 * insert
 */
int 
slist::insert(ent a)
{

    if (head) {
	slink* f = new slink(a, head, 0);
	head->prev = f;
	head = f;
    } else {
	head = new slink(a, 0, 0);
	head->next = 0;
	tail = head;
	tail->prev = 0;
    }
    return 0;
}

/*
 * append
 */
int 
slist::append(ent a)
{

    if (tail) {
	slink* f = new slink(a, 0, tail);
	tail->next = f;
	tail = f;
    } else {
	head = new slink(a, 0, 0);
	head->next = 0;
	tail = head;
	tail->prev = 0;
    }
    return 0;
}

/*
 * append_unique
 */
int 
slist::append_unique(ent a)
{
    if (!find(a))
	append(a);
    return 0;
}

/*
 * del
 */
int 
slist::del(ent a)
{
    int match = 0;
    slink* l = head;
#if 0  // unused
    slink* ll = l;
#endif
    if (l == 0) {
	return 0;
    }
    // find slink you want to delete  -- when find set flag var "match" to 1
    do {
	if (l->e == a) 
	    match = 1;
	else
	    l = l->next;
    } while ((l != 0) && (match != 1));

    // delete link pointed l
    return  del_link(l);
}

/*
 * del_all
 */
void
slist::del_all()
{
    slink* l = head;

    // loop thru the links deleting the elements - not the links
    while ( l ){
        delete l->e;
        l = l->next;
    }
    return;
}

/*
 * del_link
 */
int
slist::del_link(slink* l)
{
     // delete link pointed l
    if (l == 0)
        return 0;
    if (l->next != 0)
        l->next->prev = l->prev;
    if (l->prev != 0)
        l->prev->next = l->next;
    if (head == l)
        head = l->next;
    if (tail == l)
        tail = l->prev;
    delete l;
    return  0;
}

/*
 * get
 */
ent 
slist::get()
{
    if (head == 0)
      return 0;
    else {
      slink* f = head;
      head = head->next;
      if (head == 0) tail = 0;
      ent r = f->e;
      delete f;
      return r;
    }
}

/*
 * gethead
 */
ent 
slist::gethead()
{
    if (head == 0)
      return 0;
    else {
      ent r = head->e;
      return r;
    }
}

/*
 * gettail
 */
ent 
slist::gettail()
{
    if (tail == 0)
      return 0;
    else {
      ent r = tail->e;
      return r;
    }
}

/*
 * addbefore
        note:  nobody uses this return value, which is always zero
                                                      it should be void !!!!
 */
int 
slist::addbefore(ent a, ent b)
{
    if ((head == 0)||(b == 0)) {
      append(a);
      return 0;
    } 
    if (head->e == b) {
      insert(a);
      return 0;
    }
      slink* ptr = head;
      while (ptr) {
        if (ptr->e == b) {
	  slink* f = new slink(a, ptr, ptr->prev);
	  if (ptr->prev) ptr->prev->next = f;
	  if (ptr) ptr->prev = f;
  	  return 0;
        }
        ptr = ptr->next;
      }
    return 0;
}

/*
 * clear
 */
void 
slist::clear()
{
    slink* l = head;
    if (l == 0)
	return;
    do { 
      slink* ll = l;
      l = l->next;
      delete ll;
    } while (l != 0);
    head = 0;
    tail = 0;
}

/*
 * length
 */
int 
slist::length()
{
    int count = 0;
    slink* l = head;
    if (l == 0)
	return 0;
    do { 
#if 0  // unused
      slink* ll = l;
#endif
      l = l->next;
      count++;
    } while (l != 0);
    return count;
}

/*
 * insertlist
 */
int 
slist::insertlist(slist* ll)
{
    ent a;
    int i = 0;
    slist_iterator_back next(*ll);
    while (a = next()) { 
      insert(a); i++; 
    }
    return i;
}

/*
 * appendlist
 */
int 
slist::appendlist(slist* ll)
{
    ent a;
    int i = 0;
    slist_iterator_forw next(*ll);
    while (a = next()) { 
      append(a); i++; 
    }
    return i;
}

/*
 * find
 */
int 
slist::find(ent a)
{
    int i = 0;
    slink* ptr = head;
    while (ptr) {
      i++;
      if (ptr->e == a) return i;
      ptr = ptr->next;
    }
    return 0;
}

ent 
slist::getbefore(ent a)
{
    slink* ptr = head;
    while (ptr) {
      if (ptr->e == a) return ptr->prev;
      ptr = ptr->next;
    }
    return 0;
}

/*
 * operator==
 * exact identity of lists
 */
int
slist::operator==(slist& ll)
{
  slink* ptr1 = head;
  slink* ptr2 = ll.head;
  while ((ptr1) && (ptr2)) {
    if (ptr1->e != ptr2->e) return 0;
    ptr1 = ptr1->next;
    ptr2 = ptr2->next;
  }
  if ((ptr1) || (ptr2)) return 0;
  return 1;
}

/*
 * equal
 * lists includes same elements
 */
int
slist::equal(slist& ll)
{
  if (length() != ll.length()) return 0;
  for (slink* ptr1 = head; ptr1; ptr1 = ptr1->next) {
    for (slink* ptr2 = ll.head; ; ptr2 = ptr2->next) {
      if (ptr2 == 0) return 0;
      if (ptr1->e == ptr2->e) break;
    }
  }
  return 1;
}

/*
 * includes
 */
int
slist::includes(slist& ll)
{
  slist_iterator_forw s(ll);
  ent e;
  while (e = s()) {
    if (! find(e)) return 0;
  }
  return 1;
}
