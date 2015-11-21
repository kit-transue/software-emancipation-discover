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
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <genTmpfile.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include <genString.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
#include <shell_calls.h>

#if defined(hp10)
#include <stdlib.h>
#endif

extern int mrg_diff (const char *, const char *, const char *);
extern void mrg_diff_msg (char *);
extern void mrg_diff_err (char *);

genString diff_cmd;
int	do_debug=0;
FILE* 	log_fd=stderr;

static int is_executable (const char *fn)
{
    int res = 0;
    struct OStype_stat buf;
    int status = OSapi_stat ((char *)diff_cmd, &buf);
#ifndef _WIN32
    if ( status == 0 &&
	 ((buf.st_mode & S_IXOTH) ||
	  (buf.st_mode & S_IXGRP) || 
	  (buf.st_mode & S_IXUSR)) )
	res = 1;
#else
	if(status == 0)
		res = 1;	
#endif
    return res;
}

int set_diff_cmd ( const char *exe_name )
{
    int res = -1;
    genString tmp(exe_name);
    int len = tmp.length();
    char *txt = tmp;

    for ( int ii = len - 1 ; len > 0 ; ii -- ) {
	if (txt[ii] == '/' || txt[ii] == '\\') {
	    txt[ii + 1] = 0;
#ifndef _WIN32
	    diff_cmd.printf ("%s%s", txt, "diff");
#else
	    diff_cmd.printf ("%s%s", txt, "diff.exe");
#endif
	    if (is_executable((char *)diff_cmd)) 
		res = 0;
	    else {
		diff_cmd = "diff";
		res = 1;
	    }

	    break;
	}
    }

    return res;
}

static void mrg_info ()
{
    mrg_diff_msg ("");
    mrg_diff_msg ("NAME");
    mrg_diff_msg ("     mrg_diff - produce diff script on a per-character basis");
    mrg_diff_msg ("");
    mrg_diff_msg ("SYNOPSIS");
    mrg_diff_msg ("     mrg_diff filename1 filename2 [<output file>]");
    mrg_diff_msg ("");
    mrg_diff_msg ("DESCRIPTION");
    mrg_diff_msg ("     mrg_diff tokenizes two input files, then runs diff between");
    mrg_diff_msg ("     these files and converts diff output from lines into characters.");
    mrg_diff_msg ("     If output file is skipped, then result is produced on standard out.");
    mrg_diff_msg ("     Thus:");
    mrg_diff_msg ("");
    mrg_diff_msg ("           example% mrg_diff /tmp/f1 /tmp/f2");
    mrg_diff_msg ("");
    mrg_diff_msg ("     displays script that recreates the file /tmp/f2 from the file /tmp/file1");
    mrg_diff_msg ("     in terms of character offsets. There are following script formats:");
    mrg_diff_msg ("");
    mrg_diff_msg ("          1. command 'a' append:       n,0,S...S\\n");
    mrg_diff_msg ("          2. command 'c' change:       n,mS...S\\n");
    mrg_diff_msg ("          3. command 'd' delete:       n,m//\\n");
    mrg_diff_msg ("");
    mrg_diff_msg ("     n   - offset");
    mrg_diff_msg ("     m   - length");
    mrg_diff_msg ("     S   - separator; special character or letter that does not exist within");
    mrg_diff_msg ("           the string defined by ...");
    mrg_diff_msg ("     ... - the string, that contains any character but the separator character.");
    mrg_diff_msg ("           Each new line starts with two continuation characters:  '> '");
    mrg_diff_msg ("");
    mrg_diff_msg ("RETURNS");
    mrg_diff_msg ("     0 - success, -1 on failure. The diagnostics are printed on stderr.");
    mrg_diff_msg ("");
    mrg_diff_msg ("TMPDIR");
    mrg_diff_msg ("     mrg_diff creates three temporary files under $TMPDIR (default: /usr/tmp) and");
    mrg_diff_msg ("     delete these files after it is done. If the environment variable DIS_MRG_DEBUG");
    mrg_diff_msg ("     is set then the temporary files do not get deleted.");
    mrg_diff_msg ("");
    mrg_diff_msg ("DEPENDENCIES");
    mrg_diff_msg ("     mrg_diff uses the executable diff. mrg_diff looks for the executable in the same");
    mrg_diff_msg ("     directory where mrg_diff is started from and runs it if it exists. Otherwise,");
    mrg_diff_msg ("     mrg_diff runs diff without a directory name, relying on the system path.");
    mrg_diff_msg ("");
}

int main (int argc, char **argv)
{
    if (argc < 3 || strcmp (argv[1],"help") == 0 ||
		     strcmp (argv[1],"HELP") == 0 ||
		     strcmp (argv[1],"-h") == 0   ||
		     strcmp (argv[1],"-H") == 0 ) {
	mrg_info ();
#if defined(sun5) || defined(_WIN32)
	return 1;
#else
	exit(1);
#endif
    }

    int res = set_diff_cmd (argv[0]);
    // Parse the argument. Read the file with 3 columns and get two files to diff

    const char *prev_fn;
    const char *curr_fn;
    const char *out_fn = 0;
	
    prev_fn = argv[1];
    curr_fn = argv[2];
    if (argc > 3)
	out_fn  = argv[3];

    res = mrg_diff (prev_fn, curr_fn, out_fn);
#if defined(sun5) || defined(_WIN32)
    return res;
#else
    exit(res);
#endif
}
