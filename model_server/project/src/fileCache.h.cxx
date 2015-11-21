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
#include <msg.h>
#include <fileCache.h>
#include <fileEntry.h>
#include <genError.h>
#include <genString.h>
#include <systemconstants.h>
#include <machdep.h>

fileCache global_cache;

static fileEntry *nil_fileEntry = NULL;
static fileEntry *get_nil_entry ()
{
    if (nil_fileEntry == 0)
	nil_fileEntry = new fileEntry ();

    return nil_fileEntry;
}

fileCache::fileCache()
{
    cachecalls = 0;
    cachehits = 0;
}

fileCache::~fileCache()
{
    DO(*this,Object,fe) delete (fileEntry *)fe; OD
    delete nil_fileEntry;
}

char const *fileCache::name (const Object *ob) const 
{
    char const *nm = NULL;
    if (ob) {
	fileEntry *fe = (fileEntry *)ob;
	nm = fe->get_name();
    }
    return nm;
}

void fileCache::stats()
{
    msg("Cache access: $1  hits: $2\n") << cachecalls << eoarg << cachehits << eom;
    cachecalls = cachehits = 0;
}


void fileCache::invalidate (char const* path)
{
    Initialize (fileCache::invalidate);
    fileEntry* fentry = add_entry(path);
    fentry->reset();
}

fileEntry* fileCache::add_entry (char const* path)
{
    Initialize(fileCache::add_entry__const_char*);

    fileEntry *fe = NULL;
    if (path && path[0]) {
	int idx = -1;
	Object *ob;
	int found = find (path, idx, ob);
	if (!found) {
	    // Append new entry;
	    fe = new fileEntry(path);
	    fileEntry *new_fe = (fileEntry *)add(*fe);
	    Assert(fe == new_fe);
	} else
	    fe = (fileEntry *)ob;
    }

    if (fe == NULL)
	fe = get_nil_entry();

    return fe;
}

int
fileCache::stat (char const* path, struct OStype_stat* buf)
{
    Initialize (fileCache::stat);
    cachecalls++;
    int was_valid = 0;

    fileEntry* fentry = add_entry(path);
    int ret = fentry->stat (buf, &was_valid);
    if (was_valid)
	cachehits++;

    return ret;
}

int fileCache::lstat (char const* path, struct OStype_stat* buf)
{
    Initialize (fileCache::lstat);
    cachecalls++;
    int was_valid = 0;

    fileEntry* fentry = add_entry(path);
    int ret = fentry->lstat (buf, &was_valid);
    if (was_valid)
	cachehits++;

    return ret;
}

char* fileCache::realpath (char const * path, char* resolved_path)
{
    Initialize (fileCache::realpath);

    extern void project_path_sys_vars_translate(char const *path, genString &ret_path);
    extern void convert_home_dir(char const *path, genString &ret_path);

    genString s;
    genString s0;
    project_path_sys_vars_translate(path, s);
    convert_home_dir(s.str(), s0);
    path = s0.str();

    cachecalls++;

    fileEntry* fentry = add_entry(path);
    int was_valid = 0;
    char *val = fentry->get_realpath (resolved_path, &was_valid);
    if (was_valid)
	cachehits++;

    return val;
}

int fileCache::access (char const* path, int code)
{
    Initialize (fileCache::access);
    cachecalls++;
    int was_valid = 0;
    fileEntry* fentry = add_entry(path);
    int ret = fentry->access (code, &was_valid);
    if (was_valid)
	cachehits++;

    return ret;
}

int fileCache::is_ascii (char const *path)
{
    Initialize (fileCache::is_ascii);
    cachecalls++;
    int was_valid = 0;
    fileEntry* fentry = add_entry(path);
    int ret = fentry->is_ascii (&was_valid);
    if (was_valid)
	cachehits++;

    return ret;
}

int fileCache::is_frame (char const *path)
{
    Initialize(fileCache::is_frame);
    cachecalls++;
    int res = 0;
    int was_valid = 0;
    fileEntry *fe = add_entry (path);
    res = fe->is_frame(&was_valid);
    if (was_valid)
	cachehits++;

    return res;
}

int fileCache_stat (char const* path, struct OStype_stat* buf)
{
    return global_cache.stat (path, buf);
}

int fileCache_lstat (char const* path, struct OStype_stat* buf)
{
    return global_cache.lstat (path, buf);
}

int fileCache_access (char const* path, int code)
{
    return global_cache.access (path, code);
}

char* fileCache_realpath (char const * path, char* resolved_path)
{
    return global_cache.realpath (path, resolved_path);
}
