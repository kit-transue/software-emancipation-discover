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
#ifndef __scopeRoot__h
#define __scopeRoot__h

#include <objOper.h>
#include <scopeNode.h>
#include <genString.h>
#include <symbolScope.h>
#include <ddKind.h>
#include <objArr.h>

class scopeDLL;
class xrefSymbolSet;
class scopeUnit;

enum scopeRootKind {
    SCOPE_COMPONENT,
    SCOPE_UNIT,
    SCOPE_SLL,
    SCOPE_DLL,
    SCOPE_EXE
};

class scopeClone;
class scopeRoot : public scopeNode
{
  public:
    scopeRoot () : hidden (0) {}
    scopeClone* get_first() const;
    void put_last(scopeClone*);
    int updated;
    genString lname;
    scopeRootKind scope_kind;

    define_relational(scopeRoot,scopeNode);
    virtual ddKind get_kind () const { return DD_SCOPE; }
    virtual char const *get_name() const;

    virtual int construct_symbolPtr(symbolPtr&) const;
    virtual ddKind get_symbolPtr_kind() const;

    virtual int is_symbolPtr_def () const;         //analog to get_is_def()
    virtual app *get_symbolPtr_def_app ();

    virtual void internal_create (char const *lname ,scopeRootKind);

    virtual scopeRoot *promote_to_scope (projNode *, ostream& os) const;
    virtual scopeNode *add_component_project (projNode *, ostream& os);  
    virtual scopeNode *add_component_scope (scopeRoot *, ostream& os);  
    scopeNode *add_component_ipu (projNode *, scopeUnit*, int slash_flag = 0);
    virtual scopeNode *add_component_internal (scopeRoot *);
    virtual boolean add_import (scopeDLL *, ostream&);
    virtual boolean contains_component (scopeRoot *) const;

    virtual void scopes_cloned (symbolArr&) const;
    virtual void construct_name (genString&) const;
    virtual void construct_prefix_name(genString& st) const; 

    virtual void get_modules  (objArr& modules, int depth);
    virtual Relational *rel_copy () const;
    virtual void scopes_where_used (objArr&, objSet&) const;
    virtual void scopes_imported (symbolArr&) const;
    virtual void symbols_exported (symbolArr&) const;
    virtual void invalidate ();
    virtual void update (boolean force = false);

    bool is_hidden() const { return hidden != 0; }
    void set_hidden(int value) { hidden = value ? 1 : 0; }

  private:
    unsigned int hidden : 1;
};
generate_descriptor(scopeRoot,scopeNode);

class scopeTrash : public scopeRoot
{
  private:
    scopeNode *current_last;

  public:
    define_relational(scopeTrash,scopeRoot);
    scopeTrash (char const *lname = 0);
private:
    ~scopeTrash();
public:

    virtual scopeNode *add_component_internal (scopeRoot *)   { return NULL; }
    virtual boolean add_import (scopeDLL *, ostream&)         { return false; }

    virtual void scopes_cloned (symbolArr&) const             { }
    virtual void get_modules  (objArr& /* modules */, int /* depth */)    { }

    virtual void update (boolean /* force = false */)               { }
    virtual void invalidate ()                                { }

    virtual void get_pmod_modules (symbolArr&, symbolSet&, scopePmodQuery = PMOD_VISIBLE_SLASH)    { }

    virtual objTree *  get_root() const;

    virtual boolean contains_component (scopeRoot *rt) const { return contains(rt); }
    boolean contains (scopeNode *) const;
    void add (scopeNode *);
};
generate_descriptor(scopeTrash,scopeRoot);


#endif
