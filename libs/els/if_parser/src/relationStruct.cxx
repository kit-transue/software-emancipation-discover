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
#ifdef COUNT_STRUCTS
#include <iostream.h>
#endif

#include <parray.h>

#include <relationStruct.h>

#define RELATION_STRUCTS_TO_ALLOCATE 10000

static parray freeRels(RELATION_STRUCTS_TO_ALLOCATE);

#ifdef COUNT_STRUCTS
static int max_count = 0;
static int current_count = 0;
#endif

#ifndef USING_PURIFY
// override of new and delete
void *relation::operator new(size_t)
{
  relationStruct *retval = 0;

  if ( freeRels.size() == 0  )
    {
      relationStruct *more_rels = ::new relationStruct [ RELATION_STRUCTS_TO_ALLOCATE ];
      if ( more_rels )
	{
	  for ( int i = 0; i < RELATION_STRUCTS_TO_ALLOCATE; i++ )
	    {
	      freeRels.insert((void *)&more_rels[i]);
	    }
	}
    }
  
  retval = (relationStruct *) freeRels[freeRels.size()-1];
  freeRels.remove(freeRels.size()-1);
  
#ifdef COUNT_STRUCTS
  current_count++;
  if ( current_count > max_count )
    max_count = current_count;
#endif

  return retval;
}  
#endif

#ifndef USING_PURIFY
void relation::operator delete(void *loc)
{
  if ( loc )
    {
      freeRels.insert((void *)loc);
#ifdef COUNT_STRUCTS
      current_count--;
#endif
    }
}      
#endif

relationStruct *CreateRelationStruct(void)
{
  return new relationStruct;
}

void DestroyRelationStruct(relationStruct *rel)
{
  delete rel;
}

#ifdef COUNT_STRUCTS
void ReportRelationStructs(void)
{
  cout << "Maximum number of relation structs in use at one time: " << max_count << "\n";
  cout << "Current number of relation structs in use: " << current_count << "\n";
}
#endif

