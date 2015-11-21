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
#ifndef __fileCache__h_
#define __fileCache__h_
#include <Hash.h>
#include "machdep.h"
class fileEntry;
struct stat;
class fileCache : public nameHash {
  public:

    fileCache();
    ~fileCache();

    virtual const char* name(const Object*) const;

    fileEntry* add_entry (const char*);

    int stat (const char*,  struct OStype_stat*);
    int lstat (const char*, struct OStype_stat*);
    char* realpath(const char*, char*);
    int access(const char*, int);
    int is_ascii(const char*);
    int is_frame(const char*);

    void stats();

    void invalidate();
    void invalidate(const char*);

private:
    int cachecalls;	
    int cachehits;	
};

int fileCache_stat (const char* path, struct OStype_stat* buf);
int fileCache_lstat (const char* path, struct OStype_stat* buf);
void fileCache_invalidate(const char*);
int fileCache_access (const char* path, int code);
char* fileCache_realpath (const char * path, char* resolved_path);

extern fileCache global_cache;
#endif
