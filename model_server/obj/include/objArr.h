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
//
// -- Contains objArr Class Definition
//
#ifndef _objArr_h
#define _objArr_h

#include <objDb.h>

#include "objCollection.h"

#include "ArrayOb.h"

extern "C" {
  typedef int (*sortfunctionPtr)(const void *, const void *);
}

//------------------   objArr Class   -------------------------------

// Array of pointers to Objects; dynamically reallocated. Based on NIH ArrayOb.
class symbolArr;

        class objArr : public Obj
        {
             public:
                ArrayOb    os_list;                 // NIH Array of Pointers

             private:
                unsigned int _length;
                int provide(int size);
             public:
                objArr();
                ~objArr();
                objArr(int sz);

                objArr(const objArr &);
		objArr(const objSet &);
                objArr(const symbolArr &);

                virtual int  collectionp() const;
                void insert_first(const Obj*);
	        void insert_after(int, const Obj*);
                void insert_last(const Obj* );
                virtual void remove(const Obj* );
                virtual void removeAll();
                void remove(int );
                Obj * pop();
                int includes(const Obj *)const ; 
                virtual unsigned size() const    {return _length;}
                virtual int  empty() const {return _length == 0 ;}
                void insert_all(const objArr& );  // in fact Obj&
                void insert(const Obj* x){insert_all(*(objArr*)x);}
		void sort(sortfunctionPtr);
                void operator=( const objArr& x);
                Obj*& operator[](int i) const  {return *(Obj**)&os_list[i];}
                int operator==(const objArr& x) const; 
                int operator!=(const objArr& x) const;
 
	       virtual void put_signature(ostream& os) const;
               virtual const char * objname() const;
	      };

#endif

