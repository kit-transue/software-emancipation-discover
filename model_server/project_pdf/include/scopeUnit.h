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
#ifndef _scopeUnit__h
#define _scopeUnit__h

#include <scopeRoot.h>
#include <Hash.h>
#include <objArr.h>
#include <symbolArr.h>

class xrefSymbolSet;

class uniqNameHash : public nameHash
{
    virtual const char* name(const Object*)const;
    virtual bool isEqualObjects(const Object&, const Object&) const;
};

class scopeUnit : public scopeRoot
{
  private:
    boolean updated_hidden;

    xrefSymbolSet* hidden_syms;

    uniqNameHash visible_mod_hash;

    void update_hidden_symbols (boolean force = false);
    void regenerate();

  public:
    define_relational(scopeUnit,scopeRoot);
    scopeUnit (const char *lname);
    ~scopeUnit();

    void cleanup ();
    const xrefSymbolSet *get_hidden_syms();

    virtual scopeNode *add_component_project (projNode *, ostream& os);  
    virtual scopeNode *add_component_scope (scopeRoot *, ostream& os);  
    virtual void scopes_cloned (symbolArr&) const;

    virtual void lookup_all (symbolPtr&, symbolArr&);
    virtual void get_modules  (objArr& modules, int depth);

    scopeNode *add_project   (projNode *, int slash_flag = 0);  
    boolean    is_hidden_symbol (symbolPtr&);

    virtual void update (boolean force = false);
    virtual void invalidate ();
    void invalidate_hidden_symbols () { updated_hidden = false; }

    virtual void hide_symbols (projModule *);

    virtual void update_hidden_in_projs ();

    projModule *find_visible_module (const char *);
    virtual void get_pmod_modules (symbolArr&, symbolSet&, scopePmodQuery = PMOD_VISIBLE_SLASH);
};
generate_descriptor(scopeUnit,scopeRoot);

#endif
