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
#ifndef _SymManager_h
#define _SymManager_h

#if !defined(_WIN32)
#if defined(__MC_LINUX__)
#include <gdbm/ndbm.h>
#else
#include <ndbm.h>
#endif
#else
extern "C" {

#define PBLKSIZ 1024
#define DBLKSIZ 4096
 
typedef struct {
        int     dbm_dirf;               /* open directory file */
        int     dbm_pagf;               /* open page file */
        int     dbm_flags;              /* flags, see below */
        long    dbm_maxbno;             /* last ``bit'' in dir file */
        long    dbm_bitno;              /* current bit number */
        long    dbm_hmask;              /* hash mask */
        long    dbm_blkptr;             /* current block for dbm_nextkey */
        int     dbm_keyptr;             /* current key for dbm_nextkey */
        long    dbm_blkno;              /* current page to read/write */
        long    dbm_pagbno;             /* current page in pagbuf */
        char    dbm_pagbuf[PBLKSIZ];    /* page file block buffer */
        long    dbm_dirbno;             /* current block in dirbuf */
        char    dbm_dirbuf[DBLKSIZ];    /* directory file block buffer */
} DBM;
 
#define _DBM_RDONLY     0x1     /* data base open read-only */
#define _DBM_IOERR      0x2     /* data base I/O error */
 
#define dbm_rdonly(db)  ((db)->dbm_flags & _DBM_RDONLY)
 
#define dbm_error(db)   ((db)->dbm_flags & _DBM_IOERR)
        /* use this one at your own risk! */
#define dbm_clearerr(db)        ((db)->dbm_flags &= ~_DBM_IOERR)
 
/* for flock(2) and fstat(2) */
#define dbm_dirfno(db)  ((db)->dbm_dirf)
#define dbm_pagfno(db)  ((db)->dbm_pagf)
 
typedef struct {
        char    *dptr;
        int     dsize;
} datum;
 
/*
 * flags to dbm_store()
 */
#define DBM_INSERT      0
#define DBM_REPLACE     1
 
DBM     *dbm_open(char *, int, int);
void    dbm_close(DBM *);
datum   dbm_fetch(DBM *, datum);
datum   dbm_firstkey(DBM *);
datum   dbm_nextkey(DBM *);
int     dbm_delete(DBM *, datum);
int     dbm_store(DBM *, datum, datum, int);

};
#endif  /* _WIN32 */

#include <so_shared.h>
#ifndef _genString_h
#include <genString.h>
#endif

enum MangleType {
    CFRONT_MANGLE,
    GNU_MANGLE,
    NO_MANGLE
};

enum symbolTableType {
    A_OUT_SYMBOLS = 0,
    SO_SYMBOLS
};

class SymValue {
public:

   SymValue();
   ~SymValue();
   void *addr();
   void addr(void *);
   const char *mangled_name();
   void mangled_name(const char *);
   const char *qualifier();
   void qualifier(const char *);

/* private: */
   char *qual;
   void *value;
   char *name;
};

class SymbolManager;

struct symMgrIterHandle {
    char * function_name;
    char * variable_name;
    char * context;
    char * filename;
    char * path;
    SymValue * * vptr;
    int ret_val;
};

typedef int (SymbolManager::*symMgrIterMethod)(struct symMgrIterHandle *);

// Define type-safe symbol table manager

class SymbolManager {
private:
    genString filename;
    DBM *dbm;
    MangleType mangle_type;

    /* support for Shared Libraries (so_shared) */
    struct sharedLibraryInfo * shared_header;
    int executableName(char *);

    int getGlobalFunction(struct symMgrIterHandle *);
    int getGlobalVariable(struct symMgrIterHandle *);
    int getStaticFunction(struct symMgrIterHandle *);
    int getStaticVariable(struct symMgrIterHandle *);
    int getVariableFromPath(struct symMgrIterHandle *);

    int traverse(struct symMgrIterHandle *, symMgrIterMethod);

    struct symMgrIterHandle iter_handle;
    static genString build_exec_path;
    /* ---------------------------------------- */
public:
    SymbolManager (char *);
    SymbolManager (char *, int);
    ~SymbolManager ();
    int get_global_function (char* fnname, SymValue **vptr);
    int get_global_variable (char* fnname, SymValue **vptr);
    int get_static_function (char* flname, char* fnname, SymValue **vptr);
    int get_static_variable (char* flname, char *varname, SymValue **vptr);
    int get_variable_from_path ( char* context, char *fnname, char *path,
	char *varname, SymValue **vptr);
    int is_open () { return  (dbm != 0); }
    MangleType get_mangle_type () { return mangle_type; }
    int is_cfront_mangle () { return mangle_type == CFRONT_MANGLE; }

    /* (so_shared) */
    char * executableName() { return (char *)filename; }
    struct sharedLibraryInfo * sharedHeader() { return shared_header; }
    int sharedHeader(struct sharedLibraryInfo * header) { return (shared_header=header) == 0; }

    int updateSharedLibraries(struct sharedLibraryInfo *);
    int updateCommonSymbols(struct sharedLibraryInfo *);
    int loadSharedSymbols(struct sharedLibrary *);
    int disposeSharedSymbols(struct sharedLibrary *);

    int printSOList(genString &);
    int listedLibrary(char *);
    int rebuildSymbolTable(char *, char * = 0);

    static char * buildExecPath() { return (char *)build_exec_path; }
    static int buildExecPath(char *);
    /* ----------- */

    static int good_pag_file(char *file);
    static int linked_correctly(const char *executable);
};


#endif

/*
// SymManager.h
//------------------------------------------
// synopsis:
// Executable file symbol table manager
//------------------------------------------
// $Log: SymManager.h  $
// Revision 1.6 2000/07/07 08:12:22EDT sschmidt 
// Port to SUNpro 5 compiler
 * Revision 1.2.1.12  1994/03/22  14:16:40  pero
 * support for COMMON symbols (runtime resolve)
 *
 * Revision 1.2.1.11  1994/03/17  23:35:47  builder
 * Added include for genString.h -jef
 *
 * Revision 1.2.1.10  1994/03/17  00:07:18  pero
 * support for shared libraries
 *
 * Revision 1.2.1.9  1993/09/13  22:56:40  so
 * fix bug 4709
 *
 * Revision 1.2.1.8  1993/06/08  00:25:17  so
 * create a static function to check if the pag file is good for the executable.
 *
 * Revision 1.2.1.7  1993/06/02  22:33:18  aharlap
 * added case gnu mangle
 *
 * Revision 1.2.1.6  1993/04/17  23:12:24  builder
 * fixed bug
 *
 * Revision 1.2.1.5  1993/04/16  21:42:02  smit
 * Fix mixlib problem.
 *
 * Revision 1.2.1.4  1993/04/15  23:54:56  smit
 * Fix bug#3278 3370
 *
 * Revision 1.2.1.3  1992/12/15  21:27:28  smit
 * declare functions to support routing of file static functions.
 *
 * Revision 1.2.1.2  1992/10/09  17:39:01  builder
 * fixed rcs header
 *
 * Revision 1.2.1.1  92/10/07  21:24:26  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  21:24:24  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:24:35  smit
 * Initial revision
 * 
//Revision 1.5  92/08/10  11:22:19  aharlap
//declared destructor
//
//Revision 1.4  92/08/09  20:36:06  builder
//added method is_open ()
//
//Revision 1.3  91/12/05  10:17:40  kws
//New dbm base symbol manager
//
//Revision 1.2  91/11/20  20:27:16  kws
//Only load symbol manager once
//
//Revision 1.1  91/11/07  20:04:30  kws
//Initial revision
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/
