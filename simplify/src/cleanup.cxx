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
#include "attributeList.h"
#include "cleanup.h"
#include "maplineList.h"
#include "simplify_globals.h"
#include "symbolStruct.h"
#include "uintList.h"

void cleanup()
{
  DestroySymbols();
  DestroyAttributes();
  DestroyDeclarations();
  DestroyIncludeRels();
  DestroyMaps();
  DestroyUses();
  DestroyCppUses();
  DestroyIncludeDirs();
}

void DestroyIncludeDirs()
{
  int size = GetIncludeDirs().size();
  for (int i = 0; i < size; ++i)
    {
      char* dir = GetIncludeDirs()[i];
      free(dir);
    }
}

void DestroySymbols()
{
  int size = GetSymbols().size();
  for (int i = 0; i < size; ++i)
    {
      symbol* sym = GetSymbols()[i];
      delete sym;
    }
  
  GetSymbols().clear();
}

void DestroyMaps()
{
  int size = GetMapLines().size();

  for (int i = 0; i < size; ++i)
    {
      mapline* map = (mapline*) GetMapLines()[i];
      if (map)
	{
	  free(map->sourceFile);
	  free(map->targetFile);
	  DestroyMaplineStruct(map);
	}
    }
  
  GetMapLines().clear();
}

void DestroyDeclarations()
{
  int size = GetDeclarations().size();
  for (int i = 0; i < size; ++i)
    {
      uintList* list = GetDeclarations()[i];
      if (list)
	DestroyUintList(list);
    }

  GetDeclarations().clear();
}

void DestroyIncludeRels()
{
  int size = GetIncludeRels().size();

  for (int i = 0; i < size; ++i)
    {
      Relation* rel = (Relation*) GetIncludeRels()[i];
      delete rel;
    }

  GetIncludeRels().clear();
}

void DeleteAttributeList(attributeList* list)
{
  if (list == 0)
    return;

  int len = list->size();
  for (int i = 0; i < len; ++i)
    {
      attribute* att = (*list)[i];
      if (att)
	{
	  free(att->name);
	  free(att->value);
	  DestroyAttributeStruct(att);
	}
    }

  DestroyAttributeList(list);
}

void DestroyAttributes()
{
  int size = GetAttributes().size();

  for (int i = 0; i < size; ++i)
    {
      attributeList* list = (attributeList*) GetAttributes()[i];
      DeleteAttributeList(list);
    }

  GetAttributes().clear();
}

void DestroyUses()
{
  int size = GetUses().size();
  for (int i = 0; i < size; ++i)
    {
      SymLine* use = (SymLine*) GetUses()[i];
      delete use;
    }

  GetUses().clear();
}

void DestroyCppUses()
{
  int size = GetCppUses().size();
  for (int i = 0; i < size; ++i)
    {
      SymLoc* use = (SymLoc*) GetCppUses()[i];
      delete use;
    }

  GetCppUses().clear();
}









