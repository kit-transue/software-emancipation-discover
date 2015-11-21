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
// File containing useful utility function salvaged from propManager
//

#include <app.h>
#include <symbolPtr.h>
#include <smt.h>
#include <ddict.h>

symbolPtr get_smt_decl_from_init(symbolPtr sym, genString &sym_name)
{
  Initialize(get_smt_decl_from_init);

  if (sym.isnull() || sym.is_xrefSymbol() || sym.without_xrefp()) return sym;
  appTree *at =  sym;
  if (!is_smtTree(at)) return NULL_symbolPtr;
  smtTree *smt = checked_cast(smtTree, at);
  ddElement * dd = smt_get_dd(smt);
  smtTree * par = 0;
  if ((dd == 0) && (smt->extype == SMTT_const || smt->extype == SMTT_string)) {
    par = smt->get_parent();
    dd = smt_get_dd(par);
  }

  if (dd == 0) return NULL_symbolPtr;
  symbolPtr ret_sym = dd->get_xrefSymbol();
  if (ret_sym.isnotnull() && par)
    sym_name = smt->get_name();
  return ret_sym;
}

