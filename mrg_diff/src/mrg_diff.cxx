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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <charstream.h>
#include <shell_calls.h>
#include <mrgFile.h>
#include <genString.h>
#include <genTmpfile.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <cLibraryFunctions.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <io.h>
#include <stdlib.h>

int execute_diff(char *cmd, const char *output) 
{
	HANDLE h;
	SECURITY_ATTRIBUTES sattr;
	memset(&sattr, 0, sizeof(sattr));
	sattr.nLength        = sizeof(sattr);
	sattr.bInheritHandle = TRUE;

	h = CreateFile(output, GENERIC_WRITE, 0, &sattr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb         = sizeof(sinfo);
	sinfo.hStdOutput = h;
	sinfo.dwFlags    = STARTF_USESTDHANDLES;
	
	PROCESS_INFORMATION pinfo;

	if(!CreateProcess(NULL, cmd, NULL, NULL, TRUE,
		CREATE_DEFAULT_ERROR_MODE | DETACHED_PROCESS  | NORMAL_PRIORITY_CLASS, NULL,
		NULL, &sinfo, &pinfo)){
		CloseHandle(h);
		return -1;
	}
	CloseHandle(pinfo.hThread);
	WaitForSingleObject(pinfo.hProcess, INFINITE);
	CloseHandle(pinfo.hProcess);
	CloseHandle(h);
	return 0;
}

#endif

void mrg_diff_msg (char *);
void mrg_diff_err (char *);

class diffInfo {
public:
    int token_start;
    int token_end;

    diffInfo ();
};

diffInfo::diffInfo ()
{
    token_start = -1; token_end = -1;
}

class diffLine {
public:
    diffLine () {  oper = 0 ; }
    diffLine ( const diffLine& );
    void cleanup ();

    char oper;
    diffInfo prev;
    diffInfo curr;
};

void diffLine::cleanup () 
{
    oper = 0;
    prev.token_start = -1;
    prev.token_end   = -1;
    curr.token_start = -1;
    curr.token_end   = -1;
}

diffLine::diffLine ( const diffLine& d )
{
  oper = d.oper;
  prev.token_start = d.prev.token_start;
  prev.token_end   = d.prev.token_end;
  curr.token_start = d.curr.token_start;
  curr.token_end   = d.curr.token_end;
}

class diffParser {
    genString diff_line;
    diffLine prev_cmd;
    diffLine curr_cmd;
    int write_offsets (FILE *fout);
    int combine_cmds ();
public:
    mrgFile *prev_file;
    mrgFile *curr_file;

    void cleanup();
    diffParser(mrgFile *pr, mrgFile *cr);
    void parse (const char *line);
    int flush_offsets (FILE *fout);
};

diffParser::diffParser ( mrgFile *pr, mrgFile *cr )
{
    diff_line = 0;
    prev_file = pr;
    curr_file = cr;
}

void diffParser::cleanup ()
{
    diff_line = 0;
}

void diffParser::parse (const char *line)
{
    diff_line = line;
    curr_cmd.oper = 0;

    char *txt = diff_line;
    char *tmp = txt;
    while (isdigit(tmp[0]))
	tmp ++;

    char ttt = tmp[0];
    tmp[0] = 0;

    int t = 0;
    OSapi_sscanf(txt,"%d",&t);
    curr_cmd.prev.token_start =  t - 1;

    if (ttt == ',') {
	txt = tmp + 1;
	tmp = txt;
	while (isdigit(tmp[0]))
	    tmp ++;

	ttt    = tmp[0];
	tmp[0] = 0;
	t = 0 ;
	OSapi_sscanf(txt,"%d",&t);
	curr_cmd.prev.token_end = t - 1;
	curr_cmd.oper = ttt;
    } else {
	curr_cmd.prev.token_end = curr_cmd.prev.token_start;
	curr_cmd.oper = ttt;
    }
    
    txt = tmp + 1;
    tmp = txt;
    
    while (isdigit(tmp[0]))
	tmp ++;

    ttt    = tmp[0];
    tmp[0] = 0;
    t = 0;
    OSapi_sscanf(txt,"%d",&t);
    curr_cmd.curr.token_start = t - 1 ;
    if (ttt == ',') {
	txt = tmp + 1;
	tmp = txt;
	while (isdigit(tmp[0]))
	    tmp ++;

	ttt    = tmp[0];
	tmp[0] = 0;
	t = 0;
	OSapi_sscanf(txt,"%d",&t);
	curr_cmd.curr.token_end = t - 1 ;
    } else
	curr_cmd.curr.token_end = curr_cmd.curr.token_start;
}

static int mrg_search ( const char *txt, int len, char c )
{
    int found = 0;
    for ( int ii = 0 ; ii < len ; ii ++ ) {
	if ( txt[ii] == c ) {
	    found = 1;
	    break;
	}
    }
    return found;
}

static char char_separator ( const char *txt, int len )
{
    char c = 0;
    for (char cur = 072 ; cur < 0200 ; cur ++) {
	int found = mrg_search ( txt, len, cur );
	if (found == 0) {
	    c = cur;
	    break;
	}
    }
    return c;
}

static char special_separator ( const char *txt, int len )
{
    static char *separators = "/%$!@#^&+-|~`'()";
    char *dels = separators;
    char c = 0;

    while (dels[0]) {
	int found = mrg_search (txt, len, dels[0]);
	if (found == 0) {
	    c = dels[0];
	    break;
	}
	dels ++ ;
    }
    return c;
}

static char mrg_separator (const char *txt, int len)
{
    char c = 0;
    c = special_separator (txt, len);
    if ( c == 0 )
	c = char_separator (txt, len);

    if (c == 0) {
	c = 001;
	mrg_diff_err ("mrg_diff WARNING: could not find separator. Use C-A");
    }
    return c;
}

int diffParser::write_offsets (FILE *fout) 
{
    int off    = prev_file->token_offset (prev_cmd.prev.token_start);
    int en_off = prev_file->token_offset (prev_cmd.prev.token_end);
    int en_len = prev_file->token_length (prev_cmd.prev.token_end);
    int len    = en_off - off + en_len;

    genString l;
    if (prev_cmd.oper == 'd') {
	l.printf ("%d,%d,//\n",off, len);
    } else {
	int c_off = curr_file->token_offset (prev_cmd.curr.token_start);
	en_off    = curr_file->token_offset (prev_cmd.curr.token_end);
	en_len    = curr_file->token_length (prev_cmd.curr.token_end);
	int c_len = en_off - c_off + en_len;

	char c    =  mrg_separator (curr_file->srcbuf + c_off, c_len);

	ocharstream os;
	int i = 0;
	for (char *bbb = curr_file->srcbuf + c_off ; i < c_len ; i ++) {
	    os << bbb[i];
	    if ( bbb[i] == '\n' ) {
		os << "> ";
	    }
	}

	os << ends;
	char *buf = os.ptr();
	if (prev_cmd.oper == 'a') {
	    off = off + len;
	    len = 0;
	}
	l.printf ("%d,%d,%c%s%c\n",off, len, c, buf, c);
    }

    int res   = l.length();
    char *tmp = (char *)l;
    
    fputs (tmp, fout);
    return res;
}

extern genString diff_cmd;
int mrg_parse_diff ( const char *diff_out,  const char *ed_out, mrgFile *fprev, mrgFile *fcurr )
{
#define MAX_LEN 128
    static char line[MAX_LEN];

    int status = -1;
    FILE *fin  = OSapi_fopen(diff_out,"r");
    FILE *fout = 0;
    if (ed_out && ed_out[0])
	fout = OSapi_fopen(ed_out,  "w");
    else
	fout = stdout;

    if (!fin || !fout)
	return status;

    diffParser prs (fprev, fcurr);

    while(OSapi_fgets(line,MAX_LEN,fin))
    {
        if (isdigit(line[0])) {
	    prs.parse(line);
	    prs.flush_offsets ( fout );
	    prs.cleanup();
        }
    }

    prs.flush_offsets (fout);

    OSapi_fclose(fin);
    if (ed_out && ed_out[0])
	OSapi_fclose(fout);
    status = 0;

    return status;
}

int mrg_diff (const char *prev_fn, const char *curr_fn, const char *out_fn)
{
    int status = -1;
    mrgFile *fprev = new mrgFile(prev_fn);
    mrgFile *fcurr = new mrgFile(curr_fn);

    genTmpfile *tprev = new genTmpfile("mrg");
    genTmpfile *tcurr = new genTmpfile("mrg");
    genTmpfile *tdiff = new genTmpfile("mrg");

    status = fprev->save_token_one_line (tprev->name());
    if (status != 0) {
	genString msg;
	msg.printf ("mrg_diff ERROR: failed to load file %s", prev_fn);
	mrg_diff_err ((char *)msg);
	return status;
    }

    status = fcurr->save_token_one_line (tcurr->name());
    if (status != 0) {
	genString msg;
	msg.printf ("mrg_diff ERROR: failed to load file %s", curr_fn);
	mrg_diff_err ((char *)msg);
	return status;
    }

#ifndef _WIN32
    status = vsysteml_redirect (tdiff->name(), (char *)diff_cmd, tprev->name(), tcurr->name(), (char *)0);
#else 
	genString cmd;
	cmd = (char *)diff_cmd;
	char output[_MAX_PATH];
	strcpy(output,tprev->name()+1);
	convert_forward_slashes(output);
	cmd += " ";
	cmd += output;
	strcpy(output,tcurr->name()+1);
	convert_forward_slashes(output);
	cmd += " ";
	cmd += output;
	
	strcpy(output,tdiff->name()+1);
	convert_forward_slashes(output);
	status = execute_diff((char*)cmd, output);
#endif

    status = mrg_parse_diff (tdiff->name(), out_fn, fprev, fcurr);

    char const *debug = OSapi_getenv ("DIS_MRG_DEBUG");
    if ( debug == 0 ) {
	delete tprev;
	delete tcurr;
	delete tdiff;
    } else {
	genString msg;
	msg.printf ("DEBUG: prev, curr, diff: %s %s %s", tprev->name(), tcurr->name(), tdiff->name());
	mrg_diff_msg ((char *)msg);
    }

    delete fprev;
    delete fcurr;
    
    return status;
}

int diffParser::flush_offsets (FILE *fout)
{
    if (prev_cmd.oper == 0 && curr_cmd.oper == 0)
	return 0;

    if (curr_cmd.oper == 0) {
	int res = write_offsets (fout);
	prev_cmd.cleanup ();
	return 0;
    }

    if (prev_cmd.oper == 0) {
	prev_cmd = curr_cmd;
	curr_cmd.cleanup();
	return 0;
    }

/*
    if (combine_cmds()) {
	curr_cmd.cleanup();
	return 0;
    }
*/
    int res = write_offsets (fout);
    prev_cmd = curr_cmd;
    curr_cmd.cleanup();
    return 0;
}

int diffParser::combine_cmds ()
{
    int res = 0;
    if (prev_cmd.oper == 'c') {
	int en_off = prev_file->token_offset (prev_cmd.prev.token_end);
	int en_len = prev_file->token_length (prev_cmd.prev.token_end);

	int end_prev_cmd_off   = en_off + en_len;
	int start_curr_cmd_off = prev_file->token_offset (curr_cmd.prev.token_start);
	char *buf = prev_file->srcbuf + end_prev_cmd_off;
	int len   = start_curr_cmd_off - end_prev_cmd_off;

	int combine   = 1;
	for (int ii = 0 ; ii < len ; ii ++ ) {
	    char c_ch = buf[ii];
	    if (!isalnum(c_ch))
		continue;

	    combine = 0;
	    break;
	}

	if (combine == 1) {
	    prev_cmd.prev.token_end = curr_cmd.prev.token_end;
	    prev_cmd.curr.token_end = curr_cmd.curr.token_end;
	    res = 1;
	}
    }
    return res;
}
