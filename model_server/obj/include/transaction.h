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
#ifndef _transaction_h
#define _transaction_h

#include "genTry.h"

#include <dis_types.h>


#include "objRelation.h"


class transactional {
public:
    virtual ~transactional() {}
   void * operator new(size_t size);
   void   operator delete(void *){};
};


class objTree;

class objPropagator : public transactional
{
  private:
    AssocType assoc_type;
    RelationalPtr starter;                // who started propagation?
    notification_time when_to_notify : 1; // when to perform notification?

  public:
    RelType *rel_type;  // Used in rel_walk to extract proper links only.

    objPropagator(AssocType a = OTHER_ASSOC,
                  notification_time n = AFTER_ACCEPT,
                  RelationalPtr s = NULL)
        : assoc_type(a), starter(s), when_to_notify(n), rel_type(NULL) {}

    virtual objPropagator* accept(RelationalPtr, int);

    AssocType get_assoc_type() const { return assoc_type; }
    void put_assoc_type(AssocType aa) { assoc_type = aa; }

    void put_rel_type(RelType *rr) { rel_type = rr; }

    notification_time get_notify_flag () const 
 	{ return (notification_time) (((unsigned int)when_to_notify)&1); } //Don't sign extend 1 bit fields
    void put_notify_flag (notification_time n) { when_to_notify = n; }

    RelationalPtr get_starter_obj () const { return starter; }
    void          put_starter_obj (RelationalPtr s) { starter = s; }

    virtual void const *get_data();
    virtual objTree* get_target();
    virtual objOperType get_op_type();
};

class objWalker : public objPropagator
{
  public:
    virtual int apply(RelationalPtr) const = 0;
};

class objModifier : public objPropagator
{
  public:
    virtual int apply(RelationalPtr) = 0;
    virtual int undo(RelationalPtr) { return 1;}
};

class objCopier : public objPropagator
{
  public:
    virtual RelationalPtr apply(RelationalPtr obj)
    { return obj->rel_copy(); }
};

class objDeleter : public objPropagator
{
  public:
    virtual int apply(RelationalPtr);
};
#ifdef XXX_objPrinter
class objPrinter : public objWalker
{
    ostream& strm;

  public:
    objPrinter(ostream& st = cout) : strm(st) {}
    virtual objPropagator* accept(RelationalPtr obj, int level)
    { obj->print(strm, level); return this; }
    virtual int apply(RelationalPtr) const { return 0; }
};
#endif
class Obj;

extern  void obj_dispose(Obj *);
extern  void obj_remove_from_dispose(Obj *root, Obj *ob);

extern void obj_transaction_start();
extern void obj_transaction_end();
extern void obj_transaction_abort();

void rel_walk(RelationalPtr, const int&, objPropagator*,
              objSet&, int level = ALL_LEVELS, int cur_lev = 0);
void obj_walk(Relational&, objWalker*, const int&);
void obj_print(RelationalPtr, const int max_level = ALL_LEVELS);


struct __trans {
  __trans() { obj_transaction_start(); max_buf_reached = max_buf(); }
  ~__trans() { if(max_buf_reached == 0) 
                 pop_buf(); 
               obj_transaction_end();}
  int do_push() { return (max_buf_reached == 0); }
  int max_buf_reached;
};

#define start_transaction()						\
     do {     genError *_ge; __trans _tr;				\
       if((_ge = _tr.do_push() ? (genError *)setjmp(push_buf()) : 0)) {	\
           obj_transaction_abort(); throw_error((int)_ge);		\
       } else

#define end_transaction()     } while(0)

#endif // _transaction_h






















