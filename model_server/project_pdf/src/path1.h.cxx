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
// path1.h.C
// implementation for class WildPair
#include <cLibraryFunctions.h>

#ifndef _path_h
#include <path.h>
#endif

#ifndef _path1_h
#include <path1.h>
#endif

static void create_parens(const char *s, genString &new_s)
// If some | is not inside of the (), add () to surround the string.
//
// s: the input user"s defined matching pattern
// new_s: the returned pattern
{
    Initialize(create_parens);

    new_s = s;
    int len = strlen(s);
    if (*s == '(' && s[len-1] == ')')
        return;

    int found_paren_before = 0;
    int found_paren_after  = 0;
    int need_paren         = 0;

    char *p = (char *)s;
    while(*p)
    {
        if (*p == '(')
            found_paren_before = 1;
        else if (*p == ')')
            found_paren_after = 1;
        else if (*p == '|')
        {
            if (found_paren_before == 0)
            {
                need_paren = 1;
                break;
            }
            found_paren_after = 0;
        }
        p++;
    }

    if (need_paren || !found_paren_after)
        new_s.printf("(%s)",s);
}

// class for WildPair
WildPair::WildPair(const char *s) : matched_type(-1), str(s), match_with_end(0), match_without_end(0)
// constructor for WildPair
//
// s: the matching pattern
//
// if | is in s, match_with_end will be defined
// match_without_end is defined always
{
    Initialize(WildPair::WildPair);

    if (s && strlen(s))
    {
        if (contain_target_char((char *)s,'|', 1))
        {
            genString new_s;
            create_parens(s,new_s);
            match_with_end = (WildRegexp *)db_new(WildRegexp, (new_s, COMPLICATE_LEVEL_4));
        }

        match_without_end = (WildRegexp *)db_new(WildRegexp, (s, COMPLICATE_LEVEL_1));
    }
}

WildPair::~WildPair()
// destructor of WildPair
{
    Initialize(WildPair::~WildPair);

    if (match_with_end)
        delete match_with_end;

    if (match_without_end)
        delete match_without_end;
}

int WildPair::match(const char *path, int type)
// check if path matches the pattern
//
// type = MATCH_WITH_END or
//        MATCH_WITHOUT_END
{
    Initialize(wildPair::match);

    matched_type = -1;
    if (!path)
        return -1;

    int lens = strlen(path);
    if (type == MATCH_WITH_END)
    {
        if (!match_with_end)
            return -1;

        int len = match_with_end->Match(path,lens,0);
        if (len >= 0)
        {
            matched_type = MATCH_WITH_END;
            return len;
        }
    }
    else // if (type == MATCH_WITHOUT_END)
    {
        if (!match_without_end)
            return -1;

        int len = match_without_end->Match(path,lens,0);
        if (len >= 0)
        {
            matched_type = MATCH_WITHOUT_END;
            return len;
        }
    }
    return -1;
}

int WildPair::match(const char *path)
// check if path matches the pattern for MATCH_WITHOUT_END
{
    Initialize(wildPair::match);

    matched_type = -1;
    if (!path)
        return -1;

    return match(path, MATCH_WITHOUT_END);
}

int WildPair::match_to_end(const char *path)
// check if path matches the pattern for MATCH_WITHOUT_END first
// then if failed check if path matches the pattern for MATCH_WITH_END
{
    Initialize(wildPair::match_to_end);

    matched_type = -1;
    if (!path)
        return -1;
    int lens = strlen(path);
    int len = match(path, MATCH_WITHOUT_END);
    if (len >= 0)
    {
        if (len == lens)
            return len;
        len = match(path, MATCH_WITH_END);
        if (len >= 0 && len == lens)
            return len;
    }
    return -1;
}

void WildPair::range(int s_idx, int &start_idx, int &end_idx)
// it will return the indexes for the first byte and the last byte
// which is surrounded by the (s_idx)th pair of parentheses
{
    Initialize(WildPair::range);

    if (matched_type == -1)
    {
        start_idx = -1;
        end_idx   = -1;
    }
    else if (matched_type == MATCH_WITH_END)
    {
        start_idx = match_with_end->BeginningOfMatch(s_idx);
        end_idx   = match_with_end->EndOfMatch(s_idx);
    }
    else // if (matched_type == MATCH_WITHOUT_END)
    {
        start_idx = match_without_end->BeginningOfMatch(s_idx);
        end_idx   = match_without_end->EndOfMatch(s_idx);
    }
}

