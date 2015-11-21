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
#include <astnodeStruct.h>
#include <locationStruct.h>
#include <relationStruct.h>
#include <symbolStruct.h>

#include <astnodeList.h>
#include <attributeList.h>
#include <locationList.h>
#include <maplineList.h>
#include <stringList.h>
#include <transformationList.h>
#include <uintList.h>

#include <iff_parser.h>
#include <if_parser_cbs.h>
#include <discover-interface.h>

//#ifndef USE_EXTERN_LIB
//#include <pdutil.h>
//#else
//#include <pdustring.h>
//#include <pdumem.h>
//#endif

int InsertSymbol(unsigned int identifier, int symflags, char *symbol, char *name, location *source)
{
  int retval = -1;

  if ( name && symbol )
    {
      symbolStruct *s = new symbolStruct;
      if ( s )
	{
	  s->name        = name;
	  s->kind        = symbol;
	  s->generated   = symflags & IFP_COMPILER_GENERATED ? 1 : 0;
	  s->replaceable = symflags & IFP_REPLACEABLE ? 1 : 0;
	  if ( source )
	    s->loc  = source;
	  else
	    s->loc = 0;
	  
	  retval = sym_insert_CB(identifier, s);
	  
	  if ( retval != 0 )
	    {
	      // set to zero to make sure that symbols delete does not free too much
	      s->name = 0;
	      s->kind = 0;
	      s->loc  = 0;
	      delete s;  s = 0;
	    }
	}
    }
  
  return retval;
}

int ModifyAttributes ( unsigned int identifier, attributeList *attr_list )
{
  int retval = -1;

  if ( attr_list )
    retval = atr_modify_CB(identifier, attr_list);
  
  return retval;
}

int AddReference(int implicit_flag, unsigned int identifier, locationList *locs)
{
  int retval = -1;

  if ( locs )
    if ( implicit_flag )
      retval = smt_implicit_CB(identifier,  locs);
    else
      retval = smt_reference_CB(identifier,  locs);

  return retval;
}

int AddKeywordReference(char *name, locationList *locs)
{
  int retval = -1;

  if ( locs )
    retval = smt_keyword_CB(name,  locs);
  
  return retval;
}

int ReportRelation(uintList *source, char *name, uintList *target, attributeList *attrs )
{
  int retval = -1;
  
  relation *r = new relation;
  if ( r )
    {
      r->source = source;
      r->name = name;
      r->target = target;
      r->attributes = attrs;

      retval = rel_create_CB(r);
      
      if ( retval != 0 ) {
          delete r;  r = 0; }    
    }

  return retval;
}

int ReportIncludeFiles(stringList *sl)
{
  int retval = -1;

  if ( sl )
    retval = cpp_include_order_CB(sl);

  return retval;
}

int ReportError(locationStruct *loc, char *msg)
{
  int retval = -1;

  if ( loc && msg )
    retval = err_message_CB(msg, loc);

  return retval;
}

void ASTPropogateFilenames(astnodeList *a, char *filename, char *relpath)
{
  if ( a )
    {
      int length = a->size();
      for ( int j = 0; j < length; j++ )
	{
	  astnodeStruct *n = (*a)[j];
	  if ( n->location )
	    {
	      int length2 = n->location->size();
	      for ( int i = 0; i < length2; i++ )
		if ( ! (*(n->location))[i]->filename )
		  {
		    (*(n->location))[i]->filename = filename;
		    (*(n->location))[i]->relpath  = relpath;
		  }
	    }
	  if ( n->children )
	    ASTPropogateFilenames(n->children, filename, relpath);
	}
    }
}

int ReportAST(char *filename, char *relpath, astnodeList *a)
{
  int retval = -1;
  
  if ( a )
    {
      if ( filename && relpath )
	ASTPropogateFilenames(a, filename, relpath);
      
      retval = ast_report_tree_CB(a);
    }

  return retval;
}

int ReportMaplines(char *sourcefile, char *src_relpath,
		   char *targetfile, char *targ_relpath,
		   maplineList *l)
{
  int retval = -1;

  if ( l )
    {
      int length = l->size();
      for ( int i = 0; i < length; i++ )
	{
	  (*l)[i]->sourceFile = sourcefile;
	  (*l)[i]->srcRelpath = src_relpath;
	  (*l)[i]->targetFile = targetfile;
	  (*l)[i]->trgRelpath = targ_relpath;
	}
  
      retval = transform_lines_CB(l);
    }

  return retval;
}

int ReportTransformation(int copy_flag, 
			 char *sourcefile, char *src_relpath,
			 char *targetfile, char *trg_relpath, 
			 transformationList *tl)
{
  int retval = -1;

  if ( sourcefile && targetfile && src_relpath && trg_relpath && tl )
    {
      for ( int i = tl->size()-1; i >= 0; i-- )
	{
	  (*tl)[i]->source->filename = sourcefile;
	  (*tl)[i]->source->relpath  = src_relpath;
	  (*tl)[i]->target->filename = targetfile;
	  (*tl)[i]->target->relpath  = trg_relpath;
	}

      if ( copy_flag )
	retval = transform_copy_CB(tl);
      else
	retval = transform_fixed_CB(tl);
    }

  return retval;
}

int ReportSmtFile(char* filename)
{
  int retval = -1;

  if ( filename )
    retval = smt_file_CB(filename);

  return retval;
}

