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
//  file xref.h
//  define classes Xref and xrefSymbol
//   Note:  other related classes are to be found in xref_priv.h

#ifndef _xref_h
#define _xref_h
#ifndef _Relational_h
#include <Relational.h>
#endif
#ifndef __objSet
#include <objSet.h>
#endif
#ifndef __objArr_Int
#include <objArr_Int.h>
#endif
#include <ddSymbol.h>
#ifndef _proj_h
#include <proj.h>
#endif
#ifndef _symbolPtr_h_
#include <symbolPtr.h>
#endif

#include <_Xref.h>
#include <xrefSymbol.h>

/*
  1. distinction between local and global xref should be implicit.
  2. *globals* xref are organized and manipulated by the class xref
  */

class smtHeader;
int xref_is_excluded_smt(smtHeader*);

class hashPair;
//class XrefTable;
class xrefSymbol;
class Xref;
//class projNode;
/* class linkTypes; * NY, 09.20.95 */
typedef unsigned char uchar;
class linkType_selector;

#define READ_ONLY 1
//  Note:  remove references to "WRITE_ONLY", since it's a bad name.  Use !READ_ONLY instead
#define WRITE_ONLY 0
#define MAX_BROWSER 128

// the following values are used to initialize signature & kind in all Xref 
//    constructors,
//    and are tested in symbolPtr class functions, for safety's sake
//  I made the signature decimal, so it's easy to spot in the debugger
enum { XREF_SIGNATURE = 12345678};
enum { XREF_KIND = 0xff};

extern XrefTable* XrefTableNull;

//------------------- start of function decls -----------------
Xref* get_XREF(char const *fn = 0);
//void XREF_insert_module(app* ah, int );
//void XREF_add_link(xrefSymbol* sym, linkType lt, Relational* ah);
time_t get_last_modified_date(char const *s);
//void dd_fill_idarg_included_by(xrefTable* xr, smtHeader* sh);
ddRoot* dd_sh_get_dr(app*);
symbolPtr Xref_insert_symbol(ddSymbol* dd, app* h);
//symbolPtr Xref_file_is_newer(char const *ln); // 0 if newer, else symbol
symbolPtr Xref_file_is_newer(char const *ln, char const *fn); // 0 if newer, else symbol
void new_xref_get_filenames(RTLNode* rtl);
//smtTree* smt_get_scope_fn(smtTree* ref_smt);
void XREF_tidy_file(app* ah);
int user_ddKind(ddKind k);
ddElementPtr new_dd_find_definition(ddKind, char const *);
void new_xref_init_rtl(RTLNodePtr, const ddSelector&);
symbolPtr lookup_xrefSymbol(ddKind, char const *);
app* XREF_get_files_including(app* ah);
app* XREF_get_files_including(char const *ln);
//bool XREF_file_included_by(app* ah);
extern int XREF_check_files_including(char const *fn);
smtHeader * load_smtHeader(char const *ln, char const *text, smtLanguage l,smtTree* scope);
//smtTree* smt_get_scope_fn(smtTree*);
projNode* dd_find_proj_which_has_loaded_def_of (ddKind kind, char const *name);
//int get_build_xref_flag();
//ddElement* dd_lookup_create(char const *ns, appTree* node, int xref_type, 
//			    ddKind k, RelType* rel);
void XREF_set_last_modified_date (app *);
// ask Trung about these:
int is_smt_restore(ddElement*);
void handle_ref_smt_of_dd(ddElementPtr used, smtTreePtr smt, int oper);
void handle_decl_smt_of_dd(ddElementPtr used, smtTreePtr smt, int oper);
//Xref* load_XREF(char const *fn = 0);
Xref* load_XREF(char const *fn, int perm);
symbolPtr  lookup_file_symbol(ddKind kind, char const *ln);
symbolPtr  lookup_file_symbol_in_xref(char const *fn, Xref* xr);
//int filename_cmp(char const *pn, ddElement* ep);
void set_not_call_hook();
void unset_not_call_hook();
extern int xref_not_header_file(char const *full_path);
extern int xref_not_src_file(char const *full_path);
extern int xref_not_txt_file(char const *full_path);
extern int xref_not_allowed_file_type(char const *full_path);
int get_projList(objSet & ps);
//int get_all_xref_proj_from(objSet & fr, objSet & all);
ddElement* dd_lookup_or_create(char const *,appTree*, int, ddKind, RelType* , char const *);
ddElement *dd_get_any_assoc(appTree * node, ddElement *but_not_this);
void xref_add_link(ddElementPtr, ddElementPtr, linkType);
void handle_struct_decl_of_ref(ddElementPtr cp, ddElementPtr fp, int oper);
void show_arena(char const *full_path);
void get_pmod_projs_from(objArr& subprojs_with_pmods, projNode*);

#endif

/*
$Log: xref.h  $
Revision 1.25 1999/08/02 14:29:12EDT rpasseri 
adding return type of int for uber-picky linux compiler
// Revision 1.136  1994/07/28  21:09:56  trung
// Bug track: 0
// bugs: 7785, 7852, 7820, 7716, 7676
//
// Revision 1.135  1994/07/08  15:04:44  trung
// Bug track: 0
// 7279, 7285, 7372, 7585, 7715, 7582, demangling name for template related
// putting attributes for xrefSymbol,
//
// Revision 1.134  1994/06/29  19:48:48  davea
// Bug track: 7756
// declare new member functions get_attribute and set_attribute
// make sorted_link a friend
//
// Revision 1.133  1994/05/26  11:45:31  davea
// Bug track: n/a
// Make friends of sorted_link & spec_link, so they can efficiently
// collect lists of sorted links.
// Declare show_arena(), a debug routine to display arena sizes & time
//
// Revision 1.132  1994/04/27  00:52:39  trung
// Bug track: 0
// quick search, etc.
//
// Revision 1.131  1994/04/14  14:40:02  trung
// bugs : 6892, 7003
//
// Revision 1.130  1994/03/22  20:09:51  mg
// Bug track: 1
// validation for dormant code
//
// Revision 1.129  1994/03/21  19:36:45  trung
// Bug track: 6625
// submitting for cpp_open_file, etc.
//
// Revision 1.128  1994/03/15  15:17:36  davea
// Bug track: 6615
// Optimizations, documentation, fixed big-endian
// assumptions for code using xrefSymbol, inlined
// several xrefSymbol member functions, replaced
// symbolPtr by symbolPtr, eliminated bit-field descriptions
// for fields in xrefSymbol, added member function lookup_local()
//
// Revision 1.127  1994/03/05  14:36:54  trung
// Bug track: 0
// fixes for prop to subsys, notify rtl, restoring subsys
//
// Revision 1.126  1994/02/23  20:44:25  davea
// Bug track: 6461,6480,6440
// fix hashPair::get_first() and get_next() for lt_junk at begin of chain.
// Rewrite add_links() to use add_link().
// In app_get_proj(), put relation so next time it's quicker.
// etc.
//
// Revision 1.125  1994/02/03  23:17:16  trung
// Bug track: 0
// Xref::find_all_syms
//
// Revision 1.124  1994/02/03  15:56:28  trung
// *** empty log message ***
//
// Revision 1.123  1994/02/02  19:26:42  davea
// Bug track: n/a (performance)
// Inline functions get_start_link() and get_name_length()
// Remove function get_link_real() (use get_link)
// and function  create_link() (use Link::assign)
//
// Revision 1.122  1994/01/31  15:45:04  trung
// Bug track: 0
// saving xref processing
//
// Revision 1.121  1994/01/28  22:20:04  davea
// bug 6173:
// remove the symbolPtr from all xrefEntry structs
// in each project that's being hidden.
//
// Revision 1.120  1994/01/27  15:56:49  bhowmik
// Bug track: 6068
// Fixed bugs 6068 and 6069
//
// Revision 1.119  1994/01/12  00:45:03  davea
// bugs 5670, 5693
// new function that calls xref_notifier_report for all
// the symbols in a given project
//
// Revision 1.118  1994/01/06  17:25:46  trung
// Bug track: 0
// various fixes for change prop
//
// Revision 1.117  1993/12/15  18:38:33  wmm
// Bug track: 5520
// Fix bug 5520.
//
// Revision 1.116  1993/12/14  16:15:49  trung
// Bug track: 0
// checkin fun: Xref::symbols_get_link(....)
//
// Revision 1.115  1993/12/14  00:26:05  trung
// Bug track: 0
// change prop
//
// Revision 1.114  1993/12/08  21:30:57  wmm
// Bug track: 5043
// Fix bug 5043.
//
// Revision 1.113  1993/12/02  15:29:58  davea
// Bug track: 5106
// Add default argument to member function
//   xrefSymbol::get_subproj_Xref_of_symbol
//
// Revision 1.112  1993/11/16  00:51:39  trung
// Bug track: 0
// propagation project
//
// Revision 1.111  1993/11/07  22:15:19  davea
// Bug track: 4827
// Make class testpmod a friend of both Xref and xrefSymbol
//
// Revision 1.110  1993/10/28  23:02:27  trung
// Bug track: 4993
// do save from permanent pmod to memory pmod  when unload without save
//
// Revision 1.109  1993/10/15  18:36:44  so
// Bug track: 3812
// fix bug 3812
//
// Revision 1.108  1993/08/30  17:02:00  davea
// bug 4593 - make friend out of helper function:
// get_links_global_one_layer()
//
// Revision 1.107  1993/08/26  11:47:39  trung
// fixes for constructor of ddNamechanger, add new method for xrefSymbol
//
// Revision 1.106  1993/08/23  22:08:37  trung
// propagation project
//
// Revision 1.105  1993/08/22  19:09:45  davea
// bugs 4433, 4436 - revamp the way copy_module() and delete_module() work
//
// Revision 1.104  1993/08/19  02:11:54  trung
// add new decl for dd_get_any_assoc
//
// Revision 1.103  1993/08/18  00:02:57  trung
// for rename propagation
//
// Revision 1.102  1993/08/17  17:18:13  davea
// bug 4389 - xref.h.C won't compile without making a friend out of
// dd_print_stat().  That function was added a few days ago, but
// the header file not changed till now.
//
// Revision 1.101  1993/08/13  22:50:43  trung
// +   int get_equivalence_obj(symbolArr & sa );
//
// Revision 1.100  1993/08/11  18:39:24  trung
// !     unsigned char is_root:1;   // true if this xref belongs to a root project.
// add new field
// to xref
//

*/
