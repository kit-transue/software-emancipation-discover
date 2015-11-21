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
#ifndef _objDb_h
#define _objDb_h

//
// -- Contains objSet  Class Definition
//

#include "general.h"
#include "Object.h"
#include "dis_iosfwd.h"

typedef unsigned int uint;
class Obj;

#define MAX_NAME_LEN 32


// extern database *db1;


//---- Objectivity:  #define db_new(x, y)  ooNew( x, y, 0)
//---- ObectStore:   #define db_new(x, y)  (x *) (new(db1) x y)->put_id(1)


#define db_new(x,y)     new x y
#define ObjId           int
#define db_persistent   get_id


extern Obj        *obj_get_by_name(const char *);
extern void       obj_set_name(const char *, Obj&);
extern void       obj_rm_name(const char *);

// Macros for every database class should be defined here.
#define DefinePtr(N) class N; typedef N * paste(N,Ptr)

// validate (compare) two signatures ; 0=OK, 1=warn, 2=fail

typedef int (* validation_function)(const char * new_val, const char* old_val);
                                      
//----------------   Obj Class (abstract - top of the tree) ----------------
 
        DefinePtr (Obj);

        class Obj : public Object
        {
         private:
               int   obj_id : 24; // internal DB object ID
               unsigned kind : 8; // must be zero to differenciate from symbol

               virtual int compare(const Object& x) const;
        public:                                                       
            virtual bool isEqual(const Object& x) const; // compares pointers
            virtual unsigned hash() const;    // return (unsigned) this

               Obj ();
               Obj (const Obj &x);
               virtual ~Obj();
               int  get_id() const {
		  int id =  obj_id; 
		  if (id & 0x800000) id |= 0xff000000; return id;
		}

               void put_id(int id) {obj_id = id;}

               bool operator==(const Obj& ) const;

               virtual bool relationalp() const;
               virtual int  collectionp() const;
               virtual unsigned  size() const;
               virtual int includes(const Obj*) const;
// validation protocol
	       virtual void put_signature(ostream& os) const;
	       virtual validation_function  val_signature() const;
	       virtual void  print(ostream&, int level = 0) const;
               virtual const char * objname() const;
	     };


#endif // _objDb_h


