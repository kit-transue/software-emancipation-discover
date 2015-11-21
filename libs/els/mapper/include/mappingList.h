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
// This code is a -*- C++ -*- header file

#ifndef _mappingList_h
#define _mappingList_h

#include <iosfwd>

//++ELS
class region;
class mapping;
class position;
//--ELS

//++ADS
#ifndef _parray_h
#include <parray.h>
#endif
//--ADS

class mappingList : private parray
{
public:

  // Constructor(s) / Destructor(s)
  mappingList(int);
  virtual ~mappingList(void);

  // Public Interface
  int                  insert(mapping *);
  int                  extract(region *, region **);
  const position      *getFileBoundary(char *);
  inline int           length(void);
  virtual void         dumpTo(ostream &);

  // Public Operators
  inline mapping      *operator[](int); 

protected:

  // Protected Data Structures
  enum errorReason 
    { 
      NO_REPLACEMENT,
      NO_CONTAINS,
      NO_IS_CONTAINED_IN,
      NO_OVERLAPS_OUT_OF,
      NO_OVERLAPS_INTO,
      NO_FILE_INCONSISTANT_REPORTING,
      INVALID_COMPARASON_ATTEMPTED
    };

  // Protected Interface
  void mappingError(const mapping *, const mapping *, errorReason);
};

int mappingList::length(void)
{
  return parray::size();
}

mapping *mappingList::operator[](int x)
{
  mapping *retval = (mapping *) parray::operator[](x);
  return retval;
}

#endif 
// _mappingList_h

