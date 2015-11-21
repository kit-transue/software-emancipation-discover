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
/*  symbolArr.h.C -
    implement the class  symbolArr
Classes: symbolArr is a dynamic array of symbolPtrs

Other extern functions:
    extern "C" void psarr(symbolArr* sa)
    void arrcpy (symbolArr &to, objArr &from)
    void arrcpy (objArr &to, symbolArr &from)
    extern "C" void test_sort_symbolArr (symbolArr* a)
    extern "C" void test_usort_symbolArr (symbolArr* a)
    
    */
#include <cLibraryFunctions.h>
#include <msg.h>
#include <symbolArr.h>
#include <symbolSet.h>
#include <xref.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <path.h>
#include <machdep.h>
#include <time.h>


symbolArr::symbolArr (const symbolArr &x)
    : _arr(x.size()), n_symbols(x.n_symbols)
{
//    Initialize(symbolArr::symbolArr);

    const int sz = x._arr.size();
    for (int ii = 0; ii < sz; ++ii)
	_arr.append(x._arr[ii]);
}

void symbolArr::insert_first (const symbolPtr & x)
{
//    Initialize(symbolArr::insert_first);
    symbolPtr y(x) ;
    _arr.grow (1);
    for (int ii = _arr.size() - 1; ii > 0; --ii)
	*_arr[ii] = *_arr[ii - 1];
    *_arr[0] = y;
    if (x.is_xrefSymbol())
	++n_symbols;
}

symbolArr::symbolArr (const objArr &so)
    : _arr(so.size()), n_symbols(0)
{
//    Initialize(symbolArr::symbolArr);

    const int sz = so.size();
    for(int ii = 0; ii < sz; ++ii)
          insert_last(so[ii]);
}

const symbolArr& symbolArr::operator=(const symbolArr& src) {
//   Initialize(symbolArr::operator=);
   if (&src != this) {
       const int sz = src.size();
       _arr.provide(sz);
       removeAll();
       for (size_t i = 0; i < sz; i++) {
	   _arr.append(src._arr[i]);
       }
       n_symbols = src.n_symbols;
   }
   return *this;
}

symbolArr& symbolArr::operator=(symbolSet & ss)
{
//  Initialize(symbolArr::operator=);
  removeAll();
  symbolPtr sym;
  ForEachT(sym, ss) {
    insert_last(sym);
  }
  return *this;
}

int symbolArr::insert_symbols(const objArr &oa)
{
   Obj *el;
   int count = 0;
   ForEach(el, oa){
      symbolPtr sym = el;
      sym = sym.get_xrefSymbol();
      if(sym.isnotnull()){
        insert_last(sym);
         ++count;
      }
   }
   return count;
}
int symbolArr::includes (const symbolPtr & x) const
{
    Initialize(symbolArr::includes);

    const int sz = size();
    for (int ii = 0; ii < sz; ++ii)
	if (x == *_arr[ii])
	    return 1;
    return 0;
}

void symbolArr::remove_dup_syms()
// Remove any duplicate symbols from this symbolArr, assuming these
//   symbols are xrefSymbols, and defining duplicates as having
//   identical kind, name, and defining file
{
//    Initialize(symbolArr::remove_dup_syms);

    symbolSet syms(0, size());

    time_t st_secs;
    time_t tm = time(&st_secs);
    //cout << "start time: " << ctime(&secs);

    syms = *this;

    time_t end_secs;
    tm = time(&end_secs);

    int max_seconds = 120;
    char const *env_val = OSapi_getenv("UNIQUE_TIME");
    if (env_val)
      max_seconds = OSapi_atoi(env_val);
    if ((end_secs - st_secs) >= max_seconds) {
      msg("Original number of symbols: $1\n") << size() << eom; 
      msg("Number of unique symbols: $1\n") << syms.size() << eom;
      msg("Capacity of symbolSet: $1\n") << syms.capacity() << eom;
      msg("Time in seconds: $1\n") << end_secs - st_secs << eom;
      int pr_len = 5;
      char const *val = OSapi_getenv("PRINT_SYMS");
      if (val)
	pr_len = OSapi_atoi(val);
      syms.biggest_chain(pr_len);
    }

    *this = syms;
}

void symbolArr::insert_last (const symbolPtr & x) 
{
//    Initialize(symbolArr::insert_last);

    _arr.append ((symbolPtr*)&x); 
    if (x.is_xrefSymbol())
	++n_symbols;
} 

void symbolArr::insert_last (const symbolArr& x)
{
//    Initialize(symbolArr::insert_last);

    const int sz = x._arr.size();
    for (int ii = 0; ii < sz; ++ii)
	_arr.append(x._arr[ii]);

    n_symbols += x.n_symbols;
} 

void symbolArr::remove (const symbolPtr & x) 
{
//    Initialize(symbolArr::remove);

    const int sz = size();
    for (int ii = 0; ii < sz; ++ii)
	if (x == *_arr[ii]) {
	    if (x.is_xrefSymbol())
		--n_symbols;
	    _arr[ii]->symbolPtr::~symbolPtr();
	    _arr.shrink (ii, 1);
	    return;
	}
}    

void symbolArr::remove_index(unsigned int index)
{
//    Initialize(symbolArr::remove_index);

    if(index < size())
    {
	if (_arr[index]->is_xrefSymbol())
	    --n_symbols;
	_arr[index]->symbolPtr::~symbolPtr();
	_arr.shrink (index, 1);
    }
}

void symbolArr::print (ostream& os, int level) const
{
    Initialize(symbolArr::print);
 
    gen_print_indent (os, level);
    os << '<' << "symbolArr";
    os << ' ' << (void *)this << ' ';
    os << '>' << "contains " << size() << " elements" << endl;
    symbolPtr el;
    int i = 0;
    ForEachS (el, *this) {
	gen_print_indent (os, level + 1);
	os << '(' << i << ')';
	i++;
	if(el.is_xrefSymbol()){
            symbolArr defined;
            el->get_link(is_defined_in, defined);
            char const *defined_name = "<NO DEFINITION>";
            if(defined.size() != 0)
                defined_name = defined[0].get_name();
	    // Cast to int to satisfy SGI compiler
	    // [03/17/97, abover]:
            os << " " << (int)el.get_xrefSymbol() << " " << ddKind_name(el->get_kind()) << " " << el.get_name() << " " 
		<< defined_name << endl;
        } else {
	  el.print (os, level + 1);
	}
    }
    os << endl;
}

extern "C" void psarrel(symbolArr* sa, int ind)
{
    node_prtS((*sa)[ind]);
}

extern "C" void psarr(symbolArr* sa)
{
    sa->print();
}

static int sym_compare_mp (const void* p1, const void* p2)
{
//    Initialize (sym_compare);
    return  ((symbolPtr*)p1)->sym_compare_mp( *(symbolPtr*)p2, 1);
}

static int sym_compare (const void* p1, const void* p2)
{
//    Initialize (sym_compare);
    return ((symbolPtr*)p1)->sym_compare( *(symbolPtr*)p2);
}

int symbolArr::includes_cross_xref (const symbolPtr & x) const
{
//  Initialize(symbolArr::includes);

  const int sz = size();
  for (int ii = 0; ii < sz; ++ii) {
    int val = sym_compare(&x,_arr[ii]);
    if (val != -2 && val != 2)
      return 1;
  }
  return 0;
}

void symbolArr::remove_cross_xref (const symbolPtr & x) 
{
//    Initialize(symbolArr::remove);

    const int sz = size();
    for (int ii = 0; ii < sz; ++ii) {
      int val = sym_compare(&x,_arr[ii]);
      if (val != -2 && val != 2) {
	if (x.is_xrefSymbol())
	  --n_symbols;
	_arr[ii]->symbolPtr::~symbolPtr();
	_arr.shrink (ii, 1);
	return;
      }
    }
}    

void symbolArr::sort ()
{
//    Initialize (symbolArr::sort);
    sort_mp(0);
} 

void symbolArr::sort_mp (int mp)
{
//    Initialize (symbolArr::sort_mp);

    int nel = size ();

    if (nel <= 1)
	return;

    char* base = (char*)&operator [] (0);
    int width = sizeof (symbolPtr);

    if (mp)
	qsort (base, nel, width, sym_compare_mp);
    else 
	qsort (base, nel, width, sym_compare);
} 

void symbolArr::usort ()
//  sort symbols by kind, name, and defining file
{
//    Initialize (symbolArr::usort);
    usort_mp (0);
} 

void symbolArr::usort_mp (int mp)
//  sort symbols by kind, name, and defining file
{
//    Initialize (symbolArr::usort_mp);

    int sz = size ();
    
    int (*loc_sym_compare) (const void*, const void*);

    if (mp)
	loc_sym_compare = sym_compare_mp;
    else
	loc_sym_compare = sym_compare;

    if (sz <= 1)
	return;

    sort_mp (mp);

    symbolPtr* p0 = &operator [] (0);
    symbolPtr* p1 = &operator [] (1);

    int ind0 = 0;
    int ind1 = 1;
    projNode* cur_pr = projNode::get_current_proj();
    Xref* cur_xr = cur_pr->get_xref();
    int curr_is_local = 0;
    if (!mp)
	curr_is_local = ((*p0).get_xref() == cur_xr);
    else {
	genString fn;
	genString ln = (*p0).get_name();
	cur_pr->ln_to_fn(ln, fn, FILE_TYPE, 0, 1);
	curr_is_local = (fn.length() != 0);
    }
    for (; ind1 < sz; ++ind1, ++p1)
	if (loc_sym_compare (p0, p1)) {
	  ++ind0;
	  ++p0;
	  if (!mp)
	      curr_is_local = ((*p0).get_xref() == cur_xr);
	  else {
	      genString fn;
	      genString ln = (*p0).get_name();
	      cur_pr->ln_to_fn(ln, fn, FILE_TYPE, 0, 1);
	      curr_is_local = (fn.length() != 0);
	  }
	  if (ind0 < ind1)
	    *p0 = *p1;
	} else if (!curr_is_local) {
	    int next_is_local = 0;
	    if (!mp)
		next_is_local = ((*p1).get_xref() == cur_xr);
	    else {
	      genString fn;
	      genString ln = (*p1).get_name();
	      cur_pr->ln_to_fn(ln, fn, FILE_TYPE, 0, 1);
	      next_is_local = (fn.length() != 0);
	    }
	    if (next_is_local) {
		*p0 = *p1;
		curr_is_local = 1;
	    }
	}

    int new_size  = ind0 + 1;
    if (new_size < sz) {
	for (int ii = new_size; ii < sz; ++ii)
	    _arr[ii]->symbolPtr::~symbolPtr(); 
	_arr.grow (new_size - sz);  // shrink 
    }
} 

extern "C" void test_sort_symbolArr (symbolArr* a)
{
    msg("======================= Start sort ===============\n") << eom;
    a->print ();
    a->sort ();
    msg("======================= End sort =================\n") << eom;
    a->print ();
}

extern "C" void test_usort_symbolArr (symbolArr* a)
{
    msg("======================= Start usort ===============\n") << eom;
    a->print ();
    a->usort ();
    msg("======================= End usort =================\n") << eom;
    a->print ();
}

symbolArr::~symbolArr()
{
//    Initialize(symbolArr::~symbolArr);

    removeAll();
}

void symbolArr::removeAll() 
{ 
//    Initialize(symbolArr::removeAll());
    
    const int sz = _arr.size();
    for(int ii = 0; ii < sz; ++ii)
	_arr[ii]->symbolPtr::~symbolPtr();

    _arr.reset(); 
    n_symbols = 0; 
}

