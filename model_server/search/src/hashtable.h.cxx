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
// hashtable
//------------------------------------------
// synopsis:
// hash table implementation
//
// description:
// This file provides the implementation of the non-pure-virtual member
// functions in the hash table classes.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "genError.h"
#include "hashtable.h"

//------------------------------------------
// hash_bucket::find(const hash_item&)
//------------------------------------------

hash_item* hash_bucket::find(const hash_item& item) {
   Initialize(hash_bucket::find);

   hash_item* p;
   for (p = first(); p && !(*p == item); p = p->next()) ;

   return p;
}


//------------------------------------------
// hash_bucket::find_or_insert(hash_item*)
//------------------------------------------

hash_item* hash_bucket::find_or_insert(hash_item* itemp) {
   Initialize(hash_bucket::find_or_insert);

   hash_item* p;
   for (p = first(); p && !(*p == *itemp); p = p->next()) ;

   if (!p) {
      itemp->link_after(tail, head, tail);
      p = itemp;
   }

   return p;
}


//------------------------------------------
// hash_bucket::insert(hash_item*)
//------------------------------------------

void hash_bucket::insert(hash_item* itemp) {
   Initialize(hash_bucket::insert);

   itemp->link_after(NULL, head, tail);
}


//------------------------------------------
// hash_bucket::remove(hash_item*)
//------------------------------------------

void hash_bucket::remove(hash_item& item) {
   Initialize(hash_bucket::remove);

   hash_item* prev = 0;
   hash_item* p;
   for (p = first(); p && !(*p == item); prev = p, p = p->next()) ;
   if (p)
      p->unlink_from(prev, head, tail);

   return;
}


//------------------------------------------
// hash_table::hash_table(size_t)
//------------------------------------------

hash_table::hash_table(size_t num_buckets): bucket_count(num_buckets),
      bucket_list(new hash_bucket[num_buckets]) { }


//------------------------------------------
// hash_table::~hash_table()
//------------------------------------------

hash_table::~hash_table() {
   delete [] bucket_list;
}


//------------------------------------------
// hash_table::find(hash_item&)
//------------------------------------------

hash_item* hash_table::find(const hash_item& item) {
   Initialize(hash_table::find);

   size_t idx = hash(&item) % bucket_count;
   return bucket_list[idx].find(item);
}


//------------------------------------------
// hash_table::find_or_insert(hash_item*)
//------------------------------------------

hash_item* hash_table::find_or_insert(hash_item* itemp) {
   Initialize(hash_table::find);

   size_t idx = hash(itemp) % bucket_count;

   return bucket_list[idx].find_or_insert(itemp);
}


//------------------------------------------
// hash_table::insert(hash_item*)
//------------------------------------------

void hash_table::insert(hash_item* itemp) {
   Initialize(hash_table::insert);

   size_t idx = hash(itemp) % bucket_count;

   bucket_list[idx].insert(itemp);
}


//------------------------------------------
// hash_table::remove(hash_item&)
//------------------------------------------

void hash_table::remove(hash_item& item) {
   Initialize(hash_table::remove);

   size_t idx = hash(&item) % bucket_count;
   bucket_list[idx].remove(item);

   return;
}

/*
    START-LOG-------------------------------

    $Log: hashtable.h.cxx  $
    Revision 1.3 2000/07/10 23:08:54EDT ktrans 
    mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.2  1992/10/09  19:14:47  builder
*** empty log message ***


    END-LOG---------------------------------
*/

