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
#include <string.h>
#include <stdio.h>

char* months[] = {
    "Invalid",
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
    NULL
};

main (argc, argv)
int argc;
char** argv;
{
    char buffer[1024];
    char *ptr1, *ptr2;
    char version[100], day[100], month[100];
    int d, h, m, s, y;
    int m_num;
    int i;

    int sM, sd, sy, sh, sm, ss;

    if (argc == 2) {
        sscanf (argv[1], "%d/%d/%d %d:%d:%d %d", &sM,&sd,&sy,&sh,&sm,&ss );
	if (sy < 1900) sy += 1900;
    }

    while (gets(buffer))
    {
        sscanf (buffer, "%s %s %s %d %d:%d:%d %d", version, day, month, 
		&d, &h, &m, &s, &y);
	m_num = 0;
	for (i=1; i<=12; i++) 
	    if (strncmp(months[i],month,3) == 0) {
		m_num = i;
		break;
	    }
	if ((y > sy) ||
	    ((y == sy) && ((m_num > sM) || 
		  ((m_num == sM) && ((d > sd) ||
	                ((d == sd) && ((h > sh) ||
			     ((h == sh) && ((m > sm) ||
				    ((m == sm) && (s >= ss))))))))))) {
	    printf ("%s %04d/%02d/%02d %02d:%02d:%02d\n", version, y, m_num, d, h, m, s);
	}
    } 

    exit (0);
}


