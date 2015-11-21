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
#include <general.h>
#include <idfDict.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <iostream>
#endif /* ISO_CPP_HEADERS */

// Size of hash table. Kept low for now,
// for the sake of exercising the implementation.
int const tableSize = 11;

//============================================================================
// struct bucket, for the hash buckets
//============================================================================
struct bucket {
  bucket *next;
  void *index;
  void *value;
};


//============================================================================
// idfDictionary
//----------------------------------------------------------------------------
// dictionary class, with generic code for adaptation
// in classes with int and string-based indexes
//============================================================================
class idfDictionary {
public:
                  idfDictionary(
			       boolean  (*equal)(void *, void *),
			       unsigned (*hash)(void *),
			       int      (*size)(void *),
			       void     (*copy)(void *to, void *from)
			      );
                  ~idfDictionary();
  boolean         add(void *ptr_to_index, void *value);
  boolean         reassign(void *ptr_to_index, void *value);
  void           *lookup(void *ptr_to_index);
  void           *lookup(const idfIterator &iter);
  void           *index(const idfIterator &iter);
  boolean     	  contains(void *ptr_to_index);
  idfIterator     first();
  boolean         next(idfIterator &iter);
  boolean         done(const idfIterator &iter);
private:
  boolean  (*equal)(void *, void *);
  unsigned (*hash)(void *);
  int      (*size)(void *);
  void     (*copy)(void *to, void *from);

  bucket   *chain[tableSize];
  unsigned  chainNum(void *);
  idfIterator firstBucketAfter(int chain_num);
  bucket   *bucketOf(void *ptr_to_index);
};

//============================================================================
// implementation of ifdIntDictionary class
//============================================================================
static boolean intEqual(void *intval1, void *intval2)
{
  return *(int *)intval1 == *(int *)intval2;
}
static unsigned intHash(void *intval)
{
  return unsigned(*(int *)intval);
}
static int intSize(void *)
{
  return sizeof(int);
}
static void intCopy(void *to, void *from)
{
  *(int *)to = *(int *)from;
}
idfIntDictionary::idfIntDictionary()
{
  dict = new idfDictionary(&intEqual, &intHash, &intSize, &intCopy);
}
idfIntDictionary::~idfIntDictionary()
{
  delete dict;
}
boolean idfIntDictionary::add(int index, void *value)
{
  return dict->add(&index, value);
}
boolean idfIntDictionary::reassign(int index, void *value)
{
  return dict->reassign(&index, value);
}
void *idfIntDictionary::lookup(int index) const
{
  return dict->lookup(&index);
}
boolean idfIntDictionary::contains(int index) const
{
  return dict->contains(&index);
}
void *idfIntDictionary::lookup(const idfIterator &iter) const
{
  return dict->lookup(iter);
}
int idfIntDictionary::index(const idfIterator &iter) const
{
  return *(int *)dict->index(iter);
}
idfIterator idfIntDictionary::first() const
{
  return dict->first();
}
boolean idfIntDictionary::next(idfIterator &iter) const
{
  return dict->next(iter);
}
boolean idfIntDictionary::done(const idfIterator &iter) const
{
  return dict->done(iter);
}

//============================================================================
// copied string_hash from obj/src/objDictionary.C
//============================================================================
#define MAKE_POS(v) (v & 0x7fffffff) /* make number positive */
static unsigned string_hash(const char * sym)
{
   register int r = 0;

   unsigned const char * cur = (const unsigned char *)sym;
   while(cur[0])

     r = (r * 113) + *cur++;

   return MAKE_POS(r);
}

//============================================================================
// implementation of ifdStringDictionary class
//============================================================================
static boolean stringEqual(void *stringval1, void *stringval2)
{
  return strcmp((char *)stringval1, (char *)stringval2) == 0;
}
unsigned string_hash(const char * sym);
static unsigned stringHash(void *stringval)
{
  return string_hash((char *)stringval);
}
static int stringSize(void *stringval)
{
  // Include the terminating 0 because this is used for allocation.
  return strlen((char *)stringval) + 1;
}
static void stringCopy(void *to, void *from)
{
  strcpy((char *)to, (char *)from);
}
idfStringDictionary::idfStringDictionary()
{
  dict = new idfDictionary(&stringEqual, &stringHash, &stringSize, &stringCopy);
}
idfStringDictionary::~idfStringDictionary()
{
  delete dict;
}
boolean idfStringDictionary::add(const char *index, void *value)
{
  return index != NULL && dict->add((void *)index, value);
}
boolean idfStringDictionary::reassign(const char *index, void *value)
{
  return index != NULL && dict->reassign((void *)index, value);
}
void *idfStringDictionary::lookup(const char *index) const
{
  return index != NULL ? dict->lookup((void *)index) : NULL;
}
boolean idfStringDictionary::contains(const char *index) const
{
  return index != NULL && dict->contains((void *)index);
}
void *idfStringDictionary::lookup(const idfIterator &iter) const
{
  return dict->lookup(iter);
}
const char *idfStringDictionary::index(const idfIterator &iter) const
{
  return (const char *)dict->index(iter);
}
idfIterator idfStringDictionary::first() const
{
  return dict->first();
}
boolean idfStringDictionary::next(idfIterator &iter) const
{
  return dict->next(iter);
}
boolean idfStringDictionary::done(const idfIterator &iter) const
{
  return dict->done(iter);
}

//============================================================================
// implementation of idfDictionary
//============================================================================
// The index value is copied, but the value is captured.
// In case an entry is already present, returns false.
// It is up to the caller to manage storage of the value so that it sticks
// around as long as the dictionary is used.
//============================================================================
boolean idfDictionary::add(void *ptr_to_index, void *value)
{
  if (contains(ptr_to_index)) {
    return false;
  }
  unsigned n = chainNum(ptr_to_index);
  bucket *bp = chain[n];
  bucket *nbp = new bucket;
  chain[n] = nbp;
  nbp->next = bp;
  nbp->value = value;
  nbp->index = (void *) new char[(*size)(ptr_to_index)];
  (*copy)(nbp->index, ptr_to_index);

#if 0
  char xx[20];
  memset(xx, '\0', 20);
  *(int *)xx = 2;
  if ((*equal)(xx, ptr_to_index)) {
        cout << "add: bucket {" << (void *)nbp << "} has *" << nbp->index << " == " << *(int *)(nbp->index) << endl;
    if (bp != 0) {
        cout << "     and next {" << (void *)bp << "} has *" << bp->index << " == " << *(int *)(bp->index) << endl;
      }
  }
#endif
  
  return true;
}

//============================================================================
// The index value is copied, but the value is captured.
// In case the index entry is not already present, does nothing and
// returns false.
// It is up to the caller to manage storage of the value so that it sticks
// around as long as the dictionary is used.  It is up to the caller
// to free the previous value, if any.
//============================================================================
boolean idfDictionary::reassign(void *ptr_to_index, void *value)
{
  bucket *bp = bucketOf(ptr_to_index);
  if (bp == NULL) {
    return false;
  }
  bp->value = value;
  return true;
}

//============================================================================
idfDictionary::idfDictionary(
			       boolean  (*e)(void *, void *),
			       unsigned (*h)(void *),
			       int      (*s)(void *),
			       void     (*c)(void *to, void *from)
			      )
  : equal(e), hash(h), size(s), copy(c)
{
  for (int i = 0; i < tableSize; i += 1) {
    chain[i] = NULL;
  }
}

//============================================================================
idfDictionary::~idfDictionary()
{
  for (int i = 0; i < tableSize; i += 1) {
    bucket *to_delete = chain[i];
    while (to_delete != NULL) {
      bucket *delete_now = to_delete;
      to_delete = delete_now->next;
      delete [] ((char *)delete_now->index);
      delete delete_now;
    }
  }
}

//============================================================================
unsigned idfDictionary::chainNum(void *index)
{
  return (*hash)(index) % tableSize;
}

//============================================================================
bucket *idfDictionary::bucketOf(void *ptr_to_index)
{
  bucket *bp = chain[chainNum(ptr_to_index)];
#if 0
  cout << "on chain " << chainNum(ptr_to_index) << endl;
#endif
  while (bp != NULL) {
#if 0
    cout << " got bucket {" << (void *)bp << "} with index *" << bp->index << " == " << *(int *)(bp->index) << endl;
#endif
    if ((*equal)(bp->index, ptr_to_index)) {
      return bp;
    }
    bp = bp->next;
  }
#if 0
  cout << " bucketOf returning NULL " << endl;
#endif
  return NULL;
}

//============================================================================
void *idfDictionary::lookup(void *ptr_to_index)
{
  bucket *bp = bucketOf(ptr_to_index);
  return bp != NULL ? bp->value : NULL;
}

//============================================================================
boolean idfDictionary::contains(void *ptr_to_index)
{
  return bucketOf(ptr_to_index) != NULL;
}

//============================================================================
// return the associated value of the entry that iter refers to
//============================================================================
void *idfDictionary::lookup(const idfIterator &iter)
{
  return lookup(index(iter));
}

//============================================================================
// return the index value of the entry that iter refers to.
// Note that this may return a null value if iter is not referring.
//============================================================================
void *idfDictionary::index(const idfIterator &iter)
{
  return iter.current;
}

//============================================================================
// returns the iterator value of the first entry in the dictionary,
// note that the dictionary may be empty
//============================================================================
idfIterator idfDictionary::first()
{
  return firstBucketAfter(0);
}

//============================================================================
// Sets iter to the dictionary entry after the current;
// if there is no subsequent entry, return false and set iter
// to indicate no current entry
//============================================================================
boolean idfDictionary::next(idfIterator &iter)
{
#if 0
  cout << "in next of " << iter.current << ", calling bucketOf..." << endl;
#endif
  bucket *bp = bucketOf(iter.current);
#if 0
  cout << "next got bucket {" << (void *)bp << "}" << endl;
#endif
  int chain_num = tableSize;
  if (bp != NULL) {
    if (bp->next != NULL) {
      iter.current = bp->next->index;
#if 0
      cout << "next: bucket {" << (void *)bp << "} has *" << bp->index << " == " << *(int *)(bp->index)
           << " and next {" << (void *)bp->next << "} has *" << bp->next->index << " == " << *(int *)(bp->next->index) << endl;
      cout << "next returning with iter of " << iter.current << endl;
#elif 0
      if ((*equal)(bp->index, bp->next->index)) {
        cout << "next: bucket {" << (void *)bp << "} has *" << bp->index << " == " << *(int *)(bp->index)
             << " and next {" << (void *)bp->next << "} has *" << bp->next->index << " == " << *(int *)(bp->next->index) << endl;
        cout << "next returning with iter of " << iter.current << endl;
      }
#endif
      return true;
    }
    chain_num = chainNum(bp->index) + 1;
#if 0
    cout << " moved on to chain " << chain_num << endl;
#endif
  }
  iter = firstBucketAfter(chain_num);
#if 0
  cout << "next returning " << iter.current << endl;
#endif
  return !done(iter);
}

//============================================================================
// whether the iterator is a flag indicating done
//============================================================================
boolean idfDictionary::done(const idfIterator &iter)
{
  return index(iter) == NULL;
}

//============================================================================
// Assist in sequencing through the entire hash table by finding the next
// bucket, beginning the search with the indicated chain. Useful for finding
// the first bucket, supporting the 'next' function, and even for just
// returning a null iterator.
//============================================================================
idfIterator idfDictionary::firstBucketAfter(int chain_num)
{
  void *found_index = NULL;
  while (chain_num < tableSize) {
#if 0
    cout << "on chain " << chain_num << endl;
#endif
    bucket *bp = chain[chain_num];
    if (bp != NULL) {
      found_index = bp->index;
      break;
    }
    chain_num += 1;
  }
#if 0
  cout << "firstBucketAfter returning " << found_index << endl;
#endif
  idfIterator result;
  result.current = found_index;
  return result;
}
