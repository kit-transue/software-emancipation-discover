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
#include <proj_hash.h>
#include <genError.h>

int path_hash(const char *s, int hash_table_size)
{
    Initialize(path_hash);

    int radix_mod = 128 % hash_table_size;
    int pow = 1;
    int val = 0;
    while (*s)
    {
        if (*s != ' ')
        {
          val = (val + (int((unsigned char)*s) * pow)) % hash_table_size;
          pow = (pow * radix_mod) % hash_table_size;
        }
        s++;
    }
    return (val);
}

path_hash_entry::path_hash_entry(const char *fn, const char *ln, const char *pn)
{
    Initialize(path_hash_entry::path_hash_entry);

    fn_path = fn;
    ln_path = ln;
    sub_proj_n = pn;
    next = 0;
}

path_hash_entry::~path_hash_entry()
{
    Initialize(path_hash_entry::~path_hash_entry);

    if (next)
        delete next;
}

char *path_hash_entry::get_ln()
{
    Initialize(path_hash_entry::get_ln);

    return(ln_path);
}

char *path_hash_entry::get_fn()
{
    Initialize(path_hash_entry::get_fn);

    return(fn_path);
}

char *path_hash_entry::get_proj_n()
{
    Initialize(path_hash_entry::get_proj_n);

    return(sub_proj_n);
}

void path_hash_entry::set_proj_n(const char *pn)
{
    Initialize(path_hash_entry::set_proj_n);

    sub_proj_n = pn;
}

path_hash_entry *path_hash_entry::get_next()
{
    Initialize(path_hash_entry::get_next);

    return next;
}

void path_hash_entry::set_next(path_hash_entry *n)
{
    Initialize(path_hash_entry::set_next);

    next = n;
}

module_hash_entry::module_hash_entry(projModule *m)
{
    Initialize(module_hash_entry::module_hash_entry(projModule*));

    module   = m;
    dir_path = 0;
    next     = 0;
}

module_hash_entry::module_hash_entry(const char *dir_p)
{
    Initialize(module_hash_entry::module_hash_entry(char*));

    module   = 0;
    dir_path = dir_p;
    next     = 0;
}

module_hash_entry::~module_hash_entry()
{
    Initialize(module_hash_entry::~module_hash_entry);

    if (next)
        delete next;
}

projModule *module_hash_entry::get_module()
{
    Initialize(module_hash_entry::get_module);

    return module;
}

const char *module_hash_entry::get_dir_path()
{
    Initialize(module_hash_entry::get_dir_path);

    return dir_path;
}

module_hash_entry *module_hash_entry::get_next()
{
    Initialize(module_hash_entry::get_next);

    return next;
}

void module_hash_entry::set_next(module_hash_entry *n)
{
    Initialize(module_hash_entry::set_next);

    next = n;
}

/*
$Log: proj_hash.h.cxx  $
Revision 1.1 1994/04/12 10:01:41EDT builder 
made from unix file
 * Revision 1.4  1994/04/12  11:51:27  so
 * phase 2 of atria
 *
 * Revision 1.3  1994/02/17  23:33:21  so
 * Bug track: 6405
 * fix bug 6405
 *
 * Revision 1.2  1994/01/13  15:45:52  so
 * Bug track: 5857
 * fix bug 5857
 *
 * Revision 1.1  1994/01/11  19:28:00  so
 * Initial revision
 *
*/
