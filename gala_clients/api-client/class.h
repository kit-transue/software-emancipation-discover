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
#include <stdio.h>
extern "C" {

 typedef char *charp; 
 typedef int DI_object; 
 typedef int * intp; 
 typedef DI_object * DI_objectp;
 int DI_connect(char *,char *); 

 extern void (*api_error_handler)();
  int api_object_create();
  void api_object_delete(int);
  int  api_process_query(int, void(*foperatorun)(int,int));
  int  api_process_setop(int, int, void(*fun)(int,int));
#include "SPEC"

};

#define pto DI_temp_obj

extern char api_name_buffer[];

class pto   {
// return value of queries

 protected:
  DI_object id;

  int allocated()   { return id >= 0;}
  void create()     { id = api_object_create();}
  void destroy()    { api_object_delete(id);}
  void copy(pto&oo) { if(!allocated()) create(); DI_object_copy(oo.id, id);}
  pto() { create();}
  pto(int oo)  : id(oo) {}
  pto(pto&oo) {id = oo.id; oo.id = -1;}
  pto(pto&oo, int) { create(); DI_object_copy(oo.id, id);}
 ~pto() { destroy();}
 
public:

 int   size() {int ii; DI_object_size(id, &ii); return ii;}
 char* name() {DI_object_name(id, api_name_buffer); return api_name_buffer;}

  void operator += (pto& oo)
    { DI_array_union(oo.id, id);}

  pto operator + (pto& oo)
    { return api_process_setop(id, oo.id, DI_array_union);}

  void operator -= (pto& oo)
    { DI_array_subtract(oo.id, id);}

  pto operator - (pto& oo)
    { return api_process_setop(id, oo.id, DI_array_subtract);}

  void operator *= (pto& oo)
    { DI_array_intersect(oo.id, id);}

  pto operator * (pto& oo)
    { return api_process_setop(id, oo.id, DI_array_intersect);}

  pto  operator[](int position)
    { pto tmp; DI_array_get(id, position, tmp.id); return tmp;}

#include "browser_members.i"
};

class DI_obj : public pto {
 public:
  DI_obj() : pto(-1) {}
  DI_obj(int kind, char*name) { DI_object_lookup(id, kind, name, 0);}
  DI_obj(DI_obj&oo) : pto(oo, 1) {}
  DI_obj(pto&oo) : pto(oo)      {}
  DI_obj& operator = (pto&oo) {copy(oo); return *this;}
};


