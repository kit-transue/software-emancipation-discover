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
#include <fileEntry.h>

#include <sstream>

#include <cLibraryFunctions.h>
#include <fileCache.h>
#include <genError.h>
#include <genString.h>
#include <systemMessages.h>
#include <vpopen.h>
#include <machdep.h>
#include <proj_save.h>
#include <psetmem.h>
#include <dis_iosfwd.h>

#define F_PATH          "/usr/bin/"

static char Frame[] = "Frame";  /* to check File language */


init_relational(fileEntry,Relational);
void remove_last_slash(genString& new_path);

char const *fileEntry::get_name() const 
{
    char const *nm = path.str();
    return nm;
}


fileEntry::fileEntry() : stat_buf(0), lstat_buf(0)
{
    reset();

    path = "";

    realpath_is_valid = 1;
    stat_is_valid     = 1;
    lstat_is_valid    = 1;
    ascii_is_valid    = 1;
    frame_is_valid    = 1;
    rok_is_valid      = 1;
    wok_is_valid      = 1;
    xok_is_valid      = 1;
    fok_is_valid      = 1;
    dir_is_valid      = 1;
    link_is_valid     = 1;
}

fileEntry::fileEntry (const char *file_path) :
    path(file_path), stat_buf(0), lstat_buf(0)
{
    reset();
}

fileEntry::~fileEntry ()
{
    if (stat_buf)  OSapi_free(stat_buf);
    if (lstat_buf && lstat_buf != stat_buf) OSapi_free(lstat_buf);
}

void fileEntry::reset()
{
    stat_is_valid  = 0; _stat_ok  = 0;
    lstat_is_valid = 0; _lstat_ok = 0;
    ascii_is_valid = 0; _ascii_ok = 0;
    frame_is_valid = 0; _frame_ok = 0;
    rok_is_valid   = 0; _rok_ok   = 0;
    wok_is_valid   = 0; _wok_ok   = 0;
    xok_is_valid   = 0; _xok_ok   = 0;
    fok_is_valid   = 0; _fok_ok   = 0;
    dir_is_valid   = 0; _dir_ok   = 0;
    link_is_valid  = 0; _link_ok  = 0;

    lstat_errno = stat_errno = realpath_errno = 0;

    ascii_error_text = (char *)0;
    realpath = (char *)0; realpath_is_valid = 0; _realpath_ok = 0;

    if (stat_buf)  OSapi_free(stat_buf);
    if (lstat_buf && lstat_buf != stat_buf) OSapi_free(lstat_buf);
    stat_buf = lstat_buf = 0;
}

extern void gen_print_indent(ostream&, int);

void fileEntry::print (ostream& st, int lev) const
{
    const char *null_ptr = "NULL";
    const char *path_name     = (path.length())			? 
					(char *)path		 : null_ptr;
    const char *realpath_name = (realpath.length())		?
					(char *)realpath	 : null_ptr;
    const char *ascii_error   = (ascii_error_text.length())	?
					(char *)ascii_error_text : null_ptr;
    
    gen_print_indent (st, lev);
    st << "Name: " << path_name << "\n";

    gen_print_indent (st, lev);
    st << "Valid flags for real, stat, ascii, lstat, rok, wok, frame: "
	<< (int) realpath_is_valid << " "
	<< (int) stat_is_valid << " "
	<< (int) ascii_is_valid << " "
	<< (int) lstat_is_valid << " "
	<< (int) rok_is_valid << " "
	<< (int) wok_is_valid << " "
	<< (int) frame_is_valid << "\n";

    gen_print_indent (st, lev);
    st << "realpath: " << realpath_name << "\n";
    gen_print_indent (st, lev);
    st << "error text: " << ascii_error << "\n";
}

extern int prj_env_substitute (const char *orig, genString& sub);

/* FORMAT:      "%s%s%d%d%d"
*  (char *)path
*  prjDELEMETER = "_FLDX_"
*  is_fok
*  is_link
*  is_dir
*/
void fileEntry::put_signature(ostream& os) const
{
    Initialize(fileEntry::put_signature);
    genString txt;
    genString sub;
    char const *phy = OSPATH((char *)path);

    if ( prj_env_substitute ((char *)path, sub) )
	phy = (char *)sub;

    txt.printf ("%s%s%d%d%d%d", phy, prjDELEMETER, _fok_ok, _link_ok, _dir_ok, _xok_ok);

    os << (char*)quote_str_if_space( (char *)txt );
}

int fileEntry::is_frame (int *was_valid)
{
    Initialize(fileEntry::is_frame);
    int res = 0;
    if (was_valid) *was_valid = 0;

    if (frame_is_valid) {
	res = _frame_ok;
	if (was_valid) *was_valid = 1;
    } else {
	if (!ascii_is_valid) {
	    int fd;
	    if ((fd=OSapi_open ((char*)path,O_RDONLY))>=0) {
		char buf[11];
		int n=OSapi_read(fd,buf,11);
		OSapi_close(fd);
		if (n>4 && buf[0]=='<' && buf[1]=='M')
		    if (!strncmp(&buf[2],"ML",2) || !strncmp(&buf[2],"IFFile",6)
			|| !strncmp(&buf[2],"akerFile", 8))
			res = 1;
	    }
	    if (res) {
		ascii_is_valid = 1;
		_ascii_ok = 0;
		ascii_error_text = "Frame Maker";
	    }
	} else if (!_ascii_ok) {	
	    char *err = ascii_error_text;
	    if (err && err[0]) {
		if (strstr(err, Frame))
		    res = 1;
	    }
	    
	}
	frame_is_valid = 1;
	_frame_ok = res;
    }
    return res;
}

int fileEntry::is_ascii (int *was_valid)
{
    Initialize(fileEntry::is_ascii);

#ifndef WIN32
    char realname[MAXPATHLEN];
    genString results;
    int ret = 0;
    if (!ascii_is_valid) {
        this->get_realpath(realname);
        ostringstream stream;
        stream << F_PATH << "file " << realname;
        stream << ends;
        const char *cmd = stream.str().c_str();
        char the_str[2048];
    
        _ascii_ok = 0;
        FILE* pipe = vpopen_sync (cmd, "r");
        if (pipe) {
            while (OSapi_fgets (the_str, sizeof(the_str)-1, pipe)) {
                const int len = strlen (the_str);
                if (len == 0)
                    continue;

		//look for colon between filename and output
		char *str_out = strrchr(the_str, ':');
		if (str_out)
		    str_out = str_out + 1;
		else
		    str_out = the_str;

		if (str_out[0]) {
		    if (strstr(str_out,"text") ||
			strstr(str_out,"empty")||
#ifdef irix6
                        strstr(str_out,"Empty") ||    // for irix6
#endif
			strstr(str_out,"shell") ||
			strstr(str_out,"data") ||
			strstr(str_out,"command") ||
			strstr(str_out,"script"))
			_ascii_ok = 1;

		    results += str_out;
		}
            }
            OSapi_fclose(pipe);
        }
    
        if (!_ascii_ok) {
	    if ((char*)results)
		ascii_error_text = (char*)results;
	    else 
		ascii_error_text = NULL;
	}

        ascii_is_valid = 1;
    } else
	if (was_valid) *was_valid = 1;
    
    if (!_ascii_ok)
        ret = 0;
    else
        ret = 1;

    return ret;
#else
    return 1;
#endif
}

// Returns: 0 on success
//         -1 if file does not exist -or- permition is not granted
int fileEntry::access (int r_w, int *was_valid)
{
    int ret = 0;
    if ( !(r_w == R_OK || r_w == W_OK || r_w == X_OK || r_w || F_OK) )
	ret = OSapi_access ((char *)path, r_w);
    else {
	if ( (r_w == R_OK && !rok_is_valid) ||
	     (r_w == W_OK && !wok_is_valid) ||
	     (r_w == X_OK && !xok_is_valid) ||
	     (r_w == F_OK && !fok_is_valid) ) {
	    if (was_valid) *was_valid = 0;
	    ret = this->stat ((struct OStype_stat *)0, was_valid);
	} else
	    if (was_valid) *was_valid = 1;
	
	if (ret == 0) {
	    if (r_w == R_OK)
		ret = (_rok_ok) ? 0 : -1;
	    else if (r_w == W_OK)
		ret = (_wok_ok) ? 0 : -1;
	    else if (r_w == X_OK)
		ret = (_xok_ok) ? 0 : -1;
	    else if (r_w == F_OK)
		ret = (_fok_ok) ? 0 : -1;
	} else
	    ret = -1;
    }
    return ret;
}


int fileEntry::is_rok (int *was_valid)
{
    Initialize(fileEntry::is_rok);
    if (!rok_is_valid) {
	if (was_valid) *was_valid = 0;
	int ret = this->stat ((struct OStype_stat *)0);
    } else
	if (was_valid) *was_valid = 1;

    return _rok_ok;
}

int fileEntry::is_wok (int *was_valid)
{
    Initialize(fileEntry::is_wok);
    if (!wok_is_valid) {
	if (was_valid) *was_valid = 0;
	int ret = this->stat ((struct OStype_stat *)0);
    } else
	if (was_valid) *was_valid = 1;

    return _wok_ok;
}


int fileEntry::is_fok (int *was_valid)
{
    Initialize(fileEntry::is_fok);
    if (!fok_is_valid) {
	if (was_valid) *was_valid = 0;
	int ret = this->stat ((struct OStype_stat *)0);
    } else
	if (was_valid) *was_valid = 1;

    return _fok_ok;
}

// sets _link_ok = 1, if the path is link
int fileEntry::lstat (struct OStype_stat *buf, int *was_valid)
{
    if (was_valid) *was_valid = 0;
    int ret = -1;
    if (lstat_is_valid && lstat_buf) {
	if(was_valid) *was_valid = 1;
	errno = lstat_errno;
    } else {
	lstat_buf = (struct OStype_stat *)psetmalloc (sizeof(struct OStype_stat));
	if (lstat_buf) {
//sudha:11/13/97, lstat fails on HP if path contains a slash at the end
            genString ch_path = path;
            remove_last_slash(ch_path);
            ret = OSapi_lstat(ch_path, lstat_buf);
	    _lstat_ok = (ret == 0) ? 1 : 0;
	    lstat_errno = errno;
	    _link_ok  = (ret == 0) ? OSapi_S_ISLNK(lstat_buf) : 0;
	    link_is_valid = lstat_is_valid = 1;
	}
    }

    if (buf && lstat_buf) *buf = *lstat_buf;
    ret  = (_lstat_ok) ? 0 : -1;
    return ret;
}

int fileEntry::stat (struct OStype_stat *buf, int *was_valid)
{
    Initialize(fileEntry::stat__stat*_int*);
    int ans = -1;
    if (stat_is_valid && stat_buf) {
	if (was_valid) *was_valid = 1;
	if (buf) *buf = *stat_buf;   
	errno = stat_errno;
    } else {
	if (was_valid) *was_valid = 0;
	int lstat_ret = this->lstat (buf, was_valid);
	if (_lstat_ok && ! _link_ok) {
	    // use the same pointer for lstat_buf and stat_buf
	    stat_buf = lstat_buf;
	    errno = stat_errno = lstat_errno;
	    stat_is_valid = 1;
	    _stat_ok = _lstat_ok;
	} else if (_lstat_ok) {
	    stat_buf = (struct OStype_stat *)psetmalloc (sizeof(struct OStype_stat));
	    if (stat_buf) {
		int stat_ret  = OSapi_stat(path, stat_buf);
		_stat_ok      = (stat_ret == 0) ? 1 : 0;
		stat_errno    = errno;
		stat_is_valid = 1;
		if (buf) *buf = *stat_buf;
	    }
	}
	
	set_flags();
    }
    
    ans = (_stat_ok) ? 0 : -1;
    return ans;
}

void fileEntry::set_flags ()
{
    Initialize(fileEntry::set_flags);
    if (stat_is_valid) {
	rok_is_valid = wok_is_valid = xok_is_valid = fok_is_valid = dir_is_valid = 1;
	if (stat_buf && _stat_ok) {
	    _fok_ok = 1;
	    mode_t st = stat_buf->st_mode;

#ifndef _WIN32
	    int rok = ((st & S_IROTH) || (st & S_IRGRP) || (st & S_IRUSR));
	    int wok = ((st & S_IWOTH) || (st & S_IWGRP) || (st & S_IWUSR));
	    int xok = ((st & S_IXOTH) || (st & S_IXGRP) || (st & S_IXUSR));

	    _dir_ok = S_ISDIR(st);
	    _rok_ok = (_dir_ok) ? (rok && xok) : rok;
	    _wok_ok = (_dir_ok) ? (wok && xok) : wok;
	    _xok_ok = xok;
#else
            int rok = !!(st & _S_IREAD);
            int wok = !!(st & _S_IWRITE);
            int xok = !!(st & _S_IEXEC);
            _dir_ok = !!(st & _S_IFDIR);
            _rok_ok = (_dir_ok) ? (rok && xok) : rok;
            _wok_ok = (_dir_ok) ? (wok && xok) : wok;
            _xok_ok = xok;
#endif
	} else 
	    _fok_ok = _rok_ok = _wok_ok = _xok_ok = _dir_ok = 0;
    }
}

void fileEntry::restore_flags (projLine& pline, bool is_phys, bool can_write)
{
    Initialize(fileEntry::restore_flags__projLine&_PRJ_filed__bool);
    if (is_phys && !pline.is_flag_ok(PRJ_phys, PRJ_fok)) {
	_fok_ok = _rok_ok = _wok_ok = _xok_ok = _dir_ok = _link_ok = 0;
	rok_is_valid = wok_is_valid = xok_is_valid = fok_is_valid = dir_is_valid = link_is_valid = 0;
    } else {
	rok_is_valid = wok_is_valid = xok_is_valid = fok_is_valid = dir_is_valid = link_is_valid = 1;
	PRJ_field fld = (is_phys) ? PRJ_phys : PRJ_model;
	_fok_ok  = (pline.is_flag_ok(fld, PRJ_fok)) ? 1 : 0;
	_rok_ok  = 1;
	_wok_ok  = (can_write) ? 1 : 0;
	_xok_ok  = (pline.is_flag_ok(fld, PRJ_xok)) ? 1 : 0;
	_dir_ok  = (pline.is_flag_ok(fld, PRJ_dok)) ? 1 : 0;
	_link_ok = (pline.is_flag_ok(fld, PRJ_lok)) ? 1 : 0;
    }
}

char *fileEntry::get_realpath (char *resolved_path, int *was_valid) 
{
    Initialize(fileCache::get_realpath);

    if (realpath_is_valid) {
	if (was_valid) *was_valid = 1;
	if (realpath.length())
	    strncpy(resolved_path,(char *)realpath, MAXPATHLEN);
        else
            resolved_path[0] = 0;
    } else {
	if (was_valid) *was_valid = 0;
	char const *val = OSapi_realpath (path, resolved_path);
	realpath = resolved_path;
        realpath_is_valid = 1;
    }

    return (char *)realpath;
}

//This function replaces the last slash character in path with '\0'
void remove_last_slash(genString& new_path)
{
  unsigned int len = new_path.length();

  if ((len > 1) && (new_path[len -1] == '/'))
    ((char*)new_path)[len-1] = '\0';
}
    

