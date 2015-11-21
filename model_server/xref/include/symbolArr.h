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
#ifndef _symbolArr_h
#define _symbolArr_h


#ifndef _symbolPtr_h
#include <symbolPtr.h>
#endif
#ifndef _genArr_h
#include <genArr.h>
#endif

genArr(symbolPtr);

class objArr;
class symbolSet;

class symbolArr
{
  public:
    symbolArr();
    symbolArr(int sz);
    void provide(int sz) {_arr.provide(sz);}
    symbolArr(const symbolArr&);
    symbolArr(const objArr&);
    const symbolArr& operator=(const symbolArr&);
  symbolArr & operator=(symbolSet &);
    int insert_symbols(const objArr &oa);  // return count
    int symbols() const;
    unsigned size() const;
    int empty() const;

    void insert_last(const symbolPtr&);
    void insert_last(const symbolArr&);
    void insert_first(const symbolPtr&);
    void removeAll();
    void remove(const symbolPtr&);
    void remove_cross_xref(const symbolPtr&);
    void remove_index(unsigned int);

    symbolPtr& operator [](int) const;

    int includes(const symbolPtr&) const;
    int includes_cross_xref(const symbolPtr&) const;

    void print(ostream& = cout, int level = 0) const;
    void sort ();
    void usort ();
    void sort_mp (int);
    void usort_mp (int);
    void remove_dup_syms();
    void unsort() { remove_dup_syms();}       // temporary alias, to make optimizing easier to type

    ~symbolArr();
  private:
    genArrOf(symbolPtr) _arr;
    int n_symbols;
};

//------------------------------------------

inline            symbolArr::symbolArr() : _arr(), n_symbols(0) {}
inline            symbolArr::symbolArr(int sz) : _arr(sz), n_symbols(0) {}

inline int        symbolArr::symbols() const { return n_symbols; }
inline unsigned   symbolArr::size() const { return _arr.size(); }
inline int        symbolArr::empty() const { return _arr.size() == 0; }

inline symbolPtr& symbolArr::operator [](int i) const { return *_arr[i]; }

//------------------------------------------

#define ForEachS(el,arr)						\
for(int paste(ii,el) = 0, paste(sz,el) = (arr).size();			\
    (paste(ii,el) < paste(sz,el) && (int)(el = (arr)[paste(ii,el)]));	\
    ++paste(ii,el))

//------------------------------------------

//void arrcpy(symbolArr& to, objArr& from);
//void arrcpy(objArr& to, symbolArr& from);

// objArr& operator=(objArr& oa, const symbolArr& sa);

//------------------------------------------

/*
   START-LOG-------------------------------------------
   $Log: symbolArr.h  $
   Revision 1.6 1997/01/02 09:41:38EST mg 
   Reviewed by: mg
 * Revision 1.2.1.7  1994/03/30  22:04:29  trung
 * Bug track: 0
 * submit optimizing for change prop and some misc
 *
 * Revision 1.2.1.6  1994/02/18  20:28:03  trung
 * Bug track: 0
 * fixes group, error handling in prop
 *
 * Revision 1.2.1.5  1994/01/03  19:27:59  davea
 * Bug track: 5789
 * New symbolSet class.  Like symbolArr, but does not
 * have duplicates.
 *
 * Revision 1.2.1.4  1993/06/14  21:43:54  mg
 * insert_symbols
 *
 * Revision 1.2.1.3  1993/05/24  01:00:21  wmm
 * Fix bugs 3244, 3243, 3252, and 3152.
 *
 * Revision 1.2.1.2  1993/03/29  22:40:50  aharlap
 * added sort, usort
 *
 * Revision 1.2.1.1  1993/02/01  16:55:52  glenn
 * Remove #include objArr.h
 * Remove typedef sortfunctionPtr.
 * Add insert_last for symbolArr&.
 * Add remove_index.
 *
 * Revision 1.2.1.0  1992/12/29  19:11:14  glenn
 * Set up branch.
 *
 * Revision 1.3  1992/12/29  18:33:54  glenn
 * Edit file to conform to coding standards.
 *
   END-LOG---------------------------------------------
*/

#endif
