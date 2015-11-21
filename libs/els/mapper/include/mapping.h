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

#ifndef _mapping_h
#define _mapping_h

#include <iosfwd>

//++ELS
struct location;
#ifndef _region_h
#include <region.h>
#endif
#ifndef _mappingType_h
#include <mappingType.h>
#endif
//--ELS

class mapping : public region
{
public:

  // Constructor(s) / Destructor(s)
  mapping(const location *, const location *, mappingType);
  mapping(const mapline *);
  mapping(const region *, const region *, mappingType);
  virtual ~mapping(void);

  // Copy constructors / Assignment operators
  mapping(const mapping &);
  mapping &operator = ( const mapping & );

  // Public Interface
  inline const region *getSource(void) const;

  int                  replacementAllowed(const mapping *) const;
  int                  split(const region *, mapping **, mapping **, mapping **) const;
  int                  splitAllowed(const mapping *) const;
  position            *translate(const position *) const;
  region              *translate(const region *) const;
  virtual void         dumpTo(ostream &) const;

private:

  // the region that this region is mapped from (i.e. where it came from)
  // this mapping should *own* the pointed to region
  // (i.e. it should be able to deallocate it if necessary
  region      *source;
  mappingType  mapType;
};

const region *mapping::getSource(void) const
{
  return source;
}

#endif 
// _mapping_h

