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
#ifndef _BrowserController_h
#define _BrowserController_h

#ifndef _LanguageUI_h
#include <LanguageUI.h>
#endif

#include <tcl.h>

class elsLanguage;

class BrowserController
{
 public:
  //commands for Access interface

  static int GetCategoryCommandCmd(ClientData cd, Tcl_Interp *interp, 
				   int argc, char const *argv[]);
  static int GetQueryCommandCmd(ClientData cd, Tcl_Interp *interp, 
				int argc, char const *argv[]);
  static int GetCategoriesCmd(ClientData cd, Tcl_Interp *interp, 
			      int argc, char const *argv[]);
  static int GetQueriesCmd(ClientData cd, Tcl_Interp *interp, 
			   int argc, char const *argv[]);

  public:
    BrowserController();
    bool add_language(const elsLanguage*);
    void remove_language(const elsLanguage*);
    void remove_all_languages();

    const char* get_category_command(const char* cat_name) const;
    const char* get_query_command(const char* query_name) const;

    bool need_update_queries(const symbolArr& old_sel, 
			     const symbolArr& new_sel) const;

    void get_categories(genArrCharPtr& categories) const;
    void get_categories(genString& categories) const;
    void get_all_queries(genArrCharPtr& queries) const;
    void get_all_queries(genString& queries) const;
    void get_queries(const genMask& categories,
		     genArrCharPtr& queries) const;
    void get_queries(const symbolArr& categories,
		     genArrCharPtr& queries) const;
    void get_queries(const symbolArr& categories,
		     genString& queries) const;
    void get_queries(const genMask& categories,
		     genString& queries) const;

    void get_queries_with_submenus(const genMask& categories, 
				   genArrCharPtr& queries) const;
    void get_queries_with_submenus(const symbolArr& categories,
				   genArrCharPtr& queries) const;

    bool is_submenu(const char *name) const;
    int get_submenu_index(const char* name) const;
    void get_submenu_items(const genMask& sel, const char* submenu, 
			   genArrCharPtr& items) const;
    void get_submenu_items(const symbolArr& sel, const char* submenu,
			   genArrCharPtr& items) const;

    int get_category_kind(const char* CategoryName) const;
    const char* get_category_name(unsigned int kind) const;

    bool add_category(const char* name, const char* command);
    bool add_query(const char* name, const char* command);
    bool remove_category(const char* name);
    bool remove_query(const char* name);

    void process_commands(const genArrCharPtr& cmds_in, genString*& cmds_out,
			  genString& defines, ddSelector& defs_dd) const;
    void get_cmd_args(const genArrCharPtr& cmds,
		      const char* cmd, genString& res,
		      ddSelector& dd) const;
    void handle_closure(const char* cmd, genString& closure_cmd) const;

    symbolFormat* get_status_line_format(unsigned int kind) const;

    typedef LanguageUI* LanguageUIPtr;
  private:
    void gen_arr_union(genArrCharPtr& ar1, const genArrCharPtr& ar2) const;

    genArr(LanguageUIPtr);
    LanguageUIPtrArr m_languages;
    BrowserData* m_pBrowserData;

    genArrCharPtr m_categories;
    genArrCharPtr m_queries;
};

int get_display_string_from_symbol(symbolPtr sym, genString& line);

#endif











