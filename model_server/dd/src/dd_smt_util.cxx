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
//  files

#include "RTL.h"
#include "RTL_externs.h"
#include "astTree.h"
#include "ast_cplus_tree.h"
#include "cLibraryFunctions.h"
#include "ddict.h"
#include "ddSymbol.h"
#include "genString.h"
#include "objRelation.h"
#include "proj.h"
#include "smode_categories.h"
#include "smt.h"
#include "smt_categories.h"
#include "transaction.h"
#include "xref.h"

static objArr sa;  // XXX: unused?

smtTreePtr dd_smt_find_tok( int len, char const *string,
			   smtTreePtr start_tok, smtTreePtr end_tok)
{
  smtHeaderPtr sh;
  smtTreePtr tok;

  Initialize(dd_smt_find_tok);
  tok= start_tok;
  while ( -1 ) {
    if ( tok->tlth == len ) {
      sh= checked_cast(smtHeader,tok->get_header());
      if ( memcmp( sh->srcbuf+tok->tbeg, string, len) == 0 )
	break;
    }
    if ( tok == end_tok ) {
      tok= checked_cast(smtTree,NULL);
      break;
    }
    tok= checked_cast(smtTree,tok->get_next_leaf());
  } 
  ReturnValue(tok);
}


smtTreePtr dd_smt_find_tok( int len, char const *string, smtTreePtr start_tok)
{
  smtHeaderPtr sh;
  smtTreePtr tok;

  Initialize(dd_smt_find_tok);
  tok= start_tok;
  while ( tok ) {
    if ( tok->tlth == len ) {
      sh= checked_cast(smtHeader,tok->get_header());
      if ( memcmp( sh->srcbuf+tok->tbeg, string, len) == 0 )
	break;
    }
    tok= checked_cast(smtTree,tok->get_next_leaf());
  } 
  ReturnValue(tok);
}


smtTreePtr dd_get_local_smt(ddElementPtr ep) 
{
  Initialize(dd_get_local_smt);
//  return dd_get_smt_real(ep, false, true);
  ddElement* def_dd = ep->get_definition();
  if (def_dd == 0) return 0;
  smtTreePtr smt= checked_cast(smtTree,get_relation(def_smt_of_dd,def_dd));
  return smt;

}

smtTree* dd_get_smt_from_line(ddElement* ddel)
{
  Initialize(dd_get_smt_from_line);
  
  smtTree* node = NULL;

  int dd_line = ddel->xref_index;
  appPtr ah = ddel->get_main_header();
  if (dd_line > 0 && ah && is_smtHeader(ah)) {
    smtHeader* h = checked_cast(smtHeader, ah);
    node = h->tree_at_line (dd_line);
  }

  return node;
}

smtTreePtr dd_get_loaded_smt(ddElementPtr ep) 
{
  Initialize(dd_get_loaded_smt);

  ddElement* def_dd = ep->get_loaded_definition();
  if (def_dd == 0) 
    return 0;

  objTree* obj = checked_cast(objTree, get_relation(def_smt_of_dd,def_dd));
  if (!obj)
    obj = dd_get_smt_from_line(def_dd);

  if ( obj != DB_NULL )
    return ( checked_cast(smtTree, obj) );
  else
    return NULL;
}

smtTree* dd_get_any_smt_decl(ddElement* ep)
{
  Initialize(dd_get_any_smt_decl);

  smtTreePtr smt;
  objSet os;
  ObjPtr ob;
  smt = checked_cast(smtTree, get_relation(def_smt_of_dd, ep));
  if (smt)
    return smt;
  os = get_relation(ref_smt_of_dd, ep);
  ForEach(ob, os) {
    smt = checked_cast(smtTree, ob);
    smt = checked_cast(smtTree, get_relation(decl_of_id, smt));
    if (smt)
      return smt;
  }
  return NULL;
}

smtTreePtr dd_get_smt(ddElementPtr ep) 
{
  Initialize(dd_get_smt);
//  return dd_get_smt_real(ep, true, true);
  ddElement* def_dd = ep->get_definition();
  if (def_dd == 0) {
     return checked_cast(smtTree, get_relation(smt_of_dd, ep));
  }
  
  smtTreePtr smt= checked_cast(smtTree,get_relation(def_smt_of_dd,def_dd));
  if (!smt) { 
    return dd_get_any_smt_decl(ep);
  }
  return smt;
}

ddElement* smt_get_dd(smtTree* smt)
{
  Initialize(smt_get_dd);
  ddElement* ep;

  if (smt && (smt->extype == SMTT_macro)) {
      smtTree *par = checked_cast(smtTree, smt->get_parent());
      if(par && par->extype != SMT_m_include)
	  smt = par;
  }
  ep = checked_cast(ddElement, get_relation(def_dd_of_smt, smt));
  if (!ep)
    ep = checked_cast(ddElement, get_relation(dd_of_smt, smt));
  if (!ep)
    ep = checked_cast(ddElement, get_relation(decl_dd_of_smt, smt));
  if (!ep)
    ep = checked_cast(ddElement, get_relation(ref_dd_of_smt, smt));
  ReturnValue(ep);
}

ddElement* dd_get_def(ddKind k, char const *ns)
{
  Initialize(dd_get_def);

  return new_dd_find_definition(k, ns);
}


int smt_is_decl(smtTree* tok)
{
  return (tok->get_node_type() == SMT_fdecl ||
	  tok->get_node_type() == SMT_pdecl ||
	  tok->get_node_type() == SMT_fdef ||
	  tok->get_node_type() == SMT_cdecl ||
	  tok->get_node_type() == SMT_decl
	  );
}


int ddKind_get_smt_type(ddKind k)
{
  Initialize(ddKind_get_smt_type);

  int smt;
  switch (k)
  {
  case DD_TYPEDEF:
    smt = SMT_typedef;
    break;
  case DD_MACRO:
    smt = SMTT_macro;
    break;
  case DD_VAR_DECL :
    smt = SMT_gdecl ;
    break;
  case DD_FIELD : 
    smt = SMT_mem_decl;
    break;
  case DD_FUNC_DECL : 
    smt = SMT_fdecl;
    break;
  case DD_PARAM_DECL : 
    smt = SMT_pdecl;
    break;
  case DD_ENUM : 
    smt = SMT_edecl;
    break;
  case DD_ENUM_VAL : 
    smt = SMT_enum_field;
    break;
  case DD_CLASS : 
    smt = SMT_cdecl;
    break;
  case DD_UNION : 
    smt = SMT_cdecl;
    break;
  default:
    smt = 0;
    break;
  }
  return smt;
}


static smtTree* smt_get_decl_real(smtTree* tok, bool local_only)
{
  Initialize(smt_get_decl_real);

  if (smt_is_decl(tok)) return tok;
  smtTree* smt = NULL;
  ddElement* de;
  de = smt_get_dd(tok);
  if (de) {
    if (local_only)
      smt = dd_get_local_smt(de);
    else smt = dd_get_smt(de);
    return smt;
  }
// if pdecl or ldecl
  smt = checked_cast(smtTree, get_relation(declaration_of_reference, tok));
  if (smt)
    return smt;
// click on declaration itself
  smtTreePtr root = checked_cast(smtTree, tok->get_root());
  smtTree* ttok = tok;
  while (tok && tok != root) {
    if (tok->get_node_type() == SMT_stmt)
      return ttok;
    if (smt_is_decl(tok))
      break;
    tok = checked_cast(smtTree, tok->get_parent());
  }
  return tok;
}

smtTree* smt_get_local_decl(smtTree* tok) {
  return smt_get_decl_real(tok, true);
}

int smt_get_decl_type(smtTree* tok)
{
  Initialize(smt_get_decl_type);

  ddElement* de = smt_get_dd(tok);
  if (de)
    return ddKind_get_smt_type(de->get_kind());
  smtTree* smt = smt_get_local_decl(tok);
  return (smt?smt->get_node_type():0);
}

void myconvert(char const *def, char const *on, char const *nn, genString& gs)
{
  int nnl = strlen(nn);
  char nname [1024];
  char *np = nname;
  char *def1 = new char[strlen(def)+2];
  char *def2 = strdup(def);
  strcpy(def1, def);
  char const * const sep = "(),* &[]:";
  char *op = def1;
  char *op1 = def2;
  char *next_tok = def1;

  for (char const *p = strtok(def2, sep); p; p = strtok(NULL, sep)) {
    if (p - op1) {
      strncpy(np, op, p - op1);
      np += p - op1;
      op += p - op1;
      op1 += p - op1;
    }
    int pl = strlen(p);
    next_tok += pl;
    if (strcmp(p, on) == 0) {	// found the type
	if (*next_tok == ':' && *(next_tok+1) == ':' && *nn == '~') {
	    strcpy(np, nn+1);
	    np += nnl-1;
	} else {
	    strcpy(np, nn);
	    np += nnl;
	}
    }
    else if (*p == '~' && strcmp(p + 1, on) == 0) {	// found destructor
       *np++ = '~';
       strcpy(np, nn);
       np += nnl;
    }
    else {
      strcpy(np, p);
      np += pl;
    }
    op +=  pl;
    op1 += pl;
  }
  strcpy(np, op);
  delete def1;
  free(def2);
  gs = nname;
}  


void dd_replace_string(ddElement* de, char const *on, char const *nn)
{
  Initialize(dd_replace_string);

  if (de == 0) return;
  char const *p;
  if (de->get_kind() == DD_CLASS && strcmp(de->get_ddname(), on)==0) {
    de->set_name(nn);
  }
  else  if (p = strstr(de->get_ddname(), on)) {
    genString def = de->get_ddname();
    if (de->get_kind() == DD_FUNC_DECL){
      if (strcmp(def, on) == 0)
	de->set_assembler_name(nn);
      else {
	genString ns;
	char const *aname = de->get_assembler_name();
// if there is no assembly name, pass thru (see bug 4643)
        if (strlen(aname)>0) {
	    if (strlen(aname) > 4 && memcmp(aname, "_$_", 3) == 0) {
		ns.printf("_$_%d%s", strlen(nn), nn);
	    }
	    else {
		ns += nn;
		ns += "__";
		ns += aname + (2 + strlen(on));
	    }
	    de->set_assembler_name(ns);
	}
      }
    }
    genString gs;
    myconvert(def, on, nn, gs);
    de->set_name(gs);

  }
  return;
}

// need to tune up later
void dd_find_function_completion(char *name, objSet & os, ddElement* rt)
{
  Initialize(dd_find_function_completion);

  if (rt == 0) return;
  if ((rt->get_kind() == DD_FUNC_DECL) &&
      rt->get_is_def() && strstr(rt->get_ddname(), name))
    os.insert(rt);
  dd_find_function_completion(name, os, checked_cast(ddElement, rt->get_first()));
  dd_find_function_completion(name, os, checked_cast(ddElement, rt->get_next()));
  return;
}


void dd_rm_def_relation(appTree* dr)
{
  Initialize(dd_rm_def_relation);

  if (dr == 0) return;
  
  appTree* at = checked_cast(appTree,get_relation(def_smt_of_dd, dr));
  if (at)
    rem_relation(def_smt_of_dd, at, dr);
  for (at = checked_cast(appTree, dr->get_first()); at;
       at = checked_cast(appTree, at->get_next())) {       
    dd_rm_def_relation(at);
  }

  return;
}

int is_smt_restore(ddElement* dd)
{
  app* ah = dd->get_main_header();
  return (try_relation(tree_of_header, ah) != DB_NULL);
}

//---------------------------------------------------------------------------
// The following routine returns the .C file associated with a given class,
// according to the following criteria:
//
// 1. The file in which is defined the first member not defined in the same
//    file as the declaration.
//
// 2. The first file that includes the file containing the declaration (this
//    should take care of the case in which a class is newly created by
//    alphaSET with no members)
//
// 3. If all else fails, just use the file in which the class is declared (it
//    may be local to that file)
//
// It is possible to return NULL under buggy conditions.
smtHeaderPtr dd_smt_find_impl_file(ddElementPtr dd_class) {
   Initialize(dd_smt_find_impl_file);

   if (!dd_class)
      ReturnValue(NULL);

   smtTreePtr class_smt = dd_get_smt(dd_class);
   if (!class_smt)
      ReturnValue(NULL);

   smtHeaderPtr decl_hdr = checked_cast(smtHeader,class_smt->get_header());

   smtTreePtr mbr_smt = NULL;
   if (dd_class->get_kind() == DD_CLASS) {
      // New way to get the members of a class (efreed 01/26/93)

      ddElementPtr mbr;
      objArr       ddArr;
      Obj         *ob;

      dd_class->class_get_members(ddArr);

      ForEach(ob, ddArr) {
         if (ob) {
            mbr = checked_cast(ddElement, ob);
            smtTreePtr mbr_smt;
            if (mbr) {
               mbr_smt = dd_get_smt(mbr);
	       if (mbr_smt) {
		  smtHeaderPtr mbr_hdr = checked_cast(smtHeader,
                        mbr_smt->get_header());
		  if (mbr_hdr && mbr_hdr != decl_hdr) {
		     return mbr_hdr;
		  }
	       }
            }

         }
      }
   }

   fsymbolPtr def_file =
           dd_class->get_xrefSymbol()->get_def_file().get_xrefSymbol();
   if (def_file.xrisnotnull()) {
      symbolArr files;
      if (def_file->get_link(included_by, files)) {
	 symbolPtr file;
	 ForEachS(file, files) {
	    symbolArr trash;
	    if (!file.get_xrefSymbol()->get_link(included_by, trash))
	       return checked_cast(smtHeader,
                     file.get_xrefSymbol().load_file());
	 }
      }
   }

   return decl_hdr;

}


smtTreePtr find_fdecl(smtTreePtr smt) {
   Initialize(find_fdecl);
   if (smt && smt->get_node_type() == SMT_fdef) {
      smtTreePtr s = checked_cast(smtTree, smt->get_first());
      while (s) {
         if (s && s->get_node_type() == SMT_fdecl) {
            return s;
         } else if (s) {
            return find_fdecl(s);
         }
         s = checked_cast(smtTree, smt->get_next());
      }
   }
   return (smtTreePtr)NULL;
}


void smt_decl_get_smode_categories(smtTreePtr smt, smode_storage_class& sc,
				   smode_datatype& dt, char *type_str, int maxlen) 
{
  Initialize(smt_decl_get_smode_categories);
  if (smt->get_node_type() == SMT_fdef)
    smt = checked_cast(smtTree, smt->get_first());
  // unpack storage class (sc) and data type (dt) from extype
  sc = (smode_storage_class)((smt->extype & 0x70) >> 4);
  dt = (smode_datatype)      (smt->extype & 0xf);

  // fetch semantic type string from data dictionary

  ddElementPtr dd = checked_cast(ddElement,get_relation(semtype_of_smt_decl,smt));

  if (dd) {
    char const *name = dd->get_ddname();
    strncpy(type_str,name,maxlen-1);
    type_str[maxlen-1] = '\0';
  }
}

void dd_get_smode_categories(ddElement* dd, smode_storage_class& sc,
				  smode_datatype& dt, genString& type_str)
{
  Initialize(dd_decl_get_smode_categories);
  if (ddKind_is_local(dd->get_kind()))
    sc = SMODE_MEMBER;
  else
    sc = SMODE_GLOBAL;

  if (ddKind_is_local(dd->get_kind())) {
    ddElement* ty = checked_cast(ddElement, get_relation(semtype_of_smt_decl, dd));
    if (ty) {
      dt = (smode_datatype) ty->get_datatype();
      type_str = ty->get_ddname();
    }
    return;
  }
  fsymbolPtr sym = dd->get_xrefSymbol();
  if (sym.xrisnotnull() ){
    symbolArr as;
    if (sym->get_link(has_type, as)) {
      sym = as[0].get_xrefSymbol();
      dt = (smode_datatype) sym.get_datatype();
      type_str = sym.get_name();
    }
  }
}

//------------------------------------------
// smt_get_smode_categories(...)
//
// Looks for the declaration associated with the specified SMT node and
// returns smode storage class, data type, and type_string,
// based on info previously obtained from the AST (smt_categories.C).
//------------------------------------------
// smt is a token, goal is to get all info about it.
void smt_get_smode_categories(smtTreePtr smt, smode_storage_class& sc,
			      smode_datatype& dt, char *type_str, int maxlen) {
   Initialize(smt_get_smode_categories);
   if (smt == 0) return;

   // defaults. these are overridden unless we can't find the info
   type_str[0] = (char)0;
   sc          = SMODE_LOCAL_AUTO;
   dt          = SMODE_UNKNOWN_TYPE;

   smtTree* smt_decl = checked_cast(smtTree, get_relation(decl_of_id, smt));
   if (smt_decl) {  // deal with decl instance
     smt_decl_get_smode_categories(smt_decl, sc, dt, type_str, maxlen);
     return;
   }
   // reference instance
   ddElement* dd = checked_cast(ddElement, get_relation(ref_dd_of_smt, smt));
   if (dd) { // *global* information
     smt_decl = dd_get_any_smt_decl(dd);
     if (smt_decl) {
       smt_decl_get_smode_categories(smt_decl, sc, dt, type_str, maxlen);
       return;
     }
     // decl is in some other files, probably .h, trying to get info 
     // from xref and dd by itself
     genString ts;
     dd_get_smode_categories(dd, sc, dt, ts);
     if (ts.not_null() && type_str) {
       strcpy(type_str, ts);
     }
   }
   else { // local information, 
     smtTree* smt_decl = checked_cast(smtTree,
				      get_relation(declaration_of_reference, smt));
     if (smt_decl)
       smt_decl_get_smode_categories(smt_decl, sc, dt, type_str, maxlen);
   }
}

//boris: Moved from projModule::dependency_analysis
//   returns -1, if dr does not exist
//            0  on success
//   sets    save_flag = 1, if save is needed
int dd_dependency_analysis (app *head, char& save_flag)
{
    Initialize(dd_dependency_analysis__app*__char&);

    int res   = -1;
    save_flag = 0;

    head->get_root();
    ddRoot *dr = checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader,head));
    if (dr) {
	res = 0;
        for(int i=0;i<MAX_HASH;i++)
        {
            hash_element *he = dr->dd_hash[i];
            while(he)
            {
                ddElement *dd = he->el;
                int do_it = 0;
                if (dd->is_changed_since_checked_in() && !dd->is_file_not_checked_in())
                    do_it = 1;
                if (dd->is_changed_since_saved() && dd->is_file_not_checked_in())
                    do_it = 2;
                if (do_it)
                {
                    if (do_it == 1)
                        dd->set_changed_since_checked_in(0);
                    else
                        dd ->set_changed_since_saved(0);
                    ddNode::add_changed_ddElems(dd);
                    save_flag = 1;
                }
                he = he->next;
            }
        }
    }

    if ( !save_flag ) {
	appTree *t    = appHeader_get_dd (head);
	if (t && is_ddElement(t))
	    ddNode::add_changed_ddElems(checked_cast(ddElement,t));
    }
    return res;    
}

//boris: get aname of ddRoot of app_head
//       returns    -1    if no app_head
//                   1    if no ddRoot
//                   0    success
int dd_get_aname (app *ah, genString& aname)
{
    Initialize(dd_get_aname__app*__genString&);

    int res = -1;
    if (ah) {
	ah->get_root();
	ddElement *dd = checked_cast(ddElement, get_relation(dd_of_appHeader, ah));
	// return if no ddroot
	if (dd) {
	    aname = dd->get_assembler_name();
            res = 0;
        } else
            res = 1;
    }
    return res;
}

//boris: set aname on ddRoot of app_head
//       returns    -1    if no app_head
//                   1    if no ddRoot
//                   0    success
int dd_set_aname (app *app_head, genString& aname)
{
     Initialize(dd_set_aname__app*);
     int res = -1;
     if (app_head) {
	 app_head->get_root();
	 ddElement *dd = checked_cast(ddElement, get_relation(dd_of_appHeader, app_head));
	 if (dd == 0) {
	     ddRoot *dr = dd_sh_get_dr(app_head);
	     dd = checked_cast(ddElement, get_relation(dd_of_appHeader, app_head));
         }

	 if (dd) {
	     dd->set_assembler_name(aname); // update the version number
             res = 0;
         } else
             res = 1;
     }
     return res;
}


/*
// dd_smt_util.C
//------------------------------------------
// synopsis:
//      Handle various smt operations required to modify smt to
//      keep in sync with dd
//
// description:
//                  generate an empty class
// return value:
//                  0 indicates success
//                    else value from smt
//------------------------------------------
// $Log: dd_smt_util.cxx  $
// Revision 1.26 2001/11/07 14:09:18EST ktrans 
// Remove dormant project code, -verify_pdf option, and slow put w/ feedback.  See no_slow_put branch for details.
Revision 1.2.1.54  1994/07/08  15:22:50  trung
Bug track: 0
7279, 7285, 7372, 7585, 7715, 7582, demangling name for template related
putting attributes for xrefSymbol,

Revision 1.2.1.53  1994/06/27  23:46:04  aharlap
forgiving parse

Revision 1.2.1.52  1994/04/20  16:23:05  trung
bug 7055

Revision 1.2.1.51  1993/11/17  22:08:26  davea
Bug track: 5139
Remove orphan function initID, and all the first & second
level callers, stubbing those that are called from elsewhere.

Revision 1.2.1.50  1993/09/01  21:31:26  pero
*** empty log message ***

Revision 1.2.1.49  1993/08/31  23:54:07  pero
if there is no assembly name Renaming a member function
causes bad memory usage (purify error; see bug 4643

Revision 1.2.1.48  1993/08/31  03:11:56  trung
change arg proj

Revision 1.2.1.47  1993/08/23  23:48:14  wmm
Fix bug 4076.

Revision 1.2.1.46  1993/08/17  23:59:01  so
forget reparse

Revision 1.2.1.45  1993/08/17  22:45:20  so
fix bug 4203

Revision 1.2.1.44  1993/08/13  22:59:25  trung
open definition

Revision 1.2.1.43  1993/08/04  20:18:46  aharlap
don't use obsolete projectHeader

Revision 1.2.1.42  1993/07/07  21:51:53  wmm
Fix bug 3798.

Revision 1.2.1.41  1993/06/25  23:17:17  davea
bug 3692 - memory leaks

Revision 1.2.1.40  1993/05/24  00:59:52  wmm
Fix bugs 3244, 3243, 3252, and 3152.

Revision 1.2.1.39  1993/05/12  00:39:19  trung
fix typedef, and rename class

Revision 1.2.1.38  1993/04/22  13:26:23  davea
bug 3454 - added return to dd_get_smt_from_saved()

Revision 1.2.1.37  1993/04/14  11:00:13  wmm
Performance improvement: make dd_update_myself() a NOP, since xreNotifier
should already do that.

Revision 1.2.1.36  1993/04/08  15:07:04  wmm
Allow for subsystem headers in dd_get_smtHeader_affected (part
of bug 3191).

Revision 1.2.1.35  1993/04/07  19:02:07  aharlap
part of fix bug #3083

Revision 1.2.1.34  1993/04/06  23:03:09  aharlap
bug #3106

Revision 1.2.1.33  1993/04/04  19:34:48  trung
change assembler name when rename a fucntion

Revision 1.2.1.32  1993/03/30  14:47:30  davea
Fix a typo in dd_get_smtHeader_affected, to close bug # 2984

Revision 1.2.1.31  1993/03/27  01:45:52  davea
Changes for xrefSymbol* to symbolPtr

Revision 1.2.1.30  1993/03/25  02:29:49  aharlap
added get_ddname()

Revision 1.2.1.29  1993/03/03  21:50:27  efreed
put scope around ForEach, to prevent call to iterator destructor
after an item it references has been deleted.

Revision 1.2.1.28  1993/03/03  03:12:43  trung
fix getting scope smt

Revision 1.2.1.27  1993/03/01  15:31:28  wmm
Fix bug 2740 (make get_impl_file() not return a file that includes
the class declaration if that file it itself included anywhere).

Revision 1.2.1.26  1993/02/23  01:31:23  efreed
put check in for null pointers before call to strcpy

Revision 1.2.1.25  1993/02/18  23:30:49  trung
add some util functions

Revision 1.2.1.20  1993/01/11  08:06:25  efreed
> bug fix (incorrect use of char array pointer, causing memory corruption).

Revision 1.2.1.19  1993/01/09  19:52:26  trung
fix dd_get_smtHeaders_affected

Revision 1.2.1.18  1993/01/09  14:31:39  wmm
Make dd_get_def into a wrapper for new_dd_find_definition.

Revision 1.2.1.17  1993/01/07  21:55:37  trung
adjust to new xref

Revision 1.2.1.16  1993/01/07  04:13:38  efreed
put in missing function find_fdecl, missed in previous checking.

Revision 1.2.1.15  1993/01/07  03:21:42  efreed
replacement version of function smt_get_smode_categories.
this version looks in the SMT instead of the AST, to get
information which was earlier copied out of the AST to the
SMT, at the end of the parsing process. This makes the info
persistent, and allows us to eliminate the AST.

Revision 1.2.1.14  1992/12/21  21:36:22  wmm
Convert dd_smt_find_impl_file to use new XREF facilities.

Revision 1.2.1.13  1992/12/20  22:27:56  trung
fix purify bug in replace_string

Revision 1.2.1.12  1992/12/15  23:14:44  trung
adjust several functions: dd_get_smt* to new xrefsymbol

Revision 1.2.1.11  1992/12/04  14:23:37  trung
mv static objArr out of function (get_files_affected) scope,

Revision 1.2.1.10  1992/11/21  21:10:26  builder
typesafe casts.

Revision 1.2.1.9  1992/11/18  14:32:54  trung
 change get_relation to using functino

Revision 1.2.1.8  1992/11/12  00:05:33  trung
make modification to dd_get_smtHeader functio

Revision 1.2.1.7  92/10/15  18:06:04  trung
fix bug

Revision 1.2.1.6  92/10/15  17:14:16  trung
fix bug

Revision 1.2.1.5  92/10/12  19:33:17  trung
Aded function get_smt_using_exp

Revision 1.2.1.4  92/10/11  15:55:19  builder
fix revision

Revision 1.2.1.3  92/10/11  15:16:14  trung
fix bug

// Revision 1.73  92/10/09  04:33:22  trung
// fix bug
// 
// Revision 1.72  92/10/06  13:55:25  wmm
// Fix crash associated with fix of bug 1291.
// 
// Revision 1.71  92/09/30  06:55:08  trung
// fix bug
// 
// Revision 1.70  92/09/23  16:13:55  wmm
// Changes for S-mode formatting support: 1) Use decl_of_id relation
// to find more ASTs (e.g., for pure virtual functions); 2) use return
// type for member functions (was already doing so for nonmember
// functions); 3) correct AST's view of constructors as returning
// a pointer to the class.
// 
// Revision 1.69  92/09/22  17:29:15  wmm
// Add pointer, reference, and array S-mode categories.
// 
// Revision 1.68  92/09/22  15:16:45  wmm
// Fix bug #571.
// 
// Revision 1.67  92/09/21  13:41:54  kws
// Convert smt filename reference to a method
// 
// Revision 1.66  92/09/18  10:21:10  wmm
// Make dd_get_fn_element_from_string extern "C" for sergey.
// 
// Revision 1.65  92/09/15  15:32:20  trung
// fix bug
// 
// Revision 1.64  92/09/15  15:31:52  trung
// fix bug
// 
// Revision 1.63  92/08/26  16:28:31  ranger
// added dd_get_loaded_smt
// 
// Revision 1.62  92/08/21  21:15:19  trung
// fix bug
// 
// Revision 1.61  92/08/20  20:06:03  trung
// fix bug
// 
// Revision 1.60  92/08/20  19:22:16  wmm
// Add dd_smt_find_impl_file to get to the .C file for a given
// class.
// 
// Revision 1.59  92/08/19  15:12:14  trung
// fix bug
// 
// Revision 1.58  92/08/12  10:47:27  trung
// fix bug
// 
// Revision 1.57  92/08/07  10:25:42  trung
// dd support
// 
// Revision 1.56  92/08/05  15:49:56  trung
// fix bug
// 
// Revision 1.55  92/08/04  13:38:04  trung
// fix bug
// 
// Revision 1.54  92/08/04  00:59:14  builder
// Rename functions
// 
// Revision 1.53  92/08/04  00:48:12  builder
// Comment reference to project_load_id
// 
// Revision 1.52  92/08/03  14:54:14  builder
// fix bug
// 
// Revision 1.51  92/08/03  11:53:44  trung
// fix bug
// 
// Revision 1.50  92/07/29  18:28:55  wmm
// Fix minor editing bug in previous version; add log file.
// 
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/
