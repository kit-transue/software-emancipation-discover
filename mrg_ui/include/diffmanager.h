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
#ifndef _DIFFMANAGER_H
#define _DIFFMANAGER_H

#include "genArr.h"
#include "genString.h"
#ifndef MTK_GSTRING
#define MTK_GSTRING
typedef genString gString;
#endif

class DiffFile;

enum DiffOperation {
	DIFF_NONE,
	DIFF_VIEW,
	DIFF_EDIT,
	DIFF_MERGE
};

class DiffInfo {
public:
    DiffInfo(char const *nm);

    gString  name;
};

typedef DiffInfo *DiffInfoPtr;

genArr(DiffInfoPtr);

class SrcInfo {
public:
    SrcInfo(char const *nm);
    
    gString      name;
    DiffInfoPtrArr diffs;
};

typedef SrcInfo *SrcInfoPtr;

genArr(SrcInfoPtr);

class DiffManager {
public:
    DiffManager();
    
    void AddSourceFile(char const *name);
    void AddDiffFile(SrcInfoPtr src, DiffInfo *fl);
    void AddDiffFile(SrcInfoPtr src, char const *name);
    int  ReadDataFile(char const *data_file);
    void RequestDiffsFromServer();
    void BatchMerge();
    void ReportFilesWithConflicts();

    static void GetMergeResultName(char const *source_name, gString& result);

    //gString       source_name;
    SrcInfoPtrArr sources;
    SrcInfoPtrArr sources_with_conflicts;
};

#endif
