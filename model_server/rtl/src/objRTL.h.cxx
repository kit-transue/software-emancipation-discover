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
//
// objRTL.h.C
//
// Run-time list of objects; subclass of objArr
//
// Contains methods.
//
// History:
//   07/29/91    S. Kadakia    Original
//
#include "objRTL.h"

//
// Constructor for this object
//
objRTL::objRTL(objRTL& rtl) : name(rtl.name)
{
   obj_set_name ( (char *)rtl.name, *this);
}

//
// sort method - returns sorted RTL
//		sort criterion at this time is name of objects
//
static int compare_rtl (RelationalPtr one, RelationalPtr two)
{
   return strcmp ( one->get_name(), two->get_name() );
}

void objRTL::sort()
{
   int size, i, j;
   Obj *temp;

// implement bubble sort here
   size = this->size();

   if (size > 1) {

      for (i = 0; i < (size - 1); i++) {
         for (j = 0; j < (size - (i+1)); j++) {
             if (compare_rtl ((RelationalPtr)((*this)[j]),
                              (RelationalPtr)((*this)[j+1])) > 0) {

                // exchange
                temp = (Obj *) (*this)[j];
		(*this)[j] = (*this)[j+1];
                (*this)[j+1] = temp;
             }
         }
      }
   }
}
/*
   START-LOG-------------------------------------------

   $Log: objRTL.h.cxx  $
   Revision 1.1 1993/05/22 14:43:42EDT builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  23:51:37  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:24:41  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
