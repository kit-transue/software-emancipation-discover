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
// File smt_map_ast.C
// constructing smt from ast

#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include "smt.h"
#include "astTree.h"
#include "ast_cplus_tree.h"
#include "xref.h"
#include "ast_hook.h"
#include "smt_categories.h"
#include <setprocess.h>
#include <messages.h>

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

// The following redefinitions are to appease the sparc 3.0 compiler:
// ex: Can not assign int to smtTree* with NULL
#define NILL (smtTree *)0
void smt_set_ast_many_to_one(smtTree* smt, astTree* ast);
ostream & smt_get_map_stream();

smtTree * smt_find_stmt(smtHeader * h, int /*mode*/, int type, int s1, int s2)
  {
  Initialize(smt_find_stmt);

  if(s1 < 1 || s2 < 1 || s1 > s2) {
    msg("WARN: SMT tree: Trying to define statement with wrong token numbers\nFrom $1 to $2 - IGNORED") << s1 << eoarg << s2 << eom;
    //SMT tree: Trying to define statement with wrong token numbers\nFrom %d to %d - IGNORED
    return NULL;
  }

  smtTree*  t1 = h->tn_na(s1);			// Find first token 
  smtTree*  t2 = h->tn_na(s2);			// Find last token
  IF(t1 == 0 || t2 == 0)
    return (NULL);

  smtTree* cr = (smtTree *)obj_tree_common_root(t1, t2);
  Assert(cr);

  smtTree*stmt=NULL;
  while(cr){
    int st = cr->tnfirst;
    if(st != s1) break;

    int en = cr->tnmax;
    if(en != s2) break;

    if(cr->type == type){
      stmt = cr;
      break;
    }
    cr = cr->get_parent();
  }
  return stmt;
}


//void smt_kill_stmt(smtTree*);

int smt_get_id_line(astTree* decl)
{
  return ((-1) * decl->TREE_DEBUG());
}

int smt_trace_map = ((int) OSapi_getenv("SMT_TRACE_MAP"));
int smt_debug_flag = smt_trace_map || ((int) OSapi_getenv("SMT_DEBUG"));

static smtTree * last_tree = NULL;

extern "C" void smt_prt_mark(smtHeader * smt_header, astTree* root, 
   int /*mode*/,  int type, smtTree * smt_tree,
   int  start_line,  char const * start_file, int  end_line, char const * end_file)
{
  if(! smt_debug_flag) return;
  ostream & cstdout = smt_get_map_stream();

  char const *file = start_file ? start_file : end_file;

#ifdef XXX_NEWAST_PRINT
  if(root) root->print(cstdout); 
  if(smt_tree) smt_tree->Relational::print(cstdout);
#endif

  if(start_line <= 0){
    start_line = end_line -3;
  } else if (end_line <= 0) {
    end_line = start_line + 3;
  }

  if(start_line > end_line){
    int temp = start_line;
     start_line = end_line;
     end_line = temp;
  } 
     
  smt_prt_mark(smt_header, start_line, end_line, type);

  if(last_tree && ! smt_tree){
    cstdout << "previously marked node:" << '\n';
    smt_prt_tree(last_tree);
    cstdout << '\n';
  }
  cstdout << endl;
}

void smt_prt_mark(smtHeader* smt_header, int start_line, int end_line, int type)
{
  if(! smt_debug_flag) return;
  ostream & cstdout = smt_get_map_stream();

  smtTree * node;
  int ii; 
  char const *file = smt_header->get_filename();

  smtTree * first_token = smt_header->tn_na(start_line);
  smtTree * last_token = smt_header->tn_na(end_line);

  int first_line = first_token ? first_token->src_line_num : -1;
  int last_line = last_token ? last_token->src_line_num : -1;

  cstdout << smt_token_name(type) << " tokens " << start_line << '-' << end_line << 
      " lines " << first_line << '-' << last_line << ' ' << file << '\n';
   
  if(end_line < start_line + 28) {
    for(ii = start_line; ii <= end_line; ++ii){
       node = smt_header->tn_na(ii);
       if(node) node->send_string(cstdout);
    }
  } else {
    for(ii = start_line; ii <= start_line + 10; ++ii){
       node = smt_header->tn_na(ii);
       if(node) node->send_string(cstdout);
    }
    cstdout << "  ...  ";
    for(ii = end_line - 10; ii <= end_line; ++ii){
       node = smt_header->tn_na(ii);
       if(node) node->send_string(cstdout);
    }
  }
  cstdout << '\n' << endl;
}

extern "C" void smt_prt_tree(smtTree* tree)
 {
 Initialize(smt_prt_tree);

 IF(!tree) return;

 smtHeader * hh = checked_cast(smtHeader, tree->get_header());
 IF(!hh) return;

 int st = tree->tnfirst;
 int en = tree->tnmax;

 int type = tree->type;

 smt_prt_mark(hh, st, en, type);

 }

static void smt_find_macro(smtTree* tok, smtTreePtr& mac, smtTreePtr& arg)
{
  mac = arg = 0;
  if (tok == 0) return;
  if(! tok->is_macro)
     return;

  while((tok = tok->get_parent())){
     int tok_type = tok->type;
     if((tok_type == SMT_expr) && tok->is_macro){  // macroarg
        arg = tok;
     } else if(tok_type == SMT_macrocall) {
     smtTree* helpTok = tok->get_parent();
     if( NULL ==  helpTok )
     {
       mac = tok;
       break;
     }
     else
     {
       if( SMT_macrocall == helpTok->type )
	 arg = 0;
       else
       {
	 mac = tok;
	 break;
       }
     }
     }
  }
}

void smt_report_minor_problem ();

#define retmark(smt_node) return (last_tree = smt_node ? smt_node : \
( smt_prt_mark(smt_header, root, mode, type, smt_node, \
             start_line, start_file, end_line, end_file), NILL))
static int warn_msg(char const *msg)
{
  smt_report_minor_problem ();

  if(smt_debug_flag) {
    ostream & os = smt_get_map_stream();
    os << "Warning: smt_tree_map: " << msg << endl;
  }
  return 1;
}
#define warnIF(x) if((x) && (warn_msg(quote(x))))
int gen_strcmp(char const *, char const *);

smtTree* smt_mark_stree(astTree* root, int mode, int type,
			int  start_line,  char const * start_file, int  end_line,
			char const * end_file, smtHeader* smt_header2)
{
  Initialize(smt_mark_stree);
  
  ostream& cstdout = cout;

  smtTree* smt_tree = NULL;
  char const *file = start_file ? start_file : end_file;
  warnIF(file==0) return(0);
  smtHeader * smt_header = smtHeader::find(file);

  IF (! smt_header ) {
    return(NULL);
  }

  if(smt_header2){
     warnIF(smt_header != smt_header2){
#ifdef XXX_NEWAST_PRINT
         smt_header->print(cstdout, 0); cstdout << '\n';
        smt_header2->print(cstdout, 0); cstdout << endl;
#endif
	Assert(gen_strcmp(smt_header->get_filename(), smt_header2->get_filename()));
     }
   }

  
  if(start_line == 0 && end_line == 0) // accidental foreign file
     return NULL;

  if(start_line == 0){
     smtTree * cur_tok = smt_header->tn_na(end_line);
     if ((!cur_tok) ||  (cur_tok->is_macro == 1))  // macro skrew-up
       return NULL;
  }
  if(end_line == 0){
     smtTree * cur_tok = smt_header->tn_na(start_line);
     if ((!cur_tok) ||  (cur_tok->is_macro == 1))  // macro skrew-up
       return NULL;
  }

  warnIF(start_line == 0 || end_line == 0) {
     retmark(NILL);
  }

  warnIF(start_line <= 0 || end_line <= 0) {
     retmark(NILL);
  }

  if(!smt_header->parsed)
     retmark(NILL);

// ajust for macro calls: 
// ignore if completely inside macro, ajust end if overlaps
 
  int reversed_line_no = 0;

  if(start_line > end_line) {
    int temp = start_line;
    start_line = end_line;
    end_line = temp;

    reversed_line_no = 1;
  }

  smtTree * end_tok = smt_header->tn_na(end_line);
  smtTree * start_tok = smt_header->tn_na(start_line);
  smtTree* p1;
  smtTree* p2;

  if (type == SMT_expr) {
     p2 = (end_tok) ? checked_cast(smtTree, end_tok->get_parent())
	   : NULL;
     if (start_tok == end_tok && end_tok->get_extype() == SMTT_grouping
	 && end_tok->ilength() == 1 &&
	 smt_header->srcbuf[end_tok->istart()] == ')') {
	// This is a base constructor call, attempting to be mapped to
	// the ")" of the derived constructor definition.  Use the
	// title of the derived constructor definition instead.
	return p2;
     }
     if (p2 && p2->type == SMT_decl && !start_tok->get_prev()) {
	// This is a constructor call in a declaration: don't remap!
	return p2;
     }
     p1 = (start_tok) ? checked_cast(smtTree, start_tok->get_parent())
	   : NULL;
     if (p1 && p1->type == SMT_title && p2 && p2->type == SMT_block) {
	smtTree* pp1 = checked_cast(smtTree, p1->get_parent());
	smtTree* pp2 = checked_cast(smtTree, p2->get_parent());
	if (pp1 && pp1->type == SMT_fdecl && pp2 && pp2->type == SMT_fdef
	    && pp1->get_parent() == pp2 && !start_tok->get_prev()
	    && !end_tok->get_next()) {
	   // This is probably a call to operator delete() from a
	   // destructor definition.  Since we shouldn't be mapping an
	   // SMT_expr onto a whole function definition anyway, we'll
	   // just return the fdef instead of allocating a new node.
	   return pp2;
	}
     }
  }

  if (type == SMT_cdecl || type == SMT_edecl) {
     p1 = (start_tok) ? checked_cast(smtTree, start_tok->get_parent())
           : NULL;
     p2 = (end_tok) ? checked_cast(smtTree, end_tok->get_parent())
           : NULL;
     if (p1 && p1 == p2 && p1->type == SMT_title) {
        // This is a regurgitation of a previously-processed class or
        // enum declaration -- ignore it.
        return NULL;
     }
     if (p1 && p1->type == SMT_declspec && p2 && p2->type == SMT_title) {
	smtTree* pp2 = checked_cast(smtTree, p2->get_parent());
	if (pp2 && pp2->type == SMT_fdecl) {
	   // This is probably an attempt to remap a previously-mapped
	   // template function -- ignore it.
	   return NULL;
	}
     }
  }

  /* 960615 kit transue */
  // this isn't pretty: the better thing to do is to have the parser not
  // generate these decls for inlines.
  if (type == SMT_decl) {
      if (start_line == end_line && start_tok->get_extype() == SMTT_macro &&
	  start_tok->tlth == 7 &&
	  (memcmp(smt_header->srcbuf + start_tok->tbeg, "VARCHAR", 7) == 0 ||
	   memcmp(smt_header->srcbuf + start_tok->tbeg, "varchar", 7) == 0)) {
	  // this is a member decl within an ESQL VARCHAR type -- ignore
	  return NULL;
      }
     p1 = (start_tok) ? checked_cast(smtTree, start_tok->get_parent())
	   : NULL;
     p2 = (p1) ? checked_cast(smtTree, p1->get_parent())
	   : NULL;
     if (p2 && p2->type == SMT_fdecl)
	// inlines with decls have an extra decl wrapping them: get rid of it
	return NULL;
  }

  if (type == SMT_fdecl) {
     p1 = (start_tok) ? checked_cast(smtTree, start_tok->get_parent())
           : NULL;
     p2 = (end_tok) ? checked_cast(smtTree, end_tok->get_parent())
           : NULL;
     if (p1 && p2) {
	smtTree* pp1 = checked_cast(smtTree, p1->get_parent());
	smtTree* pp2 = checked_cast(smtTree, p2->get_parent());
	if (p1->type == SMT_declspec && p2->type == SMT_fdecl &&
	    pp1 && pp1 == pp2 && pp1->type == SMT_list_decl) {
	   // This is probably an attempt to remap a previously-mapped
	   // template function -- ignore it.
	   return NULL;
	}
	else if (p1->type == SMT_title && p2->type == SMT_decl &&
		 pp1 == p2 && pp2 && pp2->type == SMT_fdecl) {
	   // This is probably an attempt to remap a previously-mapped
	   // template constructor definition with ctor-init clause --
	   // ignore it.
	   return NULL;
	}
	else if (p1->type == SMT_title && p2->type == SMT_fdecl &&
		 pp1 && pp1->type == SMT_decl && pp1->get_parent() == p2) {
	   // Another form template constructors with ctor-init clauses
	   // can take -- ignore it.
	   return NULL;
	}
     }
  }

  smtTree * macro = 0;
  int macarg = 0;
  if(type == SMT_macrocall) {
      for(int tok_ind = start_line; tok_ind <= end_line; ++tok_ind) {
         smtTree * cur_tok = smt_header->tn_na(tok_ind);
	 if (cur_tok)
	   cur_tok->is_macro = 1;
      }
  } else if(type == SMT_macroarg) {
      macarg = 1;
      type = SMT_expr;
  } else {
      smtTreePtr start_macro, start_arg, end_macro, end_arg;
      smt_find_macro(start_tok, start_macro, start_arg);
      if(end_line != start_line){
          smt_find_macro(end_tok, end_macro, end_arg);
      } else {
          end_macro =  start_macro;
          end_arg = start_arg;
      }
      if(start_macro && end_macro){
        if(start_macro != end_macro)
           return NULL;
        else if(start_arg && (start_arg == end_arg))
           ;  // within one arg map in a regular fashion
        else
           macro = start_macro;
      } else if(start_macro) {
          start_line = start_macro->tnfirst;
      } else if(end_macro) {
          end_line =   end_macro->tnmax;
      }
   }
  
  if(macro) {
      if (root) {
	if (macro->get_node_type() == SMT_macrocall)
	  smt_set_ast_many_to_one(macro, root);
	else
	  smt_set_ast(macro, root);
	    smt_header->set_ast_exist(1);
	    smt_header->set_arn(0);
      }

      if(type == SMT_expr) {    // actually, call_expr
	 retmark( macro);
      } else if(type == SMT_declspec) {
         retmark(macro);        // Window * ww; // #define Window iv(Window)
      } else {
        return (NULL);
      }
  } else warnIF(reversed_line_no) {  // warning
      retmark(NILL);
  }   

// do it 

 smt_tree = smt_def_stmt(smt_header, mode, type, start_line, end_line);
 
 warnIF(!smt_tree) {
     retmark(NILL);
 }

  if(macarg){
        smt_tree->is_macro = 1;
  } else if (root){
	if (type != SMT_case_clause) {
	   bool from_template = false;
	   if ((type == SMT_list_decl || type == SMT_declspec || type == SMT_title ||
		type == SMT_superclass || type == SMT_pdecl || type == SMT_fdef ||
		type == SMT_fdecl || type == SMT_cdecl || type == SMT_cbody ||
		type == SMT_edecl || type == SMT_ebody || type == SMT_enum_field ||
		type == SMT_decl) && ((astDecl*) root)->artificial_flag) {
	      if (root->TREE_CODE() == MC_TYPE_DECL) {	// these are always artificial, sigh...
		 astIdentifier* id = ((astDecl*) root)->DECL_NAME();
		 if (id && strchr(id->IDENTIFIER_POINTER(), '<')) {
		    from_template = true;
		 }
		 else {
		    astTree* context = ((astDecl*) root)->DECL_CONTEXT();
		    while (context && context->TREE_CODE() == MC_RECORD_TYPE) {
		       astTree* name = ((astType*) context)->TYPE_NAME();
		       if (name) {
			  char const *namep = NULL;
			  if (name->TREE_CODE() == MC_IDENTIFIER_NODE) {
			     namep = ((astIdentifier*) name)->IDENTIFIER_POINTER();
			  }
			  else if (name->TREE_CODE() == MC_TYPE_DECL &&
				   ((astDecl*) name)->DECL_NAME()) {
			     namep = ((astIdentifier*) ((astDecl*) name)->DECL_NAME())->
				   IDENTIFIER_POINTER();
			  }
			  if (namep && strchr(namep, '<')) {
			     from_template = true;
			     break;
			  }
		       }
		       context = ((astType*) context)->context;
		    }
		 }
	      }
	      else from_template = true;
	   }
	   if (!from_template) {
	      smt_set_ast(smt_tree, root);
	   }
	   smt_header->set_ast_exist(1);
	   smt_header->set_arn(0);
	}
  }
  if(smt_trace_map && smt_tree) {
     smt_prt_mark(smt_header, start_line, end_line, type);
     DBG{
       if(root && (type!=SMT_expr) && (type!=SMT_pdecl)){
	 smtTree* find = smt_find_stmt(smt_header, mode, type, start_line, end_line);
	 warnIF(find != smt_tree);
       }
     }
  }

 retmark(smt_tree);

}



typedef char const *cchp;
static genArr( cchp ) headerfiles;

ostream & smt_get_map_stream()
{
    static int inited = 0;
    static ostream * smt_map_stream = NULL;

    if (!inited) {
	genString dirname = customize::getStrPref("logFileDir");
	if (!dirname.length())
	    dirname = OSapi_getenv("TMPDIR");
	if (!dirname.length())
	    dirname = "/usr/tmp";

	char hostname[1025];
	OSapi_gethostname(hostname, 1024);
	int pid = (int)OSapi_getpid();
	genString nm;
	nm.printf("%s/smt_map.%s.%05d", (char const *)dirname, hostname, pid);

	smt_map_stream = new ofstream(nm, ios::out);
	inited = 1;
    }
    return (*smt_map_stream);
}

/*
//------------------------------------------
// $Log: smt_map_ast.cxx  $
// Revision 1.41 2000/11/30 09:55:34EST sschmidt 
// Fix for bug 20091: improve output during crashes, sensitivity of tests
 * Revision 1.1  1996/12/05  15:40:29  joshj
 * Initial revision
 *
//------------------------------------------
*/

