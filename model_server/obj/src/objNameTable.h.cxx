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
///////////////////////////   FILE objNameTable.h.C  ////////////////////////
//
// -- NameTable class methods.
//
#include "genError.h"
#include "objCollection.h"
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
class objSet;
extern objSet *nodb_name_tbl;


class NameTable : public Obj
{
   public:
      char name[MAX_NAME_LEN];
      Obj  *obj_ptr;

      NameTable(const char *nn, Obj *oo) {strcpy(name,nn); 
                                        obj_ptr = oo; }
private:
    NameTable(const NameTable& r); // { *this = r; }
};


// - temporary naming calls - without using Database

      objSet  *nodb_name_tbl = new objSet();

      void  obj_set_name(const char *name, Obj &ob)
      {

        Initialize(obj_set_name);
        NameTable  *new_el = new NameTable(name, &ob);
        nodb_name_tbl->insert(new_el);

        Return
      }

      Obj  *obj_get_by_name(const char *name)
      {
        Obj *obj_ptr = NULL;
        Obj *el;

        Initialize(obj_get_by_name);
        ForEach(el,*nodb_name_tbl)
           if (strcmp(name, ((NameTable *) el)->name) == 0 ){
               obj_ptr = ((NameTable *) el)->obj_ptr;
               break;
           }
        ReturnValue(obj_ptr);
      }


      void  obj_rm_name(const char *name)
      {
        Obj *el;

        Initialize(obj_rm_name);

        if (nodb_name_tbl == NULL)
           Error(ERR_INPUT);

        ForEach(el,*nodb_name_tbl)
           if (strcmp(name, ((NameTable *) el)->name) == 0 ){
               nodb_name_tbl->remove((NameTable *) el);
               break;
           }
        Return
      }

/*
   START-LOG-------------------------------------------

   $Log: objNameTable.h.cxx  $
   Revision 1.3 2000/07/10 23:07:19EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.2  1992/10/09  18:55:20  boris
Fix comments

   History:   08/13/91     S.Spivakovksy      Initial coding.
              09/03/91     S.Spivakovksy      More NIH related changes.
  

   END-LOG---------------------------------------------

*/

