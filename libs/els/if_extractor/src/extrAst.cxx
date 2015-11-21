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

//++ELS
#include <astnodeList.h>
#include <astnodeStruct.h>
#include <elsMapper.h>
#include <ifFileCache.h>
#include <iff_parser.h>
#include <locationList.h>
#include <locationStruct.h>
#include <mappedFile.h>
//--ELS

//++EXTRACTOR
#include <ifExtractor.h>
#include <extrAst.h>
#include <extrFctns.h>
//--EXTRACTOR

// Class statics
const int  ifExtractor::extrAst::blockSize    = 100;
parray    *ifExtractor::extrAst::freeExtrAsts = 0;

// new and delete ovastides
void *ifExtractor::extrAst::operator new (unsigned int)
{
#ifndef USING_PURIFY
  extrAst *retval = 0;
  
  if ( ! freeExtrAsts )
    freeExtrAsts = new parray(blockSize);
  
  if ( freeExtrAsts )
    {
      if ( freeExtrAsts->size() == 0 )
	{
	  ifExtractor::extrAst *more = ::new ifExtractor::extrAst[blockSize];
	  if ( more )
	    for ( int i = 0; i < blockSize; i++ )
	      freeExtrAsts->insert((void *)&more[i]);
	}
      
      retval = (ifExtractor::extrAst *)(*freeExtrAsts)[freeExtrAsts->size()-1];
      freeExtrAsts->remove(freeExtrAsts->size()-1);
    }

  return retval;
#else
  return ::new extrAst;
#endif
}

void ifExtractor::extrAst::operator delete(void *obj)
{
#ifndef USING_PURIFY
  if ( obj )
    if ( freeExtrAsts )
      freeExtrAsts->insert(obj);
#else
  ::delete obj;
#endif
}

ifExtractor::extrAst::extrAst(void)
{
}

ifExtractor::extrAst::extrAst(astnodeList *a)
{
  ast = a;
}

ifExtractor::extrAst::~extrAst(void)
{
  DeallocateRecursiveAST(ast);
}

int ifExtractor::extrAst::resolveLocs(void)
{
  int retval = -1;

  if ( ast )
    {
      int len = ast->length();
      for ( int i = 0; i < len; i++ )
	resolveLocsAstNode((*ast)[i]);
      retval = 1;
    }

  return retval;
}

int ifExtractor::extrAst::printAstNode(astnode *a, char *filefilter, raFile *f)
{
  static int indent = 2;
  int        i;
  
  int retval = 1;
  
  if ( a && f )
    {
      for ( i = 0; i < indent; i++ )
	f->write(" ", 1);
      
      if ( a->node )
	{
	  f->write(a->node, pdstrlen(a->node));
	  if ( a->generated && a->definition )
	    f->write(":cg,def", 7);
	  else if ( a->generated )
	    f->write(":cg", 3);
	  else if ( a->definition )
	    f->write(":def", 4);
	}

      f->write(" ", 1);
      if ( a->identifier )
	{
	  char *tmp = pdunsignedtoa(a->identifier);
	  f->write("[", 1);
	  f->write(tmp, pdstrlen(tmp));
	  f->write("]", 1);
	  FREE_MEMORY(tmp);
	}

      f->write(" ", 1);
      if ( a->location )
	{
	  int len = a->location->length();
	  for ( i = 0; i < len; i++ )
	    {
	      location *l = (*a->location)[i];
	      if ( l && l->filename )
		{
		  if ( sameFile(l->filename, filefilter) )
		    {
		      if ( i != 0 )
			f->write(",", 1);
		      l->print(f);
		    }
		}
	    }
	}
      
      f->write(" ", 1);
      if ( a->children )
	{
	  f->write("{\n", 2);
	  int len = a->children->length();
	  for ( i = 0; i < len; i++ )
	    {
	      indent += 1;
	      printAstNode((*(a->children))[i], filefilter, f);
	      indent -= 1;
	    }
	}
      else if ( a->value )
	{
	  f->write("{\n", 2);
	  for ( i = 0; i < indent+1; i++ )
	    f->write(" ", 1);
	  if ( a->value )
	    {
	      char *tmp = 0;
	      if ( iff_quote(a->value, &tmp) > 0 )
		f->write(tmp, pdstrlen(tmp));
	      FREE_MEMORY(tmp);
	      f->write("\n", 1);
	    }
	}
      else
	{
	  f->write("{}\n", 3);
	}
      
      if ( a->children || a->value )
	{
	  for ( i = 0; i < indent; i++ )
	    f->write(" ", 1);
	  f->write("}\n", 2);
	}
    }
  
  return retval;
}

int ifExtractor::extrAst::print(char *filefilter, char *relname, raFile *f)
{
  if ( ast && filefilter && f )
    {
      int len = ast->length();
      int headPrinted = 0;
      for ( int i = 0; i < len; i++ )
	{
	  if ( (*ast)[i]->location )
	    {
	      int loc_len = (*ast)[i]->location->length();
	      int done = 0;
	      for ( int j = 0; j < loc_len && ! done; j++ )
		{
		  locationList *ll = (*ast)[i]->location;
		  if ( ll )
		    {
		      location *l = (*ll)[j];
		      if ( l )
			{
			  if ( sameFile(filefilter, l->filename) )
			    {
			      if ( ! headPrinted )
				{
				  outputWrapper(l->filename, relname, f);
				  headPrinted = 1;
				}
			      printAstNode((*ast)[i], filefilter, f);
			      done = 1;
			    }
			}
		    }
		}
	    }
	}
      if ( headPrinted )
	{
	  f->write(" }\n", 3);
	}
    }

  return 1;
}

int ifExtractor::extrAst::resolveLocsAstNode(astnodeStruct *a)
{
  int retval = -1;
  
  if ( a )
    {
      int i; 

      if ( a->location )
	{
	  int len = a->location->length();
	  for ( i = 0; i < len; i++ )
	    {
	      location *loc    = (location *)(*a->location)[i];
	      location *newloc = 0;
	      if ( elsMapper::getTrueLoc(loc, &newloc) >= 0 )
		{
		  *loc = *newloc;
		  delete newloc;
		}
	    }
	}
      
      if ( a->children )
	{
	  int len = a->children->length();
	  for ( i = 0; i < len; i++ )
	    resolveLocsAstNode((*(a->children))[i]);
	}

      retval = 1;
    }

  return retval;
}

int ifExtractor::extrAst::walkForSyms(char *filename, int useFilename)
{
  return walkForSyms(ast, filename, useFilename);
}

int ifExtractor::extrAst::walkForSyms(astnodeList *al, char *filename, int useFilename)
{
  int retval = -1;

  if ( al && filename )
    {
      int len = al->length();
      for ( int i = 0; i < len; i++ )
	{
	  astnode *node = (*al)[i];
	  if ( node )
	    {
	      if ( node->location )
		{
		  int loc_len = node->location->length();
		  int done = 0;
		  for ( int j = 0; j < loc_len && ! done; j++ )
		    {
		      locationList *ll = node->location;
		      if ( ll )
			{
			  location *l = (*ll)[j];
			  if ( l )
			    {
			      if ( (useFilename == 0) || sameFile(filename, l->filename) )
				{
				  ifExtractor::addAstRef(node->identifier, filename);
				  if ( node->children )
				    walkForSyms(node->children, filename, 0);
				  done = 1;
				}
			    }
			}
		    }
		}
	      else
		{
		  if ( useFilename == 0 )
		    {
		      ifExtractor::addAstRef(node->identifier, filename);
		      if ( node->children )
			walkForSyms(node->children, filename, 0);
		    }
		}
	    }
	}
      retval = 1;
    }

  return retval;
}

int ifExtractor::extrAst::outputWrapper(char *filename, char *relname, raFile *f)
{
  int retval = -1;

  if ( filename && relname && f )
    {
      // get file size in bytes
      unsigned int filesize = 0;
      mappedFile *mf = mappedFile::getFile(filename);
      if ( mf ) 
	filesize = mf->fileSize();

      char *tmp  = 0;
      char *tmp2 = pdunsignedtoa(filesize);
      iff_quote(relname, &tmp);
      
      f->write(" ast_root ", 10);
      f->write(tmp, pdstrlen(tmp));
      f->write(" 1/1 ", 5);
      f->write(tmp2, pdstrlen(tmp2));
      f->write(" {\n", 3);

      FREE_MEMORY(tmp);
      FREE_MEMORY(tmp2);

      retval = 1;
    }

  return retval;
}

