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
#ifndef _process_h
#define _process_h

#include <SymManager.h>
#include <genString.h>

#include <sys/types.h>

#define NULLID 0
#define NULLPTR ((void *)0)

#define DEFAULT_NUM_PROCS	10	// Max number of processes
#define DEFAULT_COMM_PAGE_SIZE  128	// number bytes of per process shared
					// global space

#define SEM_KEY_PREFIX 	0x10000		// Prefix for semaphore key (key|pid)
#define COM_KEY_PREFIX	0x10000		// Previx for msg key (key|pid)

#define COM_RCV_KEY_PREFIX 0x10000
#define COM_SND_KEY_PREFIX 0x20000

#define SEM_MASTER_CTL	0		// Master control semaphore
#define SEM_RPC_CTL	1		// Remote fn call semaphore locking

#define ASET_FILE_PREFIX  "ASET_"
// Class definitions
class MemoryManager;

// Type definitions
typedef int SemaphoreId;
typedef int SegmentId;
typedef unsigned char UBYTE;

struct ast_section;

// Memory Manager class
class MemoryManager {
    friend struct mem_segments;
public:
    class MemorySegment *mseg;
    void *comm_page_ptr;  // Pointer to array of per process global space

public:

    MemoryManager (char* segfile);
    ~MemoryManager();

// Communications information

    void *get_comm_page_ptr(); // Get pointer  to head of per process data

// Memory allocation

    void *malloc (unsigned size_bytes);
    void free (void *ptr);
    void *calloc (unsigned size_bytes, UBYTE fill);
    void *realloc (void *ptr, unsigned size_bytes);
    void check_seg_size ();
    void *get_ast_tree_ptr ();
    void set_ast_tree_ptr (void*);
    struct ast_section* get_section (char*);
    void *get_cpp_tree_ptr ();
    void set_cpp_tree_ptr (void*);
    void* get_esql_tree_ptr();
    void set_esql_tree_ptr(void*);
    void lock_memory ();
    void unlock_memory ();
    void semaphores_off ();
    void semaphores_on ();
    int check_fndecl (void*);
    int get_seg_used();               // find the amount of segmentation memory in use (like mallinfo)
    static MemoryManager* map (char*);
};

/* basic structure for multiple ast implementation */

struct mem_segments {
    mem_segments* next; /* linked list */
    class MemorySegment* mseg; 
    char* name;  /* name of mapped file */
    struct RelSegment* rseg; /* pointer to Relational object which has relation to all smtHeaders parsed to this segment */
    mem_segments ();
    static mem_segments* lookup (char*);
    static mem_segments* append (char*);
    MemorySegment* get_mseg () { return mseg; }
    void set_current ();
    static mem_segments* current;
};


// Communications class and type definitions
typedef struct {
    long mtype;
    char *mdata[1020];
} CommBuffer;



// Global data - pointer to manager instances
extern MemoryManager *builtin_mmgr;
extern "C" int pmgr_is_slave;

extern "C" {
    mem_segments* mem_segments_get_first();
    mem_segments* mem_segments_get_next(mem_segments*);
    mem_segments* mem_segments_get_current();
    void mem_segments_set_current(mem_segments*);
}

#endif

