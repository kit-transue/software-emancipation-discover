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
#include "SET_list.h"

typedef void *list_node_ptr;

#define LIST_BLOCK_COUNT 8000

typedef struct SET_list_block {
    SET_list_block_ptr next;
    size_t next_node;
    char node[LIST_BLOCK_COUNT];
} list_block, *list_block_ptr;

list_node_ptr new_SET_list_node(SET_list_ptr lp) {
    list_node_ptr np;
    if (!lp->last || lp->last->next_node >= LIST_BLOCK_COUNT) {
	list_block_ptr bp = (list_block_ptr) malloc(sizeof(list_block) + LIST_BLOCK_COUNT * (lp->element_size - 1));
	if (lp->last) {
	    lp->last->next = bp;
	}
	else lp->first = bp;
	lp->last = bp;
	bp->next = NULL;
	bp->next_node = 0;
    }
    np = &lp->last->node[lp->last->next_node++ * lp->element_size];
    return np;
}

void traverse_SET_list(SET_list_ptr lp, void (*fun)(list_node_ptr)) {
    list_block_ptr bp = lp->first;
    int cur_node = 0;
    while (bp) {
	list_node_ptr np;
	if (cur_node >= bp->next_node) {
	    bp = bp->next;
	    cur_node = 0;
	}
	if (bp) {
	    np = &bp->node[cur_node++ * lp->element_size];
	}
	else {
	    np = NULL;
	}
	if (np) {
    	    fun(np);
	}
    }
}

void init_SET_list(SET_list_ptr lp, size_t element_size) {
    lp->first = NULL;
    lp->last = NULL;
    lp->element_size = element_size;
}
