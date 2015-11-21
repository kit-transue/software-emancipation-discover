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
// Propagation.h
//
// Contains propagation related classes.
//

#ifndef __objPropagate
#define __objPropagate

#include "transaction.h"
#include "objArr.h"
#ifdef XXX_TblEl
class TblEl : public Relational, public transactional     // fakes hash table
{
  public:
     define_relational(TblEl,Relational);
     RelationalPtr old_ptr;           // object old_ptr
     RelationalPtr new_ptr;           // pointer to object copy

     public:
       TblEl (RelationalPtr x, RelationalPtr y) { old_ptr = x; new_ptr = y;}
       TblEl (const TblEl& ) {}
       copy_member(TblEl);
};

generate_descriptor(TblEl,Relational);
#endif
// an element of the set created by rel_walk - contains obj_ptr + walk_class

class objRelWalkSet : public Relational, public transactional
{
  public:
     define_relational(objRelWalkSet,Relational);
     objPropagator    *walk_ptr;
     RelationalPtr    targ_obj;

  public:
     objRelWalkSet (objPropagator *ww, RelationalPtr tt) 
                   {walk_ptr = ww; targ_obj = tt; }
     objRelWalkSet (const objRelWalkSet& ) {}
     ~objRelWalkSet();
     copy_member(objRelWalkSet);
     bool isEqual(const Object& x) const;
     unsigned hash() const;
  
     virtual void	print(ostream& = cout, int level = 0) const;
};

generate_descriptor(objRelWalkSet,Relational);
#ifdef XXX_trnsf_type
extern  objOperType trnsf_type(RelationalPtr , RelationalPtr,
                               objOperType);
#endif
#endif

