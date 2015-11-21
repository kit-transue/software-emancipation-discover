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

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <cstdlib>
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#define _cLibraryFunctions_h

#include "machdep.h"
#include <general.h>
#include <autosubsys-macros.h> 
#include <autosubsys-weights.h>
#include <machdep.h>
#include <objOper.h>
#include <customize.h>
#include <path.h>
#include <smt.h>
#include <autosubsys.h>
#include <transaction.h>
#include <RTL_externs.h>
#include <genError.h>
#include <autosubsys-macros.h> 
#include <gtList.h>

#ifdef NEW_UI
#include <gRTListServer.h>
#include "../../DIS_ui/interface.h"
#undef printf
#endif

#include <genString.h>

static const char *file_keyword = "*subsysPrefs.";
static const char *exist_subsys[3] = {"Ignore Membership", "Exclude Contents", "Add to Existing"};
static const char *uiExist_subsys[3] = {"IgnoreMembership", "ExcludeContents", "AddToExisting"};

static const char *scale_names[11] = {"Function call",
                                "Data reference",
                                "Instance of",
                                "Argument type",
                                "Return type",
                                "Contains",
                                "Friend of",
                                "Subclass of",
                                "Member of",
                                "File Name Prefix",
                                "Logical Name Level"};

static const char *threshold_name = "Threshold";
static const char *filename_prefix_count = "FileName_Prefix_Count";
static const char *logicname_level_count = "LogicName_Level_Count";
static const char *existing_subsystems   = "ExistingSubsystems";
static const char *num_subsystem         = "NumberOfSubsystems";

static struct {
    const char *name;
    int  value;
    int  count;
} res[WEIGHT_COUNT];

#ifdef NEW_UI
extern Application* DISui_app;
#endif

static bool input_setting (const char *filename, const char *fileContents);
extern "C" {
    void autosubsys_set_weight(int, int);
    void autosubsys_set_count(int, int);
    void autosubsys_set_threshold(int);
    void autosubsys_set_disjoint(int);
    void autosubsys_set_subsys_count(int);
}

/*
 * At some point it would not be unreasonable to have each weight contain
 * both the forward and reverse relations, rather than have the first half
 * of the array contain the forward ones, and the second half the reverse
 * ones.
 *
 * If you insert new relations here, make sure the "outbound" reference
 * goes first (in the low half of the array) and the "inbound" reference goes
 * in the second half. Several things - including the dead-code analysis -
 * depend on this.
 *
 */

#define DEFWT(a,b,c,d,e)                \
   defaultweights[a].outgoing = b;              \
   defaultweights[a+NWEIGHTS/2].outgoing=c;     \
   defaultweights[a].count = e;         \
   defaultweights[a+NWEIGHTS/2].count=e;\
   defaultweights[a].m_weight=d;

static void setupdefaults() {
  static int isinitialized;
  if (isinitialized) return;
  DEFWT(AUS_FCALL,      true,   false,  10, -1);
  DEFWT(AUS_DATAREF,    true,   false,  10, -1);
  DEFWT(AUS_INSTANCE,   true,   false,  10, -1);
  DEFWT(AUS_ARGTYPE,    true,   false,  10, -1);
  DEFWT(AUS_RETTYPE,    true,   false,  10, -1);
  DEFWT(AUS_ELEMENT,    false,  true,   10, -1);
  DEFWT(AUS_FRIEND,     false,  false,  15, -1);
  DEFWT(AUS_SUBCLASS,   true,   false,  10, -1);
  DEFWT(AUS_MEMBER,     true,   false,  15, -1);
  DEFWT(AUS_FILENAME,   false,  false,  10, 0);
  DEFWT(AUS_LOGICNAME,  false,  false,  10, 0);
  isinitialized=1;
}

#ifdef NEW_UI

weight defaultweights[NWEIGHTS];
int threshold;
int num_subsys;
static int disjointflag=0;

extern "C" int autosubsys_get_threshold(void) {
    return threshold;
}
 
extern "C" void autosubsys_set_threshold(int t) {
    threshold = t;
}
 
extern "C" int autosubsys_get_disjoint(void) {
    return disjointflag;
}
 
extern "C" void autosubsys_set_disjoint(int d) {
    disjointflag = d;
}
 
extern "C" int autosubsys_get_weight(int which) {
    setupdefaults();
    return (which<0 ? -1 : defaultweights[which].m_weight);
}
 
extern "C" void autosubsys_set_weight(int which, int val) {
    setupdefaults();
    if (which>=0) {
      defaultweights[which].m_weight = val;
      defaultweights[which+NWEIGHTS/2].m_weight = val;
        /* fwd and reverse relations have same weight */
    }
}

extern "C" int autosubsys_get_count(int which) {
    setupdefaults();
    return (which<0 ? -1 : defaultweights[which].count);
}
 
extern "C" void autosubsys_set_count(int which, int val) {
    setupdefaults();
    if (which>=0) {
      defaultweights[which].count = val;
      defaultweights[which+NWEIGHTS/2].count = val;
    }
}
 
extern "C" int autosubsys_get_subsys_count() {
     return num_subsys;
}
 
extern "C" void autosubsys_set_subsys_count(int cnt) {
     num_subsys = cnt;
}
 
#endif


#ifdef NEW_UI
int Entity_filename_prefix_count()
{
    Initialize(Entity_filename_prefix_count);

    return defaultweights[AUS_FILENAME].count;
}
#endif /*NEW_UI*/

#ifdef NEW_UI
int Entity_logicname_count()
{
    Initialize(Entity_logicname_count);

    return defaultweights[AUS_LOGICNAME].count;
}
#endif /*NEW_UI*/


#ifdef NEW_UI
bool Entity_need_cmp_filename_prefix()
{
    Initialize(Entity_need_cmp_filename_prefix);

    if (defaultweights[AUS_FILENAME].count && defaultweights[AUS_FILENAME].m_weight)
        return true;
    return false;
}
#endif /*NEW_UI*/

#ifdef NEW_UI
bool Entity_need_cmp_logicname()
{
    Initialize(Entity_need_cmp_logicname);

    if (defaultweights[AUS_LOGICNAME].count && defaultweights[AUS_LOGICNAME].m_weight)      
        return true;
    return false;
}
#endif /*NEW_UI*/           

extern "C" void extract_save (  const char *dialogID,
				char *filename,
                   		int num_subs,
                   		int scale0,
                   		int scale1,
                   		int scale2,
                   		int scale3,
                   		int scale4,
                   		int scale5,
                   		int scale6,
                   		int scale7,
                   		int scale8,
                   		int scale9,
                   		int scale10,
                   		int file_val,
                   		int logical_val,
                   		int threshold,
                   		char *in_exist_subsys) {

//    Initialize(Extract::save);

    genString contents;
    genString temp;
    temp.printf("%s%s:%d\\n", file_keyword, num_subsystem,  num_subs); 
    contents += temp.str();
    temp.printf("%s%s:%d\\n", file_keyword, scale_names[0], scale0);
    contents += temp.str();
    temp.printf("%s%s:%d\\n", file_keyword, scale_names[1], scale1);
    contents += temp.str();
    temp.printf("%s%s:%d\\n", file_keyword, scale_names[2], scale2);
    contents += temp.str();
    temp.printf("%s%s:%d\\n", file_keyword, scale_names[3], scale3);
    contents += temp.str();
    temp.printf("%s%s:%d\\n", file_keyword, scale_names[4], scale4);
    contents += temp.str();
    temp.printf("%s%s:%d\\n", file_keyword, scale_names[5], scale5);
    contents += temp.str();

    int scale_idx = 6;
    if (customize::language_enabled(smt_CPLUSPLUS)) {
        temp.printf("%s%s:%d\\n", file_keyword, scale_names[6], scale6);
        contents += temp.str();
        temp.printf("%s%s:%d\\n", file_keyword, scale_names[7], scale7);
        contents += temp.str();
        temp.printf("%s%s:%d\\n", file_keyword, scale_names[8], scale8);
        contents += temp.str();
        scale_idx = 9;
    }

    temp.printf("%s%s:%d\\n", file_keyword, scale_names[scale_idx], scale9);
    contents += temp.str();
    temp.printf("%s%s:%d\\n", file_keyword, scale_names[scale_idx+1], scale10);
    contents += temp.str();

    temp.printf("%s%s:%d\\n", file_keyword, filename_prefix_count, file_val);
    contents += temp.str();
    temp.printf("%s%s:%d\\n", file_keyword, logicname_level_count, logical_val);
    contents += temp.str();

    temp.printf("%s%s:%d\\n", file_keyword, threshold_name, threshold);
    contents += temp.str();

    for (int i = 0; i < 3; i++) {
        if (strcmp (in_exist_subsys, uiExist_subsys[i]) == 0) {
            temp.printf("%s%s:%s\\n", file_keyword, existing_subsystems, exist_subsys[i]);
            contents += temp.str();
        }
    }

#ifdef NEW_UI
    genString command;
    command.printf ("dis_gdDialogIDEval {%s} [concat {dis_file_write} {%s} {[concat %s]}]",
                     dialogID, 
                     filename,
                     contents.str());
    rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
#endif
}

extern "C" void extract_restore (const char *filename, const char *fileContents, const char *dialogID) {

//    Initialize(Extract::restore);

    if (!input_setting(filename, fileContents)) return;
#ifdef NEW_UI
    genString command;
    command.printf ("dis_gdDialogIDEval {%s} {puts [eval [concat {loadDialog} %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d {%s}]]}", 
                     dialogID, 
                     res[AUS_NUM_SUBS].value,
                     res[AUS_FCALL].value,
                     res[AUS_DATAREF].value,
                     res[AUS_INSTANCE].value,
                     res[AUS_ARGTYPE].value,
                     res[AUS_RETTYPE].value,
                     res[AUS_ELEMENT].value,
                     res[AUS_FRIEND].value,
                     res[AUS_SUBCLASS].value,
                     res[AUS_MEMBER].value,
                     res[AUS_FILENAME].value,
                     res[AUS_LOGICNAME].value,
                     res[AUS_THRESHOLD].value,
                     res[AUS_FILE_PREFIX].value,
                     res[AUS_LOGNAM_CNT].value,
                     uiExist_subsys[res[AUS_EXIST_SUBS].value]);
    rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
#endif
}


//const char *Extract::getDefault () {
extern "C" const char *extract_getDefault () {

//    Initialize(Extract::getDefault);

    static genString fullname;
    const char *name = customize::subsys_setting_file(0);

    if (strlen(name)) {
        paraset_get_fullpath((char *)name, fullname);
    }

    return (fullname.str());
}


//void Extract::setDefault (const char *filename) {
extern "C" void extract_setDefault (const char *filename) {
    customize::subsys_setting_file (filename);
    customize::save_preferences ();
}


static RTLNode *undoSet = NULL;


//int Extract::is_CPP_enabled () {
extern "C" int extract_is_CPP_enabled () {
    return customize::language_enabled (smt_CPLUSPLUS);
}


static void remove_white_space(char **ret) {
    Initialize(remove_white_space);

    char *p = *ret;

    while(*p && *p != '\n')
    {
        if (!isspace(*p) && *p != '\13')
            break;
        p++;
    }
    *ret = p;
}


//
//  Function input_setting processes the named file and Sets up the 'res' array.
//

static bool input_setting (const char *filename, const char *fileContents) {
    Initialize(input_setting);
#ifdef NEW_UI

    genString temp = fileContents;

    if (temp.length() == 0) {
        genString command;
        command.printf ("dis_prompt {OKDialog} {Extract/sw} {File %s could not open for reading.}", filename);
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
        return false;
    }

    int i;
    for (i = 0; i < WEIGHT_COUNT; i++) {
        res[i].value = 0;
        res[i].count = 0;
        if (i <= AUS_LOGICNAME)
            res[i].name = scale_names[i];
        else if (i == AUS_THRESHOLD)
            res[i].name = threshold_name;
        else if (i == AUS_FILE_PREFIX)
            res[i].name = filename_prefix_count;
        else if (i == AUS_LOGNAM_CNT)
            res[i].name = logicname_level_count;
        else if (i == AUS_NUM_SUBS)
            res[i].name = num_subsystem;

        else // if (i == AUS_EXIST_SUBS)
            res[i].name = existing_subsystems;
    }

    char *line = (char *)temp.str();
    char *next_line = strchr (line, '\n');
    if (next_line) {
        *next_line = '\0';
        next_line++;
    }

    int line_num = 0;
    while (line && *line) {
        line_num++;

        //
        // found = 1 ==> succeed
        //         0 ==> keyword not matched
        //         2 ==> garbage at end of line
        //         3 ==> garbage at beginning of line
        //         4 ==> missing :
        //         5 ==> no digit after :
        //

        char found = 0;
        char *ret;
        for (i = 0; i < WEIGHT_COUNT; i++) {
            char *p = line;

            if (strncmp (p, file_keyword, strlen (file_keyword))) {

                found = 3;
                break;
            }

            p +=  strlen (file_keyword);
            if (strncmp (p, res[i].name, strlen (res[i].name)) == 0) {
                p += strlen (res[i].name);
                if (*p != ':') {
                    if (!strchr (p, ':')) found = 4;
                    break;
                }

                p++;
                if (i == WEIGHT_COUNT - 1) {
                    char fd = 0;
                    for (int j = 0; j < 3; j++) {
                        if (strncmp (p, exist_subsys[j], strlen (exist_subsys[j])) == 0) {
                            fd = 1;
                            ret = p + strlen(exist_subsys[j]);
                            remove_white_space(&ret);

                            if (*ret && *ret != '\n') {
                                found = 2;
                                break;
                            }

                            res[i].value = j;
                            break;
                        }
                    }

                    if (!fd || found == 2) break;
                }

                else {
                    if (*p == '\0' || *p == '\n') {
                        found = 5;
                        break;
                    }

                    res[i].value = (int)strtol(p, &ret, 10);
                    remove_white_space(&ret);
                    if (*ret && *ret != '\n') {
                        found = 2; // bad character at the end
                        break;
                    }
                }

                found = 1;
                res[i].count += 1;
                break;
            }
        }

        if (found != 1) {
            line[strlen(line) - 1] = '\0';

            if (!found) {
                genString command;
                command.printf ("dis_prompt {OKDialog} {Extract/sw} {File \"%s\" line no \"%d\": containes\n\"%s\"\nthat could not be analyzed. %s}", 
                                filename, line_num, line, "The restore process quits");
                rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            }

            else if (found == 2) {
                genString command;
                command.printf ("dis_prompt {OKDialog} {Extract/sw} {File \"%s\" line no \"%d\": containes some garbage \"%s\" at the end of line. %s}", 
                                filename, line_num, ret, "The restore process quits");
                rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            }

            else if (found == 3) {
                genString command;
                command.printf ("dis_prompt {OKDialog} {Extract/sw} {File \"%s\" line no \"%d\": contains some garbage \"%s\" at the beginning of line. %s}", 
                                filename, line_num, line, "The restore process quits");
                rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            }

            else if (found == 4) {
                genString command;
                command.printf ("dis_prompt {OKDialog} {Extract/sw} {File \"%s\" line no \"%d\": missing \":\". %s}", 
                                filename, line_num, "The restore process quits");
                rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            }

            else {
                genString command;
                command.printf ("dis_prompt {OKDialog} {Extract/sw} {File \"%s\" line no \"%d\": no digits after \":\". %s}", 
                                filename, line_num, "The restore process quits");
                rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            }

            return false;
        }

        line = next_line;
        if (line) {
            next_line = strchr (line, '\n');
            if (next_line) {
                *next_line = '\0';
                next_line++;
            }
        }
    }

    genString wrn;
    for (i = 0; i < WEIGHT_COUNT; i++) {
        if (!customize::language_enabled (smt_CPLUSPLUS) && (i >= AUS_FRIEND && i <= AUS_MEMBER)) continue;
        genString tmp;
        if (res[i].count != 1) {
            if (res[i].count == 0) {
                tmp.printf("\"%s\" is not defined.\n",res[i].name);
            }

            else {
                tmp.printf("\"%s\" is defined more than once; the later definition is used.\n",res[i].name);
            }
        }

        if (res[i].count) {
            if (i <= AUS_LOGICNAME) {
                if (res[i].value < 0 || res[i].value > 20) {
                    tmp.printf("The value %d for \"%s\" is out of range. It should be an integer between 0 and 20.\n", res[i].value, res[i].name);
                }
            }

            else if (i == AUS_THRESHOLD) {
                if (res[i].value < 0 || res[i].value > 100) {
                    tmp.printf("The value %d for \"%s\" is out of range. It should be an integer between 0 and 100.\n", res[i].value, res[i].name);
                }
            }

            else if (i == AUS_FILE_PREFIX || i == AUS_LOGNAM_CNT) {
                if (res[i].value < 0) {
                    tmp.printf("The value %d for \"%s\" should not be negative.\n", res[i].value, res[i].name);
                }
            }

            else if (i == AUS_NUM_SUBS) {
                if (res[i].value <= 0) {
                    tmp.printf("The value %d for \"%s\" should be positive.\n", res[i].value, res[i].name);
                }
            }
        }

        wrn += tmp.str();
    }

    if (wrn.length()) {
        genString command;
        command.printf ("dis_prompt {OKDialog} {Extract/sw} {%s}", wrn.str());
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }

#endif
    return true;
}
