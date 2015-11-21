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
#include "galgen.h"

extern char *types[100];
extern int num_types;

static int curr_col = 1;
static int prev_col = 1;
static int line_num = 1;

void maketype (name, fix)
char* name;
int fix;
{
    if (num_types < 100) {
        if (fix) {
            char buffer[1000];
            strcpy (buffer, PTR_TYPE_PREFIX);
            strcat (buffer, name);
            strcat (buffer, PTR_TYPE_SUFFIX);
            types[num_types++] = strdup (buffer);
       } else
            types[num_types++] = strdup (name);
    } else printf ("***** out of type space\n");
}

int isfixable (name)
char *name;
{
    char buffer[1000];
    strcpy (buffer, PTR_TYPE_PREFIX);
    strcat (buffer, name);
    strcat (buffer, PTR_TYPE_SUFFIX);

    return istype(buffer);
}

int istype (name)
char *name;
{
    int i;
    for (i=0; i<num_types; i++)
	 if (strcmp(name, types[i]) == 0)
	     return 1;
    return 0;
}

void new_line() 
{ 
  line_num++;
  curr_col = 1;
  prev_col = 1;
}

void new_token( int len ) 
{
  prev_col = curr_col;
  curr_col += len;
}

int get_line() { return line_num; }
int get_curr_col() { return curr_col; }
int get_prev_col() { return prev_col; }

