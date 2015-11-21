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
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

//++C++
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
#include <pdustring.h>
#include <pdumem.h>
#include <raFile.h>
#include <bufFile.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <ifFileCache.h>
#include <iff_parser.h>
#include <symbolStruct.h>
#include <locationStruct.h>
#include <elsMapper.h>
#include <locationList.h>
#include <attributeStruct.h>
#include <attributeList.h>
#include <astnodeList.h>
#include <relationStruct.h>
#include <uintList.h>
//--ELS

//++EXTRACTOR
#include <ifExtractor.h>
#include <extrSym.h>
#include <extrErr.h>
#include <extrKeyword.h>
#include <extrAst.h>
#include <extrRel.h>
#include <extrFctns.h>
//--EXTRACTOR

ifExtractor *ifExtractor::instance     = 0;
int          ifExtractor::showmaptable = 0;
int          ifExtractor::showprogress = 0;

ifExtractor::ifExtractor(const char *filename) 
{
  instance = this;
  syms     = new parray(4096);
  asts     = new parray(64);
  rels     = new parray(4096);
  errs     = new parray(64);
  kwds     = new parray(4096);
  if_file  = pdstrdup(filename);
}

ifExtractor::~ifExtractor(void)
{
  FREE_MEMORY(if_file);
  instance = 0;
  int i;
  for ( i = syms->size() - 1; i >= 0; i--)
    {
      extrSym *s = (extrSym *)(*syms)[i];
      if ( s )
	FREE_OBJECT(s);
    }
  FREE_OBJECT(syms);
  for ( i = asts->size() - 1; i >= 0; i--)
    {
      extrAst *a = (extrAst *)(*asts)[i];
      if ( a )
	FREE_OBJECT(a);
    }
  FREE_OBJECT(asts);
  for ( i = rels->size() - 1; i >= 0; i--)
    {
      extrRel *r = (extrRel *)(*rels)[i];
      if ( r )
	FREE_OBJECT(r);
    }
  FREE_OBJECT(rels);
  for ( i = errs->size() - 1; i >= 0; i--)
    {
      extrErr *e = (extrErr *)(*errs)[i];
      if ( e )
	FREE_OBJECT(e);
    }
  FREE_OBJECT(errs);
  for ( i = kwds->size() - 1; i >= 0; i--)
    {
      extrKeyword *k = (extrKeyword *)(*kwds)[i];
      if ( k )
	FREE_OBJECT(k);
    }
  FREE_OBJECT(kwds);
}

int ifExtractor::extract(parray *sources, parray *ifs)
{
  int retval = -1;

  if ( sources && ifs )
    {
      if ( if_file )
	{
	  if ( doParse(if_file) >= 0 )
	    {
	      if ( resolveLocs() >= 0 )
		{
		  retval          = 0;
		  for ( int i = sources->size() - 1; i >= 0; i-- )
		    {
		      bufFile *outfile         = 0;
		      char    *fileToExtract   = (char *)(*sources)[i];
		      char    *fileToExtractTo = (char *)(*ifs)[i];
		      
		      if ( outfile = new bufFile(fileToExtractTo, 0, 0) )
			if ( outfile->open(raFile_ACCESS_WRITE, raFile_OPEN_CREATE | raFile_OPEN_TRUNC) >= 0 )
			  if ( collectOutput(fileToExtract) >= 0 )
			    if  ( writeOutput(fileToExtract, outfile) >= 0 )
			      {
				retval++;
				reset();
			      }
			    else
			      cout << "Error trying to write extracted output to file '" << fileToExtractTo << '\'' << endl;
			  else
			    cout << "Error trying to determine the extracted IF for file '" << fileToExtract << '\'' << endl;
			else
			  {
			    cout << "Error trying to open file '" << fileToExtractTo << '\'' << endl;
			    cout << "OS gives error: \"" << pdstrerror(-1, 0) << '"' << endl;
			  }
		      else
			cout << "Error trying to create a file object for file '" << fileToExtractTo << '\'' << endl;
		      
		      FREE_OBJECT(outfile);
		    }
		}
	      else
		cout << "Error trying to resolve locations from MAP sections." << endl;
	    }
	  else
	    cout << "Error(s) found during parse phase of extraction for file '" << if_file << '\'' << endl;
	  iff_parser_reset();
	}
    }

  return retval;
}
			
int ifExtractor::doParse(char *filename)
{
  int retval = -1;

  if ( filename )
    {
      if ( showprogress ) cout << "Starting parse..." << endl;
      
      if ( iff_parse(if_file) == 1 )
	{
	  if ( showmaptable ) elsMapper::dumpAllMappingInfo(cout);
	  if ( showprogress ) cout << "...done" << endl;
	  retval = 1;
	}
    }

  return retval;
}
	  
int ifExtractor::resolveLocs(void)
{
  int retval = 0;

  if ( showprogress ) cout << "Resolving locations..." << endl;

  int len = syms->size();
  for ( int i = 0; i < len; i++ )
    {
      extrSym *s = (extrSym *)(*syms)[i];
      if ( s )
	s->resolveLocs();
    }

  len = kwds->size();
  for ( i = 0; i < len; i++ )
    {
      retval++;
      extrKeyword *k = (extrKeyword *)(*kwds)[i];
      if ( k )
	k->resolveLocs();
    }

  len = errs->size();
  for ( i = 0; i < len; i++ )
    {
      retval++;
      extrErr *e = (extrErr *)(*errs)[i];
      if ( e )
	e->resolveLoc();
    }

  len = asts->size();
  for ( i = 0; i < len; i++ )
    {
      retval++;
      extrAst *a = (extrAst *)(*asts)[i];
      if ( a )
	a->resolveLocs();
    }

  if ( showprogress ) cout << "...done" << endl;

  return retval;
}

int ifExtractor::collectOutput(char *forFile)
{
  int retval = -1;

  if ( forFile )
    {
      if ( showprogress ) cout << "Collecting output for file '" << forFile << '\'' << endl;

      if ( walkAllAstsForSyms(forFile) >= 0 )
	if ( walkKeywordsFor(forFile) >= 0 )
	  if ( walkErrorsFor(forFile) >= 0 )
	    retval = 1;

      if ( showprogress ) cout << "...done" << endl;
    }

  return retval;
}

int ifExtractor::writeOutput(char *forFile, raFile *f)
{
  int retval = -1;

  if ( forFile && f )
    {
      if ( showprogress ) cout << "Writing output for file '" << forFile << '\'' << endl;

      //
      // Output file header (the SYM for the file and an SMT reference to it)
      //
      extrSym *sfile = getSymFile(forFile);
      if ( sfile )
	{
	  sfile->tag();
	  sfile->print(forFile, f);
	  f->write("SMT file ", 9);
	  char *tmp = 0;
	  if ( iff_quote(sfile->fromFile(), &tmp) > 0 )
	    f->write(tmp, pdstrlen(tmp));
	  FREE_MEMORY(tmp);
	  f->write("\n", 1);
      
	  //
	  // Output the tagged IF
	  //
	  dumpSyms(forFile, f);
	  dumpKeywords(forFile, f);
	  dumpErrors(forFile, f);
	  dumpAsts(forFile, sfile->fromFile(), f);
	  
	  retval = 1;
	}

      if ( showprogress ) cout << "...done" << endl;
    }
  
  return retval;
}

int ifExtractor::dumpSyms(char *filefilter, raFile *f)
{
  int retval = -1;

  if ( syms && f )
    {
      int len = syms->size();
      for ( int i = 1; i < len; i++ )  // NOTE: Syms should start at 1 for C/C++ from EDG Parser
	{
	  extrSym *s = (extrSym *)(*syms)[i];
	  if ( s )
	    s->print(filefilter, f);
	}
      retval = 0;
    }

  return retval;
}

int ifExtractor::dumpKeywords(char *filefilter, raFile *f)
{
  int retval = -1;

  if ( kwds && f )
    {
      retval = 0;
      int len = kwds->size();
      for ( int i = kwds->size() - 1; i >= 0; i-- )
	{
	  extrKeyword *k = (extrKeyword *)(*kwds)[i];
	  if ( k && k->printable() )
	    {
	      k->print(f);
	      kwds->remove(i);
	      FREE_OBJECT(k);
	      retval++;
	    }
	}
    }
  
  return retval;
}

int ifExtractor::dumpErrors(char *filefilter, raFile *f)
{
  int retval = -1;

  if ( errs && f )
    {
      retval = 0;
      int len = errs->size();
      for ( int i = len-1; i >= 0; i-- )
	{
	  extrErr *e = (extrErr *)(*errs)[i];
	  if ( e && e->printable())
	    {
	      e->print(f);
	      errs->remove(i);
	      FREE_OBJECT(e);
	      retval++;
	    }
	}
    }

  return retval;
}

int ifExtractor::dumpAsts(char *filefilter, char *relname, raFile *f)
{
  int retval = -1;
  
  if ( asts )
    {
      int len = asts->size();
      for ( int i = len - 1; i >= 0 ; i-- )
	{
	  if ( i == len - 1 )
	    f->write("AST {\n", 6);

	  extrAst *a = (extrAst *)(*asts)[i];
	  if ( a )
	    a->print(filefilter, relname, f);
	      
	  if ( i == 0 )
	    f->write("}\n", 2);
	}
      
      retval = 1;
    }

  return retval;
}

int ifExtractor::addSym(unsigned int symnum, symbol *s)
{
  int retval = -1;

  if ( s && instance && instance->syms )
    {
      extrSym *sym = getSymPtr(symnum);
      if ( sym )
	{
	  retval = sym->addSym(s);
	}
    }

  return retval;
}

int ifExtractor::addSmt(unsigned int symnum, int impl, locationList *ll)
{
  int retval = -1;

  if ( ll && instance )
    {
      extrSym *s = getSymPtr(symnum);
      if ( s )
	{
	  retval = s->addSmt(impl, ll);
	}
    }
  
  return retval;
}

int ifExtractor::addAtr(unsigned int symnum, attributeList *al)
{
  int retval = -1;

  if ( al && instance )
    {
      extrSym *sym = getSymPtr(symnum);
      if ( sym )
	{
	  retval = sym->addAtr(al);
	}
    }

  return retval;
}

int ifExtractor::addErr(char *msg, location *loc)
{
  int retval = -1;
  
  if ( msg && loc && instance )
    {
      extrErr *nerr = new extrErr(msg, loc);
      if ( nerr )
	if ( instance->errs->insert((void *)nerr ) >= 0 )
	  retval = 1;
    }

  return retval;
}

int ifExtractor::addAst(astnodeList *ast)
{
  int retval = -1;

  if ( instance && ast )
    {
      if ( instance->asts )
	{
	  extrAst *a = new extrAst(ast);
	  if ( a )
	    if ( instance->asts->insert((void *)a) >= 0 )
	      retval = 1;
	}
    }

  return retval;
}

int ifExtractor::addKeyword(char *word, locationList *ll)
{
  int retval = -1;

  if ( ll )
    {
      extrKeyword *keyw = new extrKeyword(word, ll);
      if ( keyw )
	{
	  if ( instance->kwds )
	    if ( instance->kwds->insert((void *)keyw) , instance->kwds->size() >= 0 )
	      retval = 1;
	}
    }
  
  return retval;
}

int ifExtractor::addRel(relation *r)
{
  int retval = 1;

  if ( instance )
    {
      if ( r && r->source && r->target )
	{
	  // break up the relationStruct into single source and single target pairs
	  for ( int i = r->source->size() - 1; i >= 0 && retval == 1; i-- )
	    for ( int j = r->target->size() - 1; j >= 0 && retval == 1; j-- )
	      {
		retval = -1;

		unsigned int srcNum = (unsigned int)(*r->source)[i];
		unsigned int trgNum = (unsigned int)(*r->target)[i];
		extrSym *srcSym = instance->getSymPtr(srcNum);
		extrSym *trgSym = instance->getSymPtr(trgNum);
		extrRel *newr = new extrRel(r, srcNum, trgNum);
		if ( srcSym && trgSym && newr )
		  if ( srcSym->addRel(newr) >= 0 && trgSym->addRel(newr) >= 0 )
		    if ( instance->rels && instance->rels->insert((void *)newr) >= 0 )
		      retval = 1;
	      }
	}
    }

  return retval;
}

int ifExtractor::addAstRef(unsigned int symnum, char *filename)
{
  int retval = -1;

  if ( instance->syms )
    {
      extrSym *s = getSymPtr(symnum);
      if ( s )
	s->tag();
      retval = 1;
    }

  return retval;
}


ifExtractor::extrSym *ifExtractor::getSymPtr(unsigned int num)
{
  extrSym *retval = 0;

  if ( instance )
    {
      if ( instance->syms )
	{
	  // grow the symbol table, if needed
	  for ( int i = instance->syms->size(); i <= num; i++ )
	    {
	      extrSym *nval = new extrSym;
	      nval->setNum(i);
	      instance->syms->insert((void *)nval);
	    }
	  
	  // find the sym
	  retval = (extrSym *)(*instance->syms)[num];
	}
    }

  return retval;
}

int ifExtractor::walkKeywordsFor(char *filename)
{
  int retval = -1;

  if ( kwds )
    {
      int len = kwds->size();
      for ( int i = 0; i < len; i++ )
	{
	  extrKeyword *k = (extrKeyword *)(*kwds)[i];
	  if ( k && k->refsFile(filename) )
	    {
	      k->tag();
	    }
	}
      retval = 1;
    }

  return retval;
}


int ifExtractor::walkErrorsFor(char *filename)
{
  int retval = -1;

  if ( errs )
    {
      int len = errs->size();
      for ( int i = 0; i < len; i++ )
	{
	  extrErr *e = (extrErr *)(*errs)[i];
	  if ( e && e->refsFile(filename) )
	    {
	      e->tag();
	    }
	}
      retval = 1;
    }

  return retval;
}


int ifExtractor::walkSymsFor(char *filename)
{
  int retval = -1;

  if ( syms )
    {
      int len = syms->size();
      for ( int i = 0; i < len; i++ )
	{
	  extrSym *s = (extrSym *)(*syms)[i];

	  if ( s && s->refsFile(filename) )
	    {
	      // tag syms that are directly in the file
	      s->tag();
	    }
	  else if ( s && s->smtRefsFile(filename) )
	    {
	      // tag syms that have SMTs in this file
	      s->tag();
	    }

	}
      retval = 1;
    }

  return retval;
}

int ifExtractor::walkAllAstsForSyms(char *filename)
{
  int retval = -1;

  if ( asts && filename )
    {
      int len = asts->size();
      for ( int i = 0; i < len; i++ )
	{
	  extrAst *a = (extrAst *)(*asts)[i];
	  if ( a )
	    {
	      a->walkForSyms(filename, 1);
	    }
	}
      retval = 1;
    }

  return retval;
}


int ifExtractor::reset(void)
{
  int len;
  int i;

  len = rels->size();
  for ( i = 0; i < len; i++ )
    {
      extrRel *r = (extrRel *)(*rels)[i];
      if ( r )
	r->reset();
    }

  return 1;
}

void ifExtractor::showMapTable(int flag)
{
  if ( flag )
    showmaptable = 1;
  else
    showmaptable = 0;
}

void ifExtractor::showProgress(int flag)
{
  if ( flag )
    showprogress = 1;
  else
    showprogress = 0;
}

ifExtractor::extrSym *ifExtractor::getSymFile(const char *filename)
{
  extrSym *retval = 0;

  if ( filename )
    {
      if ( syms )
	{
	  int n = syms->size();
	  for ( int i = 0; i < n && ! retval; i++ )
	    {
	      extrSym *s = (extrSym *)(*syms)[i];
	      if ( s )
		if ( s->isFile(filename) )
		  retval = s;
	    }
	}
    }

  return retval;
}

