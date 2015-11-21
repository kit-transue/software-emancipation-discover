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
#include <objRelation.h>
#include <smtRegion.h>
#include <smt.h>
#include <_Xref.h>
#include <xrefSymbol.h>
#include <symbolSet.h>
#include <ddict.h>

init_relational(smtRegion,smtMarker);

smtTree *GetSmtFromMarker(smtMarker *sm);

smtRegion *GetRegion(symbolPtr& sym)
{
    if(sym.relationalp()){
	Relational *rp = sym;
	if(is_smtRegion(rp))
	    return (smtRegion *)rp;
    }
    return NULL;
}

//---------------------------------------------------------------------

smtRegion::smtRegion(smtTree *start_tree, smtTree *end_tree) : smtMarker(start_tree)
{
    end = new smtMarker(end_tree);
}

char const *smtRegion::get_name() const
{
    static genString nm;

    smtTree *smt_start = GetSmtFromMarker((smtMarker *)this);
    smtTree *smt_end   = GetSmtFromMarker((smtMarker *)end);
    if(smt_start && smt_end){
	smtHeader *sh = (smtHeader *)smt_start->get_header();
	int len       = smt_end->start() + smt_end->length() - offset;
	nm.printf("%d-%d:%s", offset, len, sh->get_name());
    } else
	nm = "_REGION_";
    return (char *)nm;
}
