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
// mManager.h.C
//------------------------------------------

#include <cLibraryFunctions.h>
#include <msg.h>

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <sys/types.h>

#ifndef _WIN32
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <memory>
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#ifndef _WIN32
#include <alloca.h>
#else
#include <malloc.h>
#endif

#include <machdep.h>
#include <setprocess.h>
#include <MemSeg.h>

// System interface - problem with including system files




MemoryManager*          mmgr_instance;
MemoryManager*          builtin_mmgr;

//
// MemoryManager
//
// Constructor
//

MemoryManager::MemoryManager (char* value)
{
    char segfile_buffer[256];
    const char* segfile = value ? value : OSapi_getenv ("PSET_DEBUG_SEGFILE");
    if (segfile == 0  ||  *segfile == '\0')
    {
	// Emulate system tempdir() function:
	//   Use $TMPDIR if set, as tmp directory.
	//   Otherwise, use value of P_tmpdir macro from <stdio.h> but forced to /usr/tmp

	char tmpdir_buffer[256];
	const char* tmpdir = OSapi_getenv ("TMPDIR");
	if(tmpdir == 0  ||  *tmpdir == '\0')
	    tmpdir = "/usr/tmp/";

	// Make sure tmpdir ends with '/'.
	int tmpdir_len = strlen(tmpdir);
	if(tmpdir[tmpdir_len - 1] != '/')
	{
	    strcpy(tmpdir_buffer, tmpdir);
	    tmpdir_buffer[tmpdir_len] = '/';
	    tmpdir_buffer[tmpdir_len + 1] = '\0';
	    tmpdir = tmpdir_buffer;
	}

	// Get appropriate process id to create unique segfile name.
	// Slave processes get the parents process id.
	const int pid = ( pmgr_is_slave ? 0 : OS_dependent::getpid() );

	sprintf(segfile_buffer, "%sSharedMem%05d", tmpdir, pid);
	segfile = segfile_buffer;
    }

    // Create/access the memory segment.
    mseg = new((char *) segfile) MemorySegment((char *) segfile);

    if (!mseg) {
	msg("Cannot allocate Memory segment. Exiting ...") << eom;
	exit(0);
    }

    // Allocate or compute the appropriate pointer to the comm page.
    // The master process does the physical allocation.

    if ( pmgr_is_slave ) {
        comm_page_ptr =
	    (void*)(mseg->self + (sizeof(MemorySegment) | 4) + sizeof(MallocList));
    } else {
	const int size = DEFAULT_COMM_PAGE_SIZE * (DEFAULT_NUM_PROCS + 1);
	comm_page_ptr = mseg->malloc (size);
	memset (comm_page_ptr, 0,
		DEFAULT_COMM_PAGE_SIZE * (DEFAULT_NUM_PROCS + 1));
    }
}

//
// ~MemoryManager
//
// Destructor
//

MemoryManager::~MemoryManager()
{
    delete mseg;
}

//
// get_comm_page_ptr
//
// Retrive the pointer to comm page array
//

void *MemoryManager::get_comm_page_ptr ()
{
    return comm_page_ptr;
}

//
// malloc
//
// Memory allocator
//

void* MemoryManager::malloc (unsigned size_bytes)
{
    return mseg ? mseg->malloc (size_bytes) : 0;
}

//
// free
//
// Memory deallocator
//

void MemoryManager::free (void *ptr)
{
    if ( mseg )
        mseg->free (ptr);
}

//
// calloc
//
// Allocator for initialized memory
//

void *MemoryManager::calloc (unsigned size_bytes, UBYTE fill)
{
    char* ptr = 0;
    if (mseg  &&  (ptr = (char*)mseg->malloc (size_bytes)))
	memset (ptr, fill, size_bytes);

    return (void*)ptr;
}

//
// realloc
//
// Function to change size of the allocated memory
//

void *MemoryManager::realloc (void *ptr, unsigned size_bytes)
{
    return mseg ? mseg->realloc (ptr, size_bytes) : 0;
}

int MemoryManager::check_fndecl (void* ptr)
{
    return mseg ? mseg->check_fndecl (ptr) : 0;
}

void MemoryManager::check_seg_size ()
{
    mseg->check_seg_size ();
}

void MemoryManager::set_ast_tree_ptr (void* p)
{
    mseg->set_ast_tree_ptr (p);
}

void* MemoryManager::get_ast_tree_ptr ()
{
    return mseg->get_ast_tree_ptr ();
}

void MemoryManager::set_cpp_tree_ptr (void* p)
{
    mseg->set_cpp_tree_ptr (p);
}

void* MemoryManager::get_cpp_tree_ptr ()
{
    return mseg->get_cpp_tree_ptr ();
}

void MemoryManager::set_esql_tree_ptr(void* p) {
    mseg->set_esql_tree_ptr(p);
}

void* MemoryManager::get_esql_tree_ptr() {
    return mseg->get_esql_tree_ptr();
}

struct ast_section* MemoryManager::get_section (char* name)
{
    return mseg->get_section (name);
}

void MemoryManager::lock_memory ()
{
    MemorySegment::mem_flag = 1;
}

void MemoryManager::unlock_memory ()
{
    MemorySegment::mem_flag = 0;
}

void MemoryManager::semaphores_off ()
{
    MemorySegment::mem_flag = 1;
}

void MemoryManager::semaphores_on ()
{
    MemorySegment::mem_flag = 0;
}
int MemoryManager::get_seg_used()
// find the amount of segmentation memory in use (like mallinfo)
//   This effectively tells us the maximum amount we have used,
// and should be close to what du -s tells us
{
    return mseg ? mseg->seg_used : 0;
}

/* imlementation of mem_segmemts methods */

mem_segments* mem_segments::lookup (char* name)
{
    mem_segments* tmp = builtin_mmgr->mseg->head;
    for (; tmp; tmp = tmp->next)
	if (tmp->name && strcmp (name, tmp->name) == 0)
	    break;
    
    return tmp;
}

mem_segments::mem_segments ()
    : next (NULL), mseg (NULL), name (NULL), rseg (NULL)
{
}

mem_segments* mem_segments::append (char* name)
{
    mem_segments* tmp = (mem_segments*) builtin_mmgr->malloc (sizeof (mem_segments));
    tmp->name = (char*)builtin_mmgr->malloc (strlen (name) + 1);
    strcpy (tmp->name, name);
    tmp->next = builtin_mmgr->mseg->head;
    builtin_mmgr->mseg->head = tmp;
    tmp->mseg = new(name) MemorySegment(name);
    tmp->rseg = NULL;
    current = tmp;
    return tmp;
}

void mem_segments::set_current ()
{
    if (this != current) {
	mseg = new(name) MemorySegment(name) ;
	current = this;
    }
}

/* C functions interface to memory segments methods */

extern "C" void mem_segments_set_current (mem_segments *m)
{
    if (m)
	m->set_current ();
}

extern "C" mem_segments* mem_segments_get_first ()
{
    return builtin_mmgr->mseg->get_head ();
}

extern "C" mem_segments* mem_segments_get_next (mem_segments* m)
{
    return m->next;
}

extern "C" char* get_builtin_segment_name ()
{
    return builtin_mmgr->mseg->get_name ();
}

extern "C" char* get_current_segment_name ()
{
    return mem_segments::current ? (mem_segments::current->name) : 0;
}

extern "C" mem_segments *mem_segments_get_current ()
{
    return mem_segments::current;
}
/*
   START-LOG-----------------------------------

$Log: mManager.h.cxx  $
Revision 1.13 2000/07/12 18:11:21EDT ktrans 
merge from stream_message branch
Revision 1.12  1994/07/09  00:24:20  aharlap
multiple ast

Revision 1.11  1994/05/26  12:02:48  davea
Bug track: n/a
Add function to support reporting amount of SharedMemory used

Revision 1.10  1994/04/29  01:01:19  aharlap
Mixed Mode Cleanup

Revision 1.9  1994/02/08  20:02:22  builder
Port

Revision 1.8  1993/11/18  22:37:56  davea
Bug track: 3772
Change name of file in /usr/tmp from memseg*	to
SharedMem*, so that customers don't get so worried.

Revision 1.7  1993/10/15  15:29:38  aharlap
Bug track: # 4987
More messages

Revision 1.6  1993/07/20  18:27:20  harry
HP/Solaris Port

Revision 1.5  1993/05/27  18:38:05  glenn
Rename ASET_USER_DEFINED_SEGFILE to PSET_DEBUG_SEGFILE.
Emulate system tempnam() function when building name of
temporary memseg file.

Revision 1.4  1993/04/28  16:52:04  aharlap
optimized using semaphores

Revision 1.3  1993/02/17  23:34:59  glenn
Include <alloca.h> to avoid undefined symbol "alloca" (it is really
a special builtin symbol).

Revision 1.2  1993/02/17  16:36:22  smit
make tmpfile name central to all processes here.

Revision 1.1  1992/10/10  20:30:34  builder
Initial revision

   END-LOG-------------------------------------
*/

