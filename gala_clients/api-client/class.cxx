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

//#include <class.h>
extern "C" void DI_object_create(int*);
extern "C" void DI_object_copy(int, int);
#ifndef PSET_SERVER
#include <stdlib.h>
#define OS_dependent
#define psetfree free
#define psetmalloc malloc
#define psetrealloc realloc
#endif

// static genArr(int) objects;
static struct {
  int * ptr;
  int capacity;
  int length;
}  objects = {0,0,0};

static int pop()
{
  if(objects.length ==0)
     return -1;
  objects.length --;
  return objects.ptr[objects.length];
}

static void push(int id)
{
  if(objects.capacity == objects.length){
    if(objects.capacity == 0){
       objects.capacity = 4;
       objects.ptr = (int*) psetmalloc(objects.capacity*sizeof(int));
    } else {
       objects.capacity *= 2;     
       objects.ptr = (int*) psetrealloc(objects.ptr, objects.capacity*sizeof(int));
    }
  }
  objects.ptr[objects.length] = id;
  objects.length++;
}
extern "C" int api_object_create()
{
  int sz = objects.length;
  int id = -1;
  if(sz > 0){
     id = pop();
  } else {
     DI_object_create(&id);
  }
  return id;
}
extern "C" void api_object_delete(int id)
{
  if(id >= 0)
      push(id);
}
char api_name_buffer[1024];

extern "C" int api_process_query(int id, void (*fun)(int, int))
{
  int new_id = api_object_create();
  (*fun)(id, new_id);
  return new_id;
}

extern "C" int api_process_setop(int id1, int id2, void (*fun)(int, int))
{
  int new_id = api_object_create();
  DI_object_copy(id2, new_id);
  (*fun)(id1, new_id);
  return new_id;
}
