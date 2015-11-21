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

static dont_skip_flag;
static char* date = 0;
static char* time = 0;
char dd[] = "yyyy/mm/dd ";
char tt[] = "hh:mm:ss ";

void repspaces(str)
char * str;
{
 while(*str){
   if(*str==' ') *str='0';
   ++str;
 }
}
void get_date(dt, tm)
char * dt; char * tm;
{
  int i1,i2,i3;

    i1=i2=i3=0;
    date = dd;
    if(dt) sscanf(dt, "%d/%d/%d", &i1, &i2, &i3);
    sprintf(dd, "19%2d/%2d/%2d", i3, i1, i2);
    repspaces(dd);

    i1=i2=i3=0;
    time = tt;
    if(tm) sscanf(tm, "%d:%d:%d", &i1, &i2, &i3);
    sprintf(tt, "%2d:%2d:%2d", i1, i2, i3);
    repspaces(tt);

}
main(argc, argv) 
 int argc; char**argv;
 { 
    if(argc > 1)
        get_date(argv[1], argv[2]);
    else 
        get_date(0, 0);

    printf("%s %s\n", date, time);
 }
