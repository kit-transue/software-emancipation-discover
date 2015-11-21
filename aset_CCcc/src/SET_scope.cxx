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
#include <vector>

namespace std {}
using namespace std;

struct a_scope;
typedef vector<a_scope *> scope_table_t;
static scope_table_t scope_table;

extern "C" {

#include "basic_hdrs.h"
#include "fe_common.h"
#include "SET_scope.h"
#include <stdio.h>

void track_scope(a_scope_number n, a_scope_ptr p, a_scope_stack_entry_ptr sse)
{
    scope_table_t::size_type sz = scope_table.size();
    if (n < 0 || n == NO_SCOPE_NUMBER) {
    }
    else {
        if (n >= sz) {
	    scope_table.resize(n + 1, (a_scope_ptr)NULL);
        }
	scope_table[n] = p;
    }
    p->if_id = 2;
    if (sse != NULL && p->parent == NULL) {
	while (sse != scope_stack) {
	    sse -= 1;
	    if (sse->il_scope != NULL) {
		p->parent = sse->il_scope;
		break;
	    }
	}
    }
}

a_scope_ptr lookup_scope(a_scope_number n)
{
    if (0 <= n && n < scope_table.size()) {
	return scope_table[n];
    }
    else {
	return NULL;
    }
}

}
