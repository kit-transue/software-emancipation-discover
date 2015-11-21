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
/*
   file  patherr.h
   define enum for the warnings and errors in pdf, where
       warnings are from 0 to 99
       errors will be larger than 99

   and some other enums used for projMap
*/

#ifndef _patherr_h
#define _patherr_h
#include <prototypes.h>

enum
{
    PATH_PARSE_NO_ERROR,
    PATH_PARSE_IMP_CHILD_PARENTHESES_WRN,
    PATH_PARSE_ROOT_DIR_NOT_EXIST_WRN,
    PATH_PARSE_HOME_DIR_WRN,
    PATH_PARSE_SYS_VAR_UNDEF_WRN,
    PATH_PARSE_ROOT_PROJ_FILE_WRN,
    PATH_PARSE_TOPPROJ_LOGSLASH_WRN,
    PATH_PARSE_TOPPROJ_PHYSLASH_WRN,
    PATH_PARSE_SUB_PROJ_NAME_START_WITH_SLASH_WRN,
    PATH_PARSE_READ_ONLY_SUPESEDE_WRITE_ONLY_WRN,
    PATH_PARSE_SAME_PROJ_FILE_WRN,
    PATH_PARSE_SCRIPT_NOT_TOP_PROJ_WRN,
    PATH_PARSE_SCRIPT_MORE_TOP_PROJ_WRN,
    PATH_PARSE_SELECTOR_EMPTY_WRN,
    PATH_PARSE_3_STARS_WRN,
    PATH_PARSE_NO_IMPLICIT_PROJ_SUPPORT_WRN,
    PATH_PARSE_IGNORE_SELECTOR_WRN,
    PATH_PARSE_FILTER_EMPTY_WRN,
    PATH_PARSE_CANT_GEN_FILTER_PIPE_WRN,
    PATH_PARSE_NESTED_PARENTHESES_WRN,
    PATH_PARSE_MISS_EXEC_WRN,
    PATH_PARSE_MISS_PERCENT_WRN,
    PATH_PARSE_LONG_EXPAND_WRN,
    PATH_PARSE_DUP_SCOPE_NAME_WRN,
    PATH_PARSE_DUP_UNIT_NAME_WRN,
    PATH_PARSE_ILLEGAL_UNIT_NAME_WRN,
    PATH_PARSE_SYNTAX_ERR = 100,
    PATH_PARSE_UNKNOWN_SELECTOR_ERR,
    PATH_PARSE_BAD_SELECTOR_FOR_DIR_NODE_ERR,
    PATH_PARSE_BAD_SELECTOR_FOR_FILE_NODE_ERR,
    PATH_PARSE_BIG_IDX_ERR,
    PATH_PARSE_INCOMPLETE_LOG_TO_PHY_ERR,
    PATH_PARSE_MORE_PTN_ERR,
    PATH_PARSE_PROJ_FILE_NON_EXIST_ERR,
    PATH_PARSE_IMP_CHILD_NONHYBRID_ERR,
    PATH_PARSE_IMP_CHILD_SUB_PROJ_ERR,
    PATH_PARSE_NO_PROJ_NAME_ERR,
    PATH_PARSE_SAME_PROJECT_NAME_ERR,
    PATH_PARSE_DOUBLE_DOTS_ERR,
    PATH_PARSE_RULE_FAILED_ERR,
    PATH_PARSE_OR_PATH_PHY_COUNT_ERR,
    PATH_PARSE_OR_PATH_PROJ_LOG_COUNT_ERR,
    PATH_PARSE_OR_PATH_DUP_ERR,
    PATH_PARSE_LS_EMPTY_ERR,
    PATH_PARSE_MISS_PARENTHESES_ERR,
    PATH_PARSE_2_STAR_IN_DIR_ERR,
    PATH_PARSE_SQUARE_BR_NOT_MATCH_ERR,
    PATH_PARSE_UNMATCH_DIRECTION_ERR,
    PATH_PARSE_FLAT_BIG_IDX_ERR,
    PATH_PARSE_IDX_IN_FILTER_ERR,
    PATH_PARSE_MATCH_IN_SCRIPT_ERR,
    PATH_PARSE_EMPTY_PARENTHESES_ERR,
    PATH_PARSE_PIPE_IN_IDX_NAME_ERR,
    PATH_PARSE_PIPE_IN_PATH_ERR,
    PATH_PARSE_PHY_PATH_TOO_LONG_ERR,
    PATH_PARSE_TWO_PIPES_ERR,
    PATH_PARSE_TWO_TWO_STARS_ERR,
    PATH_PARSE_LEADING_TRAILING_SLASH_ERR,
    PATH_PARSE_EMPTY_PHY_PATH_ERR,
    PATH_PARSE_DUMMY_PROJ_FOR_REL_SUB_ERR,
    PATH_PARSE_IDX_PARENS_NOT_DIGIT_ERR,
    PATH_PARSE_DOT_ONLY_ERR,
    PATH_PARSE_TOP_PROJECT_NO_CHILD_ERR,
    PATH_PARSE_TOP_PROJECT_REAL_RIGHT_ERR,
    PATH_PARSE_EMPTY_PROJ_ERR,
    PATH_PARSE_PMOD_CONFLICT_ERR,
    PATH_PARSE_TOPPROJ_LOGSLASH_ERR,

    PATH_PARSE_NO_PROJ_ERR
};

enum
{
    PATH_TRAN_NO_ERROR,
    PATH_TRAN_REJECT_BY_PROJ_NAME,
    PATH_TRAN_REJECT_BY_LINK_MAP,
    PATH_TRAN_NO_MAPPING
};

#ifdef __cplusplus
EXTERN void create_one_path_parser_error PROTO((unsigned int err_code, 
				const char *str0, const char *str1 = 0, 
				int num0 = 0, int num1 = 0, int num2 = 0));
class Obj;
class projMap;

void patherr_report_project_file_name (const char *);
int projmap_get_path_parse_line_num();
void patherr_projmap_parse_line_num (int);
int parser_report_errors();
void pmod_projs_init ();
int patherr_check_pmod_projs (projMap *);
bool parser_locate_error();
void pmod_projs_delete (Obj *);
void pmod_projs_insert (Obj *);
#endif

#endif
