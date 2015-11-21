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
//++OSPORT
#include <pdumem.h>
#include <pdustring.h>
#include <raFile.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <elsMapper.h>
#include <locationStruct.h>
//--ELS

//++EXTRACTOR
#include <ifExtractor.h>
#include <extrSmt.h>
#include <extrFctns.h>
//--EXTRACTOR

// Class statics
const int  ifExtractor::extrSym::extrSmt::blockSize    = 5000;
parray    *ifExtractor::extrSym::extrSmt::freeExtrSmts = 0;

// new and delete ovsmtides
void *ifExtractor::extrSym::extrSmt::operator new (unsigned int)
{
#ifndef USING_PURIFY
  extrSmt *retval = 0;
  
  if ( ! freeExtrSmts )
    freeExtrSmts = new parray(blockSize);
  
  if ( freeExtrSmts )
    {
      if ( freeExtrSmts->size() == 0 )
	{
	  ifExtractor::extrSym::extrSmt *more = ::new ifExtractor::extrSym::extrSmt[blockSize];
	  if ( more )
	    for ( int i = 0; i < blockSize; i++ )
	      freeExtrSmts->insert((void *)&more[i]);
	}
      
      retval = (ifExtractor::extrSym::extrSmt *)(*freeExtrSmts)[freeExtrSmts->size()-1];
      freeExtrSmts->remove(freeExtrSmts->size()-1);
    }

  return retval;
#else
  return ::new extrSmt;
#endif
}

void ifExtractor::extrSym::extrSmt::operator delete(void *obj)
{
#ifndef USING_PURIFY
  if ( obj )
    if ( freeExtrSmts )
      freeExtrSmts->insert(obj);
#else
  ::delete obj;
#endif
}

ifExtractor::extrSym::extrSmt::extrSmt(void)
{
  implicit = 0;
  locs     = 0;
}

ifExtractor::extrSym::extrSmt::extrSmt(int impl, parray *ll)
{
  implicit = impl;
  locs     = ll;
}

ifExtractor::extrSym::extrSmt::~extrSmt(void)
{
  if ( locs )
    for ( int i = locs->size() - 1; i >= 0; i-- )
      {
	location *l = (location *)(*locs)[i];
	FREE_OBJECT(l);
      }
  FREE_OBJECT(locs);
}

int ifExtractor::extrSym::extrSmt::resolveLocs(void)
{
  int retval = -1;

  if ( locs )
    {
      int smt_loc_len = locs->size();
      for ( int i = smt_loc_len-1; i >= 0; i-- )
	{
	  location *loc    = (location *)(*locs)[i];
	  location *newloc = 0;
	  if ( elsMapper::getTrueLoc(loc, &newloc) >= 0 )
	    {
	      delete loc;
	      locs->remove(i);
	      locs->insert(newloc);
	    }
	}
      retval = 1;
    }
      

  return retval;
}

int ifExtractor::extrSym::extrSmt::print(unsigned int symnum, char *filefilter, raFile *f)
{
  int retval = -1;
  
  if ( f )
    {
      int headPrinted = 0;
      int count = 0;
      
      // print out locations of SMT
      if ( locs )
	{
	  int len = locs->size();
	  for ( int i = 0; i < len; i++ )
	    {
	      location *l = (location *)(*locs)[i];
	      if ( l )
		{
		  if ( sameFile(filefilter, l->filename) )
		    {
		      if ( ! headPrinted )
			{
			  char *tmp = pdunsignedtoa(symnum);
			  f->write("SMT [", 5);
			  f->write(tmp, pdstrlen(tmp));
			  f->write("] ", 2);
			  if ( implicit )
			    f->write("implicit ", 9);
			  headPrinted = 1;
			  FREE_MEMORY(tmp);
			}
		      if ( count != 0 )
			{
			  f->write(", ", 2);
			  count = 1;
			}
		      l->print(f);
		    }
		}
	    }
	}

      if ( headPrinted )
	f->write("\n", 1);

      retval = 1;
    }
  
  return retval;
}

int ifExtractor::extrSym::extrSmt::refsFile(char *filename)
{
  int retval = 0;

  if ( locs )
    {
      int len = locs->size();
      int done = 0;
      for ( int i = 0; i < len && ! done; i++ )
	{
	  location *l = (location *)(*locs)[i];
	  if ( l )
	    {
	      if ( sameFile(l->filename, filename) )
		{
		  retval = 1;
		  done = 1;
		}
	    }
	}
    }

  return retval;
}
