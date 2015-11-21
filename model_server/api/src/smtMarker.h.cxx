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
#include <smtMarker.h>
#include <smt.h>
#include <_Xref.h>
#include <xrefSymbol.h>
#include <symbolSet.h>
#include <ddict.h>

init_relational(smtMarker,Relational);

smtTree *TryToRestoreSmt(smtMarker *sm)
{
    return NULL;
}

smtTree *GetSmtFromMarker(smtMarker *sm)
{
    smtTree *smt = NULL;

    int ofs;  // dummy argument
    smt = (smtTree *)sm->get(ofs);
    if(smt == NULL && !sm->dead)
	smt = TryToRestoreSmt(sm);
    return smt;
}

smtTree *GetSmtFromMarker(symbolPtr& sym)
{
    if(sym.is_smt()){
	Relational *rp = sym;
	smtMarker *sm  = (smtMarker *)rp;
	return GetSmtFromMarker(sm);
    }
    return NULL;
}

//---------------------------------------------------------------------

smtMarker::smtMarker(smtTree *smt) : appMarker(0, smt, 0)
{
    offset       = smt->start();
    level        = 0;
    ref          = 0;  
    dead         = 0;
    appTree *tmp = smt->get_parent();
    while(tmp){
	tmp = (appTree *)tmp->get_parent();
	level++;
    }
    smtHeader *sh = (smtHeader *)smt->get_header();
    module_name   = sh->get_name();
}

#ifndef ISO_CPP_HEADERS
#include "stdio.h"
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

void smtMarker::report_remove(int rt)
{
    Initialize(smtMarker::report_remove);
    
    //printf("Dead marker\n");
    dead = 1;
    appMarker::report_remove(rt);
}

char const *smtMarker::get_name() const
{
    static genString nm;

    if(!dead) {
	nm.printf("%d,%d:%s", offset, level, (char *)module_name);
    } else
	nm = "_SMT_";
    return (char *)nm;
}

int smtMarker::construct_symbolPtr(symbolPtr& sym) const
{
    smtTree *smt = GetSmtFromMarker((smtMarker *)this);
    if(smt){
	do{
	    ddElement *dd = (ddElement *)get_relation(def_dd_of_smt, smt);
	    if(dd){
		sym = dd;
		return 1;
	    }
	    if(smt->get_parent() != NULL)
		smt = (smtTree *)smt->get_parent();
	    else { // already at the SMT_FILE get dd of header
		smtHeader *sh = (smtHeader *)smt->get_header();
		dd            = (ddElement *)get_relation(dd_of_appHeader, sh);
		if(dd){
		    sym = dd;
		    return 1;
		} else
		    return 1;
	    }
	} while(smt != NULL);
    } 
    return 1;
}
