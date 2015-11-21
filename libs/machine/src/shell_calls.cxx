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
#include <shell_calls.h>
#ifndef ISO_CPP_HEADERS
#include <stdarg.h>
#else /* ISO_CPP_HEADERS */
#include <cstdarg>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <charstream.h>
#include <general.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <vpopen.h>
#ifndef _WIN32
#include <dirent.h>
#else
#include <stdlib.h>
#endif
#include <machdep.h>
#include <genString.h>

#ifdef _WIN32
#define STDERR_DEV_NULL  "nul"
#else
#define STDERR_DEV_NULL  "/dev/null"
#endif

#ifdef _WIN32
static int comp_strings( const void *arg1, const void *arg2 )
{
   /* Compare all of both strings: */
   return OSapi_strcmp( * ( char** ) arg1, * ( char** ) arg2 );
}

int unique_sort_file( const char *source, const char *target )
{
    int fResult = 0;
    FILE *src   = OSapi_fopen(source, "r");
    if( src )
    {
	FILE *trg = OSapi_fopen(target, "w");
	if( trg )
	{
	    fseek(src, 0, SEEK_END);
	    long fsize = ftell(src);
	    fseek(src, 0, SEEK_SET);
	    char *file_buf = new char[fsize];
	    if(file_buf == NULL){
		OSapi_fclose(src);
		OSapi_fclose(trg);
		return 0;
	    }
	    long readed = OSapi_fread(file_buf, 1, fsize, src);
	    if(readed != fsize){
		OSapi_fclose(src);
		OSapi_fclose(trg);
		return 0;
	    }
	    // count number of strings
	    int nStrings = 1;
	    char *ptr    = file_buf;
	    for(int i = 0; i < fsize; i++, ptr++)
		if(*ptr == '\n')
		    nStrings++;
	    if(nStrings > 1)
	    {
		const char **sort_array = new const char *[nStrings];
		if( sort_array )
		{
		    fResult       = 1;
		    sort_array[0] = file_buf;
		    int index     = 1;
		    char *ptr     = file_buf;
		    for(int i = 0; i < fsize; i++, ptr++)
			if(*ptr == '\n'){
                            *ptr = '\0';
			    sort_array[index++] = ptr + 1;
                        }
		    qsort((void *)sort_array, (size_t)nStrings - 1,
			  sizeof(char *), comp_strings );
		    //Print all unique strings to a file
		    const char *pszCurrent = sort_array[0];
		    OSapi_fprintf( trg, "%s", pszCurrent); 
		    for( int nInd = 1; nInd < nStrings; nInd++ )
			if( OSapi_strcmp( (char *)pszCurrent, (char *)sort_array[nInd] ) )
			{
			    pszCurrent = sort_array[nInd];
			    OSapi_fprintf( trg, "\n%s", pszCurrent); 
			}
		    delete [] sort_array;
		}
	    }
	    delete [] file_buf;
	}
	OSapi_fclose( trg );  
    }
    OSapi_fclose( src );
    return fResult;
}
#endif /*_WIN32*/

static bool has_spaces(const char* arg)
{
  int ch;
  if(!arg || !*arg)
    return true;
  if(arg[0] == '"')
    return false;

  while(ch=*arg++)
    if(isspace(ch))
      return true;

  return false;
}

static void add_argument (ocharstream&, const char*, char separator = ' ');
static void add_argument ( ocharstream& cmd, const char *arg, char separator )
{
    if (separator)
	cmd << separator;

    if(has_spaces(arg))
	cmd << '"' << arg << '"';
    else
	cmd << arg;
}

static void add_ospath (ocharstream&, const char*, char separator = ' ');
static void add_ospath ( ocharstream& cmd, const char *path, char separator )
{
#ifdef _WIN32
    genString spath = OSPATH(path);
    const char* ospath = convert_forward_slashes( (char*)(spath.str()) );
    add_argument( cmd, ospath, separator );
#else
    add_argument( cmd, path, separator );
#endif
}

static void add_redirect (ocharstream&, const char *, const char *std_err = STDERR_DEV_NULL);
static void add_redirect (ocharstream& cmd, const char *std_out, const char *std_err)
{
    if (std_out && std_out[0]) {
	cmd << " 1>";
	add_ospath (cmd, std_out );
    }

    if (!std_err || !std_err[0])
	std_err = STDERR_DEV_NULL;
    
    cmd << " 2>";
    add_ospath (cmd, std_err );
}

// Add a space infront of each argument.
void static add_vsystemv_args (ocharstream& cmd, int argc, const char **argv)
{
    for(int ii=0; ii<argc; ++ii) {
	const char* arg = argv[ii];
	add_argument (cmd, arg);
    }
}

int vsystemv(int argc, const char** argv)
{
    int res = 0;
    if (argc == 0)
	return res;

    ocharstream cmd;
    const char *arg0 = argv[0];
    add_argument (cmd, arg0, (char)0); // no separator infront

    argc --;
    if (argc > 0) {
	argv = &argv[1];
	add_vsystemv_args (cmd, argc, argv);
    }

    cmd             << ends;
    const char *txt = cmd.ptr();
    res             = vsystem(txt);
    return res;
}

int vsystemv_redirect(const char *std_out, int argc, const char** argv)
{
    int res = 0;
    if (argc == 0)
	return res;

    ocharstream cmd;
    const char *arg0 = argv[0];
    add_argument (cmd, arg0, (char)0); // no separator infront

    argc --;
    if (argc > 0) {
	argv = &argv[1];
	add_vsystemv_args (cmd, argc, argv);
    }
    add_redirect (cmd, std_out, STDERR_DEV_NULL);

    cmd             << ends;
    const char *txt = cmd.ptr();
    res             = vsystem(txt);
    return res;
}

int vsystemv_command_redirect(const char *std_out, const char *arg0, int argc, const char** argv)
{
    ocharstream cmd;
    add_argument (cmd, arg0, (char)0); // no separator infront

    if (argc > 0) {
	argv = &argv[1];
	add_vsystemv_args (cmd, argc, argv);
    }
    add_redirect (cmd, std_out, STDERR_DEV_NULL);

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

// Adds separator (space) infront of each argument
static void add_vsysteml_list (ocharstream& cmd, va_list ap)
{
    const char *arg = va_arg(ap, const char*);
    while(arg){
	add_argument (cmd, arg);
	arg = va_arg(ap, const char*);
    }
}

// No space infront of first
static void add_vsysteml_args (ocharstream& cmd, const char *arg0 ...)
{
    add_argument (cmd, arg0, (char)0);  // no space infront

    va_list ap;
    va_start(ap, arg0);
    add_vsysteml_list (cmd, ap);
    va_end(ap);
}

// No space infront of first
int vsysteml(const char*arg0 ...)
{
    ocharstream cmd;
    add_argument (cmd, arg0, (char)0);   // no space infront

    va_list ap;
    va_start(ap, arg0);
    add_vsysteml_list (cmd, ap);
    va_end(ap);

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

// No space infront of first
int vsysteml_redirect (const char *std_out, const char*arg0 ...)
{
    ocharstream cmd;
    add_argument (cmd, arg0, (char)0);  // no space infront

    va_list ap;
    va_start(ap, arg0);
    add_vsysteml_list (cmd, ap);
    va_end(ap);

    add_redirect (cmd, std_out, STDERR_DEV_NULL);

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

// No space infront of first
int vsysteml_redirect_stderr (const char *std_out, const char *std_err, const char*arg0 ...)
{
    ocharstream cmd;
    add_argument (cmd, arg0, (char)0);  // no space infront

    va_list ap;
    va_start(ap, arg0);
    add_vsysteml_list (cmd, ap);
    va_end(ap);

    add_redirect (cmd, std_out, std_err);

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

static void add_cd (ocharstream& cmd, const char *dir)
{
#ifdef _WIN32
		cmd << "cd /D";
#else
    cmd << "cd";
#endif
    add_ospath (cmd, dir );
    cmd << " ; ";
}

int shell_cd_ls (const char *out_fn, const char *dir, int argc, const char **ls_argv)
{
    ocharstream cmd;
    add_cd (cmd, dir);

#ifdef _WIN32
    cmd << "dir";
#else
    cmd << "ls";
#endif    

    add_vsystemv_args (cmd, argc, ls_argv); // adds a space infront of each
    add_redirect (cmd, out_fn);

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

int shell_cd_exec (const char *dir, const char *arg0 ...)
{
    ocharstream cmd;
    add_cd (cmd, dir);
    add_argument (cmd, arg0, (char)0);  // no space infront

    va_list ap;
    va_start(ap,arg0);
    add_vsysteml_list (cmd, ap);
    va_end(ap);

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

int shell_cd_exec_touch_background (const char *dir, const char *exe, const char *out_fn, const char *touch_fn)
{
    ocharstream cmd;
    cmd << "((";
    add_cd (cmd, dir);
    cmd << exe;   // Let us leave it up to the user
    cmd << ")>";
    add_argument (cmd, out_fn, (char)0);  // no space infront
#ifdef _WIN32
    cmd << "& type nul > ";
    add_ospath (cmd, touch_fn );
    cmd << ")";
#else
    cmd << ";touch";
    add_argument (cmd, touch_fn);
    cmd << ")&";
#endif

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

int shell_cd_exec_redirect (const char *out_fn, const char *dir, const char *arg0, ...)
{
    ocharstream cmd;
    add_cd (cmd, dir);
    cmd << arg0;   // Let us leave it up to the user

    va_list ap;
    va_start(ap,arg0);
    add_vsysteml_list (cmd, ap);
    va_end(ap);

    add_redirect (cmd, out_fn);

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

int shell_mkdir (const char *dir, int force)
{
    if (!dir || !dir[0])
	return -1;

    char const *dir_name = OSPATH(dir);
    int ret_val = -1;

    char *new_dir = OSapi_strdup(dir_name);
#ifdef _WIN32
    int len = OSapi_strlen(new_dir);
    //remove trailing slash
    if (new_dir[len-1] == '/' || new_dir[len-1] == '\\') {
        new_dir[len-1] = '\0';
    }

    struct OStype_stat stat_buf;
    if(OSapi_stat(new_dir, &stat_buf) == 0) {
	if(stat_buf.st_mode & _S_IFDIR != 0) {
       	    ret_val = 0;
        }
    }
#else
    DIR *dirp = opendir(new_dir);
    if (dirp) {
	closedir(dirp);
	ret_val = 0;
    }
#endif

    if (ret_val != 0) {
	ocharstream cmd;
	add_argument (cmd, "mkdir", (char)0);
#ifndef _WIN32
	if (force > 0)
	    add_argument (cmd, "-p");
#endif
	add_ospath (cmd, new_dir);

	cmd             << ends;
	const char *txt = cmd.ptr();
	ret_val		= vsystem(txt);
    }
    return ret_val;
}

int shell_rmdir (const char *dir) 
{
    int res = -1;
    if (!dir || !dir[0])
	return res;

    ocharstream cmd;
#ifdef _WIN32
    cmd << "rmdir /s /q";
    dir = OSPATH(dir);
#else
    cmd << "rm -rf";
#endif
    add_ospath (cmd, dir);

    cmd             << ends;
    const char *txt = cmd.ptr();
    res             = vsystem(txt);
    return res;
}

int shell_rm ( const char *fn )
{
    int res = -1;
    if (!fn || !fn[0])
	return res;

    ocharstream cmd;
#ifdef _WIN32
    cmd << "del /q";
    fn = OSPATH(fn);
#else
    cmd << "rm -f";
#endif
    add_ospath (cmd, fn);

    cmd             << ends;
    const char *txt = cmd.ptr();
    res             = vsystem(txt);
    return res;
}

int shell_touch ( const char *fn )
{
    int res = -1;
    if (!fn || !fn[0])
	return res;

    ocharstream cmd;
#ifdef _WIN32
    cmd << "type nul > ";
#else
    cmd << "touch";
#endif
    add_ospath (cmd, fn );

    cmd             << ends;
    const char *txt = cmd.ptr();
    res             = vsystem(txt);
    return res;
}

int shell_cp ( const char *from, const char *to)
{
    int status = OS_dependent::access (from, R_OK);
    if ( status != 0)
	return status;

    if (OS_dependent::access (to, F_OK) == 0) {
	status = shell_rm (to);
	if (status != 0)
	return status;
    }
    
    ocharstream cmd;
#ifdef _WIN32
    cmd << "copy";
#else
    cmd << "cp";
#endif

    add_ospath (cmd, from );
    add_ospath (cmd, to );

    cmd             << ends;
    const char *txt = cmd.ptr();
    status          = vsystem(txt);
    return status;
}

int shell_mv (const char * old_fn, const char * new_fn, int force)
{
    int status = -1; 
    if ( !(old_fn && new_fn && old_fn [0] && new_fn[0]) )
	return status;

    status = OSapi_rename(old_fn, new_fn);
    if (status == 0)
	return status;

    ocharstream cmd;
#ifdef _WIN32
    cmd << "move";
    old_fn = OSPATH(old_fn);
    new_fn = OSPATH(new_fn);
#else
    if (force)
	cmd << "mv -f";
    else
	cmd << "mv";
#endif

    add_ospath (cmd, old_fn );
    add_ospath (cmd, new_fn );

    cmd << ends;
    const char *txt = cmd.ptr();
    status = vsystem(txt);

    if (status != 0) {
	status = shell_cp (old_fn, new_fn);
	if (status == 0)
	    shell_rm (old_fn);
    }
    return status;
}

int shell_sort_unique ( const char *src, const char *sorted )
{
    int status;
#ifdef _WIN32
    status = !unique_sort_file( OSPATH(src), OSPATH(sorted) );
#else
    status = vsysteml_redirect (sorted, "sort", "-u", src, NULL);
#endif
    return status;
}

int shell_cd_find (const char *out_fn, const char *dir, const char *find_arg, int do_sort)
{
    ocharstream cmd;
    add_cd (cmd, dir);

#ifdef _WIN32
     add_vsysteml_args (cmd, "dir", "/S", "/B", NULL);
#else
    add_vsysteml_args (cmd, "find", find_arg, "-print", NULL);
#endif

    if (do_sort)
	add_vsysteml_args (cmd, "|", "sort", NULL);
    add_redirect (cmd, out_fn);

    cmd             << ends;
    const char *txt = cmd.ptr();
    int res         = vsystem(txt);
    return res;
}

int shell_change_to_writable (const char *fname)
{
    int res = -1;

    genString ch_to_wr = OSapi_getenv("PSETCFG");
    if (!ch_to_wr.str())
        return res;
    ch_to_wr += "/cm_change_to_write";
    if (OS_dependent::access (ch_to_wr, X_OK))
        return res;

    res = vsysteml (ch_to_wr, fname, NULL);
    return res;
}

int shell_indent (const char * old_fn, const char * new_fn)
{
    if ( !(old_fn && new_fn && old_fn [0] && new_fn[0]) )
	return -1;

    int status = vsysteml ("indent", OSPATH(old_fn), OSPATH(new_fn), NULL);
    return status;
}
