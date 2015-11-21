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
#ifndef __smtMarker_h__
#define __smtMarker_h__

#include "genString.h"
#include "appMarker.h"

class smtTree;

class smtMarker : public appMarker
{
  public:
    smtMarker(smtTree *smt);

    define_relational(smtMarker, appMarker);

    virtual char const *get_name() const;
    virtual ddKind get_symbolPtr_kind() const { return DD_SMT; };
    virtual int    is_symbolPtr_def() const { return 1; };
    virtual int    construct_symbolPtr(symbolPtr&) const;
    virtual void   report_remove(int rt);

    int       offset;
    int       level;
    int       ref;
    int       dead;
    genString module_name;
}; 

generate_descriptor(smtMarker,appMarker);

smtTree *GetSmtFromMarker(symbolPtr& sym);
smtTree *GetSmtFromMarker(smtMarker *sm);
void    get_instances_from_smt(symbolPtr& s, symbolArr& res);

#endif

