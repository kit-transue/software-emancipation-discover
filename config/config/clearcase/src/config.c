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

struct config
{
  char * suf;
  char * dir;
  char * qual;
};

static char * ext_symbol = "@@";

static struct  config cosp[100];
static int no_cosp = 0;
static char * str_dup(suf)
     char* suf;
{
  return
    (suf && (suf[0]!='*') && (suf[0]!='@')) ? strdup(suf) : 0;
}

void cosp_put(suf, dir, qual)
     char * suf;
     char * dir;
     char * qual;
{
 cosp[no_cosp].suf = str_dup(suf);
 cosp[no_cosp].dir = str_dup(dir);
 cosp[no_cosp].qual = str_dup(qual);
 no_cosp++;
}

void cosp_prt()
{
  int ii;
  char * qual;
  for(ii=0; ii<no_cosp; ++ii){
    qual = cosp[ii].qual ? cosp[ii].qual : "";
    printf("%s %s %s \n", cosp[ii].suf, cosp[ii].dir, qual);
  }
}

static char * proj_name;
static char * dir_name;
static char * dir_test;

void print_one_proj(cdir, proj, dir, qual, ln, attr, body, lineno)
     char* cdir;
     char * proj;
     char * dir;
     char * qual;
     char * ln;
     char * attr;
     char * body;
     int lineno;
{
    if(!qual) qual = "";
    if(!attr) attr = "";

    printf("\n\n# %d\n", lineno);

    printf("%s : %s %s %s <-> %s \n", proj, dir, attr, qual, ln);
    if(cdir && strcmp(cdir, dir)){
       int len = strlen(cdir);
       char * ptr = body;
       char * next = strstr(ptr, cdir);
       while(*ptr){
	 if(ptr != next){
	   putchar(*ptr);
	   ptr++;	   
	 } else {
	   printf("%s", dir);
	   ptr += len;
	   next = strstr(ptr, cdir);
	 }
       }
    } else {
      printf("%s\n", body);
    }
}

static int do_not_change(proj, dir)
     char * proj;
     char * dir;
{
  if(proj[0] == '_' && proj[1]=='_')
    return 1;
  if(proj_name && strcmp(proj, proj_name))
      return 1;
  if(dir_name && strcmp(dir, dir_name))
      return 1;

  if(dir_test){
    char buf[1024];
    strcpy(buf, dir_test);
    strcat(buf, " ");
    strcat(buf, dir);
    strcat(buf, " > /dev/null 2> /dev/null");
/*   printf("*********** %s\n", buf); */
    return system(buf);
  }

  return 0;
}
void print_pdf(proj, dir, qual, ln, attr, body, lineno)
     char * proj;
     char * dir;
     char * qual;
     char * ln;
     char * attr;
     char * body;
     int lineno;
{
  int ii;
  char buf[1024];
  char * pr;
  char * pnm;
  if(do_not_change(proj, dir))
     print_one_proj(0, proj, dir, qual, ln, attr, body, lineno);
  else for(ii=0; ii<no_cosp; ++ii){
    if(cosp[ii].suf){
      sprintf(buf, "%s%s%s", proj, ext_symbol, cosp[ii].suf);
      pr = buf;
    } else {
      pr = proj;
    }
    if(cosp[ii].dir)
      pnm = cosp[ii].dir;
    else
      pnm = dir;

    print_one_proj(dir, pr, pnm, cosp[ii].qual, ln, attr, body, lineno);
  }
  
}

void load_cosp(file)
char*file;
{

 char buf1[1024];
 char buf2[1024];
 char buf3[1024];

 char*ptr;
 int not_null;

 FILE * ff = fopen(file, "r");
 
 fscanf(ff, "proj: %s dir: %s\n", buf1, buf2);
 proj_name = str_dup(buf1);
 dir_name = str_dup(buf2);

 not_null = 0;
 ptr = buf3;
 while((*ptr=fgetc(ff)) != '\n'){
   if(*ptr != ' ' && *ptr != '\t')
       not_null = 1;
   ++ptr;
 }
 if(*ptr != EOF && not_null){
   *ptr = '\0';
   dir_test = strdup(buf3);
 } else {
   dir_test = 0;
 }

 while(fscanf(ff, "%s %s", buf1, buf2)){
   ptr = buf3;
   while((*ptr=fgetc(ff)) != '\n'){
    if(*ptr == EOF) break;
    ++ptr;
   }
   
   if(*ptr == EOF) break;
   *ptr = '\0';
   cosp_put(buf1, buf2, buf3);
 }
 fclose(ff);
}

