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
/* This file is used in the following executables: 

   dis_path, aset, pset_server, aset_cpp.

   Real file: /paraset/src/paraset/project/pdf/path_simplify.c
   Soft link: /paraset/src/cpp/cpp/src/path_simplify.c 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/param.h>
#else
#define MAXPATHLEN _MAX_PATH
static int readlink(path,buf,size)
  char * path;
  char * buf;
  int    size;
{
    return -1;
}
#endif

#include <errno.h>

#define DIS_MAX_LINK_NESTING 13

enum STATES {
    REG,
    SLASH,
    SLASH_DOT
};

static int remove_double_slashes_and_single_dots (pathname)
    char *pathname;
{
    char *p   = pathname;
    int state = REG;
    char c    = 0;
    char *from, *to;
    static char *buf = 0;

    if (buf == 0) {
	buf = malloc (MAXPATHLEN);
	if (buf == 0)
	    return -1;
    }

    while(*p)
    {
        if (*p == '\\')
	    *p = '/';

        p++;
    }

    from = pathname;
    to   = buf;
    while ( (c = from[0]) ) {
	switch (state) {
	case REG:
	    if (c == '/')
		state = SLASH;

	    *to++ = c;
	    break;
	case SLASH:
	    if (c == '.')
		state = SLASH_DOT;
	    else if (c != '/') {
		state = REG;
		*to++ = c;
	    }
	    break;
	case SLASH_DOT:
	    if (c != '/') {
		state = REG;
		*to++ = '.';
		*to++ = c;
	    } else
		state = SLASH;
	    break;
	}
	from ++ ;
    }

    to[0] = 0;
    strcpy (pathname, buf);
    return 0;
}

char *has_double_dots (path)
    char *path;
{
    char *dots = 0;
    char *s = path;
    char *p;
    while(1) {
	p = strstr(s,"..");
	if (!p || *(p-1) == '/' && ((*(p+2) == '\0' || *(p+2) == '/'))) {
	    dots = p;
	    break;
	}
	s = p+2;
    }
    return dots;
}

static int splice_last_level (path)
    char *path;
{
    int res  = -1;
    int ii = strlen (path) - 1;
    while ( ii >= 1 ) {
	if ( path[ii - 1] == '/' || path[ii - 1] == '\\' ) {
	    res = 0;
	    path[ii] = 0;
	    break;
	}

	ii --;
    }
    return res;
}

static int resolve_one_link (path)
    char *path;
{
    int res = -1;
    int sz  = 0;
    static char *buf = 0;
    int len = strlen (path);
    char c = path[len - 1];
    if (buf == 0) {
	buf = malloc (MAXPATHLEN);
	if (buf == 0)
	    return res;
    }
    buf[0] = 0;

    if ( c == '/' )
	path[len-1] = 0;

    sz = readlink (path, buf, MAXPATHLEN);

    if ( sz == -1 )
	if (errno == EINVAL)
	    sz = 0;

    if (sz == 0 ) {
	res = 0;
	return res;
    } else if ( sz == -1) {
	res = -1;
	return res;
    }

    if ( c == '/' )
	buf[sz++] = c;

    buf[sz] = 0;
    if (remove_double_slashes_and_single_dots (buf) != 0)
	res = -1;
    else if (buf[0] == '/' || buf[0] == '\\') {
	strcpy(path,buf);
	res = 0;
    } else if (splice_last_level(path) == 0) {
	strcat (path,buf);
	res = 0;
    }
    return res;
}

static int resolve_tail_double_dots (nm, resolved_path, level)
    char *nm;
    char *resolved_path;
    int level;
{
    int res    = -1;
    char *tail = nm;
    char *dots = 0;

    if ( !nm || !nm[0] ) {
	strcpy (resolved_path, "<null>");
	return res;
    }

    if (level >= DIS_MAX_LINK_NESTING)
	return res;

    resolved_path[0] = 0;

    while (tail[0]) {
	dots = has_double_dots (tail);
	if ( dots == 0 ) {
	    res = 0;
	    break;
	}
	if (dots - tail)
	    strncat (resolved_path, tail, (dots - tail));
#ifndef _WIN32	
	if (resolve_one_link (resolved_path) != 0)
	    break;

	if (has_double_dots (resolved_path)) {
	    char *new_path = malloc(MAXPATHLEN);
	    if (new_path == 0)
		break;

	    res = resolve_tail_double_dots (resolved_path, new_path, ++ level);
	    if (res != 0)
		break;

	    strcpy (resolved_path, new_path);
	    free (new_path);
	}
#else
  res = 0;
#endif
	if (splice_last_level (resolved_path) != 0)
	    break;

	if (*(dots + 2) == 0)
	    tail = dots + 2;
	else
	    tail = dots + 3;
    }

    if (res == 0 && tail[0])
	strcat (resolved_path,tail);

    return res;
}

int
remove_all_dots_and_double_slashes( char const *nm, char *resolved_path)
{
    int res = -1;
    char *orig;

    if (!nm || nm[0] == 0 || strlen (nm) >= MAXPATHLEN) {
	fprintf (stderr, "Wrong file name parameter\n");
	return res;
    }

    orig = malloc (MAXPATHLEN);
    if ( !orig ) {
	fprintf (stderr, "Failed to allocate memory\n");
	return res;
    }
    strcpy (orig, nm);
    strcpy (resolved_path, "<null>");

    res = remove_double_slashes_and_single_dots (orig);
    if (res == 0)
	res = resolve_tail_double_dots (orig, resolved_path, 0);

    free (orig);
    return res;
}

#if _STANDALONE__DEBUG_
main (argc, argv)
    int argc;
    char **argv;
{
    char *nm = argv[1];
    char *resolved_path = malloc (MAXPATHLEN);
    int res = -1;

    if ( !resolved_path ) {
	fprintf (stderr, "Failed to allocate memory\n");
	_exit (-1);
    }

    res = remove_all_dots_and_double_slashes (nm, resolved_path);

    fprintf (stdout, "\nIn:  %s\nOut: %s\n", nm, resolved_path);
    free (resolved_path);
}
#endif
