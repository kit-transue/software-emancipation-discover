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
#ifndef _cpp_shared_h_
#define _cpp_shared_h_

#include "sharedArr.h"

#include "errorReport.h"

struct cpp_shared {
    void* htable;
    void* set_define;
    void* report_mcall;
    void* report_include;
    void* cpp_error_array;
    void* report_comment;
    void* obstack;
    char* firstobj;
};

struct setDefine {
    char* name;
    int l_name;
    char* body;
    int l_body;
    int fname;
    int lineno;
};

struct reportMcall {
    int val;
};

struct reportInclude {
    int includes;
    int included;
    int line_num;
};

struct reportComment {
    int b_file;
    int b_lnum;
    int e_file;
    int e_lnum;
};

sharedArr(setDefine);
sharedArr(reportMcall);
sharedArr(reportInclude);
sharedArr(reportComment);

extern sharedArrOf(setDefine)* set_define;
extern sharedArrOf(reportMcall)* report_mcall;
extern sharedArrOf(reportInclude)* report_include;
extern sharedArrOf(errorReport)* cpp_error_array;
extern sharedArrOf(reportComment)* report_comment;

#endif




