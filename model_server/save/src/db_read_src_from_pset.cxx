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
#include <cLibraryFunctions.h>
#include <msg.h>
#include <psetmem.h>
#include <transaction.h>
#include <fcntl.h>
#ifndef ISO_CPP_HEADERS
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
using namespace std;
#include <cstring>
#include <cstdio>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <genError.h>
#include <objOper.h>
#include <db_intern.h>
#include "db_read_str.h"
#include "db_str_desc.h"

static int paraset_db_version = 6;

int get_paraset_db_version()
{
  return  paraset_db_version;
}


bool db_restore_open (char const *fname, int& size, int& unit, char*& addr)
{
    Initialize (db_restore_open);

    if ((unit = OS_dependent::open(fname, O_RDONLY)) < 0) {
        msg("Cannot open $1") << fname << eom;
	perror (0);
        return 0;
    }
    size = (int) OSapi_lseek (unit, 0, 2);
    addr = new char [size];
    OSapi_lseek (unit, 0, 0);
    OSapi_read (unit, addr, size);
    return 1;
}

void ver_err(long version)
{
    Initialize(ver_err);

    msg("Loading raw view only. Rev '$1' .pset files are incompatible with this release of DISCOVER.", error_sev) << version << eom;
}

bool db_close (int unit, char *addr)
{
    delete [] addr;
    OSapi_close (unit);
    return 1;
}

// PSET_SCRAMBLE beginns
typedef struct
{
    byte *ptr;
} pset_scramble_alloc;

genArr(pset_scramble_alloc);

static genArrOf(pset_scramble_alloc) pset_acramble_array;

void init_pset_scramble_array()
{
    Initialize(init_pset_scramble_array);

    pset_acramble_array.reset();
}

void free_pset_scramble_allocated_space()
{
    Initialize(free_pset_scramble_allocated_space);

    for (int i=0;i<pset_acramble_array.size();i++)
    {
        pset_scramble_alloc *s = pset_acramble_array[i];
        psetfree(s->ptr);
    }
}

byte *pset_scramble(byte *p, int len, char allocated_flag, int not_scramble)
{
    Initialize(pset_scramble);

    if (not_scramble)
        return p;

#define SCARMBLE_BYTE 0x80

    if (len == 0)
        return 0;

    byte *buf;
    if (allocated_flag)
    {
        buf = (byte *)calloc(sizeof(byte),len);
        pset_scramble_alloc *s = pset_acramble_array.grow();
        s->ptr = buf;
    }
    else
        buf = p;

    byte *q = buf;
    for(int i=0;i<len;i++)
        *q++ = *p++ ^ SCARMBLE_BYTE;

    return buf;
}
// PSET_SCRAMBLE ends

static int src_from_pset(char const *pset_fname, char const *src_name, genString*res)
{
    Initialize(src_from_pset);

    int unit;
    char* addr;
    int size;
    int file_len = -2;

    if (!db_restore_open (pset_fname, size, unit, addr))
        return -1;

    db_buffer_ro header_buf (addr, size);

    SAVEABLE_STRUCT_NAME(db_str_description) desc;
    SAVEABLE_STRUCT_NAME(db_str_description) desc_tmp_buf;
    SAVEABLE_STRUCT_NAME(db_str_desc_extension) ext;

    header_buf.get(&desc_tmp_buf, sizeof(desc));
    StructReader::Read(&desc_tmp_buf, &desc, STRUCT_READ_FUNCTION(db_str_description));

    char const *rel = "#REL";
    IF (strncmp((char*)&desc.magic, rel, 4) != 0)
	return -1;

    if (desc.version > get_paraset_db_version())
    {
        ver_err(desc.version);
        db_close (unit, addr);
        return -1;
    }

    if (desc.version >= 3){
	char  desc_ext_tmp_buf[sizeof(ext) + 1];
	header_buf.get(desc_ext_tmp_buf, sizeof(ext));  // ext is not used below
    }

    SAVEABLE_STRUCT_NAME(db_str_flags) flags;
    SAVEABLE_STRUCT_NAME(db_str_flags) flags_tmp_buf;
    header_buf.get (&flags_tmp_buf, sizeof (flags));
    StructReader::Read(&flags_tmp_buf, &flags, STRUCT_READ_FUNCTION(db_str_flags));
    dbRel_format format = dbRel_format (flags.db_rel_format);

    if (!flags.db_format){
        db_close (unit, addr);
        return -2;
    }

    byte type_id;
    header_buf.get_byte (type_id);
    byte lang;
    header_buf.get_byte (lang);
    char* name;
    header_buf.get (name);

    int num_roots;
    header_buf.get_int (num_roots);

    SAVEABLE_STRUCT_NAME(db_str_Root)* dbr     = new SAVEABLE_STRUCT_NAME(db_str_Root)[num_roots];
    SAVEABLE_STRUCT_NAME(db_str_Root)* dbr_tmp = new SAVEABLE_STRUCT_NAME(db_str_Root)[num_roots];
    header_buf.get(dbr_tmp, sizeof (SAVEABLE_STRUCT_NAME(db_str_Root)) * num_roots);
    StructReader::Read(&dbr_tmp[0], &dbr[0], STRUCT_READ_FUNCTION(db_str_Root));
    delete [] dbr_tmp;

    int offset = header_buf.get_position ();

    int f_in = 0;
    if (src_name)
        f_in = OSapi_open(src_name, O_WRONLY|O_CREAT, 0600);

    init_pset_scramble_array();
    offset       += dbr[0].int_len;
    if (!dbr[0].table && !dbr[0].ind)
      {
	file_len = dbr[0].app_len;
	if (f_in || res)
	  {
	    byte *p = pset_scramble((byte *)(addr + offset), dbr[0].app_len, 1, desc.not_scramble);
	    // convert a.c.pset to a.c
	    if(f_in)
	      OSapi_write(f_in, (char *)p, dbr[0].app_len);
	    if(res)
	      res->put_value((char *)p, dbr[0].app_len);
	  }
      }

    free_pset_scramble_allocated_space();
    if (f_in)
        OSapi_close(f_in);

    db_close (unit, addr);
    delete [] dbr;
    return file_len;
}

// if src_name == 0, only return the length
// if src_name != 0, src_name is a temporary file name, which contains the contents
int db_read_src_from_pset (char const *pset_fname, char const *src_name)
{
  return  src_from_pset(pset_fname,src_name, NULL);
}

int db_read_src_from_pset (char const *pset_fname, genString&res)
{
  return  src_from_pset((char*)pset_fname, NULL, &res);
}

