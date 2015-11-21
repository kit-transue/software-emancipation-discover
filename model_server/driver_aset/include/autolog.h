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
#ifndef autolog_h
#define autolog_h

#include "fd_fstream.h"
#include "cLibraryFunctions.h"

#include "genString.h"

class autolog {
    genString hostname;
    genString username;
    genString filename;
    genString last_message;
    fd_ostream logfile;
    int lines_imported;
    int current_lines_imported;
    int files_imported;
    int current_files_imported;
    int lines_loaded;
    int current_lines_loaded;
    int files_loaded;
    int current_files_loaded;
    int open;

  public:
    autolog();
    ~autolog();

    void start();
    void end();
    void timestamp();
    void linefeed();
    void host_name();
    void user_name();
    void print_prefs(char*);
    void message(const char *string);
    void report_error(const char *);
    void import_lines(int);
    void load_lines(int);
    void import_complete(const char *);
    void load_text(const char *,int);

    int active();
 
  protected:
    void write(const char *string);
};

extern autolog *aset_log;


#define LOG_IMPORT(X) if(aset_log && aset_log->active()) aset_log->import_lines(X);
#define LOG_IMPORT_COMPLETE(X) if(aset_log && aset_log->active()) \
					aset_log->import_complete(X);
#define LOG_LOAD(buf,size) if(aset_log && aset_log->active()) aset_log->load_text(buf,size);
#define LOG_END if(aset_log && aset_log->active()) aset_log->end();
#define LOG_MESSAGE(X) if(aset_log && aset_log->active()) aset_log->message(X);

#endif

