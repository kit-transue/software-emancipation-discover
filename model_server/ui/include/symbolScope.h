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
#ifndef _symbolScope_h
#define _symbolScope_h

#ifndef _objArr_h
#include <objArr.h>
#endif

#ifndef _symbolSet_h
#include <symbolSet.h>
#endif

#ifndef _ddSelector_h
#include <ddSelector.h>
#endif

#ifndef _proj_h
#include <proj.h>
#endif

class scopeNode;

class symbolScope
{
  public:
    symbolScope();
    symbolScope(const symbolScope&);
    symbolScope(objArr&);
    ~symbolScope();
    
    void set_domain(objArr&);
    void update_modules(void);
    void update_modules(symbolPtr symbol);
    void query(symbolArr& symbols, ddSelector& selector);
    void query_local(symbolArr& symbols, ddSelector& selector);
    int  is_in_scope(symbolPtr symbol);
    void add_module(symbolPtr symbol);
    void update_proj(projNode*);	// flag creation/deletion of module

    objArr    domain;
    symbolArr modules;
    int       updated;
    struct  proscopArr* servers;
    void get_all_modules(projNode *pn, symbolSet& mod);
    void query_pmod_for_modules(projNode *pn, symbolSet& mod);
    void query_pmod_for_modules(scopeNode *pn, symbolSet& mod);
    int  pmod_on_top(projNode *pn);
};

inline symbolScope::symbolScope(const symbolScope& that)
{
    updated = 0;
    domain  = that.domain;
}

#endif 

