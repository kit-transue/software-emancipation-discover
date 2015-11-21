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
#ifndef __binsearch
#define __binsearch

// binsearch
//------------------------------------------
// synopsis:
// binary search classes
//
// description:
// This header defines two abstract classes which can be used for binary
// searching.  Class binsearch_item is used as a base class for the items
// to search for; class binsearch_table is used as a base for the
// collection of binsearch items and provides the actual binary search
// method.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

class binsearch_item {
public:
   virtual ~binsearch_item() {}
   virtual int compare(const binsearch_item&) const = 0;
   // returns <0 if this < other, 0 if this == other, >0 if this > other
};

class binsearch_table {
public:
   virtual ~binsearch_table() {}
   virtual size_t item_count() const = 0;
   virtual binsearch_item& item(size_t) const = 0;
   size_t find_item(const binsearch_item&) const;
};

#endif

/*
    START-LOG-------------------------------

    $Log: binsearch.h  $
    Revision 1.1 1993/07/28 19:53:32EDT builder 
    made from unix file
 * Revision 1.2.1.3  1992/10/09  19:16:41  builder
 * *** empty log message ***
 *
 * Revision 1.2.1.2  92/10/09  19:13:05  builder
 * *** empty log message ***
 * 

    END-LOG---------------------------------
*/

