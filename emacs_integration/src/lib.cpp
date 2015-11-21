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
#include <stdio.h>
#include <string.h>

void  message( char *txt, char *typ)
{
	fprintf (stderr, "%s: %s\n", typ, txt);
}

char *optarg=0;
int optind=0;
static unsigned int optnum=0;

int getopt(int argc, char* const argv[], const char* optstring) {
	char cur, valid_arg, optspc;
	char curopt[3];
	char ret;
	int i;

	/* End of option list */
	if( optnum >= strlen(optstring) ) 
		return -1;

	cur = optstring[optnum];
	sprintf(curopt, "-%c", optstring[optnum]);
	curopt[2] = 0;
	optnum++;

	/* : option indicate value my be concatinated with flag */
	optspc = optstring[optnum];
	if(optspc == ':') optnum++;
	
	/* Means not found, man page does not specify value if next opt not found on command line */
	ret = 0;
	for( i=1; i<argc ; i++ ) {
		/* invalid option: does not begin with - or not found in optstring */
	  if( (argv[i])[0] == '-' ) {
		valid_arg=(argv[i])[1];
		if( !valid_arg || !strchr(optstring,valid_arg))
		{
			ret = '?';
			break;
		}

		if( !strncmp(curopt,argv[i],strlen(curopt)) )
		{
			optind=i+1;
			if(optspc==':' && (strlen(argv[i]) > 2)) {
				optarg=argv[i]+2;
			}
			else if(optspc==':')
			{
				optarg=argv[i+1];
				optind=i+2;
				if(i+1 >= argc) {
					ret = ':';
					break;
				}
				
			}
			ret = cur;
			break;
		}
	  }
	}
	return ret;
}
