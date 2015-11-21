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

#include <objRelation.h>
#include <RelSentinel.h>

//
// RelSentinel is a simple Relational class that provides delete-propagation
// via callback for non-Relational objects. It hooks itself to any arbitrary
// Relational object via the sentinel_of_relational relation, which propagates
// deletes; in its destructor it calls a user-specified callback.
// 
// Caution to the user is advised; you can't reference the RelSentinel during
// the callback, because it's been deleted. If the callback deletes your 
// object, be sure not to delete the RelSentinel again in _your_ destructor.
// Also, if you delete the RelSentinel explicitly, like when you're done 
// with it, you should call done() on it first to disable its delete callback.
// These are not the only possible traps...
//
// There's a document file on using RelSentinel you can also look at.

init_relation(relational_of_sentinel,1,0,sentinel_of_relational,MANY,relationMode::D);
init_relational(RelSentinel, Relational);

RelSentinel::RelSentinel(RelSentinelCB cb, void *o, void *d, Relational *r) {
  callback = cb;
  callback_object = o;
  callback_data = d;
  guard(r);
}

RelSentinel::RelSentinel(const RelSentinel &that) {
  callback = that.callback;
  callback_object = that.callback_object;
  callback_data = that.callback_data;
#ifdef _WIN32
  guard((Relational *)get_relation(relational_of_sentinel, &(RelSentinel)that));
#else
  guard((Relational *)get_relation(relational_of_sentinel, &that));
#endif
}

RelSentinel::~RelSentinel() {
  if (callback) callback(callback_object, callback_data);
}

void RelSentinel::guard(Relational *r) {
  put_relation(relational_of_sentinel, this, r);
}

Relational *RelSentinel::guarded() {
  return (Relational *) get_relation(relational_of_sentinel, this);
}

void RelSentinel::done() {
  callback = NULL;
}

/*
   START-LOG-------------------------------------------

   $Log: RelSentinel.h.cxx  $
   Revision 1.4 2002/03/04 17:24:12EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.2  1992/10/09  18:53:01  boris
*** empty log message ***


   END-LOG---------------------------------------------

*/
