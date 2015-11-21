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
#ifndef _proj_hash_h
#define _proj_hash_h

#ifndef _genString_h
#include <genString.h>
#endif

#define MAX_PATH_HASH   431
#define MAX_MODULE_HASH 431

class path_hash_entry
{
  public:
    path_hash_entry(const char *fn, const char *ln, const char *pn);
    ~path_hash_entry();

    char *get_ln();
    char *get_fn();
    char *get_proj_n();
    path_hash_entry *get_next();
    void set_next(path_hash_entry *n);

    void set_proj_n(const char *pn);

  private:
    genString       fn_path;
    genString       ln_path;
    genString       sub_proj_n;
    path_hash_entry *next;
};

class projModule;

class module_hash_entry
{
  public:
    module_hash_entry(projModule *m);
    module_hash_entry(const char *dir_p);
    ~module_hash_entry();

    projModule *get_module();
    const char *get_dir_path();
    module_hash_entry *get_next();
    void set_next(module_hash_entry *n);

  private:
    genString         dir_path;
    projModule        *module;
    module_hash_entry *next;
};

extern int path_hash(const char *s, int hash_table_size);

/*
$Log: proj_hash.h  $
Revision 1.1 1994/04/12 10:01:28EDT builder 
made from unix file
 * Revision 1.4  1994/04/12  11:44:37  so
 * phase 2 of atria
 *
 * Revision 1.3  1994/02/17  23:28:49  so
 * Bug track: 6405
 * fix bug 6405
 *
 * Revision 1.2  1994/01/13  15:42:27  so
 * Bug track: 5857
 * fix bug 5857
 *
 * Revision 1.1  1994/01/11  19:18:18  so
 * Initial revision
 *
*/
#endif
