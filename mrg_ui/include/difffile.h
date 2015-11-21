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
#ifndef _DIFFFILE_H
#define _DIFFFILE_H

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "genArr.h"
#include "genString.h"
#ifndef MTK_GSTRING
#define MTK_GSTRING
typedef genString gString;
#endif

typedef enum { DELETE, INSERT, REPLACE } ChangeOperation;

class Change {
public:
	Change();

	ChangeOperation operation;
	int             index;
	int             offset;
	int             length;
	int             applied;
	gString		new_text;
};

class Conflict {
public:
	int    no_changes;
	int    first_change;
};

class ChangeFileName {
public:
    ChangeFileName(char const *str);
    gString name;
};

typedef Change *ChangePtr;

genArr(ChangePtr);

typedef Conflict *ConflictPtr;

genArr(ConflictPtr);

typedef ChangeFileName *ChangeFileNamePtr;

genArr(ChangeFileNamePtr);

class DiffFile {
public:
    DiffFile(char const *source);
    virtual ~DiffFile();

    void     LoadChangeFile(char const *fname);
    int      SaveChangeFile(char const *name);
    void     SaveTextField(char const *txt, FILE *fl);
    void     Sort();
    void     RemoveSimilarChanges();
    int      TestConflicts();
    void     PurgeConflictsList();
    void     BuildConflictsList();
    int      GetNumOfChanges();
    int      GetNumOfConflicts();
    Change   *GetChange(int num);
    Conflict *FindConflict(Change *change);
    void     GetConflictRange(Conflict *cf, int &start, int &end);
    void     RemoveDeltaFiles();
    int      GetDeltaFileCount();
    char     *GetDeltaFileName(int index);

    static void ReadComment(char *file_name, gString& result);

    gString              source_name;
    ChangePtrArr         changes;
    ConflictPtrArr       conflicts;
    ChangeFileNamePtrArr file_components;
        
private:
    char GetSeparator(char *str);
    int  ParseIndex(FILE *fl, int c, Change *ch);
    int  ParseStartOffset(FILE *fl, int c, Change *ch);
    int  ParseLength(FILE *fl, int c, Change *ch);
    int  ParseText(FILE *fl, int c, Change *ch);
    int  ParseLine(FILE *fl);
    int  LoadChanges(FILE *fl);
    int  LoadHeader(FILE *fl);

    static int ChangeCompare(const void *elem1, const void *elem2);
};

#endif

