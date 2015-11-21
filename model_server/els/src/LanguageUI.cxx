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
#include <LanguageUI.h>
#include <msg.h>
#include <BrowserData.h>
#include <LanguageController.h>
#include <str_utils.h>
#include <symbolArr.h>
#include <format.h>
#include <elsLanguage.h>
#include <customize.h>
#include <messages.h>

LanguageUI::Category::Category(const char* n, unsigned int k, const genMask& q, 
			       bool s, symbolFormat* sline) : 
	name(NULL), kind(k), queries(q), show(s), statusLine(sline)
{ 
	my_strcpy(name, n); 
}

LanguageUI::Category::~Category() 
{ 
	delete [] name; 
	delete  statusLine;
}

LanguageUI::~LanguageUI()
{
  Initialize(~LanguageUI);

    for (int i = 0; i < m_categories.size(); i++)
	delete *m_categories[i];
    m_categories.reset();
}

/****************** Start Init Section ********************/

bool LanguageUI::init(const char* filename)
{
    Initialize(LanguageUI::init);

    m_BrowserData = LanguageController::GetBrowserData();

    if (m_BrowserData == NULL || filename == NULL)
      return false;

    FILE* input = OSapi_fopen(filename, "r");

    if (!input)
      {
	msg("ERROR: Cannot open $1 for reading") << filename << eom;
	return false;
      }
    
    m_LineNum = 0;
    m_FileName = filename;

    //read categories
    char buf[MAXBUF];

    while(get_next_line(input, buf, m_LineNum))
    {
	char name[MAXBUF];
	int kind;
	bool show = true;
	genMask queries;

	char* next = extract_str(buf, '"', '"', name);
	
	if (next == NULL)
	  {
	    msg("$1 Line $2: Syntax error", error_sev) << m_FileName << eoarg << m_LineNum << eoarg << eom;
	    read_queries(queries, input);
	    continue;
	  }

	kind = m_BrowserData->get_category_kind(name);
	if (kind < 0)
	  {
	    msg("$1 Line $2: Category \'$3\' was not defined", error_sev) << m_FileName << eoarg << m_LineNum << eoarg << name << eom;
	    read_queries(queries, input);
	    continue;
	  }

	//check whether need to show in the Categories column
	char temp[MAXBUF];
	next = extract_str(next, '[', ']', temp);
	
	if (next == NULL)
	  {
	    msg("ERROR: $1 Line $2: Syntax error") << m_FileName << eoarg << m_LineNum << eom;
	    read_queries(queries, input);
	    continue;
	  }
	else if (strcmp(temp, "SHOW"))
	  {
	    if (strncmp(temp, "SHOW", 4))
	      show = false;
	    else
	      {
		char *pref_name = &temp[4];
		while (isspace(*pref_name))
		  ++pref_name;
		int pref_name_len = strlen (pref_name);
		if (*pref_name == '"' && pref_name_len > 2 && pref_name[pref_name_len-1]== '"')
		  {
		    pref_name[pref_name_len-1] = 0;
		    ++pref_name;
		    pref_name_len -= 2;
		  }
		if (pref_name_len > 0)
		  {
		    bool pref_value = customize::getBoolPref(pref_name);
		    if (pref_value)
		      show = false;
		  }
	      }
	  }
	
	const symbolFormat* sf;
	next = extract_str(next, '[', ']', temp);
	if (next)
	  sf = parse_format(temp, kind);
	else
	  sf = NULL;

	if (read_queries(queries, input))
	{
	    add_category(name, kind, queries, show, sf);
	}	
    }

    OSapi_fclose(input);
    return true;
}

const symbolFormat* LanguageUI::parse_format(char* str, int kind)
{
  objArr attrs;
  char frmt_str[MAXBUF];
  char* next = extract_str(str, '"', '"', frmt_str);
	
  if (next == NULL)
    {
      msg("ERROR: $1 Line $2: Error in Status Line format specification") << m_FileName << eoarg << m_LineNum << eom;
      return NULL;
    }

  ParseAttributeList(next, attrs, m_FileName, m_LineNum);
  
  if (attrs.size() > 0)
    {
      genString FormatName;
      FormatName.printf("%s%d", (const char*) m_language->get_name(), kind);
      symbolFormat* sf = new symbolFormat((char*)FormatName, attrs, frmt_str);
      return sf;
    }
  else
    return NULL;
}

bool LanguageUI::read_queries(genMask& mask, FILE* input)
{
  Initialize(LanguageUI::read_queries);

    char buf[MAXBUF];

    while(get_next_line(input, buf, m_LineNum))
    {
	char query_name[MAXBUF];
	char *next = extract_str(buf, '"', '"', query_name);

	if (next)
	{
	  char pref_name [MAXBUF];
	  next =  extract_str(next, '"', '"', pref_name);
	  if (next && pref_name[0])
	    {
	      bool do_not_add = customize::getBoolPref(pref_name);
	      if (do_not_add)
		continue;
	    }
	  int query_index = m_BrowserData->get_query_index(query_name);
	  if (query_index >= 0)
	    {
	      mask += query_index;
	    }
	  else
	    {
	      msg("ERROR: $1 Line $2: Query \'$3\' was not defined") << m_FileName << eoarg << m_LineNum << eoarg << query_name << eom;
	    }
	}
	else
	  {
	    msg("ERROR: $1 Line $2: Syntax error") << m_FileName << eoarg << m_LineNum << eom;
	  }
    }

    return true;
}

/******************* End Init Secion ************************/

bool LanguageUI::add_category(const char* name, unsigned int kind, 
			      const genMask& queries, bool show, 
			      const symbolFormat* statusLine)
{
  Initialize(LanguageUI::add_category);

  Category* NewCategory = new Category(name, kind, queries, 
				       show, (symbolFormat*)statusLine);
    
    if (NewCategory)
    {
	m_categories.append(&NewCategory);
	return true;
    }
    else
	return false;
}

/* get the list of all categories */
void LanguageUI::get_categories(genArrCharPtr& list) const
{
  Initialize(LanguageUI::get_categories);

    list.reset();
    list.provide(m_categories.size());

    for (int i = 0; i < m_categories.size(); i++)
    {
	if ((*m_categories[i])->show)
	    list.append(&(*m_categories[i])->name);
    }
}

/* get queries only for selected categories. sel is a genMask with ddKinds of all
   selected symbols */
void LanguageUI::get_queries(const genMask& sel, genArrCharPtr& list) const
{
  Initialize(LanguageUI::get_queries);

    genMask mask;
    list.reset();

    get_query_mask(sel, mask);

    for (int i = mask.peek(); i >= 0; i = mask.peek(i))
      {
	const char* temp = m_BrowserData->get_query_name_from_index(i); 
	list.append((char**)&temp);
      }
}

/* put indexes of queries for given selection into mask */
void LanguageUI::get_query_mask(const genMask& sel, genMask& mask) const
{
  Initialize(LanguageUI::get_query_mask);
    mask.clear();
    for (int dd = sel.peek(); dd >= 0; dd = sel.peek(dd))
    {
	for (int i = 0; i < m_categories.size(); i++)
	  {
	    if ((*m_categories[i])->kind == dd)
	      {
		mask += (*m_categories[i])->queries;
		break;
	      }
	  }
    }
}

	
/* return the list of all queries which should be available for given selection.
   If a query belongs to a submenu, put submenu name in stead of the query
   itself into the list */
void LanguageUI::get_queries_with_submenus(const genMask& sel, 
					   genArrCharPtr& list) const
{
  Initialize(LanguageUI::get_queries_with_submenus);

    int i;
    list.reset();
    bool* AlreadyAdded = new bool [m_BrowserData->m_submenus.size()];
    if (AlreadyAdded == NULL) return;
    for (i = 0; i < m_BrowserData->m_submenus.size(); i++) AlreadyAdded[i] = false;

    genMask mask;

    get_query_mask(sel, mask);

    for (i = mask.peek(); i >= 0; i = mask.peek(i))
    {
	int submenu = m_BrowserData->get_query_submenu(i);
	if (submenu == -1)
	  {
	    const char* temp = m_BrowserData->get_query_name_from_index(i); 
	    list.append((char**) &temp);
	  }
	else if (!AlreadyAdded[submenu])
	{
	    AlreadyAdded[submenu] = true;
	    const char* temp = m_BrowserData->get_submenu_title_from_index(submenu);
	    list.append((char**)&temp);
	}
    }

    delete [] AlreadyAdded;
}

/* return only those submenu items that should be available for current selection */
void LanguageUI::get_submenu_items(const genMask& sel, const char* submenu,
				   genArrCharPtr& list) const
{
  Initialize(LanguageUI::get_submenu_items);

    list.reset();
    int i;

    int index = m_BrowserData->get_submenu_index(submenu);
    if (index == -1)
	return;

    genMask mask;
    get_query_mask(sel, mask);

    for (i = mask.peek(); i >= 0; i = mask.peek(i))
    {
	int submenu = m_BrowserData->get_query_submenu(i);
	if (submenu == index)
	  {
	    const char* temp = m_BrowserData->get_query_name_from_index(i);
	    list.append((char**)&temp);
	  }
    }
}

void get_ddKinds(genMask& mask, const symbolArr& syms);

void LanguageUI::get_queries(const symbolArr& sel,
			     genArrCharPtr& queries) const
{
  Initialize(LanguageUI::get_queries(const symbolArr& sel, genArrCharPtr&));

    genMask mask;
    get_ddKinds(mask, sel);
    get_queries(mask, queries);
}

void LanguageUI::get_queries_with_submenus(const symbolArr& sel,
					   genArrCharPtr& queries) const
{
  Initialize(LanguageUI::get_queries_with_submenus);

    genMask mask;
    get_ddKinds(mask, sel);    
    get_queries_with_submenus(mask, queries);
}

void LanguageUI::get_submenu_items(const symbolArr& sel, const char* submenu,
				   genArrCharPtr& items) const
{
  Initialize(get_submenu_items);

    genMask mask;
    get_ddKinds(mask, sel);
    get_submenu_items(mask, submenu, items);
}

void get_ddKinds(genMask& mask, const symbolArr& syms) 
{
  Initialize(get_ddKinds);

    symbolPtr sym;
    ForEachS(sym, syms)
    {
	mask += sym.get_kind();
    }
}

int LanguageUI::get_category_kind(const char* name) const
{
  return m_BrowserData->get_category_kind(name);
}

const char* LanguageUI::get_category_name(unsigned int kind) const
{
  return m_BrowserData->get_category_name(kind);
}

bool LanguageUI::is_submenu(const char* name) const
{
  return m_BrowserData->is_submenu(name);
}

int LanguageUI::get_submenu_index(const char* name) const
{
  return m_BrowserData->get_submenu_index(name);
}

void LanguageUI::get_all_queries(genArrCharPtr& list) const
{
  Initialize(LanguageUI::get_all_queries);

  list.reset();
  genMask mask;
  for (int i = 0; i < m_categories.size(); i++)
    mask += (*m_categories[i])->kind;

  get_queries(mask, list);
}

symbolFormat* LanguageUI::get_status_line_format(unsigned int kind) const
{
  Initialize(LanguageUI::get_status_line);
  for (int i = 0; i < m_categories.size(); i++)
    {
      if ((*m_categories[i])->kind == kind)
	return (*m_categories[i])->statusLine;
    }
  
  return NULL;
}









