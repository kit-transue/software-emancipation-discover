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
#ifndef _scopeMaster__h
#define _scopeMaster__h

#include <scopeRoot.h>

class scopeSLL : public scopeRoot
{
  public:
    define_relational(scopeSLL,scopeRoot);
    scopeSLL (char const *name);
};
generate_descriptor(scopeSLL,scopeRoot);

class scopeEXE : public scopeRoot
{
    objArr import_dlls;
  public:
    define_relational(scopeEXE,scopeRoot);
    scopeEXE() { }
    scopeEXE (char const *name);
    virtual boolean add_import (scopeDLL *, ostream&);
    virtual void scopes_imported (symbolArr&) const;
};
generate_descriptor(scopeEXE,scopeRoot);

class scopeDLL : public scopeEXE
{
  public:
    symbolSet exported_syms;
    define_relational(scopeDLL,scopeEXE);
    scopeDLL (char const *name);
    boolean add_export (symbolPtr&, ostream&);
    boolean add_export (ostream&, ddKind k, char const *sym_name, xrefSymbol *dfs = NULL);
    virtual void symbols_exported (symbolArr&) const;
};
generate_descriptor(scopeDLL,scopeEXE);

defrel_many_to_many(scopeEXE,exec,scopeDLL,dll);

#endif
