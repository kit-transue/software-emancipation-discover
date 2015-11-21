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
#include "mtkargs.h"
#include "string.h"

enum { chn_done, chn_error, chn_tag, chn_assoc, chn_free };

//============================================================================
MtkArgs::MtkArgs(int ac, char *av[], MtkValidArgs sp[])
: argc(ac), argv(av), spec(sp), current_tag(0)
{
    int i;
    for (i = 0; sp[i].tag != NULL; i += 1) {}
    num_specs = i;
    num_assoc = new int[i];
}

//============================================================================
MtkArgs::~MtkArgs()
{
    delete [] num_assoc;
}

//============================================================================
boolean MtkArgs::check(int max_additions)
{
    initParse();
    for (;;) {
	int kind = parseChunk();
        if (kind == chn_done) {
	    return max_additions < 0 || num_additions <= max_additions;
	}
	else if (kind == chn_error) {
	    return false;
        }
    }
}

//============================================================================
char *MtkArgs::getAssoc(char *tag, int n)
{
    int tn = findTag(tag);
    if (tn < 0) {
	return NULL;
    }
    initParse();
    for (;;) {
	char *arg = curArg();
	int an = num_assoc[tn];
	int kind = parseChunk();
	if (kind == chn_assoc && current_tag == tn && an == n) {
	    return arg;
        }
	if (kind == chn_done) {
	    return NULL;
	}
    }
}

//============================================================================
boolean MtkArgs::isPresent(char *tag)
{
    int tn = findTag(tag);
    if (tn < 0) {
	return false;
    }
    initParse();
    for (;;) {
	int kind = parseChunk();
	if (kind == chn_tag && current_tag == tn) {
	    return true;
        }
	if (kind == chn_done) {
	    return false;
	}
    }
}

//============================================================================
char *MtkArgs::getOtherArg(int n)
{
    initParse();
    for (;;) {
	int fn = num_additions;
	char *arg = curArg();
	int kind = parseChunk();
        if (kind == chn_free && fn == n) {
	    return arg;
	}
	if (kind == chn_done) {
	    return NULL;
	}
    }
}

//============================================================================
// Initialize, prior to calling parseChunk.
//
void MtkArgs::initParse()
{
    num_additions = 0;
    for (int j = 0; j < num_specs; j += 1) {
       num_assoc[j] = 0;
    }
    next_index = 1;
}

//============================================================================
// Returns the content of the current argument in the parse.
//
char *MtkArgs::curArg()
{
    if (next_index >= argc) {
	return NULL;
    }
    return argv[next_index];
}

//============================================================================
// Parse an argument string from the argument list.
// If the argument begins with '-' then it's a tag-based argument;
// otherwise the chunk consists of just this one free-form string.
// A tag-based argument is grouped with succeeding non-tag-based arguments,
// up to the stated limit for that particular tag.  However, only one
// index in argv is processed by parseChunk at a time, based on the current
// tag.  It is an error if an unexpected tag appears in the list.
//
int MtkArgs::parseChunk()
{
    if (current_tag >= 0 && num_assoc[current_tag] >= spec[current_tag].nassocs) {
        // current tag can accept no more associations
        current_tag = -1;
    }
    char *arg = curArg();
    if (arg == NULL) {
	current_tag = -1;
	return chn_done;
    }
    next_index += 1;
    if (arg[0] == '-') {
        current_tag = findTag(arg+1);
	if (current_tag < 0) {
	    // error
	    return chn_error;
	}
	// Introduced current tag.
	return chn_tag;
    }
    else if (current_tag >= 0) {
	num_assoc[current_tag] += 1;
	// Found an association for current tag.
	return chn_assoc;
    }
    else {
        num_additions += 1;
	// Found an unassociated argument.
	return chn_free;
    }
}

//============================================================================
// Return the index int the specs array, representing the given tag.
// Returns -1 if the tag is not a valid spec.
//
int MtkArgs::findTag(char *tag) {
    for (int i = 0; i < num_specs; i += 1) {
	if (strcmp(spec[i].tag, tag) == 0) {
	    return i;
        }
    }
    return -1;
}
