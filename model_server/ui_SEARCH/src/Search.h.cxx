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
// Search.h.C
//------------

#include <cLibraryFunctions.h>
#include <msg.h>
#include <Search.h>

#include <RegExp.h>
#include <RTL_externs.h>
#include <smt.h>
#include <ste_smt_interface.h>
#include <ldrNode.h>
#include <viewNode.h>
#include <symbolLdrTree.h>
#include <viewGraHeader.h>
#include <ste_create_region.h>
#include <ste_send_message.h>
#include <ste_view_smod.h>

#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "genString.h"
#include "objOper.h"
#include "driver.h"
#include "ddict.h"

#include "graResources.h"
#include "steView.h"
#include "ste_interface.h"
#include "ste_category_handler.h"
#include "steHeader.h"
#include "ste_get.h"
#include "ste_array_handler.h"
#include "ste_smt_interface.h"
#include "steTextNode.h"
#include "ldrERDHierarchy.h"
#include "ldrBrowserHierarchy.h"
#include "symbolLdrHeader.h"
#include "assoc.h"

#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#include <fstream.h>
#include <ctype.h> 
#else /* ISO_CPP_HEADERS */
#include <strstream>
#include <fstream>
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include <cmd.h>

#include <psetmem.h>
#include <machdep.h>

RelClass(objArr);

int is_view_node_visible(viewTree *v);

extern "C" void string_prt(Relational * node)
{
  node->Relational::print(cout);
  msg(" ") << eom ;
  node->send_string(cout);
  msg("") << eom;
}

extern "C" void vprt(viewPtr vvv, ostream&ostr=cout)
{
  Initialize(vprt);
  viewNode*root = viewNodePtr(vvv->get_root());
  IF(!root) return;
  for(objTree *node = root->get_first(); node; node=node->get_next()){
    if(!is_viewSymbolNode(node))
      continue;
    if(is_viewConnectionNode(node))
      continue;

    viewSymbolNodePtr vn = viewSymbolNodePtr(node);
    ldrTree * ln = viewtree_get_ldrtree(vn);
    ostr << is_view_node_visible(vn) << ' ';
    vn->Relational::print(ostr);
    ostr << ' ';
    vn->send_string(ostr);
    ostr << ' ';
    if(ln){
      ln->Relational::print(ostr); ostr << ' ';
      ln->send_string(ostr);
    }
    ostr << endl;
  }
}

extern "C" void dprt(Domain*dom, ostream&ostr)
{
  ostr << "\n search domain " << dom << " no_nodes " << dom->nodes.size()
      << " text len " << dom->text_len << endl;
  node_prt(&(dom->nodes));
  for(int ii=0; ii < dom->text_len; ++ii)
    ostr << dom->text[ii];
  ostr << endl;
}

extern "C" void sprt(Search*srch, ostream&ostr)
{
  ostr << "\n Search struct " << srch << " domain " << &(srch->domain)
     << " no matches " << srch->num_matches 
     << " constraints ";

  ostr << oct << srch->constraints << endl;

  ostr << "pattern: " << srch->pattern << endl;
  node_prt(&(srch->constraint_nodes));
}

Search::Search(viewPtr view_header, char* search_pattern, int consts) :
   curr_view(view_header), num_matches(0), pattern(NULL),
   constraints(consts), hit_bank(0), is_ste_view(0), is_ste_tree(0), is_smt_tree(0)
{
    Initialize(Search::Search);

    setupHeaderAndRoot();
    if (!search_header)
	return;

    // build domain
    domain.text = NULL;
    search_header->search(search_root, (void*)this, 0, 0);
    // Add a carrage return BEFORE the first character to avoid a purify abr error in
    // the Regexp class when using anchored searches. -jef
    if (domain.text == 0) return;
    char *temp = (char*) psetmalloc(strlen(domain.text)+2);
    temp[0] = NULL;
    strcat(temp, "\n");
    strcat(temp, domain.text);
    free(domain.text);
    domain.text = temp + 1;

    hits.reset();
    hit_bank = new RTL(0);
    RTLNodePtr hit_bank_root = checked_cast(RTLNode,hit_bank->get_root());
    hit_bank_root->clear();


    if (!domain.text)
        return;

    Search::setupPattern(search_pattern, !is_steView(curr_view));
    if (!pattern || !*pattern)
	return;

//    constraint_nodes.removeAll();

    seekHits();
  DBG
  {
     static ofstream fs("qs.out");
     sprt(this, fs);
     dprt(&domain, fs);
     vprt(view_header, fs);
  } 
}

Search::~Search()
{
    Initialize(Search::~Search);
    
    if (hit_bank)
	obj_delete(hit_bank);
    if (pattern)
        delete pattern;
}
 
int Search::node_is_constrained(commonTree*root) const
{
 Initialize(Search::node_is_constrained);

 smtTreePtr smt_decl = NULL;
    smtTreePtr smt = NULL;
    symbolPtr sym = root;
    int ddkind = -1;
    if(is_ldrNode(root)){
       ldrNodePtr lroot = ldrNodePtr(root);
       if(is_ldrFuncCallNode(root))
           lroot = ldrNodePtr(lroot->get_parent());

       lroot->get_symbolPtr(&sym);
    }
    sym = sym.get_xrefSymbol();

    if (sym.isnotnull()){
       ddkind = sym->get_kind();
    } else if(is_smtTree(root)){
        smt = smtTreePtr(root);
        // find the declaration
        if (smt->type == SMT_token && smt->extype == SMTT_ident){
            
            smt_decl = checked_cast(smtTree,
				    get_relation(declaration_of_reference,smt));
            if (!smt_decl)
                smt_decl = checked_cast(smtTree, get_relation(decl_of_id, smt));
        }
    }
 
    int accept = 0;

    if (smt_decl) {
        if (constraints < SRCH_KEYWORD)
	    accept = 1;
	else if ((constraints & SRCH_VDECL) &&
	    (smt_decl->type == SMT_decl || smt_decl->type == SMT_sdecl ))
	    accept = 1;
	else if ((constraints & SRCH_FDECL) &&
	    ((smt_decl->type == SMT_fdecl) || (smt_decl->type == SMT_fdef)))
	    accept = 1;
	else if ((constraints & SRCH_PDECL) && (smt_decl->type == SMT_pdecl))
	    accept = 1;
	else if ((constraints & SRCH_CDECL) && (smt_decl->type == SMT_cdecl))
	    accept = 1;
	else if ((constraints & SRCH_EDECL) && (smt_decl->type == SMT_edecl))
	    accept = 1;

    } else if(ddkind>0) {
        if (constraints < SRCH_KEYWORD)
	    accept = 1;
    	else if ((constraints & SRCH_VDECL) && (ddkind == DD_VAR_DECL ))
	    accept = 1;
	else if ((constraints & SRCH_FDECL) && (ddkind == DD_FUNC_DECL) )
	    accept = 1;
	else if ((constraints & SRCH_PDECL) && (ddkind == DD_PARAM_DECL))
	    accept = 1;
	else if ((constraints & SRCH_CDECL) && (ddkind == DD_CLASS))
	    accept = 1;
	else if ((constraints & SRCH_FIELD) && (ddkind == DD_FIELD))
	    accept = 1;
	else if ((constraints & SRCH_MACRO) && (ddkind == DD_MACRO))
	    accept = 1;
	else if ((constraints & SRCH_EDECL) && 
		  ((ddkind == DD_ENUM)||(ddkind==DD_ENUM_VAL) ))
	    accept = 1;
    } else if (smt && smt->type == SMT_token) {
        unsigned int constr = (constraints < SRCH_KEYWORD)? 0xffff : constraints;
	if ((constr & SRCH_KEYWORD) && (smt->extype == SMTT_kwd))
	    accept = 2;
	else if ((constr & SRCH_CONST) && (smt->extype == SMTT_const))
	    accept = 3;
	else if ((constr & SRCH_STRING) && (smt->extype == SMTT_string))
	    accept = 4;
	else if ((constr & SRCH_COMMENT) && (smt->extype == SMTT_comment))
	    accept = 5;
	else if ((constr & SRCH_COMMENT) && (smt->extype == SMTT_commentl))
	    accept = 5;
        else if ((constraints < SRCH_KEYWORD))
	    accept = 10;     
      }
 return accept;
}
static const char* smt_token_names[] = 
   {"", "Symbol", "keyword", "const", "string", "comment"};
static const char * alpha_numeric = "[a-zA-Z0-9_]";
static void set_nw(char* pat, int&len)
{
  strcpy(pat+len, alpha_numeric);
  len += strlen(alpha_numeric);
}

void Search::setupPattern (char *user_pattern, int line_anchors)
{
    int len = 0;
    int ulen = user_pattern ? strlen(user_pattern) : 0;
    if(ulen==1 && user_pattern[0] == '*'){
      user_pattern[0] = '\0';
      ulen = 0;
    }
    if (pattern)
	delete pattern;
    
    pattern = new char [(ulen+2)*strlen(alpha_numeric)+60];  // Room for word RE and anchors
    pattern[0] = 0;

//boris: fix for bug7779. I use different  RegExp to match beginning of a line
    if (line_anchors){
/*	strcat(pattern, "^([^\\n]*)"); */
	strcat(pattern, "^.*");
	len = strlen(pattern);
    } else {
        strcat(pattern, "[ \t]*");
        len = strlen(pattern);
	set_nw(pattern, len);
	pattern[len++] = '*';
    }

//    if (constraints & SRCH_WORD) 
//        strcat (pattern, "\\<"); 
 
    if (constraints & SRCH_PAT_OFF) {
//        len  = strlen(pattern);
        while (*user_pattern) {
            if (isalnum(*user_pattern))
                pattern[len++] = *user_pattern;
            else if (*user_pattern == '.') {
                pattern[len++] = '\\';
                pattern[len++] = '.';
            } else if (*user_pattern == '*') { //### was "."
                pattern[len++] = '\\';
                pattern[len++] = '*';
            } else if (*user_pattern == '+') {
                pattern[len++] = '\\';
                pattern[len++] = '+';
            } else if (*user_pattern == '\\') {
                pattern[len++] = '\\';
                pattern[len++] = '\\';
            } else {
                pattern[len++] = '\\';
                pattern[len++] = *user_pattern;
            }
            user_pattern++;
        }
        pattern[len] = 0;
    } else if (*user_pattern == '"') {
        strcpy (pattern, user_pattern+1);
        len  = strlen(pattern) - 1;
        if (pattern[len] == '"')
            pattern[len] = 0;
    } else {
//        len  = strlen(pattern);
        char ch;
//        if(ulen==0 && !line_anchors){
        if(ulen==0){
          set_nw(pattern, len);
          pattern[len++] = '+';
	}  else  while ((ch=*user_pattern)) {
            switch (ch) {
            case '?':
                set_nw(pattern, len);
		break;
                 
	    case '*':
                set_nw(pattern, len);
                pattern[len++] = '*';
                break;
     
	    case '.':
	    case '+': 
	    case '/':   case '(':  case ')':
		pattern[len++] = '\\';
		pattern[len++] = ch;
                break;

            default:
                pattern[len++] = ch;
            }
            user_pattern++;
        }
        pattern[len] = '\0';
    }
 
 
//    if (constraints & SRCH_WORD)
//        strcat (pattern, "\\>");
 
    if (line_anchors){
	strcat (pattern, ".*$");
    } else {
//      len  = strlen(pattern);
      set_nw(pattern, len);
      pattern[len++] = '*';
      pattern[len] = '\0';
    }

    if (constraints & SRCH_INEXACT) {
        char *ptr = domain.text;
        while (*ptr) {
            if (isupper(*ptr))
                *ptr = (*ptr) + ('a'-'A');
            ptr++;
        }
 
        ptr = pattern ;
        while (*ptr) {
            if (isupper(*ptr))
                *ptr = (*ptr) + ('a'-'A');
            ptr++;
        }
    }
}
 
static int find_token_number(int pos, int *string_marks, int num_marks)
{   // |---->|----->
    int last_ind = num_marks - 1;
    int lb,mb,ub;
    lb = 0;
    ub = last_ind;
    for(;;){
        mb = (ub+lb) / 2;
        if(mb==lb) break;
        if (pos < string_marks[mb]) ub = mb;
        else lb = mb;
    } 
                 
    // skip empty tokens
    while (mb < last_ind && (string_marks[mb] == string_marks[mb+1]))
            mb++;

    return mb;
}

static void get_word_char_nums(int &word_num, int &char_num, int node_start,
			       int point, char *string)
{
    word_num = 0;
    char_num = 0;
    int ind = node_start;
    int state = 0;
    while (ind <= point && string[ind])
    {
	// CAUTION !!! Don\\'t change following condition without informing LISP/EPOCH persons !!!!!!!!!!
        if (string[ind] == ' ' || string[ind] == '\n' || string[ind] == '\t')
        {
            switch (state)
            {
              case 0:
                break;
              case 1:
                state = 2;
                char_num = -1;
                break;
              default:
                state = 0;
                word_num++;
                char_num = 0;
                break;
            }
        }
        else
        {
            switch (state)
            {
              case 0:
                state = 1;
                char_num = 0;
                break;
              case 1:
                char_num++;
                break;
              default:
                state = 1;
                word_num++;
                char_num = 0;
                break;
            }
        }
        ind++;
    }
    
//    printf("word: %d, char: %d\n\n", word_num, char_num);
}

void Search::setupHeaderAndRoot()
{
    Initialize(Search::setupHeaderAndRoot);

    search_header = NULL;
    search_root   = NULL;

    if (!curr_view)
	return;

    ldrPtr ldr_header = curr_view->get_ldrHeader();
    appPtr app_header = ldr_header ? ldr_header->get_appHeader() : NULL;
    ldrTreePtr ldr_root =
	    ldr_header ? checked_cast(ldrTree, ldr_header->get_root()) : NULL;
    appTreePtr app_tree = ldr_root ? ldr_root->get_appTree() : NULL;

    int view_type = curr_view->get_type();

    switch (view_type) {
      case Rep_VertHierarchy:
      case Rep_MultiConnection:
      case Rep_FlowChart:
      case Rep_Tree:
      case Rep_Grid:
      case Rep_ERD:
      case Rep_DGraph:
      case Rep_OODT_Inheritance:
      case Rep_OODT_Relations:
      case Rep_OODT_Scope:
      case Rep_DataChart:
      case Rep_SubsystemMap:
      case Rep_TextDiagram:
      case Rep_TextFullDiagram:
	// Search the ldr
	search_header = ldr_header;
        search_root = ldr_root;
        is_ste_view = 0;
	break;

      case Rep_TextText:
      case Rep_SmtText:
	// Search the app
	search_header = app_header;
	search_root = app_tree;
        is_ste_view = (view_type == Rep_TextText) ? 1 : 2;
	break;

      default:
      case Rep_RawText:
	// We do not know how to search these
	search_header = NULL;
	break;
    }
    is_ste_tree = app_tree ? is_steTextNode(app_tree) : 0;
    is_smt_tree = app_tree ? is_smtTree(app_tree) : 0;
    if (!search_root)
        search_root = checked_cast(commonTree, search_header->get_root());
}

void Search::seekHits()
{
  Initialize(Search::seekHits);
  if (domain.nodes.size() == 0)
    return;

  int within_constraints = 0;

  if ((constraints < SRCH_KEYWORD) && is_ste_tree)
    within_constraints = 10;

  genString new_pattern = pattern;

  Regexp *pat = new Regexp(new_pattern);
  
  int hit_start = 0;
  int hit_end = 0;
  int start_node_num = 0;
  int end_node_num;
  int ind = 0;
  while (1) {
    hit_start = pat->Search(domain.text, domain.text_len, ind, 
			    domain.text_len - ind );
    if (hit_start < 0) // then no more matches
      break;
    hit_end = pat->EndOfMatch();
    ind = (hit_end > hit_start) ? hit_end : hit_start + 1;
    
    if (isspace(domain.text[hit_start])) {
      int start_h = hit_start;
      while ((start_h < hit_end) && isspace(domain.text[start_h]))
	start_h++;
      if (start_h < hit_end)
	hit_start = start_h;
    }
	
    start_node_num=find_token_number(hit_start,
				     domain.text_marks,domain.nodes.size() + 1);
    end_node_num = (hit_end > hit_start)?
      find_token_number(hit_end,domain.text_marks,domain.nodes.size() + 1):
	start_node_num;
    int hit_type = within_constraints;
    commonTree* cmt = NULL;
    if(!hit_type)
     for (int snn = start_node_num; snn <= end_node_num; ++snn) {
      cmt = checked_cast(commonTree, domain.nodes[snn]);
      hit_type = node_is_constrained(cmt);
      if(hit_type)
		break;
    }
    if (hit_type) 
      addHit(hit_start, hit_end, start_node_num, hit_type, cmt);
  }
}

void Search::addHit(int hit_start, int hit_end, int start_node_num, int ntype,
   commonTreePtr cnd)
{
    Initialize(Search::addHit);

    RTLNodePtr hit_bank_root = checked_cast(RTLNode,hit_bank->get_root());
    int sz = domain.nodes.size() ;
    if (!start_node_num)
	start_node_num=find_token_number(hit_start, domain.text_marks, sz + 1);
    int end_node_num = find_token_number(hit_end, domain.text_marks, sz + 1);
    int node_start     = domain.text_marks[start_node_num];
    int end_node_start = domain.text_marks[end_node_num];

    rtl_add_obj (hit_bank_root, (ObjPtr)domain.nodes[start_node_num]);
    rtl_add_obj (hit_bank_root, (ObjPtr)domain.nodes[end_node_num]);

    aHit* hit = hits.grow(1);
    OSapi_bzero(hit, sizeof(aHit));

    hit->hit_bank_node = (ObjPtr)domain.nodes[start_node_num];
    hit->start = (hit_start - node_start);
    hit->end = (hit_end - end_node_start);

    int word_num, char_num;
    get_word_char_nums(word_num, char_num, node_start, hit_start, domain.text);
    hit->start_node = domain.nodes[start_node_num];
    hit->word_start = word_num;
    hit->char_start = char_num;

    get_word_char_nums(word_num, char_num, end_node_start, hit_end, domain.text);
    hit->end_node = domain.nodes[end_node_num];
    hit->word_end = word_num;
    hit->char_end = char_num;
    hit->node_type = ntype;
    hit->constrained_node = cnd;
    num_matches++;
  }

int Search::matches()
{
    return num_matches;
}

int Search::select(int ind)
{
    Initialize(Search::select);

    if ((ind > 0) && (ind <= num_matches))
    {
	aHit hit;
	if (get_hit(ind, hit))
	    return select_hit (hit, 0);
    }
    return 0;
}

// int ste_print_node_def (appTree *nd);
// int ste_print_node_def(symbolPtr &symp);

int Search::select_hit(aHit& hit, int add_selection)
{
    Initialize(Search::select_hit);
    
    commonTreePtr start_nd = checked_cast(commonTree, hit.start_node);
    commonTreePtr end_nd = checked_cast(commonTree, hit.end_node);

    if(start_nd == NULL)
	return 0;

    if(is_ste_view) {
      if(is_appTree(start_nd) && is_appTree(end_nd)){
	if (curr_view->get_type() == Rep_TextText) {
	  OperPoint start_point((Obj *)appTreePtr(NULL), FIRST);
	  OperPoint end_point((Obj *)appTreePtr(NULL), FIRST);

	  start_point.node = (Obj *)start_nd;
	  start_point.x = hit.word_start;
	  start_point.offset_of = hit.char_start;
	  start_point.subtype = 1;
    
	  end_point.node = (Obj *)end_nd;
	  end_point.x = hit.word_end;
	  end_point.offset_of = hit.char_end;
	  end_point.subtype = 1;
	    
	  ste_interface_unselect(viewPtr(NULL));
	  steRegionPtr region = ste_create_region(
			     steViewPtr(curr_view), &start_point, &end_point);
	  steViewPtr(curr_view)->insertion_point(&start_point);
	} else {
	  OperPoint start_point((Obj *)appTreePtr(NULL), FIRST);
	  OperPoint end_point((Obj *)appTreePtr(NULL), FIRST);

	  start_point.node = (Obj *)start_nd;
	  start_point.offset_of = hit.start;
	  start_point.subtype = 0;
    
	  end_point.node = (Obj *)end_nd;
	  end_point.offset_of = hit.end;
	  end_point.subtype = 0;

	  ste_interface_unselect(viewPtr(NULL));
	  steRegionPtr region = ste_create_region(
		    steViewPtr(curr_view), &start_point, &end_point);
	  start_point.node = (Obj *)start_nd;
	  start_point.offset_of = hit.start;
	  start_point.subtype = 0;
	  steViewPtr(curr_view)->insertion_point(&start_point);
	}
      }
    } else {
        viewGraHeaderPtr grav = checked_cast(viewGraHeader,curr_view);
        commonTreePtr sn = checked_cast(commonTree, hit.start_node);
        grav->select_node(sn, add_selection, hit.start, hit.end);
    }
    if(hit.constrained_node  &&  hit.constrained_node->relationalp()
	  && is_appTree(RelationalPtr(hit.constrained_node))){
      appTreePtr app_node = appTreePtr(hit.constrained_node);
      if(hit.node_type == 1){
	ste_print_node_def(app_node);
      } else if(hit.node_type != 10 && is_smt_tree){
           smtTree* node = smtTreePtr(app_node);
           ostrstream ostr;
           node->send_string(ostr);
	   ostr << ends;
           char * node_txt = ostr.str();
           genString text = smt_token_names[hit.node_type];
	   text +=' ';
	   text += node_txt;
	   ste_print_message((char*)text);
           delete node_txt;
         } else {
	   ste_print_message("");
	 }
    }
    return 1;
}

int Search::get_hit (int ind, aHit& hit)
{
    Initialize(Search::get_hit);

    if ((ind < 1) || (ind > num_matches))
	return 0;

    hit = *hits[ind - 1];

    RTLNodePtr hit_bank_root = checked_cast(RTLNode,hit_bank->get_root());
    symbolArr *hit_bank_arr = &hit_bank_root->rtl_contents();
    int hit_bank_size = hit_bank_arr->size();
 
    // Kludge: check the hit_bank rtl to make sure that the object has not
    // been deleted.  can not use rtl_includes because that will call
    // collectionp() on obj
    int foundit = 0;
    commonTreePtr obj;
    obj = checked_cast(commonTree,hit.start_node);
    int i;
    for(i = 0; i < hit_bank_size; ++i)
      if((*hit_bank_arr)[i]==(symbolPtr)obj)
      {
          foundit = 1;
	  break;
      }
    if (foundit && hit.end_node) {
	// check the end node too, if there is one
	foundit = 0;
	obj = checked_cast(commonTree,hit.end_node);
	for(i = 0; i < hit_bank_size; ++i)
	    if((*hit_bank_arr)[i]==(symbolPtr)obj)
	    {
		foundit = 1;
		break;
	    }
    }
 
    return (foundit);
}

objArrPtr RTL::search (commonTreePtr, void* search_context, int, int)
{
// Quick Search does not really search RTLs, this is historical code. -jef
    Initialize(RTL::search);

    Search* context = (Search*)search_context;

    RTLNodePtr rtlnode;
    symbolArr *contents;

    ostrstream buffer;
 
    rtlnode = checked_cast(RTLNode,get_root());
    if (rtlnode) {

	int num_nodes = rtlnode->rtl_contents().size();
        if (num_nodes > 0) {
 
            contents = &(rtlnode->rtl_contents());
            int i = 0;
            Obj *e1;
            ForEachS(e1, *contents)
                  {++i;}
            if(context->domain.text_marks) {
                delete context->domain.text_marks;
                context->domain.text_marks = 0;
            }
            context->domain.text_marks = new int[i+1]; // used to be num_nodes + 1
 
            Obj *e2;
            ForEachS(e2,*contents) {
                context->domain.text_marks[i] = buffer.pcount ();
                char *format = rtlnode->get_format_spec();
                if (format)
                    rtlnode->rtl_format_string ((commonTree *)checked_cast(commonTree,e2),
                        (char *)rtlnode->get_format_spec(),
                        (ostream &)buffer);
                else
                    buffer << rtlnode->get_name();
            }
 
            context->domain.text_len = context->domain.text_marks[i] = buffer.pcount ();
            buffer << ends;
            context->domain.text = buffer.str();
        }
    }
 
    return 0;
}

static void get_search_tok_list(objTreePtr root, objArr &search_nodes)
{
    Initialize(get_search_tok_list);

    objTreePtr child;
 
    child = root->get_first();
    if (!child )
        search_nodes.insert_last (root);
         
    for (; child; child = child->get_next())
        get_search_tok_list (child, search_nodes);
}
static int ste_end_of_struct(steTextNode*txt) 
{
   if(txt->get_node_type() != steREG)
      return 1;
   if(txt->get_next())
      return 0;

   txt = steTextNodePtr(txt->get_parent());

   if(!txt) return 1;

   return ste_end_of_struct(txt) ;
     
}
// --------------------------------------------------------------------
// commonTree::search
// --------------------------------------------------------------------
objArrPtr commonTree::search (commonTreePtr root, void* search_context, int, int)
{
  Initialize(commonTree::search);
  Search* context = (Search*)search_context;
  
  Obj *el;
  ostrstream buffer;
 
  if (!root)
    root = checked_cast(commonTree,get_root());
 
  if (!root)  return 0;

  int smt_src = is_smtTree(root) && context->curr_view && is_steView(context->curr_view);
  int ste_src = is_steTextNode(root) && context->curr_view && is_steView(context->curr_view);

    context->domain.nodes.removeAll();
    get_search_tok_list (root, context->domain.nodes);
         
    int num_nodes = context->domain.nodes.size();
    if (num_nodes > 0) {
      context->domain.text_marks = new int[num_nodes+1];
      int ind=0;
      ForEach(el,context->domain.nodes) {
	context->domain.text_marks[ind] = buffer.pcount ();
	commonTree * node = checked_cast(commonTree, el);
	if (smt_src)
	  ste_smt_send_native(smtTreePtr(node), buffer);
	else{
 	  node->send_string (buffer);
          if(ste_src && ste_end_of_struct(steTextNodePtr(node)))
             buffer << '\n';
        }	
	ind++;
      }
      context->domain.text_len = context->domain.text_marks[ind] = buffer.pcount ();
      buffer << ends;
      context->domain.text = buffer.str();
    }
  return 0;
}


static void get_ldr_tok_list (ldrTreePtr root, objArr &search_nodes)
{
  Initialize(get_ldr_tok_list);
  
  ldrTreePtr child;
  child = checked_cast(ldrTree,root->get_first());

  //Check this node

  if (is_ldrSymbolNode(root)) {
    ldrSymbolNode* lsn = ldrSymbolNodePtr(root);
    objArr* toks = lsn->get_tok_list(1);
    if(toks){
      Obj*el;
      {ForEach(el, *toks)
	search_nodes.insert_last(el);
      }
      delete toks;
    } else {
      search_nodes.insert_last(root);
    }
  }
}

static void get_view_tok_list (objArr &search_nodes, viewPtr view_hdr)
{
  Initialize(get_view_tok_list);
  viewNode*vroot = viewNodePtr(view_hdr->get_root());
  for(objTree *node = vroot->get_first(); node; node=node->get_next()){
    if(!is_viewSymbolNode(node))
      continue;
    if(is_viewConnectionNode(node))
      continue;

    viewSymbolNodePtr vn = viewSymbolNodePtr(node);

    if(!is_view_node_visible(vn))
      continue;

    ldrTree * ln = viewtree_get_ldrtree(vn);
    if(ln)
      get_ldr_tok_list(ln, search_nodes);
  }
}

objArrPtr ldr::search(commonTreePtr root, void* search_context, int, int)
{
    Initialize(ldr::search);

    Search* context = (Search*)search_context;

    Obj *el;
    ostrstream buffer;

    // Start with a carrage return so that all nodes can be line anchored. -jef
    buffer << "\n";
 
    if (!root)
        root = checked_cast(commonTree,get_root());
 
    if (root) {
        context->domain.nodes.removeAll();

        get_view_tok_list(context->domain.nodes, context->curr_view);
         
	int num_nodes = context->domain.nodes.size();
        if (num_nodes > 0) {
 
            context->domain.text_marks = new int[num_nodes+1];
 
	    boolean insert_seperators = 1;
            int i=0;
            ForEach(el,context->domain.nodes) {
                context->domain.text_marks[i] = buffer.pcount ();
                (checked_cast(commonTree,el))->send_string (buffer);
                if (insert_seperators)
                    buffer << "\n";
                i++;
            }
 
            context->domain.text_len = context->domain.text_marks[i] = buffer.pcount ();
            buffer << ends;
            context->domain.text = buffer.str();
        }
    }
 
    return 0;
}
 
/*
   START-LOG-------------------------------------------

   $Log: Search.h.C  $
   Revision 1.21 2000/07/12 18:14:54EDT ktrans 
   merge from stream_message branch
// Revision 1.9  1994/04/27  01:00:07  trung
// Bug track: 0
// quick search, etc.
//
// Revision 1.8  1994/03/28  14:30:26  kws
// psetmalloc - compliance
//
// Revision 1.7  1994/03/05  16:48:15  bhowmik
// Bug track: 6116
// Fixed attributed quick search.
//
// Revision 1.6  1994/02/28  19:39:21  andrea
// Bug track: 6535
// fixed 6535
//
// Revision 1.5  1994/02/14  23:04:05  azaparov
// Bug track: N/A
// Inserted warning comment
//
// Revision 1.4  1994/02/09  17:55:41  builder
// Port
//
// Revision 1.3  1994/02/02  14:25:38  builder
// Port
//
// Revision 1.2  1994/01/18  16:00:24  jon
// fdfd
//
// Revision 1.1  1993/12/11  16:46:32  jon
// Initial revision
//

   END-LOG---------------------------------------------

*/

