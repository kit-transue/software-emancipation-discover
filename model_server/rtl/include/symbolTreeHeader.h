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
#ifndef __symbolTreeHeader_h_
#define __symbolTreeHeader_h_

#include <RTL.h>
#include <ddHeader.h>

RelClass(symbolTreeHeader);

class symbolTreeHeader : public RTL 
{
  public:
    symbolTreeHeader (char* name);

    define_relational (symbolTreeHeader, RTL);

    virtual void xref_notify (const symbolArr& modified,
			      const symbolArr& added,
			      const symbolArr& deleted);
};

generate_descriptor(symbolTreeHeader, RTL);

defrel_many_to_one(symbolTreeHeader,symtree,ddHeader,ddheader);

#endif

/*
   START-LOG-------------------------------------------

   $Log: symbolTreeHeader.h  $
   Revision 1.1 1993/07/29 10:42:15EDT builder 
   made from unix file
// Revision 1.2  1993/02/01  18:09:04  wmm
// Fix bug 2347.
//
// Revision 1.1  1993/01/03  20:46:36  aharlap
// Initial revision
//

   END-LOG---------------------------------------------

*/
