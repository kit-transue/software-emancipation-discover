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
// This is a -*- C++ -*- header file

#ifndef _region_h
#define _region_h

#include <iosfwd>

//++ELS
class  mappedFile;
class  position;
struct location;
struct mapline;
//--ELS

class region
{
  //
  // Public Types
  //

public:
  enum regionOrder 
    {
      PRECEDES,
      FOLLOWS,
      CONTAINS,
      IS_CONTAINED_IN,
      IS_EXACTLY,
      OVERLAPS_INTO,
      OVERLAPS_OUT_OF,
      UNKNOWN,
      INVALID
    };

  //
  // Constructors, Destructors
  //

public:
  region(const location *);
  region(const mapline *, char *);
  region(const position *, const position *);
  virtual ~region(void);

  // Copy constructors / Assignment operators
  region(const region &);
  region &operator = ( const region & );


public:
  // Public Methods
  inline const mappedFile *fromFile(void) const;
  inline const position   *getStart(void) const;
  inline const position   *getEnd(void) const;

  int                invalidate(void);
  int                split(const region *, region **, region **) const;
  int                valid(void) const;
  int                offsetWith(const position *, const position *) const;
  regionOrder        orderWith(const region *) const;
  virtual void       dumpTo(ostream &) const;


  // Public casts
  operator location(void) const;

protected:

  // Protected Members
  position       *start;
  position       *end;
  mappedFile     *from_file;

};


const mappedFile *region::fromFile(void) const
{
  return from_file;
}

const position *region::getStart(void) const
{
  return start;
}

const position *region::getEnd(void) const
{
  return end;
}

#endif
