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
#include <LanguageUI.h>
#include <BrowserData.h>
#include <BrowserController.h>
#include <elsLanguage.h>
#include <objArr.h>
#include <customize.h>
#include <str_utils.h>
#include <format.h>
#include <messages.h>

enum FileType {UI, DAT};

const char* browser_data_location = "browser_data/";
const char* browser_data_name = "browser.dat";

LanguageController::LanguageUIPtrArr LanguageController::m_ui;
BrowserData* LanguageController::m_BrowserData = NULL;
BrowserController* LanguageController::m_bcontroller = NULL;
symbolFormat* LanguageController::m_default_format = NULL;

static bool get_file_name(genString& file_name, const elsLanguage* lang, FileType type);
int dis_script_file(const char *script, genString& fname);

LanguageUI* LanguageController::GetLanguageUI(const elsLanguage* lang)
{
    genString file_name;

    for(int i = 0; i < m_ui.size(); i++)
    {
	if (lang == (*m_ui[i])->get_language())
	    return (*m_ui[i]);
    }

    //need to create

    LanguageUI* NewUI = new LanguageUI(lang);
    if (get_file_name(file_name, lang, UI) && NewUI)
      {
	NewUI->init((const char*) file_name);
      }

    if (NewUI)
      m_ui.append(&NewUI);
    
    return NewUI;
}

BrowserData* LanguageController::GetBrowserData()
{
  if (m_BrowserData == NULL)
    {
      genString filename;   
      m_BrowserData = new BrowserData;
      if (get_file_name(filename, NULL, DAT) && m_BrowserData)
	{
	  m_BrowserData->init((const char*)filename);
	}
    }
  return m_BrowserData;
}

BrowserController* LanguageController::GetBrowserController()
{
  if (m_bcontroller == NULL)
    {
      m_bcontroller = new BrowserController();

      const objArr& languages = elsLanguage::list();
      
      for (int i = 0; i < languages.size(); i++)
	{
	  const elsLanguage* lang = (const elsLanguage*)languages[i];
	  m_bcontroller->add_language(lang);
	}

      if (customize::language_enabled(smt_CPLUSPLUS))
	m_bcontroller->add_language(elsLanguage::get_els_language_for_cpp());
      else if (customize::language_enabled(smt_C))
	m_bcontroller->add_language(elsLanguage::get_els_language_for_c());

    }
  return m_bcontroller;
}

static bool get_file_name(genString& file_name, const elsLanguage* lang, FileType type)
{
  genString fn;

  switch (type)
    {
    case UI:
      fn.printf("%s%s", lang->get_name(), ".ui");  
      break;
    case DAT:
      fn = browser_data_name;
      break;
    }

  genString name;
  name.printf("%s%s", browser_data_location, (const char*)fn);

  if (dis_script_file((const char*)name, file_name))
    {
      return true;
    }
  else
    {
      msg("ERROR: Cannot find $1 in $PSETHOME/lib/$2 or in $HOME/.$3") << (const char*) fn << eoarg << browser_data_location << eoarg << browser_data_location << eom;
      return false;
    }
}

void LanguageController::destroy()
{
  if (m_BrowserData)
    {
      delete m_BrowserData;
      m_BrowserData = NULL;
    }

  if (m_bcontroller)
    {
      delete m_bcontroller;
      m_bcontroller = NULL;
    }

  if (m_default_format)
    {
      delete m_default_format;
      m_default_format = NULL;
    }

  int i;

  for (i = 0; i < m_ui.size(); i++)
    delete *m_ui[i];
  m_ui.reset();
}
  
symbolFormat* LanguageController::GetDefaultFormat()
{
  if (m_default_format == NULL)
    {
      objArr atts;
      symbolAttribute* att;

      att = symbolAttribute::get_by_name("kind");
      atts.insert_last(att);

      att = symbolAttribute::get_by_name("type");
      atts.insert_last(att);

      att = symbolAttribute::get_by_name("name");
      atts.insert_last(att);

      att = symbolAttribute::get_by_name("fname");
      atts.insert_last(att);

      SetDefaultFormat("%s%s%s ==> %s", atts);
    }
  return m_default_format;
}
  
void LanguageController::SetDefaultFormat(const char* format_str, 
					  const objArr& attrs)
{
  Initialize(LanguageController::SetDefaultFormat);

  symbolFormat* sf = new symbolFormat("defsymfrm", (objArr&)attrs, 
				      (char*)format_str);
  if (sf)
    {
      if (m_default_format)
	delete m_default_format;

      m_default_format = sf;
    }
}



