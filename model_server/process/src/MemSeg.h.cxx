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
// MemSeg.h.C
//------------------------------------------
// synopsis:
// MemSeg: shared memory segment manager
// description:
// This class manages a shared memory segment. This involves creating a shared
// segment and managing exteding the size of this segment, and dividing up the
// memory for allocation and freeing.
//------------------------------------------
// Restrictions:
// This class currently impliments the shared memory segment as a mapped file. 
// However, the IBM RS/6000 and the DecStation 3100 do not have/or does not work
// - working calls for mapped files. The IBM however can allocate unlimited
// 1024 byte shared memory segments.  The IBM can use a more sophisticated
// alogrithm allocating small pages.  The Dec, well.... (wait for OSF unix)??
//
//------------------------------------------


#include <cLibraryFunctions.h>
#include <msg.h>

#ifndef ISO_CPP_HEADERS
#include <errno.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include <fcntl.h>

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <cstdio>
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#else
#include <unistd.h>
#endif

#include <machdep.h>
#include <pset_mman.h>

#ifndef _WIN32
#include <sys/mman.h>
#endif

#include <setprocess.h>
#include <MemSeg.h>
#include <pset_mmap.h>

#ifndef _WIN32
#include <sys/utsname.h>
#endif

#ifdef _WIN32

static caddr_t get_start_shared_memory ()
{
    caddr_t ret_val = (caddr_t)0;
/*    char    buf[400];
    char    hostnamebuf[200];
    int     fd;
    caddr_t addr;

    gethostname(hostnamebuf, sizeof(hostnamebuf));
    sprintf(buf,"/tmp/.%s_mmaplocker", hostnamebuf);
    fd = OSapi_open (buf, O_RDONLY);
    if (fd != -1) {
	if (sizeof (addr) == OSapi_read (fd, (char*)&addr, sizeof (addr)))
	    ret_val  = addr;
	OSapi_close (fd);
    } */
    return ret_val;
}

#else

static caddr_t get_start_shared_memory ()
{
    caddr_t ret_val = (caddr_t)0;

    char buf[400];
    struct utsname unamedata;
    int fd;
    caddr_t addr;
    uname(&unamedata);
    sprintf (buf,"/tmp/.%s_mmaplocker", unamedata.nodename);
    fd = OSapi_open (buf, O_RDONLY);
    if (fd != -1) {
	if (sizeof (addr) == OSapi_read (fd, (char*)&addr, sizeof (addr)))
	    ret_val  = addr;
	OSapi_close (fd);
    }
    return ret_val;
}

#endif /* _WIN32 */

// Define macro constants - magic bytes, pointer chasing, etc

#define ALLOCATED   ((unsigned char) 0xf7) /* allocated */
#define FREED       ((unsigned char) 0x54) /* freed */
#define HEADERSIZE  sizeof(MallocList)
#define CHAIN(a)    (*(MallocList**)(sizeof(char*) + (char*)(a)))
#define BLOCKSIZE   1024
#define ONEMEG	    (1024 * 1024)

// Null out error checking

#define ASSERT(dummy) ;
#define abort(dummy) ;

struct mem_segments* mem_segments::current = NULL;

void inform_remap ();

int MemorySegment::mem_flag = 1;
extern "C" int  pmgr_is_slave;
extern "C" int __is_master_pset;

static int shared_memory_debug;
#define SHARED_DEBUG if (shared_memory_debug == 1)
static int unit = -1;  // !! < 

// Environment variables used
static char *env_debug_shmem   = "PSET_DEBUG_SHMEM";
static char *env_cpp_shmem     = "PSET_CPP_SHARED_SIZE";
static char* env_esql_shmem    = "PSET_ESQL_SHARED_SIZE";
static char *env_parser_shmem  = "PSET_SHARED_SIZE";
static char *env_debug_segfile = "PSET_DEBUG_SEGFILE";
static char *env_using_efs     = "PSET_EFS";    // For IRIX users w/EFS filesystem

//Printout tags for errors, warnings and messages
static char *err_tag = "SHMem Error";
static char *wrn_tag = "SHMem Warning";
static char *msg_tag = "SHMem Message";
static char *dbg_tag = "SHMem DEBUG";

//static int old_size = 0;

//-----------------------------------------------------
// MemorySegment::MemorySegment
//
// Constructor
//-----------------------------------------------------


static void* addr_map;

static int __size_shared_memory = -1;

// Default parts of __SIZE_SHARED_MEMORY (16 + 144 = 160)
// pset_mmap.h:static int __SIZE_SHARED_MEMORY = 0xa000000;  160M
#define DEFAULT_CPP_SHARED_SIZE 16
#define DEFAULT_ESQL_SHARED_SIZE 16
#define DEFAULT_PARSER_SHARED_SIZE 144
#define DEFAULT_ALL_SHARED_SIZE 160
static int __cpp_shared_size    = DEFAULT_CPP_SHARED_SIZE;
static int __esql_shared_size   = DEFAULT_ESQL_SHARED_SIZE;
static int __parser_shared_size = DEFAULT_PARSER_SHARED_SIZE;
static int __all_shared_size    = DEFAULT_ALL_SHARED_SIZE;

void *MemorySegment::operator new(size_t,char* segfile)
{
    static int count = 0;
    int size;
    void* addr;
    int err;

#ifdef irix6
    // In the rare event that someone might want to enable EFS
    // filesystem compatibility (XFS is the "new" standard),
    // we provide the following capability. (Note: the "discover"
    // invoker script should set the appropriate environment
    // variable if we're using EFS, but if our filesystem
    // detection fails for some reason, the user can always set
    // this by hand.)

    static int using_efs = -1; 
    
    char *env_val = NULL;
    if (using_efs == -1)
      env_val = OSapi_getenv(env_using_efs);
      if (env_val && !strcmp(env_val, "1"))
        using_efs = 1;
      else
        using_efs = 0;
#endif

    if (__size_shared_memory == -1) {
	if (shared_memory_debug == 0)
	    shared_memory_debug = OSapi_getenv (env_debug_shmem) ? 1 : -1;

	char const *str_cpp = OSapi_getenv(env_cpp_shmem);
	if (str_cpp) {
	    int cpp_sz = atoi (str_cpp);
	    if (cpp_sz < DEFAULT_CPP_SHARED_SIZE) {
		msg("$1: $2= $3 is less than default $4. Ignore", normal_sev) << err_tag << eoarg << env_cpp_shmem << eoarg << str_cpp << eoarg << DEFAULT_CPP_SHARED_SIZE << eom;
	    } else {
		__cpp_shared_size = cpp_sz;
		SHARED_DEBUG msg("$1: Using $2= $3", normal_sev) << dbg_tag << eoarg << env_cpp_shmem << eoarg << str_cpp << eom;
	    }
	}

	char const *str_esql = OSapi_getenv(env_esql_shmem);
	if (str_esql) {
	    int esql_sz = atoi (str_esql);
	    if (esql_sz < DEFAULT_ESQL_SHARED_SIZE) {
		msg("$1: $2= $3 is less than default $4. Ignore", normal_sev) << err_tag << eoarg << env_esql_shmem << eoarg << str_esql << eoarg << DEFAULT_ESQL_SHARED_SIZE << eom;
	    } else {
		__esql_shared_size = esql_sz;
		SHARED_DEBUG msg("$1: Using $2= $3", normal_sev) << dbg_tag << eoarg << env_esql_shmem << eoarg << str_esql << eom;
	    }
	}

	char const *str = OSapi_getenv(env_parser_shmem);
	if (str) {
	    int parser_sz = atoi (str);
	    if (parser_sz < DEFAULT_PARSER_SHARED_SIZE) {
		msg("$1: $2= $3 is less than default $4. Ignore", normal_sev) << err_tag << eoarg << env_parser_shmem << eoarg << str << eoarg << DEFAULT_PARSER_SHARED_SIZE << eom;
	    } else {
		__parser_shared_size = parser_sz;
		SHARED_DEBUG msg("$1: Using $2= $3", normal_sev) << dbg_tag << eoarg << env_parser_shmem << eoarg << str << eom;
	    }
	}

	if (__cpp_shared_size != DEFAULT_CPP_SHARED_SIZE ||
	    __esql_shared_size != DEFAULT_ESQL_SHARED_SIZE ||
	    __parser_shared_size != DEFAULT_PARSER_SHARED_SIZE ) {
	    SHARED_DEBUG {
		msg("$1: SHARED memory changed CPP= $2 Mb, ESQL= $3 Mb parser= $4 Mb", normal_sev) << dbg_tag << eoarg << __cpp_shared_size << eoarg << __esql_shared_size << eoarg << __parser_shared_size << eom;
	    }

	    __all_shared_size    = __parser_shared_size + __esql_shared_size + __cpp_shared_size;
	    __size_shared_memory = __all_shared_size * ONEMEG;
	    
	    if (__size_shared_memory < 0) {
		__cpp_shared_size    = DEFAULT_CPP_SHARED_SIZE;
		__esql_shared_size   = DEFAULT_ESQL_SHARED_SIZE;
		__parser_shared_size = DEFAULT_PARSER_SHARED_SIZE;
		__all_shared_size    = DEFAULT_ALL_SHARED_SIZE;
		msg("$1: Too big total= $2 Mb. Ignore. Using default SHMEM sizes", normal_sev) << err_tag << eoarg << __all_shared_size << eom;
	    }
	}

	if (__size_shared_memory <= 0)
	    __size_shared_memory = 0;
    }

// Open the segment file for mapping

    if (__size_shared_memory) {
	pset_need_lock_shared_memory = 0;
	__SIZE_SHARED_MEMORY = __size_shared_memory;
    }

    SHARED_DEBUG {
	if (count == 0) 
	    msg("$1: Total SHARED memory CPP + ESQL + parser: $2 + $3 + $4 = $5 Mb == $6", normal_sev) << dbg_tag << eoarg << __cpp_shared_size << eoarg << __esql_shared_size << eoarg << __parser_shared_size << eoarg << __all_shared_size << eoarg << __SIZE_SHARED_MEMORY << eom;
    }

    if (__is_master_pset) {
	pset_need_lock_shared_memory = 0;
	if (count == 0) {
	    __START_SHARED_MEMORY = get_start_shared_memory ();
	}
    }
    unit = OSapi_open( segfile, O_RDWR|O_CREAT, 0600 );
    if ( unit < 0 ) {
        perror(segfile);
	exit(0);
    }
    size = (int)OSapi_lseek (unit, 0, 2);
    if ( size == 0 ) {

// File was new create initial segment of 1 megabyte
//	size = sizeof(MemorySegment) + __cpp_shared_size*ONEMEG;
	if (count == 0)
	    size = __cpp_shared_size*ONEMEG;
        else
	    size = __SIZE_SHARED_MEMORY - __cpp_shared_size*ONEMEG;
// ftruncate() on a zero-length file eats up disk space on irix 
// when we're using the old efs filesystem, so we set mmap flags
// for MAP_AUTOGROW and the file will be implicitly grown. This
// is probably a saner way of doing things than opening up a huge
// sparse file and gradually filling it up ourselves.
#ifndef _WIN32
#ifdef irix6
        if (!using_efs)
#endif
          err = ftruncate (unit, size);
#ifdef irix6
        else
          err = 0;
#endif
	if ( err != 0 ) {
	    msg("$1: Cannot truncate segfile", normal_sev) << err_tag << eom;
	    perror("");
	    OSapi_close (unit);
	    unit = -1;
	    return (MemorySegment *)0;
        }
#endif
	addr = 0;	// Set mapping address to be chosen by system
    } else {
// Segment file file existed, read its mapping address
        OSapi_lseek (unit, 0, 0);
        OSapi_read (unit, &addr, sizeof(void*));
    }
#ifdef irix6
    if (using_efs)
      size = __SIZE_SHARED_MEMORY - 16*ONEMEG;
#endif

    if ( addr == 0 ) {
// Mapping for first time - Initialize segment control variables
#ifndef _WIN32
        int flag = MAP_SHARED;
#endif
	if (count) {
	    if (addr_map) {
	        addr = addr_map;
#ifndef _WIN32
		flag |= MAP_FIXED;
#endif
            }
#ifdef irix6
            if (using_efs)
              flag |= MAP_AUTOGROW;
#endif
#ifdef _WIN32
            addr = 0;
	    HANDLE fhandle = (HANDLE)_get_osfhandle(unit);
	    if(fhandle == INVALID_HANDLE_VALUE)
		return (MemorySegment *)0;
            addr = pset_mmap (segfile, (caddr_t)addr, size,
			      PAGE_READWRITE, 0, fhandle, 0 );
#else
            addr = pset_mmap (segfile, (caddr_t)addr, size,
			      PROT_READ|PROT_WRITE, flag, unit, 0 );
#endif
	    addr_map = addr;
        } else {
	    count++;
	    if (__is_master_pset) {
		addr = __START_SHARED_MEMORY;
		if (__size_shared_memory || !__START_SHARED_MEMORY ) {
		    addr = 0;
		} else {
		    size += __parser_shared_size * ONEMEG;
#ifndef _WIN32
		    flag |= MAP_FIXED;
#endif
		}
	    }
#ifdef irix6
            if (using_efs)
              flag |= MAP_AUTOGROW;
#endif
#ifndef _WIN32
	    addr = pset_mmap (segfile, (caddr_t)addr, size,
			      PROT_READ|PROT_WRITE, flag, unit, 0 );
#else
		addr = 0;
		HANDLE fhandle = (HANDLE)_get_osfhandle(unit);
	    if(fhandle == INVALID_HANDLE_VALUE)
		return (MemorySegment *)0;
	    addr = pset_mmap (segfile, (caddr_t)addr, size,
			      PAGE_READWRITE, 0, fhandle, 0 );
#endif
	    if (__is_master_pset && !__size_shared_memory 
		&& __START_SHARED_MEMORY) {
	        addr_map = (char*)addr+__cpp_shared_size*ONEMEG;
		size -= __parser_shared_size * ONEMEG;
	    }
        }
	
        if (addr == (caddr_t)-1) {
            msg("$1: Initial mmap failed, add = $2", normal_sev) << err_tag << eoarg << addr << eom;
	    perror ("");
	    return (MemorySegment *)0;
	}
// Assign this to address of mapped segment
        ((MemorySegment *)addr)->self = (char*)addr;
        ((MemorySegment *)addr)->version = 0;
        strcpy (((MemorySegment *)addr)->name, segfile);
        ((MemorySegment *)addr)->seg_size = size;
        ((MemorySegment *)addr)->ast_tree_ptr = 0;
        ((MemorySegment *)addr)->cpp_tree_ptr = 0;
	((MemorySegment*) addr)->esql_tree_ptr = 0;
        ((MemorySegment *)addr)->ast_section = 0;
	((MemorySegment *)addr)->seg_used = sizeof(MemorySegment) | 4;
        ((MemorySegment *)addr)->head = 0;
        OSapi_bzero(((MemorySegment *)addr)->free_list, sizeof (((MemorySegment *)addr)->free_list));
#if defined(hp10)
        /*Using msync on HP to make the above changes to memory visible in 
          the physical file (which caused emacs to complain about shared
          memory not being installed).*/
        if (msync((caddr_t)addr, size, MS_INVALIDATE) == -1) {
            msg("msync failed", error_sev) << eom;
            perror("msync");
        }
#endif
    } else {
#ifdef _WIN32
	HANDLE fhandle = (HANDLE)_get_osfhandle(unit);
	if(fhandle == INVALID_HANDLE_VALUE)
	    return (MemorySegment *)0;
        addr = pset_mmap (segfile, (caddr_t)addr, size, 
			  PAGE_READWRITE, 0,
			  fhandle, 0 );
#else
#ifdef irix6
        if (!using_efs)
#endif /* irix6 */
        addr = pset_mmap (segfile, (caddr_t)addr, size, 
			  PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED,
			  unit, 0 );
#ifdef irix6
        else
          addr = pset_mmap (segfile, (caddr_t)addr, size, 
                          PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED|MAP_AUTOGROW,
                          unit, 0 );
#endif /* irix6 */
#endif /* _WIN32 */

// Check to see if segment was mapped and assign segment address to this
  if (addr != (caddr_t)-1) {
	    ((MemorySegment *)addr)->seg_size = size;
	    if (((MemorySegment *)addr)->seg_size < ((MemorySegment *)addr)->seg_used) {
		pset_munmap ((caddr_t)addr, size);
		addr = (caddr_t)-1;
	      }
	  }
      }
    if ( addr == (caddr_t)-1 ) {
      // Error mapping segment - clean up and clear - this
      msg("$1: Cannot map Memory Segment", normal_sev) << err_tag << eom;
      OSapi_close(unit);
      unit = -1;		// !! <
      return(MemorySegment *)0;
    }
    if (unit != -1)
      OSapi_close (unit);
    unit = -1;
    return (MemorySegment *)addr;
  }

//-----------------------------------------------------
// MemorySegment::~MemorySegment
//
// Free a memory segment
//-----------------------------------------------------

MemorySegment::~MemorySegment()
{
    char segfile[1024];
    strcpy(segfile, name);

// unmap the segment and if we are the master process unlink the segment 
// Note - should the segment file be unlinked

    pset_munmap (self, seg_size);

    if ( unit >= 0 )
	OSapi_close (unit);
    unit = -1;		// !! <
//    old_size = 0;
    if(pmgr_is_slave == 0) {
	char const *debug_segfile = OSapi_getenv (env_debug_segfile);
	if(debug_segfile == 0  ||  strcmp(debug_segfile, segfile))
	    OSapi_unlink (segfile);
    }

#if !defined(__GNUG__) || !defined(__GNUC__) || !defined(__GNUC_MINOR__) || !(__GNUC__==2 && __GNUC_MINOR__ == 6)
// move the following assigment to the calling function.
//    this = 0;		// To suppress de-allocation.
#endif    
}
//-----------------------------------------------------
// MemorySegment::sbrk
//
// Ask system for more memory 
//-----------------------------------------------------

void* MemorySegment::sbrk (int incr_size)
{
    int used;
// Check to see if segment has enough space left. For now, if there isnt 
// return 0 for nor more space available, here we would normally add more
// extend the segment file.

    if ( incr_size > (seg_size-seg_used)) {
	msg("$1: size of memory segment exhausted", normal_sev) << err_tag << eom;
	if (seg_size == __cpp_shared_size * ONEMEG) {
	    msg("$1: set \"setenv $2 $3\", and try again", normal_sev) << msg_tag << eoarg << env_cpp_shmem << eoarg << __cpp_shared_size + DEFAULT_CPP_SHARED_SIZE +
			 DEFAULT_ESQL_SHARED_SIZE << eom;
	} else if (seg_size == __parser_shared_size * ONEMEG) {
	    msg("$1: set \"setenv $2 $3\", and try again", normal_sev) << msg_tag << eoarg << env_parser_shmem << eoarg << __parser_shared_size + DEFAULT_CPP_SHARED_SIZE +
			 DEFAULT_ESQL_SHARED_SIZE << eom;
	}
	   
	return 0;
    }
    used = seg_used;
    seg_used += incr_size;

    return (void*)(self + used);
}

//-----------------------------------------------------
// MemorySegment::morecore
//
// Ask system for more memory 
//-----------------------------------------------------

void MemorySegment::morecore (int block_num)
{
    char *ptr;
    char *end_ptr;
    unsigned size;

// More core computes the block size from the block number and determines
// a good chunk allocation size. This size is then requested to be allocated
// by sbrk and then divided into chunks and linked together.

    if ( block_num < 8 )
	size = BLOCKSIZE;
    else
	size = 1 << (block_num + 3);

    ptr = (char*)sbrk (size);
    if (ptr == 0)
	return;


// save new header and link the blocks together

    free_list[block_num] = (MallocList*) ptr;

    end_ptr = ptr + size;
    size = 1 << (block_num + 3);
    end_ptr -= size;
    while (ptr < end_ptr) {
	((MallocList*)ptr)->flag = FREED;
	((MallocList*)ptr)->block_num = block_num;
	CHAIN ((MallocList*)ptr) = (MallocList*) (ptr + size);
	ptr += size;
    }
    ((MallocList*)ptr)->flag = FREED;
    ((MallocList*)ptr)->block_num = block_num;
    CHAIN (((MallocList*) ptr)) = 0;
}

//-----------------------------------------------------
// MemorySegment::malloc
//
// Memory allocator
//-----------------------------------------------------

void *MemorySegment::malloc (unsigned size)
{
    MallocList* ptr;
    unsigned real_size;
    int block_num = 0;
    unsigned blockshift;
    
    if (!mem_flag) {
// Malloc determines the block pool number the chunk size and grabs a chunk
// from that block list.

	real_size = (size + HEADERSIZE + 7) & ~7;

	blockshift = (real_size - 1) >> 2;
	while (blockshift >>= 1)
	    block_num++;

	if (free_list[block_num] == 0)
	    morecore (block_num);

	if ((ptr = free_list[block_num]) == 0) {
	    return (void *)0;
	}

	free_list[block_num] = CHAIN (ptr);

	if (ptr->flag != FREED || ptr->block_num != block_num) {
	    msg("$1: memory already allocated", normal_sev) << wrn_tag << eom;
#ifndef _WIN32
	    SHARED_DEBUG {
                msg("$1: attach pid $2", normal_sev) << dbg_tag << eoarg << getpid () << eom;
                while (1)
                    sleep (10);
            }
#endif
	    return  (void*)0;
	}

	ptr->flag = ALLOCATED;
	ptr->size = size;
    } else {
	real_size = (size + HEADERSIZE + 7) & ~7;

	blockshift = (real_size - 1) >> 2;
	while (blockshift >>= 1)
	    block_num++;

	if (free_list[block_num] == 0)
	    morecore (block_num);

	if ((ptr = free_list[block_num]) == 0) {
	    return (void *)0;
	}

	free_list[block_num] = CHAIN (ptr);

	if (ptr->flag != FREED || ptr->block_num != block_num) {
	    msg("$1: memory already allocated", normal_sev) << wrn_tag << eom;
#ifndef _WIN32
            SHARED_DEBUG {
 		msg("$1: attach pid $2", normal_sev) << dbg_tag << eoarg << getpid () << eom;
		while (1)
		    sleep (10);
	    }
#endif
	    return  (void*)0;
	}

	ptr->flag = ALLOCATED;
	ptr->size = size;
    }
    return (void *)(&ptr[1]);
}

//-----------------------------------------------------
// MemorySegment::free
//
// Memory deallocator
//-----------------------------------------------------

void MemorySegment::free (void *mem)
{
    MallocList* list_head;
    int block_num;

    if (!mem_flag) {

// This function puts the memory onto the free list for the corresponding
// chunk size

	if ( mem == (void *)0 ) {
	    return;
	}

	list_head = &((MallocList*)mem)[-1];

	if (list_head->flag == ALLOCATED) {
	    block_num = list_head->block_num;
    
	    list_head->flag = FREED;
	    CHAIN (list_head) = free_list[block_num];
	    free_list[block_num] = list_head;
	} else {
	    msg("$1: memory already freed", normal_sev) << wrn_tag << eom;
#ifndef _WIN32
	    SHARED_DEBUG {
		msg("$1: attach pid $2", normal_sev) << dbg_tag << eoarg << getpid () << eom;
		while (1)
		    sleep (10);
	    }
#endif
	}

    } else {
	if ( mem == (void *)0 )
	    return;

	list_head = &((MallocList*)mem)[-1];

	if (list_head->flag == ALLOCATED) {
	    block_num = list_head->block_num;
    
	    list_head->flag = FREED;
	    CHAIN (list_head) = free_list[block_num];
	    free_list[block_num] = list_head;
	} else {
	    msg("$1: memory already freed", normal_sev) << wrn_tag << eom;
#ifndef _WIN32
	    SHARED_DEBUG {
		msg("$1: attach pid $2", normal_sev) << dbg_tag << eoarg << getpid () << eom;
		while (1)
		    sleep (10);
	    }
#endif
	}
    }
}

//-----------------------------------------------------
// MemorySegment::realloc
//
// Memory reallocator
//-----------------------------------------------------

void *MemorySegment::realloc (void *mem, unsigned new_size)
{
    MallocList *list_head;
    void *new_mem;
    unsigned num_bytes;

// This function performs the classical reallocation function for a 
// block of memory

    if (mem == 0)
        return malloc (new_size);

    list_head = &((MallocList*)mem)[-1];

    ASSERT (list_head->flag == ALLOCATED);

    if (list_head->block_num >= 13)
        num_bytes = (1 << (list_head->block_num + 3)) - HEADERSIZE;
    else
        num_bytes = list_head->size;

    if (new_size < num_bytes)
        num_bytes = new_size;

// For now, reallocate new space, so the memory will actually be freed if the
// memory size gets smaller

    if ((new_mem = malloc (new_size)) == 0)
        return 0;

    if (new_size < num_bytes)
        OSapi_bcopy (mem, new_mem, new_size);
    else
        OSapi_bcopy (mem, new_mem, num_bytes);
    free (mem);

    return new_mem;
}

int MemorySegment::check_fndecl (void* ptr)
{
    int retval = 0;
    char* start = (char*)this;
    char* cur = (char*)ptr; 
    int offset = cur - start;
    if (offset > 0 && offset < seg_used)
	retval = 1;
    return retval;
}

void MemorySegment::check_seg_size ()
{
}

void MemorySegment::set_ast_tree_ptr (void* p)
{
    ast_tree_ptr = p;
}

void* MemorySegment::get_ast_tree_ptr ()
{
    return ast_tree_ptr;
}

void MemorySegment::set_cpp_tree_ptr (void* p)
{
    cpp_tree_ptr = p;
}

void* MemorySegment::get_cpp_tree_ptr ()
{
    return cpp_tree_ptr;
}

void MemorySegment::set_esql_tree_ptr(void* p) {
    esql_tree_ptr = p;
}

void* MemorySegment::get_esql_tree_ptr() {
    return esql_tree_ptr;
}

struct ast_section* MemorySegment::get_section (char* nm)
{
    int found = 0;

    struct ast_section *p;
    for (p = ast_section; p; p = p->next) {
	found = !strcmp (p->name, nm);
 	if (found)
	    break;
    }

    if (!found) {
	p = (struct ast_section*) malloc (sizeof (struct ast_section));
	p->name = (char*) malloc (strlen (nm) + 1);
	strcpy (p->name, nm);
	p->next = ast_section;
	p->ast_tree_ptr = 0;
	ast_section = p;
    }

    ast_tree_ptr = p->ast_tree_ptr;

    return p;
}

/*
$Log: MemSeg.h.cxx  $
Revision 1.35 2003/05/20 09:28:04EDT Sudha Kallem (sudha) 
Using msync to flush out memory contents to physical file on 
HP so that shared memory starting address is visible to emacs.
Fixing emacs error on HP:
***DISCOVER<->Emacs Shared memory is not installed.
**Contact DISCOVER technical support, please.
Revision 1.34 2000/07/12 18:11:16EDT ktrans 
merge from stream_message branch
Revision 1.2.1.34  1994/08/05  20:31:19  aharlap
changed size MemorySegment::name

Revision 1.2.1.33  1994/08/04  18:06:13  davea
Fix for sparc 2 platform

Revision 1.2.1.32  1994/08/03  22:12:06  builder
FIx HP region overlap problems

Revision 1.2.1.31  1994/07/11  22:19:33  aharlap
multiply ast

Revision 1.2.1.30  1994/07/09  00:24:20  aharlap
multiple ast

Revision 1.2.1.29  1994/04/29  01:01:19  aharlap
Mixed Mode Cleanup

Revision 1.2.1.28  1994/03/04  19:32:47  builder
Port

Revision 1.2.1.27  1994/03/04  19:29:59  azaparov
Bug track: N/A
Port

Revision 1.2.1.26  1994/03/04  15:31:57  aharlap
commented out extra call to mmap

Revision 1.2.1.25  1994/03/02  14:55:40  builder
Port

Revision 1.2.1.24  1993/10/15  15:29:11  aharlap
Bug track: # 4987
More messages

Revision 1.2.1.23  1993/10/14  18:01:37  aharlap
Bug track: # 4987
bzero free_list

Revision 1.2.1.22  1993/10/07  21:52:52  builder
Port

Revision 1.2.1.21  1993/07/22  16:58:51  harry
HP/Solaris Port

Revision 1.2.1.20  1993/07/16  15:47:52  aharlap
close memseg in constructor

Revision 1.2.1.19  1993/06/26  21:57:53  builder
avoid using global constructor/destructors

Revision 1.2.1.18  1993/06/25  22:24:11  aharlap
added checking before memory map and truncating if necessary

Revision 1.2.1.17  1993/06/12  22:03:21  aharlap
redirect messages

Revision 1.2.1.16  1993/06/10  08:51:54  aharlap
bug #3559

Revision 1.2.1.15  1993/06/02  19:53:46  aharlap
fixed bug in get_section

Revision 1.2.1.14  1993/05/27  18:45:02  glenn
Rename SHARED_MEMORY_DEBUG to PSET_DEBUG_SHMEM.

Revision 1.2.1.13  1993/05/27  18:38:05  glenn
Emulate system tempnam() function when building name of
temporary memseg file.
Rename ASET_USER_DEFINED_SEGFILE to PSET_DEBUG_SEGFILE.

Revision 1.2.1.12  1993/05/21  16:48:31  aharlap
*** empty log message ***

Revision 1.2.1.11  1993/05/11  22:59:22  smit
testing

Revision 1.2.1.10  1993/04/29  22:27:32  aharlap
increment memseg by 8M

Revision 1.2.1.9  1993/04/29  17:24:32  smit
24 meg in slave aset goes over to stack space and then dies.

Revision 1.2.1.8  1993/04/28  16:52:04  aharlap
optimized using semaphores

Revision 1.2.1.7  1993/04/23  17:34:54  davea
bug 3482 - activated prototypes for open, lseek, etc.

Revision 1.2.1.6  1993/04/23  02:49:52  aharlap
added debugging stuff

Revision 1.2.1.5  1993/03/01  16:39:07  aharlap
call inform_remap from check_seg_size ()
/

Revision 1.2.1.4  1992/11/05  21:41:09  so
If a file could not be open or created, display the error message and exit the program.

Revision 1.2.1.3  1992/10/10  17:35:56  builder
fixed rcs myheader

Revision 1.2.1.2  92/10/09  17:30:54  builder
fixed rcs myheader

Revision 1.2.1.1  92/10/07  21:24:56  smit
*** empty log message ***

Revision 1.2  92/10/07  21:24:54  smit
*** empty log message ***

Revision 1.1  92/10/07  18:24:38  smit
Initial revision

//Revision 1.24  92/10/01  23:19:29  builder
//added cpp_tree_ptr
//
//Revision 1.23  92/09/22  18:58:47  aharlap
//moved pmgr_is_slave from pManager.h.C
//
//Revision 1.22  92/09/20  21:32:02  builder
//removed customize
//
//Revision 1.21  92/09/20  10:46:23  oak
//Changed to use customize class.
//
//Revision 1.20  92/09/10  12:46:16  oak
//Changed USER_DEFINED_SEGFILE to ASET_USER_DEFINED_SETFILE.
//
//Revision 1.19  92/07/11  17:56:38  builder
//restored stuff with old_size
//
//Revision 1.18  92/06/26  09:15:57  aharlap
//changed stream.h --> stdio.h
//
//Revision 1.17  92/06/19  04:25:53  aharlap
//added method get_section
//
//Revision 1.16  92/06/18  11:07:17  builder
//Fixed compilation problems.
//
//Revision 1.15  92/06/18  10:59:51  aharlap
// *** empty log message ***
//
//Revision 1.14  92/06/18  10:32:42  aharlap
//modified check_seg_size function
//
//Revision 1.13  92/06/18  10:21:33  aharlap
//added variable __START_SHARED_MEMORY
//
//Revision 1.12  92/05/18  10:48:17  mfurman
//cleaning "this" before exit inserted in ~MemorySegment" to prevent calling "delete"
//
//Revision 1.11  92/05/06  03:30:21  aharlap
//changed using fixed address to this in sbrk and check_seg_size
//
//Revision 1.10  92/05/05  22:54:21  aharlap
//Defined START_SHARED_MEMORY
//
//Revision 1.9  92/05/02  17:24:58  aharlap
//Added using user defined segfile if environment variable USER_DEFINED)_SEGFILE
// is installed
//
//Revision 1.8  92/04/21  23:28:37  aharlap
//Added parser management
//
//Revision 1.6  91/12/19  18:21:17  aharlap
//ifdef for GNU
//
//Revision 1.5  91/12/05  10:19:36  kws
//Fix compiling problems for g++
//
//Revision 1.4  91/11/20  20:31:31  kws
//Change mapping address and remap file after sbrk
//
//Revision 1.3  91/11/18  20:12:47  builder
//Implement sbrk for mapped files
//
//Revision 1.2  91/11/12  15:47:23  kws
//Access master semaphore before munging shared memory data structures during
//alloc/free
//
//Revision 1.1  91/11/07  20:05:02  kws
//Initial revision
//
*/

//////////////////////////////////



