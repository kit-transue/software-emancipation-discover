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
// File db_io.C
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
#include <smt.h>
#include <proj.h>
#include <Question.h>

#include <fileCache.h>
#include <machdep.h>
#include <shell_calls.h>
#include <messages.h>
#include <driver_mode.h>

#include "db_save_str.h"
#include "db_str_desc.h"

#include "db_read_str.h"
#include "db_str_desc.h"

#define MAXPATHLEN 1024

void get_pset_fn (char const*, genString&);

#define F_OK            0       /* does file exist */
#define X_OK            1       /* is it executable by caller */
#define W_OK            2       /* is it writable by caller */
#define R_OK            4       /* is it readable by caller */

#define BACKUP_SUFF "~"
#define CORRUPT_SUFF ".BAD"

extern bool is_forgiving();

void init_pset_scramble_array();
void free_pset_scramble_allocated_space();
byte *pset_scramble(byte *p, int len, char allocated_flag, int not_scramble);

extern int xref_hook_turn_off;

int get_paraset_db_version ();

static bool db_format = 1;

static int db_incremental;

void db_set_incremental ()
{
    ++db_incremental;
}

void db_unset_incremental ()
{
   if(db_incremental > 0) --db_incremental;
}

void db_clear_incremental ()
{
    db_incremental = 0;
}

int db_get_incremental ()
{
    return db_incremental;
}

void db_name (char *name, char const *fn, char const *suff) 
{
    Initialize(db_name);

    strcpy (name, fn);
    if (suff)
	strcat(name, suff);
    else {
	genString gs;
	get_pset_fn (name, gs);
	if (gs.length ())
	    strcpy (name, gs);
	else
	    strcat (name, DB_SUFF);

    }
}

void db_name (char const * name, char const * fn, char const * suff) 
{
   db_name (name, (char const*) fn, suff) ;
}

bool db_change_to_writable(char const *fname)
{
    Initialize(db_change_to_writable);

    bool res = (shell_change_to_writable (fname) == 0);
    return res;
}

static bool db_save_open (char const * fname, int /* size */, int& unit, int mode)
{
    Initialize (db_save_open);

    int tryed_checkout = 0;
    int saved_errno = -1;
    while (1) {
	unit = OS_dependent::open (fname, O_RDWR|O_CREAT|O_TRUNC, mode);
	saved_errno = errno;

	if (unit >= 0 || tryed_checkout)
	    break;

        if (!db_change_to_writable(fname))
	    break;
	else
	    tryed_checkout = 1;
    }
	    
    if (unit < 0) {
	if (saved_errno != -1)
	    errno = saved_errno;
	if (!is_gui()) {
	    msg("Cannot create file $1", error_sev) << fname << eom;
	    perror(0);
	    return 0;
	} else {
	    genString txt;
	    msg("Cannot create file $1 : $2", error_sev) << fname << eoarg << OSapi_strerror(errno) << eom;
	    return 0;
	}
    }

    return 1;
}

//  Note, returns 1 - success, 0 - failure, -1 - cancel,
//  not just 0/1 as before.

static int db_save_open_src (char const * fname, int /* size */, int& unit, int mode)
{
    Initialize (db_save_open_src);

    char backup_fname [MAXPATHLEN];
    strcpy (backup_fname, fname);
    strcat (backup_fname, BACKUP_SUFF);

    if (OS_dependent::access (fname, F_OK) == 0) {
	struct OStype_stat buffer;
	if (OSapi_lstat (fname, &buffer)) {
	    msg ("Cannot access file $1", error_sev) << fname << eom;
	    perror (0);
	    return 0;
	}
	if (OS_dependent::access (fname, W_OK)) {
	    if (!is_gui()) {
	  	msg("File $1 is write protected", error_sev) << fname << eom;
		return 0;
	    }
	    int ret = dis_question (T_SAVE, B_YES, B_NO, Q_FILEWRITEPROTECTED, fname);
            if (ret != 1)
		return ret;
	}
	if (OSapi_rename(fname, backup_fname)) {
	    msg("Failed rename $1 to $2", error_sev) << fname << eoarg << backup_fname << eom;
	    return 0;
	}
    }

    if ((unit=OS_dependent::open (fname, O_RDWR|O_CREAT|O_TRUNC, mode)) < 0) {
        msg("Cannot open $1", error_sev) << fname << eom;
	perror (0);
        return 0;
    }

    return 1;
}

bool db_restore_open (char const *fname, int& size, int& unit, char *& addr);
bool db_close (int unit, char *addr);
void ver_err(long version);

static void delete_bufs (dbRootBufArr& roots)
{
    int num_roots = roots.size ();

    for (int i = 0; i < num_roots; ++i) {
	delete roots[i]->app_buf;
	delete roots[i]->int_buf;
	roots[i]->app_buf = 0;
	roots[i]->int_buf = 0;
    }
}

static int is_smt_or_ste_buf(dbRootBuf *rt)
{
    Initialize(is_smt_or_ste_buf);

    if (!rt->def->head || !rt->def->head->name)
        return 0;

    if (strcmp(rt->def->head->name, db_def_tables[0].def[0].head->name) == 0 ||
        strcmp(rt->def->head->name, db_def_tables[0].def[1].head->name) == 0 ||
        strcmp(rt->def->head->name, db_def_tables[0].def[2].head->name) == 0 ||
        strcmp(rt->def->head->name, db_def_tables[0].def[3].head->name) == 0)
    {
        return 1;
    }

    return 0;
}

static void db_calc_checksum (const byte * ptr, int len, unsigned& checksum)
{
    unsigned		g;
    char const *	p = (char const *) ptr;

    while (len-- > 0) {
	checksum = (checksum << 4) + *p++;
	if ((g = checksum & 0xf0000000) != 0)
	    checksum = (checksum ^ (g >> 24)) ^ g; 
    }
}

int db_write_bufs (dbRootBufArr& roots, char const *filename)
{
    Initialize (db_write_bufs);

    dbRel_format format = db_get_rel_format ();

    DBG {
	int sz = roots.size();
	msg("db_write_bufs: $1 bufs", normal_sev) << sz << eom;

	for(int ii=0; ii<sz; ++ii){
	    dbRootBuf * rp = roots[ii];
	    int asz = rp->app_buf->size();
	    int isz = rp->int_buf->size();
            db_intern_buffer* buf = rp->int_buf;
	    msg("F2 nodes  $1   $2", normal_sev) << buf->no_F2 << eoarg << buf->no_F2 * 2 << eom;
	    msg("F2a nodes $1   $2", normal_sev) << buf->no_F2a << eoarg << buf->no_F2a * 2 << eom;
	    msg("F3b nodes $1   $2", normal_sev) << buf->no_F3b << eoarg << buf->no_F3b * 3 << eom;
	    msg("F6 nodes  $1   $2", normal_sev) << buf->no_F6 << eoarg << buf->no_F6 * 6 << eom;
	    msg("relations $1   $2", normal_sev) << buf->no_rel << eoarg << buf->no_rel * (format == R4_22_8 ? 4 : 3) << eom;
	    msg("  isz $1 asz $2", normal_sev) << isz << eoarg << asz << eom;
	}
    }

    int num_roots = roots.size ();
    if (!num_roots)
	return 0;

    db_buffer header_buf;

    SAVEABLE_STRUCT_NAME(db_str_description) desc;
    SAVEABLE_STRUCT_NAME(db_str_desc_extension) ext;
    
    OSapi_bcopy("#REL", &desc.magic, sizeof(desc.magic));
    desc.version      = get_paraset_db_version();
    desc.date         = 0;
    desc.old_checksum = 0;
    desc.sparebyte    = 0;
    desc.need_reparse = 0;
    desc.not_scramble = 0;
    desc.bit2         = 0;		/* bit forgiving */
    desc.bit3         = 0;
    desc.bit4         = 0;
    desc.bit5         = 0;
    desc.bit6         = 0;
    desc.written      = 0;

    OS_dependent::bzero (&ext, sizeof(ext));

    app* header       = roots[0]->header;
    int src_modified  = header->is_src_modified ();
    int pset_modified = header->is_modified ();		// THIS MAY BE WRONG!

    char const *fname;
    if (!filename)
	filename = header->get_phys_name ();
  	
    if (filename)
	fname = filename;
    else
	fname = header->get_filename ();

    if (*fname == 0)
	fname = header->get_name ();

    if (xref_not_txt_file(fname))
        desc.not_scramble = 1;
    
    StructSaver desc_buf(&desc, STRUCT_SAVE_PARAMS(db_str_description));
    header_buf.put(desc_buf.get_buffer(), sizeof(desc));    // out
    StructSaver ext_buf(&ext, STRUCT_SAVE_PARAMS(db_str_desc_extension));
    header_buf.put(ext_buf.get_buffer(), sizeof (ext));     // out

    SAVEABLE_STRUCT_NAME(db_str_flags) flags;
    struct OStype_stat stat_buf;
    char resolved_path[MAXPATHLEN];
#ifndef _WIN32
    if (OSapi_lstat (fname, &stat_buf) == 0 && S_ISLNK (stat_buf.st_mode))
	OSapi_realpath (fname, resolved_path);
    else
	strcpy (resolved_path, fname);
#else
    strcpy (resolved_path, fname);
#endif

    flags.db_format = db_format;
//  if (!xref_not_txt_file(fname))
//      flags.db_format = 0;

    flags.db_rel_format = format;
    flags.reserv        = 0;
    StructSaver flags_buf(&flags, STRUCT_SAVE_PARAMS(db_str_flags));
    header_buf.put (flags_buf.get_buffer(), sizeof (flags));  // out

    header_buf.put_byte (header->descr ()->type_id);  // out
    header_buf.put_byte (header->language);           // out
    header_buf.put (header->get_filename ());         // out

    header_buf.put_int (num_roots);            // out
    SAVEABLE_STRUCT_NAME(db_str_Root)* dbr = new SAVEABLE_STRUCT_NAME(db_str_Root)[num_roots];
    int header_size                        = header_buf.size () + sizeof (SAVEABLE_STRUCT_NAME(db_str_Root)) * num_roots;
    int offset                             = header_size;
    int i;
    OSapi_bzero(dbr, sizeof(SAVEABLE_STRUCT_NAME(db_str_Root)) * num_roots);
    for (i = 0; i < num_roots; ++i) {
	int found   = 0;
	db_def* def = roots[i]->def;
	for (int j = 0; j < db_def_tables_size; ++j) {
	    int ind = def - db_def_tables[j].def;
	    if (0 <= ind && ind < db_def_tables[j].size) { 
		dbr[i].table = j;
		dbr[i].ind   = ind;
		found        = 1;
		break;
	    }
	}
	Assert (found);
	dbr[i].int_len = roots[i]->int_buf->size ();
	dbr[i].app_len = (def->suff == 0 || flags.db_format != 0) ?	
	    roots[i]->app_buf->size () : 0;
	dbr[i].root_id = roots[i]->root_id;
	dbr[i].last_id = roots[i]->last_id;
	offset        += (dbr[i].int_len + dbr[i].app_len);
    }
		
    // Convert dbr array to universal format
//    char tmp_dbr_buf[sizeof(SAVEABLE_STRUCT_NAME(db_str_Root)) + 1];
    SAVEABLE_STRUCT_NAME(db_str_Root)* dbr_save = new SAVEABLE_STRUCT_NAME(db_str_Root)[num_roots];
    for(i = 0; i < num_roots; i++){
	StructSaver dbr_saver((void *)&dbr[i], STRUCT_SAVE_PARAMS(db_str_Root), (char *)&dbr_save[i]);
//	OSapi_bcopy(tmp_dbr_buf.get_buffer(), &dbr_save[i], sizeof(SAVEABLE_STRUCT_NAME(db_str_Root)));
    }
    header_buf.put (dbr_save, sizeof(SAVEABLE_STRUCT_NAME(db_str_Root)) * num_roots);
    delete [] dbr_save;
	
    // open and map files
    char out_name[MAXPATHLEN];
    int unit;
    byte* pointer;
    mode_t mode = 0644;

    for (i = 0; i < num_roots; ++i) {
	db_def* def = roots[i]->def;
	if (def->suff) {
	    db_name (out_name, resolved_path, def->suff);
	    if (OS_dependent::access (out_name, F_OK) == 0) {
		struct OStype_stat buffer;
		if (!OSapi_lstat (fname, &buffer))
#ifdef _WIN32
		    mode = buffer.st_mode | _S_IWRITE;
#else
		    mode = buffer.st_mode | S_IWRITE;
#endif
	    }
	}
    }

    int to_write_src = 1;
    for (i = 0; i < num_roots; ++i) {
	db_def* def = roots[i]->def;
	if (def->suff) {
	    if (def->dont_save_unchanged_src
		&& src_modified == 0
		&& OS_dependent::access (out_name, F_OK) == 0)
		to_write_src = 0;
	    if (to_write_src) {
		int app_size = roots[i]->app_buf->size ();
		int retval = db_save_open_src (out_name, app_size, unit, mode);
                if (retval != 1){
		    delete [] dbr;
		    delete_bufs (roots);
		    return retval;
		}
		pointer = roots[i]->app_buf->operator [] (0);
		if (app_size)
		    retval = (app_size == OSapi_write (unit, (char const *)pointer, app_size));
		OSapi_close (unit);
                if (retval != 1){
		    delete [] dbr;
		    delete_bufs (roots);
		    return retval;
		}
	    }
	}
    }

    db_name (out_name, fname, 0);
    int err = projHeader::make_path (out_name);
    if (err) {
	msg("db_write_bufs() could not create the directory.", error_sev) << eom;
	return 0;
    }

    if (!db_save_open (out_name, offset, unit, mode)) {
	delete [] dbr;
	delete_bufs (roots);
	return 0;
    }

    if (!pset_modified) {
        SAVEABLE_STRUCT_NAME(db_str_description) old_desc;
        SAVEABLE_STRUCT_NAME(db_str_description) tmp;
	OSapi_read(unit, &tmp, sizeof(old_desc));
	StructReader::Read(&tmp, &old_desc, STRUCT_READ_FUNCTION(db_str_description));
        OSapi_lseek (unit, 0, 0);
	pset_modified = old_desc.written;
    }

    void *tmp_p = header_buf[0];
    SAVEABLE_STRUCT_NAME(db_str_description) tmp_desc;
    StructReader::Read(tmp_p, &tmp_desc, STRUCT_READ_FUNCTION(db_str_description));
    tmp_desc.written = pset_modified;
    StructSaver tmp_desc_buf_save(&tmp_desc, STRUCT_SAVE_PARAMS(db_str_description));
    OSapi_bcopy(tmp_desc_buf_save.get_buffer(), tmp_p, sizeof(tmp_desc));

    unsigned checksum  = 0;
    int      desc_size = sizeof (db_str_description) + sizeof (db_str_desc_extension);

    db_calc_checksum ((byte*) (header_buf[0] + desc_size),
				header_size - desc_size, checksum);

    OSapi_write (unit, (char const *)header_buf[0], header_size);

    offset = header_size;

    for (i = 0; i < num_roots; ++i) {
        pointer = roots[i]->int_buf->operator [] (0);
	if (dbr[i].int_len) {
	    db_calc_checksum (pointer, dbr[i].int_len, checksum);
	    OSapi_write (unit, (char const *)pointer, dbr[i].int_len);
	}
	offset += dbr[i].int_len;
//	OSapi_printf("ROOT %d after INT save : %x\n", i, offset);
	pointer = roots[i]->app_buf->operator [] (0);
	if (dbr[i].app_len)
        {
            byte *p;
            if (i == 0 && is_smt_or_ste_buf(roots[0]))
                p = pset_scramble(pointer, dbr[i].app_len, 0, desc.not_scramble);
            else
                p = (byte *)pointer;

	    db_calc_checksum (p, dbr[i].app_len, checksum);
	    OSapi_write (unit, (char const *)p, dbr[i].app_len);
        }
	offset += dbr[i].app_len;
//	OSapi_printf("ROOT %d after APP save : %x\n", i, offset);
    }
    ext.new_checksum = checksum;
    OSapi_lseek (unit, sizeof(db_str_description), 0);
    StructSaver ext_buf_to_write(&ext, STRUCT_SAVE_PARAMS(db_str_desc_extension));
    OSapi_write (unit, ext_buf_to_write.get_buffer(), sizeof(db_str_desc_extension));
    OSapi_close (unit);

    delete [] dbr;
    delete_bufs (roots);

    int ans = 1;
    if (to_write_src == 0)    // "untouched source" case
	ans = 2;

    global_cache.invalidate(out_name);
    return ans;
}

static int db_restore_version;

int db_get_restore_version()
{
  return db_restore_version;
}
    
app* db_read_bufs (char const *fname, char const * lname, app* h) 
{
    Initialize (db_read_bufs);

    char in_name[MAXPATHLEN];
    char bad_fname [MAXPATHLEN];
    db_name (in_name, fname, 0);
    int unit;
    char* addr;
    int size;

    if (!db_restore_open (in_name, size, unit, addr))
	return NULL;
    
    db_buffer_ro header_buf (addr, size);
    
    SAVEABLE_STRUCT_NAME(db_str_description) desc;
    SAVEABLE_STRUCT_NAME(db_str_description) desc_tmp_buf;
    SAVEABLE_STRUCT_NAME(db_str_desc_extension) ext;

    header_buf.get(&desc_tmp_buf, sizeof(desc));
    StructReader::Read(&desc_tmp_buf, &desc, STRUCT_READ_FUNCTION(db_str_description));

    char const *rel = "#REL";
    if (strncmp((char*)&desc.magic, rel, 4) != 0) {
	strcpy (bad_fname, in_name);
	strcat (bad_fname, CORRUPT_SUFF);
	msg("Format error in .pset file", catastrophe_sev) << eom;
	msg("Incorrect format in $1; loading raw view.", error_sev) << in_name << eom;
	if (OSapi_rename (in_name, bad_fname) != 0)
	    msg("Cannot rename $1 to $2", error_sev) << in_name << eoarg << bad_fname << eom;
	else msg("File renamed to $1.", normal_sev) << bad_fname << eom;
	db_close (unit, addr);
	return NULL;
    }
    
    IF (desc.version == 0) {
	db_close (unit, addr);
	return 0;
    }
    
    if (desc.version > get_paraset_db_version())
    {
        ver_err(desc.version);
	return NULL;
    }

    db_restore_version = desc.version;

    int offset;

    if (desc.version >= 3) {
	SAVEABLE_STRUCT_NAME(db_str_desc_extension) ext_tmp_buf;
	header_buf.get(&ext_tmp_buf, sizeof(ext));
	StructReader::Read(&ext_tmp_buf, &ext, STRUCT_READ_FUNCTION(db_str_desc_extension));

	offset            = header_buf.get_position ();
	unsigned checksum = 0;
	db_calc_checksum ((byte *) (addr + offset),
			  header_buf.size() - offset, checksum);
	if (checksum != ext.new_checksum) {
	    strcpy (bad_fname, in_name);
	    strcat (bad_fname, CORRUPT_SUFF);
	    msg("Checksum error in .pset file", catastrophe_sev) << eom;
	    msg("Checksum error in $1; loading raw view.", error_sev) << in_name << eom;
	    if (OSapi_rename (in_name, bad_fname) != 0)
		msg("Cannot rename $1 to $2", error_sev) << in_name << eoarg << bad_fname << eom;
	    else msg("File renamed to $1.", normal_sev) << bad_fname << eom;
	    db_close (unit, addr);
	    return NULL;
	}
    }

    SAVEABLE_STRUCT_NAME(db_str_flags) flags;
    SAVEABLE_STRUCT_NAME(db_str_flags) flags_tmp_buf;
    header_buf.get (&flags_tmp_buf, sizeof (flags));
    StructReader::Read(&flags_tmp_buf, &flags, STRUCT_READ_FUNCTION(db_str_flags));
    dbRel_format format = dbRel_format (flags.db_rel_format);

    byte type_id;
    header_buf.get_byte (type_id);
    byte lang;
    header_buf.get_byte (lang);
    char* name;
    header_buf.get (name);
    if (lname && strcmp (name, lname)) {
        genString bad_pname;
        bad_pname.printf("%s.BAK",in_name);
        OSapi_rename(in_name,bad_pname.str());
	msg("Logical name for module $1 doesn\'t match with name $2 stored in .pset file; it is renamed to $3", catastrophe_sev) << lname << eoarg << name << eoarg << bad_pname.str() << eoarg << eom;
	db_close (unit, addr);
	return 0;
    }

    int num_roots;
    header_buf.get_int (num_roots);
    
    SAVEABLE_STRUCT_NAME(db_str_Root)* dbr     = new SAVEABLE_STRUCT_NAME(db_str_Root)[num_roots];
    SAVEABLE_STRUCT_NAME(db_str_Root)* dbr_tmp = new SAVEABLE_STRUCT_NAME(db_str_Root)[num_roots];
    header_buf.get(dbr_tmp, sizeof (SAVEABLE_STRUCT_NAME(db_str_Root)) * num_roots);
    int i;
    for(i = 0; i < num_roots; i++)
	StructReader::Read(&dbr_tmp[i], &dbr[i], STRUCT_READ_FUNCTION(db_str_Root));
    delete [] dbr_tmp;

    offset         = header_buf.get_position ();
    int app_unit   = -1;
    int app_size   = 0;
    char* app_addr = NULL;

    dbRootBufArr roots;
    init_pset_scramble_array();
    for (i = 0; i < num_roots; ++i)
    {
        dbRootBuf* rt = roots.grow (1);
        rt->def       = db_def_tables[dbr[i].table].def + dbr[i].ind;
        rt->int_buf   = (db_intern_buffer*) new db_buffer_ro (addr + offset, dbr[i].int_len);
        offset       += dbr[i].int_len;
        if (!rt->def->suff || flags.db_format != 0 || (!h && (rt->def->incremental && db_get_incremental())))
        {
            if (rt->def->suff)
            {
                // check do we have readable .C file even if
                // we don't need it to restore DD part of smtHeader
                db_name (in_name, fname, rt->def->suff);
                if (!flags.db_format && OS_dependent::access(in_name, R_OK))
                {
                    msg("Cannot open for read $1", error_sev) << fname << eom;
                    perror (0);
                    delete (db_buffer_ro*)roots[i]->int_buf;
                    for (int j = 0; j < i; ++j)
                    {
                        delete (db_buffer_ro*)roots[j]->app_buf;
                        delete (db_buffer_ro*)roots[j]->int_buf;
                    }
                    db_close (unit, addr);
                    free_pset_scramble_allocated_space();
                    return NULL;
                }
            }
            byte *p;
            if (flags.db_format && i == 0 && is_smt_or_ste_buf(rt))
                p = pset_scramble((byte *)(addr + offset), dbr[i].app_len, 1, desc.not_scramble);
            else
                p = (byte *)(addr + offset);
	    
            rt->app_buf = (db_buffer*) new db_buffer_ro (p, dbr[i].app_len);
        }
        else
        {
            db_name (in_name, fname, rt->def->suff);
            if (!db_restore_open (in_name, app_size, app_unit, app_addr))
            {
                delete (db_buffer_ro*)roots[i]->int_buf;
                for (int j = 0; j < i; ++j)
                {
                    delete (db_buffer_ro*)roots[j]->app_buf;
                    delete (db_buffer_ro*)roots[j]->int_buf;
                }
                db_close (unit, addr);
                delete [] dbr;
                return NULL;
            }
            rt->app_buf = (db_buffer*) new db_buffer_ro (app_addr, app_size);
        }
        offset     += dbr[i].app_len;
        rt->root    = 0;
        rt->root_id = dbr[i].root_id;
        rt->last_id = dbr[i].last_id;
    }

// Prevent double-update to xref

    int saved_flag     = xref_hook_turn_off;
    xref_hook_turn_off = 1;
    genError* err;
    genTry {
       start_transaction() {
	  if (h) {
	      for (i = 0; i < num_roots; ++i)
		  roots[i]->header = h;
	      db_restore_roots_complete (h, roots, format);
	  } else {
	      h = (*roots[0]->def->hd_restore)(name, lang, fname);
	      for (i = 0; i < num_roots; ++i)
		  roots[i]->header = h;
	      db_restore_roots (h, roots, format);
	  }
	  db_after_rest_head rst_hd = roots[0]->def->after_rest_head;
	  if (rst_hd) {
	     (h->*rst_hd)();
	  }

	  if (is_smtHeader (h)) {
	      bool f = (desc.need_reparse == 0 || desc.bit2);
	      ((smtHeader*)h)->set_actual_mapping(f);
	      ((smtHeader*)h)->set_arn(desc.need_reparse);
	  }
       } end_transaction();
    }
    genCatch(err) { }
    genEndtry;
    xref_hook_turn_off = saved_flag;
    for (i = 0; i < num_roots; ++i) {
	delete (db_buffer_ro*)roots[i]->app_buf;
	delete (db_buffer_ro*)roots[i]->int_buf;
    }
    db_close (unit, addr);
    if (app_unit != -1)
	db_close (app_unit, app_addr);
    delete [] dbr;
    free_pset_scramble_allocated_space();

    if (!flags.db_format && h && is_smtHeader(h))
    {
        smtHeader *smt_h = checked_cast(smtHeader,h);
        smt_h->set_modified();
    }

    return h;
}

// if src_name == 0, only return the length
// if src_name != 0, src_name is a temporary file name, which contains the contents
int db_read_src_from_pset (char const *pset_fname, char const *src_name);

int db_get_db_format(char const *fname)
{
    Initialize(db_get_db_format);

    char in_name[MAXPATHLEN];
    db_name (in_name, fname, 0);
    return
      db_get_pset_format(in_name);
}

int db_get_pset_format(char const *in_name)
{
    SAVEABLE_STRUCT_NAME(db_str_desc_extension) ext;
    SAVEABLE_STRUCT_NAME(db_str_description)    desc;
    SAVEABLE_STRUCT_NAME(db_str_flags)          flags;
    SAVEABLE_STRUCT_NAME(db_str_description)    desc_tmp_buf;
    SAVEABLE_STRUCT_NAME(db_str_flags)          flags_tmp_buf;

    int fd = OS_dependent::open((char const *)in_name, O_RDONLY);
    if (fd < 0)
        return 0;
    OSapi_read(fd, &desc_tmp_buf, sizeof(desc));
    StructReader::Read(&desc_tmp_buf, &desc, STRUCT_READ_FUNCTION(db_str_description));

    if (desc.version >= 3){
	char desc_ext_tmp_buf[sizeof(ext) + 1];
	OSapi_read(fd, desc_ext_tmp_buf, sizeof(db_str_desc_extension));
    }
    
    OSapi_read(fd, &flags_tmp_buf, sizeof(flags));
    StructReader::Read(&flags_tmp_buf, &flags, STRUCT_READ_FUNCTION(db_str_flags));
    OSapi_close(fd);

    return flags.db_format;
}


static int get_pset_bits(char const *in_name, SAVEABLE_STRUCT_NAME(db_str_description) &desc) 
{   
// cout << "get_pset_bits " << in_name << endl;
    int fd = OS_dependent::open((char const *)in_name, O_RDONLY);
    if (fd < 0)
	return 1;
    SAVEABLE_STRUCT_NAME(db_str_description) desc_tmp_buf;
    OSapi_read(fd, &desc_tmp_buf, sizeof(SAVEABLE_STRUCT_NAME(db_str_description)));
    StructReader::Read(&desc_tmp_buf, &desc, STRUCT_READ_FUNCTION(db_str_description));
    OSapi_close(fd);
    return 0;
}

static int db_get_pset_header_bits(char const *fname, SAVEABLE_STRUCT_NAME(db_str_description) &desc)
  /* return 1 if fail, 0 if suceed 
   */
{
    Initialize(db_get_pset_header_bits);
    
    char    in_name[MAXPATHLEN];
    db_name (in_name, fname, 0);
    return get_pset_bits(in_name, desc);
}

int db_get_bit_need_reparse_no_forgive(char const *fname)
{
    Initialize(db_get_bit_need_reparse_no_forgive);
    
    SAVEABLE_STRUCT_NAME(db_str_description) desc;    
    if (db_get_pset_header_bits(fname, desc))
	return 1; /* consistent with old function */
    int retval = desc.need_reparse != 0;
    return retval;
}

int db_get_pset_need_reparse_no_forgive(char const *fname)
{
    Initialize(db_get_bit_need_reparse_no_forgive);
    
    SAVEABLE_STRUCT_NAME(db_str_description) desc;    
    if (get_pset_bits(fname, desc))
	return 1; /* consistent with old function */
    int retval = desc.need_reparse != 0;
    return retval;
}

int db_get_bit_forgive(char const *fname)
  /* files saved with errors */
{
    Initialize(db_get_bit_need_reparse_no_forgive);
    
    SAVEABLE_STRUCT_NAME(db_str_description) desc;    
    if (db_get_pset_header_bits(fname, desc))
	return 0; /* consistent with old function */
    return desc.bit2;
}

int db_get_pset_forgive(char const *fname)
  /* files saved with errors */
{
    SAVEABLE_STRUCT_NAME(db_str_description) desc;    
    if (get_pset_bits(fname, desc))
	return 0; /* consistent with old function */
    return desc.bit2;
}


int db_get_bit_need_reparse(char const *fname)
{
    Initialize(db_get_bit_need_reparse);

    SAVEABLE_STRUCT_NAME(db_str_description) desc;
    if (db_get_pset_header_bits(fname, desc))
	return 1; /* consistent with old function */
    int retval = 0;
    if (!is_forgiving()) 
	retval = desc.need_reparse || desc.bit2;
    else
	retval = desc.need_reparse != 0;
    return retval;
}


static void update_pset_file_header(char const *in_name, SAVEABLE_STRUCT_NAME(db_str_description) *desc)
{
    int fd = OS_dependent::open(in_name, O_WRONLY);
    StructSaver desc_buf(desc, STRUCT_SAVE_PARAMS(db_str_description));
    OSapi_write(fd, desc_buf.get_buffer(), sizeof(SAVEABLE_STRUCT_NAME(db_str_description)));
    OSapi_close(fd);
}

void db_set_pset_need_reparse (char const *in_name, int need_reparse_bit)
{
    Initialize(db_set_bit_need_reparse);
    
    SAVEABLE_STRUCT_NAME(db_str_description) desc;
    SAVEABLE_STRUCT_NAME(db_str_description) desc_tmp_buf;
    int fd = OS_dependent::open((char const *)in_name, O_RDONLY);
    if (fd < 0)
        return;
    OSapi_read(fd, &desc_tmp_buf, sizeof(desc));
    StructReader::Read(&desc_tmp_buf, &desc, STRUCT_READ_FUNCTION(db_str_description));
    OSapi_close(fd);

    desc.need_reparse = (need_reparse_bit & 1) != 0;
    desc.bit2         = (need_reparse_bit & 2) != 0;        /* forgiving_bit */

    if (OS_dependent::access(in_name, W_OK)){
        // no write permission
        struct OStype_stat buf;
        if (global_cache.stat(in_name, &buf) == 0){
#ifdef _WIN32
            OSapi_chmod(in_name, buf.st_mode|_S_IWRITE);
#else
            OSapi_chmod(in_name, buf.st_mode|S_IWRITE);
#endif
            update_pset_file_header(in_name, &desc);
            OSapi_chmod(in_name, buf.st_mode);
            update_pset_file_header(in_name, &desc);
        }
    }
    else
        update_pset_file_header(in_name, &desc);
}

void db_set_bit_need_reparse (char const *fname, int reparse_bit)
{
  char in_name[MAXPATHLEN];
  db_name (in_name, fname, 0);
  db_set_pset_need_reparse(in_name, reparse_bit);
}

void db_set_pset_written (char const *in_name, int written)
{
    Initialize(db_set_pset_written);
    
    SAVEABLE_STRUCT_NAME(db_str_description) desc;
    SAVEABLE_STRUCT_NAME(db_str_description) desc_tmp_buf;
    int fd = OS_dependent::open((char const *)in_name, O_RDONLY);
    if (fd < 0)
        return;
    OSapi_read(fd, &desc_tmp_buf, sizeof(desc));
    StructReader::Read(&desc_tmp_buf, &desc, STRUCT_READ_FUNCTION(db_str_description));
    OSapi_close(fd);
    
    desc.written = (written & 1) != 0;
    if (OS_dependent::access(in_name, W_OK)){
        // no write permission
        struct OStype_stat buf;
        if (global_cache.stat(in_name, &buf) == 0){
#ifdef _WIN32
            OSapi_chmod(in_name, buf.st_mode | _S_IWRITE);
#else
            OSapi_chmod(in_name, buf.st_mode | S_IWRITE);
#endif
            update_pset_file_header(in_name, &desc);
            OSapi_chmod(in_name, buf.st_mode);
            update_pset_file_header(in_name, &desc);
        }
    }
    else
        update_pset_file_header((char const *)in_name, &desc);
}
