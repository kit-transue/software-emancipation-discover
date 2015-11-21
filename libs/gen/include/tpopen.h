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
// file  tpopen.h
//   define class for child process

#ifndef _tpopen_h
#define _tpopen_h

#define CMD_HEADER            "%%@"
#define CMD_PUT_BEGIN         "put-begin"
#define CMD_PUT_NO_FILE       "no-more-file"
#define CMD_PUT_END           "put-end"
#define CMD_PUT_SRC_PROJ_NAME "src_proj_name-"
#define CMD_PUT_DST_PROJ_NAME "dst_proj_name-"
#define CMD_PUT_SRC_FILE_NAME "src_file_name-"
#define CMD_PUT_DST_FILE_NAME "dst_file_name-"
#define CMD_PUT_SRC_PSET_NAME "src_pset_name-"
#define CMD_PUT_DST_PSET_NAME "dst_pset_name-"
#define CMD_PUT_MOD_NAME      "module_name-"
#define CMD_PUT_PDF_NAME      "pdf_name-"
#define CMD_PUT_OPT           "put_opt-"
#define CMD_PUT_COMMENT_BEGIN "comment_begin-"
#define CMD_PUT_COMMENT       "comment-"
#define CMD_PUT_COMMENT_END   "comment_END-"
#define CMD_REQUEST           "cmd_input"
#define CMD_SUCCESS           "update"
#define CMD_FAILURE           "failure"
#define CMD_ERR_MSG           "err_msg"
#define CMD_KILL              "kill"

class tpinstance
{
  public:
    class ui_INTERP_info* x_info;
    FILE *rfd;
    FILE *wfd;

    int fin();
};

tpinstance *tpopen (const char* command, char sync_flag);
extern void send_put_error_back_aset(const char* err);
extern void put_failure(const char* proj_name, const char* mod_name , const char* other_proj_name);

#endif

/*
$Log: tpopen.h  $
Revision 1.4 2000/04/04 10:00:19EDT sschmidt 
Port to HP 10.  Merging from /main/hp10port to mainline.
 * Revision 1.6  1994/07/19  21:53:34  so
 * Bug track: n/a
 * create pdf for put process
 *
 * Revision 1.5  1994/03/03  14:20:06  so
 * Bug track: 6578
 * fix bug 6578
 *
 * Revision 1.4  1994/02/24  16:06:48  so
 * Bug track: 6505
 * fix bug 6505
 *
 * Revision 1.3  1994/02/16  13:35:22  so
 * Bug track: n/a
 * new version of put
 *
*/
