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
#include <prototypes.h>
/*
 * This needs to be split into more than one include file.
 */
#ifdef __cplusplus

#ifndef _cLibraryFunctions_h
#define _cLibraryFunctions_h
#include <systemconstants.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <sys/stat.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */

#if defined(_WIN32) && !defined(__MC_MINGW__)
/* These are not defined under NT. */
typedef unsigned short mode_t;
typedef char *caddr_t;
typedef long pid_t;
typedef unsigned int uint;

/* msdev doesn't define the following for __STDC__ */
/* but galaxy still uses them. */
#ifndef S_IFMT
#define S_IFMT   _S_IFMT
#endif
#ifndef S_IDIR
#define S_IDIR   _S_IDIR
#endif
#ifndef S_ICHR
#define S_ICHR   _S_ICHR
#endif
#ifndef S_IREG
#define S_IREG   _S_IREG
#endif
#ifndef S_IREAD
#define S_IREAD  _S_IREAD
#endif
#ifndef S_IWRITE
#define S_IWRITE _S_IWRITE
#endif
#ifndef S_IEXEC
#define S_IEXEC  _S_IEXEC
#endif

#endif



//-------------------------------------------------
//-------------------------------------------------

#define  OSPATH(f)	cnv_path_2OS((char const *)(f))

extern "C" {

#ifndef _WIN32
#define   OSapi_lstat(p, buf)     (::lstat(p, buf))
#define   OStype_stat       stat 
#else
int      OSapi_spawnv(int mode, char const *cmdname, char const **argv);
#define  OSapi_lstat(p, buf)     (OSapi_stat(p, buf))
#define  OStype_stat       _stat 
int   	 GetDiscoverRegistryDebug(char const *pszKeyName);

#endif

	char const *convert_back_slashes( char const *path );
  char const *convert_forward_slashes( char const *path );
	int	  is_abs_path	( char const *path );
	char const *cnv_path_2OS 	( char const *path );
	char const *create_path_2DIS ( char const *path, char ** new_path );
	char const *trn_path_2DIS ( char const *path );
  int   isOSPATH();

 	int     OSapi_stat( char const *path, struct OStype_stat *buffer );

  char const *  OSapi_getenv( char const *name) ;
  int     OSapi_putenv( char const *name) ;
#ifdef _WIN32
  char const *   cnv_argstr_chdir_2OS( char const *argstr );
  char const *   cnv_argstr_delims_2OS( char const *argstr );
#endif
  char const *   cnv_argstr_path_2OS( char const *argstr );
  char const *   cnv_argstr_2OS( char const *argstr );
#ifdef _WIN32
  void     cnv_arglist_path_2OS( char ** arglist );
  void     prt_trn_path_2DIS( char *path, int flag );
#endif	
	};

//-------------------------------------------------

#define  OSapi_strtol       environment::strtol
#define  OSapi_getpwuid(u)  (&(passwd(u))
#define  OSapi_getpwnam(n)  (&(passwd(n))
#ifndef _WIN32
#define  OSapi_getuid       environment::getuid
#define  OSapi_getgid       environment::getgid
#endif
#define  OSapi_atof         environment::atof
#define  OSapi_perror       environment::perror
#define  OSapi_sprintf      sprintf
extern "C" int  OSapi_printf(char const *fmt, ...);
#define  OSapi_ctime        environment::ctime
#define  OSapi_localtime    environment::localtime
#define  OSapi_strftime     environment::strftime
#define  OSapi_gmtime       environment::gmtime
#define  OSapi_qsort        environment::qsort
#define  OSapi_bsearch      environment::bsearch
#define  OSapi_chdir        environment::chdir
#define  OSapi_closeChildren environment::closeChildren
#define  OSapi_rename       environment::rename
#define  OSapi_tempnam      environment::tempnam
#ifndef _WIN32
#define  OSapi_truncate     environment::truncate
#define  OSapi_hsearch      environment::hsearch
#define  OSapi_hcreate      environment::hcreate
#endif
#define  OSapi_execl        environment::execl
#define  OSapi_fdopen       environment::fdopen
extern "C" int OSapi_fprintf (FILE * f, char const *fmt, ...);
#define  OSapi_fopen        environment::fopen
#define  OSapi_fgets        environment::fgets
#define  OSapi_fgetc        environment::_fgetc
extern "C" int OSapi_fflush (FILE * f);
#define  OSapi_fclose       environment::fclose
#define  OSapi_feof         environment::_feof
#define  OSapi_ftell        environment::ftell
#define  OSapi_fseek        environment::fseek
#define  OSapi_rewind       environment::rewind
#define  OSapi_mkdir        environment::mkdir
#define  OSapi_pipe         environment::pipe
#define  OSapi_fdopen       environment::fdopen
#define  OSapi_memcmp       environment::_memcmp
#define  OSapi_getc         environment::_getc
#define  OSapi_putc         environment::_putc
#define  OSapi_fputc        environment::_fputc
#define  OSapi_puts         environment::_puts
#ifndef _WIN32
#define  OSapi_WIFSTOPPED   environment::wifstopped
#define  OSapi_WSTOPSIG     environment::wstopsig
#define  OSapi_WIFSIGNALED  environment::wifsignaled
#define  OSapi_WTERMSIG     environment::wtermsig
#define  OSapi_WEXITSTATUS  environment::wexitstatus
#endif
#ifndef _WIN32
#define  OSapi_fstat(fd, buf)    (::fstat(fd, buf))
#else
#define  OSapi_fstat(fd, buf)    (::_fstat(fd, buf))
#endif
#ifndef _WIN32
#define  OSapi_S_ISLNK(buf)      (S_ISLNK((buf)->st_mode))
#else
#define  OSapi_S_ISLNK(buf)      (0)
#endif
#ifndef _WIN32
#define  OSapi_S_ISDIR(buf)      (S_ISDIR((buf)->st_mode))
#else
#define  OSapi_S_ISDIR(buf)      (((buf)->st_mode) & _S_IFDIR)
#endif

#define  OSapi_S_ISREG(buf)      (S_ISREG((buf)->st_mode))
#define  OSapi_S_IWRITE(buf)     (((buf)->st_mode) & S_IWRITE)
#ifndef _WIN32
#define  OSapi_S_IWUSR(buf)      (((buf)->st_mode) & S_IWUSR)
#else
#define  OSapi_S_IWUSR(buf)      (((buf)->st_mode) & _S_IWRITE)
#endif /*_WIN32*/
#define  OSapi_S_IWGRP(buf)      (((buf)->st_mode) & S_IWGRP)
#define  OSapi_st_dev(buf)       ((buf)->st_dev) 
#define  OSapi_st_ino(buf)       ((buf)->st_ino)
#define  OSapi_atol              environment::atol
#ifndef _WIN32
#define  IWUSR()                 st_mode &(S_IWUSR)
#else
#define  IWUSR()                 st_mode &(_S_IWRITE)
#endif /*_WIN32*/
#define  IWRITE()                st_mode &(S_IWGRP|S_IWUSR|S_IWOTH)
#define  IWGRP()                 st_mode &(S_IWGRP)
#define  OSapi_fputs        environment::_fputs
extern "C" int OSapi_fwrite (const void * ptr, int size, int nitems, FILE * f);
extern "C" int OSapi_fread (void * ptr, int size, int nitems, FILE * f);
#define  OSapi_fscanf       fscanf
#define  OSapi_sscanf       sscanf

#ifndef _cLibraryFunctions_C_ 

// sys/types.h/stdtypes.h
#include "dis_types.h"

#endif /* _cLibraryFunctions_C_ */

// sys/errno.h
#include <errno.h>

#ifdef _WIN32
typedef int uid_t;
#else

#endif
// corresponds to passwd(pwd.h), password information.
 class dis_passwd {
  public:
    dis_passwd();
    dis_passwd(const uid_t uid);  // find the passwd entry matching userid
    dis_passwd(char const *name); // find the passwd entry matching username
    ~dis_passwd(){};
    int operator==(int i){ return(i==valid); }
    int operator!=(int i){ return(i!=valid); }
    uid_t uid;      // user id
    char *pw_name;  // user name
    char *pw_dir;   // home directory for this entry
    int  valid;     // are other fields valid?
  };


extern "C" {
// search.h
#if !defined(_cLibraryFunctions_C_) || (defined(_WIN32) && !defined(__MC_MINGW__))
  typedef struct entry { char const *key, *data; } ENTRY;
  typedef enum { FIND, ENTER } ACTION;
#endif
  struct tm ;
  // sys/time.h timeval and timezone 
  struct timeval ;
  struct timezone ;
};

  class environment {
  public:
    static long strtol(char const *str, char **ptr, int base);
    static double atof(char const *);
    static void perror(char const *error);
    static int _sprintf(char *string, char const *format,...);
    static int _printf(char const *format, ...);
    static int _fprintf(FILE *stream, char const *format,...);
    static int _sscanf(char const *s, char const *format,...);
    static int _fscanf(FILE *, char const *format,...);
    static int execl(char const *p, char const *arg0,...);
    static char const *ctime(time_t *clock);
    static void qsort(void*, size_t, size_t, int(*)(const void*, const void*));
    static void *bsearch(const void *key, void *base, size_t, size_t, int (*compar)(const void *, const void *));
//search.h
#ifndef _WIN32
    static ENTRY *hsearch(ENTRY item, ACTION action);
    static int hcreate(unsigned nel);
#endif
    static int chdir(char const *p);
    static int rename(char const *path1, char const *path2);
    static char *tempnam(char const *dir, char const *pfx);
    static FILE *fopen(char const *filename, char const *type);
    static char *fgets(char *s, int pos,  FILE *s1);
    static int _getc(FILE *s);
    static int _putc(char const c, FILE *s);
    static int _puts(char const *s);
    static int _fflush(FILE *s);
    static int fclose(FILE *s);
    static int _feof(FILE  *fp);
    static int mkdir(char const *p, mode_t mode);
    static int _memcmp(char const *s1, char const *s2, const int n);
#ifndef _WIN32
    static int truncate(char const *p, const off_t length);
#endif
    static FILE *fdopen(const int, char const *);
    static int closeChildren();
    static int pipe(int[]);
    static struct tm *localtime(time_t *clock);
    static long atol(char const *);
    static int strftime(char *, int, char const *, struct tm*);
    static struct tm *gmtime(time_t *clock);
    static int wifsignaled(int);
    static int wstopsig(int);
    static int wtermsig(int);
    static int wexitstatus(int);
    static int wifstopped(int);
    static int _fputs(char const *s, FILE *stream);
    static int _fgetc(FILE *stream);
    static int _fputc(char const c, FILE *stream);
    static int fwrite(const void *ptr, int size, int nitems, FILE *stream);
    static int fread(void *ptr, int size, int nitems, FILE *stream);
    static long ftell(FILE *stream);
    static void rewind(FILE *stream);
    static int fseek(FILE *stream, long offset, int pos);
#ifndef _WIN32
    static int getgid();
    static int getuid();
#endif
};
#endif


#else  /* #ifndef __cplusplus */
#define  OSapi_execl        execl
#define  OSapi_fdopen       fdopen
int OSapi_printf ();
int OSapi_fprintf ();
#define  OSapi_fopen        fopen
#define  OSapi_fgets        fgets
#define  OSapi_fgetc        fgetc
int OSapi_fflush ();
int OSapi_fread ();
int OSapi_fwrite ();
#define  OSapi_fclose       fclose
#define  OSapi_feof         feof
#define  OSapi_ftell        ftell
#define  OSapi_fseek        fseek
#define  OSapi_rewind       rewind
#define  OSapi_mkdir        mkdir
#define  OSapi_pipe         pipe
#define  OSapi_fdopen       fdopen
#define  OSapi_memcmp       memcmp
#define  OSapi_fileno(p)    (fileno(p))
#define  OSapi_getc         getc
#define  OSapi_putc         putc
#define  OSapi_fputc        fputc
#define  OSapi_puts         puts

#define  OSPATH(f)	cnv_path_2OS((char *)(f))

   char const *convert_back_slashes(char const *);
   char *convert_forward_slashes();
   int	  is_abs_path	(char const *);
   char const *cnv_path_2OS(char const *);
   char const *create_path_2DIS(char const *, char ** new_path );
   char const *trn_path_2DIS (char const *path);
   int   isOSPATH();
   void  free_os_path();

   char *OSapi_getenv() ;
   int   OSapi_putenv() ;

#ifndef _WIN32
#define   OSapi_stat(p, buf)      (stat(p, buf))
#define   OSapi_lstat(p, buf)     (lstat(p, buf))
#define   OStype_stat       stat 
#define   CHDIR    "cd "
//#define   cnv_argstr_chdir_2OS(argstr)  (argstr)
//#define   cnv_argstr_delims_2OS(argstr) (argstr)
#define   cnv_argstr_path_2OS(argstr)   (argstr)
#define   cnv_argstr_2OS(argstr)        (argstr)
//#define   prt_trn_path_2DIS(path,flag )
#else
int      OSapi_spawnv(int mode, char *cmdname, char **argv);
int		   OSapi_stat( char *path, struct _stat *buffer );
#define  OSapi_lstat(p, buf)     (OSapi_stat(p, buf))
#define  OStype_stat       _stat 
#define  CHDIR    "cd /D "
char const *cnv_argstr_chdir_2OS( char *argstr );
char const *cnv_argstr_delims_2OS( char *argstr );
char const *cnv_argstr_path_2OS( char const *argstr );
char const *cnv_argstr_2OS( char const *argstr );
//void     prt_trn_path_2DIS( char *path, int flag );
int   	 GetDiscoverRegistryDebug(char *pszKeyName);

#endif

#ifndef _WIN32
#define  OSapi_ftruncate    ftruncate
#else
#define  OSapi_ftruncate    _chsize
#endif /*!_WIN32*/
#endif  /* #ifndef __cplusplus */
