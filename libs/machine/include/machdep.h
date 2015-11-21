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
#ifndef _machdep_h
#define _machdep_h
#ifndef F_OK
#define F_OK            0       /* does file exist */
#define X_OK            1       /* is it executable by caller */
#define W_OK            2       /* is it writable by caller */
#define R_OK            4       /* is it readable by caller */
#endif

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#ifndef sun5
#include <cstdio>
#include <cstdlib>
#ifdef __MC_MACOSX__
#include <sys/mman.h>
#include <sys/types.h>
#endif
#else /* sun5 */

// Needed for putenv(), popen(), pclose(), fileno(), which seem fairly
// standard but are now missing from Sun's ISO headers.  We may need to
// rewrite these ourselves.  Also picks up realpath(), which does not
// appear very standard.  All estimates of standardness are based on the
// claims in the HP10 man pages about standards conformance.

#include <stdio.h.SUNWCCh>
#include <stdlib.h.SUNWCCh>
#endif /* sun5 */
#ifndef _WIN32
namespace std {};
using namespace std;
#endif /* _WIN32 */
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
#include <sys/stat.h>
#endif

#ifndef _WIN32

#include <signal.h>
#include <unistd.h>

#ifdef sun5
#define boolean ____boolean
#undef boolean
#endif /* sun5 */

#ifdef hp10
#include <fcntl.h>
#endif /* hp10 */

#else /* _WIN32 */
#undef SIGPIPE
#include <setjmp.h>

typedef unsigned short mode_t;
#ifndef __MC_MINGW__
typedef long pid_t;
#endif
typedef char *caddr_t;
typedef unsigned int uint;

#ifndef MAXPATHLEN
#define MAXPATHLEN 512
#endif

#define off_t _off_t
#endif /* _WIN32 */

#ifndef _WIN32
#ifndef O_RDONLY
#define O_RDONLY 0
#endif
#endif

#if defined(sun5)
#undef bcopy
#undef bzero
#endif

#include <signal.h>
#ifndef sun5
typedef void (*SIG_TYP)(int);
#endif

//-------------------------------------------------
#define  OSPATH(f)	cnv_path_2OS((char const *)(f))

extern "C" {

#ifndef _WIN32
    #define  OSapi_lstat(p, buf)     (::lstat(p, buf))
    #define  OStype_stat       stat 
    #define  CHDIR    "cd "
#else
    int      OSapi_spawnv(int mode, char const *cmdname, char const **argv);
    #define  OSapi_lstat(p, buf)     (OSapi_stat(p, buf))
    #define  OStype_stat       _stat 
    #define  CHDIR    "cd /D "

    int   	 GetDiscoverRegistryDebug(char const *pszKeyName);
    void     DisDebugMessageBox( char const *pszDebugKey, char const *pszCaption );
#endif
	
  char const *  convert_back_slashes( char const *path );
  char const *  convert_forward_slashes( char const *path );
  char const *  convert_to_uniform_case( char const *path );
  int	    is_abs_path	( char const *path );
  char const *  cnv_path_2OS 	( char const *path );
  char const *  create_path_2DIS ( char const *path, char ** new_path );
  char const *  trn_path_2DIS ( char const *path );
  int     isOSPATH();
  char const *realOSPATH( char const *path );
    //char const *real_slashedOSPATH( char const *path );

#ifdef __cplusplus
 	int     OSapi_stat( char const *path, struct OStype_stat *buffer );
#else
 	int     OSapi_stat( char const *path, struct OStype_stat *buffer );
#endif

 
  char const *  OSapi_getenv( char const *name) ;
  int     OSapi_putenv( char const *name) ;
#ifdef _WIN32
  char const *  cnv_argstr_chdir_2OS( char const *argstr );
  char const *  cnv_argstr_delims_2OS( char const *argstr );
#endif
  char const *  cnv_argstr_path_2OS( char const *argstr );
  char const *  cnv_argstr_2OS( char const *argstr );
#ifdef _WIN32
  void cnv_arglist_path_2OS( char ** arglist );
  void prt_trn_path_2DIS( char *path, int flag );
#endif
	};

//-------------------------------------------------
 

#define OSapi_access 		OS_dependent::access
#define OSapi_bcopy 		OS_dependent::bcopy
#define OSapi_bzero 		OS_dependent::bzero
#define OSapi_calloc 		OS_dependent::calloc
#define OSapi_chmod		OS_dependent::chmod
#define OSapi_execv 		OS_dependent::execv
#ifndef _WIN32
#define OSapi_ftruncate		OS_dependent::ftruncate
#define OSapi_getdtablesize 	OS_dependent::getdtablesize
#endif
#define OSapi_gethostid		OS_dependent::gethostid
#define OSapi_gethostname 	OS_dependent::gethostname
#define OSapi_getpagesize 	OS_dependent::getpagesize
extern "C" pid_t		OSapi_getpid ();
#define OSapi_getppid		OS_dependent::getppid
#define OSapi_gettimeofday	OS_dependent::gettimeofday
#define OSapi_getwd		OS_dependent::getwd
#define OSapi_index		OS_dependent::index
#define OSapi_malloc		OS_dependent::malloc
#ifndef _WIN32
#define OSapi_mmap 		OS_dependent::mmap
#endif
#define OSapi_munmap		OS_dependent::munmap

#define OSapi_open		OS_dependent::open
#define OSapi_dup2 		OS_dependent::dup2
#define OSapi_close		OS_dependent::close
#define OSapi_read 		OS_dependent::read
#define OSapi_write		OS_dependent::write
#define OSapi_lseek		OS_dependent::lseek
#define OSapi_readlink 		OS_dependent::readlink
#define OSapi_realpath 		OS_dependent::realpath
#define OSapi_unlink    	OS_dependent::unlink

#define OSapi_rindex 		OS_dependent::rindex
#define OSapi_sleep 		OS_dependent::sleep

#ifdef sun5
#define OSapi_strerror 		OS_dependent::m_strerror
#else	/* sun5 */
#define OSapi_strerror 		OS_dependent::strerror
#endif	/* sun5 */
#define OSapi_usleep 		OS_dependent::usleep
#define OSapi_time 		OS_dependent::time
#ifndef irix6
#define OSapi_vfork 		::vfork
#else
#define OSapi_vfork             ::fork
#define vfork                   fork
#endif /* !irix6 */
#ifdef _WIN32
#define OSapi_setjmp(a) setjmp(a)
#else
#define OSapi_setjmp(a)	:setjmp(a)
#endif
#ifndef _WIN32
#define OSapi_utimes 		OS_dependent::utimes
#define OSapi_waitpid 	OS_dependent::waitpid
#endif

#define OSapi_va_start  OS_dependent::va_start
#define OSapi_va_arg    OS_dependent::va_arg
#define OSapi_va_end    OS_dependent::va_end

#ifndef _WIN32
#define OSapi_fileno(p)    (fileno(p))
#else
#define OSapi_fileno       _fileno
#endif

#define OSapi_free         OS_dependent::free
#define OSapi_strlen       OS_dependent::strlen
#define OSapi_strcasecmp   OS_dependent::strcasecmp
#define OSapi_strcmp       OS_dependent::strcmp
#define OSapi_strdup       OS_dependent::strdup
#define OSapi_strncpy      OS_dependent::strncpy
#define OSapi_strncmp      OS_dependent::strncmp
#define OSapi_strcpy       OS_dependent::strcpy
#define OSapi_strchr       OS_dependent::strchr
#define OSapi_strcat       OS_dependent::strcat

#define OSapi_atoi         OS_dependent::atoi

#ifndef _WIN32
#define OSapi_signal       OS_dependent::signal
#define OSapi_alarm        OS_dependent::alarm
#endif

#ifdef _WIN32
#define SIGKILL                 0
#endif
#define OSapi_kill              OS_dependent::kill

#define OSapi_exit              OS_dependent::exit
#define OSapi__exit             OS_dependent::_exit
#define OSapi_execvp            OS_dependent::execvp

#define OSapi_realloc           OS_dependent::realloc
#define OSapi_popen             OS_dependent::popen
#define OSapi_pclose            OS_dependent::pclose

class OS_dependent {
  public:
    static  int  access (char const *path, int mode);
    static  void bcopy (const void *, void *, size_t );
    static  void bzero (void *buf, int cnt);
    static  char *calloc (unsigned nelem, unsigned elsize);
    static  int chmod (char const *path, mode_t mode);
    //    static  void endpwent(){}
    static  int  execv (char const *path, char * const argv[]);

#ifndef _WIN32
    static  int  ftruncate (int fd, off_t size);
    static  int  getdtablesize ();
    static  caddr_t mmap (caddr_t addr, int len, int prot, int share, int fd, off_t off);
    static  int ptrace (int request, int pid, char const *addr, int data, char const *addr2=0);
#endif
    static  long gethostid ();
    static  void gethostname(char *, int);
    static  int  getpagesize ();
    static  pid_t  getpid ();
    static  pid_t  getppid ();
    static  int gettimeofday (struct timeval* tv, struct timezone* tz);
    static  char *getwd (char pathname[]);
    static  char *index (char const *s, char c);
    static  char *malloc (unsigned size);
    static  void free (void *);
#if !defined(_WIN32) || (defined(_WIN32) && !defined(readlink))
    static  int  readlink (char const *path, char *buf, int bufsiz);
#endif
    static  char *realpath (char const *path, char *resolved);
    static  int  munmap (caddr_t addr, int len);
    static  int  unlink (char const *path);

    static  int  open (char const *path, int flags, int mode=066);
    static  int  read (int , char *, size_t);
    static  int  dup2 (int , int);
    static  int  write (int , char const *, size_t);
    static  int  read (int , void *, size_t);
    static  off_t lseek(int fd, off_t offset, int whence);
    static  int  close (int);

    static  char const *rindex (char const *s, char c);
#ifdef sun5
    static  char const *m_strerror(int errnum);
#else	/* sun5 */
    static  char const *strerror(int errnum);
#endif	/* sun5 */
    static  size_t strlen(char const *s);
    static  unsigned int  sleep (unsigned seconds);
    static  int  usleep (unsigned useconds);

#ifndef _WIN32
    static  int  utimes (char const *file,struct timeval *tvp);
    static  int waitpid (int pid, int *statusp, int options);
#endif
    static  char *strptime (char *buf, char *fmt, struct tm *t);
    static  time_t timelocal (tm*);

    static  int strcasecmp (char const *, char const *);
    static  int strcmp (char const *, char const *);
    static  int strncmp (char const *, char const *, const int n);
    static  char *strncpy (char *, char const *, size_t);
    static  char *strcpy (char *, char const *);
    static  char *strchr (char const *, int);
    static  char *strdup (char const *);

    static int  atoi (char const *);

#ifndef _WIN32
    static SIG_TYP signal(int, SIG_TYP);
    static unsigned int alarm(unsigned int);
#endif
    static int kill(int,int);

    static void _exit(int);
    static void exit(int);

    static int execvp(char const *file, char *const argv[]);
    static char *getenv(char const *name);
    static int  putenv(char const *string);
    static char *strcat(char *s1, char const *s2);
    
    static void *realloc(void *ptr, unsigned sz);
    static int   time(long *usec, long *sec);
    static time_t time(time_t *usec);
    static FILE *popen(char const *command, char const *type);
    static int pclose(FILE * file);
 } ;


#endif /* machdep.h */
