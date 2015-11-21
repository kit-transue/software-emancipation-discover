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
#ifndef __objRTL_h
#define __objRTL_h
//
// objRTL.h
//
// Run-time list of objects; subclass of objArr
//
// Contains interface for C.
//
// History:
//   07/29/91    F. Barrett    Original
//

#ifdef __cplusplus

#ifndef _objRelation_h
#include <objRelation.h>
#endif
#ifndef _objString_h      
#include <objString.h>
#endif
#ifndef _objDb_h      
#include <objDb.h>
#endif
#ifndef _objCollection_h      
#include <objCollection.h>
#endif      
#ifndef _objArr_h
#include <objArr.h>
#endif      

RelClass(objRTL);

class objRTL : public objArr
{
 private:
    objString name;

 public:
    objRTL(const objString& nm);
    objRTL(char* nm);
    objRTL(objRTL& rtl);
    ~objRTL();

    void insert(Obj*);
    void clear();
    virtual char *get_name () const {return name;}
    virtual void sort();
};

inline objRTL::objRTL(const objString& nm) : name(nm)
{
    obj_set_name( (char*)nm, *this);
}

inline objRTL::objRTL(char* nm) : name(nm)
{
    obj_set_name( nm, *this);
}

inline objRTL::~objRTL()
{
    obj_rm_name( name );
}

inline void objRTL::insert(Obj* x)
{
    insert_last(x);
};

typedef objRTL* obj_rtl;

extern "C" {
obj_rtl obj_rtl_new       ( char* name_of_list );
int     obj_rtl_is_empty  ( obj_rtl            );
void    obj_rtl_insert    ( obj_rtl, char*     );
void    obj_rtl_insert_all( obj_rtl, obj_rtl   );
void    obj_rtl_remove    ( obj_rtl, char*     );
void    obj_rtl_clear     ( obj_rtl            );
char*   obj_rtl_get_name  ( obj_rtl            );
void    obj_rtl_sort      ( obj_rtl            );
void    obj_rtl_free      ( obj_rtl            );
};

#endif


#ifndef __cplusplus

/* C interface to above class */

typedef void* obj_rtl;

extern obj_rtl obj_rtl_new       ( /* char* name_of_list */ ):
extern int     obj_rtl_is_empty  ( /* obj_rtl            */ );
extern void    obj_rtl_insert    ( /* obj_rtl, char*     */ );
extern void    obj_rtl_insert_all( /* obj_rtl, obj_rtl   */ );
extern void    obj_rtl_remove    ( /* obj_rtl, char*     */ );
extern void    obj_rtl_clear     ( /* obj_rtl            */ );
extern char*   obj_rtl_get_name  ( /* obj_rtl            */ );
extern void    obj_rtl_sort      ( /* obj_rtl            */ );
extern void    obj_rtl_free      ( /* obj_rtl            */ );

#endif


#endif

