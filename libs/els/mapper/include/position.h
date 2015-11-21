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

#ifndef _position_h
#define _position_h

#include <iosfwd>
#include <vector>
//++ELS
class  mappedFile;
struct location;
struct mapline;
//--ELS

class position
{
public:
  static void *operator new(size_t);
  static void  operator delete(void *);

public:
  // Public Enum
  enum extractflag { START, END };

  // Constructor(s) / Destructor(s)
  position(void);
  position(unsigned int, mappedFile *);
  position(unsigned int, unsigned int, mappedFile *);
  position(const location *, extractflag);
  position(const mapline *, extractflag, const char *);
  virtual ~position(void);

  // Copy Constructor
  position(const position &);

  // Public Interface
  virtual position *next(unsigned int increment = 1) const;
  virtual position *prev(unsigned int decrement = 1) const;

  virtual int               valid() const;
  virtual int               exists() const;
  virtual void              invalidate();
  virtual void              dumpTo(ostream &) const;
  virtual int               toLocation(location *, extractflag) const;
  virtual position         *offsetWith(const position *, const position *) const;
  inline  const mappedFile *fromFile(void) const;
  

  // Position Comparasion Operators
  inline int operator != (const position &) const;
  inline int operator <  (const position &) const;
  inline int operator <= (const position &) const;
  inline int operator == (const position &) const;
  inline int operator >= (const position &) const;
  inline int operator >  (const position &) const;

private:

  // remove assignment operator from public scope
  inline position &operator = (const position &);

  // Private Enum
  enum datatype { BP, RC };

  // Private Statics
  static datatype initType(const location *, extractflag);
  static vector<position *> freePositions;
  static const int blockSize;

  // Private Methods
  inline int compareBP(const position &) const;
  inline int compareRC(const position &) const;
  inline int compare(const position &) const;

  int computeBP(unsigned int &) const;
  int computeRC(unsigned int &, unsigned int &) const;
  int computeBP(unsigned int, unsigned int, unsigned int &) const;
  int computeRC(unsigned int, unsigned int &, unsigned int &) const;

  int initWithLocation(const location *, extractflag);

  // Private Members
  unsigned int    bp;
  unsigned int    row;
  unsigned int    col;
  const datatype  type;
  mappedFile     *file;
};

const mappedFile *position::fromFile(void) const
{
  return file;
}

position &position::operator = (const position &other)
{
  if ( this != &other )
    {
      bp  = other.bp;
      row = other.row;
      col = other.col;
      file= other.file;
    }      
  return *this;
}
  

int position::compareBP(const position &other) const
{
  int retval = -2; // -2 says can't tell (so don't ask... ;-) )

  if ( bp < other.bp )
    retval = -1;
  else if ( bp > other.bp )
    retval = 1;
  else
    retval = 0;

  return retval;
}

int position::compareRC(const position &other) const
{
  int retval = -2; // Can't Tell

  if ( row < other.row )
    retval = -1;
  else if ( row > other.row )
    retval = 1;
  else if ( col < other.col )
    retval = -1;
  else if ( col > other.col )
    retval = 1;
  else
    retval = 0;

  return retval;
}

int position::compare(const position &other) const
{
  int retval = -2; // Once again, can't tell

  // can only compare from same file
  if ( file == other.file )
    {
      if ( type == BP && other.type == BP )
	retval = compareBP(other);
      else if ( type == RC && other.type == RC )
	retval = compareRC(other);
      else
	{
	  if ( type == BP )
	    {
	      unsigned int otherBP;
	      other.computeBP(otherBP);
	      position p(otherBP, file);
	      retval = compareBP(p);
	    }
	  else
	    {
	      unsigned int thisBP;
	      computeBP(thisBP);
	      position p(thisBP, file);
	      retval = p.compareBP(other);
	    }
	}
    }
      
  return retval;
}

int position::operator < ( const position &other ) const
{
  int retval = 0;

  if (compare(other) == -1 )
    retval = 1;
  
  return retval;
}

int position::operator > ( const position &other ) const
{
  int retval = 0;

  if (compare(other) == 1 )
    retval = 1;
  
  return retval;
}

int position::operator == ( const position &other ) const
{
  int retval = 0;

  if (compare(other) == 0 )
    retval = 1;
  
  return retval;
}

int position::operator != ( const position &other ) const
{
  int retval = 0;

  int cmp = compare(other);
  if ( cmp == 1 || cmp == -1 )
    retval = 1;
  
  return retval;
}

int position::operator <= ( const position &other ) const
{
  int retval = 0;

  int cmp = compare(other);
  if ( cmp == -1 || cmp == 0 )
    retval = 1;
  
  return retval;
}

int position::operator >= ( const position &other ) const
{
  int retval = 0;

  int cmp = compare(other);
  if ( cmp == 0 || cmp == 1 )
    retval = 1;
  
  return retval;
}
#endif 
// _position_h


