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
#ifndef __shell_calls__h
#define __shell_calls__h

#ifndef CONST
#ifdef __cplusplus
#define CONST const
#else
#define CONST
#endif
#endif

#define END_ARG_LIST (char *)0

int shell_mkdir(CONST char *dir, int force = 1);
int shell_rmdir(CONST char *);
int shell_rm(CONST char *);
int shell_touch(CONST char *);
int shell_cd_exec (const char *dir, const char *arg0 ...);
int shell_cd_exec_touch_background (const char *dir, const char *exe, const char *out_fn, const char *touch_fn);
int shell_cd_exec_redirect (const char *out_fn, const char *dir, const char *arg0 ...);
int shell_mv(CONST char *from, CONST char *to, int force = 0);
int shell_cp(CONST char *from, CONST char *to);
int shell_cd_ls (const char *out_fn, const char *dir, int argc, const char **ls_argv);
int vsystemv(int argc, const char** argv);
int vsystemv_redirect(const char *std_out, int argc, const char** argv);
int vsystemv_command_redirect(const char *std_out, const char *arg0, int argc, const char** argv);
int vsysteml(const char*arg0 ...);
int vsysteml_redirect (const char *std_out, const char*arg0 ...);
int vsysteml_redirect_stderr (const char *std_out, const char *std_err, const char*arg0 ...);

int shell_sort_unique ( const char *src, const char *sorted );
int shell_cd_find (const char *out_fn, const char *dir, const char *find_arg, int do_sort);
int shell_change_to_writable (const char *fname);
int shell_indent (const char * old_fn, const char * new_fn);

#endif
