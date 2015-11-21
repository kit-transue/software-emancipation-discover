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
#include "machdep.h"

#ifdef _WIN32

#include <stdio.h>
#include <direct.h>
#include <io.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <string.h>
#else
#include <iostream>
#include <cstring>
using namespace std;
#endif
#include <process.h>
#include <time.h>
#include <winsock.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <sys/timeb.h>

#else

#ifndef ISO_CPP_HEADERS
#include <signal.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <csignal>
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <sys/stat.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#if defined(hp10)
#include <sys/vfs.h>
#else
#include <sys/statvfs.h>
#endif

#include <fcntl.h>
#include <sys/param.h>

#if defined(irix6) || defined(sun5)
#include <sys/swap.h>
#include <sys/systeminfo.h>
#endif

#include <sys/wait.h>
#ifndef ISO_CPP_HEADERS
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
#endif /* ISO_CPP_HEADERS */

#include <psetmem.h>

#endif /* _WIN32 */

#include "cLibraryFunctions.h"

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#include "DisRegistry.h"
#include "DisFName.h"

static int set_file_mode()
{
  _fmode = _O_BINARY;
  return _fmode;
}

static int set_fmode = set_file_mode();

class pidExtractor{
  public:
    pidExtractor(void);
    int getppid(void);
  private:
    int ppid;
};

pidExtractor pidManager;

pidExtractor::pidExtractor(void)
{
    char *ppid_str = getenv("PSET_PPID");
    if(ppid_str != NULL)
	sscanf(ppid_str, "%d", &ppid);
    else
	ppid = 0;
}

int pidExtractor::getppid(void)
{
    cerr << "gettppid value : " << ppid << endl;
    return ppid;
}
#endif

time_t OS_dependent::time(time_t *tloc)
{
    return(::time(tloc));
}

#ifdef _WIN32
   int usleep(unsigned useconds){
       unsigned new_value = useconds >> 3;
       cerr << "Sleep not implemented" << endl;
       return 0;
   }
#else
   int  usleep (unsigned useconds)
    {
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = useconds;
      select(0,NULL, NULL, NULL, &tv);
      return 0;
    };
#endif

#ifdef sun55
extern "C" {
    int utimes(char *file, struct timeval *tvp);
#ifdef Solaris_5_3
    int gettimeofday (struct timeval*);
#endif
  }
#endif

#if defined(sun5) || defined(sun55)
#ifdef Solaris_5_3
   int OS_dependent::gettimeofday (struct timeval* tv, struct timezone* )
        {return ::gettimeofday (tv);};
#else
   int OS_dependent::gettimeofday (struct timeval* tv, struct timezone* tz)
    {return ::gettimeofday (tv, tz);};
#endif
#endif

//
//       Define all the system call interfaces here
//
int OS_dependent::access (char const *path, int mode) {
#ifdef _WIN32
    return ::_access( OSPATH(path), mode);
#else
    return ::access(path, mode);
#endif
}


void OS_dependent::bcopy (const void *src, void *dest, size_t cnt)  {
   memmove((void *)dest, (const void *)src, (size_t)cnt);
};

void OS_dependent::bzero (void *buf, int cnt){
    ::memset(buf, 0, cnt);
}

 char* OS_dependent::calloc (unsigned nelem, unsigned elsize){
     return((char *)::calloc((size_t)nelem,(size_t)elsize));
}

int OS_dependent::chmod (char const *path, mode_t mode){
#ifdef _WIN32
  return ::chmod( OSPATH(path), mode);
#else
  return ::chmod((char const *)path, mode);
#endif
}

int  OS_dependent::execv(char const *path, char *const argv[]){
#ifdef _WIN32
	for (int i=0; argv[i]; i++ )
			cnv_argstr_2OS( (char*)(argv[i]) );
			return ::execv( OSPATH(path), (char const *const *)argv);
#else
    return ::execv(path, argv);
#endif
}

int  OS_dependent::execvp(char const *path, char *const argv[]){
#ifdef _WIN32
	for (int i=0; argv[i]; i++ )
			cnv_argstr_2OS( (char*)(argv[i]) );
  return ::execvp( OSPATH(path), (char const *const *)argv);
#else
    return ::execvp(path, argv);
#endif
}

#ifndef _WIN32
int  OS_dependent::ftruncate(int fd, off_t size){
    return ::ftruncate(fd, size);
}
#endif

#ifndef _WIN32
 int  OS_dependent::getdtablesize()
        {return (int)::sysconf(_SC_OPEN_MAX);}
#endif

 long OS_dependent::gethostid (){
#if defined(sun5)
 char hostid[128];
    sysinfo (SI_HW_SERIAL,hostid, 128);
     return atol(hostid);
#elif defined(hp10)
   return ::gethostid();
#else
   return 0;
#endif
 }


 void OS_dependent::gethostname (char *ainet, int size) {
#if 1
   ::gethostname(ainet, (size_t)size);
#elif defined(_WIN32)
   ::gethostname(ainet, size);
#else
   ::sysinfo(SI_HOSTNAME, ainet, size);
#endif
   return;
 }

extern "C" pid_t OSapi_getpid (){
    return OS_dependent::getpid();
}

pid_t  OS_dependent::getpid (){
    return ::getpid();
}

#ifndef _WIN32
pid_t  OS_dependent::getppid (){
    return ::getppid();
}
#else
pid_t OS_dependent::getppid(){
    return pidManager.getppid();
}
#endif

int OS_dependent::getpagesize(){
#if defined(hp10)
  return (int)::sysconf(_SC_PAGE_SIZE);
#elif defined(_WIN32)
  return 4096;
#else
  return (int)::sysconf(_SC_PAGESIZE);
#endif
 }


#ifndef sun5
int OS_dependent::gettimeofday (struct timeval* tv, struct timezone* tz){
#ifndef _WIN32
    return ::gettimeofday (tv, tz);
#else
    struct _timeb sys_time;
    _ftime(&sys_time);
    tv->tv_sec  = sys_time.time;
    tv->tv_usec = sys_time.millitm * 1000;
    return 0;
#endif
}
#endif

char* OS_dependent::getwd (char *pathname){
#ifndef _WIN32
    return ::getcwd((char*)pathname, (sizeof(char) * MAXPATHLEN));
#else
    _getcwd( (char*)pathname, (sizeof(char) * MAXPATHLEN));
    return (char *)trn_path_2DIS( pathname );
#endif
}

char* OS_dependent::index (char const *s, char c) {
    return ::strchr(s, c);
}

int OS_dependent::dup2(int fd1, int fd2) {
#ifdef _WIN32
    return _dup2(fd1, fd2);
#else
    return ::dup2(fd1, fd2);
#endif
}

char* OS_dependent::malloc (unsigned size){
    return (char*)::malloc(size);
}

void* OS_dependent::realloc (void *ptr, unsigned size){
   return (void*)::realloc((char *)ptr, size);
}

#ifndef _WIN32
caddr_t OS_dependent::mmap (caddr_t addr, int len, int prot, int share, int fd, off_t off){
  return ((caddr_t)::mmap(addr, (size_t)len, prot, share, fd, off));
}
#endif

int  OS_dependent::munmap (caddr_t addr, int len){
#ifdef _WIN32
   if(UnmapViewOfFile(addr))
       return 0;
   else
       return -1;
#else
   return ::munmap(addr, len);
#endif
 }

 int  OS_dependent::open(char const *path, int flags, int mode){
#ifdef _WIN32
   char *tmp_path = new char[strlen(path)+1];
   strcpy (tmp_path, path);
   return ::open( OSPATH(tmp_path), flags, (mode_t)mode);
   delete tmp_path;
#else
   return ::open(path, flags, mode);
#endif
 }

 int  OS_dependent::close(int fd){
   return ::close(fd);
 }

 int  OS_dependent::read(int bytes, void *buffer, size_t amount){
   return ::read(bytes, (char *)buffer, amount);
 }

 off_t  OS_dependent::lseek(int fd, off_t offset, int  whence){
   return ::lseek(fd, offset, whence);
 }

  int  OS_dependent::read(int bytes, char *buffer, size_t amount){
    return ::read(bytes, buffer, amount);
  }

 int  OS_dependent::write(int bytes, char const *buffer, size_t amount){
   return ::write(bytes, buffer, amount);
 }

#if 0 // XXX: Linux arguments are more ugly than most
#ifndef _WIN32
 int  OS_dependent::ptrace(int request, int pid, char *addr, int data, char *addr2){
#if (defined(hp10))
   return ::ptrace(request, pid, (int)addr, data, (int)addr2);
#elif defined(__MC_MACOSX__)
   return ::ptrace(request, pid, addr, data);
#else
   return ::ptrace(request, (pid_t)pid, (int)addr, data);
#endif
 }
#endif
#endif

int  OS_dependent::readlink (char const *path, char *buf, int bufsiz){
#ifndef _WIN32
    return ::readlink(path, buf, bufsiz);
#else
    errno = EINVAL;
    return -1;
#endif
}


int  OS_dependent::unlink (char const *path){
  return ::unlink( OSPATH(path) );
}


char* OS_dependent::realpath (char const *path, char *resolved){
#ifndef _WIN32
    return ::realpath((char*)path, resolved);
#else
    return DisFN_realpath((char*)path, resolved);
#endif
}


char const *OS_dependent::rindex (char const *s, char c){
    return ::strrchr(s,c);
}

char* OS_dependent::strdup(char const *string)
{
    char *target = 0;
    if ( string )
    {
	target = (char *)malloc(strlen(string)+1);
	if ( target )
	    strcpy ( target, string );
    }

    return target;
}

#ifdef sun5
char const *OS_dependent::m_strerror(int errnum){
#else
char const *OS_dependent::strerror(int errnum){
#endif
#if defined(sun55) || defined(irix6)
   return (::strerror(errnum));
#elif defined(sun5)
   return (::strerror(errnum));
#else
   return (errnum < sys_nerr ? sys_errlist[errnum] : (char *)"Unknown error");
#endif
}

unsigned int  OS_dependent::sleep (unsigned seconds){
#ifndef _WIN32
    return ::sleep(seconds);
#else
    Sleep(seconds * 1000);
    return 0;
#endif
}

#ifndef _WIN32
unsigned int  OS_dependent::alarm (unsigned int  seconds){
    return ::alarm(seconds);
}
#endif

int OS_dependent::usleep (unsigned useconds){
    return ::usleep(useconds);
}

#ifndef _WIN32
int  OS_dependent::utimes (char const *file, struct timeval *tvp){
    return ::utimes(file, tvp);
}
#endif

#ifndef _WIN32
int OS_dependent::waitpid (int pid, int *statusp, int options){
    return ::waitpid(pid, statusp, options);
}
#endif

#if defined(sun5) || defined(_WIN32)
char *OS_dependent::strptime (char *, char *, struct tm *){
    return (char *)0;
#else
char *OS_dependent::strptime (char *buf, char *fmt, struct tm *tm){
    return ::strptime (buf, fmt, tm);
#endif
}

time_t OS_dependent::timelocal (struct tm *){
    return (time_t)0;
}

void OS_dependent::free (void *ptr) {
  ::free((char *)ptr);
}

size_t OS_dependent::strlen (char const *ptr) {
  return ::strlen(ptr);
}

char *OS_dependent::strncpy (char *s1, char const *s2, size_t n) {
  return ::strncpy(s1,s2,n);
}

char *OS_dependent::strcpy (char *s1, char const *s2) {
  return ::strcpy(s1,s2);
}

 char *OS_dependent::strcat (char *s1, char const *s2) {
  return ::strcat(s1,s2);
}

 char *OS_dependent::strchr (char const *string, int character) {
   return ::strchr((char *)string, character);
 }

int  OS_dependent::strcasecmp (char const *s1, char const *s2)
{
    while ( *s1 != '\0' )
    {
	if ( tolower(*s1) == tolower(*s2) )
	{
	    ++s1;
	    ++s2;
	}
	else
	{
	    if ( tolower(*s1) > tolower(*s2) )
		return 1;
	    else
		return -1;
	}
    }

    // At this point s1 has reached its end
    if ( *s2 == '\0' )
	return 0;
    else // s2 longer than s1
	return -1;
}

int  OS_dependent::strcmp (char const *s1, char const *s2) {
    return ::strcmp (s1, s2);
}

int  OS_dependent::strncmp (char const *s1, const  char *s2, const int n) {
    return ::strncmp (s1, s2, n);
}

int OS_dependent::atoi (char const *s1) {
    return ::atoi(s1);
}
#ifndef _WIN32
int OS_dependent::kill(int signal,int pid) {
    return ::kill(signal, pid);
}
#else
int OS_dependent::kill(int pid, int) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
    if(hProcess = NULL)
	return -1;
    if(TerminateProcess(hProcess, 0))
	return 0;
    else
	return -1;
}
#endif

void OS_dependent::exit(int errorcode){
    ::exit(errorcode);
    return;
}

void OS_dependent::_exit(int errorcode){
  ::_exit(errorcode);
  return;
}

#ifndef _WIN32
SIG_TYP OS_dependent::signal(int signal, SIG_TYP signal_handler) {
    return ::signal(signal, signal_handler);
}
#endif

#ifdef _WIN32
    struct timezone{
	char stubs[100];
    };
#endif

int OS_dependent::time(long *usec, long *sec)
{
    struct timeval tv;
    struct timezone tz;
    if(!OSapi_gettimeofday(&tv, &tz)){
	*usec=tv.tv_usec;
	*sec=tv.tv_sec;
	return 0;
    } else {
	return -1;
    }
}

FILE *OS_dependent::popen(char const *command, char const *type)
{
#ifdef _WIN32
  return _popen(command, type);
#else
  return ::popen(command, type);
#endif
}

int OS_dependent::pclose(FILE * in)
{
#ifdef _WIN32
  return _pclose(in);
#else
  return ::pclose(in);
#endif
}

#ifdef TEST_MACHDEP
main(){

time_t *t;
OSapi_time(t);
}
#endif

//-----------------------------------------------------------------------------
//	Fname convert
//
#ifdef _WIN32
int  OSapi_spawnv(int mode, char *cmdname, char **argv){
	char **argw = (char**)argv;
	for (; *argw; argw++ )
			cnv_argstr_2OS( *argw );
			return _spawnv( mode, OSPATH(cmdname), (char const* const*)argv);
}
#endif


int OSapi_stat( char const *path, struct OStype_stat *buffer )
{
#ifndef _WIN32
    return ::stat( path, buffer );

#else
    char drv_path[] = "c:\\";
    char *new_path;

    if ( isOSPATH() ) {
	new_path = (char *)OSPATH( path );
        if (new_path) {
	    if (strlen(new_path) == 2 && *(new_path+1) == ':' ) {
	        drv_path[0] = new_path[0];
	        new_path = drv_path;
            } else {
                int len = strlen(new_path);
                //remove trailing slash
                if (new_path[len-1] == '/' && new_path[len-2] != ':') {
                    new_path[len-1] = '\0';
                }
            }
        }
    }

    return _stat( new_path, buffer );
#endif
}


#ifndef _WIN32

char const *OSapi_getenv(char const *name) {
  return ::getenv(name);
}

#else



char* getenv_TMPDIR()
{
  char* pszTmpDir = ::getenv("TMPDIR_DIS");
  if ( pszTmpDir == 0 || *pszTmpDir == 0 )
  {
	  if ( (pszTmpDir = ::getenv("TMPDIR")) == 0 || *pszTmpDir == 0 )
    {
      if ( (pszTmpDir = ::getenv("TEMPDIR")) == 0 || *pszTmpDir == 0 )
      {
        if ( (pszTmpDir = ::getenv("TEMP")) == 0 || *pszTmpDir == 0 )
        {
          if ( (pszTmpDir = ::getenv("TMP")) == 0 || *pszTmpDir == 0 )
          {
            pszTmpDir = "C:\\TEMP";
          }
        }
      }
    }
  }

  if ( _access( pszTmpDir, 0) )
    CreateDirectory( pszTmpDir, NULL );
  return pszTmpDir;
}


const char *OSapi_getenv(char const *name) {

static char szPSETHOME[MAXPATHLEN] = "";
static char szTMPDIR[MAXPATHLEN] = "";
static char szHOME[MAXPATHLEN] = "";
static char szPSET_MAP_FILE[MAXPATHLEN] ="";
static char szPWD[MAXPATHLEN] ="";
static char szmp[MAXPATHLEN] ="";

#if defined(_WIN32) && defined(_DEBUG)
    char* pszDbgName = (char*)GetDiscoverRegistryDebug("getenv");
    if ( pszDbgName && stricmp(pszDbgName, name)==0 )
      _asm int 3;
#endif
	char *pszEnv = ::getenv(name);
  if ( isOSPATH() && pszEnv )
	{

    if ( stricmp(name, "TMPDIR") == 0 || stricmp(name, "TMP") == 0 || stricmp(name, "TEMP") == 0)
    {
		strcpy( szTMPDIR, pszEnv );
    if ( isOSPATH() )
      trn_path_2DIS( szTMPDIR );
    return szTMPDIR;
    }

    if ( stricmp(name, "PSETHOME") == 0 )
			{
				strcpy( szPSETHOME, pszEnv );
				int n = strlen(szPSETHOME)-1;
				if ( n >0 && szPSETHOME[n] == '/'  )
				     szPSETHOME[n] = '\0';
				return szPSETHOME;
			}
	if ( stricmp(name, "HOME") == 0 )
			{
				strcpy( szHOME, pszEnv );
				return trn_path_2DIS( szHOME );
			}
	if ( stricmp(name, "PSET_MAP_FILE") == 0 )
			{
				strcpy( szPSET_MAP_FILE, pszEnv );
				return trn_path_2DIS( szPSET_MAP_FILE );
			}
	if ( stricmp(name, "PWD") == 0 )
			{
        _getcwd( (char*)szPWD, (sizeof(char) * MAXPATHLEN));
		    return trn_path_2DIS( (char*)szPWD );
			}
			if ( strcmp(name, "mp") == 0 )
			{
				strcpy( szmp, pszEnv );
				return trn_path_2DIS( szmp );
			}
			else
				return pszEnv;
		}
		else
			return pszEnv;
}
#endif

int OSapi_putenv(char const *name) {
    return ::putenv((char *)name);
}

//----------------------------------------------
#ifdef _WIN32
int GetDiscoverRegistryDebug(char* pszKeyName) {
  return GetDisRegistryDebug(pszKeyName);
#else
int GetDiscoverRegistryDebug(char*) {
  return 0;
#endif
}

//----------------------------------------------
int isOSPATH()
{
  static int nOSPATH= -1;
	if ( nOSPATH < 0 )
		{
#if defined(_WIN32) && defined(_DEBUG)
    if ( GetDiscoverRegistryDebug("machdep") )
      _asm int 3;
#endif

     char* pszOSPATH;
		 if ( (pszOSPATH = ::getenv("DIS_NO_OS_PATH")) != NULL && atoi(pszOSPATH) != 0 )
			 nOSPATH = 0;
		 else
			 nOSPATH = 1;
		}
	return nOSPATH;
}


int is_abs_path( char const *path )
{
#ifndef _WIN32
	return ( path && *path == '/' );
#else
	return ( path && *(path+1) == ':' && (*(path+2) == '/' || *(path+2) == '\\') && isalpha(*path) );
#endif
}


const char *trn_path_2DIS ( const char *path )
{
#ifdef _WIN32
    return DisFN_trn_path_2DIS( (char *)path );
#endif
	return path;
}

void* pfun_realOSPATH_hashed =0;
typedef char const* (*PFUN_REALOSPATH_HASED)(char*);

char const *realOSPATH( char const *path )
{
// ATTENTION!
// This function if used out of pset_server or pmod_server
// returns pointer to the static buffer
// which will be overwritten in the next call.
// SO SAVE THE RESULT!
#ifndef _WIN32
		return (char*)path;
#else
  if ( pfun_realOSPATH_hashed )  // if pset_server or pmod_server
      return (char*)(*((PFUN_REALOSPATH_HASED)pfun_realOSPATH_hashed))( (char *)path);
  else
      return DisFN_realOSPATH( (char *)path ); // all other exetutables
#endif
}

char const *cnv_path_2OS( char const *path )
{
#ifndef _WIN32
		return (char*)path;
#else
                return DisFN_cnv_path_2OS( (char *)path );
#endif
}

#ifdef _WIN32
extern char* DisFN_create_path_2DIS_no_realpath( char * path, char** dis_path );
#endif

// for UNIX returns path and *dis_path == 0
// for NT *dis_path == allocated memory that a caller must free if *dis_path != 0
char* create_path_2DIS_no_realpath( char * path, char** dis_path )
{
#ifndef _WIN32
  if ( !path || !path[0] || !dis_path) return 0;
  *dis_path = 0;
  return path;
#else
  return DisFN_create_path_2DIS_no_realpath( path, dis_path );
#endif
}

char const *create_path_2DIS( char const * path, char** new_path )
{
#ifndef _WIN32
  if ( !path || !path[0] || !new_path) return 0;
  *new_path = 0;
  int len   = strlen(path);

  *new_path = (char*)malloc( len + 1);
  if ( *new_path ) strcpy( *new_path, path );
  return *new_path;
#else
  return DisFN_create_path_2DIS( (char *)path, new_path );
#endif
}


char const *cnv_argstr_path_2OS( char const * argstr )
{
#ifndef _WIN32
  return argstr;

#else
  if ( isOSPATH() && argstr )
	{
        const char *p = argstr;
	char *pcolumn, *pslash;
  while ( (pcolumn=strchr(p, ':')) )
		{
		 if ( pcolumn -2 >= p
          && (*(pslash= pcolumn -2) == '/' || *pslash == '\\')
          && (*(p=pcolumn+1) == '/' || *p == '\\')
        )
     {
       if ( strlen(pslash) >= 3 && isalpha(*(pslash+1)) )
			   memmove( pslash, pslash+1, strlen(pslash) );
       else
         p = pcolumn+1;
     }
     else
      p = pcolumn+1;
		}
  }
	return argstr;
#endif
}
#ifdef _WIN32
char const* cnv_argstr_delims_2OS( char const *argstr )
{
//#ifndef _WIN32
//  return argstr;
//
//#else
  if ( isOSPATH() && argstr )
	{
        char *delim = (char *)argstr;
	while ( (delim=strchr(delim, ';')) && *(delim-1) != '\\')
		*delim = '&';
  }
	return argstr;
}
#endif

#ifdef _WIN32
char const* cnv_argstr_chdir_2OS( char const *argstr )
{
  if ( isOSPATH() && argstr )
  {
    char *pCHDIR = (char *)argstr;
    char *delim, *pslash;
    char  c, cp;
    while ( (c= *pCHDIR) )
    {
      if (  (pCHDIR == argstr || ((cp=*(pCHDIR-1)) == ' ' || cp == '\t' || cp == '(' || cp == '"' )) &&
	    (c == 'c' || c == 'C' ) &&
	    ((cp=*(pCHDIR+1)) == 'd' || cp == 'D' ) &&
	    ((cp=*(pCHDIR+2)) == ' ' || cp == '\t') &&
	    (*(pCHDIR+3) == '/' ) &&
	    ((cp=*(pCHDIR+4)) == 'd' || cp == 'D' ) &&
	    ((cp=*(pCHDIR+5)) == ' ' || cp == '\t')
	  )
      {
	pCHDIR += sizeof(CHDIR)-1;
	while ( (c=*pCHDIR) == ' ' || c == '"' || c == '\t' )
	  pCHDIR++;
	if ( *pCHDIR  )
	{
	  delim = pCHDIR;
	  while ( (c=*delim) != '\0' && c != '&' && c != ')' && c != ' ' && c != '"' )
	    delim++;
	  *delim = '\0';
	  while ( (pslash=strchr(pCHDIR, '/')) )
	    *pslash = '\\';
	  *delim = c;
	  }
      }
      else
	pCHDIR++;
    }
  }
  return argstr;
}
#endif

#ifdef _WIN32
char const *cnv_argstr_2OS(char const *argstr)
{
  if ( isOSPATH() && argstr )
  {
    cnv_argstr_path_2OS( argstr );
    cnv_argstr_delims_2OS( argstr );
    cnv_argstr_chdir_2OS( argstr );
  }
  return argstr;
}
#endif

#ifdef _WIN32
void cnv_arglist_path_2OS( char** arglist )
{
  while ( *arglist )
  {
      *arglist = (char *)cnv_path_2OS( *arglist );
    arglist++;
  }
}
#endif

char const *convert_to_uniform_case(char const *path)
{
#ifdef _WIN32
    return DisFN_convert_to_uniform_case( (char *)path );
#else
  return path;
#endif
}


char const *convert_back_slashes( char const *path )
{
#ifdef _WIN32
	if ( path )
  {
  char *pslash;
	while ( (pslash=strchr(path, '\\')) )
		 *pslash = '/';
  }
#endif
	return path;
}

char const* convert_forward_slashes( char const *path )
{
	if ( path )
  {
	char *pslash;
	while ( (pslash=strchr(path, '/')) )
		 *pslash = '\\';
	}
	return path;
}


//--------------------------------------------------
extern "C" int dos2unix(char * text, int len)
/* remove in place carriage return from string
 * of given length; returns new length
 */
{
    int ii = 0, jj = 0;
    for (; ii < len; ++ii)
      {
        if (text[ii] != '\r')
	  {
	    if (ii != jj)
	      text[jj] = text[ii];
	    jj++;
	  }
      }
    return jj;
}


#define KB 1024

#ifndef _WIN32
#if defined(hp10)
#define STAT_TYPE statfs
#else
#define STAT_TYPE statvfs
#endif

long OSapi_freeDiskSpace(char *fname)
{
  struct STAT_TYPE buf_fs;
  if (STAT_TYPE(fname, &buf_fs) == 0) {
#if defined(sun5)
    return (buf_fs.f_bavail*buf_fs.f_frsize/KB);
#elif defined(irix6)
    return (buf_fs.f_bfree*buf_fs.f_frsize/KB);
#else
    return (buf_fs.f_bavail*buf_fs.f_bsize/KB);
#endif
  }
  else {
    return -1;
  }
}

long OSapi_freeSwapSpace(void)
{
#ifdef sun5
  struct anoninfo anon;
  int err = swapctl(SC_AINFO, &anon);
  if (err != -1) {
    long pgsz;
    pgsz = sysconf(_SC_PAGESIZE);
    return ((anon.ani_max - anon.ani_resv)*pgsz/KB);
  }
#elif defined(irix6)
  long swap_tot, free_swap, res_virt, log_swap_tot, free_phy, res_phy;

  if ((swapctl(SC_GETSWAPTOT, &swap_tot) != -1) && (swapctl(SC_GETFREESWAP, &free_swap) != -1) && ( swapctl(SC_GETRESVSWAP, &res_virt) != -1) && (swapctl(SC_GETLSWAPTOT, &log_swap_tot) != -1)) {
    res_phy = res_virt - (swap_tot - free_swap);
    free_phy = log_swap_tot - swap_tot - res_phy;
    return ((free_swap+free_phy)*getpagesize()/KB);
  }
#endif
  return -1;
}
#endif /* _WIN32 */

#ifdef irix6
char *get_fileSystemType(char *temp_dir)
{
  struct statvfs buf_fs;
  if (statvfs(temp_dir, &buf_fs) == 0)
    return  buf_fs.f_basetype;
  else
    return NULL;
}
#endif
