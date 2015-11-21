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
#ifndef __scopeNode__h
#define __scopeNode__h

enum scopePmodQuery {
    PMOD_ALL,
    PMOD_VISIBLE,
    PMOD_SLASH,
    PMOD_VISIBLE_SLASH
};

#include <objOper.h>
#include <dis_iosfwd.h>

class objArr;
class genString;
class scopeRoot;
class projNode;
class xrefSymbolSet;
class xrefSymbol;
class symbolSet;

class scopeNode : public appTree
{
  public:
    define_relational(scopeNode,appTree);
    virtual Relational *rel_copy () const;

    virtual void construct_name (genString&) const;
    virtual void construct_prefix_name(genString& st) const; 

    virtual void send_string(ostream&) const ;

    virtual void print(ostream&, int) const;
    scopeNode() { }
    static scopeNode *extract_node (projNode  *cloned_pn, int slash_flag = 0);
    static scopeNode *extract_node (scopeRoot *cloned_from);

    virtual void scopes_where_used (objArr&, objSet&) const;
    virtual void scopes_cloned (symbolArr&) const;
    virtual void pmod_scopes_cloned (objArr&) const;

    virtual symbolPtr get_def_symbol (ddKind, char const *, xrefSymbol *def_file = 0);
    virtual void get_def_symbols (symbolPtr&, symbolArr&, xrefSymbol *def_file = 0);
    virtual void lookup_all (symbolPtr&, symbolArr&);
    virtual void get_symbols (symbolPtr&, symbolArr&, xrefSymbol *dfs = 0, int is_def = 1);

    virtual void get_pmod_modules (symbolArr& modules, symbolSet& mod, scopePmodQuery = PMOD_VISIBLE_SLASH);
    virtual void get_contents (symbolArr&);
    virtual void get_modules  (objArr& /* modules */, int /* depth */) { }
    virtual objTree *  get_root() const;
};
generate_descriptor(scopeNode,objTree);

#endif
