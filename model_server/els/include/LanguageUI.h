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
#ifndef _LanguageUI_h
#define _LanguageUI_h

#ifndef _objCommon_h
#include <objCommon.h>
#endif

#ifndef _cLibraryFunctions_h
#include <cLibraryFunctions.h>
#endif

#ifndef _genMask_h
#include <genMask.h>
#endif

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif

#ifndef _genArrCharPtr_h
#include <genArrCharPtr.h>
#endif

class BrowserData;
class elsLanguage;
class symbolFormat;

class LanguageUI
{
  public:
    LanguageUI(const elsLanguage* lang) : m_language(lang) {}
    ~LanguageUI();
    bool init(const char* filename);

    const elsLanguage* get_language() const { return m_language; }

    void get_categories(genArrCharPtr& categories) const;
    void get_all_queries(genArrCharPtr& queries) const;
    void get_queries(const genMask& categories,
		     genArrCharPtr& queries) const;
    void get_queries(const symbolArr& syms,
		     genArrCharPtr& queries) const;

    void get_queries_with_submenus(const genMask& categories, 
				   genArrCharPtr& queries) const;
    void get_queries_with_submenus(const symbolArr& sel,
				   genArrCharPtr& queries) const;
    bool is_submenu(const char *name) const;
    int get_submenu_index(const char* name) const;
    void get_submenu_items(const genMask& sel, const char* submenu, 
			   genArrCharPtr& items) const;
    void get_submenu_items(const symbolArr& sel, const char* submenu,
			   genArrCharPtr& items) const;


    int get_category_kind(const char* CategoryName) const;
    const char* get_category_name(unsigned int kind) const;

    symbolFormat* get_status_line_format(unsigned int kind) const;

    struct Category
    {
	Category(const char* n, unsigned int k, const genMask& q, bool s, 
		 symbolFormat* statusline = NULL);
	~Category();
	char* name;
	unsigned int kind; //ddKind
	genMask queries; //queries represented as indexes into queries array
	bool show;
	symbolFormat* statusLine;
    };
   
    typedef Category* CategoryPtr;
    genArr(CategoryPtr);

private:

    bool add_category(const char* name, unsigned int kind, 
		      const genMask& queries, bool show, 
		      const symbolFormat* statusLine);  
    bool read_queries(genMask& mask, FILE* input);
    const symbolFormat* parse_format(char* str, int kind);

    void get_query_mask(const genMask& categories, genMask& queries) const;

  private:
    BrowserData* m_BrowserData;
    CategoryPtrArr m_categories;
    const elsLanguage* m_language;

    //used for error reporting
    const char* m_FileName;
    int m_LineNum;
};

#endif






