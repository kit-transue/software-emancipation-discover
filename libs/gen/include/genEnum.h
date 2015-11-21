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
#ifdef genENUM_SRC

#undef genENUM
#undef Etok
#undef Etok_val
#undef genENUM_prt

#define genENUM(x) genEnum_item paste(x,_names)[] =
#define Etok(x) genEnum_item(x, quote(x))
#define Etok_val(x,y) Etok(x)

#ifdef __cplusplus

#define genENUM_prt(x) \
extern const char *genEnum_get_name (genEnum_item *arr, int, int);\
extern int genEnum_get_item (genEnum_item *arr, int, const char*);\
extern int genEnum_print (genEnum_item *arr, int, ostream&);\
static int paste(x,_size) = sizeof( paste(x,_names)) / sizeof(genEnum_item);\
psetCONST char * paste(x,_get_name)(int idx) \
{\
   genEnum_item *arr_ptr = (genEnum_item *) paste(x,_names);\
   return genEnum_get_name(arr_ptr, paste(x,_size), idx);\
}\
extern "C" x paste(x,_get_item)(psetCONST char * name) \
{\
   genEnum_item *arr_ptr = (genEnum_item *) paste(x,_names);\
   return (x) genEnum_get_item(arr_ptr, paste(x,_size), name);\
}\
int paste(x,_print)(ostream& ostr) \
{\
   genEnum_item *arr_ptr = (genEnum_item *) paste(x,_names);\
   return genEnum_print(arr_ptr, paste(x,_size), ostr);\
}\
extern "C" int paste(x,_prt)() \
{\
   genEnum_item *arr_ptr = (genEnum_item *) paste(x,_names);\
   return genEnum_print(arr_ptr, paste(x,_size), *(ostream*)0);\
}\
void paste(x,_name_data)(genEnum_item** arr_ptr, int*sz) \
{\
   *arr_ptr = (genEnum_item *) paste(x,_names);\
   *sz = paste(x,_size);\
}
#else
#define genENUM_prt(x) extern const char *genEnum_get_name (genEnum_item *arr,\
 int, int);\
static int paste(x,_size) = sizeof( paste(x,_names)) / sizeof(genEnum_item);\
psetCONST char * paste(x,_get_name)(int idx) {\
   genEnum_item *arr_ptr = (genEnum_item *) paste(x,_names);\
   return genEnum_get_name(arr_ptr, paste(x,_size), idx);\
}
#endif

#else    /*  genENUM_SRC  */

#ifndef _genEnum_h
#define _genEnum_h

#ifndef _general_h
#include <general.h>
#endif

#include <dis_iosfwd.h>

/*
   genEnum.h - file to include in header/source files, defines macros:
               genENUM, Etok, Etok_val, genENUM_prt
  
   For instance in header file my.h:
  
   #include <genEnum.h>
   genENUM(myEnum){
      Etok_val(mySTART,-5),
      Etok_val(mySTART1, 0),
      Etok(myNEXT1),
      Etok(myNEXT2),
      Etok(myNEXT3),
      Etok_val(myEnd,99)
   };
   genENUM_prt(myEnum);
  
  
   in source file my.C:
  
   #include <my.h>
   #define genENUM_SRC   - switch to source generation
   #undef _my_h          - switch to include my.h second time
   #include <my.h>
   -----------------------------------
   -----------------------------------
   genENUM_SRC  - variable to be defined only once in a source file to create name array
   genEnum_item - structure {int ind, char *ind_name}; 
   genENUM      - macro to start enum
   Etok         - macro to define not named enum token
   Etok_val     - nacro to define named enum token
   genENUM_prt  - macro to declare function that returns item name(actual name is "paste(x,_name)",
                  that is in our example : char *myEnum_name(int n);
  
   In the source file if genENUM_SRC is defined, for enumeration myEnum:
  
   static genEnum myEnum_names[] - enum name array;
   static int myEnum_size        - enum name array size;
   const char *myEnum_get_name (int n) { ... } - returns the name or NULL if index n is not found;
  
   Generic function defined in genEnum.h.C:
        const char *genEnum_get_name(genEnum_item *arr_ptr, int size, int idx);
        The user can put a breakpoint in to debug his/her generated 
        paste(x,_get_name) PROTO((int)) function;
  ==========================================================================================
*/

#ifndef _general_h
#include <general.h>
#endif
#ifndef _prototypes_h
#include <prototypes.h>
#endif

#ifdef __cplusplus
struct genEnum_item {
    int idx;
    const char *idx_name;
    genEnum_item(int i, const char*n) : idx(i), idx_name(n) {}
};
#else
struct genEnum_item {
    int idx;
    const char *idx_name;
};
#endif

#define genENUM(x) enum x
#define Etok(x) x
#define Etok_val(x,y) x=y
#ifdef __cplusplus
#define genENUM_prt(x) \
  extern psetCONST char *paste(x,_get_name) PROTO((int));\
  extern "C" x paste(x,_get_name) PROTO((psetCONST char *));\
  extern  int paste(x,_print) PROTO((ostream&));\
  extern void paste(x,_name_data)(genEnum_item** arr_ptr, int*sz); \
  extern "C" int paste(x,_prt)();
#else
#define genENUM_prt(x) extern psetCONST char *paste(x,_get_name) PROTO((int));
#endif

#endif   /*  _genEnum_h   */

#endif   /*  genEnum_SRC  */ 

/*
    $log: $
*/
