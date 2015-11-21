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
#include <LanguageController.h>
#include <msg.h>
#include <BrowserController.h>
#include <LanguageUI.h>
#include <BrowserData.h>
#include <symbolPtr.h>
#include <xrefSymbol.h>
#include <scopeMaster.h>
#include <scopeUnit.h>
#include <fileEntry.h>
#include <str_utils.h>
#include <elsLanguage.h>
#include <Interpreter.h>
#include <closure.h>
#include <messages.h>

void get_ddKinds(genMask& mask, const symbolArr& syms);
static void arr_to_str(genString& str, const genArrCharPtr& list);

BrowserController::BrowserController()
{
  Initialize(BrowserController::BrowserController);
  m_pBrowserData = LanguageController::GetBrowserData();
}

bool BrowserController::add_language(const elsLanguage* lan)
{
  Initialize(BrowserController::add_language);
    for (int i = 0; i < m_languages.size(); i++)
	if (lan == (*m_languages[i])->get_language())
	    return true; /* already have this languages */

    LanguageUI* NewLan = LanguageController::GetLanguageUI(lan);
    if (NewLan)
    {
	m_languages.append(&NewLan);
	return true;
    }
    else
	return false;
	
}

void BrowserController::remove_language(const elsLanguage* lan)
{
  Initialize(BrowserController::remove_language);
    for (int i = 0; i < m_languages.size(); i++)
	if (lan == (*m_languages[i])->get_language())
	{
	    m_languages.remove(i);
	    break;
	}
}
void BrowserController::remove_all_languages()
{
  Initialize(BrowserCcntroller::remove_all_languages);
    m_languages.reset();
}

void BrowserController::get_categories(genArrCharPtr& categories) const
{
  Initialize(BrowserController::get_categories(genArrCharPtr&));
  categories.reset();
  
  //append categories added at run=time
  gen_arr_union(categories, m_categories);

  for (int i = 0; i < m_languages.size(); i++)
    {
      genArrCharPtr temp;
      (*m_languages[i])->get_categories(temp);
      gen_arr_union(categories, temp);
    }
  }

void BrowserController::get_categories(genString& categories) const
{
  Initialize(BrowserController::get_categories(genString&));
  genArrCharPtr temp;
  get_categories(temp);
  arr_to_str(categories, temp);
}

void BrowserController::get_all_queries(genArrCharPtr& queries) const
{
  Initialize(BrowserController::get_all_queries(genArrCharPtr&));
  queries.reset();

  //add queries defined at run-time
  gen_arr_union(queries, m_queries);

  for (int i = 0; i < m_languages.size(); i++)
    {
      genArrCharPtr temp;
      (*m_languages[i])->get_all_queries(temp);
      gen_arr_union(queries, temp);
    }
}

void BrowserController::get_all_queries(genString& queries) const
{
  Initialize(BrowserController::get_all_queries(genString&));
  genArrCharPtr temp;
  get_all_queries(temp);
  arr_to_str(queries, temp);
}

void BrowserController::get_queries(const genMask& sel, 
				    genArrCharPtr& queries) const
{
  Initialize(BrowserController::get_queries(const genMask&, genArrCharPtr&));
    queries.reset();

  //add queries defined at run-time
  gen_arr_union(queries, m_queries);

  for (int i = 0; i < m_languages.size(); i++)
    {
      genArrCharPtr temp;
      (*m_languages[i])->get_queries(sel, temp);
      gen_arr_union(queries, temp);
    }
}

void BrowserController::get_queries(const symbolArr& sel,
				    genArrCharPtr& queries) const
{
  Initialize(BrowserController::get_queries(const symbolArr&, genArrCharPtr&));
    genMask mask;
    get_ddKinds(mask, sel);
    get_queries(mask, queries);
}

void BrowserController::get_queries(const symbolArr& sel,
				    genString& queries) const
{
  genArrCharPtr temp;
  get_queries(sel, temp);
  arr_to_str(queries, temp);
}

void BrowserController::get_queries(const genMask& sel,
				    genString& queries) const
{
  genArrCharPtr temp;
  get_queries(sel, temp);
  arr_to_str(queries, temp);
}

void BrowserController::gen_arr_union(genArrCharPtr& a, const genArrCharPtr& b) const
{
  Initialize(BrowserController::gen_arr_union);
    int a_size = a.size();
    int b_size = b.size();

    for (int i = 0; i < b_size; ++i)
    {
	bool exist = false;
	for (int j = 0; j < a_size; ++j)
	{
	    if (strcmp(*b[i], *a[j]) == 0)
		{
		    exist = true;
		    break;
		}
	}
	if (! exist)
	    a.append(b[i]);
    }
}

void BrowserController::get_queries_with_submenus(const genMask& sel, genArrCharPtr& list) const
{
  Initialize(BrowserController::get_queries_with_submenus(const genMask&, genArrCharPtr&));
  list.reset();

  //add queries defined at run-time
  gen_arr_union(list, m_queries);

  for (int i = 0; i < m_languages.size(); i++)
    {
      genArrCharPtr temp;
      (*m_languages[i])->get_queries_with_submenus(sel, temp);
      gen_arr_union(list, temp);
    }
}

void BrowserController::get_queries_with_submenus(const symbolArr& sel, genArrCharPtr& list) const
{
  Initialize(BrowserController::get_queries_with_submenus(cosnt symbolArr&, genArrCharPtr&));
    genMask mask;
    get_ddKinds(mask, sel);
    get_queries_with_submenus(mask, list);
}

bool BrowserController::is_submenu(char const* item) const
{
  Initialize(BrowserController::is_submenu);
    for (int i = 0; i < m_languages.size(); i++)
	if ((*m_languages[i])->is_submenu(item))
	    return true;
    
    return false;
}

int BrowserController::get_submenu_index(char const* name) const
{
  Initialize(BrowserController::get_submenu_index);
  int index = -1;
  for (int i = 0; i < m_languages.size(); i++)
    {
      index = (*m_languages[i])->get_submenu_index(name);
      if (index >= 0)
	break;
    }
  return index;
}

void BrowserController::get_submenu_items(const genMask& sel, char const* submenu,
					  genArrCharPtr& items) const
{
  Initialize(BrowserController::get_submenu_items(const genMask&, ...));
    items.reset();
    for (int i = 0; i < m_languages.size(); i++)
    {
	genArrCharPtr temp;
	(*m_languages[i])->get_submenu_items(sel, submenu, temp);
	gen_arr_union(items, temp);
    }
}

void BrowserController::get_submenu_items(const symbolArr& sel, char const* submenu,
					  genArrCharPtr& items) const
{
  Initialize(BrowserController::get_submenu_items(const symbolArr&, ...));
    genMask mask;
    get_ddKinds(mask, sel);
    get_submenu_items(mask, submenu, items);
}

int BrowserController::get_category_kind(char const* name) const
{
  Initialize(BrowserController::get_category_kind);
  return m_pBrowserData->get_category_kind(name);
}

char const* BrowserController::get_category_name(unsigned int kind) const
{
  Initialize(BrowserController::get_category_name);
  return m_pBrowserData->get_category_name(kind);
}

bool BrowserController::need_update_queries(const symbolArr& old_sel, 
					    const symbolArr& new_sel) const
{
  Initialize(BrowserController::need_update_queries);

    genMask m1, m2;
    get_ddKinds(m1, old_sel);
    get_ddKinds(m2, new_sel);
    return m1 != m2;
}

static void arr_to_str(genString& str, const genArrCharPtr& list)
{
  Initialize(arr_to_str);

  str = "";
  for (int i = 0; i < list.size(); i++)
    {
      str += "{ ";
      str += *list[i];
      str += " } ";
    }
}

char const* BrowserController::get_query_command(char const* query_name) const
{
	return m_pBrowserData->get_query_command(query_name);
}

char const* BrowserController::get_category_command(char const* cat_name) const
{
	return m_pBrowserData->get_category_command(cat_name);
}

bool BrowserController::add_category(char const* name, char const* command)
{
  Initialize(BrowserController::add_category);
  
  bool ret = false;
  char* cat;
  
  if (m_pBrowserData->add_category(name, command))
    {
      if (my_strcpy(cat, name))
	{
	  m_categories.append(&cat);
	  ret = true;
	}
    }

  return ret;
}

bool BrowserController::add_query(char const* name, char const* command)
{
  Initialize(BrowserController::add_query);
  
  bool ret = false;
  char* query;
  
  if (m_pBrowserData->add_query(name, command))
    {
      if (my_strcpy(query, name))
	{
	  m_queries.append(&query);
	  ret = true;
	}
    }

  return ret;
}

bool BrowserController::remove_category(char const* name)
{
  Initialize(BrowserController::remove_category);
  
  int index = get_arr_index(m_categories, name);

  if (index >= 0)
    {
      m_categories.remove(index);
      m_pBrowserData->remove_category(name);
      return true;
    }
  else
    {
      return false;
    }
}

bool BrowserController::remove_query(char const* name)
{
  Initialize(BrowserController::remove_query);
  
  int index = get_arr_index(m_queries, name);

  if (index >= 0)
    {
      m_queries.remove(index);
      m_pBrowserData->remove_query(name);
      return true;
    }
  else
    {
      return false;
    }
}

/* for an array of commands that can include multiple occurences of
   cmd, for example defines -classes, defines -functions, etc., 
   return a string which replaces all these occurences with one,
   for example, defines -classes -functions. Also, fill ddSelector
   with ddKinds of all the arguments */ 
void BrowserController::get_cmd_args(const genArrCharPtr& cmds,
				     char const* cmd, genString& res,
				     ddSelector& dd) const
{
  Initialize(BrowserController::get_cmd_args);
  
  res = "";
  int len = strlen(cmd);

  for (int i = 0; i < cmds.size(); i++)
    {
      if (strncmp(*cmds[i], cmd, len) == 0)
	{
	  res += ' ';
	  char const* arg = strchr(*cmds[i], '-');
	  res += arg;
	  char const* temp[2];
	  temp[0] = NULL;
	  temp[1] = arg;
	  Interpreter* inter = GetActiveInterpreter();
	  if (inter)
	    {
	      if (inter->SetupSelector(dd, 2, temp) < 0)
		{
		  msg("ERROR: Unknown switch: $1 for $2") << arg << eoarg << cmd << eom;
		}
	    }
	}
    }
}

void BrowserController::handle_closure(char const* cmd, genString& closure_cmd) const
{
  Initialize(BrowserController::handle_closure);
  if (get_closure_state())
    {
      int depth = customize::getIntPref("MaxClosureDepth");
      closure_cmd.printf("query_closure %d \"%s\"", depth, cmd);
    }
  else
    {
      closure_cmd = cmd;
    }
}

/* combine multiple define and uses commands into one */
void BrowserController::process_commands(const genArrCharPtr& cmds_in, genString*& cmds_out, 
					 genString& defines, ddSelector& defs_dd) const
{
  Initialize(BrowserController::process_commands);
  
  int index = -1;

  genString args;
  
  get_cmd_args(cmds_in, "defines", args, defs_dd);
  if (args.length() > 0)
    {
      defines = "defines";
      defines += args;
    }
  else
    defines = 0;

  ddSelector uses_dd;
  get_cmd_args(cmds_in, "uses", args, uses_dd);
  if (args.length() > 0)
    {
      cmds_out[++index] = "uses";
      cmds_out[index] += args;
    }

  for (int i = 0; i < cmds_in.size(); i++)
    {
      if (strncmp(*cmds_in[i], "defines", 7) != 0 &&
	  strncmp(*cmds_in[i], "uses", 4) != 0)
	{
	  *cmds_in[i], cmds_out[++index] = *cmds_in[i];
	}
    }
}

symbolFormat* BrowserController::get_status_line_format(unsigned int kind) const
{
  Initialize(BrowserController::get_status_line_format);

  symbolFormat* ret = NULL;

  for (int i = 0; i < m_languages.size(); i++)
    {
      ret = (*m_languages[i])->get_status_line_format(kind);
      if (ret != NULL)
	break;
    }

  return ret;
}


int BrowserController::GetCategoryCommandCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  if (argc < 2)
    return TCL_ERROR;

  BrowserController* bc = LanguageController::GetBrowserController(); 
  char const* cmd = bc->get_category_command(argv[1]);

  Interpreter *i = (Interpreter *)cd;
 
  if (cmd)
    {
      Tcl_SetResult(i->interp, (char*)cmd, TCL_VOLATILE);
    }
  else
    {
      Tcl_SetResult(i->interp, "", TCL_VOLATILE);
    }

  return TCL_OK;
}

int BrowserController::GetQueryCommandCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  if (argc < 2)
    return TCL_ERROR;

  BrowserController* bc = LanguageController::GetBrowserController(); 
  char const* cmd = bc->get_query_command(argv[1]);

  Interpreter *i = (Interpreter *)cd;
 
  if (cmd)
    {
      Tcl_SetResult(i->interp, (char*)cmd, TCL_VOLATILE);
    }
  else
    {
      Tcl_SetResult(i->interp, "", TCL_VOLATILE);
    }

  return TCL_OK;
}

int BrowserController::GetCategoriesCmd(ClientData cd, Tcl_Interp *interp, 
					int argc, char const *argv[])
{
  BrowserController* bc = LanguageController::GetBrowserController(); 
  genString categories;
  bc->get_categories(categories);

  Interpreter *i = (Interpreter *)cd;
  Tcl_SetResult(i->interp, (char*) categories, TCL_VOLATILE);

  return TCL_OK;
}

int BrowserController::GetQueriesCmd(ClientData cd, Tcl_Interp *interp, 
				     int argc, char const *argv[])
{
  int ret = TCL_OK;

  BrowserController* bc = LanguageController::GetBrowserController(); 
  genString queries = "";
 
  Interpreter *i = (Interpreter *)cd;
  
  if (argc == 1)
    {
      bc->get_all_queries(queries);
    }
  else if (strcmp(argv[1], "-selection") == 0)
    {
      symbolArr sel;
      if(i->ParseArguments(2, argc, argv, sel) != 0)
	{
	  ret = TCL_ERROR;
	}
      else
	{
	  bc->get_queries(sel, queries);
	}
    }
  else if (strcmp(argv[1], "-categories") == 0)
    {
      genMask ddsel;

      for (int i = 2; i < argc; ++i)
	{
	  int kind = bc->get_category_kind(argv[i]);
	  if (kind >= 0)
	    ddsel += kind;
	}
      bc->get_queries((const genMask&)ddsel, queries);
    }
  else
    {
      ret = TCL_ERROR;
    }

  Tcl_SetResult(i->interp, (char*) queries, TCL_VOLATILE);
  
  return ret;
}


