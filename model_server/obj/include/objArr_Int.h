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
////////////////////////////////   FILE objArr_Int.h  ///////////////////////////
//
// -- Contains objArr_Int Class Definition
//
#ifndef __objArr_Int
#define __objArr_Int

#define ARRINT_CHUNK 256


//------------------   objArr_Int Class   -------------------------------

// Array of integers; dynamically reallocated. When the size increased the
// chunk size = 5 used not to make reallocations too often.


        class objArr_Int 
        {
                unsigned       sz;            // # of integers in os_list
                unsigned       total;         // array capacity (reserved)
                int            *os_list;      // Allocated Array of Integers

             public:
                objArr_Int() {os_list = (int*)NULL; sz = 0; total =0;} 
                virtual ~objArr_Int(){delete os_list;}

                objArr_Int(const objArr_Int& x);
 //             virtual int  collectionp() const  {return 1;}; 
                void insert_first(const int);
                void insert_last(const int);
                void remove(const int);
                virtual void removeAll();
                int includes(const int )const;
                unsigned size() const  {return sz;}
                unsigned get_total() const  {return total;}
                int  empty() const {return (sz ? 0 : 1);}
                void insert_all(const objArr_Int& );

                void operator=( const objArr_Int& );
                int& operator[](int ind ) { return os_list[ind]; }
                int operator==(const objArr_Int& x) const {return (os_list == 
                                                             x.os_list);}
                int operator!=(const objArr_Int& x) const {return (os_list != 
                                                             x.os_list);}
          private:
                void put_size(const unsigned s) { sz = s;}
                void put_total(const unsigned cap) { total = cap;}
        };



#endif

/*
   START-LOG-------------------------------------------

   $Log: objArr_Int.h  $
   Revision 1.1 1993/07/28 19:51:30EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  18:58:11  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
