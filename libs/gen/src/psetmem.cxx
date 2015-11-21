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
// psetmem.C
//------------------------------------------
// synopsis:
// Handle failure of memory allocation during C++ operator new and inline
// allocator psetmalloc
//
// description:
// Provide a single-retry system for handling memory failure.
// Emergency storage is allocated at the start of the application.
// On the first allocation failure, a warning message is printed,
// the emergency storage is freed and the application is allowed to
// continue.  On the second allocation failure, an error message is
// printed and an exception is raised.
//------------------------------------------
// Restrictions:
// 
// init_new_handler should be called exactly once during
// application startup.
//------------------------------------------

// include files

#include <msg.h>

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
#include <windows.h>
#endif

#include <psetmem.h>
#include <systemMessages.h>
#ifndef ISO_CPP_HEADERS
#include <new.h>
#else /* ISO_CPP_HEADERS */
#include <new>
#endif /* ISO_CPP_HEADERS */

#include <machdep.h>
#include <genError.h>
#include <messages.h>

#undef malloc

extern "C" void driver_exit();

//------------------------------------------
// psetmalloc_exit
//
// No memory left, exit system
//------------------------------------------
void psetmalloc_exit()
{
    msg("No free memory, exiting...", error_sev) << eom;
#ifdef _WIN32
    MessageBoxEx(NULL, "DIScover is low on memory. Exiting.", "DIScover",
		 MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, 0);
#endif
    driver_exit();
}

//------------------------------------------
// psetmalloc_handler
//
// Called by new handler or psetmalloc when malloc fails - it
// will free reserve memory if it still exists else exit
//------------------------------------------

char* psetmalloc_handler(unsigned /*size*/)
{
    psetmalloc_exit();
    return 0;
}

extern "C" char *psetmalloc_handler_c(unsigned size)
{
    return psetmalloc_handler(size);
}

//------------------------------------------
// psetrealloc_handler
//
// Called by psetrealloc when relloc fails - it
// will free reserve memory if it still exists else exit
//------------------------------------------
char* psetrealloc_handler(char* /*old_ptr*/, unsigned /*size*/)
{
    psetmalloc_exit();
    return 0;
}

extern "C" char *psetrealloc_handler_c(char* ptr, unsigned size)
{
   return psetrealloc_handler(ptr, size);
}

#define BZERO(ptr,len) memset(ptr, '\0', len)

char * gen_array_provide(char * ptr, int len, int& capacity, int objsize)
{
   size_t old_sz = capacity * objsize;

   if(len > capacity) {
      if(capacity == 0)
	    capacity = 1;
      while(capacity < len)
	    capacity *= 2;

      size_t new_sz = capacity * objsize;
      ptr = (ptr) ? (char *)psetrealloc(ptr, new_sz) : (char *)psetmalloc(new_sz);

      if (ptr && (new_sz > old_sz))
	 BZERO((ptr + old_sz), new_sz - old_sz);
   }
   return ptr;
}

void gen_arr_free(char*ptr)
{
  if (ptr) psetfree(ptr); 
}			      

/*
   START-LOG-------------------------------------------

   $Log: psetmem.cxx  $
   Revision 1.24 2003/06/21 14:25:59EDT Dmitry Ryachovsky (dmitry) 
   
   Revision 1.23 2001/07/25 20:41:49EDT sudha 
   Changes to support model_server.
 * Revision 1.7  1994/04/13  19:32:33  builder
 * Port
 *
 * Revision 1.6  1994/03/25  14:06:52  builder
 * Port
 *
 * Revision 1.5  1994/03/24  18:15:50  builder
 * Port
 *
 * Revision 1.4  1994/03/24  14:18:15  kws
 * Port
 *
 * Revision 1.3  1994/03/24  14:16:31  kws
 * Port
 *
 * Revision 1.2  1994/01/11  23:03:38  kws
 * *** empty log message ***
 *
 * Revision 1.1  1994/01/11  21:19:06  kws
 * Initial revision
 *

   END-LOG---------------------------------------------

*/

