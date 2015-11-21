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
/* new file pulled off the former Xref.h.C * NY *092195*/

#include <SharedXref.h>
#include <xref.h>

// class static data for Shared Xref's
SharedXref *SharedXref::cur_SharedXref = NULL;
int         SharedXref::save_active    = 0; // 0 -> not in a save operation
                                            // 1 -> in a save operation


SharedXref::SharedXref(XrefTable *table) : Xref((projNode*)0)
{
  Initialize(SharedXref::SharedXref);
  //  xref_table = NULL;
  perm_flag  = NULL;
  save_active = 1;
  if (!table) {
    lxref      = NULL;
    return;
  }


  if (!cur_SharedXref) {
      cur_SharedXref = this;
      mode     = 0;
      lxref = table;
  }
}

SharedXref::~SharedXref()
{
  save_active = 0;
  lxref = 0;
  cur_SharedXref = (SharedXref *)NULL;
  if (last_Xref==this) last_Xref=NULL;
}

Xref *SharedXref::get_cur_shared_xref() {
  return (Xref *)cur_SharedXref;
}

// *******************************************************
// Activity Flag: if 1, a  save operation is in progress
//                if 0, no save operation is in progress

int SharedXref::saving() {
  return save_active;
}
// in case somebody outside wants to set/reset saving flag
// for purposes nefarious or fair
void SharedXref::saving_on() {
  save_active = 1;
}
void SharedXref::saving_off() {
  save_active = 0;
}

