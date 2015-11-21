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
// dd_set_baseclass
//------------------------------------------
// synopsis:
//      A routine for setting baseclass
//------------------------------------------

// include files

#include "ddict.h"
#include "smt.h"
#include "objOperate.h"
#include "objRelation.h"

//-----------------------------------------
//Function Name: dd_set_baseclass_rel
//
//Description: establish a baseclass relationship in the DD
//
//    for internal use only
//

void dd_set_baseclass_rel( ddElementPtr ep, ddElementPtr cp,
		 int is_virtual, ddProtection protect)
{

  Initialize(dd_set_baseclass_rel);
  if ( protect == DD_PUBLIC ) {
    if ( is_virtual ) {
      put_relation(vpub_baseclassof,ep,cp);
    }
    else {
      put_relation(pub_baseclassof,ep,cp);
    }
  }
  else if ( protect == DD_PROTECTED ) {
    if ( is_virtual ) {
      put_relation(vpro_baseclassof,ep,cp);
    }
    else {
      put_relation(pro_baseclassof,ep,cp);
    }
  }
  else if ( protect == DD_PRIVATE ) { 
    if ( is_virtual ) {
      put_relation(vpri_baseclassof,ep,cp);
    }
    else {
      put_relation(pri_baseclassof,ep,cp);
    }
  }
  else LanguageError();

  Return
}
