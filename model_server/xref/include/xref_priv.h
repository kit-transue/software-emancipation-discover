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
/*  xref_priv.h :    this file contains those definitions needed only by
     Xref.h.C,     Xref_util.C,    xref.h.C   and   xref_util.C
     sorted_links.C    and checkpmod.C
     
  Other modules should include only  xref.h


*/

#ifndef _xref_priv_h
#define _xref_priv_h
#ifndef _Relational_h
#include <Relational.h>
#endif

#include <genError.h> /* to avoid nesting too deep for CC */

#include <linkTypes.h> /* new* NY, 09.20.95 */
#include <xref.h>
#include <SharedXref.h>
#include <symHeaderInfo.h>
#include <indHeaderInfo.h>
#include <Link.h>
#include <link_type_converter.h>
#include <XrefTable.h>
#include <xrefSymbol.h>
#include <ddSymbol.h>
/*
  1. distinction between local and global xref should be implicit.
  2. *globals* xref are organized and manipulated by the class xref
  */

class XrefTable;
class xrefSymbol;
class Xref;

//typedef unsigned char uchar;

class ddChange
{
public:
  xrefSymbol* sym;
  int sig;
  ddChange(xrefSymbol* s, int si) { sym = s; sig = si; }
};

//may not need this, for now just use get_relation
class rel_link
{
public:
  linkType lt;
  RelType* rt;
};

class XrefObj: public Relational
{
public:
  char * sym_file;
  char * ind_file;
  XrefTable *gxref;

  XrefObj(char* sf, char* ind_f);
  void init();
  void remove_gxref();
  ~XrefObj();
  XrefTable* get_gxref() { return gxref;}
};


struct link_iter
{
  int count;
  symbolArr sa;

  link_iter(xrefSymbol *s, linkType lt);


  xrefSymbol* get_next();
};


//------------------- start of function decls -----------------
void XREF_insert_module(app* ah);
void XREF_insert_module(app* ah);
//xrefTable* get_xref();
//void Xref_set_mode(int m);
void handle_superclassof(ddElementPtr user, smtTreePtr smt, int oper);
void handle_superclassof_att(ddElementPtr , ddElement *, int, RelType *);
void handle_def_smt_of_dd(ddElementPtr used, smtTreePtr smt, int oper);
void handle_user_of_used(ddElementPtr user, smtTreePtr smt, int oper);
void handle_smt_decl_of_semtype(ddElementPtr used, smtTreePtr smt, int oper);
void handle_has_friend(ddElementPtr used, smtTreePtr smt, int oper);
void handle_struct_decl_of_ref(ddElementPtr cp, ddElementPtr fp, int oper);
void handle_ref_file_of_dd(ddElementPtr used, smtTreePtr , int oper);
void handle_grp_rel(ddElementPtr src, ddElementPtr trg, int oper, RelType*);
int set_build_xref_flag(int i) ;
char* get_stripped_name( char* fn);
//int filename_cmp(char* pn, char* ln);
symbolPtr lookup_file_symbol(char* fn);
//xrefSymbol* lookup_file_symbol_priv(char* fn);

// private versions of functions declared in xref.h
xrefSymbol*  lookup_file_symbol_priv(ddKind kind, const char* ln);
xrefSymbol* Xref_file_is_newer_priv(char* ln); // 0 if newer, else symbol
xrefSymbol* Xref_file_is_newer_priv(char *ln, char* fn); // 0 if newer, else symbol
//xrefSymbol* lookup_xrefSymbol_priv(ddKind, const char*);
// definitions aren't in Xref.h.C
xrefSymbol* Xref_insert_symbol_priv(ddKind k, char* ns);
xrefSymbol* Xref_insert_symbol_priv(ddSymbol* dd, app* h);
int has_auto_rels(Relational *);
#endif


/*
   START-LOG-------------------------------------------

   $Log: xref_priv.h  $
   Revision 1.13 1996/03/01 10:26:48EST aharlap 
   stage dd cleanup
// Revision 1.25  1994/06/29  19:51:39  davea
// Bug track: 7756
// new member function get_symbols(symbolArr&, Xref*)
//
// Revision 1.24  1994/05/26  11:50:11  davea
// Bug track: n/a
// new member function XrefTable::get_symbols() used to
// collect all the symbols from one pmod.  Will be used in
// analyzing and comparing pmods & psets.
//
// Revision 1.23  1994/03/21  19:53:42  trung
// Bug track: 6625
// submitting for cpp_open_file, etc.
//
// Revision 1.22  1994/02/23  20:45:28  davea
// Bug track: 6461,6480,6440
// fix hashPair::get_first() and get_next() for lt_junk at begin of chain.
// Rewrite add_links() to use add_link().
// In app_get_proj(), put relation so next time it's quicker.
// etc.
//
// Revision 1.21  1994/02/02  20:14:17  davea
// Bug track: n/a (performance)
// I already typed in detailed comments, which were lost
// by the configurator.
//
// Summary:  speeded it up.
//
// Revision 1.20  1994/01/31  15:45:45  trung
// Bug track: 0
// saving xref processing
//
// Revision 1.19  1994/01/06  17:26:21  trung
// Bug track: 0
// various fixes for change prop
//
// Revision 1.18  1993/12/21  23:27:17  davea
// bug track: 5744
// Add new is_consistent_hashElmt* functions to
// XrefTable class.  For sun4-g use only.
//
// Revision 1.17  1993/12/08  21:31:35  wmm
// Bug track: 5043
// Fix bug 5043.
//
// Revision 1.16  1993/11/28  19:50:07  davea
// Bug track: 4877
// Add data members to XrefTable:  sym_map_size and
// ind_map_size.  Also eliminate unused SharedXrefTable class.
// And eliminate declaration of second XrefTable constructor.
//
// Revision 1.15  1993/08/22  19:10:06  davea
// bugs 4433, 4436 - revamp the way copy_module() and delete_module() work
//
// Revision 1.14  1993/08/18  00:03:15  trung
// for rename propagation
//
// Revision 1.13  1993/06/21  21:21:15  davea
// bug 3615 - added flag to cause delete of file in destructor
//
// Revision 1.12  1993/06/13  18:51:13  davea
// Add update_sym_addr_offset() member function to xrefTable
//
// Revision 1.11  1993/06/08  21:54:42  davea
// bug 3553 - memory leak - moved xrefPair.hp out of union,
// fixed initialization of next in inline functions,
// added initialization of hp in inlines.
//
// Revision 1.10  1993/06/02  22:11:17  davea
// bugs 3490, 3371 - support stuff for new pmod format
//
// Revision 1.9  1993/05/18  19:33:18  davea
// for hierarchy project:  XrefTable::add_symbols()
//
// Revision 1.8  1993/05/17  15:02:39  davea
// for searching multiple subprojects, added member xrefTable::my_Xref
//
// Revision 1.7  1993/05/05  22:46:23  wmm
// Partial fix for bug 3571 (preparation for running ParaANALYZER from the xref only, without restoring the ddElements).
//
// Revision 1.6  1993/04/30  01:03:15  trung
// speed up for xref
//
// Revision 1.5  1993/04/13  22:28:52  trung
// fixes for merge, tidy xref
//
// Revision 1.4  1993/04/01  00:37:35  trung
// performance improve
//
// Revision 1.3  1993/03/29  20:02:47  trung
// add new method
//
// Revision 1.2  1993/03/29  00:29:46  mg
// cleanup symbolPtr(xref*)
// >> .
//
// Revision 1.1  1993/03/27  02:15:41  davea
// Initial revision
//

   END-LOG---------------------------------------------
*/


