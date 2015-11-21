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
//++C
#ifndef ISO_CPP_HEADERS
#include <limits.h>
#else /* ISO_CPP_HEADERS */
#include <limits>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C

//++C++
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
#include <pdumem.h>
#include <pdustring.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <position.h>
#include <locationStruct.h>
#include <maplineStruct.h>
#include <mappedFile.h>
//--ELS

const int  position::blockSize     = 65536;
vector<position *> position::freePositions;

void *position::operator new (size_t)
{
    position *retval = 0;
  


  


    if ( freePositions.empty() ) {
        position *more = ::new position[blockSize];
        for ( int i = 0; i < blockSize; ++i ) {
            freePositions.push_back(&more[i]); 
        }
    }
      
    retval = freePositions.back();
    freePositions.pop_back();
    

    return retval;
}

void position::operator delete(void *obj)
{
    if ( obj ) {
        freePositions.push_back((position *)obj);
    }
}

position::datatype position::initType(const location *l, extractflag e)
{
  datatype retval = BP;

  if ( l )
    {
      if ( e == START )
	if ( l->start_line == 0 )
	  retval = BP;
	else
	  retval = RC;
      else
	if ( l->end_line == 0 && l->start_line == 0 )
	  retval = BP;
	else
	  retval = RC;
    }

  return retval;
}

int position::initWithLocation(const location *l, extractflag e)
{
  int retval = 0;

  if ( l && l->filename )
    {
      if ( e == START )
	{
	  if ( l->start_line == 0 )
	    {
	      bp = l->start_column;
	      retval = 1;
	    }
	  else
	    {
	      if ( mappedFile::getFile(l->filename)->resolveRC(l->start_line, l->start_column, 0L, row, col) < 0 )
		{
		  row = l->start_line;
		  if ( l->start_column == 0 )
		    col = UINT_MAX;
		  else
		    col = l->start_column;
		}
	      retval = 1;
	    }
	}
      else
	{
	  if ( l->end_line == 0 )
	    {
	      // PROBLEM: cannot make an end out of a simple byte position
	      // as the position actually represents an offset from the
	      // start position, so let's try to do the math
	      if ( l->start_line == 0 ) 
		{
		  // start position is a byte offset, just add the two, and offset by 1
		  bp = l->end_column + l->start_column - 1;
		  retval = 1;
		}
	      else if ( l->end_column > 0 )
		{
		  // start position is a row/col pair, and we are a byte offset from that RC positon
		  // so, let's first ask our mapped file to give us the RC X bytes down from
		  // the start, and that'll be us.  Of course, if we cannot get that, then we 
		  // just fudge what we can
		  unsigned int tmpr;
		  unsigned int tmpc;
		  if ( mappedFile::getFile(l->filename)->resolveRC(l->start_line, l->start_column,
								   (long)(l->end_column-1), tmpr, tmpc) >= 0 )
		    {
		      row = tmpr;
		      col = tmpc;
		    }
		  else
		    {
		      row = l->start_line;
		      col = l->start_column + l->end_column - 1;
		    }
		  retval = 1;
		}
	      else
		{
		  // end position is 0/0 i.e. no position given, just set to 0/0
		  row = 0;
		  col = 0;
		  retval = 1;
		}
	    }
	  else // end point is alread a row/col
	    {
	      if ( mappedFile::getFile(l->filename)->resolveRC(l->end_line, l->end_column, 0L, row, col) < 0 )
		{
		  row = l->end_line;
		  if ( l->end_column == 0 )
		    col = UINT_MAX;
		  else
		    col = l->end_column;
		}
	      retval = 1;
	    }
	}

      if ( retval == 1 )
	if ( (file = mappedFile::getFile(l->filename)) == 0 )
	  retval = -1;
      
    }

  return retval;
}

position::position(void) : type(RC)
{
  invalidate();
}

position::position(const location *l, extractflag e) : type(initType(l,e))
{
  invalidate();
  if ( ! initWithLocation(l, e) )
    invalidate();
}

position::position(const mapline *ml, extractflag f, const char *which) : type(RC)
{
  invalidate();

  if ( ml && ml->sourceFile && ml->targetFile )
    {
      location l;
      if ( ml->sourceFile == which )
	{
	  l.start_line   = ml->sourceStart;
	  l.start_column = 1;
	  l.end_line     = ml->sourceEnd;
	  l.end_column   = 0;
	  l.filename     = ml->sourceFile;
	}
      else
	{
	  l.start_line   = ml->targetStart;
	  l.start_column = 1;
	  l.end_line     = ml->targetStart + ml->sourceEnd - ml->sourceStart;
	  l.end_column   = 0;
	  l.filename     = ml->targetFile;
	}
      if ( ! initWithLocation(&l, f) )
	invalidate();

    }
}
  
position::position(const position &other) : type(other.type)
{
  bp   = other.bp;
  row  = other.row;
  col  = other.col;
  file = other.file;
}

position::position(unsigned int b, mappedFile *f) : bp(b), type(BP), file(f)
{
}

position::position(unsigned int r, unsigned int c, mappedFile *f) : row(r), col(c), type(RC), file(f)
{
  if ( c == 0 )
    col = UINT_MAX;
}

position::~position(void)
{
}

int position::toLocation(location *l, extractflag f) const
{
  int retval = -1;

  if ( l )
    {
      if ( f == START )
	{
	  if ( type == BP )
	    {
	     l->start_line = 0;
	     l->start_column = bp;
	    }
	  else
	    {
	      l->start_line = row;
	      l->start_column = col;
	    }
	}
      else
	{
	  if ( type == BP )
	    {
	      // end is a BP only if start was a BP
	      l->end_line = 0;
	      l->end_column = bp - l->start_column;
	    }
	  else
	    {
	      l->end_line = row;
	      l->end_column = col;
	    }
	}
      
      l->filename = file->filename();
    }
  
  return retval;
}

int position::computeRC(unsigned int &outR, unsigned int &outC) const
{
  int retval = 1;

  if ( type != RC )
    {
      if ( file->resolveRC(1, 1, (long)bp, outR, outC) < 0 )
	{
	  outR = 1;
	  outC = bp;
	  retval = 0;
	}
    }
  else
    {
      outR = row;
      outC = col;
    }
  
  return retval;
}

int position::computeRC(unsigned int inBP, unsigned int &outR, unsigned int &outC) const
{
  position p(inBP, file);
  return p.computeRC(outR, outC);
}

int position::computeBP(unsigned int &outBP) const
{
  int retval = 1;

  if ( type != BP )
    {
      if ( file->toBytePos(row, col, outBP) < 0 )
	{
	  outBP= UINT_MAX;
	  retval = 0;
	}
    }
  else
    {
      outBP = bp;
    }

  return retval;
}

int position::valid(void) const
{
  int retval = 0;

  if ( file )
    {
      if ( type == BP )
	{
	  if ( bp > 0 )
	    retval = 1;
	}
      else
	{
	  if ( row > 0 && col > 0 )
	    retval = 1;
	}
    }

  return retval;
}

int position::exists(void) const
{
  int retval = 0;

  if ( valid() )
    {
      unsigned int tmp1;
      unsigned int tmp2;

      if ( type == BP )
	{
	  if ( computeRC(tmp1, tmp2) != 0 )
	    retval = 1;
	}
      else
	{
	  if ( computeBP(tmp1) != 0 )
	    retval = 1;
	}
    }

  return retval;
}

void position::invalidate(void)
{
  bp  = 0;
  row = 0;
  col = 0;
  file= 0;
}

position *position::next(unsigned int increment) const
{
  position *retval = new position(*this);

  if ( retval )
    {
      // if we are a bp, we can do this ourselves
      if ( retval->type == BP )
	{
	  retval->bp += increment;
	}
      else
	{
	  unsigned int tmpr;
	  unsigned int tmpc;
	  if ( retval->file->resolveRC(row, col, (long)increment, tmpr, tmpc) >= 0 )
	    {
	      retval->row  = tmpr;
	      retval->col  = tmpc;
	    }
	  else
	    {
	      if ( col != UINT_MAX )
		retval->col += increment; 
	      else
		{
		  retval->row++;
		  retval->col = increment;
		}
	    }
	}
    }
  
  return retval;
}

position *position::prev(unsigned int decrement) const
{
  position *retval = new position(*this);

  if ( retval )
    {
      // if we are a bp, we can do this ourselves
      if ( retval->type == BP )
	{
	  retval->bp -= decrement;
	}
      else
	{
	  unsigned int tmpr;
	  unsigned int tmpc;
	  if ( retval->file->resolveRC(row, col, -(long)decrement, tmpr, tmpc) >= 0 )
	    {
	      retval->row = tmpr;
	      retval->col = tmpc;
	    }
	  else
	    {
	      if ( decrement > retval->col )
		{
		  retval->row--;
		  retval->col = UINT_MAX - decrement + 1;
		}
	      else
		{
		  retval->col -= decrement;
		}
	    }
	}
    }
  
  return retval;
}


void position::dumpTo(ostream &o) const
{
  if ( type == BP )
    {
      char *tmp = pdunsignedtoa(bp);
      o << tmp;
      FREE_MEMORY(tmp);
    }
  else
    {
      char *tmp = pdunsignedtoa(row);
      o << tmp;
      FREE_MEMORY(tmp);
      o << "/";
      tmp = pdunsignedtoa(col);
      o << tmp;
      FREE_MEMORY(tmp);
    }
}

position *position::offsetWith(const position *base, const position *new_base) const
{
  position *retval = new position(*this);

  if ( base && new_base && retval )
    {
      // either we need access to the file, or they all need to be the same type
      if ( type == base->type && base->type == new_base->type && type == BP )
	{
	  retval->file  = new_base->file;
	  retval->bp = bp - base->bp + new_base->bp;
	}
      else if ( type == base->type && base->type == new_base->type && type == RC )
	{
	  retval->file   = new_base->file;
	  retval->row    = row - base->row + new_base->row;
	  if ( row == base->row )
	    {
	      retval->col  = col - base->col + new_base->col;
	    }
	  else
	    {
	      retval->col = col;
	    }

	  if ( retval->col == UINT_MAX )
	    {
	      unsigned int tmp1, tmp2;
	      if ( retval->file->resolveRC(retval->row, 0, 0L, tmp1, tmp2) >= 0 )
		{
		  retval->col = tmp2;
		}
	      else
		{
		  retval->col = UINT_MAX;
		}
	    }
	} 
      else if ( fromFile()->fileExists() && new_base->fromFile()->fileExists() )
	{
	  unsigned int myBP;
	  unsigned int baseBP;
	  unsigned int newbaseBP;

	  computeBP(myBP);
	  base->computeBP(baseBP);
	  new_base->computeBP(newbaseBP);

	  retval->file  = new_base->file;
	  retval->bp    = myBP - baseBP + newbaseBP;
	  
	  if ( retval->type != BP )
	    computeRC(retval->bp, retval->row, retval->col);
	}
      else
	{
	  FREE_OBJECT(retval);
	}
    }

  return retval;
}

