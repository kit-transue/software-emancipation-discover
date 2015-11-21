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
// file  projmerge.h
//   define classes and routines for file merges

#ifndef _projmerge_h
#define _projmerge_h

#include <objArr.h>

class appMarker;

extern void merge_exe(char *, char *);

// Three files base file, first copy, second copy which correspond to file 1, 2, and 3.
// 8 different cases: aaa is impossible
//                    ccc is "3 files differ"
//                    caa, cac, cca are deletion
//                    acc, aca, acc are insertion
//
// case 0: ACTION_ALL_THREE_DIFF ==> both persons do different changes
//    all three files differ
//    ====
//
// case 1: ACTION_DELETE_1 ==> both persons do the same deleteion
//    ====1
//    1:6c1
//      6
//    2:5a
//    3:5a
//
// case 2: ACTION_DELETE_2 ==> only person a do the deletion
//    ====2
//    1:6c
//    2:5a
//    3:6c
//       6
//
// case 3: ACTION_DELETE_3 ==> only person b do the deletion
//         IT NEEDS TO INTEGRATE THE CHANGES FROM FILE3 into FILE2
//    ====2
//    1:6c
//    2:6c
//       6
//    3:5a
//
// case 4: ACTION_INSERT_1 ==> both persons do the same insertion
//    ====1
//    1:5a
//    2:6c
//    3:6c
//       5.5
//
// case 5: ACTION_INSERT_2 ==> only person a do the insertion
//    ====2
//    1:6a
//    2:7c
//       6.5
//    3:6a
//
// case 6: ACTION_INSERT_3 ==> only person b do the insertion
//         IT NEEDS TO INTEGRATE THE CHANGES FROM FILE3 into FILE2
//    ====2
//    1:8a
//    2:8a
//    3:9c
//       8.9

enum MERGE_DIFF_ACTION
{
    ACTION_ALL_THREE_DIFF,
    ACTION_SAME_CHANGE,
    ACTION_DELETE_1,
    ACTION_DELETE_2,
    ACTION_DELETE_3,
    ACTION_INSERT_1,
    ACTION_INSERT_2,
    ACTION_INSERT_3,
    ACTION_REPLACE_2,
    ACTION_REPLACE_3,

    ACTION_NOT_INITIALIZED = -1     // initial value
};

enum MERGE_FILE_NUMBER
{
    MERGE_2_FILES,
    MERGE_3_FILES
};

typedef struct
{
    char diff_field_type;
    char diff_file_type;
    char action_type;
    int  diff_begin;
    int  diff_end;
    smtTree *ht1;
    smtTree *t1;
    smtTree *t2;
    appMarker *m1;
    appMarker *m2;

    // copy of a file2 subtree to be pasted into file1
    // set in merge_info::set_for_copy (...)
    // used in merge_info::insert_this_area (...);
    smtTree *to_paste;
} DIFF_DATA;

class merge_info
{
  public:
    merge_info();
    ~merge_info();

    void set_view(view *v0);
    void set_smt_info(char *file, smtHeader* h);
    void init_diff_data(int len, MERGE_FILE_NUMBER mfn);
    void set_diff_data(int idx, char diff_field_type, char diff_file_type,
                       int start, int ending);
    void get_diff_data(int idx, char *diff_field_type, char *diff_file_typ,
                       char *action, int *start = 0, int *end = 0);
    void set_action_type(int idx, char action_type);
    void highlight(int idx, int for_delete = 0);

    // type == 0 from paraset; type == 1 form -batch mode
    void delete_this_area(int idx, char type);

    void insert_this_area(int idx, merge_info &mo);
    void verify_idx(int *idx);
    void verify_tbllen(int *tbl_idx);
    void free_diff_data();
    void set_mark(int idx);
    void free_markers();

    // Creates and deletes smtHeader for copy from file2 to file1
    void set_for_copy (merge_info *);
    void unset_for_copy ();

    // fills arrays for smt_unite ()
    int fill_for_copy (int idx, objArr& nodes, objArr_Int& regions);

    // creates copy smtHeader
    void deep_copy (objArr& nodes, objArr_Int& regions);

    void update_the_title_of_view();
    void set_one_viewer(viewerShell *vsh, int idx); // vsh = 0 for merge 2 files
    smtHeader *load_one_file_token(char *file, char *ln, char *temp, projModule *pm, smtHeader *h);
    smtHeader *local_tokenize(char *ln, char *fn, fileLanguage lang,  projNode *proj);

    void set_module(projModule *pm);
    projModule *get_module();

    void set_local_tokenize_flag(char lt);
    char get_local_tokenize_flag();

    char *get_filename();

    int db_save();

    int get_len_diff_data();
    int is_conflict_section(int idx);

    view      *get_view();
    smtHeader *get_smtHeader();

    void set_title(char *fn, char *rev, int idx);
    char *get_title();

    void save_ver_num();
    void set_ver(char *ver);

  private:
    MERGE_FILE_NUMBER mfn;

    genString   name;
    genString   title;
    genString   ver;

    char        local_tokenize_flag;
    smtHeader   *smt_h;
    projModule  *pm;

    view        *vv;

    int         total_idx;
    DIFF_DATA   *diff_data;
    objArr      markers;
    appPtr      for_copy;
};

extern void open_views_for_three(viewerShell *);

#endif

/*
$Log: projmerge.h  $
Revision 1.4 1995/07/27 20:26:27EDT rajan 
Port
// Revision 1.14  1994/07/24  18:58:58  so
// Bug track: 7203,7903,7295
// fix bugs 7203,7903,7295
//
// Revision 1.13  1994/05/05  20:05:38  boris
// Bug track: 6794
// Fixed "slow merge" problem, and #6977 asyncronous save
//
// Revision 1.12  1993/12/22  13:45:32  so
// Bug track: 5337
// fix part for bug 5337
//
// Revision 1.11  1993/12/19  16:51:19  so
// Bug track: 5726, 5337
// fix bug 5726 and part of 5337
//
// Revision 1.10  1993/12/16  19:52:13  so
// Bug track: n/a
// move some static functions to be the member functions of class merge_info
//
// Revision 1.8  1993/12/14  12:36:28  so
// Bug track: 5337
// part of 5337
//
// Revision 1.7  1993/12/08  22:11:03  so
// Bug track: 5442, 5443
// fix bugs 5442 and 5443
//
// Revision 1.6  1993/12/06  19:43:37  so
// Bug track: n/a
// Some change in the user interface for 3 file merge
//
// Revision 1.5  1993/11/11  15:56:32  so
// Bug track: 5186 5211
// fix bug 5186 5211
//
// Revision 1.4  1993/10/08  17:43:09  so
// Bug track: 001
// for merge 2 files
//
// Revision 1.3  1993/10/07  20:35:52  so
// Bug track: 000
// partially for merge project. It supports full 3-files merge.
// This check-in will help to duplicate an UI bug which displays the incorrect
// file name in the viewer file name window.
//
// Revision 1.2  1993/08/04  17:51:25  so
// add "$Log: projmerge.h  $
// add "Revision 1.4 1995/07/27 20:26:27EDT rajan 
// add "Port
// Revision 1.14  1994/07/24  18:58:58  so
// Bug track: 7203,7903,7295
// fix bugs 7203,7903,7295
//
// Revision 1.13  1994/05/05  20:05:38  boris
// Bug track: 6794
// Fixed "slow merge" problem, and #6977 asyncronous save
//
// Revision 1.12  1993/12/22  13:45:32  so
// Bug track: 5337
// fix part for bug 5337
//
// Revision 1.11  1993/12/19  16:51:19  so
// Bug track: 5726, 5337
// fix bug 5726 and part of 5337
//
// Revision 1.10  1993/12/16  19:52:13  so
// Bug track: n/a
// move some static functions to be the member functions of class merge_info
//
// Revision 1.8  1993/12/14  12:36:28  so
// Bug track: 5337
// part of 5337
//
// Revision 1.7  1993/12/08  22:11:03  so
// Bug track: 5442, 5443
// fix bugs 5442 and 5443
//
// Revision 1.6  1993/12/06  19:43:37  so
// Bug track: n/a
// Some change in the user interface for 3 file merge
//
// Revision 1.5  1993/11/11  15:56:32  so
// Bug track: 5186 5211
// fix bug 5186 5211
//
// Revision 1.4  1993/10/08  17:43:09  so
// Bug track: 001
// for merge 2 files
//
// Revision 1.3  1993/10/07  20:35:52  so
// Bug track: 000
// partially for merge project. It supports full 3-files merge.
// This check-in will help to duplicate an UI bug which displays the incorrect
// file name in the viewer file name window.
//"
//
*/
