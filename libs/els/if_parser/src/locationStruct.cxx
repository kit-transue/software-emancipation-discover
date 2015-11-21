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
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
#ifdef USE_EXTERN_LIB
#include <pdustring.h>
#include <pdumem.h>
#include <raFile.h>
#endif
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <iff_parser.h>
#include <locationStruct.h>
//--ELS

#define LOCATION_STRUCTS_TO_ALLOCATE 30000
static parray freeLocs(LOCATION_STRUCTS_TO_ALLOCATE);

#ifdef COUNT_STRUCTS
static int max_count = 0;
static int current_count = 0;
#endif


#ifndef USING_PURIFY
void *location::operator new(size_t)
{
  locationStruct *retval = 0;

  if ( freeLocs.size() == 0  )
    {
      locationStruct *more_locs = ::new locationStruct [ LOCATION_STRUCTS_TO_ALLOCATE ];
      if ( more_locs )
	{
	  for ( int i = 0; i < LOCATION_STRUCTS_TO_ALLOCATE; i++ )
	    {
	      freeLocs.insert((void *)&more_locs[i]);
	    }
	}
    }
  
  retval = (locationStruct *) freeLocs[freeLocs.size()-1];
  freeLocs.remove(freeLocs.size()-1);
  
#ifdef COUNT_STRUCTS
  current_count++;
  if ( current_count > max_count )
    max_count = current_count;
#endif

  return retval;
}  
#endif


#ifndef USING_PURIFY
void location::operator delete(void *loc)
{
  if ( loc )
    {
      freeLocs.insert((void *)loc);
#ifdef COUNT_STRUCTS
      current_count--;
#endif
    }
}      
#endif

int location::print(ostream &o, int withFile /*=0*/)
{
  int retval = 1;
  
  if ( withFile )
    o << '"' << filename << "\" ";
  
  if ( start_line == 0 )
    o << start_column << " ";
  else
    o << start_line  << "/" << start_column << " ";
  
  if ( end_line == 0 )
    {
      if ( end_column != 0 )
	o << end_column << " ";
    }
  else
    o << end_line  << "/" << end_column;

  return retval;
}

#ifdef USE_EXTERN_LIB
int location::print(raFile *f, int withFile /*=0*/)
{
  int retval = -1;
  
  if ( f )
    {
      if ( withFile )
	{
	  char *tmp = 0;
	  iff_quote(filename, &tmp);
	  f->write(tmp, pdstrlen(tmp));
	  f->write(" ", 1);
	  FREE_MEMORY(tmp);
	}

      char *tmp1 = pdunsignedtoa(start_line);
      char *tmp2 = pdunsignedtoa(start_column);
      char *tmp3 = pdunsignedtoa(end_line);
      char *tmp4 = pdunsignedtoa(end_column);
  
      if ( start_line == 0 )
	{
	  f->write(tmp2, pdstrlen(tmp2));
	  f->write(" ", 1);
	}
      else
	{
	  f->write(tmp1, pdstrlen(tmp1));
	  f->write("/", 1);
	  f->write(tmp2, pdstrlen(tmp2));
	  f->write(" ", 1);
	}
      
      if ( end_line == 0 )
	{
	  if ( end_column != 0 )
	    {
	      f->write(tmp4, pdstrlen(tmp4));
	      f->write(" ", 1);
	    }
	}
      else
	{
	  f->write(tmp3, pdstrlen(tmp3));
	  f->write("/", 1);
	  f->write(tmp4, pdstrlen(tmp4));
	}
      
      FREE_MEMORY(tmp1);
      FREE_MEMORY(tmp2);
      FREE_MEMORY(tmp3);
      FREE_MEMORY(tmp4);
      retval = 1;
    }
  
  return retval;
}
#endif

locationStruct *CreateLocationStruct(void)
{
  return new locationStruct;
}

void DestroyLocationStruct(locationStruct *loc)
{
  delete loc;
}
      
#ifdef COUNT_STRUCTS
void ReportLocationStructs(void)
{
  cout << "Maximum number of location structs in use at one time: " << max_count << "\n";
  cout << "Current number of location structs in use: " << current_count << "\n";
}
#endif


