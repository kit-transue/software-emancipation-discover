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
#include <pdf_selector.h>
#include <genArr.h>
#include <path.h>
#include <genString.h>
#include <patherr.h>

genArrOf(selector_entry) multi_selectors;

extern "C" void init_multi_selectors()
{
    multi_selectors.reset();
}

static int no_space(char *p)
{
    Initialize(no_space);

    while(*p && !isspace(*p))
        p++;

    if (*p == '\0')
        return 1;

    return 0;
}

char selector_analyze(char *selector, genString &cnt)
// analyze a selector
//
// selector: the input string [[ ... ]]
//
// cnt: the content of the selector
//
// returne = EMPTY_TYPE: [[ ls ]] or [[ filter ]]
//         = PURE_FILE_TYPE: [[ dDfF ]]
//         = LINK_MAP_FILE_TYPE: [[ -> ... ]]
//         = REALPATH_MAP_FILE_TYPE: [[ => ... ]]
//         = SEARCH_FILE_TYPE: [[ ls ... ]]
//         = SCRIPT_TYPE: a filter
//         = FILTER_TYPE: a script
//         = EQUAL_EVAL     : [[ == (1) (2) ]]
//         = NOT_EQUAL_EVAL : [[ != (1) (2) ]]
{
    Initialize(selector_analyze);

    selector += 2; // skip [[
    while(isspace(*selector))
        selector++;

    int len = strlen(selector);
    char *p = selector + len - 1 - 2; // remove the tailing ]]
    for (int i=0;i<len-2;i++)
    {
        if (isspace(*p))
            p--;
        else
           break;
    }
    *(p+1) = '\0';

    if (strlen(selector) == 0)
    {
        create_one_path_parser_error(PATH_PARSE_SELECTOR_EMPTY_WRN, 0, 0, 0, 0);
        return EMPTY_TYPE;
    }

    genString buf = selector;

    p = (char *)buf.str();

    char type;
    if (strncmp(p,"->",2) == 0)
    {
        type = LINK_MAP_FILE_TYPE;
        p += 2;
    }
    else if (strncmp(p,"=>",2) == 0)
    {
        type = REALPATH_MAP_FILE_TYPE;
        p += 2;
    }
    else if (strncmp(p,"ls",2) == 0)
    {
        type = SEARCH_FILE_TYPE;
        p += 2;
    }
    else if (strncmp(p,"!= ", 3) == 0)
    {
        type = NOT_EQUAL_TYPE;
        p += 2;
        while(*p && isspace(*p))
            p++;

        if (*p == '\0')
        {
            create_one_path_parser_error(PATH_PARSE_FILTER_EMPTY_WRN, 0, 0, 0, 0);
            return EMPTY_TYPE;
        }
        cnt = p;
        return type;
    }
    else if (strncmp(p,"== ", 3) == 0)
    {
        type = EQUAL_TYPE;
        p += 2;
        while(*p && isspace(*p))
            p++;

        if (*p == '\0')
        {
            create_one_path_parser_error(PATH_PARSE_FILTER_EMPTY_WRN, 0, 0, 0, 0);
            return EMPTY_TYPE;
        }
        cnt = p;
        return type;
    }
    else if (strncmp(p,"tcl ", 4) == 0)
    {
        type = TCL_BOOLEAN_TYPE;
        p += 3;
        while(*p && isspace(*p))
            p++;

        if (*p == '\0')
        {
            create_one_path_parser_error(PATH_PARSE_FILTER_EMPTY_WRN, 0, 0, 0, 0);
            return EMPTY_TYPE;
        }
        cnt = p;
        return type;
    }
    else if (strncmp(p,"filter ", 7) == 0)
    {
        type = FILTER_TYPE;
        p += 6;
        while(*p && isspace(*p))
            p++;
        if (*p == '\0')
        {
            create_one_path_parser_error(PATH_PARSE_FILTER_EMPTY_WRN, 0, 0, 0, 0);
            return EMPTY_TYPE;
        }
        cnt = p;
        return type;
    }
    
    else if (strncmp(p,"flist ", 6) == 0)
    {
        type = FILTER_TYPE;
        cnt = p;
        return type;
    }
    else if (strncmp(p,"elist ", 6) == 0)
    {
        type = FILTER_TYPE;
        cnt = p;
        return type;
    }
    else if (strncmp(p,"pmod", 4) == 0)
    {
        type = PMOD_TYPE;
        cnt = selector;
        return type;
    }
    else if (strncmp(p,"i", 1) == 0)
    {
        type = INCLUDED_TYPE;
        cnt = selector;
        return type;
    }
    else
        type = PURE_FILE_TYPE;

    if (type == PURE_FILE_TYPE)
    {
        int script_flag = 0;
        if (!no_space(p))
            script_flag = 1;
        else
        {
            char *q = p;
            while(*q)
            {
                if (strchr("dDfFRW",*q) == 0)
                {
                    script_flag = 1;
                    break;
                }
                q++;
            }
        }
        if (script_flag)
        {
            type = SCRIPT_TYPE;
            cnt = selector;
        }
        else
            cnt = p; // [[ dDfFRW ]]
    }
    else
    {
        while(*p && isspace(*p))
            p++;
        if (*p == '\0')
        {
            if (type == SEARCH_FILE_TYPE)
            {
                create_one_path_parser_error(PATH_PARSE_LS_EMPTY_ERR, 0, 0, 0, 0);
                return EMPTY_TYPE;
            }
            cnt = 0;
            return type; // [[ -> ]] or [[ => ]]
        }
        else
        {
            if (no_space(p) && *p != '-')
            {
                cnt = p;
                return type; // [[ -> path ]], [[ => path ]], or [[ ls path ]]
            }
        }
        // ls -1 (see 6 lines above)
        type = SCRIPT_TYPE;
        cnt = selector;
    }
    return type;
}

