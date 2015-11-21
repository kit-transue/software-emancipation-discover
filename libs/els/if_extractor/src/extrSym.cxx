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
#include <attributeStruct.h>
#include <attributeList.h>
#include <uintList.h>
#include <symbolStruct.h>
#include <locationList.h>
#include <elsMapper.h>
#include <position.h>
#include <ifFileCache.h>
#include <iff_parser.h>
//--ELS

//++EXTRACTOR
#include <ifExtractor.h>
#include <extrSym.h>
#include <extrSmt.h>
#include <extrRel.h>
#include <extrFctns.h>
//--EXTRACTOR


// Class statics
const int  ifExtractor::extrSym::blockSize    = 20000;
parray    *ifExtractor::extrSym::freeExtrSyms = 0;

// new and delete overrides
void *ifExtractor::extrSym::operator new (unsigned int)
{
#ifndef USING_PURIFY
  extrSym *retval = 0;
  
  if ( ! freeExtrSyms )
    freeExtrSyms = new parray(blockSize);
  
  if ( freeExtrSyms )
    {
      if ( freeExtrSyms->size() == 0 )
	{
	  ifExtractor::extrSym *more = ::new ifExtractor::extrSym[blockSize];
	  if ( more )
	    for ( int i = 0; i < blockSize; i++ )
	      freeExtrSyms->insert((void *)&more[i]);
	}
      
      retval = (ifExtractor::extrSym *)(*freeExtrSyms)[freeExtrSyms->size()-1];
      freeExtrSyms->remove(freeExtrSyms->size()-1);
    }

  return retval;
#else
  return ::new extrSym;
#endif
}

void ifExtractor::extrSym::operator delete(void *obj)
{
#ifndef USING_PURIFY
  if ( obj )
    if ( freeExtrSyms )
      freeExtrSyms->insert(obj);
#else
  ::delete obj;
#endif
}

ifExtractor::extrSym::extrSym(void)
{
  num   = 0;
  sym   = 0;

  printflag = 0;
  visited   = 0;

  attrs = new parray(2);
  smts  = new parray(4);
  rels  = new parray(4);
}

ifExtractor::extrSym::~extrSym(void)
{
  int i;
  for ( i = attrs->size() - 1; i >= 0; i--)
    {
      attribute *a = (attribute *)(*attrs)[i];
      if ( a )
	{
	  FREE_MEMORY(a->name);
	  FREE_MEMORY(a->value);
	  FREE_OBJECT(a);
	}
    }
  FREE_OBJECT(attrs);

  for ( i = smts->size() - 1; i >= 0; i--)
    {
      extrSmt *s = (extrSmt *)(*smts)[i];
      if ( s )
	FREE_OBJECT(s);
    }
  FREE_OBJECT(smts);

  FREE_OBJECT(rels);

  // if the sym is a SYM file, we *DO NOT* want to have
  // the symbol destructor free the values in kind or
  // name, as these are shared amongst several objects
  if ( sym )
    {
      if ( pdstrcmp(sym->kind, "file") == 0 )
	{
	  sym->kind = 0;
	  sym->name = 0;
	}
      FREE_OBJECT(sym);
    }

  
  //FREE_OBJECT(originalLoc);
}

ifExtractor::extrSym::addSmt(int impl, locationList *ll)
{
  int retval = -1;

  if ( ll )
    {
      extrSmt *smt = new extrSmt(impl, ll);
      if ( smt )
	{
	  if ( smts )
	    if ( smts->insert((void *)smt) >= 0 )
	      retval = 1;
	}
    }
  
  return retval;
}

int ifExtractor::extrSym::addSym(symbol *s)
{
  if ( sym )
    FREE_OBJECT(sym);
  sym = s;
  return 1;
}

int ifExtractor::extrSym::setNum(unsigned int i)
{
  num = i;
  return 1;
}

int ifExtractor::extrSym::addAtr(attributeList *al)
{
  int retval = -1;

  if ( al && attrs )
    {
      for ( int i = al->length()-1; i >= 0; i-- )
	{
	  attribute *a = (*al)[i];
	  if ( a )
	    attrs->insert((void *)a);
	}
      retval = 1;
    }
  
  return retval;
}

int ifExtractor::extrSym::addRel(ifExtractor::extrRel *r)
{
  int retval = -1;

  if ( r )
    {
      if ( rels )
	retval = rels->insert((void *)r);
    }

  return retval;
}

int ifExtractor::extrSym::tag(void)
{
  if ( ! visited )
    {
      visited = 1;
      printflag = 1;
      tagFromRels();
    }
  
  return 1;
}

int ifExtractor::extrSym::tagFromRels(void)
{
  int retval = -1;

  if ( rels )
    for ( int i = rels->size() - 1; i >= 0; i-- )
      {
	ifExtractor::extrRel *r = (ifExtractor::extrRel *)(*rels)[i];
	if ( r && r->hasTarget(num) && r->isImportant() ) 
	  {
	    unsigned int sourceSym = r->getSource();
	    if ( sourceSym )
	      {
		ifExtractor::extrSym *sourceSymPtr = ifExtractor::getSymPtr(sourceSym);
		if ( sourceSymPtr )
		  sourceSymPtr->tag();
	      }
	    parray attrSyms(1);
	    for ( int j = r->getAttrSyms(&attrSyms) - 1; j >= 0; j-- )
	      {
		unsigned int attrSym = (unsigned int)attrSyms[j];
		if ( attrSym )
		  {
		    ifExtractor::extrSym *attrSymPtr = ifExtractor::getSymPtr(attrSym);
		    if ( attrSymPtr )
		      attrSymPtr->tag();
		  }
	      }
	    retval = 1;
	  }
      }
  
  return retval;
}

int ifExtractor::extrSym::resolveLocs(void)
{
  //
  // Resolve own location
  //
  if ( sym )
    {
      
      location *newloc = 0;
      if ( elsMapper::getTrueLoc(sym->loc, &newloc) >= 0 )
	{
	  FREE_OBJECT(sym->loc);
	  sym->loc = newloc;
	}
    }
  
  // 
  // Resolve the location of all the SYM's SMT references
  //
  if ( smts )
    {
      int smt_len = smts->size();
      for ( int i = 0; i < smt_len; i++ )
	{
	  extrSmt *smt = (extrSmt *)(*smts)[i];
	  if ( smt )
	    smt->resolveLocs();
	}
    }

  return 1;
}

int ifExtractor::extrSym::printable(void)
{
  return printflag;
}


int ifExtractor::extrSym::print(char *filefilter, raFile *f)
{
  int retval = -1;

  if ( f )
    {
      if ( printflag )
	{
	  char *tmp = pdunsignedtoa(num);

	  // Print the SYM lines
	  if ( sym )
	    {
	      f->write("SYM [", 5);
	      f->write(tmp, pdstrlen(tmp));
	      f->write("] ", 2);
	      f->write(sym->kind, pdstrlen(sym->kind));
	      f->write(" ", 1);

	      if ( sym->generated )
		f->write(":cg ", 4);

	      if ( sym->name )
		{
		  char *tmp2     = 0;
		  char *print_me = 0;
		  if ( pdstrcmp(sym->kind, "file") == 0 )
		    print_me = sym->name;
		  else
		    print_me = sym->name;
		  if ( iff_quote(print_me, &tmp2) > 0 )
		    {
		      f->write(tmp2, pdstrlen(tmp2));
		      f->write(" ", 1);
		    }
		  FREE_MEMORY(tmp2);
		}
	      if ( sym->loc )
		if ( sameFile(filefilter, sym->loc->filename) )
		  sym->loc->print(f);
	      
	      f->write("\n", 1);
	    }
      
	  // Print all ATR lines for this sym
	  if ( attrs )
	    for ( int i = attrs->size()-1; i >=0 ; i-- )
	      {
		attribute *a = (attribute*)(*attrs)[i];
		if ( a )
		  {
		    f->write("ATR [", 5);
		    f->write(tmp, pdstrlen(tmp));
		    f->write("] ", 2);
		    a->print(f);
		    f->write("\n", 1);
		  }
	      }
      
	  // Print all SMT lines for this sym
	  if ( smts )
	    {
	      for ( int i = smts->size()-1; i >=0 ; i-- )
		{
		  extrSmt *s = (extrSmt *)(*smts)[i];
		  s->print(num, filefilter, f);
		  //smts->remove(i);
		  //FREE_OBJECT(s);
		}
	    }

	  // Let the Rels know that it has been printed
	  if ( rels )
	    {
	      for ( int i = rels->size()-1; i >=0 ; i-- )
		{
		  ifExtractor::extrRel *r = (ifExtractor::extrRel *)(*rels)[i];
		  r->symPrinting(num, f);
		}
	    }

	  FREE_MEMORY(tmp);
	}
      
      printflag  = 0;
      visited    = 0;

      retval = 1;
    }

  return retval;
}

int ifExtractor::extrSym::refsFile(char *filename)
{
  int retval = 0;

  if ( sym )
    if ( sym->loc )
      retval =  sameFile(filename, sym->loc->filename);
  
  return retval;
}

int ifExtractor::extrSym::smtRefsFile(char *filename)
{
  int retval = 0;

  if ( smts )
    {
      int len = smts->size();
      int done = 0;
      for ( int i = 0; i < len && ! done; i++ )
	{
	  extrSmt *s = (extrSmt *)(*smts)[i];
	  if ( s )
	    {
	      if ( s->refsFile(filename) )
		{
		  retval = 1;
		  done = 1;
		}
	    }
	}
    }

  return retval;
}

int ifExtractor::extrSym::isFile(const char *filename)
{
  int retval = 0;

  if ( filename )
    {
      if ( sym && sym->kind && sym->name )
	{
#ifndef _WIN32
	  if ( pdstrcmp(sym->kind, "file") == 0 && pdstrcmp(sym->name, filename) == 0 )
#else
	  if ( pdstrcmp(sym->kind, "file") == 0 && pdstrcasecmp(sym->name, filename) == 0 )
#endif
	    retval = 1;
	}
    }
  
  return retval;
}

char *ifExtractor::extrSym::fromFile(void)
{
  char *retval = 0;

  if ( sym )
    {
      if ( sym->loc )
	{
	  retval = sym->loc->relpath;
	}
      else if ( pdstrcmp(sym->kind, "file") == 0 )
	{
	  if ( sym->name )
	    {
	      retval = sym->name;
	    }
	}
    }

  return retval;
}


