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
#ifndef _gen_hash_h
#define _gen_hash_h

/*
  Create a simple hash package: (key, value). Lookup by key, and get value back.
  Usage: genHash(keyType, valueType); -> generate keyTypeHash class;
  genHash(KEY,VAL) should only be called in **src** file since it defines some
  regular methods
  currently, hash function is just address modulo a prime number. For more
  interesting hashing (esp. for strings), just inherit and overide the
  hash function. This hash package works as normal, resolving collision
  by chain. When a chain length is greater than some pre-define constant, it will
  rehash the chain into a new table of size approximately twice the current
  size. The table index now point to a new table instead of a chain, and so on.
  Use at your own risk, I believe it is more efficient than conventional hashing,
  but then again, who knows!
*/


enum HASH_SIZE_DATA {LEVEL0 = 191, LEVEL1 = 373, LEVEL2 = 739,
		     MAX_LEVEL = 3, MAX_CHAIN = 7};
enum TB {BUCKET = 0, TABLE};

#define Bucket_of(KEY,VAL) paste3(KEY,VAL,Bucket)
#define Table_or_Bucket_of(KEY,VAL) paste3(KEY,VAL,Table_or_Bucket)
#define genHashOf(KEY,VAL) paste3(KEY,VAL,Hash)
#define genHash(KEY,VAL)  					   	\
class genHashOf(KEY,VAL);						\
class Bucket_of(KEY,VAL) 						\
{ 									\
  KEY * key; 								\
  VAL * val;								\
  Bucket_of(KEY,VAL) *next;						\
public:									\
  Bucket_of(KEY,VAL)(KEY *k, VAL *v) {key = k; val = v; next = 0;}	\
  KEY *get_key() {return key;}						\
  VAL *get_val() {return val;}						\
  void set_val(VAL *v) {val = v;}					\
  VAL *lookup_Bucket(KEY *k);						\
  Bucket_of(KEY,VAL) *lookup_key(KEY *k);				\
  int chain_length();							\
  Bucket_of(KEY,VAL) *insert(KEY *k, VAL *v)				\
  {									\
    Bucket_of(KEY,VAL) *b = lookup_key(k);				\
    if (b)								\
      b->set_val(v);							\
    else {								\
      b = new Bucket_of(KEY,VAL)(k,v);					\
      if (b)								\
        b = insert(b);							\
    }									\
    return b;								\
  }									\
  Bucket_of(KEY,VAL) *insert(Bucket_of(KEY,VAL) *b)			\
  {									\
									\
    if (b) {								\
      b->set_next(get_next());						\
      set_next(b);							\
    }									\
    return b;								\
  }									\
  void set_next(Bucket_of(KEY,VAL) *b) {next = b;}			\
  Bucket_of(KEY,VAL) *get_next() {return next;}				\
};									\
Bucket_of(KEY,VAL) * Bucket_of(KEY,VAL)::lookup_key(KEY *k)		\
{									\
  Bucket_of(KEY,VAL) *tmp = this;					\
  while (tmp) {								\
    if (tmp->get_key() == k)						\
      return tmp;							\
    tmp = tmp->get_next();						\
  }									\
  return NULL;								\
}									\
VAL *Bucket_of(KEY,VAL)::lookup_Bucket(KEY *k)				\
{									\
  Bucket_of(KEY,VAL) *b = lookup_key(k);				\
  return (b) ? b->get_val() : NULL;					\
}									\
int Bucket_of(KEY,VAL)::chain_length()					\
{									\
  int ret_val = 0;							\
  Bucket_of(KEY,VAL) *tmp = this;					\
  while (tmp) {								\
    ret_val ++;								\
    tmp = tmp->get_next();						\
  }									\
  return ret_val;							\
}									\
class Table_or_Bucket_of(KEY,VAL)					\
{									\
  TB type;								\
  union {								\
    Bucket_of(KEY,VAL) *bucket;						\
    genHashOf(KEY,VAL) *next_level;					\
  };									\
public:									\
  Table_or_Bucket_of(KEY,VAL) () {}					\
  Table_or_Bucket_of(KEY,VAL) (TB wh)					\
  {									\
    type = wh;								\
    if (wh == BUCKET) bucket = NULL;					\
    else next_level = NULL;						\
  }									\
  void set_type(TB t) {type = t;}					\
  Bucket_of(KEY,VAL) *make_bucket(KEY *k, VAL *v)			\
  { bucket = new Bucket_of(KEY,VAL)(k,v); return bucket; } 		\
  Bucket_of(KEY,VAL) *get_bucket() {return bucket;}			\
  void set_bucket(Bucket_of(KEY,VAL) *b) {bucket = b;}			\
  void set_table(genHashOf(KEY,VAL) *t) {next_level = t;}  		\
  VAL *remove_bucket(KEY *k);						\
  genHashOf(KEY,VAL) *get_table() { return next_level;}			\
  int isA(TB tb) {return tb == type;}					\
  void free_buckets();  						\
  ~Table_or_Bucket_of(KEY,VAL) () {					\
    if (type == BUCKET) free_buckets();					\
  }									\
};									\
void Table_or_Bucket_of(KEY,VAL)::free_buckets()			\
{									\
  Bucket_of(KEY,VAL) *bl = get_bucket();				\
  if (bl) {								\
    Bucket_of(KEY,VAL) *next;						\
    while (bl) {							\
      next = bl->get_next();						\
      delete (bl);							\
      bl = next;							\
    }									\
  }									\
}									\
VAL *Table_or_Bucket_of(KEY,VAL)::remove_bucket(KEY *k)			\
{	/* caller is responsible for deleting *this */			\
  VAL *ret_val = 0;							\
  Bucket_of(KEY,VAL) *tmp = get_bucket();				\
  if (tmp->get_key() == k) { /* first bucket case */			\
    set_bucket(tmp->get_next());					\
    ret_val = tmp->get_val();						\
    tmp->set_next(0);							\
    delete tmp;								\
  }									\
  Bucket_of(KEY,VAL) *prev = tmp;					\
  tmp = tmp->get_next();						\
  while (!ret_val && tmp) {						\
    if (tmp->get_key() == k) {						\
      prev->set_next(tmp->get_next());					\
      ret_val = tmp->get_val();						\
      tmp->set_next(0);      						\
      delete tmp;							\
    }									\
    prev = tmp;								\
    tmp = tmp->get_next();						\
  }									\
  return ret_val;							\
}									\
class genHashOf(KEY,VAL) 					   	\
{  									\
  unsigned int level;							\
  unsigned int size;							\
  Table_or_Bucket_of(KEY,VAL)  **data;					\
  int insert_buckets(Bucket_of(KEY,VAL) *bl);				\
  Table_or_Bucket_of(KEY,VAL)  *create_table_or_bucket(KEY *k, VAL *v)	\
  {									\
    Table_or_Bucket_of(KEY,VAL)  * tb = new Table_or_Bucket_of(KEY,VAL) (BUCKET); \
    tb->make_bucket(k,v);						\
    return tb;								\
  }									\
  genHashOf(KEY,VAL) * convert_to_table(int, int );			\
  Table_or_Bucket_of(KEY,VAL)  *create_table_or_bucket(Bucket_of(KEY,VAL) *b) \
  {									\
    Table_or_Bucket_of(KEY,VAL)  * tb = new Table_or_Bucket_of(KEY,VAL) (BUCKET); \
    tb->set_bucket(b);							\
    return tb;								\
  }									\
  unsigned int get_size_of_level(int lev)				\
  {									\
    unsigned int ret_val;						\
    if (lev == 0) ret_val = LEVEL0;					\
    else if (lev == 1) ret_val = LEVEL1;				\
    else if (lev == 2) ret_val = LEVEL2;				\
    else ret_val = LEVEL2;						\
    return ret_val;							\
  }									\
  int get_level() { return level;}					\
  unsigned int hash(KEY *k) { return ((unsigned int)k) % size;}		\
  genHashOf(KEY,VAL) *make_table(Table_or_Bucket_of(KEY,VAL)  *tb, int lev) \
  {									\
    genHashOf(KEY,VAL) *nt = new genHashOf(KEY,VAL)(lev);		\
    tb->set_table(nt);							\
    return nt;								\
  }									\
  int insert_one_bucket(Bucket_of(KEY,VAL) *);				\
public:  								\
  genHashOf(KEY,VAL)(int);						\
  int insert(KEY *, VAL *);						\
  VAL *lookup(KEY *k);							\
  VAL *remove(KEY *k);							\
  int update(KEY *, VAL *);						\
~genHashOf(KEY,VAL)();							\
};									\
genHashOf(KEY,VAL)::~genHashOf(KEY,VAL)()				\
{									\
  for (int i = 0; i < size; ++i) {					\
    Table_or_Bucket_of(KEY,VAL)  *tb = data[i];				\
    if (tb == 0) continue;						\
    if (tb->isA(BUCKET))						\
      delete tb;							\
    else if (tb->get_table()) {						\
      delete tb->get_table();						\
      delete tb;							\
    }									\
  }									\
  delete [] data;						     	\
}									\
int genHashOf(KEY,VAL)::insert_one_bucket(Bucket_of(KEY,VAL) *b)	\
  /* this is a simple table, called during rehash one chain */		\
{								      	\
  int ret_val = 0;							\
  unsigned int hv = hash(b->get_key());					\
  if (data[hv] == NULL) {						\
    data[hv] = create_table_or_bucket(b);				\
    ret_val = 1;							\
  }									\
  else { /* has to be a simple table */					\
    Bucket_of(KEY,VAL) *bl = data[hv]->get_bucket();			\
    bl->insert(b);							\
    ret_val = 1;							\
  }									\
  return ret_val;							\
}									\
int genHashOf(KEY,VAL)::insert_buckets(Bucket_of(KEY,VAL) *bl)		\
{									\
  int ret_val = 0;							\
  if (!bl) return ret_val;						\
  while (bl) {								\
    ret_val ++;								\
    Bucket_of(KEY,VAL) *next = bl->get_next();				\
    bl->set_next(0);							\
    insert_one_bucket(bl);					      	\
    bl = next;								\
  }									\
  return ret_val;							\
}									\
genHashOf(KEY,VAL) * genHashOf(KEY,VAL)::convert_to_table(int index, int lev) \
/* convert chain at index to a table */					\
{									\
  Table_or_Bucket_of(KEY,VAL)  *tb = data[index];			\
  if (!tb) return NULL;							\
  Bucket_of(KEY,VAL) * bl = tb->get_bucket();				\
  tb->set_type(TABLE);							\
  genHashOf(KEY,VAL) *ghk = make_table(tb, lev); /* make an empty table */ \
  if (!ghk) return NULL;						\
  ghk->insert_buckets(bl);						\
  return ghk;								\
}									\
genHashOf(KEY,VAL)::genHashOf(KEY,VAL)(int lev)				\
{									\
  level = lev;								\
  size = get_size_of_level(lev);					\
  data = (Table_or_Bucket_of(KEY,VAL)  **)				\
   new char[sizeof(Table_or_Bucket_of(KEY,VAL)  *)*size];		\
  for (int i = 0; i < size; ++i)					\
    data[i] = NULL;							\
}									\
int genHashOf(KEY,VAL)::insert(KEY *k, VAL *v)				\
{									\
  int ret_val = 0;							\
  unsigned int hv = hash(k);						\
  if (data[hv] == 0) { /* insert a bucket here */			\
    data[hv] = create_table_or_bucket(k,v);				\
    ret_val = 1;							\
  }									\
  else if (data[hv]->isA(TABLE)) { /* should assert table is non-empty */  \
    genHashOf(KEY,VAL) *nl = data[hv]->get_table(); /* insert in next level */ \
    ret_val = nl->insert(k, v);						\
  }									\
  else { /* it has to be a chain of bucket here */		     	\
    Bucket_of(KEY,VAL) *bl = data[hv]->get_bucket();			\
    if ((get_level() < MAX_LEVEL) && (bl->chain_length() > MAX_CHAIN)) { \
	genHashOf(KEY,VAL) * nl = convert_to_table(hv, get_level() + 1); \
	ret_val = (nl && (nl->insert(k,v)));				\
      }									\
      else								\
	ret_val = (bl->insert(k,v) != 0);				\
  }									\
  return ret_val;							\
}									\
VAL *genHashOf(KEY,VAL)::lookup(KEY *k)					\
{									\
  VAL* ret_val = 0;							\
  unsigned int hv = hash(k);						\
  if (data[hv] == 0) { 							\
    ret_val = NULL;							\
  }									\
  else if (data[hv]->isA(TABLE)) { /* should assert table is non-empty */  \
    genHashOf(KEY,VAL) *nl = data[hv]->get_table(); /* insert in next level */ \
    ret_val = nl->lookup(k);						\
  }									\
  else { /* it has to be a chain of bucket here */			\
    Bucket_of(KEY,VAL) *bl = data[hv]->get_bucket();			\
    ret_val = bl->lookup_Bucket(k);					\
  }									\
  return ret_val;							\
}									\
VAL *genHashOf(KEY,VAL)::remove(KEY *k)					\
{									\
  VAL* ret_val = 0;							\
  unsigned int hv = hash(k);						\
  if (data[hv] == 0) { /* insert a bucket here */			\
    ret_val = NULL;							\
  }								      	\
  else if (data[hv]->isA(TABLE)) { /* should assert table is non-empty */ \
    genHashOf(KEY,VAL) *nl = data[hv]->get_table(); /* insert in next level */ \
    ret_val = nl->remove(k);					      	\
  }									\
  else { /* it has to be a chain of bucket here */			\
    ret_val = data[hv]->remove_bucket(k);				\
    if (data[hv]->get_bucket() == 0) {					\
      delete data[hv];							\
      data[hv] = NULL;							\
    }									\
  }									\
  return ret_val;							\
}									\

#endif

