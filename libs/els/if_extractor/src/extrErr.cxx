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
#include <pdustring.h>
#include <pdumem.h>
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
#include <extrErr.h>
#include <extrFctns.h>
//--EXTRACTOR

// Class statics
const int  ifExtractor::extrErr::blockSize    = 25;
parray    *ifExtractor::extrErr::freeExtrErrs = 0;

// new and delete overrides
void *ifExtractor::extrErr::operator new (unsigned int)
{
#ifndef USING_PURIFY
  extrErr *retval = 0;
  
  if ( ! freeExtrErrs )
    freeExtrErrs = new parray(blockSize);
  
  if ( freeExtrErrs )
    {
      if ( freeExtrErrs->size() == 0 )
	{
	  ifExtractor::extrErr *more = ::new ifExtractor::extrErr[blockSize];
	  if ( more )
	    for ( int i = 0; i < blockSize; i++ )
	      freeExtrErrs->insert((void *)&more[i]);
	}
      
      retval = (ifExtractor::extrErr *)(*freeExtrErrs)[freeExtrErrs->size()-1];
      freeExtrErrs->remove(freeExtrErrs->size()-1);
    }

  return retval;
#else
  return :: new extrErr;
#endif
}

void ifExtractor::extrErr::operator delete(void *obj)
{
#ifndef USING_PURIFY
  if ( obj )
    if ( freeExtrErrs )
      freeExtrErrs->insert(obj);
#else
  ::delete obj;
#endif
}

ifExtractor::extrErr::extrErr(void)
{
  printflag = 0;
}

ifExtractor::extrErr::extrErr(char *msg, location *l)
{
  message   = msg;
  loc       = l;
  printflag = 0;
}

ifExtractor::extrErr::~extrErr(void)
{
  FREE_MEMORY(message);
  FREE_OBJECT(loc);
}

int ifExtractor::extrErr::resolveLoc(void)
{
  int retval = -1;

  if ( loc )
    {
      location *newloc = 0;
      if ( elsMapper::getTrueLoc(loc, &newloc) >= 0 )
	{
	  delete loc;
	  loc = newloc;
	  retval = 1;
	}
    }

  return retval;
}

int ifExtractor::extrErr::print(raFile *f)
{
  int retval = -1;

  if ( f )
    {
      f->write("ERR ", 4);

      if ( loc )
	loc->print(f);

      f->write(" ", 1);

      if ( message )
	{
	  char *tmp = 0;
	  if ( iff_quote(message, &tmp) > 0 )
	    f->write(tmp, pdstrlen(tmp));
	  FREE_MEMORY(tmp);
	}

      f->write("\n", 1);
    }

  printflag = 0;
  return retval;
}


int ifExtractor::extrErr::printable(void)
{
  return printflag;
}

int ifExtractor::extrErr::tag(void)
{
  printflag = 1;
  return 1;
}

int ifExtractor::extrErr::reset(void)
{
  printflag = 0;
  return 1;
}

int ifExtractor::extrErr::refsFile(char *filename)
{
  return sameFile(filename, loc->filename);
}
