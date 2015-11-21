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

#include "ast_section.h"
#include <sys/types.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#ifdef NULL
#undef NULL
#endif
// the includes abouve have the habit of undefining NULL
// this causes hard to track undefs.
#define NULL 0


typedef unsigned char UByte;

// (4) byte header for each malloc'ed space
typedef struct {
        UByte flag;		// Allocated or freed flag
        UByte block_num;	// Index of free list slot
        unsigned short size;    // Valid if <64K segment
} MallocList;

//
// Memory Segment interface
//

class MemorySegment {
    friend class MemoryManager;
    friend struct mem_segments;
private:
    char* self;			// Pointer to beginning of mapped space
    int version;		// Version of malloc manager
    char name[1024];		// Name of segment file
//  int seg_unit;		// Unit of segment file

    void* ast_tree_ptr;         // pointer to ast save structure

    struct ast_section* ast_section;
    int seg_size;		// Size of segment
    int seg_used;		// Space allocated from segment
    void* cpp_tree_ptr;
    void* esql_tree_ptr;
    struct mem_segments* head;
    MallocList *free_list[32];	// Free list chains

public:
    int language;
    void *sbrk (int num_bytes);	// Extend current segment
    void morecore (int num_blocks); // ...
    void check_seg_size ();
    void *get_ast_tree_ptr ();
    void set_ast_tree_ptr (void*);
    void *get_cpp_tree_ptr ();
    void set_cpp_tree_ptr (void*);
    void* get_esql_tree_ptr();
    void set_esql_tree_ptr(void*);
    struct ast_section *get_section (char*);
    mem_segments* get_head () {return head;};

public:
//
// Constructor/Destructor
//
    void *operator new(size_t ){return (void *)0;};
    void *operator new(size_t, char*);
    MemorySegment();
    MemorySegment(char *){};
    void operator delete(void *){};
    ~MemorySegment ();

//
// Standard allocation/free functions
//
    void *malloc (unsigned size);
    void *realloc (void *mem, unsigned new_size);
    void free (void *ptr);
    int check_fndecl (void*);
    static int mem_flag;
    char* get_name () {return (char*)name;}
};

/*
// MemSeg.h
//------------------------------------------
// synopsis:
//
// Definitions for segement of shared memory
//
//------------------------------------------
// $Log: MemSeg.h  $
// Revision 1.11 2000/07/10 23:07:41EDT ktrans 
// mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2.1.7  1994/08/05  20:30:00  aharlap
 * changed size MemorySegment::name
 *
 * Revision 1.2.1.6  1994/07/09  00:16:52  aharlap
 * multiple ast
 *
 * Revision 1.2.1.5  1994/04/28  20:50:34  aharlap
 * Mixed mode cleanup
 *
 * Revision 1.2.1.4  1994/02/19  17:20:33  aharlap
 * changed size of free_list from 20 to 32
 *
 * Revision 1.2.1.3  1993/04/28  16:50:58  aharlap
 * optimized using semaphores
 *
 * Revision 1.2.1.2  1992/10/09  17:38:41  builder
 * fixed rcs header
 *
 * Revision 1.2.1.1  92/10/07  21:24:18  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  21:24:17  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:24:35  smit
 * Initial revision
 * 
//Revision 1.4  92/10/01  23:20:52  builder
//added cpp_tree_ptr
//
//Revision 1.3  92/06/19  04:28:10  aharlap
//added field ast_section and method get_section
//
//Revision 1.2  92/04/21  21:48:39  aharlap
//Added ptr to ast save segment; seg_unit will be static variable file scope
/i/
//Revision 1.1  91/11/07  20:04:18  kws
//Initial revision
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/


