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
#ifndef _INTERVAL_H
#define _INTERVAL_H

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

// choose margin numbers with sufficient distance to the bounds of int range
#define NEGINF 0x8F000000     
#define POSINF 0x70FFFFFF

#define shift_range(s,offset) (s + offset)

class interval {
 public:
  int low;                    // lower bound;
  int high;                   // upper bound;
  int low_fixed;              // if true, no application of function
                              // lower_bound may change low;
  int high_fixed;             // if true, no application of function
                              // upper_bound may change high;

  interval(interval* i)
    { 
      low = i->low; high = i->high; 
      low_fixed = i->low_fixed; high_fixed = i->high_fixed; 
    }

  interval(int l = NEGINF, int h = POSINF)
    { low = l; high = h; low_fixed = 0; high_fixed = 0; }
  
  ~interval() {}

  int intersect(interval* i)
                              // intersect with interval i,
                              // return 0 if interval empty, 1 otherwise;
  {
    if (i->low > low) low = i->low;
    if (i->high < high) high = i->high;
    return low <= high;
  }

  int unite(interval* i)      // unite with interval i,
                              // return 0 if interval empty, 1 otherwise;
  {
    if (i->low < low) low = i->low;
    if (i->high > high) high = i->high;
    return low <= high;
  }

  int includes(int i)         // return 1 if i lies in interval, 0 otherwise;
  { return ((low <= i) && (i <= high)); }

  int shift(int i)            // shift this interval i steps to the right;
  {
    low = shift_range(low,i);
    high = shift_range(high,i);
    return low <= high;
  }

  int upper_bound(int b)      // make upper bound to the minimum of the old
                              // upper bound and b,
                              // return 0 if interval empty, 1 otherwise;
  { 
    if (b < high) {
      if (high_fixed) return 0;
      else { high = b; return low <= high; }
    }
    else return low <= high;
  }
  
  int lower_bound(int b)      // make upper bound to the maximum of the old
                              // lower bound and b,
                              // return 0 if interval empty, 1 otherwise;
  {
    if (b > low) {
      if (low_fixed) return 0;
      else { low = b; return low <= high; }
    }
    else return low <= high;
  }

  int empty() 
                              // return 1 if this interval is empty;
  { return (low > high); }


};

#endif
