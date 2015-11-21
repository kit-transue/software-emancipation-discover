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
#ifndef __msgSet_h
#define __msgSet_h


#ifndef _objDb_h
#include "objDb.h"
#endif

#ifndef _objSet_h
#include "objSet.h"
#endif

class msgObj : public Obj {

 public:
        bool operator==(const msgObj&) const;
        bool operator!=(const msgObj& a) const { return !(*this==a); }
        int compare(const Object&) const;
        bool isEqual (const Object& a) const;

private:
   int msg;                           // message id
   void *data;                        // message static data
   void (*fn) (void *, void *, void *); // message handler

public:
   msgObj() {};
 private:
   msgObj(const msgObj& r); // : Obj (r)
//     {
//       msg = r.msg;
//       data = r.data;
//       fn = r.fn;
//     }
 public:
   msgObj(int msg, void *data, void (*fn) (void *, void *, void *));
   ~msgObj() {};
   unsigned hash () const;

   void call_handler(void *object, void *dynamic_data);
};

class msgSet : public objSet {
public:
   msgSet() {};
   ~msgSet();
   void add_message (int msg, void *data, void (*fn) (void *, void *, void *));
   void send (int msg, void *object, void *data);
};

/*
   START-LOG-------------------------------------------

   $Log: msgSet.h  $
   Revision 1.1 1993/10/05 09:33:20EDT builder 
   made from unix file
 * Revision 1.2.1.3  1993/10/05  13:31:28  builder
 * Reduce number of include files
 *
 * Revision 1.2.1.2  1992/10/09  18:53:10  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/
#endif
