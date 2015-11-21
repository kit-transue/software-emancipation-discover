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
#include <cstring>
#include <string>
using namespace std;

#include "InclusionNode.h"
#include "InclusionTree.h"
#include "astnodeStruct.h"
#include "astnodeList.h"
#include "attributeList.h"
#include "locationList.h"
#include "maplineList.h"
#include "pdupath.h"
#include "relationStruct.h"
#include "simplify.h"
#include "simplify_globals.h"
#include "symbolStruct.h"
#include "uintList.h"

int s_final = 0; //set to 1 when SMT file "" is processed

int ReportSym(unsigned int id, symbolStruct* sym)
{
  int ret = 1;
  symbol* newsym = new symbol(*sym);
  if (newsym)
    {
      GetSymbols()[id] = newsym;
      if (strcmp(newsym->kind, "file") == 0)
	{
	  //get absolute path
	  char* path = pdrealpath(newsym->name);
	  if (path)
	    {
	      free(newsym->name);
	      newsym->name = path;
	    }
	  else
	    {
	      Error(FILE_DOESNOT_EXIST, newsym->name);
	    }
	  if (source_id == 0) {
	      // The main source file is the first symbol
              // reported in the IF, but the id is not necessarily 1.
	      source_id = id;
	  }
	}
    }
  else
    {
      Error(OUT_OF_MEMORY);
      ret = 0;
    }

  return ret;
}

int ReportAttr(unsigned int id, attributeList *l)
{
  int ret = 1;

  attributeList* list = GetAttributes()[id];

  if (list == 0)
    {
      list = CreateAttributeList();
      GetAttributes()[id] = list;
    }

  if (list != 0)
    {
      int num = l->size();
      for (int i = 0; i < num; ++i)
	{
	  attribute* newattr = CreateAttributeStruct();
	  if (newattr)
	    {
	      newattr->name = strdup((*l)[i]->name);
              char const *value = (*l)[i]->value;
	      newattr->value = value ? strdup(value) : 0;
	      list->push_back(newattr);
	    }
	  else
	    {
	      Error(OUT_OF_MEMORY);
	      ret = 0;
	      break;
	    }
	}
    }
  else
    {
      Error(OUT_OF_MEMORY);
      ret = 0;
    }

  return ret;
}

int ReportRel(relation* rel)
{
  int ret = 1;

  int includerel = 0;
  int definerel = 0;

  if (strcmp(rel->name, "include") == 0)
    includerel = 1;
  else if (strcmp(rel->name, "define") == 0)
    definerel = 1;
  else
    return 1;

  int source = -1;
  int target = -1;
  
  if (rel->source && rel->source->size() > 0)
    source = (*rel->source)[0];

  if (rel->target && rel->target->size() > 0)
    target = (*rel->target)[0];

  if (source != -1 && target != -1)
    {
      
      if (includerel)
	{
	    int is_preinclude       = 0;
	    attributeList *att_list = rel->attributes;
	    if(att_list && att_list->size()){
		for(int i = att_list->size() - 1; i >= 0; i++) {
		    attribute *att = att_list->operator[](i);
		    if(strcmp(att->name, "preinclude") == 0){ // do not include relations to "-preinclude" include files into tree
			is_preinclude = 1;
			break;
		    }
		}
	    }
	    Relation* rel = new Relation(source, target);
	    if (rel) {
		rel->preinclude = is_preinclude;
		GetIncludeRels().push_back(rel);
	    } else {
		ret = 0;
		Error(OUT_OF_MEMORY);
	    }
	}
      else if (definerel)
          GetDefineRels()[target] = source;

    }

  return ret;
}

//////////////////////////////////////

inline int IsFinal(const char* file)
{
    return string(file).find("::Anonymous-Target") != string::npos;
}

int ReportMaplines(maplineList* l)
{
  int ret = 1;
  maplineList& list = *l;

  int num = list.size();
  for (int i = 0; i < num; ++i)
    {
      if (!IsFinal(list[i]->targetFile))
	continue;

      mapline* newmap = CreateMaplineStruct();
      if (newmap)
	{
	  newmap->sourceStart = list[i]->sourceStart;
	  newmap->sourceEnd = list[i]->sourceEnd;
	  newmap->targetStart = list[i]->targetStart;
	  newmap->sourceFile = strdup(list[i]->sourceFile);
	  newmap->targetFile = strdup(list[i]->targetFile);
	  
	  GetMapLines().push_back(newmap);
	}
      else
	{
	  Error(OUT_OF_MEMORY);
	  ret = 0;
	  break;
	}
    }

  return ret;
}

int ProcessAstNode(astnode*, int);
int ReportAstTree(astnodeList* list)
{
  int size = list->size();
  for (int i = 0; i < size; ++i)
    {
      astnode* node = (*list)[i];

      int root = 0;

      if (s_final && node->location && node->location->size() > 0)
	{
	  location* loc = (*node->location)[0];
      
	  if (GetInclusionTree()->GetRoot()->ContainsLine(loc->start_line))
	    root = 1;
	}

      ProcessAstNode(node, root);
    }
  return 1;
}

int ReportSmtReference(unsigned int id, locationList* l)
{
  if (s_final && GetSymbols()[id])
    {
      int size = l->size();
      for (int i = 0; i < size; ++i)
	{
	  SymLine* newuse = new SymLine(id, (*l)[i]->start_line);
	  if (newuse)
	    GetUses().push_back(newuse);
	  else
	    Error(OUT_OF_MEMORY);
	}
    }
  return 1;
}

void ConstructInclusionTree();
int ReportFile(const char* file)
{
  if (IsFinal(file))
    {
      s_final = 1;
      ConstructInclusionTree();
    }
  return 1;
}









