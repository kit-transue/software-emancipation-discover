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
#ifndef _BrowserData_h
#define _BrowserData_h

#ifndef _cLibraryFunctions_h
#include <cLibraryFunctions.h>
#endif

#ifndef _genArrCharPtr_h
#include <genArrCharPtr.h>
#endif

#ifndef _ddKind_h
#include <ddKind.h>
#endif

char* my_strcpy(char*& to, const char* from);

class LanguageUI;

class BrowserData
{
 public:
  ~BrowserData();
  bool init(const char* filename);

  bool is_query(const char* name) const;
  bool is_category(const char* name) const;
  bool is_submenu(const char *name) const;

  const char* get_category_command(const char* cat_name) const;
  const char* get_query_command(const char* query_name) const;

  int get_category_kind(const char* name) const;

  int get_submenu_index(const char* subname) const;
  const char* get_category_name(unsigned int kind) const;

  int get_query_submenu(unsigned int query_index) const;

  int get_category_index(const char* name) const;
  int get_category_index(unsigned int kind) const;
  int get_query_index(const char* name) const;

  const char* get_query_name_from_index(unsigned int index) const;
  const char* get_category_name_from_index(unsigned int index) const;
  int get_category_kind_from_index(unsigned int index) const;
 
  const char* get_submenu_title_from_index(unsigned int index) const;

  bool add_category(const char* name, const char* cmd, 
		    unsigned int kind = NUM_OF_DDS, const char* action = NULL);
  bool add_query(const char* name, const char* cmd, int submenu = -1);
  bool remove_category(const char* name);
  bool remove_query(const char* name);

  const char* get_action_cmd(unsigned int kind) const;

    struct Category
    {
      Category(const char* n, const char* command, unsigned int k, const char* action_cmd);
      ~Category();
      char* name;
      char* command;
      unsigned int kind; 
      char* action_cmd;
    };
   
    struct Query
    {
	Query(const char* name, const char* command, int sub = -1);
	~Query();
	char* name;
	char* command;
	int submenu;
    };

    typedef Category* CategoryPtr;
    typedef Query* QueryPtr;
    genArr(CategoryPtr);
    genArr(QueryPtr);


private:
  friend class LanguageUI;
  
  bool read_category_section(FILE*);
  bool read_query_section(FILE*);
  bool read_submenu_section(FILE*);
  bool read_default_symbol_format(FILE* input);

  private:

    CategoryPtrArr m_categories;
    QueryPtrArr m_queries;
    genArrCharPtr m_submenus;

  //used for error reporint
  const char* m_FileName;
  int m_LineNum;
};

#endif






