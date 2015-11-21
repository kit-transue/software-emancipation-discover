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
#include <pduio.h>
#include <raFile.h>
//--OSPORT

//++ADS
#include <parray.h>
//__ADS

//++ELS
#include <relationStruct.h>
#include <uintList.h>
#include <attributeList.h>
#include <iff_parser.h>
//--ELS

//++EXTRACTOR
#include <ifExtractor.h>
#include <extrRel.h>
#include <extrSym.h>
#include <extrFctns.h>
//--EXTRACTOR

// Class statics
const int  ifExtractor::extrRel::blockSize    = 5000;
parray    *ifExtractor::extrRel::freeExtrRels = 0;

// new and delete ovrelides
void *ifExtractor::extrRel::operator new (unsigned int)
{
#ifndef USING_PURIFY
  extrRel *retval = 0;
  
  if ( ! freeExtrRels )
    freeExtrRels = new parray(blockSize);
  
  if ( freeExtrRels )
    {
      if ( freeExtrRels->size() == 0 )
	{
	  ifExtractor::extrRel *more = ::new ifExtractor::extrRel[blockSize];
	  if ( more )
	    for ( int i = 0; i < blockSize; i++ )
	      freeExtrRels->insert((void *)&more[i]);
	}
      
      retval = (ifExtractor::extrRel *)(*freeExtrRels)[freeExtrRels->size()-1];
      freeExtrRels->remove(freeExtrRels->size()-1);
    }

  return retval;
#else
  return ::new extrRel;
#endif
}

void ifExtractor::extrRel::operator delete(void *obj)
{
#ifndef USING_PURIFY
  if ( obj )
    if ( freeExtrRels )
      freeExtrRels->insert(obj);
#else
  ::delete obj;
#endif
}

ifExtractor::extrRel::extrRel(void)
{
  name          = 0;
  numAttrs      = 0;
  attrs         = 0;
  sourceNum     = 0;
  targetNum     = 0;
  sourcePrinted = 0;
  targetPrinted = 0;
}

ifExtractor::extrRel::extrRel(relation *r, unsigned int srcNum, unsigned int trgNum)
{
  name         = 0;
  numAttrs     = 0;
  attrs        = 0;
  if ( r )
    {
      name     = pdstrdup(r->name);
      numAttrs = r->attributes ? r->attributes->length() : 0;
      if ( numAttrs )
	{
	  attrs = new attribute[numAttrs];
	  if ( attrs )
	    for ( int i = 0; i < numAttrs; i++ )
	      {
		attrs[i].name  = pdstrdup(((attribute *)(*r->attributes)[i])->name);
		attrs[i].value = pdstrdup(((attribute *)(*r->attributes)[i])->value);
	      }
	}
    }
  sourceNum     = srcNum;
  targetNum     = trgNum;
  sourcePrinted = 0;
  targetPrinted = 0;
}

ifExtractor::extrRel::~extrRel(void)
{
  FREE_MEMORY(name);
  for ( int i = 0; i < numAttrs; i++ )
    {
      FREE_MEMORY(attrs[i].name);
      FREE_MEMORY(attrs[i].value);
    }
  delete [] attrs;
}

int ifExtractor::extrRel::print(raFile *f)
{
  if ( sourcePrinted && targetPrinted && f)
    {
      char *tmp1 = pdunsignedtoa(sourceNum);
      char *tmp2 = pdunsignedtoa(targetNum);

      f->write("REL ", 4);

      f->write("[", 1);
      f->write(tmp1, pdstrlen(tmp1));
      f->write("] ", 2);

      if ( name )
	{
	  char *tmp3 = 0;
	  if ( iff_quote(name, &tmp3) > 0 )
	    {
	      f->write(tmp3, pdstrlen(tmp3));
	      f->write(" ", 1);
	    }
	  FREE_MEMORY(tmp3);
	}

      f->write("[", 1);
      f->write(tmp2, pdstrlen(tmp2));
      f->write("] ", 2);

      if ( attrs )
	{
	  f->write(" ", 1);
	  for ( int i = 0; i < numAttrs; i++ )
	    {
	      if ( i )
		f->write(", ", 2);
	      if ( attrs[i].name )
		f->write(attrs[i].name, pdstrlen(attrs[i].name));
	      if ( attrs[i].value )
		{
		  f->write("(", 1);
		  if ( isSpecialAttr(attrs[i]) )
		    {
		      f->write("[", 1);
		      f->write(attrs[i].value, pdstrlen(attrs[i].value));
		      f->write("]", 1);
		    }
		  else
		    f->write(attrs[i].value, pdstrlen(attrs[i].value));
		  f->write(")", 1);
		}
	    }
	}

      f->write("\n", 1);

      FREE_MEMORY(tmp1);
      FREE_MEMORY(tmp2);
    }
  
  sourcePrinted = 0;
  targetPrinted = 0;

  return 1;
}

int ifExtractor::extrRel::isImportant(void)
{
  return ( name && name[0] == 't' && pdstrcmp(name, "type") == 0 );
}

int ifExtractor::extrRel::getAttrSyms(parray *l)
{
  int retval = 0;

  if ( l )
    {
      for ( int i = 0; i < numAttrs; i++ )
	{
	  if ( isSpecialAttr(attrs[i]) )
	    {
	      int n = pdatoint(attrs[i].value);
	      if ( n > 0 )
		{
		  l->insert((void *)(unsigned int)n);
		  retval++;
		}
	    }
	}
    }

  return retval;
}

int ifExtractor::extrRel::isSpecialAttr(attribute &a)
{
  int retval = 0;

  if ( a.value &&
       pdstrcmp(a.name, "pointer" ) == 0 || 
       pdstrcmp(a.name, "reference" ) == 0 ||
       pdstrcmp(a.name, "return" ) == 0 )
    retval = 1;

  return retval;
}
