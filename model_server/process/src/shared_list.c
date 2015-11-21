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
/***************************************************************************
**  This file exports three functions:
**    1. build_so_lib_list()  - creates ordered list of shared libraries,
**                              which used by given executable   
**    2. get_so_lib_list()    - returns pointer to this list.
**    3. get_addr_so_lib_f()  - returns address of function from shared
**                              libraries
**    ---------------------------------------------------------------
**    4. build_so_user_list() - creates ordered list of user-defined shared
**                              objects
**    5. get_so_user_list()   - returns pointer to this list.
**    6. get_addr_so_user_f() - returns address of function from user-
**                              defined shared objects
***************************************************************************/

#include <sys/types.h>

struct so_lib_list {
   void               *map_addr;    /* address at which object mapped */
   char               *name;
   struct so_lib_list *next;
   void               *handle;
};

static struct so_lib_list *so_list_head = 0;
static struct so_lib_list *so_list_user_head = 0;

void build_so_lib_list()
{
}

void build_so_user_list()
{
   /* yet not implemented */
}

struct so_lib_list *get_so_lib_list()
{
  return so_list_head;
}

struct so_lib_list *get_so_lib_user_list()
{
  return so_list_user_head;
}

void *get_addr_so_lib_f(name)
   char *name;
{
   return 0;
}

void *get_addr_so_user_f(name)
   char *name;
{
}

void close_shared (ptr)
    struct so_lib_list *ptr; 
{
}

void close_shared_list ()
{
  close_shared (so_list_head);
}

