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
#ifndef _so_shared_h
#define _so_shared_h

#define RUNTIME_NAME_SUFFIX ".runtime"

#ifdef _WIN32
#define ALLOCATE(type, count) (type *)malloc(sizeof(type)*(count))
#define DISPOSE(ptr) { if (ptr) free(((char *)ptr)); }
#else
#define ALLOCATE(type, count) (type *)malloc(sizeof(type)*(count))
#define DISPOSE(ptr) { if (ptr) free((ptr)); }

#if defined(sun5) || defined(irix6)
#include <elf.h>
#if !defined(irix6)
#include <link.h>
#else
#include <sys/types.h>
#endif 

#define START_VIRT ((size_t)0x10000)
#define SLIB_HEADER Link_map
#define SLIB_SO_HEAD(header,objects,type) {\
    Elf32_Ehdr *ehdr=(Elf32_Ehdr *)START_VIRT; \
    Elf32_Phdr *phdr=(Elf32_Phdr *)(START_VIRT + (size_t)ehdr->e_phoff); \
    Elf32_Dyn *ptr=(Elf32_Dyn *)0; \
    struct r_debug *r=(struct r_debug *)0; \
    int i; \
    objects=(type)0; \
    for (i=0; i < (int)ehdr->e_phnum; i++) \
	if (phdr[i].p_type==PT_DYNAMIC) { \
	    ptr=(Elf32_Dyn *)phdr[i].p_vaddr; \
	    break; \
	} \
    for (;ptr && ptr->d_tag!=DT_NULL; ++ptr) \
	if (ptr->d_tag==DT_DEBUG) { \
	    r=(struct r_debug *)ptr->d_un.d_ptr; \
	    break; \
	} \
    if (r) objects=r->r_map; \
    if (objects) objects=objects->l_next; \
    header=objects; \
    }

#define SO_ID_TYPE Link_map
#define SO_MAPPED_ADDR(object) ((object)->l_addr)
#define SO_NAME(object) ((object)->l_name)
#define SO_NEXT(object) ((object)->l_next)

#define SO_TEXT_SIZE(header,object,text_size,type) {\
	   unsigned long curr_addr=(unsigned long)0; \
	   Link_map *cur; \
	   for(cur=header;cur;cur=cur->l_next) \
	       if(cur->l_addr>object->l_addr \
		  && (!curr_addr || curr_addr>cur->l_addr)) \
		  curr_addr=cur->l_addr; \
	   text_size=(type)(curr_addr-object->l_addr); \
	}

#endif /* sun5, irix6, irix6 */

#if defined(hp10)

#include <sys/types.h>
#include <shl.h>


#define SLIB_HEADER struct link_dynamic
#define SLIB_SO_HEAD(header, objects, type) { \
	    if (header) { \
               struct link_dynamic_1 * dyn_1 = (struct link_dynamic_1 *)0; \
               struct link_dynamic_2 * dyn_2 = (struct link_dynamic_2 *)0; \
	       if (header->ld_version < 2) { \
	          if (dyn_1 = header->ld_un.ld_1) \
		     objects = (type)dyn_1->ld_loaded; \
	       } else if (dyn_2 = header->ld_un.ld_2) \
		  objects = (type)dyn_2->ld_loaded; \
	      } }

#define SO_ID_TYPE struct link_map
#define SO_ID_MAP_TYPE long
#define SO_MAPPED_ADDR(object) (object->lm_addr)
#define SO_NAME(object) (object->lm_name)
#define SO_NEXT(object) (object->lm_next)
#define SO_TEXT_SIZE(header,object, text_size, type) {\
	   if(object && object->lm_ld) { \
           struct link_dynamic_1 * dyn_1 = (struct link_dynamic_1 *)0; \
           struct link_dynamic_2 * dyn_2 = (struct link_dynamic_2 *)0; \
	   if (object->lm_ld->ld_version < 2) { \
	      if (dyn_1 = object->lm_ld->ld_un.ld_1) \
		  text_size = (type)dyn_1->ld_text; \
	   } else if (dyn_2 = shared_objects->lm_ld->ld_un.ld_2) \
	   text_size = (type)dyn_2->ld_text; } }

#define SO_COMMON_ID_TYPE struct rtc_symb
#define SLIB_SO_COMMONS(header, commons) { if (header) { \
	   struct ld_debug * ldd = header->ldd; \
	   if (ldd) \
	      commons = ldd->ldd_cp; \
	   } }
#define SO_COMMON_SYMBOL(commons) (commons->rtc_sp)
#define SO_COMMON_NEXT(commons) (commons->rtc_next)

#endif /* hp10 */

extern int _DYNAMIC;

#ifdef _so_shared_h_C
//extern struct sharedLibrary * newSharedLibrary();
    extern struct sharedLibrary * disposeSharedLibrary();
    
//    extern int collectSharedLibraries();
    extern int examineSharedLibraries();
//    extern int refreshSharedLibraryInfo();
#endif
    
#endif /* _WINNT */

struct sharedLibrary {
    void * mapped_addr;
    long   text_size;
    char * name;
    void * symtab_manager;
    struct sharedLibrary * next;
    struct sharedLibrary * prev;
};

struct sharedLibraryInfo {
    void * dynamic;
    struct sharedLibrary * current_list;
    struct sharedLibrary * new_list;
    struct sharedLibrary * disposed_list;
    struct sharedLibrary * commons;
    int  list_compare_status;
    int  recache_status;
    char * executable;
    int force_loading;
};

#endif

/*
   START-LOG-------------------------------------------
 
   $Log: so_shared.h  $
   Revision 1.14 2000/07/07 08:12:27EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.3  1994/04/13  19:37:05  builder
 * Port
 *
 * Revision 1.2  1994/03/22  14:18:00  pero
 * support for COMMON symbols (runtime resolve)
 *
 * Revision 1.1  1994/03/17  00:07:46  pero
 * Initial revision
 *
 
   END-LOG---------------------------------------------
 
*/
