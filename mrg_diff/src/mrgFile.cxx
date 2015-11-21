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
#include <stdlib.h>

#include <mrgFile.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
#include <genString.h>

#ifdef _WIN32
#include <io.h>
#endif

mrgToken::mrgToken ()
{ 
    off   = -1;
}

mrgFile::mrgFile(const char *fn)
{
    fname    = fn;
    srcbuf   = 0;
    src_size = 0;
}

int mrgFile::load ()
{
    if (src_size)
	return src_size;

    struct OStype_stat statbuf;
    int file, flth;

    if(OSapi_stat(fname, &statbuf) != 0) // Read file parameters
	return -1;
    flth = (int)statbuf.st_size;
    srcbuf = (char *)malloc(flth + 1); //Allocate buffer
    if(srcbuf == 0)
	return -1;

    src_size = flth;
    srcbuf[flth] = '\0';
    file = OSapi_open((char *)fname, O_RDONLY);

#ifdef _WIN32
    if(OSapi_getenv("DIS_NLUNIXSTYLE")) {
      _setmode(file, _O_TEXT );
    }
#endif	
    int n;
    if(file >= 0 && (n=OSapi_read(file, srcbuf, flth)) >= 0) {
      srcbuf[n] = '\0';
    } else {
      if(file >= 0) OSapi_close(file);
      free(srcbuf);
      srcbuf = 0;
      src_size = 0;
      return -1;
    }
    OSapi_close(file);
    return src_size;
}

void hide_newlines (char *buf)
{
    while (buf && buf[0]) {
	if (buf[0] == '\n' || buf[0] == '\r')
	    buf[0] = 'A';

	buf ++;
    }
}

int mrgFile::save_token_one_line (const char *out_fname)
{
    int res = -1;
    if (!out_fname || !out_fname[0])
	return res;

    FILE *fout = OSapi_fopen ((char *)out_fname, "w");
    if (!fout)
	return res;

    load ();
    if (srcbuf == 0)
	return res;
    
    tokenize ();

    for (int i = 0 ; i < _arr.size() ; i ++) {
	genString txt;
	int off = token_offset (i);
	int len = token_length (i);
	txt.put_value (srcbuf + off, len + 1);
	hide_newlines ((char *)txt);
	char *tmp = txt;
	tmp[len]  = '\n';
	OSapi_fputs (tmp, fout);
    }

    OSapi_fclose (fout);
    res = 0;
    return res;
}

int dis_lex_tokenize (const char *fname, mrgFile *cur);

int mrgFile::tokenize ()
{
    int res = -1;
    load ();
    if (srcbuf == 0)
	return res;

    res = dis_lex_tokenize (fname, this);
    return res;
}

int mrgFile::token_offset (int i)
{
    int off = 0;
    if (i >= 0 && i < _arr.size()) {
	mrgToken *t = _arr[i];
	off         = t->off;
    }
    return off;
}

int mrgFile::token_length (int i)
{
    int len = 0;
    if ( i < 0 || i >= _arr.size())
	return len;

    mrgToken *t = _arr[i];
    if ( (i+1) >= _arr.size() )
	len = src_size - t->off;
    else {
	mrgToken *nxt = _arr[(i+1)];
	len = nxt->off - t->off;
    }
    return len;
}

int mrgFile::append (int off)
{
    mrgToken *t = _arr.grow(1);
    t->off = off;
    int sz = _arr.size();
    return sz;
}
