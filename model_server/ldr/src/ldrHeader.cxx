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
#include "genError.h"
#include "ldrNode.h"
#include "ldrHeader.h"

init_abstract_relational(ldrHeader,ldr);

void ldrHeader::build_selection(const ldrSelection&, OperPoint&)
{
 Initialize(ldrHeader::build_selection);
 Error(NOT_IMPLEMENTED_YET);
}

 appPtr ldrHeader::get_appHeader() const
{
   return ldr_get_app(this);
}

/*
   START-LOG-------------------------------------------

   $Log: ldrHeader.cxx  $
   Revision 1.3 1997/05/21 12:13:52EDT twight 
   Bug track: 14168
   Reviewed by: mb
   
   Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
Revision 1.2.1.5  1993/04/27  16:10:41  davea
bug 3510 - remove erroneous const in formal arglist of build_selection()

Revision 1.2.1.4  1993/03/04  00:11:58  mg
build_selection(

Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:51  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
