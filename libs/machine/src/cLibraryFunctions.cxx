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
#define _cLibraryFunctions_C_ 1

#ifndef _WIN32

#include <unistd.h>
#include <sys/time.h>

#include <sys/resource.h>
#include <pwd.h>
#include <sys/wait.h>

#else

#include <direct.h>
#include <process.h>
#include <io.h>
#include <string.h>

#endif

#include <search.h>
#include <stdarg.h>
#include <sys/types.h>
#ifndef ISO_CPP_HEADERS
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <memory>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <sys/stat.h>

#include <cLibraryFunctions.h>
#include <msg.h>

 long OSapi_strtol(char const *str, char **ptr, int base){
   return(::strtol(str, ptr, base));
 }

 long OSapi_atol(char const *s){
   return ::atol(s);
 }

 double OSapi_atof(char const *s){
   return ::atof(s);
 }

  void OSapi_perror(char const *error){
    ::perror(error);
  }
 
  int environment::_fflush(FILE * f)
  {
    return OSapi_fflush(f);
  }

  int environment::fread (void * ptr, int size, int nitems, FILE * f)
  {
    return OSapi_fread (ptr, size, nitems, f);
  }

  int environment::_sprintf(char *string, char const *format,...)
{
    va_list argv;
    va_start(argv, format);
    return((int)::sprintf(string, format, argv));
    va_end(argv);
  }

  int environment::_printf(char const *format,...) {
    va_list argv;
    va_start(argv, format);
    return(::vprintf(format, argv));
    va_end(argv);
  }

  char const *OSapi_ctime(time_t *clock) {
    return(::ctime(clock));
  }

  struct tm *OSapi_gmtime(time_t *clock) {
    return(::gmtime(clock));
  }
  
  void OSapi_qsort(void *base, size_t nel, size_t width, int (*compar)(const void*, const void*)){

    ::qsort(base, nel, width, compar);
    return ;
  }

#ifndef _WIN32
  ENTRY *OSapi_hsearch(ENTRY item, ACTION action) {
   return ::hsearch(item, action); 
  }

  int OSapi_hcreate(unsigned nel){
   return(::hcreate(nel));
  }
#endif

  int OSapi_chdir(char const *p){
   return(::chdir(OSPATH(p))); 
  }

  void *OSapi_bsearch(const void *key, void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)){

    return ::bsearch((char const *)key, (char const *)base, nmemb, size, compar);
  }

 int  OSapi_execl(char const *p, char const* arg0,...){
    va_list argv;
    va_start(argv, arg0);
#ifdef _WIN32

		char* pszArg= (char*)arg0;
		while ( pszArg )
		{
			cnv_argstr_2OS( (char*)pszArg );
			pszArg = va_arg( argv, char* );
		}
		va_end( argv );
#endif
    return(::execl(OSPATH(p), arg0, argv));
    va_end(argv);
 }

  int OSapi_rename(char const *path1, char const* path2)
  {
#ifdef _WIN32
   path1 = OSPATH(path1);
   path2 = OSPATH(path2);
   //on NT, if both files exist, remove a file we are renaming to
   if( !_access( path1, 0 ) && !_access( path2, 0 ) )
     _unlink( path2 );
#endif /*_WIN32*/
   return(::rename(path1, path2));
  }

#ifndef _WIN32
char *OSapi_tempnam(char const *dir, char const *pfx) {
    return(::tempnam(dir, pfx));
}
#else

char *OSapi_tempnam(char const *dir, char const *pfx) {
  char const *temp_name;
  char *DIS_temp_name;
  if(dir != NULL) {
		char *tmp_dir = new char[strlen(dir)+1];
		strcpy(tmp_dir, dir);
		temp_name = ::tempnam(OSPATH(tmp_dir), pfx);
		delete tmp_dir;
  } else {
		char tmp_pfx[9];
		if (pfx)
		{
			strncpy(tmp_pfx, pfx, 9);
			tmp_pfx[4] = 0;
		}
		else
		{
			tmp_pfx[0] = 0;
		}
		char const *tmp_dir  = OSapi_getenv("TMPDIR");
    if ( isOSPATH() )
        tmp_dir++;
    temp_name =_tempnam(tmp_dir, tmp_pfx);
   }
  DIS_temp_name = (char*)malloc( strlen(temp_name) + 3 );
  if ( DIS_temp_name )
  {
  strcpy(DIS_temp_name, temp_name);	
  trn_path_2DIS(DIS_temp_name);
  }
  return DIS_temp_name;
}
#endif

  int environment::_fprintf(FILE *stream, char const *format,...) {
    va_list argv;
    va_start(argv, format);
    return(::fprintf(stream, format, argv));
    va_end(argv);
  }

  FILE *OSapi_fopen(char const *filename, char const *type){
    return ::fopen( OSPATH(filename), type);
  }

  int OSapi_fclose(FILE *stream){
    return ::fclose(stream);
  }

  int OSapi_feof(FILE *stream){
    return feof(stream);
  }

  int OSapi_mkdir(char const *p, mode_t mode){
#ifdef _WIN32
      return _mkdir(OSPATH(p));
#else
      return(::mkdir(p,mode));
#endif
  }

 int OSapi_memcmp(char const *s1, char const *s2, const int n){
   return(::memcmp(s1, s2, n));
 }

#ifndef _WIN32
 int OSapi_truncate(char const* p, const off_t length){
   return(::truncate(p, length));
 }
#endif

 FILE *OSapi_fdopen(const int fd, char const *type){
   return(::fdopen(fd, type));
 }

 int OSapi_pipe(int fds[]){
#ifndef _WIN32
   return(::pipe(fds));
#else
   return(_pipe(fds, 4096, _O_BINARY));
#endif
 }

 struct tm *OSapi_localtime(time_t *clock){
   return(::localtime(clock));
 }

   char *OSapi_fgets(char *s, int pos, FILE *stream){
    return ::fgets(s, pos, stream);
  }

  int OSapi_fgetc(FILE *stream){
    return ::fgetc(stream);
  }

  int OSapi_puts(char const *s){
    return ::puts(s);
  }

 int OSapi_fputc(char const c, FILE *stream){
    return ::fputc(c, stream);
  }

 int OSapi_fputs(char const *s, FILE *stream){
    return ::fputs(s, stream);
  }

 int environment::_sscanf(char const *s, char const *format,...){
    va_list argv;
    va_start(argv, format);
    return(::sscanf(s, format, argv));
    va_end(argv);
   
 }

 int environment::_fscanf(FILE *fp, char const *format,...){
    va_list argv;
    va_start(argv, format);
    return(::fscanf(fp, format, argv));
    va_end(argv);
 }


  int OSapi_getc(FILE *stream){
    return getc(stream);
  }

  int OSapi_putc(char const c, FILE *stream){
    return putc(c, stream);
  }

  void OSapi_rewind(FILE *stream){
    ::rewind(stream);
    return;	   
  }

  int OSapi_fseek(FILE *stream, long offset, int pos){
    return(::fseek(stream, offset, pos));
  }

  long OSapi_ftell(FILE *stream){
    return(::ftell(stream));
  }

#ifndef _WIN32   
  int OSapi_getgid(){
    return(::getgid());
  }
  
  int OSapi_getuid(){
    return(::getuid());
  }
#endif


dis_passwd::dis_passwd(){
#ifndef _WIN32
    setpwent();
#endif
    valid=0;
}

dis_passwd::dis_passwd(char const *name){
#ifndef _WIN32
  struct passwd *p;
  p=getpwnam(name);
  if(p){
    uid=p->pw_uid;
    pw_name=p->pw_name;
    pw_dir=p->pw_dir;
    valid=1; }
  else {
	valid=0;
      }
#else
  valid = 0;
#endif
}

dis_passwd::dis_passwd(const uid_t u){
#ifndef _WIN32
  struct passwd *p;
  p=getpwuid(u);
  if(p){
    uid=p->pw_uid;
    pw_name=p->pw_name;
    pw_dir=p->pw_dir;
    valid=1;} 
  else {
      valid=0;
    }
#else
  valid = 0;
#endif
}

int OSapi_closeChildren(){
#ifndef _WIN32
        struct rlimit syst_limit;
	::getrlimit (RLIMIT_NOFILE, &syst_limit);
        for (int i = 3; i < syst_limit.rlim_cur; ++i)
            ::close(i);
	return 1;
#else
	return 1;
#endif
}

int OSapi_strftime(char *buf, int bufsize, char const *fmt, struct tm *tm){
  return(::strftime(buf, bufsize, fmt, tm));
}

#define CHANNEL_OF(f) (f==stdout?1:(f==stderr?2:-1))

FILE * tee_file = (FILE *)0;

#define FIX_FILE(ch,out) (ch==1?stdout:(ch==2?stderr:out))

extern "C" int gala_fprintf (int channel, FILE * out,  char const * fmt, va_list ap)
{
    FILE * f = FIX_FILE(channel,out);
    if (tee_file && (f==stderr||f==stdout)) {
        vfprintf(tee_file, fmt, ap);
        fflush(tee_file);
    }
    int retval = vfprintf (f, fmt, ap);
    fflush(f);
    return retval;
}

extern "C" int gala_fflush (int channel, FILE * out)
{
    FILE * f = FIX_FILE(channel,out);
    if (tee_file && (f==stderr||f==stdout))
	fflush(tee_file);
    return fflush(f);
}

extern "C" int gala_fwrite (const void * p, int size, int nitems, int channel, FILE * out)
{
    FILE * f = FIX_FILE(channel,out);
    char const * ptr = (char const *)p;
    int retval = -1;
    if (f==stderr||f==stdout){
	if (tee_file) {
	    fwrite(ptr,size,nitems,tee_file);
            fflush(tee_file);
        }
	char const *p = ptr;
	int num = size*nitems;
	for(int ii=0; ii<num; ++ii)
	    putc(p[ii],f);
	retval = num;
    } else 
        retval = fwrite(ptr, size, nitems, f);
    fflush(f);
    return retval;
}

extern "C" int OSapi_printf (char const * fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
#ifndef _WIN32
    int retval = gala_fprintf (1, NULL, fmt, ap);
#else
    int retval = vprintf( fmt, ap );
#endif /*_WIN32*/
    va_end(ap);
    return retval;
}

extern "C" int OSapi_fprintf (FILE * f, char const * fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
#ifndef _WIN32
    int channel = CHANNEL_OF(f);
    int retval = gala_fprintf (channel, f, fmt, ap);
#else
    int retval = vfprintf( f, fmt, ap );
#endif /*_WIN32*/
    va_end(ap);
    return retval;
}

extern "C" int OSapi_fflush (FILE *f)
{
#ifndef _WIN32
    int channel = CHANNEL_OF(f);
    return gala_fflush (channel, f);
#else
    return fflush( f );
#endif /*_WIN32*/
}

extern "C" int OSapi_fread (void * ptr, int size, int nitems, FILE *f)
{
    return ::fread ((char *)ptr, size, nitems, f);
}

extern "C" int OSapi_fwrite (const void * ptr, int size, int nitems, FILE * f)
{
#ifndef _WIN32
    int channel = CHANNEL_OF(f);
    return gala_fwrite (ptr, size, nitems, channel, f);
#else
    return fwrite(ptr, size, nitems, f);
#endif
}

#ifndef _WIN32
int OSapi_WIFSIGNALED(int status){
  return(WIFSIGNALED(status));
}

int OSapi_WSTOPSIG(int status){
  return(WSTOPSIG(status));
  }

int OSapi_WTERMSIG(int status){
  return(WTERMSIG(status));
}

int OSapi_WEXITSTATUS(int status){
  return(WEXITSTATUS(status));
}

int OSapi_WIFSTOPPED(int status){
  return(WIFSTOPPED(status));
}
#endif

#ifdef TEST_LIB_FUNCTIONS
int f(char const *args){
char const *i="hello"; 
char const *j="world\n";
cout << "---OSapi_printf-----\n";
cout << "expected:\n\t hello world\n "<< flush;
OSapi_sprintf(args, "%s%s", j, i);
OSapi_sscanf(args, "%s%s", i, j);
OSapi_printf("\t %s %s", i, j);
// Check stat..with options
// check passwd with options
return 1;
}
main()
{
char args[100];
char const *i="hello"; 
char const *j="world\n";
cout << "---OSapi_printf-----\n";
cout << "expected:\n\t hello world\n "<< flush;
OSapi_sprintf(args, "%s%s", j, i);
cout << "end scanf \n";
f(args);
cout <<  " end call f\n";
OSapi_sscanf(args, "%s%s", i, j);
printf("\t %s %s", i, j);
dis_passwd p("builder");
OSapi_printf ("password entry home directory for %s is: %s\n", p.pw_name, p.pw_dir);
OSapi_printf("-----------------Done!-------------\n");
}
#endif

