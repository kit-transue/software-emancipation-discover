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
#include "msgSet.h"
#include "general.h"
#include "objCollection.h"

bool msgObj::operator==(const msgObj& a) const
{
   return this->msg == a.msg;
}

int msgObj::compare(const Object& ) const
{

   return 1;
}

bool msgObj::isEqual (const Object& a) const
{
    return (this->msg == ((msgObj&)a).msg) ;
}

msgObj::msgObj(int msg, void *data, void (*fn) (void *, void *, void *))
{
   this->msg = msg;
   this->data = data;
   this->fn = fn;
}

unsigned msgObj::hash() const
{
   return (unsigned) this->msg;
}

void msgObj::call_handler (void *object, void *dynamic_data)
{
   this->fn (object, this->data, dynamic_data);
}

msgSet::~msgSet()
{
   purge();
}

void msgSet::add_message (int msg, void *data, void (*fn) (void *, void *, void *))
{
   msgObj *new_msg;
   msgObj *msgobj;
   
   new_msg = new msgObj (msg, data, fn);

   // Delete the message if already in the set
   if ( (msgobj = (msgObj *) os_list.findObjectWithKey (*new_msg)) != Object::nil ) {
      remove(msgobj);
      delete msgobj;
   }
   
   insert (new_msg);
}

void msgSet::send (int msg, void *object, void *data)
{
   msgObj msgkey(msg, NULL, NULL);
   msgObj *msgobj;

   if ( (msgobj =  (msgObj *) os_list.findObjectWithKey (msgkey)) != Object::nil )
	msgobj->call_handler (object, data);
}

/*
   START-LOG-------------------------------------------

   $Log: msgSet.cxx  $
   Revision 1.2 1995/07/27 20:20:11EDT rajan 
   Port
Revision 1.2.1.3  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:44:15  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
