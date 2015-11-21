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
#ifndef _MTKARGS_H
#define _MTKARGS_H

#include "general.h"

//============================================================================
// This class was created for the mrg_ui utility, when converted to be built
// with the Tk grapics toolkit, rather than Galaxy.
// It fills in for the Galaxy Args class.
// Mtk stands for mrg-tk.
//============================================================================

struct MtkValidArgs;
struct ParseProgress;

class MtkArgs {
public:
    // Spec is terminated by a null tag.
    MtkArgs(int argc, char *argv[], MtkValidArgs spec[]);
    ~MtkArgs();

    // Check whether this set of arguments conforms to the requirements
    // of the MtkValidArgs, with no more than max_additions unassociated
    // list elements.  A negative value for max_additions indicates no limit.
    boolean check(int max_additions);

    // Get the nth value associated with the given tag.
    // 0-based: If the tag has 1 association, then use getAssoc("tag", 0).
    char *getAssoc(char *tag, int n);

    // Return whether the given tag is present as an argument.
    boolean isPresent(char *tag);

    // Return the nth unassociated argument, 0-based.
    char *getOtherArg(int n);

private:
    int argc;
    char **argv;
    MtkValidArgs *spec;

    int num_additions;
    int *num_assoc;
    int num_specs;
    int next_index;
    int current_tag;

    void initParse();
    int parseChunk();
    char *curArg();
    int findTag(char *tag);
};

extern MtkArgs *startup_args;

// Represents a tag with associated values in the command argument list.
// If tag is "name" and nassocs is 2, then a command line might be:
//    doit -name George Washington
struct MtkValidArgs {
    char const *tag;
    int nassocs;
};
#endif // _MTKARGS_H
