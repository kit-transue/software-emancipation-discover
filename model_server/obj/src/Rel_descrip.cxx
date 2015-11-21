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
// Rel_descrip.C
//
#include "Relational.h"
#include "genString.h"
#include "genError.h"

void Relational::description(genString& str)
{
    Initialize(Relational::description);
    str = "Relational";
    Return
}
/*
   START-LOG-------------------------------------------

   $Log: Rel_descrip.cxx  $
   Revision 1.1 1993/07/28 19:51:44EDT builder 
   made from unix file
Revision 1.2.1.2  1992/10/09  18:53:02  boris
*** empty log message ***

   Revision 1.2.1.1  92/10/07  20:49:53  smit
   *** empty log message ***

   Revision 1.2  92/10/07  20:49:52  smit
   *** empty log message ***

   Revision 1.1  92/10/07  18:21:14  smit
   Initial revision

   Revision 1.1  92/10/07  17:56:52  smit
   Initial revision

   Revision 1.2  92/04/19  10:29:13  builder
   Add genString.h, genError.h.
 
   Revision 1.1  92/04/19  10:28:05  glenn
   Initial revision

   END-LOG---------------------------------------------

*/
