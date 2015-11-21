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
#ifndef __scopeClone__h
#define __scopeClone__h

#include <scopeNode.h>
#include <scopeRoot.h>

class Xref;
class scopeLeaf;
class scopeRoot;
class scopeUnit;

class scopeClone : public scopeNode
{
 public:
  scopeRoot*proot;
  scopeClone() : proot(0) {}
  scopeClone(const scopeRoot*);
  scopeClone(const scopeClone&);
  define_relational(scopeClone,scopeNode);
  declare_copy_member(scopeClone);
  virtual void	print(ostream& = cout, int level = 0) const;
  obj_tree(scopeClone);

  virtual void construct_name (genString&) const;
  virtual void construct_prefix_name(genString& st) const;

  virtual void scopes_cloned (symbolArr&) const;

    virtual ddKind get_kind () const { return DD_SCOPE; }
    virtual char const *get_name() const;

    virtual int construct_symbolPtr(symbolPtr&) const;
    virtual ddKind get_symbolPtr_kind() const;

    virtual int is_symbolPtr_def () const;         //analog to get_is_def()
    virtual app *get_symbolPtr_def_app ();

    virtual void set_root(scopeRoot*rp, char const *pname);
    virtual void get_modules  (objArr& modules, int depth);
    virtual void get_pmod_modules (symbolArr&, symbolSet&, scopePmodQuery = PMOD_VISIBLE_SLASH);
 protected:
    genString lname;
};

class scopeProj : public scopeClone
{
 public:

  scopeUnit* punit;
  void set_unit(scopeUnit*);

  scopeProj(const projNode*p );
  scopeProj(const scopeProj& other);
  define_relational(scopeProj,scopeClone);
  declare_copy_member(scopeProj);

  obj_tree(scopeProj);

  virtual void construct_name (genString&) const;
  virtual void construct_prefix_name(genString& st) const;
  virtual void scopes_cloned (symbolArr&) const;

  virtual void set_root(scopeRoot*rp, char const *pname);

  virtual void get_modules  (objArr& modules, int depth);
};

defrel_many_to_one(scopeClone,clone,scopeRoot,master);
defrel_many_to_one(scopeProj, clone,projNode,proj);


RelClass(scopeClone);
RelClass(scopeProj);

#endif

