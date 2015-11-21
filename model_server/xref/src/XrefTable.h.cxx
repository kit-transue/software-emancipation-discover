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
#ifndef _WIN32
#include <sys/mman.h>
#else
#include <windows.h>
#endif
#include <pset_mman.h>
#include <genTmpfile.h>
#include <genWild.h>

#include <genError.h>

#include <indHeaderInfo.h>
#include <xrefPair.h>
#include <Link.h>
#include <_Xref.h>
#include <SharedXref.h>
#include <xrefSymbol.h>
#include <XrefTable.h>
#include <linkTypes.h>
#include <proj.h>

#include <fileCache.h>
#include <ParaCancel.h>

#include <RTL_externs.h>
#include <systemMessages.h>
#include <Question.h>
#include <shell_calls.h>
#include <machdep.h>
#include <symbolSet.h>
#include <messages.h>
#include <Question.h>
#include <driver_mode.h>

#include "gen_hash.h"
#ifndef ISO_CPP_HEADERS
#include "errno.h"
#else /* ISO_CPP_HEADERS */
#include <cerrno>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#define  SEEK_END    2
#define SMALL 1
#define TABLE_MAX 6113

#define mprotect pset_mprotect

const int MSYNC_FLAGS = 0;

#ifdef  EXCLUSIVE_OPEN
static bool  abort_flag=0;
#endif

init_relational(XrefTable,Relational);

static const int oneK = 1024;
static const int XREF_LIMIT_SIZE = oneK * oneK * 16;
static  int	_SHOW_MOVE=-1;
static int pset_copy_rdonly_pmods;
static int default_make_room_size = 0;
//
// when 0, this flag signals that there has been crash or interrupt (precisely,
// anything caught by aset_trap_handler) in -batch mode and that permanent
// pmod should not be overwritten.
//
extern int ok_to_delete_old_pmod;
extern char const *PSET_PRINT_LINK;  /* in xrefSymbol.h.C * NY */
extern int call_from_put();
char const *xref_c_proto(const symbolPtr &src);

//  this function gets "called" when you want to die...
extern "C" void driver_exit(int exit_status);
static void error_truncating_file(char const *name, int term_number=92);
RelType* get_rel_type(linkType lt);
static int build_xref_flag = 1;
//int get_build_xref_flag() { return build_xref_flag; }
// called by projectHeader constructor which is obsolete code
int set_build_xref_flag(int i) 
{ build_xref_flag = i; return i;}

#ifdef _WIN32

static int valid_handle(HANDLE h)
{
    return h != INVALID_HANDLE_VALUE;
}

static HANDLE close_handle(HANDLE h)
{
    if (valid_handle(h))
	CloseHandle(h);
    return INVALID_HANDLE_VALUE;
}

#else

static int valid_handle(int h)
{
    return h >= 0;
}

static int close_handle(int h)
{
    if (valid_handle(h))
	OSapi_close(h);
    return -1;
}

#endif


#ifdef _WIN32
// Note:  following 2 functions may need tinkering for different architectures,
//    or across a network
static long get_actual_file_size(HANDLE fd)
    // figure the actual file size, the size that we can safely mmap,
    //   and not get any segmentation faults reading from
{
    long size = GetFileSize(fd, NULL);
    return size;
}

static bool file_size_is_at_least(HANDLE fd, long size, char const *)
{
    long actual_size = get_actual_file_size(fd);
    if (actual_size >= size )
	return 1;
    return 0;              // this file is truncated
}
#else
// Note:  following 2 functions may need tinkering for different architectures,
//    or across a network
static long get_actual_file_size(int fd)
    // figure the actual file size, the size that we can safely mmap,
    //   and not get any segmentation faults reading from
{
    long size = OSapi_lseek(fd, 0L, SEEK_END);
    return size;
}

static bool file_size_is_at_least(int fd, long size, char const *)
{
    long actual_size = get_actual_file_size(fd);
    if (actual_size >= size )
	return 1;
    return 0;              // this file is truncated
}
#endif

// utility function needed by deleteHashTable

static void deleteHashChain(hashElmt *p) {
  // destroy this element's subchain
  if (p) {
    deleteHashChain(p->next);

    p->el   = 0;
    p->next = 0;
    p->prev = 0;

    delete p; // uses the default destructor,
              // which destroys just this element
  }
}

#define MAX_SLEEP_TIME 20
#ifdef _WIN32

HANDLE open_timeout(char const *name, int flags, int mode)
{
    HANDLE fhandle   = INVALID_HANDLE_VALUE;  
    int    AbortFlag = 0;
    if (name && *name) 
    {
	int   SleepTime    = 1;
	int   MaxSleepTime = MAX_SLEEP_TIME;
	char const *timing = OSapi_getenv("DIS_OPEN_TIMEOUT");
	if (timing) {
	    MaxSleepTime = OSapi_atoi(timing);
	    if ( MaxSleepTime <= 0 )
		MaxSleepTime = MAX_SLEEP_TIME;
	}
	while (!AbortFlag)
	{
	    DWORD OpenError            = 0;
	    DWORD Access               = GENERIC_READ;
	    DWORD CreationDistribution = 0;
	    CreationDistribution      |= (flags & O_CREAT) != 0 ? OPEN_ALWAYS : OPEN_EXISTING;
	    Access                    |= (flags & O_RDWR) != 0 ? GENERIC_WRITE : 0;
	    DWORD ShareMode            = FILE_SHARE_READ;
	    ShareMode                 |= (flags & O_RDWR) != 0 ? 0 : FILE_SHARE_WRITE;

	    fhandle = CreateFile( OSPATH(name), Access, ShareMode,
				  NULL, CreationDistribution,
				  FILE_ATTRIBUTE_NORMAL, NULL);

	    OpenError= GetLastError();
	    if( fhandle != INVALID_HANDLE_VALUE )
		break;
	    if ( OpenError != ERROR_SHARING_VIOLATION )
		AbortFlag = 1;
	    else
	    {
		OS_dependent::sleep(SleepTime);
		if ( ++SleepTime > MaxSleepTime )
		AbortFlag = 1;
	    }
	}
    }
    if ( AbortFlag ) {
	msg("Cannot open $1", error_sev) << OSPATH(name) << eom;
    }
    return fhandle;
}

#else

int open_timeout(char const *name, int flags, int mode)
{
    int unit = -1;
    if (name && *name) {
#ifdef  EXCLUSIVE_OPEN
	int sleep_time=1;
	abort_flag = 0;
	int   max_sleep_time = MAX_SLEEP_TIME;
	char const *timing = OSapi_getenv("DIS_OPEN_TIMEOUT");
	if (timing) {
	    max_sleep_time = OSapi_atoi(timing);
	    if ( max_sleep_time <= 0 )
		max_sleep_time = MAX_SLEEP_TIME;
	}

	while (!abort_flag) {
	  unit=OS_dependent::open(name, flags, mode);
	  if (unit > -1)
		  break;
	  sleep(sleep_time);
	  if (++sleep_time > max_sleep_time)
			  abort_flag = 1;
	}
	if ( abort_flag ) {
	    msg("Cannot open $1", error_sev) << OSPATH(name) << eom;
	}

#else
	flags &= (O_EXCL ^ 0xffffffff);
	unit=OS_dependent::open(name, flags, mode);
#endif
    }
    return unit;
}

#endif

//
// Set x protection for file name. The x access (for owner, group and other) is simply 
// added to the file\'s existing access mode. Returns 0 for success, -1 for failure.
//
static int setx(char const *name)
{
    struct OStype_stat buf;
    struct OStype_stat* buf_ptr = &buf;
    int retval = OSapi_stat(name, buf_ptr);
    if (!retval)
	retval = OSapi_chmod(name, buf_ptr->st_mode|0111);
//
// patch; return success for now.
//
    retval = 0;
    return retval;
}

static void error_message_box(char const *mesg, char const *name, 
	bool system_error, int term_number)
{
    //  If term_number is nonzero, exit with that return code.  Otherwise, 
    //  return to caller.
    char buffer[400];
    char const *errmsg = "";
    if (system_error)
	errmsg = OSapi_strerror(errno);
    sprintf(buffer, mesg, name, errmsg);
    msg("$1", error_sev) << buffer << eom;
    if (term_number) 	  driver_exit(term_number);
}

//  normally called with a single argument, the term_number defaults to 92
static void error_truncating_file(char const *name, int term_number)
{
    error_message_box("Error truncating file %s: %s", name, YES, term_number);
}

static int force_move()
{
    if (_SHOW_MOVE==-1) {
	char const *hack = OSapi_getenv("PSETSHOWMOVE");
	if (!hack) hack = "none selected";
	if (!strcmp(hack,"show")) _SHOW_MOVE=1;
	else _SHOW_MOVE = 0;
    }

    return 0;
}

static int SYM_FILE_SIZE(bool /* flag */=0)
{
	if (is_model_build())
	    return 1000000;
        else return 250;
}
static int IND_FILE_SIZE(bool /* flag */=0)
{
  return SYM_FILE_SIZE();
}

static int LINK_FILE_SIZE(bool /* flag */=0)
{
        return 250;
}

void XrefTable::msync_xref()
/* only turn on for debugging memory map, invalidate cache too */
{
  if (!PSET_MSYNC_XREF) return;

#ifndef _WIN32
  if (ind_header)
    msync((caddr_t)ind_header, ind_map_size, MSYNC_FLAGS | MS_INVALIDATE);
  if (sym_header)
    msync((caddr_t)sym_header, sym_map_size, MSYNC_FLAGS | MS_INVALIDATE);
  if (link_header)
    msync((caddr_t)sym_header, link_map_size, MSYNC_FLAGS | MS_INVALIDATE);
#endif
}


Xref* XrefTable::get_Xref()
{
//  Initialize(Xreftable::get_Xref);
    if (my_Xref == SharedXref::get_cur_shared_xref())
	my_Xref = 0;
    if (my_Xref && my_Xref->lxref != this)
	my_Xref = 0;
    if (!my_Xref) {
	xrefSymbol* sym   = get_ind_addr();
	if (sym)  my_Xref = sym->get_Xref();
    }
    return my_Xref;
}

//  make enough room for n bytes at end of xref, returning the
//     offset of the first of those bytes
uint XrefTable::update_sym_addr_offset(int n)
{
  Initialize(xrefSymbol::update_sym_addr_offset);
  headerInfo* hi = this->get_header();
  uint tmp       = ntohl(hi->next_offset_sym);
  hi->next_offset_sym = htonl(tmp + n);
  return tmp;
}



// XrefTable's utility function to delete a hash table.
// there ought to be a hash table class with a destructor!

void XrefTable::deleteHashTable() {
  if (table && header) {
    uint limit = ntohl(header->table_max);

    // for each hash bucket
    for (int i = 0; i < limit; i++) {
      hashElmt *p = table[i];
      table[i]    = 0;

      deleteHashChain(p);
    }
    delete table;
  }
  table = 0;
}

#ifndef _WIN32
int truncate_file(int fl, unsigned size)
{
    return OSapi_ftruncate(fl, size);
}
#else
int truncate_file(HANDLE fl, unsigned size)
{
    unsigned int prev_size = GetFileSize(fl, NULL);

    DWORD dw_set  = SetFilePointer(fl, size, NULL, FILE_BEGIN);
    if (dw_set == -1) {
      msg("SetFilePointer failed with a return value of: $1", catastrophe_sev) << GetLastError() << eoarg << eom; 
    }

    BOOL ret_val = SetEndOfFile(fl);
    if (!ret_val) {
      msg("SetEndOfFile failed with a return value of: $1", catastrophe_sev) << GetLastError() << eoarg << eom; 
    }

    if (dw_set == -1 || !ret_val) {
      msg("File is being truncated to size: $1, previous size: $2", normal_sev) << size << eoarg << prev_size << eoarg << eom;  
    }

    return ret_val ? 0 : -1;
}
#endif

#ifndef _WIN32
unsigned file_size(int fl)
{
    unsigned size = (unsigned)OSapi_lseek (fl, 0, 2);
    return size;
}
#else
unsigned file_size(HANDLE fl)
{
    unsigned size = GetFileSize(fl, NULL);
    return size;
}
#endif

void XrefTable::reset_pmod(char const *sym_file, char const *ind_file, char const *link_file)
{
  Initialize(XrefTable::reset_pmod);
  int err;
  char *addr = 0;
  
  OSapi_unlink(sym_file);
  OSapi_unlink(ind_file);
  OSapi_unlink(link_file);
  int size = SYM_FILE_SIZE(SMALL);
  unit = open_timeout(symfile_name, O_RDWR|O_CREAT|O_EXCL, mode);
//
// Force x permission on newly created pmods (Smit\'s request, bug 9305).
//
    int sym_x = 0;
#ifdef _WIN32
    if(unit != INVALID_HANDLE_VALUE)
#else
    if(unit >= 0)
#endif
	sym_x = setx(symfile_name);
    unit1 = open_timeout(indfile_name, O_RDWR|O_CREAT|O_EXCL, mode);  

    int ind_x = 0;
#ifdef _WIN32
    if(unit1 != INVALID_HANDLE_VALUE)
#else
    if (unit1 >= 0)
#endif
       ind_x = setx(indfile_name);
  
    unit2 =  open_timeout(linkfile_name, O_RDWR|O_CREAT|O_EXCL, mode);
    int link_x = 0;
#ifdef _WIN32
    if(unit2 != INVALID_HANDLE_VALUE)
#else
    if(unit2 >= 0)
#endif
      link_x = setx(linkfile_name);
  
#ifdef _WIN32
    if(unit == INVALID_HANDLE_VALUE || unit1 == INVALID_HANDLE_VALUE || unit2 == INVALID_HANDLE_VALUE || sym_x || ind_x || link_x)
#else
    if (unit < 0 || unit1 < 0 || unit2 < 0 || sym_x || ind_x || link_x)
#endif
        return;
    err = truncate_file (unit, size); // make it of size
    if ( err != 0 ) {
	unit   = close_handle (unit);
	header = 0;
	error_truncating_file(sym_file); // fatal error message
    }
  // map shared initially so we can write the
  // initial state of the pmod.sym through to the disk.
  sym_map_size = size;
#ifdef _WIN32
  addr         = pset_mmap(sym_file, (caddr_t)0, sym_map_size, PAGE_READWRITE,
			   0, unit, 0);
#else
  addr         = pset_mmap(sym_file, (caddr_t)0, sym_map_size, PROT_READ|PROT_WRITE,
			   MAP_SHARED, unit, 0);
#endif
  if (addr == (char *)-1) {
      error_message_box("Unable to map %s: %s", sym_file, YES, 91); // fatal error
  }

  //  (note, we can debug these various maps by setenv PSET_MAP_FILE )
  //  (      see process/src/map_manager.C)
  sym_header      = (symHeaderInfo*) addr;
  int time_of_day = get_time_of_day();
  long sym_magic;
  OSapi_bcopy("#SYM", &sym_magic, sizeof(sym_magic));
  sym_header->magic    = sym_magic;
  sym_header->version  = htonl(2);
  sym_header->date     = htonl(time_of_day);
  sym_header->checksum = htonl(0);

  sym_header->offset_to_data = htonl(sizeof(symHeaderInfo));
  sym_header->dummy[0]       = htonl(0);
  sym_header->dummy[1]       = htonl(0);
  sym_header->dummy[2]       = htonl(0);
  sym_header->dummy[3]       = htonl(0);

  sym_addr = ((char *)addr + ntohl(sym_header->offset_to_data));
  header   = (headerInfo*) (sym_addr - sizeof(headerInfo));

  header->table_max       = htonl(TABLE_MAX);
  header->entry_cnt       = htonl(0);
  header->fragment_cnt    = htonl(0);
  header->sym_file_size   = htonl(size - ntohl(sym_header->offset_to_data));
  header->ind_file_size   = htonl(IND_FILE_SIZE(SMALL) - sizeof(indHeaderInfo));
  header->link_file_size  = htonl(LINK_FILE_SIZE() - sizeof(linkHeaderInfo));
  header->next_offset_sym = htonl(0);
  header->next_offset_ind = htonl(0);
  header->next_offset_link = htonl(0);

  /* start of ind file */

  size = ntohl(header->ind_file_size) + sizeof(indHeaderInfo);
  err  = truncate_file (unit1, size); // make it of size
  if ( err != 0 ) {
    unit1  = close_handle (unit1);
    unit   = close_handle (unit);
    header =  0;
    error_truncating_file(ind_file); // fatal error message
    return;
  }
  if ((ntohl(header->ind_file_size) < IND_FILE_SIZE())) {
    if (truncate_file(unit1, IND_FILE_SIZE())) {
	perror("Unable to resize ind file ");
	header=0;
	error_truncating_file(ind_file); // fatal error message
    } else {
	size = IND_FILE_SIZE();
    }
  }
  header->ind_file_size = htonl(size - sizeof(indHeaderInfo));

  // map shared initially so we can write the
  // initial state of the pmod.ind through to the disk.

  ind_map_size = size;
#ifdef _WIN32
  addr = pset_mmap(ind_file, (caddr_t)0,ind_map_size,PAGE_READWRITE,
		   0,unit1,0);
#else
  addr = pset_mmap(ind_file, (caddr_t)0,ind_map_size,PROT_READ|PROT_WRITE,
		   MAP_SHARED,unit1,0);
#endif

  if (addr == (char *)-1) {
    error_message_box("Unable to map %s: %s", ind_file, YES, 91); // fatal error
  }

  //  (note, we can debug these various maps by setenv PSET_MAP_FILE )
  //  (      see process/src/map_manager.C)

  ind_header = (indHeaderInfo*) addr;
  long ind_magic;
  OSapi_bcopy("#IND", &ind_magic, sizeof(ind_magic));
  ind_header->magic    = ind_magic;
  ind_header->version  = htonl(2);
  ind_header->date     = htonl(time_of_day);
  ind_header->checksum = htonl(0);

  ind_header->offset_to_data = htonl(sizeof(indHeaderInfo));
  ind_addr = (xrefSymbol *) (addr + ntohl(ind_header->offset_to_data));
  *((char *)ind_addr-1) = htonl(0);	// make sure file is at least big enough for header
  ind_header->dummy[0] = htonl(0);
  ind_header->dummy[1] = htonl(0);
  ind_header->dummy[2] = htonl(0);

  /* start of link file */

  size = ntohl(header->link_file_size) + sizeof(linkHeaderInfo);
  err  = truncate_file (unit2, size); // make it of size
  if ( err != 0 ) {
    unit1  = close_handle (unit1);
    unit   = close_handle (unit);
    unit2  = close_handle (unit2);
    header =  0;
    error_truncating_file(link_file); // fatal error message
    return;
  }
  if ((ntohl(header->link_file_size) < LINK_FILE_SIZE())) {
    if (truncate_file(unit2, LINK_FILE_SIZE())) {
	perror("Unable to resize ind file ");
	header=0;
	error_truncating_file(link_file); // fatal error message
    } else {
	size = LINK_FILE_SIZE();
    }
  }
  header->link_file_size = htonl(size - sizeof(linkHeaderInfo));
  link_map_size = size;
#ifdef _WIN32
  addr = pset_mmap(link_file, (caddr_t)0,link_map_size,PAGE_READWRITE,
		   0,unit2,0);
#else
  addr = pset_mmap(link_file, (caddr_t)0,link_map_size,PROT_READ|PROT_WRITE,
		   MAP_SHARED,unit2,0);
#endif

  if (addr == (char *)-1) {
    error_message_box("Unable to map %s: %s", link_file, YES, 91); // fatal error
  }

  link_header = (linkHeaderInfo*) addr;
  long link_magic;
  OSapi_bcopy("#LNK", &link_magic, sizeof(link_magic));
  link_header->magic    = link_magic;
  link_header->version  = htonl(2);
  link_header->date     = htonl(time_of_day);
  link_header->checksum = htonl(0);

  link_header->offset_to_data = htonl(sizeof(linkHeaderInfo));
  link_addr = (Link*) (addr + ntohl(link_header->offset_to_data));
  *((char *)link_addr-1) = htonl(0);	// make sure file is at least big enough for header
  ind_header->dummy[0] = htonl(0);
  ind_header->dummy[1] = htonl(0);
  ind_header->dummy[2] = htonl(0);

  fresh = 1;
  
  table = (hashElmt**)new char[ntohl(header->table_max) * sizeof(hashElmt *)];
  for (int i = 0; i < ntohl(header->table_max); ++i)
      table[i] = NULL;
  

  unit1 = close_handle(unit1);
  unit = close_handle(unit);
  unit2 = close_handle(unit2);
  
  do_backup();
}

void XrefTable::insert_sdo_update_time()
{
  time_t cl;
  OSapi_time(&cl);
  insert_time_t(&cl);
}

time_t *XrefTable::get_last_sdo_update(time_t *cl)
{
  if (cl)
    extract_time_t(cl);
  return cl;
}

static void strip_suffix(char const *suffix, genString &name)
{
  Initialize(strip_suffix);
  if (!suffix) return ;
  int nl = name.length(); 
  int sl = strlen(suffix);
  if (nl < sl ) return;
  char *strip = new char [nl + 1];
  strncpy(strip, name.str(), nl - sl);
  strip[nl - sl] = 0;
  name = strip;
  return;
}

XrefTable::XrefTable(Xref* parent) : my_Xref(parent), my_proj(0), next(0)
{
    Initialize(XrefTable::XrefTable(Xref*));
}



// This code has been changed to now *only* accept the new kind of pmod files
// call with flag==1 to request a read-only xref
//    or with flag==2 to request writable xref without backup, delete in dtor
//    or with flag==3 to request writable xref without backup, no delete in dtor

XrefTable::XrefTable(Xref* parent, char const *sym_file, char const *ind_file,
                     char const *link_file, int flag, projNode*) :
    my_Xref(parent),
    my_proj(0),
    next(0),
    symfile_name (sym_file),
    indfile_name (ind_file),
    linkfile_name(link_file),
    symbackup(0),
    indbackup(0),
    pmod_num(0),
    mode(0666)
{ 

    Initialize(XrefTable::XrefTable);
    nosaveflag = 0;		// do not disable save mechanism

    units_are_backup = 0;
    delete_in_destructor=0;	// by default, dont delete them

#ifdef _WIN32
    unit   = unit1 = unit2 = INVALID_HANDLE_VALUE;
#else
    unit   = -1;
    unit1  = -1;
    unit2 = -1;
#endif
    backup = 0;

    table      = 0;
    symbolIndex_table = 0;
    sym_header = 0;
    header     = 0;
    sym_addr   = 0;
    ind_header = 0;
    ind_addr   = 0;
    link_header = 0;
    link_addr = 0;
    int bck    = 0;		// do a backup by default
    if (flag==2) {
	bck                  = 1;   // suppress the backup
        delete_in_destructor = 1;   // delete files in destructor
        flag                 = 0;   // otherwise, treat like flag=0:  writable
    }
    if (flag==3) {
	bck                  = 1;   // suppress the backup
        delete_in_destructor = 0;	// do not delete files in destructor
        flag                 = 0;	// otherwise, treat like flag=0:  writable
    }

    fresh = 0;			// mark as using existing files (ie, not fresh)
    // until we get a chance to find out what's what

    int preexistingRO = 0;	// if 1, preexisting read-only XREF file
    
    check_old_version_of_pmod(symfile_name, indfile_name, parent);

    if(flag == 1){
	unit            = open_timeout(symfile_name, O_RDONLY, mode);
	sym_is_writable = 0;
    } else {
       sym_is_writable = 1;		// assume writable, until proven otherwise
       unit            = open_timeout(symfile_name, O_RDWR|O_CREAT|O_EXCL, mode);
    }
    //
    // Force x permission on newly created pmods (Smit\'s request, bug 9305), 
    // for newly created pmod files. 
    //
    int sym_x = 0;
#ifdef _WIN32
    if(unit != INVALID_HANDLE_VALUE)
#else
    if(unit >= 0)
#endif
	sym_x = setx(symfile_name);
#ifdef _WIN32
    if(unit == INVALID_HANDLE_VALUE){
	unit = open_timeout(symfile_name, O_RDONLY, mode);
	if(unit != INVALID_HANDLE_VALUE){
#else
    if(unit < 0) {
	unit = open_timeout(symfile_name, O_RDONLY, mode);
	if (unit >= 0) {
#endif
	    preexistingRO   = 1;
	    sym_is_writable = 0;	// it is a read-only file
	}
    }

#ifdef _WIN32
    if(unit == INVALID_HANDLE_VALUE || sym_x){
#else
    if (unit < 0 || sym_x) {
#endif
	msg("Cannot open $1", catastrophe_sev) << sym_file << eom;
	perror(NULL);
	//
	//  Do not return from here; exit for good.    
	//
	driver_exit(1);
    }
#ifdef _WIN32  
    int sym_map_prot = (sym_is_writable) ? PAGE_READWRITE : PAGE_READONLY;
#else
    int sym_map_prot = (sym_is_writable) ? (PROT_READ|PROT_WRITE) : PROT_READ;
#endif
    int size         = file_size(unit);
    if (preexistingRO && !size) {
	msg("Preexisting zero-length read-only Xref file: $1", warning_sev) << sym_file << eom;
#ifdef _WIN32
	CloseHandle(unit);
	unit = INVALID_HANDLE_VALUE;
#else
	OSapi_close(unit);
	unit = -1;
#endif
	header =  0;
	return;
    }
  
    int err;
    char *addr        = 0;
    int should_restore = 0;
    int time_of_day    = get_time_of_day();
    
    if ((!size) && !preexistingRO) { // fresh .sym file
	if (flag) {			// allocate small, it won't be used immediately
	    size = SYM_FILE_SIZE(SMALL);
	} else {
	    size = SYM_FILE_SIZE()*build_xref_flag;
	}
	err = truncate_file (unit, size); // make it of size
	if ( err != 0 ) {
	    unit   = close_handle (unit);
	    header = 0;
	    error_truncating_file(sym_file); // fatal error message
	}
	// map shared initially so we can write the
	// initial state of the pmod.sym through to the disk.
	sym_map_size = size;
#ifdef _WIN32
	addr         = pset_mmap(sym_file, (caddr_t)0,sym_map_size,sym_map_prot,
				 0,unit,0);
#else
	addr         = pset_mmap(sym_file, (caddr_t)0,sym_map_size,sym_map_prot,
				 MAP_SHARED,unit,0);
#endif
      
	if (addr == (char *)-1) {
	    error_message_box("Unable to map %s: %s", sym_file, YES, 91); // fatal error
	}
	
	//  (note, we can debug these various maps by setenv PSET_MAP_FILE )
	//  (      see process/src/map_manager.C)
	sym_header = (symHeaderInfo*) addr;
	long tmp_magic;
	OSapi_bcopy("#SYM", &tmp_magic, sizeof(tmp_magic));
	sym_header->magic    = tmp_magic;
	sym_header->version  = htonl(2);
	sym_header->date     = htonl(time_of_day);
	sym_header->checksum = htonl(0);
	
	sym_header->offset_to_data = htonl(sizeof(symHeaderInfo));
	sym_header->dummy[0]       = htonl(0);
	sym_header->dummy[1]       = htonl(0);
	sym_header->dummy[2]       = htonl(0);
	sym_header->dummy[3]       = htonl(0);

	sym_addr = ((char *)addr + ntohl(sym_header->offset_to_data));
	header   = (headerInfo*) (sym_addr - sizeof(headerInfo));
	
	header->table_max     = htonl(TABLE_MAX);
	header->entry_cnt     = htonl(0);
	header->fragment_cnt  = htonl(0);
	header->sym_file_size = size - ntohl(sym_header->offset_to_data);
	if (flag)
	    header->ind_file_size = htonl(IND_FILE_SIZE(SMALL) - sizeof(indHeaderInfo));
	else
	    header->ind_file_size = htonl(IND_FILE_SIZE() * build_xref_flag - sizeof(indHeaderInfo));
	header->next_offset_sym = htonl(0); 
	header->next_offset_ind = htonl(0);
	header->next_offset_link = htonl(0);
	
	fresh = 1;
  } else {			// pre-existing .sym file
#ifdef _WIN32
      SetFilePointer(unit, 0, NULL, FILE_BEGIN);
#else
      OSapi_lseek(unit, 0, 0);
#endif
      symHeaderInfo shi;
      OSapi_bzero(&shi, sizeof(symHeaderInfo));
#ifdef _WIN32
      DWORD bytes_readed;
      ReadFile(unit, &shi, sizeof(symHeaderInfo), &bytes_readed, NULL);
#else
      OSapi_read(unit, (char *)&shi, sizeof(symHeaderInfo));
#endif
      long tmp_magic;
      OSapi_bcopy("#SYM", &tmp_magic, sizeof(tmp_magic));
      if (shi.magic != tmp_magic || ntohl(shi.version) != 2) {
	  msg("Wrong version of pmod file: $1\n"
	      "Please rebuild the project $2", catastrophe_sev)
			<< sym_file << eoarg
			<< parent->get_projNode()->get_ln() << eom;
#ifdef _WIN32
	  CloseHandle(unit);
	  unit = INVALID_HANDLE_VALUE;
#else
	  OSapi_close(unit);
	  unit   = -1;
#endif
	  header = 0;
	  driver_exit(90);
      }
      sym_map_size = ntohl(shi.offset_to_data) + ntohl(shi.oldheader.next_offset_sym);
      if (!file_size_is_at_least(unit, sym_map_size, sym_file)) {
	  msg("Truncated file $1\n\tPlease re-run model build for project $2", catastrophe_sev)
			<< sym_file << eoarg
			<< parent->get_projNode()->get_ln() << eom;
#ifdef _WIN32
	  CloseHandle(unit);
	  unit = INVALID_HANDLE_VALUE;
#else
	  OSapi_close(unit);
	  unit   = -1;
#endif
	  header = 0;
	  if (!is_model_build())
	      driver_exit(90);
	  else
	      reset_pmod(sym_file, ind_file, link_file);
	  return;
      }

#ifdef _WIN32
      if (flag)
	  addr = pset_mmap(sym_file, (caddr_t)0, sym_map_size, PAGE_READONLY, 0, unit, 0);
      else
	  addr = pset_mmap(sym_file, (caddr_t)0, sym_map_size, sym_map_prot, 0, unit, 0);
#else
      if (flag)
	  addr = pset_mmap(sym_file, (caddr_t)0, sym_map_size, PROT_READ, MAP_SHARED, unit, 0);
      else
	  addr = pset_mmap(sym_file, (caddr_t)0, sym_map_size, sym_map_prot, MAP_SHARED, unit, 0);
#endif
      if (addr == (char *)-1) {
	  error_message_box("Unable to map %s: %s", sym_file, YES, 91); // fatal error
      }
      sym_header = (symHeaderInfo*) addr;
      sym_addr   = ((char *)addr + ntohl(sym_header->offset_to_data));
      header     = (headerInfo*) (sym_addr - sizeof(headerInfo));
      should_restore = 1;
  }				// common logic for both new and pre-existing .sym files
    
    // SIGBUS should probably be inhibited for entire constructor...
    //  SIG_TYP old_signal_handler=(SIG_TYP) signal(SIGBUS, (SIG_TYP)bus_signal_handler);
    //  bus_error = 0;             // no error so far
    
    // following logic was to test whether we would get a bus error while reading up
    //   to the number of bytes specified by stat.  Keep it around to test other
    //   architectures that might permit holes in files.
    
    //  restore the signal handler
    //  signal(SIGBUS, (SIG_TYP)old_signal_handler);
    
    if(flag == 1){
	    unit1            = open_timeout(ind_file, O_RDONLY, mode);
	    ind_is_writable = 0;
    } else {
       ind_is_writable = 1;		// assume writable, until proven otherwise
       unit1            = open_timeout(ind_file, O_RDWR|O_CREAT|O_EXCL, mode);
    }

    //
    // Force x permission on newly created pmods (Smit\'s request, bug 9305).
    //
    int ind_x = 0;
    if (unit1 >= 0)
	ind_x = setx(ind_file);
    if (unit1 < 0) {
	unit1 = open_timeout(indfile_name, O_RDONLY, mode ); // class member
	if (unit1 >= 0) {
	    preexistingRO   = 1;
	    ind_is_writable = 0;	// it is a read-only file
	}
    }
    
    if (unit1 < 0 || ind_x) {
	msg("Cannot open $1", catastrophe_sev) << ind_file << eom;
	perror(NULL);
	driver_exit(1);
    }
#ifdef _WIN32
    int ind_map_prot = (ind_is_writable) ? PAGE_READWRITE : PAGE_READONLY;
#else
    int ind_map_prot = (ind_is_writable) ? (PROT_READ|PROT_WRITE) : PROT_READ;
#endif
    size             = (int)file_size(unit1);
    if (preexistingRO && !size) {
	msg("Preexisting zero-length read-only index file: $1", warning_sev) << ind_file << eom;
	unit1  = close_handle (unit1);
	unit   = close_handle (unit);
	header =  0;
	return;
    }
    if (!size && !should_restore) { // fresh .ind file
	size = ntohl(header->ind_file_size) + sizeof(indHeaderInfo);
	err  = truncate_file (unit1, size); // make it of size
	if ( err != 0 ) {
	    unit1  = close_handle (unit1);
	    unit   = close_handle (unit);
	    header =  0;
	    error_truncating_file(ind_file); // fatal error message
	    return;
	}
	if (ntohl(header->ind_file_size) < IND_FILE_SIZE() && !preexistingRO) {
	    if (truncate_file(unit1, IND_FILE_SIZE())) {
		perror("Unable to resize ind file ");
		header = 0;
		error_truncating_file(ind_file); // fatal error message
	    } else {
		size = IND_FILE_SIZE();
	    }
	}
	header->ind_file_size = htonl(size - sizeof(indHeaderInfo));
	
	// map shared initially so we can write the
	// initial state of the pmod.ind through to the disk.
	
	ind_map_size = size;
#ifdef _WIN32       
	addr         = pset_mmap(ind_file, (caddr_t)0, ind_map_size, ind_map_prot,
				 0, unit1, 0);
#else
	addr         = pset_mmap(ind_file, (caddr_t)0, ind_map_size, ind_map_prot,
				 MAP_SHARED, unit1, 0);
#endif
	
	if (addr == (char *)-1) {
	    error_message_box("Unable to map %s: %s", ind_file, YES, 91); // fatal error
	}
	
	//  (note, we can debug these various maps by setenv PSET_MAP_FILE )
	//  (      see process/src/map_manager.C)
	
	ind_header = (indHeaderInfo*) addr;
	long ind_header_magic;
	OSapi_bcopy("#IND", &ind_header_magic, sizeof(ind_header_magic));
	ind_header->magic    = ind_header_magic;
	ind_header->version  = htonl(2);
	ind_header->date     = htonl(time_of_day);
	ind_header->checksum = htonl(0);
	
	ind_header->offset_to_data = htonl(sizeof(indHeaderInfo));
	ind_addr                   = (xrefSymbol*)((char *)addr + ntohl(ind_header->offset_to_data));
	*((char *)ind_addr - 1)     = 0;	// make sure file is at least big enough for header
	ind_header->dummy[0]       = htonl(0);
	ind_header->dummy[1]       = htonl(0);
	ind_header->dummy[2]       = htonl(0);
	
#ifndef _WIN32	
	if (flag) {
	    // Caller asked for read only. Now that we've initialized, make it so
	    mprotect((caddr_t) ind_header, ind_map_size, PROT_READ);
	    mprotect((caddr_t) sym_header, sym_map_size, PROT_READ);
	}
#endif
	fresh = 1;
    } else {	
	// pre-existing .ind file
	if (fresh) {
	    msg("Apparently symbol file $1 is fresh, but index file $2 is not?", warning_sev) << sym_file << eoarg << ind_file << eom;
	}
	
#ifdef _WIN32
	SetFilePointer(unit1, 0, NULL, FILE_BEGIN);
#else
	OSapi_lseek(unit1, 0, 0);
#endif
	indHeaderInfo ihi;
	OSapi_bzero(&ihi, sizeof(indHeaderInfo));
#ifdef _WIN32
	DWORD bytes_readed;
	ReadFile(unit1, &ihi, sizeof(indHeaderInfo), &bytes_readed, NULL);
#else
	OSapi_read(unit1, (char *)&ihi, sizeof(indHeaderInfo));
#endif
	long ind_header_magic;
	OSapi_bcopy("#IND", &ind_header_magic, sizeof(ind_header_magic));
	if (ihi.magic != ind_header_magic || ntohl(ihi.version) != 2) {
	    unit1 = close_handle(unit1);
	    unit = close_handle(unit);
	    header = 0;
	    error_message_box("Wrong version of %s: %s", ind_file, YES, 90); // fatal error
	    //return;
	}
	ind_map_size = ntohl(header->next_offset_ind) + ntohl(ihi.offset_to_data);
	if (!file_size_is_at_least(unit1, ind_map_size, ind_file)) {
	    msg("Fatal error: Truncated file $1\n\tPlease re-run model build for project $2", catastrophe_sev)
			<< ind_file << eoarg
			<< parent->get_projNode()->get_ln() << eom;      
	    unit1 = close_handle(unit1);
	    unit = close_handle(unit);
	    header = 0;
	    if (!is_model_build())
		driver_exit(90);
	    else 
		reset_pmod(sym_file, ind_file, link_file);
	    return;
	}
	
#ifdef _WIN32
	if (flag)
	    addr = pset_mmap(ind_file, (caddr_t)0,ind_map_size,PAGE_READONLY, 0, unit1, 0);
	else
	    addr = pset_mmap(ind_file, (caddr_t)0,ind_map_size,ind_map_prot, 0, unit1, 0);
#else
	if (flag)
	    addr = pset_mmap(ind_file, (caddr_t)0,ind_map_size,PROT_READ,MAP_SHARED,unit1,0);
	else
	    addr = pset_mmap(ind_file, (caddr_t)0,ind_map_size,ind_map_prot, MAP_SHARED,unit1,0);
#endif
	if (addr == (char *)-1) {
	    error_message_box("Unable to map %s: %s", ind_file, YES, 91); // fatal error
	}
	
	ind_header = (indHeaderInfo*) addr;
	ind_addr   = (xrefSymbol*) ((char *)addr + ntohl(ind_header->offset_to_data));
	
    }				// end of else, both new file and pre-existing come here

   //open link file
    if(flag == 1){
      unit2            = open_timeout(link_file, O_RDONLY, mode);
      link_is_writable = 0;
    } else {
      link_is_writable = 1;		// assume writable, until proven otherwise
      unit2            = open_timeout(link_file, O_RDWR|O_CREAT|O_EXCL, mode);
    }
    //
    // Force x permission on newly created pmods (Smit\'s request, bug 9305).
    //
    int link_x = 0;
    if (unit2 >= 0)
	link_x = setx(link_file);
    if (unit2 < 0) {
	unit2 = open_timeout(linkfile_name, O_RDONLY, mode ); // class member
	if (unit2 >= 0) {
	    preexistingRO   = 1;
	    link_is_writable = 0;	// it is a read-only file
	}
    }
    
    if (unit2 < 0 || link_x) {
	msg("Cannot open $1", catastrophe_sev) << link_file << eom;
	perror(NULL);
	driver_exit(1);
    }
#ifdef _WIN32
    int link_map_prot = (link_is_writable) ? PAGE_READWRITE : PAGE_READONLY;
#else
    int link_map_prot = (link_is_writable) ? (PROT_READ|PROT_WRITE) : PROT_READ;
#endif
    size             = (int)file_size(unit2);
    if (preexistingRO && !size) {
	msg("Preexisting zero-length read-only link file: $1", warning_sev) << link_file << eom;
	unit1  = close_handle (unit1);
	unit   = close_handle (unit);
	unit2 = close_handle (unit2);
	header =  0;
	return;
    }
    if (!size && !should_restore) { // fresh .lin file
	size = ntohl(header->link_file_size) + sizeof(linkHeaderInfo);
	err  = truncate_file (unit2, size); // make it of size
	if ( err != 0 ) {
	    unit1  = close_handle (unit1);
	    unit   = close_handle (unit);
            unit2  = close_handle (unit2);
	    header =  0;
	    error_truncating_file(link_file); // fatal error message
	    return;
	}
	if (ntohl(header->link_file_size) < LINK_FILE_SIZE() && !preexistingRO) {
	    if (truncate_file(unit2, LINK_FILE_SIZE())) {
		perror("Unable to resize link file ");
		header = 0;
		error_truncating_file(link_file); // fatal error message
	    } else {
		size = LINK_FILE_SIZE();
	    }
	}
	header->link_file_size = htonl(size - sizeof(linkHeaderInfo));
	
	// map shared initially so we can write the
	// initial state of the pmod.lin through to the disk.
	
	link_map_size = size;
#ifdef _WIN32       
	addr         = pset_mmap(link_file, (caddr_t)0, link_map_size, link_map_prot,
				 0, unit2, 0);
#else
	addr         = pset_mmap(link_file, (caddr_t)0, link_map_size, link_map_prot,
				 MAP_SHARED, unit2, 0);
#endif
	
	if (addr == (char *)-1) {
	    error_message_box("Unable to map %s: %s", link_file, YES, 91); // fatal error
	}
	
	link_header = (linkHeaderInfo*) addr;
	long link_header_magic;
	OSapi_bcopy("#LNK", &link_header_magic, sizeof(link_header_magic));
	link_header->magic    = link_header_magic;
	link_header->version  = htonl(2);
	link_header->date     = htonl(time_of_day);
	link_header->checksum = htonl(0);
	
	link_header->offset_to_data = htonl(sizeof(linkHeaderInfo));
	link_addr                   = (Link*)((char *)addr + ntohl(link_header->offset_to_data));
	*((char *)link_addr - 1)     = 0;	// make sure file is at least big enough for header
	link_header->dummy[0]       = htonl(0);
	link_header->dummy[1]       = htonl(0);
	link_header->dummy[2]       = htonl(0);

#ifndef _WIN32	
	if (flag) {
	    // Caller asked for read only. Now that we've initialized, make it so
	    mprotect((caddr_t) ind_header, ind_map_size, PROT_READ);
	    mprotect((caddr_t) sym_header, sym_map_size, PROT_READ);
	    mprotect((caddr_t) link_header, link_map_size, PROT_READ);
	}
#endif
	fresh = 1;
    } else {	
	// pre-existing .lin file
	if (fresh) {
	    msg("Apparently symbol file $1 is fresh, but link file $2 is not?", warning_sev) << sym_file << eoarg << link_file << eom;
	}
	
#ifdef _WIN32
	SetFilePointer(unit2, 0, NULL, FILE_BEGIN);
#else
	OSapi_lseek(unit2, 0, 0);
#endif
	linkHeaderInfo link_hi;
	OSapi_bzero(&link_hi, sizeof(linkHeaderInfo));
#ifdef _WIN32
	DWORD bytes_read;
	ReadFile(unit2, &link_hi, sizeof(linkHeaderInfo), &bytes_read, NULL);
#else
	OSapi_read(unit2, (char *)&link_hi, sizeof(linkHeaderInfo));
#endif
	long link_header_magic;
	OSapi_bcopy("#LNK", &link_header_magic, sizeof(link_header_magic));
	if (link_hi.magic != link_header_magic || ntohl(link_hi.version) != 2) {
	    unit1 = close_handle(unit1);
	    unit = close_handle(unit);
	    unit2 = close_handle(unit2);
	    header = 0;
	    error_message_box("Wrong version of %s: %s", link_file, YES, 90); // fatal error
	    //return;
	}

	link_map_size = ntohl(header->next_offset_link) + ntohl(link_hi.offset_to_data);
	if (!file_size_is_at_least(unit2, link_map_size, link_file)) {
	  msg("Fatal error: Truncated file $1\n\tPlease re-run model build for project $2", catastrophe_sev)
			<< link_file << eoarg
			<< parent->get_projNode()->get_ln() << eom;      
	  unit1 = close_handle(unit1);
	  unit = close_handle(unit);
	  unit2 = close_handle(unit2);
	  header = 0;
	  if (!is_model_build())
	    driver_exit(90);
	  else 
	    reset_pmod(sym_file, ind_file, link_file);
	  return;
	} 

#ifdef _WIN32
	if (flag)
	    addr = pset_mmap(link_file, (caddr_t)0,link_map_size,PAGE_READONLY, 0, unit2, 0);
	else
	    addr = pset_mmap(link_file, (caddr_t)0,link_map_size,link_map_prot, 0, unit2, 0);
#else
	if (flag)
	    addr = pset_mmap(link_file, (caddr_t)0,link_map_size,PROT_READ,MAP_SHARED,unit2,0);
	else
	    addr = pset_mmap(link_file, (caddr_t)0,link_map_size,link_map_prot, MAP_SHARED,unit2,0);
#endif
	if (addr == (char *)-1) {
	    error_message_box("Unable to map %s: %s", link_file, YES, 91); // fatal error
	}
	
	link_header = (linkHeaderInfo*) addr;
	link_addr   = (Link*) ((char *)addr + ntohl(link_header->offset_to_data));
	
	    // for bug 5119:
	if (ntohl(header->entry_cnt)) {
	  if (file_size(unit2) <= sizeof(linkHeaderInfo)) {
	    msg("Link pmod file \"$1\" has no links,\n"
		"please rebuild the project \"$2\".", catastrophe_sev)
			<< link_file << eoarg
			<< parent->get_projNode()->get_ln() << eom;
	    unit = close_handle(unit);
	    unit1 = close_handle(unit1);
	    unit2 = close_handle(unit2);
	    header = 0;
	    if (!is_model_build())
	      driver_exit(93);
	    else
	      reset_pmod(sym_file, ind_file, link_file);
	  }
	  else {
	  // if there is anything in the pmod, make sure it ends with a last_link
	  Link* lnk = (Link*)((char *)link_addr + ntohl(header->next_offset_link)) - 1;
	  if (lnk->lt() != lt_last) {
	    if (!is_model_build()) {
	      msg("A link in $1 is broken. Please rebuild the model.", error_sev) << link_file << eoarg << eom;
	    } else {
	      msg("A link in $1 is broken.") << link_file << eom;
	    }
	  }
	}
	}
    }				// end of else, both new file and pre-existing come here
    saved_next_loffset = ntohl(header->next_offset_link);

    /* make sure sym and ind files is a pair */
    if (sym_header && ind_header && link_header && ((ntohl(sym_header->date) != ntohl(ind_header->date)) || (ntohl(ind_header->date) != ntohl(link_header->date)))) {
	genString xref_n = sym_file;
	strip_suffix(".sym", xref_n);
	if (!xref_n.str()) xref_n = sym_file;
	msg("SYM, IND and LIN files for %s are incompatible. Please check date and/or rebuild this model.", catastrophe_sev) << (char *)xref_n << eom;
	driver_exit(91);
    }

    next_offset_sym = 0;
    table           = (hashElmt**)new char[ntohl(header->table_max) * sizeof(hashElmt *)];
    for (int i = 0; i < ntohl(header->table_max); ++i)
	table[i] = NULL;
    if (should_restore) {
        int sis          = SIZEOF_XREFSYM;
	bool need_rehash = 0;
	long max_offset  = ntohl(header->next_offset_ind);
	for (int ii = 0; ii * sis < max_offset; ++ii) {
	    xrefSymbol* xrefsym = get_ind_addr() + ii;
	    hashElmt* he    = new hashElmt(xrefsym);
	    int h           = xrefsym->get_hash_value() % ntohl(header->table_max); // 3/17
	    if (h >= ntohl(header->table_max)) {
		need_rehash = 1;
		h           = h % ntohl(header->table_max);
	    }
	    if (table[h])
		table[h]->prev = he;
	    he->next = table[h];
	    he->prev = NULL;
	    table[h] = he;
	}				// end for()

	if (need_rehash) {
	    if (!is_model_build()) {
		msg("Hash value(s) in $1 is broken.  Please rebuild the model." , error_sev) << ind_file << eom;
	    } else {
		msg("Hash value(s) in $1 is broken.") << ind_file << eom;
	    }
	}
    }
      
    if (!pset_copy_rdonly_pmods)
	if (is_model_build() || !OSapi_getenv ("PSET_COPY_RDONLY_PMODS"))
	    pset_copy_rdonly_pmods = -1;
	else
	    pset_copy_rdonly_pmods = 1;
    
    unit1 = close_handle(unit1);
    unit = close_handle(unit);
    unit2 = close_handle(unit2);

    if (!bck && !flag)
	do_backup ();
    else if (sym_is_writable == 0 && pset_copy_rdonly_pmods == 1) {
	do_backup ();
	units_are_backup = 2;
    }

#ifdef _PSET_DEBUG
    is_consistent_hashElmts();
#endif
    pmod_num = insert_to_pmod_array();
}

//
// Returns NULL string pointer for success, pointer to offending file name for
// failure. 
//
// In addition, do_backup immediately exits paraset upon detecting certain system 
// call failures, while it returns offending file names for others. The reason for this
// inconsistency is that the function, and its callers, were poorly designed to start 
// with, so it is nearly impossible to fix their flaws now (they return from all over the 
// code, do not check return values of functions they call, etc.). do_backup should _always_ 
// return name of offending file, if any, and let the caller report it.
//
//                                                              ljb, 05/15/1995
//

char const *XrefTable::do_backup ()
{
    Initialize(XrefTable::do_backup);    

    char const *retval = NULL;
    if ((char const *)symbackup == 0) {
	char const *pure_name = strrchr ((char const *)symfile_name, '/');
	pure_name = pure_name ? pure_name + 1 : (char const *)symfile_name;
	char *tn = OSapi_tempnam (NULL, pure_name);
	symbackup = tn;
	free(tn);
	symbackup += ".pmod";
	indbackup = symbackup;
	linkbackup = symbackup;
	symbackup += ".sym";
	indbackup += ".ind";
	linkbackup += ".lin";
	// open temporary files
	int sym_x = 0;
#ifdef _WIN32
	HANDLE un = open_timeout(symbackup, O_RDWR|O_CREAT|O_EXCL, mode);
	if(un != INVALID_HANDLE_VALUE)
#else
	int un = open_timeout(symbackup, O_RDWR|O_CREAT|O_EXCL, mode);
	if (un >= 0)
#endif
	    sym_x = setx(symbackup);
	int ind_x = 0;
#ifdef _WIN32
	HANDLE un1 = open_timeout(indbackup, O_RDWR|O_CREAT|O_EXCL, mode);
	if(un1 != INVALID_HANDLE_VALUE)
#else
	int un1 = open_timeout(indbackup, O_RDWR|O_CREAT|O_EXCL, mode);
	if (un1 >= 0)
#endif
	    ind_x = setx(indbackup);

	int link_x = 0;
#ifdef _WIN32
	HANDLE un2 = open_timeout(linkbackup, O_RDWR|O_CREAT|O_EXCL, mode);
	if(un2 != INVALID_HANDLE_VALUE)
#else
	int un2 = open_timeout(linkbackup, O_RDWR|O_CREAT|O_EXCL, mode);
	if (un2 >= 0)
#endif
	  link_x = setx(linkbackup);

#ifdef _WIN32
	if(un == INVALID_HANDLE_VALUE || sym_x)
#else
	if (un < 0 || sym_x)
#endif
	    retval = symbackup;
#ifdef _WIN32
	if((un1 == INVALID_HANDLE_VALUE || ind_x) && (un != INVALID_HANDLE_VALUE && sym_x))
#else
	if ((un1 < 0 || ind_x) && (un >= 0 && sym_x))
#endif
	    retval = indbackup;
#ifdef _WIN32
	if(retval == NULL && (un2 == INVALID_HANDLE_VALUE || link_x))
#else
	if (retval == NULL && (un2 < 0 || link_x))
#endif
	    retval = linkbackup;

	if (retval == NULL) {
	    // copy symbol file
	    int sz1 = ntohl(header->next_offset_sym) + ntohl(sym_header->offset_to_data);
	    caddr_t ad = (caddr_t)sym_header;
	    int err = truncate_file(un, MAX(sz1,sym_map_size));
	    if ( err != 0 ) {
		close_handle (un1);
		close_handle (un);
		close_handle (un2);
		char const *crash_file = symbackup;
		symbackup = 0;
		indbackup = 0;
		linkbackup = 0;
		header    = 0;
		error_truncating_file(crash_file);      // fatal error message
	    }
	    pset_munmap(ad, sym_map_size);
#ifndef _WIN32
	    ad = pset_mmap(symbackup, (caddr_t)0, sym_map_size,
			   PROT_READ|PROT_WRITE, MAP_SHARED, un, 0);
#else
	    ad = pset_mmap(symbackup, (caddr_t)0, sym_map_size,
			   PAGE_READWRITE, 0, un, 0);
#endif
	    if (ad == (char *)-1)
		error_message_box("Unable to map %s: %s", symbackup, YES, 91); // fatal error
           // open original files for read.
    // open original files for read.
	    unit = open_timeout (symfile_name, O_RDONLY|O_EXCL, mode);
#ifdef _WIN32
		  if(unit == INVALID_HANDLE_VALUE)
#else
	    if(unit < 0)
#endif
			retval = symfile_name;

	    if (retval == NULL) {
#ifdef _WIN32		    
		DWORD bytes_readed;
		if(!ReadFile(unit, ad, sz1, &bytes_readed, NULL))
		    retval = symbackup;
#else
		if (OSapi_read (unit, ad, sz1) < 0)
		    retval = symbackup;
#endif
		if (retval == NULL) {
		    // fixup some data after remap to new address
		    sym_header = (symHeaderInfo *)ad;
		    sym_addr = (char *)(ad + ntohl(sym_header->offset_to_data));
		    header     = (headerInfo *)(sym_addr - sizeof(headerInfo));
		    // copy index file
		    int sz2 = ntohl(header->next_offset_ind) + ntohl(ind_header->offset_to_data);
		    caddr_t ad1 = (caddr_t)ind_header;
		    err     = truncate_file (un1, MAX(sz2,ind_map_size));
		    if ( err != 0 ) {
			close_handle (un1);
			close_handle (un);
			symbackup = 0;
			char const *crash_file = indbackup;
			indbackup = 0;
			header=0;
			error_truncating_file(crash_file);      // fatal error message
		    }
		    indHeaderInfo* oldaddress = ind_header;
		    pset_munmap(ad1,ind_map_size);
#ifndef _WIN32
		    ad1 = pset_mmap(indbackup, 0, ind_map_size,
				    PROT_READ|PROT_WRITE, MAP_SHARED,
				    un1, 0);
#else
		    ad1 = pset_mmap(indbackup, 0, ind_map_size,
				    PAGE_READWRITE, 0,
				    un1, 0);
#endif
		    if (ad1 == (char *)-1) 
			error_message_box("Unable to map %s: %s", indbackup, YES, 91); // fatal error
		    unit1 = open_timeout (indfile_name, O_RDONLY|O_EXCL, mode);
#ifdef _WIN32
		    if(unit1 == INVALID_HANDLE_VALUE)
#else
	            if(unit1 < 0)
#endif
			retval = symfile_name;
		    if (retval == NULL) {
#ifdef _WIN32
			DWORD bytes_readed;
			if(!ReadFile(unit1, ad1, sz2, &bytes_readed, NULL))
			    retval = indbackup;
#else
			if (OSapi_read (unit1, ad1, sz2) < 0)
			    retval = indbackup;
#endif
			if (retval == NULL) {
			    // fixup some data after remap to new address
			    ind_header = (indHeaderInfo *)ad1;
			    ind_addr   = (xrefSymbol*) (ad1 + ntohl(ind_header->offset_to_data));   
			    int distance_moved =  ad1 - (char *)oldaddress;
			    if (distance_moved) {
				int h;
				for (h=0; h<ntohl(header->table_max); h++) {
				    hashElmt * hel = table[h];
				    while (hel) {
					hel->el = (xrefSymbol*)( ((char *) hel->el) + distance_moved);
					hel = hel->next;
				    }
				}
			    }
#ifdef _PSET_DEBUG
			    is_consistent_hashElmts();
#endif
			  }
		      }
		    // copy link file
		    int sz3 = ntohl(header->next_offset_link) + ntohl(link_header->offset_to_data);
		    caddr_t ad2 = (caddr_t)link_header;
		    err     = truncate_file (un2, MAX(sz3,link_map_size)); 
		    if ( err != 0 ) {
			close_handle (un1);
			close_handle (un);
			close_handle (un2);
			symbackup = 0;
			indbackup = 0;
			char const *crash_file = linkbackup;
			linkbackup = 0;
			header=0;
			error_truncating_file(crash_file);      // fatal error message
		    }
		    pset_munmap(ad2,link_map_size);
#ifndef _WIN32
		    ad2 = pset_mmap(linkbackup, 0, link_map_size,
				    PROT_READ|PROT_WRITE, MAP_SHARED,
				    un2, 0);
#else
		    ad2 = pset_mmap(linkbackup, 0, link_map_size,
				    PAGE_READWRITE, 0,
				    un2, 0);
#endif
		    if (ad2 == (char *)-1) 
			error_message_box("Unable to map %s: %s", linkbackup, YES, 91); // fatal error
		    unit2 = open_timeout (linkfile_name, O_RDONLY|O_EXCL, mode);
#ifdef _WIN32
		    if(unit2 == INVALID_HANDLE_VALUE)
#else
	            if(unit2 < 0)
#endif
			retval = linkfile_name;
		    if (retval == NULL) {
#ifdef _WIN32
			DWORD bytes_read;
			if(!ReadFile(unit2, ad2, sz3, &bytes_read, NULL))
			    retval = linkbackup;
#else
			if (OSapi_read (unit2, ad2, sz3) < 0)
			    retval = linkbackup;
#endif
			if (retval == NULL) {
			  // fixup some data after remap to new address
			  link_header = (linkHeaderInfo *)ad2;
			  link_addr = (Link *)(ad2 + ntohl(link_header->offset_to_data));
			}
		      }
		    // close original files
		    unit1 = close_handle (unit1);
		    unit  = close_handle (unit);
		    unit2 = close_handle(unit2);
		    un1   = close_handle (un1);
		    un    = close_handle (un);
		    un2 = close_handle(un2);
		    units_are_backup = 1;

		  }
	    }
	}
    }
    return retval;
}

extern void xref_attribute_invalidate();

static objArr * pmod_arr;

int XrefTable::insert_to_pmod_array() const
{
    Initialize(XrefTable::insert_to_pmod_array);

    if (!pmod_arr) {
	pmod_arr = new objArr;
	pmod_arr->insert_last(0);
    }
    pmod_arr->insert_last(this);
    return pmod_arr->size() - 1;
}

void XrefTable::remove_from_pmod_array() const
{
    Initialize(XrefTable::remove_from_pmod_array);

    if (pmod_arr) {
	if (pmod_num > 0 && pmod_num < pmod_arr->size())
	    (*pmod_arr)[pmod_num] = 0;
    }
}

XrefTable* XrefTable::get_XrefTable_from_num(int num)
{
    Initialize(XrefTable::get_XrefTable_from_num);

    XrefTable* retval = (XrefTable*)0;

    if (pmod_arr)
	if (num > 0 && num < pmod_arr->size())
	    retval = (XrefTable*)(*pmod_arr)[num];

    return retval;
}

static void update_offset_array(int add_flag, int *offset_arr, int index, int max, int num_of_bytes)
{
  for (int i = index; i < max; i++) {
    if(offset_arr[i] >= 0) { //for old symbols, for new syms this val is -1
      if (add_flag)
	offset_arr[i] += num_of_bytes;
      else
	offset_arr[i] -= num_of_bytes;
    }
    else
      break;
  }
}

void XrefTable::update_link_offsets_of_syms(int *nlink_offsets)
{
  xrefSymbol* sym;
  int sz = (symbolIndex_table) ? symbolIndex_table->size() : 0;
  for (int i=1; i<sz; i++) {
    sym = (xrefSymbol *)(get_ind_addr()+i-1);
    sym->set_offset_to_first_link(nlink_offsets[i]);
  }
  return;
}

/*This function merges new links(in memory) with old links(in pmod.lnk)
  The merged results are placed in file "fn", which should linkfile_name
  with a '#' at the end. This '#' file later gets moved to become the 
  link pmod file.
*/
int XrefTable::dump_new_links_to_pmod(genString& fn)
{
  Initialize(XrefTable::dump_new_links_to_pmod );

  if (!symbolIndex_table) return 0;
  msg("Moving link information to \"$1\" ...", normal_sev) << (char const *)fn << eom;

  int ret_val = 0;
  int sym_itab_sz = (symbolIndex_table) ? symbolIndex_table->size() : 0;

  //get all the link offsets into nlink_offsets array for calculating
  //new offsets, offset of -1 implies the symbol has been newly created
  //and the new offset has to be calculated 
  int *nlink_offsets = new int[sym_itab_sz];
  for (int k = 0; k < sym_itab_sz; ++k)
    nlink_offsets[k] = *orig_link_offsets[k];

  int nbytes = sizeof(linkHeaderInfo);
#ifdef _WIN32
  DWORD bytes_written;
  HANDLE fn_handle = open_timeout(fn, O_RDWR|O_CREAT, mode);
  if (fn_handle == INVALID_HANDLE_VALUE || !WriteFile(fn_handle, (caddr_t) link_header, nbytes, &bytes_written, NULL))
    ret_val = 1;
  if(bytes_written != nbytes)
    ret_val = 1;
#else
  int fn_handle = open_timeout(fn, O_RDWR|O_CREAT, mode);
  if (fn_handle < 0 || OSapi_write (fn_handle, (caddr_t) link_header, nbytes) < nbytes)
    ret_val = 1;
#endif
  if (!ret_val && setx(fn)) // set x permission for fn
      ret_val = 1;
      
  if (!ret_val) {
    int nl, total_offset = 0, links_copied, cur_offset;
    int cur_links;
    SymbolIndex sip;
    Link* start_lp;
    //i starts at 1 since first entry in symbolIndex_table is dummy
    for (int i=1; i<sym_itab_sz && !ret_val; i++)
      {
	cur_offset = *orig_link_offsets[i];
	// copy new links
	nbytes = sizeof(Link);
	sip = *((*symbolIndex_table)[i]);
	nl = (sip) ? sip->size() : 0;
	links_copied = 0;
	if (nl > 0) {
          int j=0;
          while(j<nl && !ret_val) {
	    start_lp = (*sip)[j];
            cur_links = 0;
            Link *lp;
	    while (j<nl) {
              lp = (*sip)[j++];
              if (lp->lt() == lt_junk) break;
              cur_links++;
            }
            if (cur_links > 0) {
              unsigned int bytes_to_copy = nbytes*cur_links;
#ifdef _WIN32
	      if (!WriteFile(fn_handle, start_lp, bytes_to_copy, &bytes_written, NULL)) {
	        ret_val = 1;
              }
	      if(bytes_written != bytes_to_copy) {
	        ret_val = 1;
              }
#else
	      if (OSapi_write(fn_handle,(char const *)start_lp,bytes_to_copy) < bytes_to_copy) {
	        ret_val = 1;
              }
#endif	  
	      links_copied+=cur_links;
            }
	  }
	}
	//update offsets to accomodate new links
	if (links_copied)
	  update_offset_array(1,nlink_offsets,i+1, sym_itab_sz, links_copied*sizeof(Link));

	int num_bytes=0;
	if (cur_offset < 0) //if the current symbol is new
	  nlink_offsets[i] = total_offset;
	else {           //else copy already existing links from pmod.lin
          char const *lcp = (char const *)get_link_addr() + cur_offset;
	  if (i+1 < sym_itab_sz) 
	    num_bytes = *orig_link_offsets[i+1] - cur_offset;
	  // we have reached links of last symbol in the pmod
	  if (num_bytes <= 0 && (i+1 == sym_itab_sz || (*orig_link_offsets[i+1] < 0)))
	    num_bytes = saved_next_loffset - cur_offset;
	  if (num_bytes > 0) {
#ifdef _WIN32
	    if (!WriteFile(fn_handle, lcp, num_bytes, &bytes_written, NULL))
	      ret_val = 1;
	    if(bytes_written != num_bytes)
	      ret_val = 1;
#else
	    if (OSapi_write(fn_handle,lcp,num_bytes) < num_bytes)
	      ret_val = 1;
#endif
	    total_offset += num_bytes;
	  }
	}
	total_offset += links_copied*sizeof(Link);
      }
    header->next_offset_link = htonl(total_offset);
    update_link_offsets_of_syms(nlink_offsets);
  }
  fn_handle = close_handle(fn_handle);

  delete [] nlink_offsets;
  msg("dump_new_links_done:XrefTable.h.C", normal_sev) << eom;
  return ret_val;
}

XrefTable::~XrefTable()
{
  Initialize(XrefTable::~XrefTable);
  remove_from_pmod_array();
  if (header == 0) return;
  my_Xref = 0;     
  int need_to_unmap = 1;
  delete backup;
  deleteHashTable();
  int snext = ntohl(header->next_offset_sym); // remember after unmap
  int inext = ntohl(header->next_offset_ind);
  int lnext = ntohl(header->next_offset_link);
  int need_to_mv_pmod = (units_are_backup != 2) && need_to_mv_pmod_to_perm() && ok_to_delete_old_pmod;

#ifndef irix6
  // merge new links and old links, for irix dump_new_links_to_pmod 
  // will be called from mv_pmod_to_perm function
  genString link_file_p = linkfile_name;
  link_file_p += "#";
  if (link_is_writable && need_to_mv_pmod) {
    if (copy_link_mmap_to(link_file_p)) {
      genString err_msg;
      err_msg.printf("error while copying new links to: %s",link_file_p.str());
      perror((char *)err_msg);
    }
  }
  
  if (ind_header) {
      int err1  = pset_munmap((caddr_t)ind_header, ind_map_size);
      if (err1 == -1) perror("Error detected on unmap of ind file");
  }
  if (sym_header) {
      int err   = pset_munmap((caddr_t)sym_header, sym_map_size);
      if (err) perror("Error detected on unmap of sym file");
  }
  if (link_header) {
    int err2 = pset_munmap((caddr_t)link_header, link_map_size);
    if (err2) perror("Error detected on unmap of link file");
  }
#endif
  
  if (units_are_backup != 2) {
      if (!valid_handle(unit))
      {
	  unit = open_timeout (symbackup, O_RDWR|O_EXCL, mode);
	  int sym_x = 0;
	  
	  if(valid_handle(unit))
	      //
	      // Force x permission on newly created pmods (Smit\'s request, bug 9305).
	      //
	      sym_x = setx(symbackup);
      }
      if (!valid_handle(unit1))
      {
	  unit1 = open_timeout (indbackup, O_RDWR|O_EXCL, mode);
	  int ind_x = 0;
	  if (valid_handle(unit1))
	      ind_x = setx(indbackup);
      }
      if (!valid_handle(unit2))
      {
	  unit2 = open_timeout (linkbackup, O_RDWR|O_EXCL, mode);
	  int link_x = 0;
	  if (valid_handle(unit2))
	      link_x = setx(linkbackup);
      }
  }
  
  if (units_are_backup) {
      need_to_unmap = 0;		// dont bother unmapping backup files
      // and real files not mapped
      // OSapi_close the backup files
      //
      // do not overwrite permanent pmod in case of errors
      //
      int do_move = 0;
      if (need_to_mv_pmod && !call_from_put()) {
	  if (valid_handle(unit) && sym_is_writable && snext >= 0) {
	      if (truncate_file(unit, snext + sizeof(symHeaderInfo))) {
		  header=0;
		  perror("Error truncating pmod.sym file ");
		  error_truncating_file((char *)symbackup); // fatal error message
	      }
	  }
	  if (valid_handle(unit1) && ind_is_writable && inext >= 0) {
	      if (truncate_file(unit1, inext + sizeof(indHeaderInfo))) {
		  header=0;
		  perror("Error truncating pmod.ind file ");
		  error_truncating_file((char *)indbackup); // fatal error message
	      }
	  }
	  if (valid_handle(unit2) && link_is_writable && lnext >= 0) {
	      if (truncate_file(unit2, lnext + sizeof(linkHeaderInfo))) {
		  header=0;
		  perror("Error truncating pmod.lin file ");
		  error_truncating_file((char *)linkbackup); // fatal error message
		}
	    }
	  do_move = 1;
      }
      else {
	  if (symbackup != (char *)0)
	      OSapi_unlink((char *)symbackup);
	  if (indbackup != (char *)0)
	      OSapi_unlink((char *)indbackup);
	  if (linkbackup != (char *)0)
	      OSapi_unlink((char *)linkbackup);
	  if (units_are_backup != 2) {
#ifdef _WIN32
	      HANDLE un = open_timeout(symfile_name, O_RDWR|O_EXCL, mode);
#else
	      int un = open_timeout (symfile_name, O_RDWR|O_EXCL, mode);
#endif
	      int sym_x = 0;
	      if (valid_handle(un))
		  sym_x = setx(symfile_name);
#ifdef _WIN32
	      HANDLE un1 = open_timeout (indfile_name, O_RDWR|O_EXCL, mode);
#else
	      int un1 = open_timeout (indfile_name, O_RDWR|O_EXCL, mode);
#endif
	      int ind_x = 0;
	      if (valid_handle(un1))
		  ind_x = setx(indfile_name);
#ifdef _WIN32
	      HANDLE un2 = open_timeout (linkfile_name, O_RDWR|O_EXCL, mode);
#else
	      int un2 = open_timeout (linkfile_name, O_RDWR|O_EXCL, mode);
#endif
	      int link_x = 0;
	      if (valid_handle(un2))
		  link_x = setx(linkfile_name);
	      if (!valid_handle(un) || !valid_handle(un1) || !valid_handle(un2) || sym_x || ind_x || link_x) return;
	      int siz = file_size(un);
	      if (siz == 0) return;
	      symHeaderInfo shi;
	      indHeaderInfo ihi;
	      linkHeaderInfo lhi;
#ifdef _WIN32
	      SetFilePointer(un, 0, NULL, FILE_BEGIN);
	      DWORD bytes_readed;
	      ReadFile(un, &shi, sizeof(symHeaderInfo), &bytes_readed, NULL);

	      SetFilePointer(un1, 0, NULL, FILE_BEGIN);
	      ReadFile(un1, &ihi, sizeof(indHeaderInfo), &bytes_readed, NULL);

	      SetFilePointer(un2, 0, NULL, FILE_BEGIN);
	      ReadFile(un2, &lhi, sizeof(linkHeaderInfo), &bytes_readed, NULL);
#else
	      OSapi_lseek(un, 0, 0);
	      OSapi_read(un, (char *)&shi, sizeof(symHeaderInfo));

	      OSapi_lseek(un1, 0, 0);
	      OSapi_read(un1, (char *)&ihi, sizeof(indHeaderInfo));

	      OSapi_lseek(un2, 0, 0);
	      OSapi_read(un2, (char *)&lhi, sizeof(linkHeaderInfo));
#endif
	      if ((ntohl(shi.oldheader.next_offset_sym) + ntohl(shi.offset_to_data) < siz)) {
		  if (truncate_file(un, ntohl(shi.offset_to_data) + ntohl(shi.oldheader.next_offset_sym))) {
		      header=0;
		      error_truncating_file(symfile_name); // fatal error message
		  }
		  if (truncate_file(un1, ntohl(ihi.offset_to_data) + ntohl(shi.oldheader.next_offset_ind))) {
		      header=0;
		      error_truncating_file(indfile_name); // fatal error message
		  }
		  if (truncate_file(un2, ntohl(lhi.offset_to_data) + ntohl(shi.oldheader.next_offset_link))) {
		      header=0;
		      error_truncating_file(linkfile_name); // fatal error message
		  }
	      }
	      un = close_handle(un);
	      un1 = close_handle(un1);
	      un2 = close_handle(un2);
	  }
      }
      unit1 = close_handle(unit1);
      unit = close_handle(unit);
      unit2 = close_handle(unit2);

      if(do_move)
	  mv_pmod_to_perm();
#ifdef irix6
      if (ind_header) {
	int err1  = pset_munmap((caddr_t)ind_header, ind_map_size);
	if (err1 == -1) perror("Error detected on unmap of ind file");
      }
      if (sym_header) {
	int err   = pset_munmap((caddr_t)sym_header, sym_map_size);
	if (err) perror("Error detected on unmap of sym file");
      }
      if (link_header) {
	int err2 = pset_munmap((caddr_t)link_header, link_map_size);
	if (err2) perror("Error detected on unmap of link file");
      }
#endif
  }
  else { //  must be !units_are_backup
#ifdef irix6
    if (ind_header) {
      int err1  = pset_munmap((caddr_t)ind_header, ind_map_size);
      if (err1 == -1) perror("Error detected on unmap of ind file");
    }
    if (sym_header) {
      int err   = pset_munmap((caddr_t)sym_header, sym_map_size);
      if (err) perror("Error detected on unmap of sym file");
    }
    if (link_header) {
      int err2 = pset_munmap((caddr_t)link_header, link_map_size);
      if (err2) perror("Error detected on unmap of link file");
    }
#endif
      if (valid_handle(unit) && sym_is_writable && snext >= 0) {
	  if (truncate_file(unit, snext + sizeof(symHeaderInfo))) {
	      header=0;
	      perror("Error truncating pmod.sym file ");
	      error_truncating_file((char *)symfile_name); // fatal error message
	  }
      }
      if (valid_handle(unit1) && ind_is_writable && inext >= 0) {
	  if (truncate_file(unit1, inext + sizeof(indHeaderInfo))) {
	      header=0;
	      perror("Error truncating pmod.ind file ");
	      error_truncating_file((char *)indfile_name); // fatal error message
	  }
      }
      
      if (valid_handle(unit2) && link_is_writable && lnext >= 0) {
	  if (truncate_file(unit2, lnext + sizeof(linkHeaderInfo))) {
	      header=0;
	      perror("Error truncating pmod.lin file ");
	      error_truncating_file((char *)linkfile_name); // fatal error message
	  }
      }
      close_handle(unit);
      close_handle(unit1);
      close_handle(unit2);
      if(delete_in_destructor) {
	  OSapi_unlink((char *)symfile_name);      
	  OSapi_unlink((char *)indfile_name);
	  OSapi_unlink((char *)linkfile_name);
      }
  }
  
  if (symbolIndex_table) {
    for(int j=0; j < symbolIndex_table->size(); j++)
      delete (*((*symbolIndex_table)[j]));
    delete symbolIndex_table;
  }
  
  xref_attribute_invalidate();
}

int XrefTable::copy_ind_mmap_to(genString &fn)
{
    Initialize(XrefTable::copy_ind_mmap_to);
    
#ifdef _WIN32
    HANDLE munit = open_timeout (fn, O_RDWR|O_CREAT|O_EXCL, mode);
#else
    int munit = open_timeout (fn, O_RDWR|O_CREAT|O_EXCL, mode);
#endif
    int ind_x = 0;
    if (valid_handle(munit))
	ind_x = setx(fn);
    if (!valid_handle(munit) || ind_x) {
	return 1;
    }
#ifdef _WIN32
    SetFilePointer(munit, 0, NULL, FILE_BEGIN);
#else    
    OSapi_lseek (munit, 0, 0);
#endif
    int nbytes  = ntohl(header->next_offset_ind) + ntohl(ind_header->offset_to_data);
    int ret_val = 0;
#ifdef _WIN32
    DWORD bytes_written;
    if (!WriteFile(munit, (caddr_t) ind_header, nbytes, &bytes_written, NULL))
	ret_val = 1;
    if(bytes_written != nbytes)
	ret_val = 1;
#else
    if (OSapi_write (munit, (caddr_t) ind_header, nbytes) < nbytes)
	ret_val = 1;
#endif
    close_handle(munit);
    return ret_val;
}

int XrefTable::copy_sym_mmap_to(genString &fn)
{
    Initialize(XrefTable::copy_sym_mmap_to);
    
#ifdef _WIN32    
    HANDLE munit = open_timeout (fn, O_RDWR|O_CREAT|O_EXCL, mode);
#else
    int munit = open_timeout (fn, O_RDWR|O_CREAT|O_EXCL, mode);
#endif
    int sym_x = 0;
    if (valid_handle(munit))
	sym_x = setx(fn);
    if (!valid_handle(munit) < 0 || sym_x) {
	return 1;
    }
#ifdef _WIN32
    SetFilePointer(munit, 0, NULL, FILE_BEGIN);
#else
    OSapi_lseek (munit, 0, 0);
#endif
    int nbytes  = ntohl(header->next_offset_sym) + ntohl(sym_header->offset_to_data);
    int ret_val = 0;
#ifdef _WIN32
    DWORD bytes_written;
    if (!WriteFile(munit, (caddr_t) sym_header, nbytes, &bytes_written, NULL))
	ret_val = 1;
    if(bytes_written != nbytes)
	ret_val = 1;
#else
    if (OSapi_write (munit, (caddr_t) sym_header, nbytes) < nbytes)
	ret_val = 1;
#endif
    close_handle(munit);
    return ret_val;
}

int XrefTable::copy_link_mmap_to(genString &fn)
{
  Initialize(XrefTable::copy_link_mmap_to);

  /* If symbolIndex_table exists it means a new symbol or new link has
     been created in this session so links in symbolIndex_table and links 
     in old pmod.lnk file have to be merged. 
     If symbolIndex_table does not exist just copy the whole link file
  */
  int ret_val = 0;
  OSapi_unlink(fn);
  if (symbolIndex_table) { 
    if (dump_new_links_to_pmod(fn)) {
      perror("dump_new_links_to_pmod");
      error_message_box("error while dumping new links to \"%s\".", fn, NO, 91);
    }
  }
  else {
#ifdef _WIN32
    HANDLE munit = open_timeout (fn, O_RDWR|O_CREAT|O_EXCL, mode);
#else
    int munit = open_timeout (fn, O_RDWR|O_CREAT|O_EXCL, mode);
#endif
    int link_x = 0;
    if (valid_handle(munit))
      link_x = setx(fn);
    if (!valid_handle(munit) < 0 || link_x) {
      return 1;
    }
#ifdef _WIN32
    SetFilePointer(munit, 0, NULL, FILE_BEGIN);
#else
    OSapi_lseek (munit, 0, 0);
#endif
    int nbytes  = ntohl(header->next_offset_link) + ntohl(link_header->offset_to_data);
    
#ifdef _WIN32
    DWORD bytes_written;
    if (!WriteFile(munit, (caddr_t) link_header, nbytes, &bytes_written, NULL))
      ret_val = 1;
    if(bytes_written != nbytes)
      ret_val = 1;
#else
    if (OSapi_write (munit, (caddr_t) link_header, nbytes) < nbytes)
      ret_val = 1;
#endif
    close_handle(munit);
  }
  return ret_val;
}

int XrefTable::copy_pmod_to_perm()
//  Move the pmod sym/ind from /usr/tmp to the permanent place
//  nonzero result means we could not successfully move the pmods to the permanent location
{
    Initialize(XrefTable::copy_pmod_to_perm);
    
    int need_to_mv_pmod = (units_are_backup != 2) && need_to_mv_pmod_to_perm();
    if (!need_to_mv_pmod) return 1;
    
    //net effect of this function is to move from tmp directory to perm one
    //    file  symbackup    to   file   symfile_name
    //    file  indbackup    to   file   indfile_name
    
    int err;
    int err2;
    int err3;
    genString command;
    genString symfilet = symfile_name;        // tilda file holds old version
    symfilet += "~";
    genString symfilep = symfile_name;        // # file holds new version in final directory
    symfilep += "#";
    genString indfilet = indfile_name;
    indfilet += "~";
    genString indfilep = indfile_name;
    indfilep += "#";
    genString linkfilet = linkfile_name;
    linkfilet += "~";
    genString linkfilep = linkfile_name;
    linkfilep += "#";

    OSapi_unlink(symfilep);   // unlink # file, in case it already exists
    OSapi_unlink(indfilep);
    OSapi_unlink(linkfilep);
    err3 = copy_link_mmap_to(linkfilep);
    if (err3) {
	msg("Error copying link files to permanent", catastrophe_sev) << eom;
	return 0;
    }
    err = copy_sym_mmap_to(symfilep);
    if (err) {
	msg("Error copying sym files to permanent", catastrophe_sev) << eom;
	return 0;
    }
    err2 = copy_ind_mmap_to(indfilep);
    if (err2) {
	msg("Error moving ind files to permanent", catastrophe_sev) << eom;
	return 0;
    }

    // At this point, all files are in the same directory, so rename should be quick & safe
    err = OSapi_rename(symfile_name, symfilet);  // rename old to tilda
    if (!err) {
	err = OSapi_rename(symfilep, symfile_name);  // rename new to permanent name
	if (!err) {
	    err = OSapi_rename(indfile_name, indfilet);   // rename old .ind to tilda
	    if (!err) {
		err = OSapi_rename(indfilep, indfile_name);   // rename new to permanent name
		if (!err) {
		  err =  OSapi_rename(linkfile_name, linkfilet); //rename old .lin to tilda
		  if (!err)
		    err = OSapi_rename(linkfilep, linkfile_name); //rename new to parmanent name
		}
	    }
	}
    }
    if (err) {
	msg("Error copying sym/ind/lin files to permanent: $1", catastrophe_sev) << symfile_name.str() << eom;
	return 0;
    }
    OSapi_unlink(symfilet);
    OSapi_unlink(indfilet);
    OSapi_unlink(linkfilet);
    genString crash_file_name = symfile_name;
    strip_suffix(".sym", crash_file_name);
    crash_file_name += ".outdated_pset";
    OSapi_unlink(crash_file_name.str());
    
    return 1;
}


int XrefTable::mv_pmod_to_perm()
//  Move the pmod sym/ind from /usr/tmp to the permanent place
//  nonzero result means we could not successfully move the pmods to the permanent location
{
    Initialize(XrefTable::mv_pmod_to_perm);
  
#ifdef irix6
    return copy_pmod_to_perm();
#else
    //net effect of this function is to move from tmp directory to perm one
    //    file  symbackup    to   file   symfile_name
    //    file  indbackup    to   file   indfile_name
    
    int err;
    int err2;
    genString command;
    genString symfilet = symfile_name;        // tilda file holds old version
    symfilet += "~";
    genString symfilep = symfile_name;        // # file holds new version in final directory
    symfilep += "#";
    genString indfilet = indfile_name;
    indfilet += "~";
    genString indfilep = indfile_name;
    indfilep += "#";
    genString linkfilet = linkfile_name;
    linkfilet += "~";
    genString linkfilep = linkfile_name;
    linkfilep += "#";

    OSapi_unlink(symfilep);   // unlink # file, in case it already exists
    OSapi_unlink(indfilep);


    err = shell_mv (symbackup,symfilep,1); // force mv -f
    if (err) perror("Error moving sym files to permanent");
    err2 = shell_mv (indbackup, indfilep, 1); // force mv -f
    if (err2) perror("Error moving ind files to permanent");

    /* At this point links are either merged or copied to linkfilep.
       We do not need linkbackup file anymore.
    */
    OSapi_unlink(linkbackup);

    if (err | err2) {
	msg("   After fixing the problem, you should manually move:\n") << eom;
	msg("   $1 to $2\n") << (char *)symbackup << eoarg << (char *) symfile_name << eom;
	msg("   $1 to $2\n") << (char *)indbackup << eoarg << (char *) indfile_name << eom;
	return 0;
    }
    
    // At this point, all 4 files are in the same directory, so rename should be quick & safe
    err = OSapi_rename(symfile_name, symfilet);  // rename old to tilda
    int sym_x = 0;
    int ind_x = 0;
    int link_x = 0;
    if (!err) {
	err = OSapi_rename(symfilep, symfile_name);  // rename new to permanent name
	if (!err)
	    sym_x = setx(symfile_name);
	if (!err && !sym_x) {
	    err = OSapi_rename(indfile_name, indfilet);   // rename old .ind to tilda
	    if (!err) {
		err = OSapi_rename(indfilep, indfile_name);   // rename new to permanent name
		if (!err)
		    ind_x = setx(indfile_name);
	    }
	}
	if (!err && !sym_x && !ind_x) {
	  err = OSapi_rename(linkfile_name, linkfilet);   // rename old .lin to tilda
	    if (!err) {
		err = OSapi_rename(linkfilep, linkfile_name);   // rename new to permanent name
		if (!err)
		    link_x = setx(linkfile_name);
	    }
	}
    }
    if (err || sym_x || ind_x || link_x) {
	perror("Error copying sym/ind/lin files to permanent");
	msg("$1", error_sev) << symfile_name.str() << eom;
	return 0;
    }
    OSapi_unlink(symfilet);
    OSapi_unlink(indfilet);
    OSapi_unlink(linkfilet);
    return 1;
#endif
}

// test if there's enough room for xref symbol area to grow without moving
//     return 1 if there's enough room in both sym and ind files
bool XrefTable::is_room(int size)
{ 
    headerInfo* hi = get_header();
    return (ntohl(hi->next_offset_sym) + size <=  sym_map_size)
	&& (ntohl(hi->next_offset_ind) <=  ind_map_size/2 - 20000);
    // true if sym file has at least size bytes, and if ind file is less than half full
}

static uint get_make_room_size()
// fetch the user-chosen minimum size by which a sym file should grow
// Default is 250*1000, but he changes it by having an environment variable
//   PSET_SYMGROW to the desired number of K.
{
    if (!default_make_room_size) {
	int size = 250;
	char const *asize = OSapi_getenv("PSET_SYMGROW");
	if (asize) {
	    size = OSapi_atoi(asize);
	    if (size <10 || size > 10000) {
		msg("Could not convert PSET_SYMGROW value to integer between 100 and 10,000", warning_sev) << eom;
		size = 250;
	    }
	}
	default_make_room_size = 1000 * size;
    }
    return default_make_room_size;
}

bool XrefTable::make_room(int size)
//
//  if there isn't enough room, move the xref symbol area now
//    
{
    Initialize(XrefTable::make_room);
//
// If the pmod is not already backup, do it now. We want to backup every pmod we are 
// writing to and do all changes on backup version. Later XrefTable::~XrefTable will 
// copy the changed (backup) version into permanent, and truncate it in the process.
//
// Before this addition, in certain cases changes to pmod were done on original file,
// not the copy. In addition, no truncation was performed (since truncation is only 
// done on backup version of pmod, in XrefTable::~XrefTable). That led to huge pmods.
//
// Success/failure policy: make_room always returns 1 (success). If it detects failure
// in calls to do_backup or extend_file (the only reason that it can possibly fail), it 
// prints the message associated with the latest system call error (through perror), 
// and then exits paraset. The reason for this is that no functions which call make_room 
// test for its success, so it has to exit if it detects failure (the alternative of changing 
// all those functions, and possibly the functions that call them, is at present impractical). 
//
//                                                                 ljb, 05/15/1995
//
    int retval = 1;
    char const *err_message = NULL;
    if (!units_are_backup)
    {
 	err_message = do_backup();
	if (err_message != NULL)
	{
	    perror(err_message);
	    driver_exit(1);
	}
    }
#ifdef _PSET_DEBUG
    if (force_move()<2 && is_room(size))  return 1;      // if 1, move when needed
#else
    if (is_room(size)) return 1;
#endif

    headerInfo* hi = get_header();

    size = MAX(size,ntohl(hi->next_offset_sym)/4);     // keep at least 25% slack space
    size = MAX(size,get_make_room_size());             // ask for at least xxxk additional
    
    // ask for at least 4*xxxk total             bug6079
    size = MAX(size,4*get_make_room_size()-ntohl(hi->next_offset_sym));


    int needed = ntohl(hi->next_offset_sym) + size - sym_map_size;
    if (needed<0) needed = 0;

    // grow by at least 25% if we\'re gonna grow at all
    if (needed) needed = MAX(needed, (sym_map_size)/4); 

    err_message = extend_file(needed, 0, 1);
    if (err_message != NULL)
    {
	perror(err_message);
	driver_exit(1);
    }
#ifdef _PSET_DEBUG
    is_consistent_hashElmts();
#endif
    return retval;
}

//  


// more generalized function to extend the files
//     by moving the mapped area.  Normally, it doesn't move unless the 
//     appropriate size is nonzero.  But static function 'force_move()' can be
//     used to force it, for testing purposes.
// 
// (note, last parm used be called "movable", but is no longer used)
//
// Returns NULL string pointer for success; otherwise pointer to offending
// file name.
//
char const *XrefTable::extend_file(int symgrow, int indgrow, bool )
{
    Initialize(XrefTable::extend_file);

    char const *retval = NULL;
    char const *sym_file_name = symfile_name;
    char const *ind_file_name = indfile_name;
    if (units_are_backup) {
	sym_file_name = symbackup;
	ind_file_name = indbackup;
    }

    indgrow =  2 * ntohl(header->next_offset_ind) - ind_map_size;
    if (indgrow<0) indgrow = 0;

    // temporary fix to force rapid growth
    if (force_move() && ind_map_size < 500000)
        indgrow = MAX(indgrow, ind_map_size/20);  // force it to grow by at least 5 % each time

    bool symgrowtrue = (symgrow!=0);

    if (force_move())		// fake for always "moving" table, even if no grow
        symgrowtrue = 1;

    int sym_request_size = sym_map_size + ntohl(sym_header->offset_to_data) + symgrow;
    int ind_request_size = ind_map_size + ntohl(ind_header->offset_to_data) + indgrow;
    int was_ratio = (100*ntohl(header->next_offset_ind)/ntohl(header->next_offset_sym+1));  // percentage of ind/sym
    int ratio = MAX(100,was_ratio);
    int minindsize = (sym_request_size * ratio / 100);
    if (minindsize > ind_request_size) {
        ind_request_size = minindsize;
        indgrow = 1;          // note:  after this, this is only used as a boolean flag
    }

    // unmap the files first, so that certain architectures do not get upset 
    // when we call OSapi_ftruncate or open_timeout.

    if (header) {
        if (symgrowtrue) {
            pset_munmap((caddr_t)sym_header, sym_map_size);
            if (!valid_handle(unit)) {
	       unit = open_timeout(sym_file_name, O_RDWR|O_EXCL, mode );
	       retval = (unit < 0) ? sym_file_name : NULL;
            }
            if (retval == NULL) {
		int err = truncate_file(unit, sym_request_size);
		if (err) {
		    header=0;
		    error_truncating_file(sym_file_name);
		}
            }
        }
        if (indgrow) {
	    pset_munmap((caddr_t)ind_header, ind_map_size);
            if (!valid_handle(unit1)) {
	        unit1 = open_timeout(ind_file_name,O_RDWR|O_EXCL, mode );
	        retval = (unit1 < 0) ? ind_file_name : NULL;
            }
            if (retval == NULL) {
		int err1 = truncate_file(unit1, ind_request_size);
		if (err1) {
		    header=0;
		    error_truncating_file(ind_file_name);
		}
            }
        }
    }

    if (retval == NULL) {
	char *addr= (caddr_t) -1;
	char *addr1=(caddr_t) -1;
	if (symgrowtrue) {
	    // allocate room for first 3 xrefTable's to grow dynamically
	    static const int DUMMY_ARRAY_SIZE = 3;
	    static int dummyoffset = 0;
#ifdef _WIN32
	    static HANDLE dummyhandle = INVALID_HANDLE_VALUE;
#else
	    static int dummyhandle = -1;
#endif
	    static caddr_t dummyaddr = 0;
	    static int dummysize = (4096-32) * OS_dependent::getpagesize();
	    // apparently system reserves extra 128k or 32 pages between mappings
	    static caddr_t dummyadd[DUMMY_ARRAY_SIZE];
	    static int dummysiz[DUMMY_ARRAY_SIZE];
	    static int dummyoff[DUMMY_ARRAY_SIZE];
	    static char dummy_name[DUMMY_ARRAY_SIZE][8];
	    static XrefTable* whichXrefTable[DUMMY_ARRAY_SIZE];
	    static int nextdummy=-1;

	    // only do this part the first time through
	    if (force_move() && nextdummy==-1) {
		
		genTmpfile tmpfile("epmod");
#ifdef _WIN32
		dummyhandle = open_timeout((char const *)tmpfile.name(), O_RDWR|O_CREAT, mode);
#else
		dummyhandle = tmpfile.open();
		Assert(dummyhandle >= 0);
		tmpfile.unlink();
#endif		
		for (int i=0;i<DUMMY_ARRAY_SIZE; i++) {
		    strcpy(dummy_name[i], "dummyX");
		    dummy_name[i][5] = (char) (i+ 0x30);
		    dummysiz[i]= dummysize;
#ifdef _WIN32
		    dummyadd[i] =
			pset_mmap(dummy_name[i], 0, dummysiz[i], PAGE_READWRITE, 0,
				  dummyhandle, 0);
#else
		    dummyadd[i] =
			pset_mmap(dummy_name[i], 0, dummysiz[i], PROT_READ|PROT_WRITE, MAP_SHARED,
				  dummyhandle, 0);
#endif
		    if (_SHOW_MOVE) {
			msg("(dummy $1 is at $2 $3)\n") << i << eoarg << (void*)dummyadd[i] << eoarg << (void*)(dummyadd[i] + dummysiz[i] - 1) << eom;
		    }
		    dummyoff[i]=0;
		    whichXrefTable[i]=0;
		}
		nextdummy=0;
	    }
	    int mydummy=-1;
// Next section is only active if  force_move() is true, for debugging
	    if (force_move()) {
		// check to see whether we've already mapped this xrefTable
		for (int i=0; i<DUMMY_ARRAY_SIZE; i++) {
		    if (whichXrefTable[i] == this) {
			mydummy = i;
			break;
		    }
		}
		// if not, see whether there's any more room in the table
		if (mydummy==-1 && nextdummy<DUMMY_ARRAY_SIZE) {
		    mydummy=nextdummy++;
		    whichXrefTable[mydummy]=this;
		    if (_SHOW_MOVE) {
			msg("(Allocated yet another dummy area for $1)\n") << (char const *)sym_file_name << eom;
			msg("(Buffer $1 is at $2 $3)\n") << mydummy << eoarg << (void *) dummyadd[mydummy] << eoarg << (void*)(dummyadd[mydummy] + dummysiz[mydummy] - 1) << eom;
		    }
		}
	    }

	    if (force_move() && mydummy>=0 ) {
		dummyoff[mydummy] += 256*OS_dependent::getpagesize();
		if ( dummyoff[mydummy]+sym_request_size > dummysiz[mydummy])
		    dummyoff[mydummy] = 0;
		//	    pset_munmap((caddr_t)sym_header, sym_map_size);
#ifdef _WIN32
		pset_mmap(dummy_name[mydummy], dummyadd[mydummy], dummysiz[mydummy], PAGE_READWRITE, 0,
			  dummyhandle,0);
#else
		pset_mmap(dummy_name[mydummy], dummyadd[mydummy], dummysiz[mydummy], PROT_READ|PROT_WRITE,
			  MAP_SHARED|MAP_FIXED, dummyhandle,0);
#endif
		sym_map_size = sym_request_size;
#ifdef _WIN32
		addr = pset_mmap(sym_file_name, dummyadd[mydummy]+dummyoff[mydummy], sym_map_size,
				 PAGE_READWRITE, 0,
				 unit, 0 );
#else
		addr = pset_mmap(sym_file_name, dummyadd[mydummy]+dummyoff[mydummy], sym_map_size,
				 PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED,
				 unit, 0 );
#endif
		if (_SHOW_MOVE) {
		    msg("(Moving $1 $2\n") << mydummy << eoarg << (char const *)sym_file_name << eom;
		    msg("    from $1 to $2-$3)\n") << (void*)sym_header << eoarg << (void*)addr << eoarg << (void*)(addr + sym_request_size - 1) << eom;
		}
	    }
// This is "normal" logic, which lets Unix decide where this map will move to
	    else {
//	    pset_munmap((caddr_t)sym_header, sym_map_size);
		sym_map_size = sym_request_size;
#ifdef _WIN32
		addr = pset_mmap(sym_file_name, 0, sym_map_size,
				 PAGE_READWRITE, 0,
				 unit, 0 );
#else
		addr = pset_mmap(sym_file_name, 0, sym_map_size,
				 PROT_READ|PROT_WRITE, MAP_SHARED,
				 unit, 0 );
#endif
	    }
	    
	    if (addr == (char *)-1) {
		error_message_box("Unable to map %s: %s", sym_file_name, YES, 91); // fatal error
	    } else {
		sym_header = (symHeaderInfo*)addr;
		sym_addr = (char *) (addr + ntohl(sym_header->offset_to_data));
		
		// must change header *first*, or get a segmentation fault on other changes
		header = (headerInfo*) (sym_addr-sizeof(headerInfo));
		header->sym_file_size = htonl(sym_request_size - ntohl(sym_header->offset_to_data));
	    }
	}
//    if (force_move())
//	cerr << "ind address is " << (void*) ind_addr << endl;
	if (indgrow) {
//	    pset_munmap((caddr_t)ind_header, ind_map_size);
#ifdef _WIN32
	    addr1 = pset_mmap(ind_file_name, 0, ind_request_size, 
			      PAGE_READWRITE, 0,
			      unit1, 0 );
#else
	    addr1 = pset_mmap(ind_file_name, 0, ind_request_size, 
			      PROT_READ|PROT_WRITE, MAP_SHARED,
			      unit1, 0 );
#endif	    
	    
#ifdef _PSET_DEBUG
	    if (force_move() && _SHOW_MOVE)
		cerr << "(Moving " << ind_file_name << " from " << (void*)ind_header
		     << " to " << (void*)addr1 << "-" << (void*)(addr1 + ind_request_size -1) << ")" << endl;
#endif
	}
	IF ((symgrowtrue && int(addr) == -1) || (indgrow && int(addr1) == -1)) {
	    // error:  requested increase did not happen, and we may have unmapped
	    //   one or both of the mappable areas -- no way to recover ???
	    error_message_box("Pmod file %s could not be extended: %s", sym_file_name, NO, 92); // fatal error
	}
	if (indgrow) {
	    ind_map_size = ind_request_size;
	    indHeaderInfo* oldaddress = ind_header;
	    ind_header = (indHeaderInfo*) addr1;
	    ind_addr = (xrefSymbol*) (addr1 + ntohl(ind_header->offset_to_data));
	    header->ind_file_size = htonl(ind_request_size - ntohl(ind_header->offset_to_data));
	    
	    // change all the hashElmt that still point into this ind file mapped area
	    int distance_moved = (char *) addr1 - (char *) oldaddress;
	    int h;
	    for (h=0; h < ntohl(header->table_max); h++) {
		hashElmt * hel = table[h];
		while (hel) {
		    hel->el = (xrefSymbol*)  ( ((char *) hel->el) + distance_moved);
		    hel = hel->next;
		}
	    }
	}
    }
    unit  = close_handle(unit);
    unit1 = close_handle(unit1);

    xref_attribute_invalidate();

    return(retval);
}


extern "C" void gen_stack_prt(int level);

void XrefTable::add_symbols(symbolArr& arr, const ddSelector& selector)
// get all symbols matching the selector 
// updated to use field want_used of ddSelector
// (this one is not implemented elsewhere)    
{
  Initialize(XrefTable::add_symbols);

  Xref* xr = get_Xref();
  uint noi = ntohl(header->next_offset_ind)/SIZEOF_XREFSYM;

  for (int i = 0; i < noi; ++i)
  {
    if (ParaCancel::is_cancelled()) break;

    xrefSymbol* sym = get_ind_addr() + i;
    if(selector.selects(sym->get_kind())) {

      // if selector.want_used is true, take only symbols without definitions
     // instead of default (only symbols with defs)
       int add_to_arr = !selector.want_used;
	if (sym->get_has_def_file(this) == 0)
	  add_to_arr = !add_to_arr;
	else if (sym->get_kind()==DD_MACRO) {
	  xrefSymbol* def_file_sym = sym->get_def_file_priv(this);
	  XrefTable* Xr_fs = (def_file_sym) ? def_file_sym->get_Xref()->get_lxref():0;
	  if (!def_file_sym || def_file_sym->get_has_def_file(Xr_fs) == 0) 
	    add_to_arr = !add_to_arr;
	}

      if(add_to_arr) {
	symbolPtr symp(xr, sym);
	arr.insert_last(symp);
//	cout << "add:" << sym->get_name();
      }
//      else cout << "noadd:" << sym->get_name();
    }
  }
}


/* just look at string as a number of base 128, and mod MAX_HASH which is a prime
   and not near power of 2.
   This is used for Xref table
   return a hash number < 0x000FFFFF, caller need to % header->table_max
*/

int XrefTable::Mhash(char const *s1)
{Initialize(XrefTable::Mhash );

//  char *ss = (char *) malloc(strlen(s1) + 1);
//  dd_get_short_name(s1, ss);
//  char *s = ss;
  char const *s = s1;
  unsigned int pow = 1;
  unsigned int val = 0;
  while (*s) {
    if (*s != ' ') {
      if (val > 0x00FFFFFF)
	val %= ntohl(header->table_max);
      if (pow > 0x00FFFFFF)
	pow %= ntohl(header->table_max);      
      val += ((unsigned char)*s) * pow;
      pow <<= 7; // radix 128
    }
    s++;
  }
 /* val %= ntohl(header->table_max); */
 return (val);
}

xrefSymbol* XrefTable::insert_xrefSymbol()
{ 
  Initialize(XrefTable::insert_xrefSymbol);
  xrefSymbol* si = (xrefSymbol*) (((char const *)ind_addr) +  ntohl(header->next_offset_ind));
  header->next_offset_ind = htonl(ntohl(header->next_offset_ind) + SIZEOF_XREFSYM);
  return si;
}

void XrefTable::insert_xrefSymbol_name(uint nm_off, char const *ns, uint nl)
{
  Initialize(XrefTable::insert_xrefSymbol_name);
  char const *s_add = get_sym_addr();
  if (s_add) strncpy((char *)(s_add+nm_off), ns, nl+1);
}

inline void dump_new_symbol(xrefSymbol *sym, Link *ll, XrefTable* Xr_t)
{
  if (PSET_PRINT_LINK) {  
    msg("$1 $2 last link: ") << ddKind_name(sym->get_kind()) << eoarg << sym->get_name(Xr_t) << eom;
    dump_link(0, 0, ll, "new symbol");
  }
}

// create a new symbol
xrefSymbol* XrefTable::newSymbol(ddKind kind, int nl, char const *ns)

{
  Initialize(newSymbol);
  if (my_Xref && (my_Xref->get_home_flag() == 0 || my_Xref->is_project_writable() == 0)) return 0;
  set_need_to_mv_pmod_to_perm();    

  uint symbol_index = make_new_symbolIndex(); //make new symbolIndex in symbolIndex_table, value returned is the index of this symbol
  xrefSymbol* sym = insert_xrefSymbol();
  OS_dependent::bzero((char *)sym, SIZEOF_XREFSYM);
  uint h = Mhash(ns);
  uint sym_name_offset = update_sym_addr_offset(nl+1);
  insert_xrefSymbol_name(sym_name_offset, ns, nl);
  sym->init(symbol_index, kind, sym_name_offset, h);
  hashElmt* he = new hashElmt(sym);
  
  sym->make_new_link(symbol_index,this);
  if (header)
    header->entry_cnt = htonl(ntohl(header->entry_cnt) + 1);
  Link* lnk = sym->get_start_link(symbol_index, this);
  if (lnk)
    lnk->make_last_link();
  if (header)
    h %= ntohl(header->table_max); // modulo table_max to get good index
  // set up link for hash table, insert at the begining
  if (table[h])
    table[h]->prev = he;
  he->next = table[h];
  he->prev = NULL;
  table[h] = he;
#ifdef _PSET_DEBUG
  //  is_consistent_hashElmt(he);
#endif
  
  if (header)
    if (ntohl(header->entry_cnt) >  ntohl(header->table_max))
      rehash_xref();
  last_Xref = get_Xref();
  // notify, but not during a save operation!!!!!!
  if (!SharedXref::saving()) {
    xref_notifier_report(1, sym);
  }
  if (lnk)
    dump_new_symbol(sym, lnk, this);
  msync_xref();  
  return sym;
}

//------------------- addSymbol and addLink cli commands implementation

xrefSymbol* addSymbolCmd_impl( ddKind kind, char const *sym_str )
{
  xrefSymbol* new_sym = 0;
  projNode* home_project = projNode::get_home_proj();

  if ( kind != DD_UNKNOWN && home_project && sym_str && *sym_str )
  {
    XrefTable* xrt = 0;
    Xref* xr = home_project->get_xref();
    if ( xr )
      xrt = (XrefTable*)(xr->get_lxref());
    if ( xrt ) {
        new_sym = xrt->insert_symbol( kind, strlen(sym_str), (char *)sym_str );
    }
  }
 return new_sym;
}

xrefSymbol* addLinkCmd_impl( linkType lt, symbolPtr sym1, symbolPtr sym2 )
{
  symbolPtr xsym1 = sym1.get_xrefSymbol();

  xrefSymbol* new_sym = 0;
  projNode* home_project = projNode::get_home_proj();

  if ( home_project  )
  {
    XrefTable* xrt = 0;
    Xref* xr = home_project->get_xref();
    if ( xr )
      xrt = (XrefTable*)(xr->get_lxref());
    if ( xrt ) {
      xrefSymbol*xs = sym2.operator->();
      xsym1->add_link(lt, xs, 1, xrt);
    }
  }
 return new_sym;
}
//------------------------

void XrefTable::is_consistent_hashElmts()
{
    Initialize(XrefTable::is_consistent_hashElmts);
    struct hashElmt* xe, * new_xe;
    int num_elmts=0;
    for(int i = 0; i < ntohl(header->table_max); ++i) {
	xe = table[i];
	while (xe) {
	    num_elmts++;
	    new_xe = xe->next;
	    IF(new_xe && new_xe->prev != xe) 
		{;}
#ifdef _PSET_DEBUG
	    is_consistent_hashElmt(xe);
#endif
	    IF((ntohl(xe->el->hash_val) % ntohl(header->table_max)) != i)
	    {;}
	    IF(xe->el < get_ind_addr() || xe->el > (xrefSymbol*)((char *)get_ind_addr() + ntohl(header->next_offset_ind)-SIZEOF_XREFSYM))
	    {;}
	    
	    xe = new_xe;
	}
    }
    IF (num_elmts != ntohl(header->entry_cnt))
      {;}
}

#ifdef  _PSET_DEBUG
void XrefTable::is_consistent_hashElmt(hashElmt* xe)
{
    Initialize(XrefTable::is_consistent_hashElmt);
    if (xe->el < ind_addr || xe->el > (xrefSymbol*)(char *)ind_addr + ntohl(header->next_offset_ind) - SIZEOF_XREFSYM) {
	IF(1) cerr << "Inconsistent hashElmt in " << (char const *) indfile_name << endl;
	    ;
    }
}
#endif

void XrefTable::rehash_xref()
{
  Initialize(XrefTable::rehash_xref);

  hashElmt **new_table=(hashElmt**)new 
                       char[(ntohl(header->table_max) * 2 + 1) * sizeof(hashElmt*)];
  hashElmt **tmp_table = table;
  hashElmt *new_xe;
  table = new_table;
  int tmp_table_max = ntohl(header->table_max);
  header->table_max = htonl(ntohl(header->table_max) * 2 + 1);
  hashElmt* xe;
  for (int j = 0; j < ntohl(header->table_max); ++j)
    table[j] = NULL;
  for(int i = 0; i < tmp_table_max; ++i) {
    xe = tmp_table[i];
    while (xe) {
      new_xe = xe->next;
      re_insert(xe);
#ifdef _PSET_DEBUG
//      is_consistent_hashElmt(xe);
#endif
      xe = new_xe;
    }
  }
#ifdef _PSET_DEBUG
  is_consistent_hashElmts();
#endif
  delete tmp_table;
}

void XrefTable::re_insert(hashElmt* xe)
{
  Initialize(XrefTable::re_insert);

  xrefSymbol* sym = xe->el;
  if (sym == 0) return;
  char const *name1 = sym->get_name(this);
  // if there is a name for this symbol, insert it
#ifdef _PSET_DEBUG
  IF(!name1) {;}
#endif
  if (name1) {
      int h = Mhash(name1);
      sym->set_hash_value(h);
      h %= ntohl(header->table_max);
      if (table[h])
	  table[h]->prev = xe;
      xe->next = table[h];
      xe->prev = NULL;
      table[h] = xe;
  }
}




// if more than one xrefSymbol matches rel, should we delete them
// all?
// if rel is a def, then mark xrefSymbol of type lt_junk
// if not, just remove one of the ref_file link
void XrefTable::deleteSymbol(Relational* rel)

{Initialize(XrefTable::deleteSymbol );

  objArr_Int res;
  int is_def = (is_ddSymbol(rel))?((ddSymbol*)rel)->get_is_def():1;

  xrefSymbol* file_sym = 0;
  xrefSymbol* dfs = 0;
  if (is_def == 0)
    file_sym = lookup_module(((ddSymbol*)rel)->get_file_name());
  else
    dfs = get_def_file_symbol(rel);

  int n = lookup(res, rel, dfs);
  if (n == 0) return;

  for (int i = 0; i < n; ++i) {
    xrefSymbol* sym = get_symbol_from_offset(res[i]);

    // notify, but not during a save operation!!!!!!
    if (!SharedXref::saving()) {
      xref_notifier_report(-1, sym);
    }

    if (is_def) {
      sym->set_kind(lt_junk);
      header->entry_cnt = htonl(ntohl(header->entry_cnt) - 1);
      header->fragment_cnt = htonl(ntohl(header->fragment_cnt) + 1);
    }
    else if (file_sym)
      sym->rm_link(ref_file, file_sym, this);
  }
  return;
}


xrefSymbol* XrefTable::insert_symbol(xrefSymbol* sym, XrefTable* sym_xrt)
{
  Initialize(XrefTable::insert_symbol);
  
  xrefSymbol* dfs = sym->get_has_def_file(sym_xrt) ? sym->get_def_file_priv(sym_xrt) : 0;
  char const *filename = NULL;
  if (dfs)
    filename = dfs->get_name();
  char const *ns = sym->get_name(sym_xrt);
  int nl = strlen(ns); // sym->get_name_length();
  dfs = (filename)?lookup_module(filename):0;

  symbolArr res;
  ddKind kind = sym->get_kind();
  lookup(res, kind, ns, dfs);
  xrefSymbol* new_sym;
  if (res.size() == 0) {
    new_sym = newSymbol(kind, nl, ns);
  }
  else {			// for now just return the first one
    new_sym = res[0]; 
  }
  return new_sym;
}

xrefSymbol* XrefTable::insert_symbol(ddKind kind, int nl, char const *ns)
{
  Initialize(XrefTable::insert_symbol);
  
  symbolArr res;
  lookup(res, kind, ns, 0);
  xrefSymbol* new_sym;
  if (res.size() == 0) {
    new_sym = newSymbol(kind, nl, ns);
  }
  else {			// for now just return the first one
    new_sym = res[0];
  }
  return new_sym;
}


// automatically add ref_file, and def_file links.

static symbolPtr lookup_with_attribute(ddSymbol *dd, symbolArr &res)
  /* dd is a ref instance, lookup for non-definition,
     filter out those with definition */
{
  Initialize(lookup_with_attribute);
  if (res.size() == 0) return NULL_symbolPtr;
  if (dd) {
     char const *obj_def_file = dd->get_def_file();
     if (obj_def_file && dd->get_kind() != DD_SEMTYPE && dd->get_datatype() == 1) {
	// is "cloned" ddElement -- only match if has same def file
	symbolPtr s;
	ForEachS(s, res) {
	   if (s.get_has_def_file()) {
	      symbolPtr dfs = s->get_def_file();
	      if (dfs.xrisnotnull() && strcmp(dfs.get_name(), obj_def_file) == 0) {
		 return s;
	      }
	   }
	}
	return NULL_symbolPtr;
     }
  }
  if (!dd || !(dd->get_kind() == DD_VAR_DECL || dd->get_kind() == DD_FUNC_DECL))
    return res[0];
  
  if (dd->get_from_method())
    return res[0];
  
  symbolPtr sym;
  symbolPtr ret;
  ForEachS(sym, res) {
    if (!(sym.get_has_def_file() && sym->get_attribute(STAT_ATT, 1))){
      ret = sym;
      break;
    }
  }
  if (ret.xrisnull() && dd->is_static()) {
    ret = res[0];
  }
  return ret;
}


xrefSymbol* XrefTable::insert_symbol(Relational* obj, int lk)
{
  Initialize(XrefTable::insert_symbol );

  ddKind kind;
  char const *ns = 0;
  char const *obj_def_file = 0;
  app  *ah = 0;
  ddSymbol* dd = 0;
  xrefSymbol* sym = 0;
  genString gs;
  // The following variable was originally used for macros, but "cloned" ddElements
  // (for relations and groups) also need the same processing, i.e., to create a
  // reference symbol for the file and put "is_defined_in" to that symbol.
  int handle_macro = 0;

  if (is_projNode(obj)) {
    kind = (ddKind) DD_PROJECT;
    ns = obj->get_name();
    obj_def_file = ns;
  } else if (is_app(obj)) {
    kind = (ddKind) DD_MODULE;
    ah = (app*) obj;
    ns = ah->get_filename();
    obj_def_file = ns;
  } else if (is_projModule(obj)) {
    kind = (ddKind) DD_MODULE;
    projModule *mod = (projModule *) obj;
    ns = mod->get_name();
    obj_def_file = ns;
  } else if (is_ddSymbol(obj)) {
    dd = (ddSymbol*) obj;
    kind = dd->get_kind();
    ns = dd->get_ddname();
    obj_def_file = dd->get_def_file();
    if ((kind == DD_MACRO || (kind != DD_SEMTYPE && dd->get_datatype() == 1) || kind == DD_TYPEDEF
                          || kind == DD_ENUM || kind == DD_ENUM_VAL
                          || kind == DD_VAR_DECL
                          || kind == DD_FUNC_DECL
                          || kind == DD_CLASS)
	&& (!dd->get_is_def()) && obj_def_file)
      handle_macro = 1;
  }
  if (ns == 0 || *ns == 0) return 0;
  int nl = strlen(ns);
  xrefSymbol* dfs = (lk == 0 && obj_def_file)?lookup_module(obj_def_file):0;
  if (obj_def_file && *obj_def_file && dfs == 0)
    dfs = newSymbol(DD_MODULE, strlen(obj_def_file), obj_def_file);

  symbolArr res;
  if (lk == 0)
    lookup(res, kind, ns, dfs);
  if (res.size()) {			// for now just return the first one
    if (!dfs) {
      sym = lookup_with_attribute(dd, res);
      if (!sym)
	sym = newSymbol(kind, nl, ns);
    }
    else
      sym = res[0]; 
  }
  else
    sym = newSymbol(kind, nl, ns);

  if (handle_macro && dfs && sym) 
    sym->add_link(is_defined_in, dfs, get_remove_hashPair(), 0);


  return sym;
}

xrefSymbol* XrefTable::find_symbol(Relational* obj)
{
  Initialize(XrefTable::insert_symbol );

  char const *ns = 0;
  ddKind kind = DD_UNKNOWN;
  xrefSymbol *dfs = 0;

  char const *obj_def_file = 0;
  app* ah = 0;
  ddSymbol* dd = 0;
  xrefSymbol* sym = 0;
  if (is_app(obj)) {
    kind = (ddKind) DD_MODULE;
    ah = (app*) obj;
    ns = ah->get_filename();
    obj_def_file = ns;
  }
  else if (is_ddSymbol(obj)) {
    dd = (ddSymbol*) obj;
    kind = dd->get_kind();
    ns = dd->get_ddname();
    obj_def_file = dd->get_def_file();
  }
  if (ns == 0 || *ns == 0) return 0;

  dfs = (obj_def_file)?lookup_module(obj_def_file):0;
/*  if (kind == DD_MODULE) return dfs; */

  symbolArr res;
  lookup(res, kind, ns, dfs);
  if (res.size()) {
    if (!dfs) {
      sym = lookup_with_attribute(dd, res);
    }
    else 
      sym = res[0]; 
  }

  return sym;
}


// create a symbol if not exist yet
xrefSymbol* XrefTable::get_symbol(Relational* obj)
{
  Initialize(XrefTable::get_symbol );

  xrefSymbol* sym =0;// (is_ddElement(obj))?((ddElement*)obj)->get_symbol():
                   //                 ((app*)obj)->get_symbol();
//  if (sym) return sym;
//  else
  return insert_symbol(obj);
}


// el is the app level of this symbol
void XrefTable::init_link(xrefSymbol* sym, Relational* el)
{
  Initialize(XrefTable::init_link);

  if (is_ddSymbol(el)) {
    add_link(sym, is_using, el); // add many links
    add_link(sym, used_by, el);
    add_link(sym, has_superclass, el);
    add_link(sym, has_subclass, el);
    // will add in other relations later
  }
  else { // is app header, interesting relations are
    // defining (from root), including, is_included (from head)
    add_link(sym, is_including, el); // add many links
    add_link(sym, included_by, el); // add many links
    sym->rm_link(lt_lmd, 0, this);
    sym->rm_link(lt_lmdhigh, 0, this);
    sym->set_language(checked_cast(app, el)->get_language(), this);
//    appTree* root = (appTree*) ((app*)el)->get_root();
//    add_link(sym, is_defining, root); // add many links
  }
}

// el is the app level of sym
void XrefTable::add_link(xrefSymbol* sym, linkType lt, Relational* el)
{
  Initialize(XrefTable::add_link );

  objSet os;
  RelType* rt = get_rel_type(lt);
  if (rt == NULL) return;
  os = get_relation(rt, el);
  Obj* ob;
  ForEach( ob, os) {
    if ((ob->get_id() >= 1) && 
	(is_app((Relational*)ob) || is_ddSymbol((Relational*)ob))) {
      xrefSymbol* sym1 = get_symbol((Relational*) ob);
      if (sym1)
	sym->add_link(lt, sym1, get_remove_hashPair(), 1);
    }
  }
}

void XrefTable::build_remove_module( char const *module_name, hashPair *rhps)
{
  Initialize(XrefTable::build_remove_module);
  xrefSymbol * dfs = lookup_module(module_name);
  if (!dfs || !dfs->get_has_def_file(this)) return;
  last_Xref = get_Xref();
  symbolSet all_symbols(1);
  symbolSet def_symbols(1);
  // collecting all relavent symbols in this file
  all_symbols.insert(symbolPtr(last_Xref,dfs));
  def_symbols.insert(symbolPtr(last_Xref,dfs));
  dfs->get_all_links(all_symbols, def_symbols, last_Xref);
  // collecting all relavent link in this file

  symbolPtr sym;
  ForEachT(sym, all_symbols) {
    sym->build_remove_module_link( def_symbols, all_symbols, rhps, last_Xref);
  }
}

int Xref_size_limit_exceed(headerInfo * hi, int size)
{
  if (hi)
    return ((ntohl(hi->next_offset_ind)/SIZEOF_XREFSYM + size) > XREF_LIMIT_SIZE);
  else
    return 0;
}

/* --------- used to cache module ------------*/
static XrefTable *cache_app_xref = 0;
static genString cache_app_name;
static symbolPtr cache_app_sym;
inline symbolPtr & get_cache_app_sym(XrefTable *xrt, char const *fn)
{
  return (cache_app_name.str() && cache_app_xref == xrt && cache_app_name == fn) ?
    (symbolPtr &)cache_app_sym : (symbolPtr &)NULL_symbolPtr;
}

void set_cache_app(XrefTable *xrt, char const *fn, symbolPtr sym)
{
  cache_app_xref = xrt;
  cache_app_name = fn;
  cache_app_sym = sym;
}

void file_rm_dup_links(symbolPtr sym)
{
  Initialize(rm_file_dup_link(symbolPtr));
  
  if (!sym.is_xrefSymbol() || sym.xrisnull() ||
      (sym.get_kind() != DD_MODULE))
    return;  
  
  last_Xref = sym.get_xref();
  symbolSet all_symbols(1);
  symbolSet def_symbols(1);
  
  // collecting all relavent symbols in this file
  all_symbols.insert(sym);
  def_symbols.insert(sym);
  sym->get_all_links(all_symbols, def_symbols, last_Xref);
  // collecting all relavent link in this file

  symbolPtr dsym;
  ForEachT(dsym, all_symbols) {
    dsym.rm_dup_links();
  }
}

symbolPtr XrefTable::insert_def_file_link(ddSymbol * dd, symbolPtr dfs)
  /* assume dfs be def file symbol for dd */
{
  Initialize(XrefTable::insert_def_file_link);
  symbolArr res;
  if (!dd) return NULL_symbolPtr;

  if (dfs.xrisnull()) {
    app *ah = dd->get_main_header();
    if (ah)
      dfs = lookup_module(ah->get_filename());
  }
  /* lookup(res, dd->get_kind(), dd->get_name(), dfs); */
  /* lookup(res, dd->get_kind(), dd->get_name(), dfs); */
  symbolPtr sym = dd->get_xrefSymbol();

  hashPair *rhps1 = get_remove_hashPair();
  if (sym.xrisnotnull() && dfs.xrisnotnull()) {
    linkType lt = (dd->get_is_def()) ? is_defined_in : ref_file;
    sym->add_link(lt, dfs, rhps1, 1);

    char const *ns = dd->def;
    if (ns && *ns)
      {
	char const *old_proto = xref_c_proto(sym);
	if(!old_proto || strcmp(ns,old_proto) > 0)
	  {
	    sym->rm_link(has_c_proto, this);
	    xrefSymbol * c_proto = lookup(res, DD_SYNTYPE, ns, 0); 
	    if (!c_proto)
	      c_proto = newSymbol(DD_SYNTYPE, strlen(ns), ns);
	    if (c_proto)
	      sym->add_link(has_c_proto, c_proto, rhps1, 0);
	  }
      }
  }
  return sym;
}

void XrefTable::remove_old_links(hashPair* rhps)
{
  Initialize(XrefTable::remove_old_links);
  for (xrefPair* xp = rhps->get_first(); xp; (xp = rhps->get_next()) )
  {
    xrefSymbol* sym = xp->src;
    IF (sym == 0) continue;
    hashPair* lhp = xp->hp;
    sym->rm_links(*lhp);
  }
}

int strcmp_name(ddKind, char const *, char const *); // char pointer converter

xrefSymbol *
XrefTable::confirm_is_symbol(xrefSymbol *sym,
				   ddKind k,
				   char const *name,
				   char const *file_sym_name)
{
  Initialize(XrefTable::confirm_is_symbol(symInd *, ddKind, char const *, xrefSymbol *));

  if ((k == sym->get_kind() || k == DD_UNKNOWN) &&
      (strcmp_name(k, name, sym->get_name(this)) == 0)) {
    if (file_sym_name) {
      xrefSymbol* def_file = 
           sym->get_has_def_file(this)?sym->get_def_file_priv(this):0;
      if (def_file) {
	  char const *name2 = def_file->get_name();
	  if (name2 && strcmp(file_sym_name, name2))
	      return 0;
// extra test for debugging purposes
	  IF (!file_sym_name || !name2) {}
      }
    }
    return sym;
  }
  return 0;
}

int XrefTable::lookup(objArr_Int& , Relational* , xrefSymbol*  )
{
  return 0;
}

// return an array of offset to mapped file
int XrefTable::lookup(objArr_Int& res, ddKind k, 
		      char const *name, xrefSymbol* file_sym)
{
  Initialize(XrefTable::lookup );

  if (!header) return 0;
  char const *file_sym_name = 0;
  if (file_sym) 
    file_sym_name = file_sym->get_name();
  unsigned int lh = Mhash(name);
  unsigned int h = lh % ntohl(header->table_max);
  hashElmt * hel = table[h];
  while (hel) {
    xrefSymbol* sym = 0;
    int wf = lh;
    if (ntohl(hel->el->hash_val) < ntohl(header->table_max) && lh >= ntohl(header->table_max))
      wf = lh % ntohl(header->table_max);
    if ((wf == ntohl(hel->el->hash_val)) && (sym = confirm_is_symbol(hel->el, k, name, file_sym_name)))
      res.insert_last(get_offset_from_symbol(sym));
    hel = hel->next;
  }
  return res.size();
}


xrefSymbol* XrefTable::lookup(symbolArr& res, ddKind k, 
		      char const *name, xrefSymbol* file_sym)
//  lookup a particular symbol in this one XrefTable
{
  Initialize(XrefTable::lookup);
  int init_size = res.size();
  char const *file_sym_name = 0;
  if (file_sym)
    file_sym_name = file_sym->get_name();
  int lh = Mhash(name);
  int h = lh % ntohl(header->table_max);
  Xref* Xr = my_Xref;
  hashElmt * hel = table[h];
  symbolArr new_res;
  while (hel) {
    xrefSymbol* sym = 0;
    int wf = lh;
    if (ntohl(hel->el->hash_val) < ntohl(header->table_max) && lh >= ntohl(header->table_max))
      wf = lh % ntohl(header->table_max);
    if ((wf == ntohl(hel->el->hash_val)) && (sym = confirm_is_symbol(hel->el, k, name, file_sym_name)))
	new_res.insert_last(symbolPtr(Xr,sym));
    hel = hel->next;
  }
  int new_size = new_res.size();
  if (new_size) {
    int unique_res = -1;
    for (int i = 0; file_sym && (i < new_size); ++i) {
      xrefSymbol *ret_sym = new_res[i];
      if (ret_sym->get_has_def_file(this)) {
	xrefSymbol* dfs = ret_sym->get_def_file_priv(this);
	if (dfs && ((file_sym == dfs) ||
		    (strcmp(file_sym_name, dfs->get_name()) == 0))) {
	  unique_res = i;
	  break;
	}
      }	
    }
    if (unique_res >= 0)
      res.insert_last(new_res[unique_res]);
    else {
      symbolPtr sym1;
      ForEachS(sym1, new_res) {
	res.insert_last(sym1);
      }
    }
    return res[init_size];
  }
  else
    return NULL;
}


xrefSymbol* XrefTable::lookup_module(char const *filename)
{Initialize(XrefTable::lookup_module );

  if (filename == 0) return 0;
 symbolPtr & sym = get_cache_app_sym(this, filename);
 if (sym.xrisnotnull()) return sym;
  symbolArr sym_ind_array;
  lookup(sym_ind_array, (ddKind)DD_MODULE, filename, 0);
  if (sym_ind_array.size() == 0) return 0;
  return sym_ind_array[0];
}

// get file_symbol for def_file of dd, 
void XrefTable::put_def_file_for_sym(ddSymbol* dd, xrefSymbol* sym)
{
  Initialize(XrefTable::put_def_file_for_sym );

  char const *dfn = dd->get_def_file();
  if (dfn == 0 || *dfn == 0) return;
  xrefSymbol* dfs = lookup_module(dfn);
  if (dfs == 0) {
    dfs = newSymbol((ddKind)DD_MODULE, strlen(dfn), dfn);
  }
  hashPair *rhps = get_remove_hashPair();
  if (dfs) sym->add_link(is_defined_in, dfs, rhps, 1);
  return;
}


xrefSymbol* XrefTable::get_symbol_from_offset(int offset)
{ 
  return ((xrefSymbol*) ((char *)ind_addr + offset));
}

int XrefTable::get_offset_from_symbol(xrefSymbol* sym)
{ 
  return ((((char *)sym) -  ((char *)ind_addr)));
}

xrefSymbol* XrefTable::get_def_file_symbol(Relational* obj)
{
  char const *obj_def_file = 0;

  if (is_ddSymbol(obj)) {
    ddSymbol* dd = (ddSymbol*) obj;
    obj_def_file = dd->get_def_file();
  }
  else if (is_app(obj)) {
    xrefSymbol* sym = 0;// ((app*)obj)->get_symbol();
    if (sym) return sym;
    app* ah = (app*) obj;
    obj_def_file = ah->get_filename();
  }

  return lookup_module(obj_def_file);
}


void XrefTable::write_whole_XrefTable(char const *, char const *, char const *)
{
  Initialize(XrefTable::write_whole_XrefTable);

  IF (symbackup == (char const *)0)
      return;
  if (backup) {
      delete backup;
      backup = 0;
  }
  OSapi_unlink (symfile_name);
  OSapi_unlink (indfile_name);
  OSapi_unlink(linkfile_name);
  OSapi_rename (symbackup, symfile_name);
  OSapi_rename (indbackup, indfile_name);
  OSapi_rename(linkbackup, linkfile_name);
  symbackup = 0;
  indbackup = 0;
  linkbackup = 0;
  do_backup ();
}

XrefTable* XrefTable::map_backup ()
{
    Initialize (XrefTable::map_backup);

    IF (symbackup == (char const *)0)
	return NULL;
    if (backup) delete backup;
    backup = new XrefTable(my_Xref, symfile_name, indfile_name, linkfile_name, 3);
    return backup;
}
   
//  To make a testing version, uncomment the following 
#ifdef _PSET_DEBUG
//#define  TEST_VERSION
#endif

int XrefTable::to_write(char const *xn)
{
  Initialize(XrefTable::to_write);

#ifndef _WIN32
  int i, j, k;

  i = mprotect((caddr_t) sym_header, sym_map_size, PROT_READ|PROT_WRITE);
  j = mprotect((caddr_t) ind_header, ind_map_size, PROT_READ|PROT_WRITE);
  k = mprotect((caddr_t) link_header, link_map_size, PROT_READ|PROT_WRITE);
  if ( (i || j || k) && (!my_Xref || my_Xref->writable_on_disk)  )
  {
      char const *errm = "error" ;
      if (errno == EACCES) {
	  msg("DISCOVER does not have write access to the model file:\n$1\nPlease change the permissions on this file and rerun DISCOVER.", catastrophe_sev) << xn << eom;
	  driver_exit(1);
      }
      else if (errno == EINVAL) errm = "EINVAL";
      else if (errno == ENOMEM) errm = "ENOMEM";
      msg("DISCOVER reports error code $1 on file: \n$2\nPlease quit without saving any changes.", warning_sev) << errm << eoarg << xn << eom;
       return 0;
  }
#endif
  IF (get_Xref()->is_unreal_file()) {
      msg("Failed to write file $1", error_sev) << xn << eoarg << eom;
      return 0;
  }
  if (symbackup == (char const *)0) do_backup();

  make_room(250000);	// move the table if not big enough for a few odds and ends
  return 1;
}

int XrefTable::to_read(char const *xn)
{
  Initialize(XrefTable::to_read);

  if (symbackup != (char const *)0) {
      if (backup) {
	  delete backup;
	  backup = 0;
      }
/*
      unlink (symfile_name);
      unlink (indfile_name);
      OSapi_rename (symbackup, symfile_name);
      OSapi_rename (indbackup, indfile_name);
      symbackup = 0;
      indbackup = 0;
*/
  }
#ifndef _WIN32
  IF (mprotect((caddr_t) header, sym_map_size, PROT_READ) ||
      mprotect((caddr_t) ind_addr, ind_map_size, PROT_READ) || mprotect((caddr_t) link_addr, link_map_size, PROT_READ)) {
    msg("FAIL TO CONVERT $1 TO READ!!!!, PLEASE QUIT", error_sev) << xn << eom;
    return 0;
  }
#endif
  unit  = close_handle(unit);
  unit1 = close_handle(unit1);
  unit2 = close_handle(unit2);

  return 1;
}

//------------------------------------------
// XrefTable::max_offset() 
//------------------------------------------

unsigned int XrefTable::max_offset() {
   Initialize(XrefTable::max_offset);
   
   headerInfo* hi = get_header();
   return ntohl(hi->next_offset_ind);
}

#ifdef  EXCLUSIVE_OPEN
static bool  abort_flag=0;
#endif

//  To make a testing version, uncomment the following 
#ifdef _PSET_DEBUG
//#define  TEST_VERSION
#endif

void XrefTable_insert_SWT_Entity_Status (app* head);

void XrefTable::insert_SWT_Entity_Status (app* head) 
{
  XrefTable_insert_SWT_Entity_Status (head);
}

void XrefTable_call_rel_hooks(XrefTable * that, ddRoot* dr);

void XrefTable::call_rel_hooks(ddRoot* dr)
{
  XrefTable_call_rel_hooks(this, dr);
}


void XrefTable_insert_module(XrefTable* that, app* sh);

void XrefTable::insert_module(app* sh)
{
  XrefTable_insert_module(this, sh);
}

void XrefTable_insert_module(XrefTable* that, app* sh, int);

void XrefTable::insert_module(app* sh, int val)
{
  XrefTable_insert_module(this, sh, val);;
}


static int check(xrefSymbol* sym, linkTypes & la, int nlinks, XrefTable* Xr)
{
    int retval = 0;

    if (nlinks == 1) {
	linkType lt = la;
	retval = sym->local_only(lt,0,Xr);
    } else 
	retval = la.local_only();

    return retval;
}

int XrefTable::collect_symbols(xrefSymbol* sym, linkTypes& la, int nlinks, symbolArr& as, xrefSymbol* dfs, int from_cxr, XrefTable* sym_xrt)
{
    Initialize(XrefTable::collect_symbols);
    
    int retval = 0;
    
    linkType lt = la;
    symbolArr xra;
    symbolPtr xr_sym = lookup(xra,sym->get_kind(),sym->get_name(sym_xrt),dfs);
    if (xra.size() == 0 && from_cxr)
	xr_sym = lookup(xra,sym->get_kind(),sym->get_name(sym_xrt),0);
    if(xra.size()) {
	if (xr_sym.xrisnull())
	    xr_sym =xra[0];
	if (nlinks == 1)
	    xr_sym->get_link(lt,as,1);
	else
	    xr_sym.get_links(la,as,1);
	retval = check(xr_sym,la,nlinks, this);
    }

    return retval;
}



char const *XrefTable::get_sym_addr()
{
    return sym_addr;
}

int xref_convert_to_num (const symbolPtr& sym, int & pmod_num, int & offset)
{
    int retval = 0; // failure

    Xref * Xr = sym.get_xref();
    if (Xr) {
	XrefTable *xrt = Xr->get_lxref();
	if (xrt)
	    if (is_XrefTable(xrt)) {
		pmod_num = ((XrefTable*)xrt)->get_pmod_num();
		offset = sym.get_offset();
		retval = 1; // success
	    }
    }	
	
    return retval;
}


int xref_convert_from_num(symbolPtr & sym, int pmod_num, int offset)
{
    int retval = 0; // failure

    if (pmod_num > 0 && offset > 0) {
	XrefTable* xrt = XrefTable::get_XrefTable_from_num(pmod_num);
	if (xrt) {
	    headerInfo* hi = xrt->get_header();
	    if (offset*SIZEOF_XREFSYM <= ntohl(hi->next_offset_ind)) {
		Xref * Xr = xrt->get_Xref();
		sym.put_info(Xr, (unsigned)offset);
		ddKind Kind = sym.get_kind();
		if (Kind > 0 && Kind < NUM_OF_DDS)
		  retval = 1; // success
	    }
	}
    }

    if (!retval)
	sym = NULL_symbolPtr;

    return retval;
}

int XrefTable::get_sym_offset_from_ind(int ind)
{
  return sizeof(xrefSymbol)*(ind-1);
}

xrefSymbol * XrefTable::get_symbol_from_index(int ind)
{
  return get_ind_addr() + ind - 1;
}

//initialize symbolIndex_table, number of entries is one greater than total num of 
//syms. Dummy first entry, real symbols start at index 1, since offset of 
//symbolPtr has to be > 0
uint XrefTable::init_symbolIndex_table() 
{
  symbolIndex_table = new SymbolIndexArr;
  uint num_syms  = ntohl(header->next_offset_ind)/SIZEOF_XREFSYM;
  symbolIndex_table->grow(num_syms+1);
  
  int val = -1;
  orig_link_offsets.append(&val);  //first entry, dummy
  xrefSymbol* xrefsym;
  for(int i=0; i<num_syms; i++) {
    xrefsym = get_ind_addr() + i;
    val = xrefsym->get_offset_to_first_link();
    orig_link_offsets.append(&val);
  }

  return num_syms;
}


//makes new SymbolIndex entry, returning the index of the symbol in the table
//if symbolIndex_table is not initialized yet, do it now if link file is writable
//initialization of table is delayed until either a new symbol or link is created
uint XrefTable::make_new_symbolIndex()
{
  uint ret_val = 0;
  if (!symbolIndex_table) {
    if (link_is_writable) ret_val = init_symbolIndex_table();
  }
  ret_val = symbolIndex_table->size();
  if (ret_val > XREF_LIMIT_SIZE) {
    msg("Pmod file $1 exceeds the 16 million symbol size limit.  Please partition your project further.", catastrophe_sev) << indfile_name.str() << eom;
    driver_exit(76);
  }
  symbolIndex_table->grow(1);
  
  int val = -1;
  orig_link_offsets.append(&val);
  
  return ret_val;
}

void check_old_version_of_pmod(char const *sym_file, char const *ind_file, Xref* parent)
{
  if (OSapi_access(sym_file, R_OK) == 0) {
#ifdef _WIN32
  HANDLE fd =  open_timeout(sym_file, O_RDONLY, 0444);
  if(fd != INVALID_HANDLE_VALUE){
#else
  int fd = open_timeout(sym_file, O_RDONLY, 0444);
  if (fd >= 0) {
#endif
    symHeaderInfo sym_hi;
    OSapi_bzero(&sym_hi, sizeof(symHeaderInfo));
#ifdef _WIN32
    DWORD bytes_read;
    ReadFile(fd, &sym_hi, sizeof(symHeaderInfo), &bytes_read, NULL);
#else
    OSapi_read(fd, (char *)&sym_hi, sizeof(symHeaderInfo));
#endif
    fd = close_handle(fd);
    if (ntohl(sym_hi.version) == 1) {
      if (!is_model_build() || (customize::getIntPref("buildNewPmod") <= 0)) {
       msg("Wrong version of pmod file: $1\n"
	   "The pmod format has been changed for the"
	   " current version of DISCOVER.\n"
	   "Please run the previous version of DISCOVER,"
	   " or rebuild the project: \"$2\",\n"
	   "by setting the preference \"buildNewPmod\" to 1"
	   " or by deleting the pmods.\n", catastrophe_sev)
		<< sym_file << eoarg
		<< parent->get_projNode()->get_ln() << eom;
       driver_exit(91);
      }
     else {
      msg("Warning: Old version of pmod files. Deleting pmod.sym, pmod.ind, pmod.met and pmod.dfa files", warning_sev) << eom;
      OSapi_unlink(sym_file);              //remove pmod.sym file
      OSapi_unlink(ind_file);              // remove pmod.ind file
      genString pmod_filename = sym_file;
      int len = pmod_filename.length();
      if (len) {
        char *nm = (char *)pmod_filename.str();
	nm[len - 3] = 'd';
	nm[len - 2] = 'f';
	nm[len - 1] = 'a';
	OSapi_unlink(nm);       //remove pmod.dfa file
	
	nm[len - 3] = 'm';
	nm[len - 2] = 'e';
	nm[len - 1] = 't';
	OSapi_unlink(nm);      //remove pmod.met file
      }
    }
   }
  }
 }
}
    

