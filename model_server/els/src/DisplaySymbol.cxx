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
#include <sstream>

#include <LanguageUI.h>
#include <LanguageController.h>
#include <format.h>
#include <cmd.h>
#include <cmd_enums.h>
#include <elsLanguage.h>
#include <xrefSymbol.h>
#include <str_utils.h>
#include <BrowserController.h>

void project_query(symbolPtr, linkType, ddKind, symbolArr&);
int get_display_string_from_symbol(symbolPtr sym, genString& str)
{
  Initialize(BrowserController::get_display_string_from_symbol);

  if (sym.isnull() || (sym.xrisnull() && !sym.without_xrefp()))
    {
      str = "";
      return 0;
    }

  ddKind kind = sym.get_kind();
  symbolFormat* sf = NULL;
  const elsLanguage* lang = elsLanguage::get_els_language(sym);

  if (lang)
    {
      LanguageUI* langui = LanguageController::GetLanguageUI(lang);
      if (langui)
	{
	  sf = langui->get_status_line_format(kind);	
	}
    }

  if (sf == NULL)
    {
      /* could not determine symbol language, try to get display
	 format for an active language */
      BrowserController* bc = LanguageController::GetBrowserController();
      sf = bc->get_status_line_format(kind);
      if (sf == NULL)
	{
	  sf = LanguageController::GetDefaultFormat();
	}  
    }

  ostringstream os;
  sf->printOut(os, sym);
  os << ends;

  char const *os_str = os.str().c_str();
  str = os_str;

  if (os_str)
    delete os_str;

  if (str.is_null())
    str = "";

  return str.length();
}














