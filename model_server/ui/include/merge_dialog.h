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
#ifndef _merge_dialog_h
// file  merge_dialog.h
//   define some declarations for merge

#define _merge_dialog_h

#define BASE_FILE    "Base File:"
#define MY_BRANCH    "My Branch:"
#define OTHER_BRANCH "Other Branch:"

extern int merge_change_title_flag;
extern int merge_in_process();

extern void merge_dialog();
extern int merge_file_type(char const *file);
// type == 0 from paraset
// type == 1 form -batch mode
extern int merge_2_files(char *file1, char *file2, char type, char unload_flag, int &do_not_save_flag);
extern int being_merged(app *cur);
extern int get_cmt_idx(char *cmt);
extern "C" void merge_3_files(char *fn_base, char *fn, char *fn_other, char *rev0, char *rev);

extern void merge_3f_prev();
extern void merge_3f_curr();
extern void merge_3f_next();
extern void merge_3f_done();
extern void merge_3f_cancel();
extern void merge_3f_status();
extern void merge_3f_set_travel_only_conflict(int f);
extern int  merge_3f_get_travel_only_conflict();
extern int  merge_3f_get_conflict_num();

#endif

/*
$Log: merge_dialog.h  $
Revision 1.3 1994/09/16 18:34:41EDT so 
Bug track: n/a
fix for adac
// Revision 1.9  1993/12/19  16:58:47  so
// Bug track: 5726, 5337
// fix bug 5726 and part of 5337
//
// Revision 1.8  1993/12/17  14:43:02  so
// Bug track: 5688
// fix bug 5688
//
// Revision 1.7  1993/12/16  19:56:38  so
// Bug track: n/a
// move some static functions to be the member functions of class merge_info
//
// Revision 1.6  1993/11/30  21:22:26  so
// Bug track: n/a
// add help text button
// do not unload the file which is being merged
//
// Revision 1.5  1993/11/27  15:32:56  so
// Bug track: 5337
// fix part of bug 5337
// It could not unload the file which is in the process of merging.
//
// Revision 1.4  1993/11/11  16:04:50  so
// Bug track: 5186 5211
// fix bug 5186 5211
//
// Revision 1.3  1993/11/06  22:16:08  so
// Bug track: n/a
// merge project
//
// Revision 1.2  1993/08/04  17:56:30  so
// add "$Log: merge_dialog.h  $
// add "Revision 1.3 1994/09/16 18:34:41EDT so 
// add "Bug track: n/a
// add "fix for adac
// Revision 1.9  1993/12/19  16:58:47  so
// Bug track: 5726, 5337
// fix bug 5726 and part of 5337
//
// Revision 1.8  1993/12/17  14:43:02  so
// Bug track: 5688
// fix bug 5688
//
// Revision 1.7  1993/12/16  19:56:38  so
// Bug track: n/a
// move some static functions to be the member functions of class merge_info
//
// Revision 1.6  1993/11/30  21:22:26  so
// Bug track: n/a
// add help text button
// do not unload the file which is being merged
//
// Revision 1.5  1993/11/27  15:32:56  so
// Bug track: 5337
// fix part of bug 5337
// It could not unload the file which is in the process of merging.
//
// Revision 1.4  1993/11/11  16:04:50  so
// Bug track: 5186 5211
// fix bug 5186 5211
//
// Revision 1.3  1993/11/06  22:16:08  so
// Bug track: n/a
// merge project
//"
//
*/
