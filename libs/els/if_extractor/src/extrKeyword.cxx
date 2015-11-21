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
#include <iff_parser.h>
//--ELS

//++EXTRACTOR
#include <ifExtractor.h>
#include <extrKeyword.h>
#include <extrFctns.h>
//--EXTRACTOR

// Class statics
const int  ifExtractor::extrKeyword::blockSize        = 2500;
parray    *ifExtractor::extrKeyword::freeExtrKeywords = 0;

// new and delete ovkeywordides
void *ifExtractor::extrKeyword::operator new (unsigned int)
{
#ifndef USING_PURIFY
  extrKeyword *retval = 0;
  
  if ( ! freeExtrKeywords )
    freeExtrKeywords = new parray(blockSize);
  
  if ( freeExtrKeywords )
    {
      if ( freeExtrKeywords->size() == 0 )
	{
	  ifExtractor::extrKeyword *more = ::new ifExtractor::extrKeyword[blockSize];
	  if ( more )
	    for ( int i = 0; i < blockSize; i++ )
	      freeExtrKeywords->insert((void *)&more[i]);
	}
      
      retval = (ifExtractor::extrKeyword *)(*freeExtrKeywords)[freeExtrKeywords->size()-1];
      freeExtrKeywords->remove(freeExtrKeywords->size()-1);
    }

  return retval;
#else
  return ::new extrKeyword;
#endif
}

void ifExtractor::extrKeyword::operator delete(void *obj)
{
#ifndef USING_PURIFY
  if ( obj )
    if ( freeExtrKeywords )
      freeExtrKeywords->insert(obj);
#else
  ::delete obj;
#endif
}

ifExtractor::extrKeyword::extrKeyword(void)
{
  optkey    = 0;
  locs      = 0;
  printflag = 0;
}

ifExtractor::extrKeyword::extrKeyword(char *word, parray *ll)
{
  optkey    = word;
  locs      = ll;
  printflag = 0;
}

ifExtractor::extrKeyword::~extrKeyword(void)
{
  FREE_MEMORY(optkey);
  if ( locs )
    {
      for ( int i = locs->size() - 1; i >= 0; i-- )
	{
	  location *l = (location *)(*locs)[i];
	  if ( l )
	    FREE_OBJECT(l);
	}
      FREE_OBJECT(locs);
    }
}

int ifExtractor::extrKeyword::resolveLocs(void)
{
  int retval = -1;

  if ( locs )
    {
      int loc_len = locs->size();
      for ( int i = loc_len-1; i >= 0; i-- )
	{
	  location *loc    = (location *)(*locs)[i];
	  location *newloc = 0;
	  if ( elsMapper::getTrueLoc(loc, &newloc) >= 0 )
	    {
	      locs->remove(i);
	      locs->insert(newloc);
	      delete loc;
	    }
	}
      retval = 1;
    }
      

  return retval;
}

int ifExtractor::extrKeyword::print(raFile *f)
{
  int retval = -1;
  
  if ( f )
    {
      f->write("SMT keyword ", 12);

      if ( optkey )
	{
	  char *tmp = 0;
	  if ( iff_quote(optkey, &tmp) > 0 )
	    {
	      f->write(tmp, pdstrlen(tmp));
	      f->write(" ", 1);
	    }
	  FREE_MEMORY(tmp);
	}
  
      // print out locations of SMT
      if ( locs )
	{
	  int len = locs->size();
	  for ( int i = 0; i < len; i++ )
	    {
	      if ( i != 0 )
		f->write(", ", 2);
	      location *l = (location *)(*locs)[i];
	      if ( l )
		l->print(f);
	    }
	}
      f->write("\n", 1);
      
      retval = 1;
    }

  printflag = 0;
  return retval;
}

int ifExtractor::extrKeyword::printable(void)
{
  return printflag;
}

int ifExtractor::extrKeyword::tag(void)
{
  printflag = 1;
  return 1;
}

int ifExtractor::extrKeyword::reset(void)
{
  printflag = 0;
  return 1;
}


int ifExtractor::extrKeyword::refsFile(char *filename)
{
  int retval = 0;

  if ( locs )
    {
      int len = locs->size();
      for ( int i = 0; i < len && retval == 0; i++ )
	{
	  location *l = (location *)(*locs)[i];
	  retval = sameFile(filename, l->filename);
	}
    }

  return retval;
}

