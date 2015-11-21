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
#include <msg.h>
#include <ldrList.h>
#include <ldeList.h>
#include <customize.h>
#include <proj.h>
#include <smt.h>
#include <fileCache.h>
#include <genWild.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <steDocument.h>
#include <instanceBrowser.h>
#include <messages.h>
#include <attribute.h>
#include <format.h>

#ifndef _groupHdr_h
#include <groupHdr.h>
#endif
#include <machdep.h>
#include <objOperate.h>
#include <attribute.h>
#include <format.h>
#include <metric.h>
#include <attribute.h>
#include <elsLanguage.h>
#include <XrefTable.h>

extern "C" int abs(int);

init_relational (ldrList, ldrHeader);
init_relational(ldrListNode, ldrTree);

void gen_print_indent(ostream&, int);
char* astNode_get_src_info(symbolPtr& msym);
int ATT_line(symbolPtr& sym);

SortedList* SortedList::current_sort_list = NULL;
ldrListNode* ldrListNode::current_ldr_node = NULL;

extern const char NEW_SORT_SPEC_SIG = '#';  // The first character in a new sort spec.

//convert sort specs to a new format.
genString renew_sort_spec(const char * spec) {
    Initialize(renew_sort_spec);

    genString new_spec = "";

    if (!spec) 
      new_spec += NEW_SORT_SPEC_SIG;
	
    else if (*spec == NEW_SORT_SPEC_SIG) 
      new_spec = spec;

    else {
	new_spec += NEW_SORT_SPEC_SIG;
	
	const char * s = spec;
	while (*s) {
	    if (!isspace(*s)) {
		if ( !strncmp(s, "ns", 2) ) {
		    new_spec += "ns" ;
		    break;
		}
		new_spec += '%';
		if (s[2]) new_spec += s[2];
		else new_spec += 'f';
		// Now replace "of" with "ff" for formatted string, to avoid confusion with
		// filenames.
		// Also replace dn with of.
		if ( !strncmp(s, "of", 2) )
		  new_spec += "ff";
		else if ( !strncmp(s, "dn", 2) ) 
		  new_spec += "of";
		else {
		    new_spec += s[0];
		    new_spec += s[1];
		}
		if (s[2]) s += 3;
		else s += 2;
	    }
	    else s++;
	}
    }
    return new_spec;
}

void get_show_and_cli_filts(const char * str, genString& show, genString& cli) {
    Initialize(get_show_and_cli_filts);

    show = "";
    cli = "";
    if (str && *str) {
        const char * s = str;
        while (*s && *s != '\t') show += *s++;
        if (*s) cli = (s+1);
    }
}

ldrList::ldrList (RTLPtr rtl_head)
{
    Initialize(ldrList::ldrList);
    
    ldrListNodePtr root = NULL;
    RTLNodePtr app_root;
    
    if (rtl_head) {
	ldr_put_app(this, rtl_head);
	
	app_root = checked_cast(RTLNode, rtl_head->get_root());
	
	root = lde_list_extract (app_root); 
	root->set_appTree(app_root);
    }
    
    this->put_root (root);
}

ldrList::ldrList (ldrList const &)
{
}

ldrList::~ldrList()
{
}

void ldrList::build_selection(const ldrSelection&, OperPoint&)
{
    Initialize (ldrList::build_selection);
    Error(ERR_INPUT);
}

void ldrList::insert_obj (objInserter* old_inserter,
                               objInserter* new_inserter)
{
   Initialize (ldrRTLHierarchy::insert_obj);

   symbolPtr src_obj = old_inserter->src_obj;
   int idx = -1;
 
   ldrListNodePtr list_node = checked_cast(ldrListNode,get_root());
   if (list_node) {
       if (old_inserter->targ_obj == old_inserter->src_obj && 
	   is_RTLNode ( (RelationalPtr) old_inserter->src_obj) ) {

	   // means regenerating LDR
	   SortedList* v = list_node->views;
	   while (v) {
	       SortedList* vv = v;
	       v = v->next;
	       delete vv;
	   }
	   list_node->views = NULL;

	   lde_list_extract (list_node, checked_cast (RTLNode, src_obj)); 

	   new_inserter->type = NULLOP;
       } else {
	   
	   idx = find_ldr_index (src_obj);
	   
	   if (idx < 0) {
	       idx = list_node->insert_symbol (src_obj, idx);
	       new_inserter->type = BEFORE;
	   } else {
	       int res = list_node->update_symbol (src_obj, idx);
	       if (list_node->views && res < 0)
		   new_inserter->type = NULLOP;
	       else
		   new_inserter->type = REPLACE;
	   }
       }
   } else
       new_inserter->type = NULLOP;
 
   // Set up new inserter
   new_inserter->src_obj = NULL;
   new_inserter->targ_obj = NULL;
   new_inserter->data = (void*) idx;
}

void ldrList::remove_obj(objRemover *old_remover,
				 objRemover *)
{
   Initialize (ldrRTLHierarchy::remove_obj);
 
   commonTreePtr src_obj = checked_cast(commonTree,old_remover->src_obj);
 
   int idx = find_ldr_index (src_obj);

   if (idx >= 0) {
       ldrListNodePtr list_node = checked_cast(ldrListNode,get_root());
       if (list_node)
	   list_node->remove_symbol (src_obj, idx);
   }
}

void ldrList::hilite_obj(objHiliter *,
				 objHiliter *)
{
}

void ldrList::focus_obj(objFocuser *, objFocuser *)
{
}

void ldrList::send_string(ostream& ) const
{
}

symbolPtr* ldrList::get_obj(int index)
{
    Initialize(ldrList::get_obj);

    ldrListNodePtr list_node = ldrListNodePtr(get_root());
    if (list_node) {
	RTLNodePtr rtl_node = checked_cast(RTLNode,list_node->get_appTree());
        if (rtl_node) {
	    symbolArr& sym_arr = rtl_node->rtl_contents();
	    
	    if (index < 0 || index >= list_node->list.size())
		return NULL;
	    else 
		return &sym_arr[index];
	}
        return NULL;
    }
    return NULL;
}

int ldrList::find_rtl_index (symbolPtr node)
{
    Initialize(ldrList::find_ldr_index);

    ldrListNodePtr list_node = ldrListNodePtr(get_root());
    if (list_node) {
	for (int i=0; i<list_node->list.size(); i++) {
	    if (*list_node->list[i]->sym == node)
		return list_node->list[i]->index;
	}
    }
    
    return -1;
}

int ldrList::find_ldr_index (symbolPtr node)
{
    Initialize(ldrList::find_ldr_index);

    ldrListNodePtr list_node = ldrListNodePtr(get_root());
    if (list_node) {
	for (int i=0; i<list_node->list.size(); i++) {
	    if (*list_node->list[i]->sym == node)
		return i;
	}
    }

    return -1;
}

ldrListNode::ldrListNode()
{
    views = NULL;
}

ldrListNode::ldrListNode (ldrListNode const &) 
{
}

ldrListNode::~ldrListNode()
{
    for (int i=0; i<list.size(); i++) {
	delete list[i]->sym;
    }

    while (views) {
        SortedList* v = views;
        views = views->next;
	delete v;
    }
}

void ldrListNode::send_string(ostream& ) const
{
}


void ldrListNode::print(ostream& , int ) const
{
}


const char* ldrListNode::get_str (int index)
{
    if (index < 0 || index >= list.size())
      return NULL;
    else
      return text[list[index]->oname];
}

void ldrListNode::get_curr_name (ListRecord *rec, const unsigned char *& pref, const char *& nm)
{
    Initialize(ldrListNode::get_curr_name);
    pref = (const unsigned char *)(text[rec->prefix]);
    nm   = (const char *)(text[rec->oname]);
}

void get_sym_name_for_symbol (symbolPtr, const unsigned char*&, const char*&);

void ldrListNode::get_sym_name (symbolPtr sym,
				const unsigned char *& sym_pref,
				const char *& sym_name)
// apparently called only for figuring out name strings on each RTL.
// These are the names the user will see for each object
{
    get_sym_name_for_symbol(sym, sym_pref, sym_name);
}

void ldrListNode::fill_text (symbolPtr sym, ListRecord* rec)
{
    Initialize(ldrListNode::fill_text);

    const char* sym_name;
    const unsigned char* sym_prefix;
    get_sym_name (sym, sym_prefix, sym_name);

    int len;
    char *the_text;

    if (!sym_prefix) sym_prefix = (const unsigned char*)"";
    if (!sym_name) sym_name = "<symbol problem>";

	genString filled_name;
    char short_name[257];
    if (*sym_name == '#') {
      const char * p;
      if (p = strstr(sym_name, "#define")) {
	int jj;
	strcpy (short_name, "#define ");
	jj = 8;
	p += 8;
	while (*p && isspace(*p)) p++;
	while (*p && !isspace(*p) && (jj < 256))
	  short_name[jj++] = *(p++);
	short_name[jj] = 0;
	sym_name = short_name;
      }
    } else {
		// Because the name might be a String-Literal, we need to
		// scan for non-printing characters.  But do not escape
    	    	// other characters by inserting backslash, because the name
    	    	// in the model is already supposed to resemble the source
    	    	// representation.
		char* p = (char*) sym_name;
		bool modified = false;
		while (*p) {
			if (!isprint(*p) ) {
				genString num_string;
				num_string.printf("\\%03o", (int)*p);
				filled_name += num_string;
				modified = true;
				p++;
			} else {
				filled_name += *p++;
			}
		}
		if (modified) sym_name = filled_name;
	}

    len = strlen((const char*)sym_prefix);
    rec->prefix = text.size();
    the_text = text.grow(len+1);
    strcpy(the_text, (const char*)sym_prefix);

    len = strlen(sym_name);
    rec->oname = text.size();
    the_text = text.grow(len+1);
    strcpy(the_text, sym_name);

    return;
}

int ldrListNode::insert_symbol (symbolPtr sym, int idx)
{
    idx = list.size();

    ListRecord* instance = list.grow(1);

    instance->sym = new symbolPtr(sym);
    
    instance->index = idx;

    fill_text (sym, instance);

    insert_sorted (idx);
    return (idx);
}

void ldrListNode::remove_symbol (symbolPtr, int idx)
{
    int rtl_idx = list[idx]->index;

    if (idx >= 0 || idx < list.size()) 
	delete list[idx]->sym;
    list.remove (idx);
    remove_sorted (idx);

    for (int i=0; i<list.size(); i++) {
	if (list[i]->index > rtl_idx)
	    list[i]->index--;
    }
}

int ldrListNode::update_symbol (symbolPtr sym, int idx)
{
    if (idx < 0)
	return idx;

    ListRecord* rec = list[idx];

    int changed = 0;
    const char *curr_name, *new_name;
    const unsigned char *curr_pref, *new_pref;

    get_sym_name (sym, new_pref, new_name);
    get_curr_name (rec, curr_pref, curr_name);

    if (strcmp ((const char *)curr_pref, (const char *)new_pref) == 0 &&
	strcmp (curr_name, new_name) == 0)
	changed = -1;
    else {
	fill_text (sym, rec);
	update_sorted (idx);
    }

    if (changed < 0)
	idx = changed;

    return (idx);
}

extern char *current_name; // defined in api/src

void ldrListNode::insert_sorted_instance (SortedList* v, int idx, int sortit)
{
    needLoad temp_needLoad(false);   // When computing a metric, don't load files.

    int start = v->text.size();
    
    int len = strlen (text[list[idx]->prefix]);

    char * att_fmt = NULL;
    if (v->att_format) {
	ostrstream out;
	current_name = text[list[idx]->oname];
	v->att_format->printOut(out, *list[idx]->sym);
	out << ends;
	current_name = 0;
	att_fmt = out.str();
	if (att_fmt) len += strlen(att_fmt);
    }
    char* str = v->text.grow (len+1);
    
    strcpy (str, text[list[idx]->prefix]);
    len = strlen (text[list[idx]->prefix]);
    
    if ( att_fmt) {
	strcpy(&str[len], att_fmt);
	len += strlen(att_fmt);
	delete att_fmt;
    }
    str[len] = 0;
    
    WildRegexp yes_filter(v->show_str);
    WildRegexp no_filter(v->hide_str);
    
    char *str1 = str;
    while (isicon(*str1)) {str1++; len--;}

    if((!v->show_str.length() || (yes_filter.Match(str1,len,0) >= 0)) &&
       (!v->hide_str.length() || !(no_filter.Match(str1,len,0) >= 0)) &&
       (!v->cli_expr          || v->cli_expr->value(*list[idx]->sym)))  {
	
	SortedRecord* rec = v->sorted.grow (1);
	rec->index = idx;
	rec->offset = start;
	// Now form the string for sorting.
	genString sort_str = "";
	for (int jj = 0; jj < N_SORT_FIELDS && v->s_info[jj].field_type; jj++) {
	    if (jj>0) sort_str += '\t';  // field terminator, later changed to '\0'.
	    sort_str += v->s_info[jj].field_type->value(*list[idx]->sym);
	}
	rec->sort_offset = v->text.size();
	int len = sort_str.length();
	str = v->text.grow(len+1);
	strcpy(str, (char*)sort_str);
	while(*str) {
	    if (*str == '\t') *str = 0;
	    str ++;
	}
	
    } else
      v->text.shrink(start, len+1);
    
    if (sortit) v->sort();
}

void ldrListNode::insert_sorted (int i, int sortit)
{
    SortedList* v = views;
    while (v) {
	insert_sorted_instance (v, i, sortit);
	v = v->next;
    }
}

void ldrListNode::update_sorted (int idx, int sortit)
{
    SortedList*  v = views;

    needLoad temp_needLoad(false);   // When computing a metric, don't load files.

    while (v) {
	int j;
	for (j=0; j<v->sorted.size(); j++) {
	    if (v->sorted[j]->index == idx)
		break;
        }

	if ( j >= v->sorted.size()) {
	    insert_sorted (idx, sortit);
	    return;
	}

        SortedRecord* rec = v->sorted[j];

	int start = v->text.size();
	
	int len = strlen (text[list[idx]->prefix]);

	char * att_fmt = NULL;
	if (v->att_format) {
	    ostrstream out;
	    current_name = text[list[idx]->oname];
	    v->att_format->printOut(out, *list[idx]->sym);
	    out << ends;
	    current_name = 0;
	    att_fmt = out.str();
	    len += strlen(att_fmt);
	}
	
	char* str = v->text.grow (len+1);
	
	strcpy (str, text[list[idx]->prefix]);
	len = strlen (text[list[idx]->prefix]);
	
	if ( att_fmt) {
	    strcpy(&str[len], att_fmt);
	    len += strlen(att_fmt);
	    delete att_fmt;
	}
	
        str[len] = 0;
	
	WildRegexp yes_filter(v->show_str);
	WildRegexp no_filter(v->hide_str);
	
        char *str1 = str;
        while (isicon(*str1)) {str1++; len--;}

	if((!v->show_str.length() || (yes_filter.Match(str1,len,0) >= 0)) &&
	   (!v->hide_str.length() || !(no_filter.Match(str1,len,0) >= 0)) &&
	   (!v->cli_expr          || v->cli_expr->value(*list[idx]->sym)))  {
	    
	    rec->index = idx;
	    rec->offset = start;
	    // Now form the string for sorting.
	    genString sort_str = "";
	    for (int jj = 0; jj < N_SORT_FIELDS && v->s_info[jj].field_type; jj++) {
		if (jj>0) sort_str += '\t';  // field terminator, later changed to '\0'.
		sort_str += v->s_info[jj].field_type->value(*list[idx]->sym);
	    }
	    rec->sort_offset = v->text.size();
	    int len = sort_str.length();
	    str = v->text.grow(len+1);
	    strcpy(str, (char*)sort_str);
	    while(*str) {
		if (*str == '\t') *str = 0;
		str ++;
	    }
	} else
	    v->text.shrink(start, len+1);
	
        v->sort();
        v = v->next;
    }
}

void ldrListNode::remove_sorted (int i)
{    SortedList*  v = views;
     
     while (v) {
	 int save_idx = -1;
	 for (int j=0; j<v->sorted.size(); j++) {
	     if (v->sorted[j]->index == i) {
		 save_idx = j;
	     } else {
		 if (v->sorted[j]->index > i)
		     v->sorted[j]->index--;
	     }
	 }
	 if (save_idx >= 0) v->sorted.remove(save_idx);
	 v = v->next;
     }
 }

SortedList* ldrList::get_list (genString& sort, genString& format,
    genString& show, genString& hide, int descending_sort) 
{
    Initialize (ldrList::get_list);

    ldrListNodePtr node = checked_cast(ldrListNode,get_root());
    if (node) {
	return node->get_list (sort, format, show, hide, descending_sort) ;
    }
    return NULL;
}

SortedList* ldrListNode::get_list (genString& sort, genString& format,
    genString& show, genString& hide, int descending_sort) 
{
    Initialize (ldrListNode::get_list);

    SortedList* v = views;
    while (v) {
	if (sort == v->sort_str && format == v->format_str &&
	    show == v->show_and_cli_str && hide == v->hide_str &&
	    descending_sort == v->descending_sort)
	    return v;

	v = v->next;
    }

    v = new SortedList (this, sort, format, show, hide, descending_sort);

    for (int i=0; i<list.size(); i++) {
	insert_sorted_instance(v, i, 0);
    }
 
    v->sort();

    v->next = views;
    views = v;
    return v;
}

extern "C" expr * api_parse_expression(const char *);

SortedList::SortedList (ldrListNodePtr l_node, genString& sort, genString& format,
    genString& show, genString& hide, int descending)
{
    genString show_filt, cli_filt;
    
    get_show_and_cli_filts((const char*)show, show_filt, cli_filt);
    
    this->sort_str =  sort;
    this->format_str = format;
    this->show_and_cli_str = show;
    this->show_str = show_filt;
    this->hide_str = hide;
    att_format = NULL;
    this->descending_sort = descending;
    
    if (cli_filt[0])
      cli_expr = api_parse_expression((const char *)cli_filt);
    else
      cli_expr = NULL;

    parse_specs();

    this->list_node = l_node;
}

SortedList::~SortedList ()
{
    if (att_format) delete att_format;
    if (cli_expr) delete cli_expr;
}

void SortedList::parse_specs ()
{
    const char* format_spec = format_str;
    genString new_spec = renew_sort_spec((char*)sort_str);
    const char* sort_spec = new_spec;
    sort_spec ++;   // skip NEW_SORT_SPEC_SIG
    
    int i;
    for(i = 0; i < N_SORT_FIELDS; ++i) {
	s_info[i].field_type = NULL;
	s_info[i].forward = 1;
	s_info[i].index = i;
    }
    
    i = 0;
    if(!format_spec || !*format_spec) {
	symbolAttribute * att = symbolAttribute::get_by_name("on");
	objArr attrs;
	attrs.insert_last(att);
	att_format = new symbolFormat("SortedListFormat", attrs, "%s");
    }
    else {
	objArr attrs;
	genString form = "";
	while(*format_spec) 
	  if(*format_spec == '%') {
	      format_spec++;
	      form += '%';
	      while(isdigit(*format_spec) || (*format_spec == '-')) form += *format_spec++;
	      form += 's';
	      genString name = "";
	      while (*format_spec && *format_spec != ',' && !isspace(*format_spec) && *format_spec != '%') 
		name += *format_spec++;
	      if (*format_spec == ',') format_spec ++;
	      symbolAttribute * att = symbolAttribute::get_by_name((char*)name);
	      attrs.insert_last(att);
	      i++;
	  } else form += *format_spec++;
	att_format = new symbolFormat("ldrListFormat", attrs, form);
    }
    
    i = 0;
    if(!sort_spec || !*sort_spec)
      s_info[0].field_type = symbolAttribute::get_by_name("on");
    else {
	while(*sort_spec && i < N_SORT_FIELDS) {
	    if(*sort_spec++ == '%') {
		if (!*sort_spec) {
		    s_info[i].field_type = NULL;
		    break;
		}
		s_info[i].forward = (*sort_spec++ == 'f');
		genString name = "";
		while (*sort_spec && !isspace(*sort_spec) && *sort_spec != '%') name += (*sort_spec++);
		
		s_info[i].field_type = symbolAttribute::get_by_name(name);

		++i;
	    }
	}
    }
}

int SortedList::compare (const void* a, const void* b) 
{
    SortedRecord* aa = (SortedRecord*)a;
    SortedRecord* bb = (SortedRecord*)b;
 
    int sorted, i;
    symbolAttribute * current_sort_form = NULL;

    // strings containing sort info.

    const char * s1 = SortedList::current_sort_list->text[aa->sort_offset];
    const char * s2 = SortedList::current_sort_list->text[bb->sort_offset];
   
    // For "Formatted String" sorting.

    const char * fs1 = SortedList::current_sort_list->text[aa->offset];
    const char * fs2 = SortedList::current_sort_list->text[bb->offset];
    
    const char *tmp1, *tmp2;

    for (i=0, sorted = 0; !sorted && i < N_SORT_FIELDS && 
	 (current_sort_form = SortedList::current_sort_list->s_info[i].field_type);
	 i++) {

	if (current_sort_form->is_int()) {
	    int n1 = OSapi_atoi(s1);
	    int n2 = OSapi_atoi(s2);
	    
	    if (n1<n2) sorted = -1;
	    else if (n1 > n2) sorted = 1;
		//now have to increment the pointers--we look for a space or a NULL)
		if (!sorted&&i<N_SORT_FIELDS)
		{
			//advance the iterators _only_ if we can and it is necessary.
			int len1=strlen(s1);
			int len2=strlen(s2);
			s1=len1+s1+1;
			s2=s2+len2+1;
		}
	}
	else {
	    
	    bool use_form_str = ( strcmp(current_sort_form->get_name(), "ff") == 0 );
	    
	    if (use_form_str) {
		tmp1 = s1;
		tmp2 = s2;
		s1 = fs1;
		s2 = fs2;
	    }
	    
	    int len1 = strlen(s1);
	    int len2 = strlen(s2);
	    
	    if (!sorted) 
	      sorted = strcmp(s1,s2);
	    
	    if (use_form_str) {
		s1 = tmp1 + strlen(tmp1) + 1;
		s2 = tmp2 + strlen(tmp2) + 1;
	    }
	    else {
		s1 += (len1+1);
		s2 += (len2+1);
	    }
	}
	if (SortedList::current_sort_list->s_info[i].forward == 0) 
	  sorted = -sorted;
    }
    return sorted;
}

void SortedList::sort()
{
    SortedList::current_sort_list = this;
    ldrListNode::current_ldr_node = list_node;
    
    if (s_info[0].field_type)
	OSapi_qsort (sorted[0], sorted.size(), sizeof(SortedRecord), SortedList::compare);
}


