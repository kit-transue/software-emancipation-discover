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
// file  db.h
//   define classes and routines for "load" and "restore" pset

#ifndef _db_h
#define _db_h

#ifndef _genArr_h
#include <genArr.h>
#endif

// Filename suffix for database files created by ParaSET
#define DB_SUFF ".pset"

class app;
class appTree;
class objTree;

int  db_save (app*, char const *fname);
int  db_save (app*, char const *fname, int save_xref);
int  db_save_or_cancel (app*, char const *fname, int save_xref);
app* db_restore (char const *fname);
app* db_restore (char const *fname, char const *ln);
bool db_restore (app*);
void db_make_name (char *name, char const *fn, char const *suff);

typedef unsigned char byte;
genArr(byte);

class db_buffer : public genArrOf(byte)
{
  public:
    db_buffer() : pos(0), line_no(0) {}

    void put (char const *);  // \0 - terminated
    void put (void*, int);
    void put_byte (byte);
    void put_short (short);
    void put_int (int);
    void put_double (double);
    void put_spaces (int);

    bool get (char *&);  // 1 = success, 0 = reached the end of the buffer
    bool get (void*, int);
    bool get_byte (byte&);
    bool get_short (short&);
    bool get_int (int&);
    bool get_double (double&);
    bool skip (int);
    int  get_position() { return pos; }
    void compress();
  protected:
    int pos;
    int line_no;

    byte* get_cur() { return operator[](pos); }
};


struct bt
{
    unsigned f0 : 1;
    unsigned f1 : 1;
    unsigned f2 : 1;
    unsigned f3 : 1;
    unsigned f4 : 1;
    unsigned f5 : 1;
    unsigned f6 : 1;
    unsigned f7 : 1;
};


class db_app
{
  public:
    unsigned is_node :  1; // not used by app;
    unsigned dh      :  2; // not used by app;

    void clean();
    void set_dh(int i) { dh = i; }

  public:
    unsigned is_leaf :  1;
    unsigned is_last :  1;
    unsigned flag    :  3;
    unsigned val     : 24;
    unsigned extype  :  8;
    unsigned type    :  8;

    db_app(objTree*);
    db_app() {}

friend class db_intern_buffer;
};


typedef void
 (*db_save_node)(app*h, appTree*n, db_app&, db_buffer&);     // static

typedef appTree*
 (*db_restore_node)(app*h, appTree *parent, db_app&, db_buffer&); // static

typedef app*
 (*db_restore_header)(char const *title, byte  language, char const *pn_name);       // static

typedef void
 (appTree::*db_after_rest)(db_buffer&);   // nonstatic non-virtual on root.

typedef void
 (app::*db_after_rest_head)();            // nonstatic non-virtual on header.


#if 0
// ------- interfaces ----------
static void      appTree::db_save(app*, appTree*node, db_app&, db_buffer&);
static appTree*  appTree::db_restore(app*,appTree *parent, db_app&, db_buffer&);
static app*      app::db_restore(char const *title, byte language, char const *ph_name);
void  appTree::db_after_restore(db_buffer&); // optional member on root
#endif


/*
    START-LOG-------------------------------

    $Log: db.h  $
    Revision 1.3 1997/03/25 17:47:51EST aharlap 
    added methods to db_buffer()
// Revision 1.22  1994/07/24  19:02:24  so
// Bug track: 7203,7903,7295
// fix bugs 7203,7903,7295
//
// Revision 1.21  1994/05/05  21:14:10  aharlap
// *** empty log message ***
//
// Revision 1.20  1993/08/22  22:43:44  sergey
// Added definition of db_save_or_cancel. Part of bug #4388.
//
// Revision 1.19  1993/03/08  14:23:41  aharlap
// added phys_name argument to hd_restore ()
//
// Revision 1.18  1993/02/22  21:22:39  glenn
// Remove stdlib.h.
// Protect include file genArr.h
// Conform to other coding standards.
//
// Revision 1.17  1993/02/21  16:33:11  mg
// db_after_restore for app
//
// Revision 1.16  1993/02/12  15:07:18  efreed
// added declaration for additional interface to db_save which
// permits control over whether Xref module is saved when a source
// file is saved.
//
// Revision 1.15  1993/01/28  18:34:48  aharlap
// added incremental restoring
//
// Revision 1.14  1993/01/25  00:41:57  aharlap
// cleanup
//
// Revision 1.13  1993/01/24  21:52:16  aharlap
// changed return type for db_restore ()
// >> .
//
// Revision 1.12  1993/01/13  20:29:59  aharlap
// *** empty log message ***
//
// Revision 1.11  1993/01/12  00:55:50  aharlap
// add class app declaration
//
// Revision 1.10  1993/01/11  03:55:12  aharlap
// *** empty log message ***
//
// Revision 1.9  1993/01/08  20:08:22  aharlap
// fixed alignment problems
//
// Revision 1.8  1993/01/07  19:21:43  aharlap
//  .
//
// Revision 1.7  1993/01/07  19:15:26  aharlap
// fixed bug
//
// Revision 1.6  1993/01/07  19:14:13  aharlap
// *** empty log message ***
//
// Revision 1.5  1993/01/07  18:51:16  aharlap
// added db_buffer_ro
//
// Revision 1.4  1993/01/07  04:05:23  mg
// bugs
//
// Revision 1.3  1993/01/06  19:06:30  mg
// *** empty log message ***
//
// Revision 1.2  1993/01/05  21:58:19  mg
// *** empty log message ***
//
    END-LOG---------------------------------
*/

#endif // _db_h
