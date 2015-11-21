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
#define _so_shared_h_C
#include <so_shared.h>
#undef  _so_shared_h_C

#ifndef _WIN32
#include <nlist.h>
#endif
#include <stdarg.h>
#include <fCall.h>
#include <stdio.h>
#include <fcntl.h>
#ifdef XXX_newSharedLibrary
/* constructor/init for a library */
struct sharedLibrary *
newSharedLibrary(item)
    struct sharedLibrary * item;
{
    if (!item)
	item = ALLOCATE(struct sharedLibrary, 1);

    if (item) {
	item->mapped_addr = (void *)0;
	item->name = (char *)0;
	item->symtab_manager = (void *)0;
	item->text_size = 0;
	item->next = item->prev = (struct sharedLibrary *)0;
    }

    return item;
}
#endif
struct sharedLibrary *
disposeSharedLibrary(list)
    struct sharedLibrary * list;
{
    struct sharedLibrary * ret_val = list;

    if (list) {
	if (list->prev)
	    list->prev->next = (struct sharedLibrary *)0;
	while (list) {
	    ret_val = list->next;
	    DISPOSE(list->name);
	    DISPOSE(list);
	    list = ret_val;
   	}
    }

    return ret_val;
}

/*
  returns: 
  (char *)0 : cannot build tagged filename
        !=0 : tagged filename
	
  INTERP, ParaSET
*/
char *
buildCommonsFilename(filename)
    char * filename;
{
    char * ret_name = (char *)0;

    if (filename) {
	ret_name = ALLOCATE(char, (strlen(filename)+strlen(RUNTIME_NAME_SUFFIX)+1));
	if (ret_name) {
	    strcpy(ret_name, filename);
	    strcat(ret_name, RUNTIME_NAME_SUFFIX);
	}
    }

    return ret_name;
}

/*
  returns: 
   0 : no symbols, no dynamic, or no debug
  <0 : zero symbol at place (-return)
  >0 : number of symbols found
  
  INTERP --> ParaSET (executable.RUNTIME_NAME_SUFFIX)
*/

int
collectCommonSymbols(header)
    struct sharedLibraryInfo * header;
{
    int ret_val = 0;

    return ret_val;
}
/*  -1 : error, (header does not exist)
     0 : OK (equal lists),
     1 : difference */
int
examineSharedLibraries(header)
    struct sharedLibraryInfo * header;
{
    int ret_val = -1;

#if defined (sun5)
    if (header) {
	SLIB_HEADER  * shared_start = (SLIB_HEADER *)0;
	SO_ID_TYPE * shared_objects = (SO_ID_TYPE *)0;
	struct sharedLibrary * item = (struct sharedLibrary *)0;
	long text_size = -1;
	void * mapped_addr = (void *)0;
	int count = 0;

	if (shared_start = (SLIB_HEADER *)(header->dynamic))
	    ret_val = 0;

	SLIB_SO_HEAD(shared_start, shared_objects, SO_ID_TYPE *);
	item = header->current_list;
	while (shared_objects && item && !ret_val) {
	    text_size = -1;
	    SO_TEXT_SIZE(shared_start,shared_objects, text_size, long);
	    mapped_addr = (void *)SO_MAPPED_ADDR(shared_objects);
	    ret_val = !(item->mapped_addr == mapped_addr && item->text_size == text_size); 
	    shared_objects = SO_NEXT(shared_objects);
	    item = item->next;
	    count++;
	}
	header->list_compare_status = count;
	ret_val = (ret_val > 0) ? ret_val : (item || shared_objects);
    }

#endif

    return ret_val;
}
/*
   START-LOG-------------------------------------------
 
   $Log: so_shared.c  $
   Revision 1.6 2000/07/07 08:12:48EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.5  1994/04/29  01:01:19  aharlap
 * Mixed Mode Cleanup
 *
 * Revision 1.4  1994/04/13  19:39:50  builder
 * Port
 *
 * Revision 1.3  1994/03/22  20:45:38  pero
 * eliminate printing of leading "0x" for address in hex (for "symtab" purposes)
 *
 * Revision 1.2  1994/03/22  14:19:11  pero
 * support for COMMON symbols (runtime resolve)
 *
 * Revision 1.1  1994/03/17  00:09:23  pero
 * Initial revision
 *
 
   END-LOG---------------------------------------------
 
*/
