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
#include <astnodeStruct.h>

#define ASTNODE_STRUCTS_TO_ALLOCATE 35000

static parray freeAstnodes(ASTNODE_STRUCTS_TO_ALLOCATE);

#ifdef COUNT_STRUCTS
static int max_count = 0;
static int current_count = 0;
#endif

astnodeStruct *CreateAstnodeStruct(void)
{
  return new astnodeStruct;
}

#ifndef USING_PURIFY
void *astnodeStruct::operator new (size_t)
{
  void *retval = 0;

  if ( freeAstnodes.size() == 0  )
    {
      astnodeStruct *more = ::new astnodeStruct [ ASTNODE_STRUCTS_TO_ALLOCATE ];
      if ( more )
	{
	  for ( int i = 0; i < ASTNODE_STRUCTS_TO_ALLOCATE; i++ )
	    {
	      freeAstnodes.insert((void *)&more[i]);
	    }
	}
    }
  
  retval = freeAstnodes[freeAstnodes.size()-1];
  freeAstnodes.remove(freeAstnodes.size()-1);
  
#ifdef COUNT_STRUCTS
  current_count++;
  if ( current_count > max_count )
    max_count = current_count;
#endif

  return retval;
}
#endif

void DestroyAstnodeStruct(astnodeStruct *a)
{
  delete a;
}

#ifndef USING_PURIFY
void astnode::operator delete(void *a)
{
  if ( a )
    {
      freeAstnodes.insert((void *)a);
#ifdef COUNT_STRUCTS
      current_count--;
#endif
    }
}
#endif

#ifdef COUNT_STRUCTS
void ReportAstnodeStructs(void)
{
  cout << "Maximum number of astnode structs in use at one time: " << max_count << "\n";
  cout << "Current number of astnode structs in use: " << current_count << "\n";
}
#endif
