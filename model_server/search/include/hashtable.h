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
#ifndef __hashtable
#define __hashtable

// hashtable
//------------------------------------------
// synopsis:
// hash table classes
//
// description:
// This header defines three abstract classes which can be used for hash
// table data storage and retrieval.  Class hash_item is used as a base
// class for the items to be stored; items are stored in a singly-linked
// list based on a class derived from class hash_bucket.  Class hash_table
// is the base for the entire table.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#include "linked_list.h"

class hash_item: public slink {
public:
   virtual ~hash_item() { }
   hash_item* next() { return (hash_item*) slink::next(); }
   virtual int operator==(const hash_item&) const = 0;
};

class hash_bucket {
public:
   hash_bucket(): head(0), tail(0) { }
   hash_item* find(const hash_item&);
   hash_item* find_or_insert(hash_item*);
   void insert(hash_item*);
   void remove(hash_item&);
   hash_item* first() { return (hash_item*) head; }
private:
   slink* head;
   slink* tail;
};

class hash_table {
public:
   hash_table(size_t num_buckets);
   virtual ~hash_table();
   hash_item* find(const hash_item&);
   hash_item* find_or_insert(hash_item*);
   void insert(hash_item*);
   void remove(hash_item&);
protected:
   size_t bucket_count;
   hash_bucket* bucket_list;
private:
   virtual size_t hash(const hash_item*) = 0;
};

#endif

/*
    START-LOG-------------------------------

    $Log: hashtable.h  $
    Revision 1.2 1996/04/04 15:26:56EST wmm 
    Bug track: N/A
    Incorporate 4.0.0patch changes to groups/subsystems into main branch.  (Regression scripts already submitted.)
 * Revision 1.2.1.2  1992/10/09  19:13:06  builder
 * *** empty log message ***
 *

    END-LOG---------------------------------
*/

