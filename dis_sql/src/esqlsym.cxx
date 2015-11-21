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
/*
 * E S Q L S Y M . C
 *
 * 1997, 1998 Software Emancipation Technology, Inc.
 *
 * Adapted for ELS March 1998 Mark B. Kaminsky
 * Created by William M. Miller
 *
 * This file provides a simple symbol table (basically just a hash
 * table).  It is used for the conditional compilation facility in the
 * esql preprocessor.
 */

#include <malloc.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "sql_all.h"

typedef struct id {
    struct id* next;
    const char* text;
} id;

#define NUM_BUCKETS 213

static id** buckets = 0;

static size_t hash(const char* text) {
    unsigned int accumulator = 0;
    while (text && *text) {
	accumulator *= 3;
	accumulator += *text++;
    }
    return accumulator % NUM_BUCKETS;
}

static void init_buckets() {
    if (!buckets) {
	size_t i;
	buckets = (id**) malloc(sizeof(id*) * NUM_BUCKETS);
	for (i = 0; i < NUM_BUCKETS; i++) {
	    buckets[i] = 0;
	}
    }
}

EXTERN_C void free_hashtable() {
    if (buckets) {
	size_t i;
	for (i = 0; i < NUM_BUCKETS; i++) {
	    id* p;
	    id* q;
	    for (p = buckets[i]; p; p = q) {
		q = p->next;
		free(
#ifdef sun4
		     (char*)
#endif
		     p);
	    }
	}
	free(
#ifdef sun4
	     (char*)
#endif
	     buckets);
        buckets = 0;
    }
}

EXTERN_C void add_id(const char* text) {
    size_t idx;
    id* p;
    init_buckets();
    idx = hash(text);
    for (p = buckets[idx]; p; p = p->next) {
	if (strcmp(p->text, text) == 0) {
	    return; /* already there */
	}
    }
    p = (id*) malloc(sizeof(id));
    p->next = buckets[idx];
    p->text = text;
    buckets[idx] = p;
}

EXTERN_C int is_defined(const char* text) {
    size_t idx;
    id* p;
    init_buckets();
    idx = hash(text);
    for (p = buckets[idx]; p; p = p->next) {
	if (strcmp(p->text, text) == 0) {
	    return 1;
	}
    }
    return 0;
}
