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
//++C++
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
#ifdef USE_EXTERN_LIB
#include <pdustring.h>
//#include <pdumem.h>
#include <raFile.h>
#endif
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <attributeStruct.h>
#include <iff_parser.h>
//--ELS

#define ATTRIBUTE_STRUCTS_TO_ALLOCATE 10000
static parray freeAttributes(ATTRIBUTE_STRUCTS_TO_ALLOCATE);

#ifdef COUNT_STRUCTS
static int max_count = 0;
static int current_count = 0;
#endif

attributeStruct *CreateAttributeStruct(void)
{
  return new attributeStruct;
}

#ifndef USING_PURIFY
void *attribute::operator new(size_t)
{
  void *retval = 0;

  if ( freeAttributes.size() == 0  )
    {
      attributeStruct *more = ::new attributeStruct [ ATTRIBUTE_STRUCTS_TO_ALLOCATE ];
      if ( more )
	{
	  for ( int i = 0; i < ATTRIBUTE_STRUCTS_TO_ALLOCATE; i++ )
	    {
	      freeAttributes.insert((void *)&more[i]);
	    }
	}
    }
  
  retval = (attributeStruct *) freeAttributes[freeAttributes.size()-1];
  freeAttributes.remove(freeAttributes.size()-1);
  
#ifdef COUNT_STRUCTS
  current_count++;
  if ( current_count > max_count )
    max_count = current_count;
#endif

  return retval;
}
#endif

void DestroyAttributeStruct(attributeStruct *a)
{
  delete a;
}

#ifndef USING_PURIFY
void attribute::operator delete(void *a)
{
  if ( a )
    {
      freeAttributes.insert((void *)a);
#ifdef COUNT_STRUCTS
      current_count--;
#endif
    }
}
#endif

int attribute::print(ostream &o)
{
  if ( name )
    o << name << ' ';
  if ( value )
    {
       char *str = 0;
       if ( iff_quote(value, &str) >= 0 )
          o << "(\"" << value << "\")";
       free(str);
    }
  return 1;
}  

#ifdef USE_EXTERN_LIB
int attribute::print(raFile *f)
{
  int retval = -1;

  if ( f )
    {
      if ( name )
	{
	  f->write(name, pdstrlen(name));
	  f->write(" ", 1);
	}
      if ( value )
	{
	  char *str = 0;
	  if ( iff_quote(value, &str) >= 0 )
	    {
	      f->write("(\"", 2);
	      f->write(value, pdstrlen(value));
	      f->write("\")",2);
	    }
	  free(str);
	}

      retval = 1;
    }

  return retval;
}  
#endif

#ifdef COUNT_STRUCTS
void ReportAttributeStructs(void)
{
  cout << "Maximum number of attribute structs in use at one time: " << max_count << "\n";
  cout << "Current number of atrribute structs in use: " << current_count << "\n";
}
#endif
