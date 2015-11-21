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
#include <path.h>

#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <customize.h>
#include <psetmem.h>

#include <patherr.h>
#include <genArr.h>
#include <messages.h>
#include <objArr.h>
#include <objSet.h>
#include <driver_mode.h>

#define TWO_STAR_DEF TXT("\"**\" are allowed in the file name, the link definition, and the rule definition")

static genString project_file_name;
void patherr_report_project_file_name (char const *nm)
{
    project_file_name = nm;
}

char const *get_PDF()
{
    Initialize(get_PDF);

    return project_file_name;
}

// This flag will be set when the project file is parsed.
// The error will be in the project file.
static int projmap_parse_line_num = -1;
void patherr_projmap_parse_line_num (int ln)
{
    projmap_parse_line_num = ln;
}

int projmap_get_path_parse_line_num()
{
    Initialize(projmap_get_path_parse_line_num);

    if (projmap_parse_line_num >= 0)
        return projmap_parse_line_num;

    return get_path_parse_line_num();
}


// FOR parser error.
static genArrOf(pathParserErrEntryPtr) parse_err;

bool parser_locate_error()
{
    Initialize(parser_locate_error);

    for (int i=0;i<projMap::num_entries();i++)
    {
        const pathParserErrEntry *p = projMap::entry(i);
        if (p->err_code() >= PATH_PARSE_SYNTAX_ERR)
            return TRUE;
    }

    return FALSE;
}

void create_one_path_parser_error(unsigned int err_code, char const *str0, 
		char const *str1, int num0, int num1, int num2)
// display warnings or errors for pdf
//
// err_code: is enum defined in patherr.h for differenr warnings and errors
// str0, str1: are input messages
// num0, num1, num2: are input integers
{
    Initialize(create_one_path_parser_error);

    static unsigned int last_err_code = 999999;

    if (( customize::getIntPref("DIS_internal.PDFWarnings_Disabled") < 1) && 
        (err_code < PATH_PARSE_SYNTAX_ERR) )
        return;

    genString err_msg;

    switch(err_code)
    {
      case PATH_PARSE_NO_ERROR:
        break;
      case PATH_PARSE_ROOT_DIR_NOT_EXIST_WRN:
        if (num1)
            err_msg.printf(TXT("\"%s\", line %d: Root directory '%s'\nof project does not exist.\n%s\n%s\n"),
                           realOSPATH(project_file_name.str()),
                           num0, str1,
                           TXT("If this is a rule project, its directory is checked only when"),
                           TXT("some physical path of its child projects does not have a leading slash."));
        else
        {
            err_msg.printf(TXT("\"%s\", line %d: Root directory '%s'\nof project does not exist.\n"),
                           realOSPATH(project_file_name.str()),
                           num0, str1);
        }
        break;
      case PATH_PARSE_HOME_DIR_WRN:
        err_msg.printf(TXT("\"%s\", line %d: Cannot expand home directory for \"%s\"."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       str0);
        break;
      case PATH_PARSE_SYS_VAR_UNDEF_WRN:
        err_msg.printf(TXT("\"%s\", line %d: System variable \"%s\" is undefined."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       str0);
        break;
    case PATH_PARSE_ROOT_PROJ_FILE_WRN:
        err_msg.printf(TXT("\"%s\", line %d: Root of project is not a directory."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_TOPPROJ_LOGSLASH_WRN:
        err_msg.printf(TXT("\"%s\", line %d: The logical name of the topmost project should have a leading slash."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_TOPPROJ_PHYSLASH_WRN:
        err_msg.printf(TXT("\"%s\", line %d: The physical name of the topmost project should have a leading slash."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_SUB_PROJ_NAME_START_WITH_SLASH_WRN:
        err_msg.printf(TXT("\"%s\", line %d: Sub-project names may not start with slash (/)."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_SAME_PROJ_FILE_WRN:
        err_msg.printf(TXT("\"%s\" is specified more than once."),
                       realOSPATH(project_file_name.str()));
        break;
    case PATH_PARSE_READ_ONLY_SUPESEDE_WRITE_ONLY_WRN:
        err_msg.printf(TXT("\"%s\", line %d: [[W]] will be ignored because [[R]] is defined."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_SCRIPT_NOT_TOP_PROJ_WRN:
        err_msg.printf(TXT("\"%s\", line %d: The sub-project contains script %s"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       "which is only allowed in the top project and will be ignored.\n");
        break;
    case PATH_PARSE_SCRIPT_MORE_TOP_PROJ_WRN:
        err_msg.printf(TXT("\"%s\", line %d: The top project contains script %s"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       "and other SELECTORS which will be ignored.\n");
        break;
    case PATH_PARSE_SELECTOR_EMPTY_WRN:
        err_msg.printf(TXT("\"%s\", line %d: One selector is empty which will be ignored."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_3_STARS_WRN:
         err_msg.printf(
            TXT("\"%s\", line %d: Can not use more than 2 *'s in \"**\" specification,\nreplacing \"%s\" with \"**\"."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       str0);
        break;
    case PATH_PARSE_NO_IMPLICIT_PROJ_SUPPORT_WRN:
         err_msg.printf(
     TXT("\"%s\", line %d: warning: The implicit project \"=\" is no longer supported and may not work in the future."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_IGNORE_SELECTOR_WRN:
        err_msg.printf(TXT("\"%s\", line %d: %s%s"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       TXT("This pdf is written to flatten projects. \n"),
                       TXT("The selector defined in this line is ignored.\n"));
        break;
    case PATH_PARSE_FILTER_EMPTY_WRN:
        break;
    case PATH_PARSE_CANT_GEN_FILTER_PIPE_WRN:
        err_msg.printf(TXT("\"%s\", line %d: Could not create pipe.\n%s"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       TXT("Will use system() call instead, which will be a little slower.\n"));
        break;
    case PATH_PARSE_NESTED_PARENTHESES_WRN:
        err_msg.printf(TXT("\"%s\", line %d: It defines nested parentheses.\n%s"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       TXT("Only the outmost pair of parentheses will be mapped.\n"));
        break;
    case PATH_PARSE_MISS_EXEC_WRN:
        err_msg.printf(TXT("Executable cannot be found: %s"),
                       str0);
        break;
    case PATH_PARSE_MISS_PERCENT_WRN:
        err_msg.printf(TXT("\"%s\", line %d: Missed '%%' in the rule : %s"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       str0);
        break;
    case PATH_PARSE_LONG_EXPAND_WRN:
        err_msg.printf(TXT("\"%s\", line %d: The physical path contains \"%s\" which might take a very long time to expand."),
                       realOSPATH(project_file_name.str()),
                       num0,
                       str0);
        break;


    case PATH_PARSE_DUP_SCOPE_NAME_WRN:
    {
        genString ign;
        ign.printf(TXT("Scope_name \"%s\" in line %d will be ignored."),str0, num0);
        if (num2 == 0)
            err_msg.printf(TXT("\"%s\", line %d: Scope_name \"%s\" is identical to that in line %d.\n%s"),
                           realOSPATH(project_file_name.str()),
                           num0,
                           str0,
                           num1,
                           ign.str());
        else if (num2 == 1)
            err_msg.printf(TXT("\"%s\", line %d: Scope_name \"%s\" contains identical unit name(s) as in line %d.\n%s"),
                           realOSPATH(project_file_name.str()),
                           num0,
                           str0,
                           num1,
                           ign.str());
        else // if (num2 == 2)
            err_msg.printf(TXT("\"%s\", line %d: Scope_name \"%s\" does not contain any legal unit name.\n%s"),
                           realOSPATH(project_file_name.str()),
                           num0,
                           str0,
                           ign.str());
    }
        break;
    case PATH_PARSE_DUP_UNIT_NAME_WRN:
        err_msg.printf(TXT("\"%s\", line %d: Scope_name \"%s\" contains repeated unit name(s) \"%s\" which will be ignored."),
                           realOSPATH(project_file_name.str()),
                           num0,
                           str0,
                           str1);
        break;
    case PATH_PARSE_ILLEGAL_UNIT_NAME_WRN:
        if (num0 == -2) // from command line (a switch after -unit_scope)

        err_msg.printf(TXT("Command switch \"-unit_scope\" contains illegal unit name(s) \"%s\" which will be ignored."),
                           str1);
        else
        err_msg.printf(TXT("\"%s\", line %d: Scope_name \"%s\" contains illegal unit name(s) \"%s\" which will be ignored."),
                           realOSPATH(project_file_name.str()),
                           num0,
                           str0,
                           str1);
        break;


    case PATH_PARSE_SYNTAX_ERR:
    {
        genString dyn_msg = "";
        genString eq_msg = "";
        if (strcmp(str0,"=") == 0)
            eq_msg.printf(TXT("If it tries to define an \"=\" project, please use \":\" instead.\n%s\n%s\n"),
                          TXT("The \"=\" project is no longer supported."),
                          TXT("If it tries to define a variable, it should be on the top level."));;
        err_msg.printf(TXT("\"%s\", line %d: Syntax error. The last token is \"%s\".\n%s\n%s"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       str0,
                       dyn_msg.str(),
                       eq_msg.str());
    }
        break;
    case PATH_PARSE_UNKNOWN_SELECTOR_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Unknown selector \"%s\". Must be [[ d ]], [[ D ]], [[ f ]], or [[ F ]])."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       str0);
        break;
    case PATH_PARSE_BAD_SELECTOR_FOR_DIR_NODE_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Bad selector for directory. Must be either [[ d ]] or [[ D ]])."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_BAD_SELECTOR_FOR_FILE_NODE_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Bad selector for file. Must be either [[ f ]] or [[ F ]])."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_BIG_IDX_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Pattern (%d) of \"%s\" out of range in physical mapping, maximum (%d)."),
                       realOSPATH(project_file_name.str()),
                       num0,
                       num1,
                       str1,
                       num2);
        break;
    case PATH_PARSE_INCOMPLETE_LOG_TO_PHY_ERR:
    {
        int lno = num0;
        if (lno == 0)
            lno = projmap_get_path_parse_line_num();
        char const *name = "logical";
        if (str0)
            name = str0;
        err_msg.printf(TXT("\"%s\", line %d: It is impossible to match back from the %s name to physical name."),
                       realOSPATH(project_file_name.str()),
                       lno,
                       name);
    }
        break;
    case PATH_PARSE_MORE_PTN_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Too few patterns of \"%s\" used in physical mapping, missing (%d)."),
                       realOSPATH(project_file_name.str()),
                       num0,
                       str1,
                       num1);
        break;
    case PATH_PARSE_PROJ_FILE_NON_EXIST_ERR:
        err_msg.printf(TXT("\"%s\" does not exist."),
                       realOSPATH(project_file_name.str()));
        break;
    case PATH_PARSE_IMP_CHILD_SUB_PROJ_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Sub-project is not allowed."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_IMP_CHILD_NONHYBRID_ERR:
        err_msg.printf(TXT("\"%s\", line %d: This project is an implicit project with \"=\".\n%s"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       "All its sub-projects must be FILTER PROJECTS or none of its sub-projects could be FILTER PROJECT.");
        break;
    case PATH_PARSE_NO_PROJ_NAME_ERR:
        err_msg.printf(TXT("\"%s\", line %d: This project contains empty project name."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_SAME_PROJECT_NAME_ERR:
        if (str1 == 0)
        err_msg.printf(TXT("\"%s\", line %d: This project and the project in line %d share the same project name.\n"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       num1);
        else
        err_msg.printf(TXT("\"%s\", line %d: This project and the project in line %d of file \"%s\" share the same project name.\n"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       num1,
                       str1);
        break;
    case PATH_PARSE_DOUBLE_DOTS_ERR:
        err_msg.printf(TXT("\"%s\", line %d: It contains \"..\".\n"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_RULE_FAILED_ERR:
        if (last_err_code != PATH_PARSE_UNKNOWN_SELECTOR_ERR &&
            last_err_code != PATH_PARSE_BAD_SELECTOR_FOR_DIR_NODE_ERR &&
            last_err_code != PATH_PARSE_BAD_SELECTOR_FOR_FILE_NODE_ERR)
            err_msg.printf(TXT("\"%s\", line %d: The combination of file selectors makes this rule always fail.\n"),
                       realOSPATH(project_file_name.str()),
                       num0);
        else
            err_code = PATH_PARSE_NO_ERROR;
        break;
    case PATH_PARSE_OR_PATH_PHY_COUNT_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The count (%d) of ORed paths for the physical path\n%s%d%s%d%s"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       num1,
                       "should be equal to the larger of both the count (",
                       num0,
                       ") for the project name and\nthe count (",
                       num2,
                       ") for the logical name.\n");
        break;
    case PATH_PARSE_OR_PATH_PROJ_LOG_COUNT_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The count (%d) of ORed paths for the project name or\n%s%d%s%d%s"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       num0,
                       "the count (",
                       num2,
                       ") for the logic name should be zero, one, or the same as the count\n(",
                       num1,
                       ") for the physical path.\n");
        break;
    case PATH_PARSE_OR_PATH_DUP_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Two ORed paths in the %s are identical.\n"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       str0);
        break;
    case PATH_PARSE_LS_EMPTY_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The selector only contains \"ls\"."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_MISS_PARENTHESES_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Mismatched parentheses."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_2_STAR_IN_DIR_ERR:
        err_msg.printf(TXT("\"%s\", line %d: \"**\" should not be part of directory name.\n%s"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       TWO_STAR_DEF);
        break;
    case PATH_PARSE_SQUARE_BR_NOT_MATCH_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Mismatched square brackets.\n%s\n%s\n%s\n"),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num(),
                       TXT("ACTION: 1. Make square brackets matched. Or"),
                       TXT("        2. If you treat square brackets as normal characters,"),
                       TXT("           please insert back slash \"\\\" in front of them."));
        break;
    case PATH_PARSE_UNMATCH_DIRECTION_ERR:
err_msg.printf(TXT("\"%s\", line %d: The relation between the physical path and the logical name\n%s%s%s"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       TXT("for this sub-project is defined either LEFT_WAY \"<-\" or RIGHT WAY \"->\";\n"),
                       TXT("And one of its parent projects defines the opposit relation.\n"),
                       TXT("This combination cannot match any path.\n"));
        break;
    case PATH_PARSE_FLAT_BIG_IDX_ERR:
err_msg.printf(TXT("\"%s\", line %d: %spattern (%d) of \"%s\" name to be out of range in physical mapping, maximum (%d)."),
                       realOSPATH(project_file_name.str()),
                       num0,
                       TXT("This pdf is written to flatten projects, and causes that\n"),
                       num1,
                       str0,
                       num2);
        break;
    case PATH_PARSE_IDX_IN_FILTER_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Filter can't define (n) index fields.\n"),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_MATCH_IN_SCRIPT_ERR:
        err_msg.printf(TXT("\"%s\", line %d: \"%s\" can't define (*) matched fields.\n"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       str0);
        break;
    case PATH_PARSE_EMPTY_PARENTHESES_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Nothing between parentheses."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_PIPE_IN_PATH_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The 'or' operator '|' is not allowed on the top level of a physical path.\n"),
                       realOSPATH(project_file_name.str()),
                           num0);
        break;
    case PATH_PARSE_PIPE_IN_IDX_NAME_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The 'or' operator '|' is not allowed in the logical or project name."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_PHY_PATH_TOO_LONG_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The physical path is longer than %d characters."),
                       realOSPATH(project_file_name.str()),
                       num0,
                       MAXPATHLEN-1);
        break;
    case PATH_PARSE_TWO_PIPES_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The physical path contains\"||\" which must have text between them."),
                       realOSPATH(project_file_name.str()),
                       num0,
                       MAXPATHLEN-1);
        break;
    case PATH_PARSE_TWO_TWO_STARS_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The physical path contains more than one \"**\"."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_LEADING_TRAILING_SLASH_ERR:
        err_msg.printf(TXT("\"%s\", line %d: It does not allow a leading or trailing slash inside a matching pattern."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_EMPTY_PHY_PATH_ERR:
        err_msg.printf(TXT("\"%s\", line %d: It contains an empty physical path \"\"."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_DUMMY_PROJ_FOR_REL_SUB_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Dummy project for \"%s\". Please remove \"__\" and re-start DISCOVER"),
                       realOSPATH(project_file_name.str()),
                       num0,
                       str0);
        break;
    case PATH_PARSE_IDX_PARENS_NOT_DIGIT_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The logic name or project name contains some non-digits string inside the parentheses."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_DOT_ONLY_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The \".\" is not allowed in the physical path."),
                       realOSPATH(project_file_name.str()),
                       projmap_get_path_parse_line_num());
        break;
    case PATH_PARSE_TOP_PROJECT_NO_CHILD_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Top project should contain some sub-projects or modules."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_TOP_PROJECT_REAL_RIGHT_ERR:
        err_msg.printf(TXT("\"%s\", line %d: Top project cannot be a rule."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_EMPTY_PROJ_ERR:
        err_msg.printf(TXT("\"%s\", line %d: empty project \"{}\" is not allowed."),
                       realOSPATH(project_file_name.str()),
                       num0);
        break;
    case PATH_PARSE_NO_PROJ_ERR:
        err_msg.printf(TXT("\"%s\": No project is defined."),
                       realOSPATH(project_file_name.str()));
        break;
    case PATH_PARSE_PMOD_CONFLICT_ERR:
	err_msg.printf(TXT("\"%s\": pmod project conflict: proj %s, line %d *** proj2 %s, line %d"),
		       realOSPATH(project_file_name.str()), str0, num0, str1, num1);
	break;
    case PATH_PARSE_TOPPROJ_LOGSLASH_ERR:
        err_msg.printf(TXT("\"%s\", line %d: The logical name of the topmost project should have a leading slash."),
                       realOSPATH(project_file_name.str()),
                       num0);
	break;
    default:
        break;
    }
    last_err_code = err_code;
    if (err_code != PATH_PARSE_NO_ERROR)
        projMap::add_one_parser_error(err_code, err_msg);
}

void projMap::reset_parser_err()
{
    Initialize(projMap::reset_parser_err);

    for (int i=0;i<parse_err.size();i++)
        delete *parse_err[i];

    parse_err.reset();
}

genArrOf(pathParserErrEntryPtr) *projMap::get_parse_err()
{
    return &parse_err;
}

void projMap::add_one_parser_error(unsigned int err_code, char const *err_msg)
{
    Initialize(projMap::add_one_parser_error);

    pathParserErrEntryPtr *p = projMap::get_parse_err()->grow(1);
    pathParserErrEntry *s = db_new(pathParserErrEntry,(err_code, err_msg));
    *p = s;
}

const pathParserErrEntry *projMap::entry(unsigned int i)
{
    return *parse_err[i];
}

unsigned int projMap::num_entries()
{
    return parse_err.size();
}

int parser_report_errors()
{
    Initialize(parser_report_errors);

    int count = 0;
    for (int i=0;i<projMap::num_entries();i++) {
        const pathParserErrEntry *p = projMap::entry(i);
//      if (p->err_code() >= PATH_PARSE_SYNTAX_ERR) {
	    count ++;
	    char const *msg1 = p->err_msg();
	    if (msg1)
		msg("$1") << msg1 << eom;
//	}
	if (count)
	    ;
    }

    return count;
}


static objArr pmod_projs_arr;
static objSet pmod_projs_set;

void pmod_projs_init () { 
    pmod_projs_arr.removeAll() ; 
    pmod_projs_set.remove_all() ; 
}

void pmod_projs_insert (Obj *p) { 
    if (!pmod_projs_set.includes (p)) {
	pmod_projs_set.insert (p); 
	pmod_projs_arr.insert_last (p); 
    }
}

void pmod_projs_delete (Obj *p) { 
    if (pmod_projs_set.includes (p)) {
	pmod_projs_arr.remove (p);
	pmod_projs_set.remove (p);
    }
}

static projMap *unset_pmod_type_up (projMap *pm)
{
    Initialize(unset_pmod_type_up);

    projMap *err = pm;
    while ( (err = checked_cast(projMap,err->get_parent())) ) {
	int tp = err->get_pmod_type();
	if (tp != PMOD_NO && tp != PMOD_UNKNOWN)
	    break;
	    
	err->set_pmod_type(PMOD_NO);
    }
    return err;
}

static projMap *unset_pmod_type_down (projMap *pm, projMap *pmod)
{
    Initialize(unset_pmod_type_down);

    projMap *err = 0;
    if (pm != pmod) {
	int tp = pm->get_pmod_type();
	if (tp == PMOD_UNKNOWN)
	    pm->set_pmod_type(PMOD_NO);
	else if (tp != PMOD_NO)
	    err = pm;
    }

    if (err == 0) {
	for ( projMap *cur = checked_cast(projMap,pm->get_first()) ;
	     cur ;
	     cur = checked_cast(projMap,cur->get_next()) )
	    {
		if ( (err = unset_pmod_type_down (cur, pmod)) )
		    break;
	    }
    }
    return err;
}

static void report_pmod_error_conflict (projMap *frst, projMap *scnd)
{
    if (!frst | !scnd) return;

    char const *frst_name = frst->get_org_project_name();
    char const *scnd_name = scnd->get_org_project_name();
    int frst_line   = frst->get_line_num();
    int scnd_line   = scnd->get_line_num();

    create_one_path_parser_error(PATH_PARSE_PMOD_CONFLICT_ERR, frst_name, scnd_name, frst_line, scnd_line, 0);
}

static void set_default_pmods_recursive (projMap *cur)
{
    int tp = cur->get_pmod_type ();
    if (tp == PMOD_PDF)
	return;
    else if (tp == PMOD_UNKNOWN) {
	cur->set_pmod_type (PMOD_PDF);
	unset_pmod_type_down (cur, cur);
	return;
    }

    for (projMap *t = (projMap *)cur->get_first() ;
	 t ;
	 t = (projMap *)t->get_next())
	set_default_pmods_recursive (t);
}

// return 0 - success; 1 if there are more than one PMOD on a branch
int patherr_check_pmod_projs (projMap *rt)
{
    Initialize(patherr_check_pmod_projs);

    int status = 0;
    if (pmod_projs_arr.size()) {
	Obj *cur;
	ForEach(cur,pmod_projs_arr) {
	    projMap *pmod_pm  = checked_cast(projMap,cur);
	    projMap *err_down = 0;
	    projMap *err_up   = unset_pmod_type_up (pmod_pm);
	    if (err_up == 0)
		err_down = unset_pmod_type_down (pmod_pm, pmod_pm);

	    if (err_up || err_down) {
		if (err_up)
		    report_pmod_error_conflict (err_up, pmod_pm);
		else
		    report_pmod_error_conflict (pmod_pm, err_down);
    
		status = 1;
	    }
	}
    }

    if (status == 0) {
	while (rt) {
	    set_default_pmods_recursive (rt);
	    rt = (projMap *)rt->get_next();
	}
    }

    return status;
}

