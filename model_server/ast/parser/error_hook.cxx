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
// error_hook.C
//------------------------------------------
// synopsis:
// The function used to report errors from the parser
// to the user interface.
//------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstdio>
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include "genError.h"
#include <machdep.h>


#include "errorReport.h"

extern "C" char* parser_malloc(size_t);
extern "C" char* parser_realloc(void*, size_t);
extern "C" void parser_free(void*);

extern "C" void ast_error_hook (const char*, int, const char*);

static sharedArrOf(errorReport) error_report_array;
static struct ast_shared_errors* errors_ptr;

extern "C" int no_init_hook;

// Read error_report_array from shared memory into local copy.

extern "C" void assign_ast_shared_errors(struct ast_shared_errors* p)
{
    OS_dependent::bcopy(p, &error_report_array, sizeof(error_report_array));
    errors_ptr = p;
    if (!no_init_hook)
      ast_error_hook (0,0,0);
}


// Write error_report_array from local copy into shared memory.

extern "C" void save_ast_shared_errors(struct ast_shared_errors* p)
{
      OS_dependent::bcopy(&error_report_array, p, sizeof(error_report_array));
}


// ast_error_hook() is called for each error encountered by the parser.
// Its job is to store each error into an array in shared memory, where it
// can be read by the parent (aset) process when the parser is done.

extern "C" void ast_error_hook(
    const char* filename, int linenum, const char* message)
{
    if(filename == NULL)	// Indicates RESET, free all message storage
    {
	for(int i = 0; i < error_report_array.size(); ++i)
	{
	    errorReport* report = error_report_array[i];
	    if(report->filename)
	    {
		parser_free(report->filename);
		report->filename = NULL;
	    }
	    report->linenum = 0;
	    if(report->message)
	    {
		parser_free(report->message);
		report->message = NULL;
	    }
	}
	error_report_array.reset();
    }
    else			// Store new error message
    {
	errorReport* report = error_report_array.grow(1);
	if(report == NULL)
	  return;

	report->filename = (char*)parser_malloc(strlen(filename) + 1);
	if(report->filename)
	    strcpy(report->filename, filename);

	report->linenum = linenum;

	if (message)
	  {
	    report->message = (char*)parser_malloc(strlen(message) + 1);
	    if(report->message)
	      strcpy(report->message, message);
	  }
	else
	  {
	    report->message=(char*)parser_malloc(strlen("<NULL ERROR>")+1);
	    if(report->message)
	      strcpy(report->message,"<NULL ERROR>" );
	  }
      }

    if (errors_ptr) save_ast_shared_errors (errors_ptr);
      return;
}


#define psetmalloc(s)       parser_malloc(s)
#define psetrealloc(a,s)    parser_realloc(a,s)
#define psetfree(a)


extern "C" char * shared_array_provide(char * ptr, int len, int& capacity, int objsize)
{
  if(len > capacity) {
    if(capacity == 0)
      capacity = 1;
    while(capacity < len)
      capacity *= 2;
    ptr =  (ptr ? psetrealloc((char*)ptr, capacity * objsize) : 
	    psetmalloc(capacity * objsize));
  }
  return ptr;
}

extern "C" void shared_arr_free(char*ptr)
{
  if (ptr) psetfree(ptr); 
}			      
