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
// file map_manager.C
#include <cLibraryFunctions.h>
#include <machdep.h>
#ifndef _WIN32
#include <sys/mman.h>
#else // _WIN32
#include <windows.h>
#include <iostream>	// for cerr
using namespace std;
#endif

static char const *map_debug;
static FILE *map_out = NULL;

int pset_munmap (caddr_t addr, int len);
#ifdef _WIN32

caddr_t pset_mmap (char const *title, caddr_t addr, int len, int prot, int share, HANDLE fd, off_t)
{
    if (!map_debug) {
	char* tmp = (char *)OSapi_getenv ("PSET_MAP_FILE");
	map_debug = tmp ? tmp : (char*)0x1;
    }
    if (addr)
	pset_munmap(addr,len);
    HANDLE file_mapping = CreateFileMapping(fd, NULL, prot, 0, len, NULL);
    if(file_mapping == NULL )  {
        OSapi_printf( "WARNING : Can't reserve requested disk space. Will use all available\n");
		len/=2;
	    file_mapping = CreateFileMapping(fd, NULL, prot, 0, len, NULL);
        if( file_mapping==NULL) { 
           len/=2; 
	       file_mapping = CreateFileMapping(fd, NULL, prot, 0, len, NULL);
           if( file_mapping==NULL) { 
              long lErr = GetLastError();
              OSapi_printf( "File mapping failed. Win32 error: %d\n", lErr );
              OSapi_fflush(stdout);
	          return (caddr_t)-1;
		   }
		}
    }
    DWORD access = 0;
    if(prot == PAGE_READWRITE)
	access |= FILE_MAP_WRITE;
    if(prot == PAGE_READONLY)
	access |= FILE_MAP_READ;
    void *ret = 0;
    if(addr != 0)
	ret = MapViewOfFileEx(file_mapping, access, 0, 0, len, addr);
    else
	ret = MapViewOfFile(file_mapping, access, 0, 0, len);
    if(ret == NULL){
	DWORD error_code = GetLastError();
	ret              = (void *)-1;
    }
    CloseHandle(file_mapping);
    if (map_debug != (char*)0x1) {
	if (!map_out) {
	    char map_file_name[MAXPATHLEN + 10];
	    int pid = OSapi_getpid ();
	    OSapi_strcpy (map_file_name, map_debug);
	    OSapi_sprintf (map_file_name + OSapi_strlen (map_file_name), "%05d", pid);
	    map_out = OSapi_fopen (map_file_name, "w");
	    if (!map_out) {
		map_debug = NULL;
		return (caddr_t)ret;
	    }
	}
	if ((int)ret == 0)
	    OSapi_fprintf (map_out, "map   0x%08x 0x%08x %s %s\n", addr, addr+len, 
			   "-", title);
	else
	    OSapi_fprintf (map_out, "map   0x%08x 0x%08x %s %s\n", (caddr_t)ret, (caddr_t)ret+len, 
		     "+", title);
	OSapi_fflush (map_out);
    }
    return (caddr_t)ret;
}

extern "C" char * pset_mmap_c(char* title, char * addr, int len, int prot, int share, HANDLE fd, long offset)
{
    return pset_mmap(title, addr, len, prot, share, fd, offset);
}

#else 

caddr_t pset_mmap (char const *title, caddr_t addr, int len, int prot, int share, int fd, off_t off)
{
#if !defined(irix6)
    caddr_t ret = (caddr_t)mmap (addr, len, prot, share, fd, off);
#else
    void *ret = mmap ((void *)addr, (size_t)len, prot, share, fd, off);
#endif
    if (!map_debug) {
	char const *tmp = OSapi_getenv ("PSET_MAP_FILE");
	map_debug = tmp ? tmp : (char*)0x1;
    }
    if (map_debug != (char*)0x1) {
	if (!map_out) {
	    char map_file_name[MAXPATHLEN + 10];
	    int pid = OSapi_getpid ();
	    OSapi_strcpy (map_file_name, map_debug);
	    OSapi_sprintf (map_file_name + OSapi_strlen (map_file_name), "%05d", pid);
	    map_out = OSapi_fopen (map_file_name, "w");
	    if (!map_out) {
		map_debug = NULL;
		return (caddr_t)ret;
	    }
	}
	if ((int)ret == -1)
	    OSapi_fprintf (map_out, "map   0x%08x 0x%08x %s %s\n", addr, addr+len, 
		     "-", title);
	else
	    OSapi_fprintf (map_out, "map   0x%08x 0x%08x %s %s\n", (caddr_t)ret, (caddr_t)ret+len, 
		     "+", title);
	OSapi_fflush (map_out);
    }
    return (caddr_t)ret;
}
#endif


#ifdef _WIN32

int pset_munmap (caddr_t addr, int len)
{
    BOOL result = UnmapViewOfFile(addr);
    if (map_debug && map_debug != (char*)1){
	OSapi_fprintf (map_out, "unmap 0x%08x 0x%08x\n", addr, addr+len);
	OSapi_fflush (map_out);
    }
    return (result ? 0 : -1);
}

#else

int pset_munmap (caddr_t addr, int len)
{
    int ret = munmap (addr, len);
    if (map_debug && map_debug != (char*)1) {
	OSapi_fprintf (map_out, "unmap 0x%08x 0x%08x\n", addr, addr+len);
	OSapi_fflush (map_out);
    }
    return ret;
}

#endif


#ifdef _WIN32

int pset_mprotect (caddr_t addr, int len, int prot)
{
    cerr << "pset_mprotect not implemented" << endl;
    return -1;
}

#else

int pset_mprotect (caddr_t addr, int len, int prot)
{
    return mprotect (addr, len, prot);
}

#endif

/*
//-----------------------------------------
//$Log: map_manager.cxx  $
//Revision 1.11 2000/07/10 23:06:36EDT ktrans 
//mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.8  1994/02/08  20:23:26  builder
 * Port
 *
 * Revision 1.7  1993/10/12  18:55:25  kws
 * Port (unmap region before map)
 *
 * Revision 1.6  1993/09/30  19:16:07  builder
 * Port
 *
 * Revision 1.5  1993/09/30  17:55:15  kws
 * Port
 *
 * Revision 1.4  1993/07/20  18:27:51  harry
 * HP/Solaris Port
 *
 * Revision 1.3  1993/06/26  21:41:17  aharlap
 * avoid using global constructors/destructors
 *
 * Revision 1.2  1993/06/25  20:57:05  bakshi
 * mprotect decl for cfront 3.0.1
 *
 * Revision 1.1  1993/05/21  16:48:15  aharlap
 * Initial revision
 *
//Initial revision
//------------------------------------------
*/
