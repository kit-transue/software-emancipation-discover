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
#ifndef __fileEntry__h_
#define __fileEntry__h_

#include <objRelation.h>
#include <genString.h>
#include <sys/stat.h>
#include <cLibraryFunctions.h> // OStype_stat
#include <dis_iosfwd.h>

class projLine;

class fileEntry : public Relational
{
  public:
    define_relational(fileEntry,Relational);
    fileEntry();
    fileEntry(const char *path_name);
    ~fileEntry();

    virtual char const *get_name() const;
    virtual void  print(ostream&, int level = 0) const;
    virtual void put_signature(ostream&) const;
    void reset ();
    void set_flags();
    void restore_flags (projLine&, bool is_phys, bool is_writable);

    char *get_realpath (char *resolved_path, int *was_valid = 0);
    int stat     (struct OStype_stat*, int *was_valid = 0);
    int lstat    (struct OStype_stat*, int *was_valid = 0);
    int is_ascii (int *was_valid = 0);
    int is_frame (int *was_valid = 0);

    int access   (int code, int *was_valid = 0);

    int is_rok   (int *was_valid = 0);
    int is_wok   (int *was_valid = 0);
    int is_xok   (int *was_valid = 0);
    int is_fok   (int *was_valid = 0);
    int is_dir   (int *was_valid = 0);
    int is_link  (int *was_valid = 0);

    genString path;
    genString realpath;
    genString ascii_error_text;

    int realpath_errno;
    int stat_errno;
    int lstat_errno;

  protected:
    struct OStype_stat *stat_buf;
    struct OStype_stat *lstat_buf;

    unsigned int stat_is_valid     : 1;   // 16 *(Flag, Value) = 32 bits
    unsigned int _stat_ok          : 1;

    unsigned int lstat_is_valid    : 1;
    unsigned int _lstat_ok         : 1;

    unsigned int ascii_is_valid    : 1;
    unsigned int _ascii_ok         : 1;

    unsigned int frame_is_valid    : 1;
    unsigned int _frame_ok         : 1;

    unsigned int rok_is_valid      : 1;
    unsigned int _rok_ok           : 1;

    unsigned int wok_is_valid      : 1;
    unsigned int _wok_ok           : 1;

    unsigned int xok_is_valid      : 1;
    unsigned int _xok_ok           : 1;

    unsigned int fok_is_valid      : 1;
    unsigned int _fok_ok           : 1;

    unsigned int dir_is_valid      : 1;
    unsigned int _dir_ok           : 1;

    unsigned int link_is_valid     : 1;
    unsigned int _link_ok          : 1;

    unsigned int realpath_is_valid : 1;
    unsigned int _realpath_ok      : 1;

    unsigned int reserv            : 10;   // Here is reserv for flags and values
};
generate_descriptor(fileEntry,Relational);

#endif
