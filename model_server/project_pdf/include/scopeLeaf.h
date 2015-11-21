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
#ifndef _scopeLeaf__h
#define _scopeLeaf__h

#include <scopeClone.h>

class scopeLeaf;
class scopeRoot;
class scopeUnit;
class xrefSymbol;

class xrefHandle {
 public:
   virtual ~xrefHandle() {}
 private:
   virtual Xref*        xref() const = 0;
   virtual scopeLeaf*   leaf() const = 0;
   virtual scopeRoot*   root() const = 0;
   virtual scopeUnit*   unit() const = 0;
   virtual bool      is_xref() const = 0;
   virtual bool      is_leaf() const = 0;
};

class scopeLeaf : public scopeProj, public xrefHandle
{
  Xref*      pxref;
 public:
   virtual Xref*        xref() const;
   virtual scopeLeaf*   leaf() const;
   virtual scopeRoot*   root() const;
   virtual scopeUnit*   unit() const;
   virtual bool      is_xref() const;
   virtual bool      is_leaf() const;

 public:
  
  scopeLeaf(projNode*p, Xref*x ) : scopeProj(p), pxref(x) {}
  scopeLeaf(const scopeLeaf&other) : scopeProj(other), pxref(other.pxref) {}
  define_relational(scopeLeaf,scopeProj);
  declare_copy_member(scopeLeaf);
  virtual void	print(ostream& = cout, int level = 0) const;
  obj_tree(scopeLeaf);

  virtual void get_contents (symbolArr&);
  virtual void get_pmod_modules (symbolArr&, symbolSet&, scopePmodQuery = PMOD_VISIBLE_SLASH);
  virtual void pmod_scopes_cloned (objArr&) const;
  virtual void get_symbols (symbolPtr&, symbolArr&, xrefSymbol *dfs = 0, int is_def = 1);
  virtual scopeClone *get_unit_clone();
  virtual symbolPtr get_visible_def_symbol (symbolPtr&, xrefSymbol* dfs = 0);
};

RelClass(scopeLeaf);

class scopeSlashLeaf : public scopeLeaf {
 public:
    define_relational(scopeSlashLeaf,scopeLeaf);
    declare_copy_member(scopeSlashLeaf);
    scopeSlashLeaf (projNode*p, Xref*x ) : scopeLeaf (p, x) { }
    scopeSlashLeaf(const scopeSlashLeaf &other) : scopeLeaf(other) { }
};

RelClass(scopeSlashLeaf);

#endif
