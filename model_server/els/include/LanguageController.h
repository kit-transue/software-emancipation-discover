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
#ifndef _LANGUAGECONTROLLER_H
#define _LANGUAGECONTROLLER_H

#ifndef _genArr_h
#include <genArr.h>
#endif

#ifndef _objCommon_h
#include <objCommon.h>
#endif

class LanguageUI;
class BrowserData;
class BrowserController;
class elsLanguage;
class symbolFormat;

class LanguageController
{
  public:
  static void destroy();

  static LanguageUI* GetLanguageUI(const elsLanguage* lan);
  static BrowserData* GetBrowserData();
  static BrowserController* GetBrowserController();
  static symbolFormat* GetDefaultFormat();
  static void SetDefaultFormat(const char* format_str, const objArr& AttList);
  typedef LanguageUI* LanguageUIPtr;
  genArr(LanguageUIPtr);

  private:

  static LanguageUIPtrArr m_ui;
  static BrowserData* m_BrowserData;
  static BrowserController* m_bcontroller;
  static symbolFormat* m_default_format;
};


#endif









