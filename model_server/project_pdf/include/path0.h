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
#ifndef _path0_h
#define _path0_h

#ifndef _path_h
#include <path.h>
#endif

enum DIR_NAME_TYPE
{
    CONST_STRING,
    MATCH_ALL,
    MATCH_PATERN
};

class Dir_Name_Info
{
  public:
    Dir_Name_Info(char const *name);
    ~Dir_Name_Info();

    void set_next(Dir_Name_Info *next);

    DIR_NAME_TYPE  get_type();
    char *   get_name();
    projFilter*    get_pf();
    Dir_Name_Info* get_next();

    void get_full_path_name(genString &fn);

    void dbg_dnip();

  private:
    DIR_NAME_TYPE  type;
    genString      name;
    projFilter     *pf;
    Dir_Name_Info  *next;
};

class Verify_Rule
{
  public:
    Verify_Rule(char *proj_name, char *phy_name, char *log_name, int line_num, int file_type, char const *fr, projMap *pm);
    ~Verify_Rule();

    int            get_line_num();
    int            get_file_type();
    int            get_proj_name_num();
    Dir_Name_Info* get_proj_name();
    int            get_phy_name_num();
    Dir_Name_Info* get_phy_name();
    int            get_log_name_num();
    Dir_Name_Info* get_log_name();
    char const *         get_family_relation();
    projMap*       get_pm();
    char           get_const_flag();
    int            get_const_level();

    void dbg_vrp();

  private:
    Dir_Name_Info *set_name(char *n);

    int           line_num;
    int           file_type;
    int           proj_name_num;
    Dir_Name_Info *proj_name;
    int           phy_name_num;
    Dir_Name_Info *phy_name;
    int           log_name_num;
    Dir_Name_Info *log_name;
    genString     family_relation;

    projMap       *pm;
    char          const_flag; // 1 for const string; 0 for matching pattern
    int           const_level; // for const_flag == 0
};

inline void chop_trail(char *s)
{
    if (s && *s)
        s[strlen(s)-1] = '\0';
}

inline int trailing_char (char const *s, char c)
{
    return (s && *s && s[strlen(s)-1] == c);
}

inline int leading_char (char const *s, char c)
{
    return (s && *s == c);
}

#endif
/*
$Log: path0.h  $
Revision 1.8 2000/07/07 08:12:55EDT sschmidt 
Port to SUNpro 5 compiler
// Revision 1.4  1994/01/21  16:49:52  so
// Bug track: 6010, 6064
// fix bugs 6010 and 6064
//
// Revision 1.3  1993/10/28  20:14:08  so
// Bug track: 4999, 5037, 5082
// fix bugs 4999, 5037, 5082
//
// Revision 1.2  1993/10/27  14:11:19  so
// Bug track: 5049 5050 5051
// fix bugs 5049, 5050, 5051
//
// Revision 1.1  1993/09/20  14:37:53  so
// Initial revision
//
*/
