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
/* This file is main function (driver) to the file path_simplify.c
   that is used in the executables:

                 dis_path,  aset, pset_server, aset_cpp.
*/

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <sys/param.h>
#else
#include <stdlib.h>
#define MAXPATHLEN _MAX_PATH
#endif

static void usage ()
{
    printf ("\n");
    printf ("%s\n", "NAME");
    printf ("%s\n", "     dis_path - resolves dots and extra slashes in path");
    printf ("\n");
    printf ("%s\n", "SYNOPSIS");
    printf ("%s\n", "     dis_path");
    printf ("%s\n", "     dis_path pathname");
    printf ("%s\n", "     dis_path <filename1> <filename2>");
    printf ("\n");
    printf ("%s\n", "DESCRIPTION");
    printf ("%s\n", "     dis_path takes absolute path name and removes single dots, extra");
    printf ("%s\n", "     slashes, and resolves double dots. dis_path prints the resolved name");
    printf ("%s\n", "     on standard out and does not check to see if the path exists.");
    printf ("%s\n", "     With 2 parameters dis_path processes each line in filename1 and");
    printf ("%s\n", "     writes the corresponding line in filename2.");
    printf ("%s\n", "     Without parameters dis_path takes input from stdin.");
    printf ("%s\n", "     dis_path recursively follows directory links resolving double dots.");
    printf ("%s\n", "     Thus:");
    printf ("\n");
    printf ("%s\n", "     example% ls -l /aset/src");
    printf ("%s\n", "     /aset/src -> /vobs/PROJS/pmod_server/src");
    printf ("\n");
    printf ("%s\n", "     example% dis_path /aset/./src/..//..//../path.h");
    printf ("%s\n", "     /vobs/path.h");
    printf ("\n");
    printf ("%s\n", "RETURNS");
    printf ("%s\n", "     0 - success, -1 on failure.");
    printf ("\n");
}


extern "C" int remove_all_dots_and_double_slashes (char const *path, char *resolved_path);

int main (int argc, char **argv)
{
    int res = -1;
    
    if (argc == 1) { /* two way pipe */
	char nm[MAXPATHLEN];
	char resolved_path[MAXPATHLEN];
	while (fgets(nm,MAXPATHLEN,stdin)) {
	    int len = strlen(nm);
	    if (nm[len - 1] = '\n') nm[len -1] = 0;
	    res = remove_all_dots_and_double_slashes (nm, resolved_path);
	    if (res == 0) 
		fprintf (stdout, "%s\n", resolved_path);
	    else
		fprintf (stdout, "ERROR\n");

	    fflush (stdout);
	}
	res = 0;
    } else if (!(argc == 2 || argc == 3) || !argv[1] || !(argv[1])[0] || 
	strcmp ("-H", argv[1])   == 0          ||
	strcmp ("-h", argv[1])   == 0          ||
	strcmp ("help", argv[1]) == 0          ||
	strcmp ("HELP", argv[1]) == 0          ||
	strcmp ("?", argv[1])    == 0          ||
	strcmp ("-?", argv[1])   == 0) {

	usage();
    } else if (argc == 2) {
	char *nm = argv[1];
	char resolved_path[MAXPATHLEN];
	res = remove_all_dots_and_double_slashes (nm, resolved_path);
	if (res == 0) {
	    fprintf (stdout, "%s\n", resolved_path);
	    fflush (stdout);
	}
    } else {
	FILE *f1 = fopen (argv[1], "r");
	int line = 0;
	if (f1) {
	    FILE *f2 = fopen (argv[2], "w");
	    if (f2) {
		char nm[MAXPATHLEN];
		char resolved_path[MAXPATHLEN];
		while (fgets(nm, MAXPATHLEN, f1)) {
		    line ++;
		    res = remove_all_dots_and_double_slashes (nm, resolved_path);
		    if (res == 0) 
			fprintf (f2, "%s", resolved_path);
		    else
			printf ("error: dis_path conversion, line %d\n", line);
		}
		fclose (f2);
		res = 0;
	    } else
		printf ("error: Failed to write file %s\n", argv[2]);

	    fclose (f1);
	} else
	    printf ("error: Failed to read file %s\n", argv[1]);

	printf ("dis_path: processed %d lines\n", line);
    }
    return res;
}

