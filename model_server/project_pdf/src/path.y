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
%{
#define YYDEBUG 1
#include <stdio.h>
#include "patherr.h"

#ifndef _pathdef_h
#include "pathdef.h"
#endif

#if 0
#define my_print(a,b) fprintf(a,b)
#else
#define my_print(a,b)
#endif

static char *projn;
static char *phyn;
static char *logn;
static char *groupn;
static char *var;
static char *val;
static char *rule;
int inside_proj_count = 0;

static void assign_proj_name();
static void assign_phy_name();
static void assign_log_name();
static void assign_rule();
static void assign_var();
static void assign_val();
static void assign_group_name();
static void my_setenv();

#if 0  // XXX
#define YYSTYPE YYSTYPE_DEF
#endif

%}

%union {
    char *string;
}

%token LEFT_WAY_SYMBOL RIGHT_WAY_SYMBOL TWO_WAY_SYMBOL NULL_TWO_WAY_SYMBOL <string> WILDCARD LEFT_CURLY_SQ RIGHT_CURLY_SQ
%token <string> RIGHT_WAY_REALPATH_SYMBOL SELECTOR BAD_TOKEN TOP_EXCLUDE_PROJ

%type <string> groupname
%%

        proj_specs: proj_spec
                    { my_print(stderr,"proj_specs: proj_spec\n");
                      close_proj(); }
                  | proj_specs proj_spec
                    { my_print(stderr,"proj_specs: proj_specs proj_spec\n"); }
                  ;

         proj_spec: proj_def
                    { my_print(stderr,"proj_spec: proj_def\n"); }
                  | proj_def LCS member_list RCS
                    { my_print(stderr,"proj_spec: proj_def '{' member_list '}'\n"); }
                  | var_def
                    { my_print(stderr,"proj_spec: var_def\n"); }
                  | top_ex_proj
                    { my_print(stderr,"proj_spec: top_ex_proj\n"); }
                  ;

          proj_def: named_proj
                    { my_print(stderr,"proj_def: named_proj\n"); }
                  | phy_name LEFT_CURLY_SQ
                    { my_print(stderr,"proj_def: phy_name '{'\n");
                      crt_whole_proj(0,phyn,0, NO_IMPLICIT_DIR_PROJ);
                      pathunput('{'); }
                  ;

        named_proj: proj_name ':' unnamed_proj
                    { my_print(stderr,"named_proj: proj_name ':' unnamed_proj\n");
                      crt_proj(projn, NO_IMPLICIT_DIR_PROJ); }
                  ;

      unnamed_proj: phy_name tran_symbol logic_name
                    { my_print(stderr,"unnamed_proj: phy_name tran_symbol logic_name\n");
                      save_phy_logic_names(phyn,logn);
                    }
                  | phy_name rule_right_part
                    { my_print(stderr,"unnamed_proj: phy_name rule_right_part\n");
                      save_phy_logic_names(phyn,rule);
                    }
                  ;

          phy_name: groupname
                    { my_print(stderr,"phy_name: groupname\n");
                      assign_phy_name();
                    }
                  | groupname selector_list
                    { my_print(stderr,"phy_name: groupname selector_list\n");
                      assign_phy_name();
                    }
                  ;

         proj_name: groupname
                    { my_print(stderr,"proj_name: groupname\n");
                      assign_proj_name();
                    }
                  ;

       tran_symbol: TWO_WAY_SYMBOL
                    { my_print(stderr,"tran_symbol: TWO_WAY_SYMBOL\n");
                      save_tran_flag(TWO_WAY); }
                  | LEFT_WAY_SYMBOL
                    { my_print(stderr,"tran_symbol: LEFT_WAY_SYMBOL\n");
                      save_tran_flag(LEFT_WAY); }
                  | RIGHT_WAY_SYMBOL
                    { my_print(stderr,"tran_symbol: RIGHT_WAY_SYMBOL\n");
                      save_tran_flag(RIGHT_WAY); }
                  ;

        rule_right_part: RIGHT_WAY_REALPATH_SYMBOL
                    { my_print(stderr,"rule_right_part: RIGHT_WAY_REALPATH_SYMBOL\n");
                      save_tran_flag(REAL_RIGHT_WAY);
                      assign_rule($1);
                    }
                   ;

        logic_name: groupname
                    { my_print(stderr,"logic_name: groupname\n");
                      assign_log_name();
                    }
                  ;

         groupname: WILDCARD
                    {my_print(stderr,"groupname: WILDCARD\n");
                     assign_group_name($1);
                    }

       member_list: member
                    { my_print(stderr,"member_list: member\n"); }
                  | member_list member
                    { my_print(stderr,"member_list: member_list member\n"); }
                  ;

            member: groupname
                    { my_print(stderr,"member: WILDCARD\n");
                      add_member($1, 1); }
                  | groupname selector_list
                    { my_print(stderr,"member: WILDCARD selector_list\n");
                      add_member($1, 1); }
                  | proj_spec
                    { my_print(stderr,"member: proj_spec\n"); }
                  ;

               LCS: LEFT_CURLY_SQ
                    { my_print(stderr,"'{': LEFT_CURLY_SQ\n");
                      inc_level(0, inside_proj_count); 
                      inside_proj_count++; }
                  ;

               RCS: RIGHT_CURLY_SQ
                    { my_print(stderr,"'}': RIGHT_CURLY_SQ\n");
                      inside_proj_count--;
                      dec_level(0, inside_proj_count); }
                  ;


     selector_list: selector_list selector
                    { my_print(stderr,"selector_list: selector_list selector\n"); }
                  | selector
                    { my_print(stderr,"selector_list: selector\n"); }

          selector: SELECTOR
                    { my_print(stderr,"selector: SELECTOR\n");
                      set_proj_file_type($1); }

               var: groupname
                    { my_print(stderr,"var: groupname\n");
                      assign_var(); }

               val: groupname
                    { my_print(stderr,"val: groupname\n");
                      assign_val(); }

           var_def: var '=' val
                    { my_print(stderr,"var_def: WILDCARD '=' WILDCARD\n");
                      my_setenv(var, val); }

       top_ex_proj: TOP_EXCLUDE_PROJ
                    { my_print(stderr,"top_ex_proj: TOP_EXCLUDE_PROJ\n");
                      save_top_exclude_proj($1); }

         unit_list: unit
                    { my_print(stderr,"unit_list: unit\n");
                      save_unit(groupn, 0); }
                  | unit_list unit
                    { my_print(stderr,"unit_list: unit_list unit\n");
                      save_unit(groupn, 1); }

              unit: groupname
                    { my_print(stderr,"unit: WILDCARD\n"); }

%%

int pathwrap()
{
    return 1;
}

static void assign_proj_name()
{
    projn = groupn;
}

static void assign_phy_name()
{
    phyn = groupn;
}

static void assign_log_name()
{
    logn = groupn;
}

static void assign_var()
{
    var = groupn;
}

static void assign_val()
{
    val = groupn;
}

static void assign_group_name(s1)
char *s1;
{
    groupn = s1;
}

static void assign_rule(s1)
char *s1;
{
    rule = s1;
}

static void combine_groupname(s1)
char *s1;
{
    char *line_no = s1 + strlen(s1) + 1;
    char *q;

    char *p = (char *)malloc(strlen(s1) + 1 + strlen(groupn) + 1 + strlen(line_no) + 1);
    sprintf(p,"%s|%s",s1,groupn);

    q = p + strlen(p) + 1;
    strcpy(q,line_no);

    free(s1);
    free(groupn);

    groupn = p;
}

extern char *path_sys_vars_translate();
static void my_setenv(name, val)
char *name;
char *val;
{
    char *ret_val = path_sys_vars_translate(val);
    int len = strlen(name) + strlen(ret_val) + 2;
    char *cmd = (char *)malloc(len);
    sprintf(cmd,"%s=%s",name,ret_val);
    free(ret_val);
    putenv(cmd);
}

#if 0
main(argc,argv)
int  argc;
char **argv;
{
FILE *in;
extern FILE *yyin;

    if (argc != 2)
    {
        fprintf(stderr,"Please type a.out file_name\n");
        exit(0);
    }

    in = fopen(argv[1],"r");
    if (in <= 0)
    {
        fprintf(stderr,"file %s does not exist.\n",argv[1]);
        exit(0);
    }
    yyin = in;
    yyparse();
}
#endif

#if 0
          proj_def: named_proj
                    { my_print(stderr,"proj_def: named_proj\n"); }
                  | unnamed_proj
                    { my_print(stderr,"proj_def: unnamed_proj\n"); }
                  | phy_name '{'
                    { my_print(stderr,"proj_def: phy_name '{'\n");
                      pathunput('{'); }
                  ;

        named_proj: proj_name ':' unnamed_proj
                    { my_print(stderr,"named_proj: proj_name ':' unnamed_proj\n"); }
                  | proj_name '=' unnamed_proj
                    { my_print(stderr,"named_proj: proj_name '=' unnamed_proj\n"); }
                  | proj_name ':' phy_name
                    { my_print(stderr,"named_proj: proj_name ':' phy_name\n"); }
                  | proj_name '=' phy_name
                    { my_print(stderr,"named_proj: proj_name '=' phy_name\n"); }
                  | proj_name ':' '{'
                    { my_print(stderr,"named_proj: proj_name ':' '{'\n");
                      pathunput('{'); }
                  ;

      unnamed_proj: phy_name TWO_WAY_SYMBOL logic_name
                    { my_print(stderr,"unnamed_proj: phy_name TWO_WAY_SYMBOL logic_name\n"); }
                  | NULL_TWO_WAY_SYMBOL logic_name
                    { my_print(stderr,"unnamed_proj: NULL_TWO_WAY_SYMBOL logic_name\n"); }
                  | phy_name TWO_WAY_SYMBOL '{'
                    { my_print(stderr,"unnamed_proj: phy_name TWO_WAY_SYMBOL '{'\n");
                      pathunput('{'); }
                  ;
#endif

/*
$Log: path.y  $
Revision 1.11 2002/01/24 10:00:37EST ktrans 
PDF scope cleanup
 * Revision 1.13  1994/04/05  11:25:28  so
 * phase 1 of atria cm
 *
 * Revision 1.12  1994/02/18  00:36:04  builder
 * Port
 *
 * Revision 1.10  1993/11/16  22:19:32  so
 * Bug track: 5155
 * fix bug 5155
 * A new feature '|' for the pdf file.
 *
 * Revision 1.9  1993/10/19  20:51:07  so
 * Bug track: 5006
 * fix bug 5006, 5015, 4839
 *
 * Revision 1.8  1993/09/24  21:50:13  so
 * Bug track: 4243
 * New feature
 * PDF needs to handle copy of the file instead of link
 *
 * Revision 1.7  1993/05/19  15:48:54  so
 * implement filter for implicit project.
 *
 * Revision 1.6  1993/05/05  23:36:38  so
 * => will not pre_pend the path for the project name and the logical name.
 *
 * Revision 1.5  1993/03/05  23:20:00  so
 * implement link map [[ -> ]] and [[ => ]]
 *
 * Revision 1.4  1993/02/08  22:56:56  so
 * Implement simpler format.
 *
 * Revision 1.3  1993/01/07  23:03:40  so
 * mv yyerror to path.l
 *
 * Revision 1.2  1992/12/21  17:36:58  so
 * 2nd revision
 *
 * Revision 1.1  1992/12/19  21:23:23  so
 * Initial revision
 *
*/
