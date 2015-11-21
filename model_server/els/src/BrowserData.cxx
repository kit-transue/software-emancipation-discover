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
#include <genError.h>
#include <msg.h>
#include <BrowserData.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include <str_utils.h>
#include <genString.h>
#include <objArr.h>
#include <LanguageController.h>
#include <messages.h>

BrowserData::Category::Category(const char* n, const char* cmd, uint k, const char* action) :
  name(NULL), command(NULL), kind(k), action_cmd(NULL)
{
  my_strcpy(name, n);
  my_strcpy(command, cmd);
  my_strcpy(action_cmd, action);
}
BrowserData::Category::~Category()
{
  delete [] name;
  delete [] command;
  delete [] action_cmd;
}

BrowserData::Query::Query(const char* aname, const char* acommand, int asubmenu):
     name(NULL), command(NULL), submenu(asubmenu) 
{ 
  my_strcpy(name, aname); 
  my_strcpy(command, acommand);
}
BrowserData::Query::~Query()
{
  delete [] name;
  delete [] command;
}

BrowserData::~BrowserData()
{
  Initialize(BrowserData::~BrowserData);

    clear_gen_arr(m_submenus);
    int i;
    for (i = 0; i < m_categories.size(); i++)
	delete *m_categories[i];
    m_categories.reset();
    for (i = 0; i < m_queries.size(); i++)
	delete *m_queries[i];
    m_queries.reset();
}

/****************** Start Init Section ********************/

bool BrowserData::init(const char* filename)
{
  Initialize(BrowserData::init);

  if (filename == NULL)
    return false;

    FILE* input = OSapi_fopen(filename, "r");
    char buf[MAXBUF];

    if (!input)
      {
	msg("ERROR: Cannot open $1 for reading") << filename << eom;
	return false;
      }
  
  m_LineNum = 0;
  m_FileName = filename;
    while(get_next_line(input, buf, m_LineNum))
    {
      char str[MAXBUF];
      if (extract_str(buf, '[', ']', str))
	{
	  if (strcmp(str, "QUERY") == 0)
	    read_query_section(input);
	  else if (strcmp(str, "CATEGORY") == 0)
	    read_category_section(input);
	  else if (strcmp(str, "SUBMENU") == 0)
	    read_submenu_section(input);
	  else if (strcmp(str, "DEFAULT_SYMBOL_FORMAT") == 0)
	    read_default_symbol_format(input);
	  else
	    {
	      msg("ERROR: $1 Line $2: Unknown Section Name $3") << m_FileName << eoarg << m_LineNum << eoarg << str << eom;
	    }
	}
    }
    OSapi_fclose(input);
    return true;
}

bool BrowserData::read_default_symbol_format(FILE* input)
{
  Initialize(BrowserData::read_default_symbol_format);
  bool ret = false;
  char buf[MAXBUF];
  if (get_next_line(input, buf, m_LineNum))
    {
      objArr attrs;
      char frmt_str[MAXBUF];
      char* next = extract_str(buf, '"', '"', frmt_str);
	
      if (next == NULL)
	{
	  msg("ERROR: $1 Line $2: Error in Status Line format specification") << m_FileName << eoarg << m_LineNum << eom;
	  return false;
	}

      ParseAttributeList(next, attrs, m_FileName, m_LineNum);
  
      LanguageController::SetDefaultFormat(frmt_str, attrs);
      ret = true;
    }

  return ret;
}

/* all categories should be defined on one line with the following format:
   <ws>"Category name"<ws>"Access command"<ws><ddKind> */
bool BrowserData::read_category_section(FILE* input)
{
  Initialize(BrowserData::read_category_section);

    char buf[MAXBUF];

    while(get_next_line(input, buf, m_LineNum))
    {
      char name[MAXBUF];
      char cmd[MAXBUF];
      int kind;
      
      char* next = extract_str(buf, '"', '"', name);
	  
      if (next == NULL)
	{
	  msg("ERROR: $1 Line $2: Syntax error") << m_FileName << eoarg << m_LineNum << eom;
	  continue;
	}

      next = extract_str(next, '"', '"', cmd);

      if (next == NULL)
	{
	  msg("ERROR: $1 Line $2: Access command for \'$3\' is not specified") << m_FileName << eoarg << m_LineNum << eoarg << name << eom;
	  continue;
	}
	      
      char action_cmd[MAXBUF];
      char* action_cmd_ptr = NULL;
      next = extract_num(next, kind);
      if (next == NULL || kind == -1)
	{
	  //assign some not existing ddKind value. 
	  kind = NUM_OF_DDS;
	}
      else
	{
	  next = extract_str(next, '"', '"', action_cmd);
	  if (next)
	    action_cmd_ptr = action_cmd;
	}
      add_category(name, cmd, kind, action_cmd_ptr);
    }
    return true;
}

/* all queries should be defined on one line with the following format:
   <ws>"Query name"<ws>"Access command"<ws><SubmenuIndex> */
bool BrowserData::read_query_section(FILE* input)
{
  Initialize(BrowserData::read_query_section);

    char buf[MAXBUF];

    while(get_next_line(input, buf, m_LineNum))
    {
	char name[MAXBUF];
	char command[MAXBUF];
	int submenu;

	char* next = extract_str(buf, '"', '"', name);
	
	if (next == NULL)
	  {
	    msg("ERROR: $1 Line $2: Syntax error") << m_FileName << eoarg << m_LineNum << eom;
	    continue;
	  }

	next = extract_str(next, '"', '"', command);

	if (next == NULL)
	  {	
	    msg("ERROR: $1 Line $2: Access command for \'$3\' is not specified") << m_FileName << eoarg << m_LineNum << eoarg << name << eom;
	    continue;
	  }  

	next = extract_num(next, submenu);

	if (next == NULL)
	  {
	    submenu = -1;
	  }
	else if (submenu >= (int)m_submenus.size())
	  {
	    msg("ERROR: $1 Line $2: Submenu index $3 for \'$4\' is out of range\nMake sure that submenu with this index was defined in Submenu section") << m_FileName << eoarg << m_LineNum << eoarg << submenu << eoarg << name << eom;
	    submenu = -1;
	  }

	add_query(name, command, submenu);	
    }
    return true;
}

bool BrowserData::read_submenu_section(FILE* input)
{
  Initialize(BrowserData::read_submenu_section);

    char buf[MAXBUF];

    while(get_next_line(input, buf, m_LineNum))
    {
	char name[MAXBUF];
	char* temp = NULL;
	if (extract_str(buf, '"', '"', name))
	  {
	    my_strcpy(temp, name);
	    m_submenus.append(&temp);
	  }
	else
	  {
	    msg("ERROR: $1 Line $2: Syntax error") << m_FileName << eoarg << m_LineNum << eom;
	  }
    }
    return true;
}

/******************* End Init Secion ************************/

bool BrowserData::add_query(const char* name, const char* command, int submenu)
{
  Initialize(BrowserData::add_query);

    Query* NewQuery = new Query(name, command, submenu);
    if (NewQuery)
    {
	m_queries.append(&NewQuery);
	return true;
    }
    else
	return false;
}

bool BrowserData::remove_query(const char* name)
{
  Initialize(BrowserData::remove_query);
  int index = get_query_index(name);
  if (index >= 0)
    {
      m_queries.remove(index);
      return true;
    }
  else
    return false;
}
  
bool BrowserData::add_category(const char* name, const char* cmd, uint kind, const char* action) 
{
  Initialize(BrowserData::add_category);

    Category* NewCategory = new Category(name, cmd, kind, action);
    
    if (NewCategory)
    {
	m_categories.append(&NewCategory);
	return true;
    }
    else
	return false;
}

bool BrowserData::remove_category(const char* name)
{
  Initialize(BrowserData::remove_category);

  int index = get_category_index(name);
  if (index >= 0)
    {
      m_categories.remove(index);
      return true;
    }
  else
    return false;
}
  
int BrowserData::get_category_index(unsigned int kind) const
{
  Initialize(BrowserData::get_category_index);

    int index = -1;
    for (int i = 0; i < m_categories.size(); i++)
    {
	if ((*m_categories[i])->kind == kind)
	{
	    index = i;
	    break;
	}
    }
    return index;
}

int BrowserData::get_category_index(const char* name) const
{
  Initialize(BrowserData::get_category_index);

  int index = -1;
  for (int i = 0; i < m_categories.size(); i++)
    {
      if (strcmp(name, (*m_categories[i])->name) == 0)
	{
	  index = i;
	  break;
	}
    }
  return index;
}

int BrowserData::get_query_index(const char* name) const
{
  Initialize(BrowserData::get_query_index);

  int index = -1;
  for (int i = 0; i < m_queries.size(); i++)
    {
      if (strcmp(name, (*m_queries[i])->name) == 0)
	{
	  index = i;
	  break;
	}
    }
  return index;
}

int BrowserData::get_category_kind(const char* name) const
{
  Initialize(BrowserData::get_category_kind);

  int index = get_category_index(name);
  if (index >= 0)
    return (*m_categories[index])->kind;
  else
    return -1;
}

const char* BrowserData::get_category_name(unsigned int kind) const
{
  Initialize(BrowserData::get_category_name);

    int index = get_category_index(kind);
    if (index)
	return (*m_categories[index])->name;
    else
	return NULL;
}

bool BrowserData::is_category(const char* name) const
{
    return get_category_index(name) >= 0;
}

bool BrowserData::is_query(const char* name) const
{
    return get_query_index(name) >= 0;
}

bool BrowserData::is_submenu(const char* name) const
{
    return get_submenu_index(name) >= 0;
}

int BrowserData::get_submenu_index(const char* name) const
{
  Initialize(BrowserData::get_submenu_index);

  int index = -1;
  for (int i = 0; i < m_submenus.size(); i++)
    if (strcmp(name, *m_submenus[i]) == 0)
      {
	index = i;
	break;
      }
  return index;
}
	
int BrowserData::get_query_submenu(unsigned int query_index) const
{
  return (*m_queries[query_index])->submenu;
}

const char* BrowserData::get_query_name_from_index(unsigned int index) const
{
  if (index >= 0 && index < m_queries.size())
    return (*m_queries[index])->name;
  else
    return NULL;
}

const char* BrowserData::get_category_name_from_index(unsigned int index) const
{
  if (index >= 0 && index < m_categories.size())
    return (*m_categories[index])->name;
  else
    return NULL;
}

int BrowserData::get_category_kind_from_index(unsigned int index) const
{
  if (index >= 0 && index < m_categories.size())
    return (*m_categories[index])->kind;
  else
    return -1;
}

const char* BrowserData::get_submenu_title_from_index(unsigned int index) const
{
  return *m_submenus[index];
}

const char* BrowserData::get_query_command(const char* query_name) const
{
  Initialize(BrowserData::get_query_command);

  int index = get_query_index(query_name);
  if (index >= 0)
    return (*m_queries[index])->command;
  else
    return NULL;
}

const char* BrowserData::get_category_command(const char* cat_name) const
{
  Initialize(BrowserData::get_category_command);

  int index = get_category_index(cat_name);
  if (index >= 0)
    return (*m_categories[index])->command;
  else
    return NULL;
}

const char* BrowserData::get_action_cmd(unsigned int kind) const
{
  Initialize(BrowserData::get_action_cmd);
  int index = get_category_index(kind);
  if (index >= 0)
    return (*m_categories[index])->action_cmd;
  else
    return NULL;
}




