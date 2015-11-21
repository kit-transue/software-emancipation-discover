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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <malloc.h>
#include <Hash.h>
#include <string.h>

static int basename_flag;

char *psetmalloc_handler (unsigned sz)
{
     return (char *) malloc(sz);
}
 
class charHash : public nameHash
{
virtual const char* name(const Object* oo)const 
      { return (const char*) oo;}

virtual bool isEqualObjects(const Object& o1, const Object&o2) const
  {return &o1 == &o2;}

}  name_hash;


static char* convert(char* name)
{
  for(char* p = name; *p; ++p) {
    int c = *p;
    if (c == '\\')
      *p = '/';
    else
      *p = tolower(c);
  }
    
  return name;
}

void report_name(char*name)
{
  char* nm = name;
#ifdef _WIN32
  convert(nm);
#endif
  if (basename_flag) {
    char* slash = strrchr(nm, '/');
    if (slash)
      nm = slash + 1;
  }
  name_hash.add(*((Object*)nm));
}

int hash_file(char* fname)
{
  struct stat st;
  stat(fname, &st);

  FILE* fl = fopen(fname, "r");

  if(!fl){
    fprintf(stderr, "can not open %s\n", fname);
    return 1;
  }

  int  fsize = (int) st.st_size;
  char* buf = (char *) malloc(fsize + 1);
  int sz = fread(buf, 1, fsize, fl);
  buf[fsize] = '\n';  // just in case no endline at the end
  char* ptr = buf;
  int no_names = 0;
  int name_len = 0;
  int ii= 0;
  while(isspace(*ptr)) {++ii; ++ptr;}
  for(; ii<sz; ++ii){
   if(isspace(buf[ii])){
      ++no_names;
      buf[ii] = '\0';
      report_name(ptr);
      ptr = buf + ii + 1;
      while(isspace(*ptr)) {++ii; ++ptr;}
   }
  }
  return 0;
}

char* listfile = 0;
char* true_string = 0;
char* false_string = 0;


void parg(char* nm, char * descr)
{
  fprintf(stderr, "   %s: %s\n", nm, descr);
}

int print_help()
{
  fprintf(stderr, "checklist <file> [-true <string>] [-false <string>] [-suff <suffixes>] [-basename]\n");
  
  parg("-help", "prints this message");
  parg("<file>", "list of strings separated by newline(char)");
  parg("-true <string>", "output string in case of hit ");
  parg("-false <string>", "output string in case of miss");
  parg("-suff <suffixes>", "list of suffixes for which result always true"); 
  parg("-basename", "compare only basename"); 

  fprintf(stderr, "\n example: \n    %s\n %s\n",
       "checklist -true '' list1 < list2",
       "prints on stdout all the elements in list2 which are not in list1");	  

  return 1;
}

struct Suff {
  char* nm;
  int len;
  Suff* next;
  Suff(int, const char*);
};

Suff::Suff(int length, const char* name) : len (length), next(0)
{
  nm = new char[len+1];
  strcpy(nm, name);
}

static Suff * suffixes;

static int report_suff(const char* nm)
{
  int found = 0;
  if (nm) {
    int len = strlen(nm);
    Suff* prev = NULL;
    for (Suff* cur = suffixes; cur; prev = cur, cur = cur->next) {
      if (cur->len == len && !strcmp(cur->nm, nm)) {
	found = 1;
	break;
      }
    }
    if (!found) {
      Suff* el = new Suff(len,nm);
      if (prev)
	prev->next = el;
      else
	suffixes = el;
    }
  }
  return found;
}

static int lookup_suff(const char* str)
{
  int retval = 0;

  if (str) {
    int len = strlen(str);
    const char* end = str + len;
    for(Suff* cur = suffixes; cur; cur = cur->next)
      if (len >= cur->len) {
	const char* tail = end - cur->len;
	if (strcmp(tail, cur->nm) == 0) {
	  retval = 1;
	  break;
	}
      }
  }

  return retval;
}

static int tokenize_suff(const char* str)
{
  int token_count = 0;
  char* st = (char*) str;
  bool flag = true;
  for(;;){
    while(*st == ' ' || *st == '\t')
      ++st;

    if(*st == '\0')
      break;

    char *en = st+1;
    int ch;
    while ((ch = *en) != ' ' && ch != '\t' && ch != '\0')
      ++en;

    *en = '\0';

    report_suff(st);
    ++ token_count;

    if(ch) {
      *en = ch;
      st = en + 1;
    } else {
      break;
    }
  }
  return token_count;
}

#define errmsg(x) (fprintf(stderr, "%s: %s\n", arg, x), status=1)
#define ifeqarg(x) if(strcmp(arg, x) == 0)

int process_args(int argc, char**argv)
{
 int status = 0;
 if(argc < 2){
   print_help();
   return 1;
 }
 char* arg = argv[0];

 for(int ii=1; ii<argc; ++ii) {
   char* arg = argv[ii];
   if(arg[0] != '-'){
     if(!listfile)
       listfile = arg;
     else
       errmsg("More then 1 file specified");

     continue;
   }
   arg++;  // skip '-'
   ifeqarg("true") {
     ii++;
     true_string = argv[ii];
     if(false_string && (strcmp(false_string, true_string) == 0))
       errmsg("true = false");
   } else ifeqarg("false"){
     ii++;
     false_string = argv[ii];
     if(true_string && (strcmp(false_string, true_string) == 0))
       errmsg("true = false");
   } else ifeqarg("help"){
     print_help();
     status = 1;
   } else ifeqarg("suff") {
     ii++;
     tokenize_suff(argv[ii]);
   } else ifeqarg("basename") {
     basename_flag = 1;
   } else {
     errmsg("unknown argument");
   }
 }

 if(!listfile)
     errmsg("no input file");

 if( !true_string && !false_string)
   false_string = "";

 return status;
}

int process_query()
{
  char buf[4000];
  int idx = 0;
  char*ppp=buf;
  int ch = ' ';
  char* answer;
  while(ch != EOF){  // for each query
   ppp = buf;
   while((ch=getchar()) != EOF){
    if(ch == '\n'){
      *ppp = '\0';
      
      if (lookup_suff(buf)) {
	answer = true_string;
      } else {
	Object*cur = 0;
	
	char* name = buf;
#ifdef _WIN32
	convert(name);
#endif
	if (basename_flag) {
	  char* slash = strrchr(name, '/');
	  if (slash)
	    name = slash + 1;
	}
	name_hash.find(name, idx, cur); 

	answer = cur ? true_string : false_string;
      }
      printf("%s\n", answer ? answer : buf); 
      fflush(stdout);
      break;
    } else {
      *ppp = ch;
      ++ppp;
    }
   }
 }
 return 0;
}

int main(int argc, char**argv)
{

  int status;

  status = process_args(argc, argv);
  if(status)  return status;

  status = hash_file(listfile);
  if(status)  return status;

  status = process_query();
  return status;
}
